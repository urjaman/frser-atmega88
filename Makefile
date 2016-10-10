##
## This file is part of the frser-avr project.
##
## Copyright (C) 2009 Urja Rannikko <urjaman@gmail.com>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
##

PROJECT=frser-avr
DEPS=uart.h main.h Makefile
SOURCES=main.c uart.c flash.c
CC=avr-gcc
OBJCOPY=avr-objcopy
MMCU=atmega328p

BAUD ?= 115200
F_CPU ?= 18432000

#Device
SERIAL_DEV ?= /dev/ttyACM0

#Tools directory
AVRTOOLDIR ?= /usr/avr/

AVRBINDIR=$(AVRTOOLDIR)/bin/
AVRETCDIR=$(AVRTOOLDIR)/etc/
AVRDUDECMD=avrdude -C $(AVRETCDIR)avrdude.conf -p m328p -c arduino -P $(SERIAL_DEV) -b $(BAUD)
CFLAGS=-mmcu=$(MMCU) -Os -Wl,--relax -fno-inline-small-functions -fno-tree-scev-cprop -frename-registers -g -Wall -W -pipe -DRAMSTART=0x100 -DBAUD=$(BAUD) -DF_CPU=$(F_CPU)

include libfrser/Makefile.frser

all: $(PROJECT).out
	$(AVRBINDIR)avr-size $(PROJECT).out
 
$(PROJECT).hex: $(PROJECT).out
	$(AVRBINDIR)$(OBJCOPY) -j .text -j .data -O ihex $(PROJECT).out $(PROJECT).hex
 
$(PROJECT).out: $(SOURCES) $(DEPS)
	$(AVRBINDIR)$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)

asm: $(SOURCES) $(DEPS)
	$(AVRBINDIR)$(CC) $(CFLAGS) -S  -I./ -o $(PROJECT).s $(SOURCES)
 
program: $(PROJECT).hex
	$(AVRBINDIR)$(AVRDUDECMD) -U flash:w:$(PROJECT).hex

objdump: $(PROJECT).out
	$(AVRBINDIR)avr-objdump -xdC $(PROJECT).out | less

clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex
	rm -f $(PROJECT).s

backup:
	$(AVRBINDIR)$(AVRDUDECMD) -U flash:r:backup.bin:r
