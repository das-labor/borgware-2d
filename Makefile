TARGET     := image
TARGET_SIM := borgsim
TOPDIR = src
MAKETOPDIR = .

SRC = \
	$(TOPDIR)/main.c            \
	$(TOPDIR)/display_loop.c    \
	$(TOPDIR)/eeprom_reserve.c  \
	$(TOPDIR)/pixel.c           \
	$(TOPDIR)/util.c            \

SRC_SIM = \
	$(TOPDIR)/display_loop.c    \
	$(TOPDIR)/pixel.c           \


LAUNCH_BOOTLOADER = launch-bootloader
#SERIAL = /dev/ttyUSB0
SERIAL = COM6
export TOPDIR
##############################################################################

all: compile-$(TARGET)
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
	@echo "size is: "
	@$(CONFIG_SHELL) scripts/size $(TARGET)
	@echo "==============================="

##############################################################################
# generic fluff
include $(MAKETOPDIR)/defaults.mk
#include $(MAKETOPDIR)/rules.mk

##############################################################################
# generate SUBDIRS variable
#

.subdirs: $(TOPDIR)/autoconf.h
	@ echo "checking in which subdirs to build"
	@ $(RM) -f $@
	@ echo "SUBDIRS += $(TOPDIR)/animations" >> $@
	@ echo "SUBDIRS += $(TOPDIR)/animations/bitmapscroller" >> $@
	@ echo "SUBDIRS += $(TOPDIR)/smallani" >> $@
	@ (for subdir in `grep -e "^#define .*_SUPPORT" $(TOPDIR)/autoconf.h \
	      | sed -e "s/^#define /$(TOPDIR)\//" -e "s/_SUPPORT.*//" \
	      | tr "[A-Z]\\n" "[a-z] " `; do \
	  test -d $$subdir && echo "SUBDIRS += $$subdir" ; \
	done) | sort -u >> $@

ifneq ($(no_deps),t)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),mrproper)
ifneq ($(MAKECMDGOALS),menuconfig)

include $(MAKETOPDIR)/.subdirs
include $(MAKETOPDIR)/.config
include $(TOPDIR)/games/games.mk

endif # MAKECMDGOALS!=menuconfig
endif # MAKECMDGOALS!=mrproper
endif # MAKECMDGOALS!=clean
endif # no_deps!=t


##############################################################################

SUBDIRS_AVR = $(TOPDIR)/borg_hw
SUBDIRS_AVR += $(SUBDIRS)

.PHONY: compile-subdirs_avr
compile-subdirs_avr:
	@ for dir in $(SUBDIRS_AVR); do $(MAKE) -C $$dir objects_avr || exit 5; done

.PHONY: compile-$(TARGET)
compile-$(TARGET): compile-subdirs_avr $(TARGET).hex $(TARGET).bin $(TARGET).lst



OBJECTS += $(patsubst $(TOPDIR)/%.c,$(TOPDIR)/obj_avr/%.o,${SRC})
SUBDIROBJECTS = $(foreach subdir,$(SUBDIRS_AVR),$(foreach object,$(shell cat $(subdir)/obj_avr/.objects 2>/dev/null),$(subdir)/$(object)))

$(TARGET): $(OBJECTS) $(SUBDIROBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(SUBDIROBJECTS) $(LIBS)


##############################################################################
#generic rules for AVR-Build
$(TOPDIR)/obj_avr/%.o: $(TOPDIR)/%.c
	@ if [ ! -d $(TOPDIR)/obj_avr ]; then mkdir $(TOPDIR)/obj_avr ; fi
	@ echo "compiling $<"
	@ $(CC) -o $@ $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c $<

%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@

%.bin: %
	$(OBJCOPY) -O binary -R .eeprom $< $@

%.eep.hex: %
	$(OBJCOPY) --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex -j .eeprom $< $@

%.lst: %
	$(OBJDUMP) -h -S $< > $@

%-size: %.hex
	$(SIZE) $<

##############################################################################
#Rules for simulator build

SUBDIRS_SIM  = $(TOPDIR)/simulator
SUBDIRS_SIM += $(SUBDIRS)

.PHONY: compile-subdirs_sim
compile-subdirs_sim:
	@ for dir in $(SUBDIRS_SIM); do $(MAKE) -C $$dir objects_sim || exit 5; done
	@ $(MAKE) -C $(TOPDIR)/simulator/ objects_sim || exit 5;

simulator: $(TOPDIR)/autoconf.h .config .subdirs compile-subdirs_sim $(TARGET_SIM)

SUBDIROBJECTS_SIM = $(foreach subdir,$(SUBDIRS_SIM),$(foreach object,$(shell cat $(subdir)/obj_sim/.objects 2>/dev/null),$(subdir)/$(object)))

OBJECTS_SIM = $(patsubst $(TOPDIR)/%.c,$(TOPDIR)/obj_sim/%.o,${SRC_SIM})

$(TARGET_SIM): $(OBJECTS_SIM) $(SUBDIROBJECTS_SIM)
	$(HOSTCC) $(LDFLAGS_SIM) -o $@ $(OBJECTS_SIM) $(SUBDIROBJECTS_SIM) $(LIBS_SIM)

$(TOPDIR)/obj_sim/%.o: $(TOPDIR)/%.c
	@ if [ ! -d $(TOPDIR)/obj_sim ]; then mkdir $(TOPDIR)/obj_sim ; fi
	@ echo "compiling $<"
	@ $(HOSTCC) -o $@ $(CFLAGS_SIM) -c $<

##############################################################################
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
          else if [ -x $$(which bash) ]; then echo $$(which bash); \
          else echo sh; fi ; fi)

