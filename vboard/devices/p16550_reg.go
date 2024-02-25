// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package devices

// Read register indexes.
const (
	RD_RX_BUF uint16 = iota
	RD_IER           = 1
	RD_IIR           = 2
	RD_LCR           = 3
	RD_MCR           = 4
	RD_LSR           = 5
	RD_MSR           = 6
)

// Write register indexes.
const (
	WR_TX_BUF uint16 = iota
	WR_IER           = 1
	WR_FCR           = 2
	WR_LCR           = 3
	WR_MCR           = 4
	WR_MSR           = 6
)

// FIFO Control Register bit flags.
const (
	FCR_FIFO_ENABLED uint8 = 1
	FCR_RX_CLEAR           = (1 << 1)
	FCR_TX_CLEAR           = (1 << 2)
	FCR_FIFO_TRIGGER       = (3 << 6)
)

type fifoControlRegister uint8

func (r fifoControlRegister) fifoEnabled() bool {
	return (uint8(r) & FCR_FIFO_ENABLED) == FCR_FIFO_ENABLED
}

func (r fifoControlRegister) rxClear() bool {
	return (uint8(r) & FCR_RX_CLEAR) == FCR_RX_CLEAR
}

func (r fifoControlRegister) txClear() bool {
	return (uint8(r) & FCR_TX_CLEAR) == FCR_TX_CLEAR
}

func (r fifoControlRegister) rxTriggerLevel() int {
	level := (uint8(r) & FCR_FIFO_TRIGGER) >> 6
	switch level {
	case 0:
		// 0 means 1 byte trigger.
		return 1
	case 1:
		// 1 means 4 bytes trigger.
		return 4
	case 2:
		// 2 means 8 bytes trigger.
		return 8
	case 3:
		// 3 means 14 bytes trigger.
		return 14
	}
	// In case the value is not valid, return the minimal trigger size.
	return 1
}

// Line Control Register bit flags.
const (
	LCR_DLAB uint8 = 1 << 7
)

type lineControlRegister uint8

func (r lineControlRegister) hasDLAB() bool {
	return (uint8(r) & LCR_DLAB) == LCR_DLAB
}

// Interrupt Enable Register bit flags
const (
	IER_RX_DATA_AVAILABLE uint8 = 1
	IER_TX_EMPTY                = (1 << 1)
	IER_RX_LINE_STATUS          = (1 << 2)
	IER_MODEM_STATUS            = (1 << 3)
)

type interruptEnableRegister uint8

func (r interruptEnableRegister) rxDataAvailable() bool {
	return (uint8(r) & IER_RX_DATA_AVAILABLE) == IER_RX_DATA_AVAILABLE
}

func (r interruptEnableRegister) rxLineStatus() bool {
	return (uint8(r) & IER_RX_LINE_STATUS) == IER_RX_LINE_STATUS
}

func (r interruptEnableRegister) txEmpty() bool {
	return (uint8(r) & IER_TX_EMPTY) == IER_TX_EMPTY
}

// Line Status Register bit flags.
const (
	LSR_DATA_READY    = 1
	LSR_OVERRUN_ERROR = (1 << 1)
	LSR_TX_REG_EMPTY  = (1 << 5)
	LSR_TX_EMPTY      = (1 << 6)
)

// Interrupt Identification Register bit flags.
const (
	IIR_NO_INTERRUPT = 0x1
	IIR_TX_EMPTY     = (1 << 1)
	IIR_RX_DATA      = (2 << 1)
	IIR_LINE_STATUS  = (3 << 1)
	IIR_FIFO_TIMEOUT = (6 << 1)
	IIR_FIFO_ENABLED = (3 << 6)
)
