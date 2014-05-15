MAKETOPDIR = ../..

TARGET =

include $(MAKETOPDIR)/defaults.mk

SRC_SIM =

ifeq ($(NES_PAD_SUPPORT), y)
  SRC = nes_pad.c
endif
ifeq ($(PARALLEL_JOYSTICK_SUPPORT), y)
  SRC  = joystick.c
endif
ifeq ($(RFM12_JOYSTICK_SUPPORT), y)
  SRC  = rfm12_joystick.c
endif
ifeq ($(HC165_JOYSTICK_SUPPORT), y)
  SRC  = hc165_joystick.c
endif
ifeq ($(NULL_JOYSTICK_SUPPORT), y)
  SRC  = null_joystick.c
endif

include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
