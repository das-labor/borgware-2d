MAKETOPDIR = ../..

TARGET = objects

include $(MAKETOPDIR)/defaults.mk

SRC = scrolltext3.c
SRC += $(shell echo $(SCROLLTEXT_FONT) | tr A-Z a-z).c

include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
