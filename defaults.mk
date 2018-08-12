# ethersex project specific defaults, each can be overridden in config.mk
F_CPU = 20000000UL
MCU = atmega644

CC=avr-gcc
AR=avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AS = avr-as
CP = cp
RM = rm -f
AVRDUDE = avrdude
AVRDUDE_BAUDRATE = 115200
SIZE = avr-size
STRIP = avr-strip

HOSTCC := gcc
export HOSTCC

LIBS = -lm

# flags for the compiler
CFLAGS ?= -Wall -W -Wno-unused-parameter -Wno-sign-compare
CFLAGS += -g -Os -std=gnu99 -fgnu89-inline -D_XOPEN_SOURCE=600 -DNDEBUG

#############################################################################
#Settings for Simulator build

#ugly hack to define a newline (yes, there are two blank lines, on purpose!!)
define n


endef

OSTYPE = $(shell uname)
MACHINE = $(shell uname -m)

ifeq ($(findstring CYGWIN,$(OSTYPE)),CYGWIN)
	CFLAGS_SIM  = -g -Wall -pedantic -std=c99 -O0 -D_WIN32 -D_XOPEN_SOURCE=600
	LDFLAGS_SIM = 
	LIBS_SIM = -lgdi32 -lwinmm -lm
else
	ifeq ($(OSTYPE),FreeBSD)
		CFLAGS_SIM  = -g -I/usr/local/include -Wall -pedantic -std=c99 -O0
		CFLAGS_SIM += -D_XOPEN_SOURCE=600
		LDFLAGS_SIM = -L/usr/local/lib
		LIBS_SIM = -lglut -lpthread -lGL -lGLU -lm
	else
		ifeq ($(OSTYPE),NetBSD)
			CFLAGS_SIM = -g -I /usr/pkg/include -I/usr/X11R7/include -Wall -pedantic -std=c99 -O0
			CFLAGS_SIM += -D_XOPEN_SOURCE=600
			LDFLAGS_SIM = -L/usr/pkg/lib -L/usr/X11R7/lib -Wl,-R/usr/pkg/lib,-R/usr/X11R7/lib
			LIBS_SIM = -lglut -lpthread -lGL -lGLU -lm
		else
			ifeq ($(OSTYPE),OpenBSD)
				CFLAGS_SIM = -g -I/usr/local/include -I/usr/X11R6/include -Wall -pedantic -std=c99 -O0
				CFLAGS_SIM += -D_XOPEN_SOURCE=600
				LDFLAGS_SIM = -L/usr/local/lib -L/usr/X11R6/lib
				LIBS_SIM = -lglut -lpthread -lGL -lGLU -lm
			else
				ifeq ($(OSTYPE),Darwin)
					CFLAGS_SIM = -g -Wall -pedantic -std=c99 -O0 -DOSX_
					LDFLAGS_SIM = -Wall -prebind
					LIBS_SIM = -lpthread -framework Carbon -framework GLUT -framework OpenGL -framework Foundation -framework AppKit
				else
					ifeq ($(OSTYPE),Linux)
						CFLAGS_SIM  = -g -Wall -pedantic -std=c99 -O0 -D_XOPEN_SOURCE=600
						LDFLAGS_SIM = 
						LIBS_SIM = -lglut -lpthread -lGL -lGLU -lm
					else
						($(warning $(n)$(n)Simulator build is not supported on your system.$(n)$(n)\
Currently supported platforms:$(n) \
Linux on i386 and amd64$(n) \
FreeBSD on i386 and amd64$(n) \
NetBSD on i386 and amd64$(n) \
Windows (via Cygwin) on i386 and amd64)
					endif
				endif
			endif
		endif
	endif
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

##############################################################################
# configure load address for bootloader, if enabled
#
ifneq ($(MAKECMDGOALS),clean)
	ifneq ($(MAKECMDGOALS),mrproper)
		ifneq ($(MAKECMDGOALS),menuconfig)  
			include $(MAKETOPDIR)/.config
			CPPFLAGS += -DF_CPU=$(FREQ)UL -mmcu=$(MCU)
			LDFLAGS += -Wl,-Map,image.map -mmcu=$(MCU)
		endif # MAKECMDGOALS!=menuconfig
	endif # MAKECMDGOALS!=mrproper
endif # MAKECMDGOALS!=clean

ifeq ($(BOOTLOADER_SUPPORT),y)	
	LDFLAGS += -Wl,--section-start=.text=0xE000
	CFLAGS  += -mcall-prologues
endif


%.s: %.c
	$(CC) -o $@ -O0 $(CPPFLAGS) -S $<

%.E: %.c
	$(CC) -o $@ -O0 $(CPPFLAGS) -C -E -dD $<

