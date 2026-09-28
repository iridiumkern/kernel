#!/usr/bin/env python3

# Make sure to cut off the top actual log before running this script with serial.log!

import sys

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} input.log output.bin")
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]

total = 0

with open(input_file, "r", encoding="ascii") as src, \
     open(output_file, "wb") as dst:

    for line_number, line in enumerate(src, 1):
        # Expected format:
        # 00000000  AA BB CC DD EE FF ... |................|
        #
        # The first 8 characters are the offset.
        # The next 48 characters contain the 16 hex bytes.

        if len(line) < 58:
            continue

        fields = line[10:58].split()

        if len(fields) != 16:
            # Last/partial lines aren't expected with your 512 KiB
            # chunks, but reject malformed input rather than silently
            # producing bad entropy data.
            print(f"malformed line {line_number}", file=sys.stderr)
            sys.exit(1)

        try:
            data = bytes.fromhex("".join(fields))
        except ValueError:
            print(f"invalid hex on line {line_number}", file=sys.stderr)
            sys.exit(1)

        dst.write(data)
        total += len(data)

print(f"wrote {total} bytes ({total * 8} bits)")