/*
 * This file is part of the frser-avr project.
 *
 * Copyright (C) 2009,2015 Urja Rannikko <urjaman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "main.h"
#include "flash.h"
#include "uart.h"

static uint8_t flash_databus_read(void) {
	uint8_t rv;
	rv = (PIND & 0xFC);
	rv |= (PINB & 0x03);
	return rv;
}

static void flash_databus_tristate(void) {
	DDRB &= ~(_BV(0) | _BV(1));
	DDRD &= ~(_BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6) | _BV(7));
	PORTB &= ~(_BV(0) | _BV(1));
	PORTD &= ~(_BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6) | _BV(7));
}

static void flash_databus_output(unsigned char data) {
	PORTB = ((PORTB & 0xFC) | (data & 0x03));
	PORTD = ((PORTD & 0x03) | (data & 0xFC));
	DDRB |= (_BV(0) | _BV(1));
	DDRD |= (_BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6) | _BV(7));
}

static void flash_chip_enable(void) {
	PORTC &= ~_BV(3);
}

void flash_init(void) {
	PORTC |= _BV(2);
	DDRC |= (_BV(2) | _BV(1) | _BV(0));
	PORTB &= ~_BV(2);
	DDRB |= (_BV(4) | _BV(2));
	// ADDR unit init done
	PORTC |= (_BV(3) | _BV(4) | _BV(5));
	DDRC |= (_BV(3) | _BV(4) | _BV(5));
	// control bus init done
	// hmm, i should probably tristate the data bus by default...
	flash_databus_tristate();
	// CE control is not absolutely necessary...
	flash_chip_enable();
}

// 'push' 3 bits of addr
static void push_addr_bits(unsigned char bits) {
/*	if (bits&4) PORTB |= _BV(4);
	else PORTB &= ~(_BV(4));
	if (bits&1) PORTC |= _BV(1);
	else PORTC &= ~(_BV(1));
	if (bits&2) PORTC |= _BV(0);
	else PORTC &= ~(_BV(0));*/
	// SET THE BITS: (functionality near as-above)
	asm volatile(
	"sbi %0, 4\n\t"
	"sbi %1, 1\n\t"
	"sbi %1, 0\n\t"
	"sbrs %2, 2\n\t"
	"cbi %0, 4\n\t"
	"sbrs %2, 0\n\t"
	"cbi %1, 1\n\t"
	"sbrs %2, 1\n\t"
	"cbi %1, 0\n\t"
	::
	"I" (_SFR_IO_ADDR(PORTB)),
	"I" (_SFR_IO_ADDR(PORTC)),
	"r" (bits)
	);
	// double-toggle CP
	asm volatile(
	"sbi %0, 2\n\t"
	"sbi %0, 2\n\t"
	::
	"I" (_SFR_IO_ADDR(PINB))
	);
}


static void flash_output_enable(void) {
	PORTC &= ~_BV(4);
}

static void flash_output_disable(void) {
	PORTC |= _BV(4);
}

static void flash_setaddr(uint32_t addr) {
	uint8_t i,n,d;
	// Currently uses 18-bit addresses
	for (i=6;i>0;i--) { // as i isn't really used here, this way generates faster & smaller code
		asm volatile(
		"mov %0, %C1\n\t"
		"lsl %0 \n\t"
		"bst %B1, 7\n\t"
		"bld %0, 0\n\t"
		: "=r" (n)
		: "r" (addr)
		);
//		push_addr_bits((addr>>15)&0x07);
		push_addr_bits(n); // ABOVE REPLACED BY ASM
		d = 3;
		asm volatile(
		"lsl %A0\n\t"
		"rol %B0\n\t"
		"rol %C0\n\t"
		"dec %1\n\t"
		"brne .-10\n\t"
		: "+r" (addr), "+r" (d)
		:
		);
		// addr = (addr<<3); // Done as 24-bit op in above asm + d=3
	}
}

static void flash_pulse_we(void) {
	asm volatile(
	"nop\n\t"
	"sbi %0, 5\n\t"
	"nop\n\t"
	"sbi %0, 5\n\t"
	::
	"I" (_SFR_IO_ADDR(PINC))
	);
}

static void flash_read_init(void) {
	flash_databus_tristate();
	flash_output_enable();
}


// assume chip enabled & output enabled & databus tristate
static uint8_t flash_readcycle(uint32_t addr) {
	flash_setaddr(addr);
	asm volatile(
	"nop\n\t"
	"nop\n\t"
	:: ); // 250 ns @ 8 mhz // assembler inspection shows that these shouldn't be necessary
	return flash_databus_read();
}

// assume only CE, and perform single cycle
uint8_t flash_read(uint32_t addr) {
	uint8_t data;
	flash_read_init();
	flash_setaddr(addr);
	data = flash_databus_read();
	flash_output_disable();
	return data;
}

// assume only CE, perform single cycle
void flash_write(uint32_t addr, uint8_t data) {
	flash_output_disable();
	flash_databus_output(data);
	flash_setaddr(addr);
	flash_pulse_we();
}

void flash_readn(uint32_t addr, uint32_t len) {
	flash_read_init();
	do {
		SEND(flash_readcycle(addr++));
	} while(--len);
	// safety features
	flash_output_disable();
}


void flash_select_protocol(uint8_t allowed_protocols) {
	(void)allowed_protocols;
	flash_init();
}
