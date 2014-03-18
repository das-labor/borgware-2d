
ifeq ($(GAME_TETRIS_CORE),y)
  SUBDIRS += $(TOPDIR)/games/tetris
endif

ifeq ($(GAME_SPACE_INVADERS),y)
  SUBDIRS += $(TOPDIR)/games/space_invaders
endif

ifeq ($(GAME_SNAKE),y)
  SUBDIRS += $(TOPDIR)/games/snake
endif

ifeq ($(ANIMATION_SNAKE),y)
  ifneq ($(GAME_SNAKE),y)
    SUBDIRS += $(TOPDIR)/games/snake
  endif
endif

ifeq ($(GAME_BREAKOUT),y)
  SUBDIRS += $(TOPDIR)/games/breakout
endif
