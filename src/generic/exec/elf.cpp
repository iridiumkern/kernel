#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.hpp>
#include <pmm.hpp>

#ifdef __x86_64__
#include <x86_64/vmm.hpp>
#endif

#define ELF_NIDENT	16

#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define R_NONE 0
#ifdef __x86_64__
#define ELFCLASS 2
#define ELFDATA 1 
#define ELF_R_NONE 0
#define ELF_R_RELATIVE 8
#else
#error YOUR ARCH DOES NOT DEFINE 
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

#define DT_NULL 0
#define DT_NEEDED	1
#define DT_RELA		7
#define DT_RELASZ	8
#define DT_RELAENT	9

#define ELF64_R_SYM(info)	((uint32_t)((info) >> 32))
#define ELF64_R_TYPE(info)	((uint32_t)(info))

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

typedef struct {
	Elf64_Addr r_offset;
	Elf64_Xword r_info;
	Elf64_Sxword r_addend;
} Elf64_Rela;

typedef struct {
	Elf64_Sxword d_tag;
	union {
		Elf64_Xword d_val;
		Elf64_Addr d_ptr;
	} d_un;
} Elf64_Dyn;

static uint64_t align_down(uint64_t value) {
	return value & ~PAGE_MASK;
}

static bool align_up(uint64_t value, uint64_t *result) {
	if (value > UINT64_MAX - PAGE_MASK) return false;
	*result = (value + PAGE_MASK) & ~PAGE_MASK;

	return true;
}

static bool add_overflow_u64(uint64_t a, uint64_t b, uint64_t *result) {
	if (b > UINT64_MAX - a) return true;
	*result = a + b;

	return false;
}

static bool mul_overflow_u64(uint64_t a, uint64_t b, uint64_t *result) {
	if (a != 0 && b > UINT64_MAX / a) return true;
	*result = a * b;

	return false;
}

static bool elf_range_valid(size_t file_size, uint64_t offset, uint64_t size) {
	if (offset > file_size) return false;
	if (size > file_size - offset) return false;

	return true;
}

static bool vmm_map_page(uint64_t virt, uint64_t phys, uint64_t size, uint64_t flags) {
	bool result = false;

	#ifdef __x86_64__
	uint64_t newsize;
	uint64_t realflags = VMM_P;

	if (size == 0) return false;

	if (!align_up(size, &newsize)) return false;
	if ((newsize & PAGE_MASK) != 0) return false;
	if (flags & VMM_MAP_WRITE) realflags |= VMM_RW;
	if (flags & VMM_MAP_NX) realflags |= VMM_NX;
	realflags |= VMM_US;

	result = vmm_map_pages(virt, phys, newsize / PAGE_SIZE, realflags);
	#else
	(void)virt;
	(void)phys;
	(void)size;
	(void)flags;
	#endif

	return result;
}

static bool elf_check_header(const Elf64_Ehdr *hdr, size_t file_size) {
	if (hdr == NULL) return false;

	if (file_size < sizeof(Elf64_Ehdr)) return false;
	if (hdr->e_ident[EI_MAG0] != ELFMAG0) return false;
	if (hdr->e_ident[EI_MAG1] != ELFMAG1) return false;
	if (hdr->e_ident[EI_MAG2] != ELFMAG2) return false;
	if (hdr->e_ident[EI_MAG3] != ELFMAG3) return false;
	if (hdr->e_ident[EI_CLASS] != ELFCLASS) return false;
	if (hdr->e_ident[EI_DATA] != ELFDATA) return false;
	if (hdr->e_ident[EI_VERSION] != EV_CURRENT) return false;
	if (hdr->e_machine != EM_X86_64) return false;
	if (hdr->e_version != EV_CURRENT) return false;
	if (hdr->e_ehsize != sizeof(Elf64_Ehdr)) return false;
	if (hdr->e_phentsize != sizeof(Elf64_Phdr)) return false;
	if (hdr->e_phnum == 0) return false;

	for (size_t i = EI_PAD; i < ELF_NIDENT; i++) {
		if (hdr->e_ident[i] != 0) return false;
	}

	return true;
}

