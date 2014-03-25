MAKETOPDIR = ../..

TARGET = libcan.a

include $(MAKETOPDIR)/defaults.mk

ifeq ($(CAN_SUPPORT),y)
  SRC = can.c
  SRC += borg_can.c
  SRC += lap.c
  SRC += spi.c
endif

include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
