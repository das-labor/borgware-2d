TARGET := image

SRC += src/platform/avr/platform.c

compile-main: compile-sources $(TARGET).hex $(TARGET).bin $(TARGET).lst
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
	@echo "size is: "
	@$(CONFIG_SHELL) scripts/size $(TARGET)
	@echo "==============================="

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