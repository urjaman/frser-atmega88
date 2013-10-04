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

unsigned char flash_databus_read(void);
void flash_databus_tristate(void);
void flash_databus_output(unsigned char data);
void flash_init(void);
void push_addr_bits(unsigned char bits);
void flash_chip_enable(void);
void flash_chip_disable(void);
void flash_output_enable(void);
void flash_output_disable(void);
void flash_setaddr(unsigned long int addr);
unsigned char flash_readcycle(unsigned long int addr);
void flash_read_init(void);
void flash_write_init(void);
unsigned char flash_readcycle_single(unsigned long int addr);
void flash_writecycle(unsigned long int addr, unsigned char data);
