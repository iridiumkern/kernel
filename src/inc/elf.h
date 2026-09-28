#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <pmm.h>

#ifdef __x86_64__
#include <x86_64/vmm.h>
#endif

#define ELF_NIDENT	16

#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#ifdef __x86_64__
#define ELFCLASS 2
#define ELFDATA 1
#else
#error YOUR ARCH DOES NOT DEFINE ELFCLASS AND ELFDATA
#endif

#define EV_CURRENT 1

#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_ABIVERSION 8
#define EI_PAD 9

#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4

#define EM_X86_64 62

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE	4
#define PT_SHLIB 5
#define PT_PHDR	6
#define PT_TLS 7

#define PF_X 0x01
#define PF_W 0x02
#define PF_R 0x04

#define VMM_MAP_READ (1ULL << 0)
#define VMM_MAP_WRITE (1ULL << 1)
#define VMM_MAP_NX (1ULL << 2)

#define PAGE_SIZE 0x1000ULL
#define PAGE_MASK (PAGE_SIZE - 1)

typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;

typedef struct {
	uint8_t e_ident[ELF_NIDENT];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off e_phoff;
	Elf64_Off e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct {
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	Elf64_Xword p_filesz;
	Elf64_Xword p_memsz;
	Elf64_Xword p_align;
} Elf64_Phdr;

typedef struct {
	uint64_t entry;
	uint64_t load_bias;
	uint64_t virt_base;
	uint64_t phys_base;
	uint64_t size;
} Elf64_Image;

bool elf_load_file(const void *file, size_t file_size, uint64_t load_bias, Elf64_Image *image);