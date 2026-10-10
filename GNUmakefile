ARCH ?= x86_64
PLATFORM ?= generic

CC := clang
CPP := clang++
LD := ld.lld
OBJCOPY := llvm-objcopy

OUTPUT := iridium
GIT_HASH := $(shell git rev-parse --short=7 HEAD)
GIT_BRANCH := $(shell git branch --show-current)
MAJOR = 0
MINOR = 1

IMAGE := build/$(ARCH)/$(PLATFORM)/$(OUTPUT).hdd

SRC_DIR := src
ARCH_DIR := $(SRC_DIR)/arch/$(ARCH)/$(PLATFORM)
BUILD := build/$(ARCH)/$(PLATFORM)
KERNEL := $(BUILD)/$(OUTPUT)

LINKER_SCRIPT := $(ARCH_DIR)/linker.lds

CFLAGS := -g -O3
LDFLAGS :=

CFLAGS += -DGIT_HASH=\"$(GIT_HASH)\"
CFLAGS += -DGIT_BRANCH=\"$(GIT_BRANCH)\"
CFLAGS += -DMAJORVER=\"$(MAJOR)\"
CFLAGS += -DMINORVER=\"$(MINOR)\"

ifeq ($(ARCH),x86_64)
	TARGET := x86_64-unknown-none-elf
	CFLAGS += -m64 -march=x86-64 -mabi=sysv -mno-red-zone -mcmodel=kernel -masm=intel
	LDFLAGS += -m elf_x86_64
endif

CC += -target $(TARGET)
CPP += -target $(TARGET)

override CFLAGS += \
	-Wall \
	-Wextra \
	-Werror \
	-ffreestanding \
	-fno-stack-check \
	-fno-PIC \
	-ffunction-sections \
	-fdata-sections \
	-flto \
	-fvisibility=default \
	-fno-omit-frame-pointer

# Security related flags
override CFLAGS += \
	-fstack-protector-all \
	-fsanitize=undefined \
	-fsanitize-minimal-runtime \
	-fsanitize=bounds \
	-fsanitize=object-size \
	-fsanitize=alignment \
	-fsanitize=bool \
	-fsanitize=enum \
	-fsanitize=shift \
	-fsanitize=shift-base \
	-fsanitize=shift-exponent \
	-fsanitize=integer-divide-by-zero \
	-fsanitize=unreachable \
	-fsanitize=return \
	-fsanitize=nonnull-attribute \
	-fsanitize=null \
	-fsanitize=implicit-integer-truncation \
	-fsanitize=implicit-integer-sign-change \
	-fsanitize=cfi

CPFLAGS = $(CFLAGS) \
	-std=c++23 -fno-exceptions -fno-rtti

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

GENERIC_SRCFILES := $(shell find -L $(GENERIC_DIR) -type f \( -name '*.c' -o -name '*.cpp' \) 2>/dev/null | LC_ALL=C sort)
ARCH_SRCFILES := $(shell find -L $(ARCH_DIR) -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.S' \) 2>/dev/null | LC_ALL=C sort)

SRCFILES := $(GENERIC_SRCFILES) $(ARCH_SRCFILES)

CFILES := $(filter %.c,$(SRCFILES))
CPFILES := $(filter %.cpp,$(SRCFILES))
ASFILES := $(filter %.S,$(SRCFILES))

OBJECTS := \
	$(patsubst $(SRC_DIR)/%.c,$(BUILD)/%.c.o,$(CFILES)) \
	$(patsubst $(SRC_DIR)/%.cpp,$(BUILD)/%.cpp.o,$(CPFILES)) \
	$(patsubst $(SRC_DIR)/%.S,$(BUILD)/%.S.o,$(ASFILES))

HEADER_DEPS := \
	$(patsubst $(SRC_DIR)/%.c,$(BUILD)/%.c.d,$(CFILES)) \
	$(patsubst $(SRC_DIR)/%.cpp,$(BUILD)/%.cpp.d,$(CPFILES)) \
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
	$(CC) $(CFLAGS) -std=c99 $(CPPFLAGS) -c $< -o $@

$(BUILD)/%.cpp.o: $(SRC_DIR)/%.cpp GNUmakefile
	@mkdir -p "$(dir $@)"
	$(CPP) $(CPFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD)/%.S.o: $(SRC_DIR)/%.S GNUmakefile
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf build
	make -C userland clean

qemu:
ifeq ($(ARCH),x86_64)
	qemu-system-x86_64 -hda $(IMAGE) -serial file:serial.log -bios /usr/share/edk2/OvmfX64/OVMF_CODE.fd -m 512M -monitor stdio
endif