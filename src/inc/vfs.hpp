#pragma once

#include <stdint.h>
#include <stddef.h>

namespace vfs {
	// Holds some basic info such as UID/owner
	// Also holds a pointer to private data for the filesystem layer to hold FS specific data in
	typedef struct open_file_t {
		void* priv;
		uint64_t uid;
	}open_file_t;

	struct file_descriptor {
		uint64_t fd;
		uint64_t owner_pid;
		uint8_t flags; // 255 possible flags, none exist yet, all files are R/W

		uint64_t fsid;
		open_file_t *file;

		struct file_descriptor *next;
	};

	typedef struct fs_ops {
	    // File operations

		// Returns pointer to data based on its path
		open_file_t* (*open)(const char* path);
	    bool (*read)(open_file_t* filedata, void* buffer, size_t size);
	    bool (*write)(open_file_t* filedata, const void* buffer, size_t size);
		bool (*close)(open_file_t* filedata);

	    // Creation
	    bool (*makefile)(const char* path);
	    bool (*makefolder)(const char* path);

	    // Destruction
	    bool (*destroyfile)(const char* path);
	    bool (*destroyfolder)(const char* path);
	} fs_ops;

	struct mounted_fs {
		uint64_t id;
		bool isramfs;
		fs_ops *ops;
		mounted_fs* next;
	};

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

	/**
	 * @brief Registers a FS in the VFS
	 * 
	 * @param fsid Filesystem ID.
	 * @param ops Filesystem ops
	 * @return true It mounted
	 * @return false It did not mo
	 */
	bool registerfs(uint64_t *fsid, fs_ops *ops);
}