menuconfig:
	@echo $(CONFIG_SHELL)
	$(MAKE) -C scripts/lxdialog all
	$(CONFIG_SHELL) scripts/Menuconfig config.in
	test -e .config
	@echo ""
	@echo "Next, you can: "
	@echo " * 'make' to compile your borgware"

test:
	@echo $(CONFIG_SHELL)
	@echo
	@echo "========== Testing borg-16 ========== "
	$(MAKE) -C scripts/lxdialog all
	$(CONFIG_SHELL) scripts/Menuconfig config.in profiles/borg-16
	$(MAKE) 
	$(MAKE) clean
	@echo
	@echo "========== Testing borg-ls ========== "
	$(MAKE) -C scripts/lxdialog all
	$(CONFIG_SHELL) scripts/Menuconfig config.in profiles/borg-ls
	$(MAKE) 
	$(MAKE) clean

#%/menuconfig:
#	$(SH) "$(@D)/configure"
#	@$(MAKE) what-now-msg

##############################################################################
clean:
	$(MAKE) -f rules.mk no_deps=t clean-common
	$(RM) -f $(TARGET) $(TARGET).bin $(TARGET).hex $(TARGET).lst .subdirs
	$(RM) -f $(TARGET).map 
	for subdir in `find . -type d` ; do \
	  test "x$$subdir" != "x." \
	  && test -e $$subdir/Makefile \
	  && $(MAKE) no_deps=t -C $$subdir clean ; done ; true
	$(RM) -fr $(TOPDIR)/obj_avr $(TOPDIR)/obj_sim
	$(RM) -f $(TARGET_SIM) $(TARGET_SIM).exe

mrproper:
	$(MAKE) clean
	$(RM) -f $(TOPDIR)/autoconf.h .config config.mk .menuconfig.log .config.old

#sflash: $(TARGET).hex
#	$(LAUNCH_BOOTLOADER) $(SERIAL) 115200
#	avrdude -p m32 -b 115200 -u -c avr109 -P $(SERIAL) -U f:w:$< -F
#	echo X > $(SERIAL)

#uflash: $(TARGET).hex
#	avrdude -c usbasp  -p atmega32 -V -U f:w:$< -F

.PHONY: clean mrproper sflash uflash
##############################################################################
# configure ethersex
#
show-config: $(TOPDIR)/autoconf.h
	@echo
	@echo "These modules are currently enabled: "
	@echo "======================================"
	@grep -e "^#define .*_SUPPORT" $(TOPDIR)/autoconf.h | sed -e "s/^#define / * /" -e "s/_SUPPORT.*//"

.PHONY: show-config

$(TOPDIR)/autoconf.h .config:
	@echo make\'s goal: $(MAKECMDGOALS)
ifneq ($(MAKECMDGOALS),menuconfig)
	# make sure menuconfig isn't called twice, on `make menuconfig'
	#test -s $(TOPDIR)/autoconf.h -a -s .config || $(MAKE) no_deps=t menuconfig
	# test the target file, test fails if it doesn't exist
	# and will keep make from looping menuconfig.
	#test -s $(TOPDIR)/autoconf.h -a -s .config
	touch $(TOPDIR)/autoconf.h .config
endif

include $(MAKETOPDIR)/depend.mk
