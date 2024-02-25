// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package kvm

import (
	"fmt"
	"unsafe"

	"golang.org/x/sys/unix"
)

type VCPU struct {
	fd  uintptr
	run *Run
}

func (c *VCPU) GetSystemRegs(sregs *SRegs) error {
	if _, _, errno := unix.RawSyscall(unix.SYS_IOCTL,
		c.fd,
		KVM_GET_SREGS,
		uintptr(unsafe.Pointer(sregs))); errno != 0 {
		return fmt.Errorf("failed to get system registers: %v", errno)
	}
	return nil
}

func (c *VCPU) SetSystemRegs(sregs *SRegs) error {
	if _, _, errno := unix.RawSyscall(unix.SYS_IOCTL,
		c.fd,
		KVM_SET_SREGS,
		uintptr(unsafe.Pointer(sregs))); errno != 0 {
		return fmt.Errorf("failed to set system registers: %v", errno)
	}
	return nil
}

func (c *VCPU) GetRegs(regs *Regs) error {
	if _, _, errno := unix.RawSyscall(unix.SYS_IOCTL,
		c.fd,
		KVM_GET_REGS,
		uintptr(unsafe.Pointer(regs))); errno != 0 {
		return fmt.Errorf("failed to get registers: %v", errno)
	}
	return nil
}

func (c *VCPU) SetRegs(regs *Regs) error {
	if _, _, errno := unix.RawSyscall(unix.SYS_IOCTL,
		c.fd,
		KVM_SET_REGS,
		uintptr(unsafe.Pointer(regs))); errno != 0 {
		return fmt.Errorf("failed to set registers: %v", errno)
	}
	return nil
}

func (c *VCPU) Run() error {
	for {
		_, _, errno := unix.RawSyscall(unix.SYS_IOCTL, c.fd, KVM_RUN, 0)
		if errno == unix.EINTR {
			continue
		}
		if errno != 0 {
			return fmt.Errorf("failed to run vCPU: %v", errno)
		}
		break
	}
	return nil
}

func (c *VCPU) Context() *Run {
	return c.run
}
