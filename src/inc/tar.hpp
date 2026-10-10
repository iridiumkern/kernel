#pragma once

#include <stdint.h>

/**
 * @brief Header of the tar stuff
 * 
 */
struct tar_header {
	char filename[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char padding[12];
};

/**
 * @brief Wrapper around the tar header
 * 
 */
struct tar_wrapper {
	struct tar_header h;
	uintptr_t address;
	struct tar_wrapper *next;
};

/**
 * @brief Gets a file from the tarfile
 * 
 * @param name The file name to check
 * @return struct tar_wrapper* The tarheader and address of the files data
 */
struct tar_wrapper *tar_getfile(const char* name);