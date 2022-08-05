ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_rules


################################################################################

IPL_LOAD_ADDR := 0x40000000
MAX_PAYLOAD_SIZE := 49152

################################################################################

BUILD_DIR = ./build
OUT_DIR = ./output

SRC_DIR = ./ums-loader

BDK_DIR = ./bdk
GFX_DIR = ./ums-loader/gfx
STORAGE_DIR = ./ums-loader/storage

VPATH = $(dir $(STORAGE_DIR)/) $(dir $(GFX_DIR)/) $(dir $(SRC_DIR)/) $(dir $(BDK_DIR)/) $(BDK_DIR)/ $(dir $(wildcard $(BDK_DIR)/*/)) $(dir $(wildcard $(BDK_DIR)/*/*/))

TARGET = ums-loader
PAYLOAD_NAME = $(TARGET)

OBJS = $(addprefix $(BUILD_DIR)/$(TARGET)/, \
    start.o main.o \
    exception_handlers.o irq.o \
	heap.o mc.o bpmp.o clock.o fuse.o se.o hw_init.o gpio.o pinmux.o i2c.o util.o btn.o \
    max7762x.o bq24193.o max77620-rtc.o \
    sdmmc.o nx_sd.o sdmmc_driver.o \
	usb_gadget_ums.o usb_descriptors.o xusbd.o sprintf.o \
	di.o gfx.o tui.o)

GFX_INC = '"../$(GFX_DIR)/gfx.h"'
INC_DIR = -I./$(BDK_DIR) -I./$(SRC_DIR) -I./$(GFX_DIR)

CUSTOMDEFINES += -DGFX_INC=$(GFX_INC) -DMAX_PAYLOAD_SIZE=$(MAX_PAYLOAD_SIZE)

WARNINGS := -Wall -Wno-array-bounds -Wno-stringop-overread -Wno-stringop-overflow
ARCH := -march=armv4t -mtune=arm7tdmi -mthumb-interwork -mthumb -Wstack-usage=2048
CFLAGS = $(ARCH) -O2 -flto -g -nostdlib -ffunction-sections -fdata-sections -std=gnu11  $(WARNINGS) $(CUSTOMDEFINES) -fno-inline
LDFLAGS = $(ARCH) -nostartfiles -lgcc -Wl,--nmagic,--gc-sections -Xlinker --defsym=IPL_LOAD_ADDR=$(IPL_LOAD_ADDR)


.PHONY: all

all: $(OUT_DIR)/$(PAYLOAD_NAME).bin
	$(eval PAYLOAD_SIZE = $(shell wc -c < $(OUT_DIR)/$(PAYLOAD_NAME).bin))
	@echo "Payload size is ${PAYLOAD_SIZE}"
	@if [ ${PAYLOAD_SIZE} -gt ${MAX_PAYLOAD_SIZE} ]; then echo "\033[0;31m ERROR: Payload size is ${PAYLOAD_SIZE} bytes. Maximum allowed is ${MAX_PAYLOAD_SIZE} bytes \033[0m"; fi

$(OUT_DIR)/$(PAYLOAD_NAME).bin: $(BUILD_DIR)/$(TARGET)/$(TARGET).elf | $(OUT_DIR)
	$(OBJCOPY) -S -O binary $< $@

$(BUILD_DIR)/$(TARGET)/$(TARGET).elf: $(OBJS)
	@$(CC) $(LDFLAGS) -T $(SRC_DIR)/link.ld $^ -o $@

$(BUILD_DIR)/$(TARGET)/%.o: %.c
	$(CC) $(CFLAGS) $(INC_DIR) -c $< -o $@

$(BUILD_DIR)/$(TARGET)/%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET)/%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS): | $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET):
	@mkdir -p "$(BUILD_DIR)/$(TARGET)"

$(OUT_DIR):
	@mkdir -p "$(OUT_DIR)"

clean:
	rm -rf $(OUT_DIR)
	rm -rf $(BUILD_DIR)/$(TARGET)
