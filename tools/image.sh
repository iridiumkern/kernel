#!/bin/sh
set -eu

IMAGE="${1:?missing image path}"
KERNEL="${2:?missing kernel path}"

LIMINE_DIR="${LIMINE_DIR:-limine-binary}"
LIMINE_URL="${LIMINE_URL:-https://github.com/Limine-Bootloader/Limine/releases/latest/download/limine-binary.tar.gz}"

rm -f "$IMAGE"

mkdir -p "$(dirname "$IMAGE")"

# Create an empty zeroed-out 64 MiB image file.
dd if=/dev/zero bs=1M count=0 seek=64 of="$IMAGE"

# Create a partition table.
PATH="$PATH:/usr/sbin:/sbin" sgdisk "$IMAGE" -n 1:2048 -t 1:ef00 -m 1

# Download the latest Limine binary release if necessary.
if [ ! -d "$LIMINE_DIR" ]; then
	curl -L "$LIMINE_URL" | gunzip | tar -xf -
fi

# Build the Limine utility.
make -C "$LIMINE_DIR"

# Install the Limine BIOS stages.
"./$LIMINE_DIR/limine" bios-install "$IMAGE"

# Format the image as FAT32.
mformat -i "$IMAGE@@1M"

# Create relevant directories.
mmd -i "$IMAGE@@1M" \
	::/EFI \
	::/EFI/BOOT \
	::/boot \
	::/boot/limine

# Copy kernel and Limine files.
mcopy -i "$IMAGE@@1M" "$KERNEL" ::/boot
mcopy -i "$IMAGE@@1M" limine.conf "$LIMINE_DIR/limine-bios.sys" ::/boot/limine
mcopy -i "$IMAGE@@1M" "$LIMINE_DIR/BOOTX64.EFI" ::/EFI/BOOT
mcopy -i "$IMAGE@@1M" "$LIMINE_DIR/BOOTIA32.EFI" ::/EFI/BOOT
mcopy -i "$IMAGE@@1M" "$LIMINE_DIR/BOOTAA64.EFI" ::/EFI/BOOT