// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package kvm

const KVM_API_VERSION = 12

// KVM ioctls.
const (
	KVM_GET_API_VERSION    = 0xae00
	KVM_CREATE_VM          = 0xae01
	KVM_GET_VCPU_MMAP_SIZE = 0xae04
)

// KVM ioctls values for VM fds.
const (
	KVM_CREATE_VCPU            = 0xae41
	KVM_SET_USER_MEMORY_REGION = 0x4020ae46
)

// KVM ioctls values for vCPU fds.
const (
	KVM_RUN       = 0xae80
	KVM_GET_REGS  = 0x8090ae81
	KVM_SET_REGS  = 0x4090ae82
	KVM_GET_SREGS = 0x8138ae83
	KVM_SET_SREGS = 0x4138ae84
)

// KVM limits.
const (
	KVM_NR_MEMSLOTS      = 0x100
	KVM_NR_VCPUS         = 0xff
	KVM_NR_INTERRUPTS    = 0x100
	KVM_NR_CPUID_ENTRIES = 0x100
)

// KVM exit reasons.
const (
	KVM_EXIT_UNKNOWN         = 0
	KVM_EXIT_EXCEPTION       = 1
	KVM_EXIT_IO              = 2
	KVM_EXIT_HYPERCALL       = 3
	KVM_EXIT_DEBUG           = 4
	KVM_EXIT_HLT             = 5
	KVM_EXIT_MMIO            = 6
	KVM_EXIT_IRQ_WINDOW_OPEN = 7
	KVM_EXIT_SHUTDOWN        = 8
	KVM_EXIT_FAIL_ENTRY      = 9
	KVM_EXIT_INTR            = 10
	KVM_EXIT_SET_TPR         = 11
	KVM_EXIT_TPR_ACCESS      = 12
	KVM_EXIT_S390_SIEIC      = 13
	KVM_EXIT_S390_RESET      = 14
	KVM_EXIT_DCR             = 15
	KVM_EXIT_NMI             = 16
	KVM_EXIT_INTERNAL_ERROR  = 17
)
