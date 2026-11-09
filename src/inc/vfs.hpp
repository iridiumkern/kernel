#pragma once

#include <stdint.h>
#include <stddef.h>

namespace vfs {
	/**
	 * @brief Sets up the VFS
	 * 
	 * @return true The VFS was able to be setup
	 * @return false The VFS was inable to be setup
	 */
	bool vfs_init(void);

	/**
	 * @brief Opens a file
	 * 
	 * @param fdret The File Descriptor ID
	 * @param path Path to the file
	 * @param fsid The filesystem ID (0, 1, 2, 3, etc)
	 * @return true File was opened
	 * @return false File was not opened
	 */
	bool open(uint64_t *fdret, const char* path, uint64_t fsid);

	/**
	 * @brief Reads a file
	 * 
	 * @param fd The files descriptor
	 * @param buf The buffer to read out to
	 * @param bufsize The size of the buffer
	 * @return true It was properly read and put into the buffer
	 * @return false It was not read and put into the buffer
	 */
	bool read(uint64_t fd, void* buf, size_t bufsize);

	/**
	 * @brief Writes to a file
	 * 
	 * @param fd The file being written to
	 * @param buf The buffer to read
	 * @param bufsize The size of the buffer being written
	 * @param user Is the buffer from the userland
	 * @return true Buffer was written
	 * @return false Buffer was not written
	 */
	bool write(uint64_t fd, void* buf, size_t bufsize, bool user);

	/**
	 * @brief Closes a file
	 * 
	 * @param fd The file to close
	 * @return true The file was closed
	 * @return false The file was not closed
	 */
	bool close(uint64_t fd);
}