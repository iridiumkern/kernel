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

static bool sub_underflow_u64(uint64_t a, uint64_t b, uint64_t *result) {
	if (b > a) return true;
	*result = a - b;

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

	if (phdr == NULL) return false;
	if (phdr->p_type != PT_LOAD) return true;
	if (phdr->p_flags & ~(PF_R | PF_W | PF_X)) return false;
	if (phdr->p_filesz > phdr->p_memsz) return false;
	if (!elf_range_valid(file_size, phdr->p_offset, phdr->p_filesz)) return false;
	if (add_overflow_u64(phdr->p_offset, phdr->p_filesz, &file_end)) return false;
	if (add_overflow_u64(phdr->p_vaddr, phdr->p_memsz, &mem_end)) return false;

	if (phdr->p_memsz == 0) return true;

	if (phdr->p_align > 1) {
		if ((phdr->p_align & (phdr->p_align - 1)) != 0) return false;
		if (phdr->p_align < PAGE_SIZE) return false;
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
			case PT_DYNAMIC:
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

static bool elf_load_segment(const Elf64_Phdr *phdr, const uint8_t *file, size_t file_size, uint64_t load_bias, uint64_t image_low, uint64_t phys_base) {
	uint64_t segment_start;
	uint64_t segment_end;
	uint64_t destination_offset;
	uint64_t destination;

	if (phdr->p_type != PT_LOAD) return true;

	if (!elf_check_load_segment(phdr, file_size)) return false;
	if (phdr->p_memsz == 0) return true;
	if (add_overflow_u64(phdr->p_vaddr, load_bias, &segment_start)) return false;
	if (add_overflow_u64(segment_start, phdr->p_memsz, &segment_end)) return false;
	if (segment_start < image_low) return false;

	if (!sub_underflow_u64(segment_start, image_low, &destination_offset)) {
		if (destination_offset > UINT64_MAX - phys_base) return false;
	} else {
		return false;
	}

	destination = phys_base + destination_offset;

	if (phdr->p_memsz > UINT64_MAX - destination) return false;

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

static bool elf_map_segments(const Elf64_Ehdr *hdr, size_t file_size, uint64_t load_bias, uint64_t image_low, uint64_t image_high, uint64_t phys_base) {
	uint64_t page;

	if (image_high <= image_low)
		return false;

	for (page = image_low; page < image_high; ) {
		uint64_t map_flags = VMM_MAP_NX;
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
			if (!elf_check_load_segment( phdr, file_size)) return false;
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
			uint64_t physical_offset;
			uint64_t physical_page;
			if (!sub_underflow_u64(page, image_low, &physical_offset)) {
				return false;
			}

			if (physical_offset > UINT64_MAX - phys_base) return false;
			physical_page = phys_base + physical_offset;

			if (!vmm_map_page(page, physical_page, PAGE_SIZE, map_flags)) return false;
		}

		if (page > UINT64_MAX - PAGE_SIZE) break;
		page += PAGE_SIZE;
	}

	return true;
}

static bool elf_load_segments(const Elf64_Ehdr *hdr, const uint8_t *file, size_t file_size, uint64_t load_bias, uint64_t image_low, uint64_t phys_base) {
	for (uint16_t i = 0; i < hdr->e_phnum; i++) {
		const Elf64_Phdr *phdr = elf_phdr(hdr, i);
		if (!elf_load_segment(phdr, file, file_size, load_bias, image_low, phys_base)) return false;
	}

	return true;
}

bool elf_load_file(const void *file, size_t file_size, uint64_t load_bias, Elf64_Image *image) {
	const uint8_t *data = (const uint8_t *)file;
	const Elf64_Ehdr *hdr;
	uint64_t image_low;
	uint64_t image_high;
	uint64_t relocated_low;
	uint64_t relocated_high;
	uint64_t image_size;
	uint64_t pages;
	uint64_t phys_base;

	if (data == NULL || image == NULL) return false;

	hdr = (const Elf64_Ehdr *)data;

	if (!elf_check_header(hdr, file_size)) return false;
	if (hdr->e_type != ET_EXEC && hdr->e_type != ET_DYN) return false;
	if (!elf_check_phdrs(hdr, file_size)) return false;
	if (!elf_check_special_segments(hdr)) return false;
	if (!elf_check_segment_overlap(hdr, file_size)) return false;
	if (!elf_image_bounds(hdr, file_size, &image_low, &image_high)) return false;
	if (hdr->e_type == ET_EXEC) return false;
	if (add_overflow_u64(image_low, load_bias, &relocated_low)) return false;
	if (add_overflow_u64(image_high, load_bias, &relocated_high)) return false;
	if (relocated_high <= relocated_low) return false;
	if ((relocated_low & PAGE_MASK) != 0) return false;
	if ((relocated_high & PAGE_MASK) != 0) return false;
	if (!elf_entry_valid(hdr, load_bias)) return false;

	image_size = relocated_high - relocated_low;

	if (image_size == 0) return false;
	if ((image_size & PAGE_MASK) != 0) return false;

	pages = image_size / PAGE_SIZE;
	if (pages == 0) return false;

	phys_base = pmm_alloc_pages(pages);
	if (phys_base == 0) return false;

	if (!elf_map_segments(hdr, file_size, load_bias, relocated_low, relocated_high, phys_base)) {
		pmm_free_pages(phys_base, pages);
		return false;
	}

	if (!elf_load_segments(hdr, data, file_size, load_bias, relocated_low, phys_base)) {
		pmm_free_pages(phys_base, pages);
		return false;
	}

	image->entry = hdr->e_entry + load_bias;
	image->load_bias = load_bias;
	image->virt_base = relocated_low;
	image->phys_base = phys_base;
	image->size = image_size;

	return true;
}