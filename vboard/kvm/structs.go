// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package kvm

// UserspaceMemoryRegion mirrors struct kvm_userspace_memory_region.
type UserspaceMemoryRegion struct {
	Slot          uint32
	Flags         uint32
	GuestPhysAddr uint64
	MemorySize    uint64
	UserspaceAddr uint64
}

// Run mirrors struct kvm_run
type Run struct {
	RequestInterruptWindow uint8
	_                      [7]uint8

	ExitReason                 uint32
	ReadyForInterruptInjection uint8
	IfFlag                     uint8
	_                          [2]uint8

	CR8      uint64
	ApicBase uint64

	// This is the union data for exits. Interpretation depends entirely on
	// the exitReason above (see vCPU code for more information).
	Data [32]uint64
}

// IO unmarshals an IO struct from the data contained in struct Run.
func (r *Run) IO() *IO {
	if r.ExitReason != KVM_EXIT_IO || len(r.Data) < 2 {
		return nil
	}
	return &IO{
		Direction:  IODirection(r.Data[0]),
		Size:       uint8(r.Data[0] >> 8),
		Port:       uint16(r.Data[0] >> 16),
		Count:      uint32(r.Data[0] >> 32),
		DataOffset: r.Data[1],
	}
}

// IO mirrors struct kvm_run.io.
type IO struct {
	Direction  IODirection
	Size       uint8
	Port       uint16
	Count      uint32
	DataOffset uint64
}

// Regs mirrors struct kvm_regs.
type Regs struct {
	RAX    uint64
	RBX    uint64
	RCX    uint64
	RDX    uint64
	RSI    uint64
	RDI    uint64
	RSP    uint64
	RBP    uint64
	R8     uint64
	R9     uint64
	R10    uint64
	R11    uint64
	R12    uint64
	R13    uint64
	R14    uint64
	R15    uint64
	RIP    uint64
	RFLAGS uint64
}

// SRegs mirrors struct kvm_sregs.
type SRegs struct {
	CS              Segment
	DS              Segment
	ES              Segment
	FS              Segment
	GS              Segment
	SS              Segment
	TR              Segment
	LDT             Segment
	GDT             DTable
	IDT             DTable
	CR0             uint64
	CR2             uint64
	CR3             uint64
	CR4             uint64
	CR8             uint64
	EFER            uint64
	ApicBase        uint64
	InterruptBitmap [(KVM_NR_INTERRUPTS + 63) / 64]uint64
}

// Segment mirrors struct kvm_segment.
type Segment struct {
	Base     uint64
	Limit    uint32
	Selector uint16
	Typ      uint8
	Present  uint8
	DPL      uint8
	DB       uint8
	S        uint8
	L        uint8
	G        uint8
	AVL      uint8
	Unusable uint8
	_        uint8
}

// Dtable mirrors struct kvm_dtable.
type DTable struct {
	Base  uint64
	Limit uint16
	_     [3]uint16
}
