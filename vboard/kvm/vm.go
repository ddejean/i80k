// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package kvm

import (
	"fmt"
	"os"
	"syscall"
	"unsafe"

	"golang.org/x/sys/unix"
)

type VM struct {
	fd uintptr
}

func NewVM(deviceFile *os.File) (*VM, error) {
	var (
		fd    uintptr
		errno unix.Errno
	)
	for {
		fd, _, errno = unix.Syscall(unix.SYS_IOCTL, deviceFile.Fd(), KVM_CREATE_VM, 0)
		if errno == unix.EINTR {
			continue
		}
		if errno != 0 {
			return nil, fmt.Errorf("failed to create VM: %v", errno)
		}
		break
	}
	return &VM{
		fd: fd,
	}, nil
}

func (vm *VM) SetUserspaceMemoryRegion(region *UserspaceMemoryRegion) error {
	if region == nil {
		return fmt.Errorf("failed to set user memory region: nil region")
	}
	// Set the region.
	// Note: syscall.RawSyscall is used to fit the nosplit stack limit.
	_, _, errno := syscall.RawSyscall(
		unix.SYS_IOCTL,
		uintptr(vm.fd),
		KVM_SET_USER_MEMORY_REGION,
		uintptr(unsafe.Pointer(region)))
	if errno != 0 {
		return fmt.Errorf("failed to set user memory region: %v", errno)
	}
	return nil
}

func (vm *VM) CreateVCPU() (*VCPU, error) {
	fd, _, errno := unix.Syscall(unix.SYS_IOCTL, vm.fd, KVM_CREATE_VCPU, 0)
	if errno != 0 {
		return nil, fmt.Errorf("failed to create vCPU: %v", errno)
	}

	mapRun, _, errno := unix.RawSyscall6(
		unix.SYS_MMAP,
		0, // Suggested address.
		mapRunSize,
		unix.PROT_READ|unix.PROT_WRITE,
		unix.MAP_SHARED,
		fd, 0)
	if errno != 0 {
		return nil, fmt.Errorf("failed to mmap vCPU map run context: %v", errno)
	}

	return &VCPU{
		fd:  fd,
		run: (*Run)(unsafe.Pointer(mapRun)),
	}, nil
}
