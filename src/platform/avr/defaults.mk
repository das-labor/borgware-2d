# ethersex project specific defaults, each can be overridden in config.mk
F_CPU = 20000000UL
MCU = atmega644

CC=avr-gcc
AR=avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AS = avr-as

AVRDUDE = avrdude
AVRDUDE_BAUDRATE = 115200
SIZE = avr-size
STRIP = avr-strip

##############################################################################
# configure load address for bootloader, if enabled
#
ifneq ($(MAKECMDGOALS),clean)
	ifneq ($(MAKECMDGOALS),mrproper)
		ifneq ($(MAKECMDGOALS),menuconfig)
			ifneq ($(MCU),wasm)
				CPPFLAGS += -DF_CPU=$(FREQ)UL -mmcu=$(MCU)
				LDFLAGS += -Wl,-Map,image.map -mmcu=$(MCU)
			endif
		endif # MAKECMDGOALS!=menuconfig
	endif # MAKECMDGOALS!=mrproper
endif # MAKECMDGOALS!=clean

ifeq ($(BOOTLOADER_SUPPORT),y)	
	LDFLAGS += -Wl,--section-start=.text=0xE000
	CFLAGS  += -mcall-prologues
endif