static bool elf_check_phdrs(const Elf64_Ehdr *hdr, size_t file_size) {
	uint64_t size;

	if (mul_overflow_u64(hdr->e_phnum, hdr->e_phentsize, &size)) return false;
	if (!elf_range_valid(file_size, hdr->e_phoff, size)) return false;

	return true;
}

static const Elf64_Phdr *elf_phdr(const Elf64_Ehdr *hdr, uint16_t index) {
	const uint8_t *base = (const uint8_t *)hdr;
	return (const Elf64_Phdr *)(base + hdr->e_phoff + ((uint64_t)index * hdr->e_phentsize));
}

static bool elf_check_load_segment(const Elf64_Phdr *phdr, size_t file_size) {
	uint64_t file_end;
	uint64_t mem_end;
	uint64_t page_size = (uint64_t)PAGE_SIZE;

	if (phdr == NULL) return false;
	if (phdr->p_type != (Elf64_Word)PT_LOAD) return true;
	if (phdr->p_flags & ~(Elf64_Xword)(PF_R | PF_W | PF_X)) return false;
	if (phdr->p_filesz > phdr->p_memsz) return false;
	if (!elf_range_valid(file_size, phdr->p_offset, phdr->p_filesz)) return false;
	if (add_overflow_u64(phdr->p_offset, phdr->p_filesz, &file_end)) return false;
	if (add_overflow_u64(phdr->p_vaddr, phdr->p_memsz, &mem_end)) return false;

	if (phdr->p_memsz == UINT64_C(0)) return true;

	if (phdr->p_align > UINT64_C(1)) {
		if ((phdr->p_align & (phdr->p_align - UINT64_C(1))) != UINT64_C(0)) return false;
		if (phdr->p_align < page_size) return false;
		if ((phdr->p_vaddr % phdr->p_align) != (phdr->p_offset % phdr->p_align)) return false;
	}

	if (mem_end <= phdr->p_vaddr) return false;

	return true;
}

static uint64_t elf_segment_flags(uint32_t flags) {
	uint64_t vmm_flags = 0;

	if (flags & PF_R) vmm_flags |= VMM_MAP_READ;
	if (flags & PF_W) vmm_flags |= VMM_MAP_WRITE;
	if (!(flags & PF_X)) vmm_flags |= VMM_MAP_NX;

	return vmm_flags;
}

static bool elf_image_bounds(const Elf64_Ehdr *hdr, size_t file_size, uint64_t *low, uint64_t *high) {
	bool found = false;

	*low = UINT64_MAX;
	*high = 0;

	for (uint16_t i = 0; i < hdr->e_phnum; i++) {
		const Elf64_Phdr *phdr = elf_phdr(hdr, i);
		uint64_t end;

		if (phdr->p_type != PT_LOAD) continue;
		if (!elf_check_load_segment(phdr, file_size)) return false;
		if (phdr->p_memsz == 0) continue;
		if (add_overflow_u64(phdr->p_vaddr, phdr->p_memsz, &end)) return false;
		if (phdr->p_vaddr < *low) *low = phdr->p_vaddr;
		if (end > *high) *high = end;

		found = true;
	}

	if (!found) return false;
	*low = align_down(*low);
	if (!align_up(*high, high)) return false;
	if (*high <= *low) return false;

	return true;
}

static bool elf_check_special_segments(const Elf64_Ehdr *hdr) {
	for (uint16_t i = 0; i < hdr->e_phnum; i++) {
		const Elf64_Phdr *phdr = elf_phdr(hdr, i);

		switch (phdr->p_type) {
			case PT_INTERP:
			case PT_SHLIB:
			case PT_TLS: return false;
			default: break;
		}
	}

	return true;
}

