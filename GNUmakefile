ARCH      ?= aarch64
PLATFORM  ?= generic

CC        := clang
LD        := ld.lld
OBJCOPY   := llvm-objcopy

OUTPUT    := iridium

IMAGE := build/$(ARCH)/$(PLATFORM)/$(OUTPUT).hdd

SRC_DIR   := src
ARCH_DIR  := $(SRC_DIR)/arch/$(ARCH)/$(PLATFORM)
BUILD     := build/$(ARCH)/$(PLATFORM)
KERNEL    := $(BUILD)/$(OUTPUT)

LINKER_SCRIPT := $(ARCH_DIR)/linker.lds

CFLAGS    := -g -O0
CPPFLAGS  :=
LDFLAGS   :=

ifeq ($(ARCH),x86_64)
	TARGET := x86_64-unknown-none-elf
	CFLAGS += -m64 -march=x86-64 -mabi=sysv \
			  -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
			  -mno-red-zone -mcmodel=kernel
	LDFLAGS += -m elf_x86_64
endif

ifeq ($(ARCH),aarch64)
	TARGET := aarch64-unknown-none-elf
endif

CC += -target $(TARGET)

override CFLAGS += \
	-Wall \
	-Wextra \
	-Werror \
	-std=c99 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-fno-PIC \
	-ffunction-sections \
	-fdata-sections

override CPPFLAGS := \
	-I$(SRC_DIR)/inc \
	$(CPPFLAGS) \
	-MMD \
	-MP

override LDFLAGS += \
	-nostdlib \
	-static \
	-z max-page-size=0x1000 \
	--gc-sections \
	-T $(LINKER_SCRIPT)

GENERIC_DIR := $(SRC_DIR)/generic

GENERIC_SRCFILES := $(shell find -L $(GENERIC_DIR) -type f 2>/dev/null | LC_ALL=C sort)
ARCH_SRCFILES    := $(shell find -L $(ARCH_DIR) -type f -name '*.[cS]' 2>/dev/null | LC_ALL=C sort)

SRCFILES := $(GENERIC_SRCFILES) $(ARCH_SRCFILES)

CFILES  := $(filter %.c,$(SRCFILES))
ASFILES := $(filter %.S,$(SRCFILES))

OBJECTS := \
	$(patsubst $(SRC_DIR)/%.c,$(BUILD)/%.c.o,$(CFILES)) \
	$(patsubst $(SRC_DIR)/%.S,$(BUILD)/%.S.o,$(ASFILES))

HEADER_DEPS := \
	$(patsubst $(SRC_DIR)/%.c,$(BUILD)/%.c.d,$(CFILES)) \
	$(patsubst $(SRC_DIR)/%.S,$(BUILD)/%.S.d,$(ASFILES))

.PHONY: all clean

all: $(KERNEL)

image: $(KERNEL)
	@tools/image.sh "$(IMAGE)" "$(KERNEL)"

-include $(HEADER_DEPS)

$(KERNEL): GNUmakefile $(LINKER_SCRIPT) $(OBJECTS)
	@mkdir -p "$(dir $@)"
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(BUILD)/%.c.o: $(SRC_DIR)/%.c GNUmakefile
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD)/%.S.o: $(SRC_DIR)/%.S GNUmakefile
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf build

qemu:
ifeq ($(ARCH),x86_64)
	qemu-system-x86_64 -hda $(IMAGE)
endif

ifeq ($(ARCH),aarch64)
	qemu-system-aarch64 \
	    -M virt \
	    -cpu cortex-a72 \
	    -m 512M \
	    -smp 1 \
	    -drive if=pflash,unit=0,format=qcow2,file=/usr/share/edk2/ArmVirtQemu-AARCH64/QEMU_EFI.qcow2,readonly=on \
	    -device ramfb \
	    -serial stdio \
		-hda $(IMAGE) \
		-device qemu-xhci \
		-device usb-kbd
endif