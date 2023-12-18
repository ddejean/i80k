// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package kvm

import (
	"fmt"
	"os"

	"golang.org/x/sys/unix"
)

var (
	version    int
	mapRunSize uintptr
)

func OpenDevice(devicePath string) (*os.File, error) {
	if devicePath == "" {
		return nil, fmt.Errorf("invalid device path %q", devicePath)
	}
	f, err := os.OpenFile(devicePath, unix.O_RDWR, 0)
	if err != nil {
		return nil, fmt.Errorf("failed to open %q: %w", devicePath, err)
	}

	apiVersion, _, errno := unix.Syscall(unix.SYS_IOCTL, f.Fd(), KVM_GET_API_VERSION, 0)
	if errno != 0 {
		return nil, fmt.Errorf("failed to get KVM API version: %v", errno)
	}

	if apiVersion != KVM_API_VERSION {
		return nil, fmt.Errorf("KVM API version does not match: got %d expecting %d", apiVersion, KVM_API_VERSION)
	}
	version = int(apiVersion)

	mapRunSize, _, errno = unix.Syscall(unix.SYS_IOCTL, f.Fd(), KVM_GET_VCPU_MMAP_SIZE, 0)
	if errno != 0 {
		return nil, fmt.Errorf("failed to obtain vCPU map run size: %v", errno)
	}

	return f, nil
}

func GetApiVersion() int {
	return version
}