static bool elf_check_segment_overlap(const Elf64_Ehdr *hdr, size_t file_size) {
	for (uint16_t i = 0; i < hdr->e_phnum; i++) {
		const Elf64_Phdr *a = elf_phdr(hdr, i);
		uint64_t a_end;

		if (a->p_type != PT_LOAD || a->p_memsz == 0) continue;
		if (!elf_check_load_segment(a, file_size)) return false;
		if (add_overflow_u64(a->p_vaddr, a->p_memsz, &a_end)) return false;

		for (uint16_t j = i + 1; j < hdr->e_phnum; j++) {
			const Elf64_Phdr *b = elf_phdr(hdr, j);
			uint64_t b_end;

			if (b->p_type != PT_LOAD || b->p_memsz == 0) continue;
			if (!elf_check_load_segment(b, file_size)) return false;
			if (add_overflow_u64(b->p_vaddr, b->p_memsz, &b_end)) return false;
			if (a->p_vaddr < b_end && b->p_vaddr < a_end) return false;
		}
	}

	return true;
}

static bool elf_entry_valid(const Elf64_Ehdr *hdr, uint64_t load_bias) {
	uint64_t entry;
	if (add_overflow_u64(hdr->e_entry, load_bias, &entry)) return false;

	for (uint16_t i = 0; i < hdr->e_phnum; i++) {
		const Elf64_Phdr *phdr = elf_phdr(hdr, i);
		uint64_t start;
		uint64_t file_end;
		uint64_t mem_end;

		if (phdr->p_type != PT_LOAD) continue;
		if (!(phdr->p_flags & PF_X)) continue;
		if (phdr->p_memsz == 0) continue;
		if (phdr->p_filesz == 0) continue;

		if (add_overflow_u64(phdr->p_vaddr, load_bias, &start)) return false;
		if (add_overflow_u64(start, phdr->p_filesz, &file_end)) return false;
		if (add_overflow_u64(start, phdr->p_memsz, &mem_end)) return false;
		if (file_end > mem_end) return false;
		if (entry >= start && entry < file_end) return true;
	}

	return false;
}

static bool elf_load_segment(const Elf64_Phdr *phdr, const uint8_t *file, size_t file_size, uint64_t load_bias, uint64_t image_low) {
	uint64_t segment_start;
	uint64_t segment_end;
	uint64_t destination;

	if (phdr->p_type != PT_LOAD) return true;

	if (!elf_check_load_segment(phdr, file_size)) return false;
	if (phdr->p_memsz == 0) return true;
	if (add_overflow_u64(phdr->p_vaddr, load_bias, &segment_start)) return false;
	if (add_overflow_u64(segment_start, phdr->p_memsz, &segment_end)) return false;
	if (segment_start < image_low) return false;

	destination = segment_start;

	if (phdr->p_filesz != 0) {
		memcpy((void *)(uintptr_t)destination, file + phdr->p_offset, (size_t)phdr->p_filesz);
	}

	if (phdr->p_memsz > phdr->p_filesz) {
		uint64_t bss_address;

		if (phdr->p_filesz > UINT64_MAX - destination) return false;
		bss_address = destination + phdr->p_filesz;
		memset((void *)(uintptr_t)bss_address, 0, (size_t)(phdr->p_memsz - phdr->p_filesz));
	}

	return true;
}

