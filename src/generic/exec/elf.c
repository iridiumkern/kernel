#include <stdbool.h>
#include <stdint.h>

#define ELF_NIDENT	16

#define ELFMAG0	0x7F // e_ident[EI_MAG0]
#define ELFMAG1	'E'  // e_ident[EI_MAG1]
#define ELFMAG2	'L'  // e_ident[EI_MAG2]
#define ELFMAG3	'F'  // e_ident[EI_MAG3]

#ifdef __x86_64__
#define ELFDATA2LSB	(1)  // Little Endian
#define ELFCLASS	(2)  // x86_64
#define EM_CURRENT  (62)  // x86_64 Machine Type
#else
#error Please define the ELF data for your current target arch.
#endif

#define EV_CURRENT	(1)  // ELF Current Version

typedef uint16_t Elf32_Half; // Unsigned half int
typedef uint32_t Elf32_Off; // Unsigned offset
typedef uint32_t Elf32_Addr; // Unsigned address
typedef uint32_t Elf32_Word; // Unsigned int
typedef int32_t  Elf32_Sword; // Signed int

typedef struct {
	uint8_t e_ident[ELF_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off  sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf32_Shdr;

enum Elf_Ident {
	EI_MAG0		= 0, // 0x7F
	EI_MAG1		= 1, // 'E'
	EI_MAG2		= 2, // 'L'
	EI_MAG3		= 3, // 'F'
	EI_CLASS	= 4, // Architecture (32/64)
	EI_DATA		= 5, // Byte Order
	EI_VERSION	= 6, // ELF Version
	EI_OSABI	= 7, // OS Specific
	EI_ABIVERSION	= 8, // OS Specific
	EI_PAD		= 9  // Padding
};

enum Elf_Type {
	ET_NONE		= 0, // Unkown Type
	ET_REL		= 1, // Relocatable File
	ET_EXEC		= 2  // Executable File
};

static bool elf_check_file(Elf32_Ehdr *hdr) {
	if(!hdr) return false;
	if(hdr->e_ident[EI_MAG0] != ELFMAG0) {
		return false;
	}
	if(hdr->e_ident[EI_MAG1] != ELFMAG1) {
		return false;
	}
	if(hdr->e_ident[EI_MAG2] != ELFMAG2) {
		return false;
	}
	if(hdr->e_ident[EI_MAG3] != ELFMAG3) {
		return false;
	}
	return true;
}

static bool elf_check_supported(Elf32_Ehdr *hdr) {
	if(!elf_check_file(hdr)) {
		return false;
	}
	if(hdr->e_ident[EI_CLASS] != ELFCLASS) {
		return false;
	}
	if(hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
		return false;
	}
	if(hdr->e_machine != EM_CURRENT) {
		return false;
	}
	if(hdr->e_ident[EI_VERSION] != EV_CURRENT) {
		return false;
	}
	/*
	ET_EXEC is not supported by Iridium as its mostly just a nightmare to properly implement.
	ET_REL is the only supported system.
	*/
	if(hdr->e_type != ET_REL) {
		return false;
	}
	return true;
}

static inline void *elf_load_rel(Elf32_Ehdr *hdr) {
	int result;
	result = elf_load_stage1(hdr);
	if(result == ELF_RELOC_ERR) {
		ERROR("Unable to load ELF file.\n");
		return NULL;
	}
	result = elf_load_stage2(hdr);
	if(result == ELF_RELOC_ERR) {
		ERROR("Unable to load ELF file.\n");
		return NULL;
	}
	// TODO : Parse the program header (if present)
	return (void *)hdr->e_entry;
}

void *elf_load_file(void *file) {
	Elf32_Ehdr *hdr = (Elf32_Ehdr *)file;
	if(!elf_check_supported(hdr)) {
		ERROR("ELF File cannot be loaded.\n");
		return;
	}
	switch(hdr->e_type) {
		case ET_EXEC:
			// TODO : Implement
			return NULL;
		case ET_REL:
			return elf_load_rel(hdr);
	}
	return NULL;
}