CP = cp
RM = rm -f

HOSTCC := gcc
export HOSTCC

LIBS = -lm

# flags for the compiler
CFLAGS ?= -Wall -W -Wno-unused-parameter -Wno-sign-compare
CFLAGS += -g -Os -std=gnu99 -fgnu89-inline -D_XOPEN_SOURCE=600 -DNDEBUG

ifneq ($(PLATFORM),)
	include $(MAKETOPDIR)/src/platform/$(PLATFORM)/defaults.mk
endif

##############################################################################
# the default target
$(TARGET):

##############################################################################
# include user's config.mk file

$(MAKETOPDIR)/config.mk:
	@echo "# Customize your config here!" >$@
	@echo "# Add further CFLAGS by using the += operator, eg." >>$@
	@echo "# CFLAGS += -mstrict-X" >>$@
	@echo "#" >>$@
	@echo "# In case you wonder: -mstrict-X produces smaller code, but" >>$@
	@echo "# is only available on avr-gcc 4.7.0 or higher." >>$@
	@echo "#" >>$@
	@echo "# Other flags you might want to tune: CPPFLAGS, LDFLAGS ..." >>$@
	@echo "Created default config.mk, tune your settings there!"

-include $(MAKETOPDIR)/config.mk

ifneq ($(MAKECMDGOALS),clean)
	ifneq ($(MAKECMDGOALS),mrproper)
		ifneq ($(MAKECMDGOALS),menuconfig)  
			include $(MAKETOPDIR)/.config
		endif # MAKECMDGOALS!=menuconfig
	endif # MAKECMDGOALS!=mrproper
endif # MAKECMDGOALS!=clean

%.s: %.c
	$(CC) -o $@ -O0 $(CPPFLAGS) -S $<

%.E: %.c
	$(CC) -o $@ -O0 $(CPPFLAGS) -C -E -dD $<

