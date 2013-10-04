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
DEPS=uart.h frser.h udelay.h main.h
SOURCES=main.c uart.c flash.c udelay.c frser.c
CC=avr-gcc
OBJCOPY=avr-objcopy
MMCU=atmega168
AVRBINDIR=~/avr-tools/bin
AVRDUDECMD=avrdude -p m168 -c dt006 -E noreset
CFLAGS=-mmcu=$(MMCU) -Os -mcall-prologues -fno-inline-small-functions -fno-tree-scev-cprop -frename-registers -g -Werror -Wall -W -pipe -combine -fwhole-program
 
$(PROJECT).hex: $(PROJECT).out
	$(AVRBINDIR)/$(OBJCOPY) -j .text -j .data -O ihex $(PROJECT).out $(PROJECT).hex
 
$(PROJECT).out: $(SOURCES) $(DEPS)
	$(AVRBINDIR)/$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)

asm: $(SOURCES) $(DEPS)
	$(AVRBINDIR)/$(CC) $(CFLAGS) -S  -I./ -o $(PROJECT).s $(SOURCES)
 
program: $(PROJECT).hex
	$(AVRBINDIR)/$(AVRDUDECMD) -U flash:w:$(PROJECT).hex

clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex
	rm -f $(PROJECT).s

backup:
	$(AVRBINDIR)/$(AVRDUDECMD) -U flash:r:backup.bin:r
