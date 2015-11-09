# This file eases the integration of your personal animations which are not
# meant to be included in the official Borgware-2D repository.
#
# Just add your own source files to the SRC variable (checking for options
# introduced in your config.in file) as shown in the commented samples.

MAKETOPDIR = ../..

TARGET = libuser.a

include $(MAKETOPDIR)/defaults.mk


# ifeq ($(ANIMATION_MY_SIMPLE_ANIM),y)
#   SRC += my_simple_anim.c
# endif

# ifeq ($(ANIMATION_MY_COMPLEX_ANIM),y)
#   SRC += my_complex_anim.c
# endif


include $(MAKETOPDIR)/rules.mk

include $(MAKETOPDIR)/depend.mk
