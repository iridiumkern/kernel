#ifndef DEBUG
#include <panic.hpp>
#else

#include "../../../../generic/lib/flanterm/src/flanterm.h"
#include "../dt/idt/exception.h"
#include <x86_64/io.hpp>
#include <stdio.hpp>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

extern flanterm_context *flantermctx;
#define DEBUG_INPUT_SIZE 128

extern void TraceStackTrace(unsigned int MaxFrames);

namespace debug {
	static char input[DEBUG_INPUT_SIZE];
	static size_t input_pos = 0;

	static bool shift = false;
	static bool capslock = false;
	static bool extended = false;

	static char decode_scancode(uint8_t scancode) {
		bool released = scancode & 0x80;
		scancode &= 0x7F;

		if (scancode == 0x2A || scancode == 0x36) {
			shift = !released;
			return 0;
		}

		if (scancode == 0x3A && !released) {
			capslock = !capslock;
			return 0;
		}

		if (released)
			return 0;

		if (extended) {
			extended = false;
			return 0;
		}

		if (scancode == 0xE0) {
			extended = true;
			return 0;
		}

		switch (scancode) {
			case 0x01: return 27;
			case 0x02: return shift ? '!' : '1';
			case 0x03: return shift ? '@' : '2';
			case 0x04: return shift ? '#' : '3';
			case 0x05: return shift ? '$' : '4';
			case 0x06: return shift ? '%' : '5';
			case 0x07: return shift ? '^' : '6';
			case 0x08: return shift ? '&' : '7';
			case 0x09: return shift ? '*' : '8';
			case 0x0A: return shift ? '(' : '9';
			case 0x0B: return shift ? ')' : '0';
			case 0x0C: return shift ? '_' : '-';
			case 0x0D: return shift ? '+' : '=';
			case 0x0E: return '\b';
			case 0x0F: return '\t';

			case 0x10: return (shift ^ capslock) ? 'Q' : 'q';
			case 0x11: return (shift ^ capslock) ? 'W' : 'w';
			case 0x12: return (shift ^ capslock) ? 'E' : 'e';
			case 0x13: return (shift ^ capslock) ? 'R' : 'r';
			case 0x14: return (shift ^ capslock) ? 'T' : 't';
			case 0x15: return (shift ^ capslock) ? 'Y' : 'y';
			case 0x16: return (shift ^ capslock) ? 'U' : 'u';
			case 0x17: return (shift ^ capslock) ? 'I' : 'i';
			case 0x18: return (shift ^ capslock) ? 'O' : 'o';
			case 0x19: return (shift ^ capslock) ? 'P' : 'p';
			case 0x1A: return shift ? '{' : '[';
			case 0x1B: return shift ? '}' : ']';
			case 0x1C: return '\n';

			case 0x1E: return (shift ^ capslock) ? 'A' : 'a';
			case 0x1F: return (shift ^ capslock) ? 'S' : 's';
			case 0x20: return (shift ^ capslock) ? 'D' : 'd';
			case 0x21: return (shift ^ capslock) ? 'F' : 'f';
			case 0x22: return (shift ^ capslock) ? 'G' : 'g';
			case 0x23: return (shift ^ capslock) ? 'H' : 'h';
			case 0x24: return (shift ^ capslock) ? 'J' : 'j';
			case 0x25: return (shift ^ capslock) ? 'K' : 'k';
			case 0x26: return (shift ^ capslock) ? 'L' : 'l';
			case 0x27: return shift ? ':' : ';';
			case 0x28: return shift ? '"' : '\'';
			case 0x29: return shift ? '~' : '`';
			case 0x2B: return shift ? '|' : '\\';

			case 0x2C: return (shift ^ capslock) ? 'Z' : 'z';
			case 0x2D: return (shift ^ capslock) ? 'X' : 'x';
			case 0x2E: return (shift ^ capslock) ? 'C' : 'c';
			case 0x2F: return (shift ^ capslock) ? 'V' : 'v';
			case 0x30: return (shift ^ capslock) ? 'B' : 'b';
			case 0x31: return (shift ^ capslock) ? 'N' : 'n';
			case 0x32: return (shift ^ capslock) ? 'M' : 'm';
			case 0x33: return shift ? '<' : ',';
			case 0x34: return shift ? '>' : '.';
			case 0x35: return shift ? '?' : '/';

			case 0x37: return '*';
			case 0x39: return ' ';

			case 0x47: return '7';
			case 0x48: return '8';
			case 0x49: return '9';
			case 0x4A: return '-';
			case 0x4B: return '4';
			case 0x4C: return '5';
			case 0x4D: return '6';
			case 0x4E: return '+';
			case 0x4F: return '1';
			case 0x50: return '2';
			case 0x51: return '3';
			case 0x52: return '0';
			case 0x53: return '.';

			default: return 0;
		}
	}

	bool parse_uint64(const char *str, uint64_t *out) {
		uint64_t value = 0;
		uint32_t base = 10;

		if (!str || !out || !*str)
			return false;

		if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
			base = 16;
			str += 2;

			if (!*str)
				return false;
		}

		for (; *str; ++str) {
			uint32_t digit;

			if (*str >= '0' && *str <= '9')
				digit = *str - '0';
			else if (base == 16 && *str >= 'a' && *str <= 'f')
				digit = *str - 'a' + 10;
			else if (base == 16 && *str >= 'A' && *str <= 'F')
				digit = *str - 'A' + 10;
			else
				return false;

			if (digit >= base)
				return false;

			if (value > (UINT64_MAX - digit) / base)
				return false;

			value = value * base + digit;
		}

		*out = value;
		return true;
	}

	static bool keyboard_poll(char* output) {
		uint8_t status = inb(0x64);

		if (!(status & 1))
			return false;

		// Ignore auxiliary-device data.
		if (status & 0x20) {
			(void)inb(0x60);
			return false;
		}

		uint8_t scancode = inb(0x60);
		*output = decode_scancode(scancode);

		return *output != 0;
	}

	static void dumpframectx(const struct interrupt_frame *frame) {
		printf("EXCEPTION!\n");
		printf("Exception:  %lu\n", frame->exception_code);
		printf("Error code: %lu\n", frame->errorcode);
		printf("RIP:        0x%lx\n", frame->rip);
		printf("CS:         0x%lx\n", frame->cs);
		printf("RFLAGS:     0x%lx\n", frame->rflags);
		if (frame->cs & 0x03) {
			printf("RSP:	0x%lx\n", frame->rsp);
			printf("SS:	0x%lx\n", frame->ss);
		}
	}

	static void dumpstack(char* argv[8], size_t argc) {
		if (argc != 2) {
			printf("USAGE: rewindstack [rewind count]\n");
			return;
		}

		uint64_t rewindcount = 0;
		if (!parse_uint64(argv[1], &rewindcount)) {
			return;
		}

		TraceStackTrace(rewindcount);
	}

	static void execute_command(char* command, const struct interrupt_frame *frame) {
		(void)frame;
		char* argv[8];
		size_t argc = 0;

		// Tokenize in-place.
		char* p = command;

		while (*p && argc < 8) {
			while (*p == ' ' || *p == '\t')
				p++;

			if (!*p)
				break;

			argv[argc++] = p;

			while (*p && *p != ' ' && *p != '\t')
				p++;

			if (*p)
				*p++ = '\0';
		}

		if (!argc)
			return;

		if (!strcmp(argv[0], "help")) {
			printf("Available commands:\n");
			printf("  help        Show this message\n");
			printf("  halt        Halt the system\n");
			printf("  clear       Clears the screen\n");
			printf("  dumpctx     Dumps arch CTX\n");
			printf("  rewindstack Dumps callstack\n");
		} else if (!strcmp(argv[0], "halt")) {
			printf("Halting...\n");

			asm volatile("cli");

			for (;;)
				asm volatile("hlt");
		} else if (!strcmp(argv[0], "clear")) {
			flanterm_clear(flantermctx, true);
		} else if (!strcmp(argv[0], "dumpctx")) {
			dumpframectx(frame);
		} else if (!strcmp(argv[0], "rewindstack")) {
			dumpstack(argv, argc);
		} else {
			printf("Unknown command: %s\n", argv[0]);
		}
	}

	static void shell_loop(struct interrupt_frame *frame) {
		printf("\nIridium Debug Shell\n");
		printf("Type 'help' for commands.\n\n");
		printf("> ");

		for (;;) {
			char c = 0;

			if (!keyboard_poll(&c))
				continue;

			if (c == '\b') {
				if (input_pos > 0) {
					input[--input_pos] = '\0';
					printf("\b \b");
				}
				continue;
			}

			if (c == '\n') {
				printf("\n");

				input[input_pos] = '\0';
				execute_command(input, frame);

				memset(input, 0, sizeof(input));
				input_pos = 0;

				printf("> ");
				continue;
			}

			if (c == 27)
				continue;

			if (input_pos < DEBUG_INPUT_SIZE - 1) {
				input[input_pos++] = c;
				printf("%c", c);
			}
		}
	}
}

extern "C" void debug_shell_enter(struct interrupt_frame *frame) {
	debug::shell_loop(frame);
}

#endif