static bool elf_relocate(const Elf64_Ehdr *hdr, size_t file_size,
	uint64_t load_bias, uint64_t image_low, uint64_t image_high) {
	const Elf64_Phdr *dynamic_phdr = NULL;
	const Elf64_Dyn *dynamic;
	const Elf64_Rela *rela = NULL;
	uint64_t rela_addr = 0;
	uint64_t rela_size = 0;
	uint64_t rela_ent = sizeof(Elf64_Rela);
	uint64_t dynamic_addr;
	uint64_t dynamic_size;
	bool have_rela = false;
	bool have_relasz = false;
	bool have_relaent = false;
	bool terminated = false;

	for (uint16_t i = 0; i < hdr->e_phnum; i++) {
		const Elf64_Phdr *phdr = elf_phdr(hdr, i);

		if (phdr->p_type != PT_DYNAMIC) continue;
		if (dynamic_phdr != NULL) return false;

		dynamic_phdr = phdr;
	}

	if (dynamic_phdr == NULL) return true;
	if (dynamic_phdr->p_filesz > dynamic_phdr->p_memsz) return false;
	if (dynamic_phdr->p_filesz < sizeof(Elf64_Dyn)) return false;
	if (dynamic_phdr->p_filesz % sizeof(Elf64_Dyn) != 0) return false;
	if (!elf_range_valid(file_size, dynamic_phdr->p_offset,
		dynamic_phdr->p_filesz)) return false;

	if (add_overflow_u64(dynamic_phdr->p_vaddr, load_bias,
		&dynamic_addr)) return false;

	dynamic_size = dynamic_phdr->p_filesz;

	if (dynamic_addr < image_low ||
		dynamic_addr > image_high ||
		dynamic_size > image_high - dynamic_addr) return false;

	dynamic = (const Elf64_Dyn *)(uintptr_t)dynamic_addr;

	for (uint64_t i = 0; i < dynamic_size / sizeof(Elf64_Dyn); i++) {
		switch (dynamic[i].d_tag) {
			case DT_NULL:
				terminated = true;
				goto dynamic_done;

			case DT_NEEDED:
				return false;

			case DT_RELA:
				if (have_rela) return false;
				rela_addr = dynamic[i].d_un.d_ptr;
				have_rela = true;
				break;

			case DT_RELASZ:
				if (have_relasz) return false;
				rela_size = dynamic[i].d_un.d_val;
				have_relasz = true;
				break;

			case DT_RELAENT:
				if (have_relaent) return false;
				rela_ent = dynamic[i].d_un.d_val;
				have_relaent = true;
				break;

			default:
				break;
		}
	}

	dynamic_done:

	if (!terminated) return false;
	if (!have_rela && !have_relasz) return true;
	if (!have_rela || !have_relasz || !have_relaent) return false;
	if (rela_ent != sizeof(Elf64_Rela)) return false;
	if (rela_size % rela_ent != 0) return false;
	if (rela_size == 0) return true;

	if (add_overflow_u64(rela_addr, load_bias, &rela_addr)) return false;
	if (rela_addr < image_low ||
		rela_addr > image_high ||
		rela_size > image_high - rela_addr) return false;

	rela = (const Elf64_Rela *)(uintptr_t)rela_addr;

	for (uint64_t i = 0; i < rela_size / rela_ent; i++) {
		uint64_t target;
		uint64_t value;
		uint32_t type = ELF64_R_TYPE(rela[i].r_info);

		switch (type) {
			case ELF_R_NONE:
				break;

			case ELF_R_RELATIVE:
				if (ELF64_R_SYM(rela[i].r_info) != 0)
					return false;

				if (add_overflow_u64(load_bias,
					rela[i].r_offset, &target)) return false;

				if (target < image_low ||
					target > image_high ||
					sizeof(uint64_t) > image_high - target)
					return false;

				if (rela[i].r_addend < 0) {
					uint64_t magnitude =
						(uint64_t)(-(rela[i].r_addend + 1)) + 1;

					if (magnitude > load_bias) return false;
					value = load_bias - magnitude;
				} else {
					if (add_overflow_u64(load_bias,
						(uint64_t)rela[i].r_addend,
						&value)) return false;
				}

				*(uint64_t *)(uintptr_t)target = value;
				break;

			default:
				return false;
		}
	}

	return true;
}

static bool elf_map_segments(const Elf64_Ehdr *hdr, size_t file_size, uint64_t load_bias, uint64_t image_low, uint64_t image_high) {
	uint64_t page;

	if (image_high <= image_low)
		return false;

	for (page = image_low; page < image_high; ) {
		uint64_t map_flags = VMM_MAP_NX | VMM_MAP_WRITE;
		bool mapped = false;
		bool writable = false;
		bool executable = false;

		for (uint16_t i = 0; i < hdr->e_phnum; i++) {
			const Elf64_Phdr *phdr = elf_phdr(hdr, i);
			uint64_t segment_start;
			uint64_t segment_end;
			uint64_t segment_page_start;
			uint64_t segment_page_end;

			if (phdr->p_type != PT_LOAD) continue;
			if (phdr->p_memsz == 0) continue;
			if (!elf_check_load_segment(phdr, file_size)) return false;
			if (add_overflow_u64(phdr->p_vaddr, load_bias, &segment_start)) return false;
			if (add_overflow_u64(segment_start, phdr->p_memsz, &segment_end)) return false;
			if (!align_up(segment_end, &segment_page_end)) return false;

			segment_page_start = align_down(segment_start);

			if (page < segment_page_start || page >= segment_page_end) continue;
			if (phdr->p_flags & PF_W) writable = true;
			if (phdr->p_flags & PF_X) executable = true;

			map_flags |= elf_segment_flags(phdr->p_flags);

			mapped = true;
		}

		if (writable && executable) return false;

		if (mapped) {
			uint64_t phys;

			phys = pmm_alloc();

			if (phys == 0) return false;
			if (!vmm_map_page(page, phys, PAGE_SIZE, map_flags)) {
				pmm_free(phys);
				return false;
			}
		}

		if (page > UINT64_MAX - PAGE_SIZE) break;
		page += PAGE_SIZE;
	}

	return true;
}

