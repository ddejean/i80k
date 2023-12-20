// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package main

import "fmt"

type ioDevice interface {
	Read(reg uint16) uint8
	Write(reg uint16, data uint8)
}

type ioMapper struct {
	devices map[uint16]ioDevice
}

func newIOMapper() *ioMapper {
	return &ioMapper{
		devices: make(map[uint16]ioDevice),
	}
}

func (m *ioMapper) Add(base uint16, device ioDevice) error {
	if _, ok := m.devices[base]; ok {
		return fmt.Errorf("device already present at address %x", base)
	}
	m.devices[base] = device
	return nil
}

func (m *ioMapper) Read(port uint16) (uint8, error) {
	dev, reg := m.getDevice(port)
	if dev == nil {
		return 0, fmt.Errorf("failed to obtain device at %x", port)
	}
	return dev.Read(reg), nil
}

func (m *ioMapper) Write(port uint16, data uint8) error {
	dev, reg := m.getDevice(port)
	if dev == nil {
		return fmt.Errorf("failed to obtain device at %x", port)
	}
	dev.Write(reg, data)
	return nil
}

func (m *ioMapper) getDevice(port uint16) (ioDevice, uint16) {
	return m.devices[port&0xFFF8], port & 0x7
}
