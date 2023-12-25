// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

package devices

type P8254 struct{}

func NewP8254() *P8254 {
	return &P8254{}
}

func (u *P8254) Start() {
}

func (u *P8254) Stop() {
}

func (u *P8254) Read(reg uint16) uint8 {
	return 0
}

func (u *P8254) Write(reg uint16, data uint8) {
}