static bool elf_load_segments(const Elf64_Ehdr *hdr, const uint8_t *file, size_t file_size, uint64_t load_bias, uint64_t image_low) {
	for (uint16_t i = 0; i < hdr->e_phnum; i++) {
		const Elf64_Phdr *phdr = elf_phdr(hdr, i);

		if (!elf_load_segment(phdr, file, file_size, load_bias, image_low)) return false;
	}

	return true;
}

static bool elf_protect_segments(const Elf64_Ehdr *hdr, uint64_t load_bias) {
	const Elf64_Phdr *phdr;

	if (hdr == NULL) return false;

	phdr = (const Elf64_Phdr *)((const uint8_t *)hdr + hdr->e_phoff);

	for (uint16_t i = 0; i < hdr->e_phnum; i++) {
		uint64_t seg_start;
		uint64_t seg_end;
		uint64_t page;
		uint64_t flags;

		if (phdr[i].p_type != PT_LOAD) continue;
		if (phdr[i].p_memsz == 0) continue;

		if (add_overflow_u64(phdr[i].p_vaddr, load_bias, &seg_start))
			return false;

		if (add_overflow_u64(seg_start, phdr[i].p_memsz, &seg_end))
			return false;

		seg_start &= ~(uint64_t)(PAGE_SIZE - 1);
		seg_end = (seg_end + PAGE_SIZE - 1) &
			~(uint64_t)(PAGE_SIZE - 1);

		flags = 0;

		if (phdr[i].p_flags & PF_W)
			flags |= VMM_MAP_WRITE;

		if (!(phdr[i].p_flags & PF_X))
			flags |= VMM_MAP_NX;

		for (page = seg_start; page < seg_end; page += PAGE_SIZE) {
			uint64_t phys;
		
			phys = vmm_get_phys(page);
			if (phys == 0) return false;
		
			if (!vmm_unmap(page)) return false;
			if (!vmm_map_page(page, phys, 4096, flags)) return false;
		}
	}

	return true;
}

