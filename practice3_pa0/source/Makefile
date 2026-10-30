CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -mcpu=cortex-m3 -mthumb -std=c99 -Os -ffreestanding -ffunction-sections -fdata-sections -DSTM32F103C8 -DSTM32F10X_MD -Icmsis -Istm32
LDFLAGS = -nostdlib -nostartfiles -Tstm32f103c8.ld -Wl,--gc-sections
SOURCES = main.c stm32/startup/startup_stm32f10x_md.c stm32/system_stm32f10x.c

all: firmware.bin firmware.hex

firmware.elf: $(SOURCES) stm32f103c8.ld
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $@

firmware.bin: firmware.elf
	$(OBJCOPY) -O binary $< $@

firmware.hex: firmware.elf
	$(OBJCOPY) -O ihex $< $@

clean:
	rm -f firmware.elf firmware.bin firmware.hex

.PHONY: all clean
