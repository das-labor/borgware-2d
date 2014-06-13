MAKETOPDIR = ../../..

include $(MAKETOPDIR)/defaults.mk

ifeq ($(ANIMATION_BMSCROLLER),y)
  SRC = bitmapscroller.c
endif

ifeq ($(ANIMATION_LABORLOGO),y)
  SRC += laborlogo.c
endif

ifeq ($(ANIMATION_AMPHIBIAN),y)
  SRC += amphibian.c
endif

ifeq ($(ANIMATION_LOGO_OOS),y)
  SRC += outofspec.c
endif

ifeq ($(ANIMATION_FAIRYDUST),y)
  SRC += fairydust.c
endif

ifeq ($(ANIMATION_THISISNOTDETROIT),y)
  SRC += thisisnotdetroit.c
endif

include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
