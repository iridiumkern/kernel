CC := clang
LD := ld.lld

CFLAGS := -Wall -Wextra -Werror -Iinc -fPIE
LDFLAGS := -T linker.ld -pie
ARCH = x86_64

SRC := $(shell find src libc/arch/$(ARCH) libc/generic -type f -name '*.c')
OBJ := $(SRC:.c=.o)
TARGET := pid0.bin

all: $(TARGET)

$(TARGET): $(OBJ) linker.ld
	$(LD) $(LDFLAGS) $(OBJ) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean