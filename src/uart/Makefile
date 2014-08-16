MAKETOPDIR = ../..

TARGET = libuart.a

SRC_SIM :=

include $(MAKETOPDIR)/defaults.mk

ifeq ($(UART_SUPPORT),y)
  SRC = uart.c
  SRC += uart_commands.c
endif

include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
