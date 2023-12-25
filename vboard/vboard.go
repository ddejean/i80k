// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"unsafe"
	"vboard/devices"
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

	if err := vm.CreateIRQChip(); err != nil {
		log.Fatalf("failed to create interrupt controller: %v", err)
	}

	vcpu, err := vm.CreateVCPU()
	if err != nil {
		log.Fatal("failed to create vCPU: ", err)
	}

	var sregs kvm.SRegs
	if err := vcpu.GetSystemRegs(&sregs); err != nil {
		log.Fatal("failed to read system registers: ", err)
	}

	// The 808{6,8} interrupt table is located at address 0000:0000 and has a
	// 256 entries of 4 bytes.
	sregs.IDT.Base = 0
	sregs.IDT.Limit = 0x400
	// The 808{6,8} starts at address F000:FFF0.
	sregs.CS.Selector = 0
	sregs.CS.Base = 0xFFFF0

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

	// Create and IO mapper to match ports and devices.
	iom := newIOMapper()

	timer := devices.NewP8254()
	timer.Start()
	defer timer.Stop()
	iom.Add(0x40, timer)

	uart := devices.NewP16550(func(level uint32) {
		if err := vm.SetIRQLine(4, level); err != nil {
			log.Printf("failed to set IRQ line %d to level %d", 4, level)
		}
	})
	uart.Start()
	defer uart.Stop()
	iom.Add(0x3f8, uart)

	for {
		if err := vcpu.Run(); err != nil {
			log.Fatal(err)
		}
		switch vcpu.Context().ExitReason {
		case kvm.KVM_EXIT_IO:
			if err := doIO(iom, vcpu); err != nil {
				log.Fatal("IO failed: ", err)
			}

		default:
		}
	}
}

func doIO(iom *ioMapper, cpu *kvm.VCPU) error {
	io := cpu.Context().IO()

	// Obtain a reference to the data contained in vCPU mapped memory.
	dataptr := cpu.ContextData(io.DataOffset)

	switch io.Direction {
	case kvm.KVM_EXIT_IO_IN:
		var err error
		*dataptr, err = iom.Read(io.Port)
		if err != nil {
			log.Println(err)
			return nil
			//return fmt.Errorf("failed I/O read: %v", err)
		}
	case kvm.KVM_EXIT_IO_OUT:
		if err := iom.Write(io.Port, *dataptr); err != nil {
			log.Println(err)
			return nil
			//return fmt.Errorf("failed I/O write: %v", err)
		}
	default:
		return fmt.Errorf("invalid I/O direction %d", io.Direction)
	}
	return nil
}
