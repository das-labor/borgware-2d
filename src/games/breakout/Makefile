MAKETOPDIR = ../../..

TARGET =

include $(MAKETOPDIR)/defaults.mk

ifeq ($(GAME_BREAKOUT),y)
  SRC = breakout.c playfield.c rebound.c score.c level.c ball.c messages.c
endif

include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
