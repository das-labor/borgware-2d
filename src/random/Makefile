MAKETOPDIR = ../..

TARGET = objects_avr

include $(MAKETOPDIR)/defaults.mk

#for AVR
SRC  = prng.c persistentCounter.c
ASRC = noekeon_asm.S memxor.S

#for simulator
SRC_SIM  = prng.c persistentCounter.c noekeon.c memxor_c.c

include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
