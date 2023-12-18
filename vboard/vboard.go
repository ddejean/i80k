// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package main

import (
	"flag"
	"log"
	"os"
	"unsafe"
	"vboard/kvm"

	"golang.org/x/sys/unix"
)

var (
	r = flag.String("r", "", "path to the ROM file")
)

func main() {
	log.SetFlags(0)
	flag.Parse()

	// Read the ROM file.
	rom, err := os.ReadFile(*r)
	if err != nil {
		log.Fatal("failed to open ROM file: ", err)
	}

	f, err := kvm.OpenDevice("/dev/kvm")
	if err != nil {
		log.Fatal("failed to open KVM device: ", err)
	}

	log.Printf("vboard starting on KVM, API version: %d", kvm.GetApiVersion())

	vm, err := kvm.NewVM(f)
	if err != nil {
		log.Fatal("failed to create VM: ", err)
	}

	// Allocate guest memory.
	sz := 1024 * 1024
	addr, _, errno := unix.RawSyscall6(
		unix.SYS_MMAP,
		0, // Suggested address.
		uintptr(sz),
		unix.PROT_READ|unix.PROT_WRITE,
		unix.MAP_PRIVATE|unix.MAP_ANONYMOUS|unix.MAP_NORESERVE,
		0, 0)
	if errno != 0 {
		log.Fatalf("failed to mmap guest memory: %v", errno)
	}
	guestMemory := unsafe.Slice((*byte)(unsafe.Pointer(addr)), sz)

	// Copy ROM to guest memory.
	copy(guestMemory[0xe0000:], rom)

	if err := vm.SetUserspaceMemoryRegion(&kvm.UserspaceMemoryRegion{
		Slot:          0,
		Flags:         0,
		GuestPhysAddr: 0,
		MemorySize:    uint64(sz),
		UserspaceAddr: uint64(addr),
	}); err != nil {
		log.Fatalf("failed to attach memory to guest: %v", err)
	}

	vcpu, err := vm.CreateVCPU()
	if err != nil {
		log.Fatal("failed to create vCPU: ", err)
	}

	var sregs kvm.SRegs
	if err := vcpu.GetSystemRegs(&sregs); err != nil {
		log.Fatal("failed to read system registers: ", err)
	}

	sregs.CS.Selector = 0
	sregs.CS.Base = 0xFFFF0
	sregs.IDT.Base = 0
	sregs.IDT.Limit = 0x3FF

	if err := vcpu.SetSystemRegs(&sregs); err != nil {
		log.Fatal("failed to write system registers: ", err)
	}

	regs := &kvm.Regs{
		RFLAGS: 2,
		RIP:    0,
	}

	if err := vcpu.SetRegs(regs); err != nil {
		log.Fatal("failed to write registers: ", err)
	}

	for {
		if err := vcpu.Run(); err != nil {
			log.Fatal(err)
		}
		switch vcpu.Context().ExitReason {
		case kvm.KVM_EXIT_HLT:
			log.Println("HLT: interrupts not implemented. Leaving.")
			return
		case kvm.KVM_EXIT_IO:
			log.Printf("IO: %v", vcpu.Context().IO())
		default:
		}
	}
}
