# Makefile to create .ihx files
# $Id$

CC = sdcc

# generic rules

%.rel: %.c
	$(CC) -c $<

%.ihx: %.rel
	$(CC) $<

# default target

default: ../ihx/firm_test.ihx

../ihx/firm_test.ihx: firm_test.ihx ../ihx
	cp firm_test.ihx ../ihx

../ihx:
	mkdir ../ihx

clean:
	$(RM) *.asm *.lnk *.map *.rst *.sym *.rel *.lst *.ihx
