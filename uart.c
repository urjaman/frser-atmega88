/*
 * This file is part of the frser-avr project.
 *
 * Copyright (C) 2009 Urja Rannikko <urjaman@gmail.com>
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
#include "uart.h"

// UART MODULE START
typedef unsigned char urxbufoff_t;
typedef unsigned char utxbufoff_t;
unsigned char volatile uart_rcvbuf[UART_BUFLEN];
urxbufoff_t volatile uart_rcvwptr;
urxbufoff_t volatile uart_rcvrptr;

#define UARTTX_BUFLEN 16
unsigned char volatile uart_sndbuf[UARTTX_BUFLEN];
utxbufoff_t volatile uart_sndwptr;
utxbufoff_t volatile uart_sndrptr;

ISR(USART_RX_vect) {
	urxbufoff_t reg = uart_rcvwptr;
	uart_rcvbuf[reg++] = UDR0;
	if(reg==UART_BUFLEN) reg = 0;	
	if(reg==uart_rcvrptr) {
		if (reg) reg--;
		else reg = UART_BUFLEN-1;
	}
	uart_rcvwptr = reg;
}


ISR(USART_UDRE_vect) {
	utxbufoff_t reg = uart_sndrptr;
	if (uart_sndwptr != reg) {
		UDR0 = uart_sndbuf[reg++];
		if (reg==UARTTX_BUFLEN) reg = 0;
		uart_sndrptr = reg;
		return;
	} else {
		UCSR0B &= ~_BV(5);
		return;
	}
}
	
	
unsigned char uart_isdata(void) {
	cli();
	if (uart_rcvwptr != uart_rcvrptr) { sei(); return 1; }
	else { sei(); return 0; }
	}

unsigned char uart_recv(void) {
	urxbufoff_t reg;
	unsigned char val;
	while (!uart_isdata()) sleep_mode(); // when there's nothing to do, one might idle...
	cli();
	reg = uart_rcvrptr;
	val = uart_rcvbuf[reg++];
	if(reg==UART_BUFLEN) reg = 0;
	uart_rcvrptr = reg;
	sei();
	return val;
	}

void uart_send(unsigned char val) {
	utxbufoff_t reg;
	while (uart_sndwptr+1==uart_sndrptr || (uart_sndwptr+1==UARTTX_BUFLEN && !uart_sndrptr)); // wait for space in buf
	cli();
	reg = uart_sndwptr;
	UCSR0B |= _BV(5); // make sure the transmit int is on
	uart_sndbuf[reg++] = val; // add byte to the transmit queue
	if(reg==UARTTX_BUFLEN) reg = 0;
	uart_sndwptr = reg;
	sei();
	}

void uart_init(void) {
	cli();

#include <util/setbaud.h>
// Assuming uart.h defines BAUD
	uart_rcvwptr = 0;
	uart_rcvrptr = 0;
	uart_sndwptr = 0;
	uart_sndrptr = 0;
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = 0x06; // Async USART,No Parity,1 stop bit, 8 bit chars
	UCSR0A &= 0xFC;
#if USE_2X
	UCSR0A |= (1 << U2X);
#endif
	UCSR0B = 0xB8; // RX complete interrupt enable, UDRE int en, Receiver & Transmitter enable
	sei();
	}
	


void uart_wait_txdone(void) {
	while (uart_sndwptr != uart_sndrptr);
}
