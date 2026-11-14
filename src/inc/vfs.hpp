#pragma once

#include <stdint.h>
#include <stddef.h>

namespace vfs {
	// Filesystem-private open state. The VFS owns this object and passes it
	// back to the filesystem callbacks until close().
	typedef struct open_file_t {
		void* priv;
		uint64_t uid;
	} open_file_t;

	struct file_descriptor {
		uint64_t fd;
		uint64_t owner_pid;
		uint8_t flags;

		uint64_t fsid;
		open_file_t* file;

		file_descriptor* next;
	};

	typedef struct fs_ops {
		open_file_t* (*open)(const char* path);
		bool (*read)(open_file_t* filedata, void* buffer, size_t size);
		bool (*write)(open_file_t* filedata, const void* buffer, size_t size);
		bool (*close)(open_file_t* filedata);

		bool (*makefile)(const char* path);
		bool (*makefolder)(const char* path);
		bool (*destroyfile)(const char* path);
		bool (*destroyfolder)(const char* path);
	} fs_ops;

	struct mounted_fs {
		uint64_t id;
		bool isramfs;
		fs_ops* ops;
		mounted_fs* next;
	};

	// Initializes the VFS and registers the built-in RAMFS.
	bool vfs_init(void);

	// Opens a path on the filesystem identified by fsid.
	bool open(uint64_t* fdret, const char* path, uint64_t fsid);

	bool read(uint64_t fd, void* buf, size_t bufsize);
	bool write(uint64_t fd, void* buf, size_t bufsize, bool user);

	bool close(uint64_t fd);

	// Registers a filesystem. The fs_ops object must remain valid for the
	// lifetime of the registration.
	bool registerfs(uint64_t* fsid, fs_ops* ops);
}