uint64_t elf_load_file(const void *file, size_t file_size) {
	if (file == NULL) {
		printf("ELF: file is NULL\n");
		return 0;
	}
	
	const uint8_t *data = (const uint8_t *)file;
	const Elf64_Ehdr *hdr;
	uint64_t image_low = 0;
	uint64_t image_high = 0;
	uint64_t relocated_low = 0;
	uint64_t relocated_high = 0;
	uint64_t image_size = 0;
	uint64_t pages = 0;
	uint64_t virt_base = 0;
	uint64_t load_bias = 0;

	hdr = (const Elf64_Ehdr *)data;

	if (!elf_check_header(hdr, file_size)) {
		printf("ELF: invalid ELF header\n");
		return 0;
	}

	if (hdr->e_type != ET_DYN) {
		printf("ELF: unsupported ELF type: %u\n", hdr->e_type);
		return 0;
	}

	if (!elf_check_phdrs(hdr, file_size)) {
		printf("ELF: invalid program headers\n");
		return 0;
	}

	if (!elf_check_special_segments(hdr)) {
		printf("ELF: unsupported special segment\n");
		return 0;
	}

	if (!elf_check_segment_overlap(hdr, file_size)) {
		printf("ELF: overlapping or invalid PT_LOAD segments\n");
		return 0;
	}

	if (!elf_image_bounds(hdr, file_size, &image_low, &image_high)) {
		printf("ELF: failed to calculate image bounds\n");
		return 0;
	}

	printf("ELF: image bounds: %lx-%lx\n", image_low, image_high);

	image_size = image_high - image_low;

	if (image_size == 0) {
		printf("ELF: image size is zero\n");
		return 0;
	}

	if ((image_size & PAGE_MASK) != 0) {
		printf("ELF: image size is not page aligned: %lx\n", image_size);
		return 0;
	}

	pages = image_size / PAGE_SIZE;

	if (pages == 0) {
		printf("ELF: calculated page count is zero\n");
		return 0;
	}

	printf("ELF: image size: %lx, pages: %lu\n", image_size, pages);

	virt_base = vmm_find_free_pages(pages, true);

	if (virt_base == 0) {
		printf("ELF: vmm_find_free_pages failed for %lu pages\n", pages);
		return 0;
	}

	printf("ELF: virtual base: %lx\n", virt_base);

	if (virt_base < image_low) {
		printf("ELF: virtual base %lx is below image low %lx\n", virt_base, image_low);
		return 0;
	}

	load_bias = virt_base - image_low;

	printf("ELF: load bias: %lx\n", load_bias);

	if (add_overflow_u64(image_low, load_bias, &relocated_low)) {
		printf("ELF: relocated low address overflow\n");
		return 0;
	}

	if (add_overflow_u64(image_high, load_bias, &relocated_high)) {
		printf("ELF: relocated high address overflow\n");
		return 0;
	}

	if (relocated_high <= relocated_low) {
		printf("ELF: invalid relocated bounds: %lx-%lx\n", relocated_low, relocated_high);
		return 0;
	}

	if ((relocated_low & PAGE_MASK) != 0) {
		printf("ELF: relocated low is not page aligned: %lx\n", relocated_low);
		return 0;
	}

	if ((relocated_high & PAGE_MASK) != 0) {
		printf("ELF: relocated high is not page aligned: %lx\n", relocated_high);
		return 0;
	}

	if (!elf_entry_valid(hdr, load_bias)) {
		printf("ELF: invalid entry point: %lx\n", hdr->e_entry + load_bias);
		return 0;
	}

	printf("ELF: entry: %lx\n", hdr->e_entry + load_bias);

	if (!elf_map_segments(hdr, file_size, load_bias, relocated_low, relocated_high)) {
		printf("ELF: failed to map segments\n");
		goto fail;
	}

	printf("ELF: segments mapped\n");

	if (!elf_load_segments(hdr, data, file_size, load_bias, relocated_low)) {
		printf("ELF: failed to load segments\n");
		goto fail;
	}

	if (!elf_relocate(hdr, file_size, load_bias,
		relocated_low, relocated_high)) {
		printf("ELF: failed to apply relocations\n");
		goto fail;
	}

	if (!elf_protect_segments(hdr, load_bias)) {
		printf("ELF: failed to apply segment permissions\n");
		goto fail;
	}

	printf("ELF: relocations applied\n");

	printf("ELF: segment permissions applied\n");

	printf("ELF: load successful: entry=%lx base=%lx size=%lx\n", hdr->e_entry + load_bias, relocated_low, image_size);

	return hdr->e_entry + load_bias;

	fail:
	printf("ELF: cleaning up failed load\n");

	for (uint64_t i = 0; i < pages; i++) {
		uint64_t virt;
		uint64_t phys;

		if (i > UINT64_MAX / PAGE_SIZE) {
			printf("ELF: cleanup index overflow at page %lu\n", i);
			break;
		}

		if (i * PAGE_SIZE > UINT64_MAX - relocated_low) {
			printf("ELF: cleanup address overflow at page %lu\n", i);
			break;
		}

		virt = relocated_low + i * PAGE_SIZE;
		if (!vmm_is_page_mapped(virt)) continue;
		phys = vmm_get_phys(virt);

		if (!vmm_unmap(virt)) {
			printf("ELF: failed to unmap page %lx\n", virt);
			continue;
		}

		pmm_free(phys);
	}

	printf("ELF: load failed\n");

	return 0;
}