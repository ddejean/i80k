// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package devices

import (
	"context"
	"log"
	"os"
	"sync"
	"time"
)

const (
	// Size of the RX and TX queues.
	QUEUE_SZ = 16
	// UART clock.
	CLOCK = int64(4915200)
)

type P16550 struct {
	// Mutex protecting this struct.
	mu sync.Mutex

	// IRQ queue.
	irqChannel chan uint8
	// Set the IRQ line level.
	setIRQLine func(uint32)
	// RX fifo overrun.
	rxOverrun bool
	// Last received char timestamp.
	lastTs time.Time

	// Receive buffer.
	rx_buf chan uint8
	// Transmit buffer.
	tx_buf chan uint8
	// Interrupt Enable Register
	ier interruptEnableRegister
	// FIFO Control Register
	fcr fifoControlRegister
	// Line Control Register
	lcr lineControlRegister
	// Modem Control Register
	mcr uint8
	// Modem Status Register
	msr uint8
	// Divisor Latch LSB
	dlsb uint8
	// Divisor Latch MSB
	dmsb uint8
	// Cancel function used to stop the Loop.
	cancel context.CancelFunc
}

func NewP16550(setIRQLine func(uint32)) *P16550 {
	// Initialize the UART with the reset configuration documented in the
	// datasheet.
	return &P16550{
		irqChannel: make(chan uint8),
		setIRQLine: setIRQLine,
		rx_buf:     make(chan uint8, QUEUE_SZ),
		tx_buf:     make(chan uint8, QUEUE_SZ),
	}
}

func (u *P16550) Start() {
	ctx, cancel := context.WithCancel(context.Background())
	go u.loop(ctx)
	u.cancel = cancel
}

func (u *P16550) Stop() {
	u.cancel()
}

func (u *P16550) Read(reg uint16) uint8 {
	u.mu.Lock()
	defer u.mu.Unlock()

	switch reg {
	case RD_RX_BUF:
		if u.lcr.hasDLAB() {
			return u.dlsb
		}
		if len(u.rx_buf) > 0 {
			char := <-u.rx_buf
			return char
		}
	case RD_IER:
		if u.lcr.hasDLAB() {
			return u.dmsb
		}
		return uint8(u.ier)
	case RD_IIR:
		return u.interruptIdentificationRegister()
	case RD_LCR:
		return uint8(u.lcr)
	case RD_MCR:
		return u.mcr
	case RD_LSR:
		return u.lineStatusRegister()
	case RD_MSR:
		return u.msr
	}
	return 0
}

func (u *P16550) Write(reg uint16, data uint8) {
	u.mu.Lock()
	defer u.mu.Unlock()

	switch reg {
	case WR_TX_BUF:
		// If DLAB bit is set, we're writing the divider LSB.
		if u.lcr.hasDLAB() {
			u.dlsb = data
			return
		}
		if len(u.tx_buf) < cap(u.tx_buf) {
			u.tx_buf <- data
		}
	case WR_IER:
		if u.lcr.hasDLAB() {
			u.dmsb = data
			return
		}
		u.ier = interruptEnableRegister(data)
	case WR_FCR:
		u.fcr = fifoControlRegister(data)
	case WR_LCR:
		u.lcr = lineControlRegister(data)
	case WR_MCR:
		u.mcr = data
	case WR_MSR:
		u.msr = data
	}
}

func (u *P16550) lineStatusRegister() uint8 {
	if len(u.rx_buf) > 0 {
		return LSR_DATA_READY
	}
	if len(u.tx_buf) == 0 {
		return LSR_TX_EMPTY | LSR_TX_REG_EMPTY
	}
	if u.rxOverrun {
		u.rxOverrun = false
		return LSR_OVERRUN_ERROR
	}

	return uint8(0)
}

func (u *P16550) interruptIdentificationRegister() uint8 {
	var r uint8 = 0

	// When FIFO is enabled, the two most significant bits of the register are
	// always set.
	if u.fcr.fifoEnabled() {
		r |= IIR_FIFO_ENABLED
	}

	if u.ier.rxDataAvailable() && len(u.rx_buf) >= u.fcr.rxTriggerLevel() {
		return r | IIR_RX_DATA
	}

	if u.ier.rxLineStatus() && len(u.rx_buf) > 0 && time.Since(u.lastTs) > u.getTriggerDuration() {
		return r | IIR_FIFO_TIMEOUT
	}

	if u.rxOverrun {
		return r | IIR_LINE_STATUS
	}

	if u.ier.txEmpty() && len(u.tx_buf) == 0 {
		return r | IIR_TX_EMPTY
	}

	return IIR_NO_INTERRUPT
}

func (u *P16550) getBaudDelay() time.Duration {
	counter := int64(u.dmsb)<<8 | int64(u.dlsb)
	baudRate := CLOCK / (16 * counter)
	return time.Second / time.Duration(baudRate)
}

func (u *P16550) getTriggerDuration() time.Duration {
	return time.Duration(u.fcr.rxTriggerLevel()) * u.getBaudDelay()
}

func (u *P16550) loop(ctx context.Context) {
	ticker := time.NewTicker(1 * time.Millisecond)
	input := make(chan uint8, QUEUE_SZ)
	go u.irqManager(ctx)
	go u.readInput(input)

	for {
		select {
		case char := <-u.tx_buf:
			if _, err := os.Stdout.Write([]byte{char}); err != nil {
				log.Fatal("failed to write to stdout: ", err)
			}
		case char := <-input:
			u.mu.Lock()
			if len(u.rx_buf) < cap(u.rx_buf) {
				u.rx_buf <- char
				u.lastTs = time.Now()
			} else {
				u.rxOverrun = true
			}
			u.mu.Unlock()
		case <-ticker.C:
			// Nothing to do
		case <-ctx.Done():
			return
		}

		u.mu.Lock()
		intr := u.interruptIdentificationRegister()
		u.mu.Unlock()
		u.irqChannel <- intr
	}
}

func (u *P16550) irqManager(ctx context.Context) {
	var lastIrq uint8 = IIR_NO_INTERRUPT

	for {
		select {
		case intr := <-u.irqChannel:
			if intr != lastIrq {
				u.setIRQLine(0)
				if intr != IIR_NO_INTERRUPT {
					u.setIRQLine(1)
				}
			}
			lastIrq = intr
		case <-ctx.Done():
			return
		}
	}
}

func (u *P16550) readInput(c chan uint8) {
	b := make([]byte, 1)

	for {
		if _, err := os.Stdin.Read(b); err != nil {
			log.Println(err)
			return
		}
		c <- uint8(b[0])
	}
}
