MAKETOPDIR = ../..

TARGET = libanimations.a

include $(MAKETOPDIR)/defaults.mk

ifeq ($(findstring CYGWIN,$(OSTYPE)),CYGWIN)
	SRC_SIM = winmain.c eeprom.c
else
	SRC_SIM = main.c trackball.c eeprom.c
endif

include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
