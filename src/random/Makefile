MAKETOPDIR = ../..

TARGET = objects_avr

include $(MAKETOPDIR)/defaults.mk

#for AVR
ifeq ($(findstring atmega256,$(MCU)),atmega256)
	# handmade assembler routines don't work on ATmega2560
	SRC  = prng.c persistentCounter.c noekeon.c memxor_c.c  
else
	SRC  = prng.c persistentCounter.c
	ASRC = noekeon_asm.S memxor.S
endif

#for simulator
SRC_SIM  = prng.c persistentCounter.c noekeon.c memxor_c.c

include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
