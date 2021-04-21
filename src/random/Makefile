MAKETOPDIR = ../..

TARGET = objects_avr

include $(MAKETOPDIR)/defaults.mk

ifeq ($(RANDOM_IMPLEMENTATION),lfsr)
	SRC = prng_lfsr.c persistentCounter.c
endif

ifeq ($(RANDOM_IMPLEMENTATION),noekeon)
	ifeq ($(PLATFORM),avr)
		#for AVR
		ifeq ($(findstring atmega256,$(MCU)),atmega256)
			# handmade assembler routines don't work on ATmega2560
			SRC  = prng_noekeon.c persistentCounter.c noekeon.c memxor_c.c
		else
			SRC  = prng_noekeon.c persistentCounter.c
			ASRC = noekeon_asm.S memxor.S
		endif
	else
		SRC  = prng_noekeon.c persistentCounter.c noekeon.c memxor_c.c
	endif
endif

include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
