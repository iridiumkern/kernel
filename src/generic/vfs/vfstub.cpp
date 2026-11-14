/**
 * @file vfstub.cpp
 * @brief Virtual filesystem layer and built-in RAMFS.
 *
 */

#include <sec/memory.hpp>
#include <scheduler.hpp>
#include <stdlib.hpp>
#include <panic.hpp>
#include <stdio.hpp>
#include <debug.hpp>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <vfs.hpp>

using namespace security::memory;

namespace {
	namespace ramfs {
		struct nodeinfo {
			uint64_t uid;
		};

		struct node {
			char name[512];
			size_t namesize;

			bool ignorename;
			bool is_folder;

			struct nodeinfo info;

			struct node* parentnode;
			struct node* childnoderoot;
			struct node* nextnode;

			unsigned char* data;
			size_t size;
		};

		struct node* rootnode = NULL;
		bool init = false;

		struct node* create_node(struct node* parent, const char* name,
			size_t namelen, bool is_folder) {
			if (!init || parent == NULL || name == NULL)
				return NULL;

			if (!parent->is_folder || namelen == 0 ||
				namelen >= sizeof(((struct node*)0)->name))
				return NULL;

			if (!range_is_mapped((uintptr_t)name, namelen))
				return NULL;

			for (size_t i = 0; i < namelen; ++i) {
				if (name[i] == '/' || name[i] == '\0')
					return NULL;
			}

			struct node* existing = parent->childnoderoot;
			while (existing != NULL) {
				if (existing->namesize == namelen &&
					memcmp(existing->name, name, namelen) == 0)
					return NULL;
				existing = existing->nextnode;
			}

			struct node* newnode = (struct node*)malloc(sizeof(struct node));
			if (newnode == NULL)
				return NULL;

			memset(newnode, 0, sizeof(struct node));
			newnode->parentnode = parent;
			newnode->is_folder = is_folder;
			memcpy(newnode->name, name, namelen);
			newnode->name[namelen] = '\0';
			newnode->namesize = namelen;

			if (parent->childnoderoot == NULL) {
				parent->childnoderoot = newnode;
			} else {
				struct node* tail = parent->childnoderoot;
				while (tail->nextnode != NULL)
					tail = tail->nextnode;
				tail->nextnode = newnode;
			}
			return newnode;
		}

		bool destroy_node(struct node* target) {
			if (!init || target == NULL || target == rootnode)
				return false;

			struct node* parent = target->parentnode;
			if (parent == NULL)
				return false;

			struct node* current = parent->childnoderoot;
			struct node* previous = NULL;
			while (current != NULL && current != target) {
				previous = current;
				current = current->nextnode;
			}
			if (current == NULL)
				return false;

			if (previous == NULL)
				parent->childnoderoot = target->nextnode;
			else
				previous->nextnode = target->nextnode;

			struct node* child = target->childnoderoot;
			while (child != NULL) {
				struct node* next = child->nextnode;
				destroy_node(child);
				child = next;
			}

			if (target->data != NULL)
				free(target->data);
			free(target);
			return true;
		}

		size_t tokenize(const char* str, char*** tokens) {
			const size_t max_path_len = 4096;
			if (tokens == NULL)
				return 0;
			*tokens = NULL;
			if (str == NULL)
				return 0;

			size_t pathlen = 0;
			while (pathlen < max_path_len) {
				if (!range_is_mapped((uintptr_t)str + pathlen, 1))
					return 0;
				if (str[pathlen] == '\0')
					break;
				++pathlen;
			}
			if (pathlen == max_path_len)
				return 0;

			size_t count = 0;
			size_t capacity = 0;
			const char* start = str;

			for (const char* p = str; ; ++p) {
				if (*p != '/' && *p != '\0')
					continue;

				size_t len = (size_t)(p - start);
				if (len > 0) {
					char* token = (char*)malloc(len + 1);
					if (token == NULL)
						goto error;
					memcpy(token, start, len);
					token[len] = '\0';

					if (count == capacity) {
						size_t new_capacity = capacity ? capacity * 2 : 4;
						if (new_capacity < capacity ||
							new_capacity > (size_t)-1 / sizeof(**tokens)) {
							free(token);
							goto error;
						}
						char** new_tokens =
							(char**)malloc(new_capacity * sizeof(*new_tokens));
						if (new_tokens == NULL) {
							free(token);
							goto error;
						}
						if (count > 0)
							memcpy(new_tokens, *tokens,
								count * sizeof(*new_tokens));
						free(*tokens);
						*tokens = new_tokens;
						capacity = new_capacity;
					}
					(*tokens)[count++] = token;
				}

				if (*p == '\0')
					return count;
				start = p + 1;
			}

		error:
			for (size_t i = 0; i < count; ++i)
				free((*tokens)[i]);
			free(*tokens);
			*tokens = NULL;
			return 0;
		}

		struct node* backtrack_from_path(const char* name) {
			if (!init || rootnode == NULL || name == NULL)
				return NULL;

			if (name[0] == '/' && name[1] == '\0')
				return rootnode;
			if (name[0] == '\0')
				return NULL;

			char** tokens = NULL;
			size_t count = tokenize(name, &tokens);
			if (count == 0 || tokens == NULL)
				return NULL;

			struct node* current = rootnode;
			for (size_t i = 0; i < count; ++i) {
				struct node* child = current->childnoderoot;
				struct node* found = NULL;
				while (child != NULL) {
					if (child->namesize == strlen(tokens[i]) &&
						memcmp(child->name, tokens[i], child->namesize) == 0) {
						found = child;
						break;
					}
					child = child->nextnode;
				}
				if (found == NULL) {
					current = NULL;
					break;
				}
				if (i + 1 < count && !found->is_folder) {
					current = NULL;
					break;
				}
				current = found;
			}

			for (size_t i = 0; i < count; ++i)
				free(tokens[i]);
			free(tokens);
			return current;
		}

		// Ops-table callbacks begin here. The node representation and path
		// traversal helpers above are intentionally left as-is.

		vfs::open_file_t* open(const char* path) {
			struct node* target = backtrack_from_path(path);
			if (target == NULL || target->is_folder)
				return NULL;

			vfs::open_file_t* state =
				(vfs::open_file_t*)malloc(sizeof(vfs::open_file_t));
			if (state == NULL)
				return NULL;

			state->priv = target;
			state->uid = target->info.uid;
			return state;
		}

		bool read(vfs::open_file_t* filedata, void* buffer, size_t size) {
			if (filedata == NULL || filedata->priv == NULL)
				return false;
			if (size == 0)
				return true;
			if (buffer == NULL || !range_is_mapped((uintptr_t)buffer, size))
				return false;

			struct node* target = (struct node*)filedata->priv;
			if (target->is_folder || target->size < size)
				return false;
			if (target->size != 0 && target->data == NULL)
				return false;

			memcpy(buffer, target->data, size);
			return true;
		}

		bool write(vfs::open_file_t* filedata, const void* buffer, size_t size) {
			if (filedata == NULL || filedata->priv == NULL)
				return false;
			if (size != 0 &&
				(buffer == NULL || !range_is_mapped((uintptr_t)buffer, size)))
				return false;

			struct node* target = (struct node*)filedata->priv;
			if (target->is_folder)
				return false;

			unsigned char* replacement = NULL;
			if (size != 0) {
				replacement = (unsigned char*)malloc(size);
				if (replacement == NULL)
					return false;
				memcpy(replacement, buffer, size);
			}

			if (target->data != NULL)
				free(target->data);
			target->data = replacement;
			target->size = size;
			return true;
		}

		bool makefile(const char* path) {
			if (!init || path == NULL)
				return false;

			size_t length = 0;
			while (length < 4096 &&
				range_is_mapped((uintptr_t)path + length, 1) &&
				path[length] != '\0')
				++length;
			if (length == 0 || length == 4096 ||
				!range_is_mapped((uintptr_t)path + length, 1))
				return false;

			size_t base = length;
			while (base > 0 && path[base - 1] != '/')
				--base;
			const char* name = path + base;
			size_t name_length = length - base;
			if (name_length == 0 || name_length >= 512)
				return false;

			char parent_path[4096];
			if (base == 0 || (base == 1 && path[0] == '/')) {
				parent_path[0] = '/';
				parent_path[1] = '\0';
			} else {
				size_t parent_length = base - 1;
				memcpy(parent_path, path, parent_length);
				parent_path[parent_length] = '\0';
			}

			struct node* parent = backtrack_from_path(parent_path);
			if (parent == NULL || !parent->is_folder)
				return false;
			return create_node(parent, name, name_length, false) != NULL;
		}

		bool makefolder(const char* path) {
			if (!init || path == NULL)
				return false;

			size_t length = 0;
			while (length < 4096 &&
				range_is_mapped((uintptr_t)path + length, 1) &&
				path[length] != '\0')
				++length;
			if (length == 0 || length == 4096 ||
				!range_is_mapped((uintptr_t)path + length, 1))
				return false;

			size_t base = length;
			while (base > 0 && path[base - 1] != '/')
				--base;
			const char* name = path + base;
			size_t name_length = length - base;
			if (name_length == 0 || name_length >= 512)
				return false;

			char parent_path[4096];
			if (base == 0 || (base == 1 && path[0] == '/')) {
				parent_path[0] = '/';
				parent_path[1] = '\0';
			} else {
				size_t parent_length = base - 1;
				memcpy(parent_path, path, parent_length);
				parent_path[parent_length] = '\0';
			}

			struct node* parent = backtrack_from_path(parent_path);
			if (parent == NULL || !parent->is_folder)
				return false;
			return create_node(parent, name, name_length, true) != NULL;
		}

		bool destroyfile(const char* path) {
			struct node* target = backtrack_from_path(path);
			if (target == NULL || target->is_folder)
				return false;
			return destroy_node(target);
		}

		bool destroyfolder(const char* path) {
			struct node* target = backtrack_from_path(path);
			if (target == NULL || !target->is_folder ||
				target == rootnode || target->childnoderoot != NULL)
				return false;
			return destroy_node(target);
		}

		bool close(vfs::open_file_t* filedata) {
			if (filedata == NULL)
				return false;
			free(filedata);
			return true;
		}

		vfs::fs_ops ops = {
			.open = open,
			.read = read,
			.write = write,
			.close = close,
			.makefile = makefile,
			.makefolder = makefolder,
			.destroyfile = destroyfile,
			.destroyfolder = destroyfolder,
		};

		bool ramfs_init(void) {
			if (init || rootnode != NULL)
				return false;

			struct node* newroot = (struct node*)malloc(sizeof(struct node));
			if (newroot == NULL)
				return false;

			memset(newroot, 0, sizeof(struct node));
			newroot->ignorename = true;
			newroot->is_folder = true;
			rootnode = newroot;
			init = true;

			uint64_t ramfs_id = 0;
			if (!vfs::registerfs(&ramfs_id, &ops)) {
				free(rootnode);
				rootnode = NULL;
				init = false;
				return false;
			}

			struct node* kernelinfo =
				create_node(rootnode, "kernelinfo", 10, true);
			if (kernelinfo == NULL)
				kpanic("FAILED TO CREATE KERNELINFO!");
			kernelinfo->info.uid = 1;

			if (backtrack_from_path("kernelinfo") != kernelinfo)
				kpanic("RAMFS BACKTRACK CODE FAILED!");

			struct node* testlog =
				create_node(kernelinfo, "test.log", 8, false);
			if (testlog == NULL)
				kpanic("FAILED TO MAKE TESTLOG!");

			const char* teststr = "Test Log!\n";
			size_t teststrlen = strlen(teststr);
			uint8_t* data = (uint8_t*)malloc(teststrlen);
			if (data == NULL)
				kpanic("FAILED TO ALLOCATE TESTLOG DATA!");

			memcpy(data, teststr, teststrlen);
			testlog->data = data;
			testlog->size = teststrlen;

			struct node* log = backtrack_from_path("kernelinfo/test.log");
			if (log == NULL || log != testlog)
				kpanic("Backtrack code failed when grabbing test log!");

			debug::hexdump(log->data, log->size);
			return true;
		}
	}
}

namespace vfs {
	namespace {
		mounted_fs* filesystems = NULL;
		file_descriptor* descriptors = NULL;
		uint64_t next_filesystem_id = 0;
		uint64_t next_descriptor_id = 1;
		bool initialized = false;

		thread_t* current_thread(void) {
			thread_t* thread = get_current_thread();
			if (thread == NULL || thread->owner == NULL)
				return NULL;
			return thread;
		}

		file_descriptor* find_descriptor(uint64_t id) {
			for (file_descriptor* item = descriptors; item != NULL;
				item = item->next) {
				if (item->fd == id)
					return item;
			}
			return NULL;
		}

		mounted_fs* find_filesystem(uint64_t id) {
			for (mounted_fs* item = filesystems; item != NULL;
				item = item->next) {
				if (item->id == id)
					return item;
			}
			return NULL;
		}

		bool descriptor_owned_by_current(const file_descriptor* desc) {
			thread_t* thread = current_thread();
			return desc != NULL && thread != NULL &&
				desc->owner_pid == thread->owner->pid;
		}

		bool valid_user_buffer(const void* buffer, size_t size, bool writable) {
			if (size == 0)
				return true;
			if (buffer == NULL)
				return false;

			uintptr_t address = (uintptr_t)buffer;
			if (size > (size_t)-1 - address)
				return false;
			if (!range_is_user(address, size) ||
				!range_is_mapped(address, size))
				return false;
			return !writable || range_is_write(address, size);
		}

		bool valid_kernel_or_user_buffer(const void* buffer, size_t size,
			bool user, bool writable) {
			if (!user) {
				if (size == 0)
					return true;
				if (buffer == NULL)
					return false;
				uintptr_t address = (uintptr_t)buffer;
				if (size > (size_t)-1 - address)
					return false;
				if (!range_is_mapped(address, size))
					return false;
				return !writable || range_is_write(address, size);
			}
			return valid_user_buffer(buffer, size, writable);
		}
	}

	bool vfs_init(void) {
		if (initialized)
			return false;

		filesystems = NULL;
		descriptors = NULL;
		next_filesystem_id = 0;
		next_descriptor_id = 1;

		if (!ramfs::ramfs_init())
			return false;

		initialized = true;
		return true;
	}

	bool open(uint64_t* fdret, const char* path, uint64_t fsid) {
		if (!initialized || fdret == NULL || path == NULL)
			return false;
		if (!range_is_mapped((uintptr_t)fdret, sizeof(*fdret)))
			return false;

		mounted_fs* fs = find_filesystem(fsid);
		if (fs == NULL || fs->ops == NULL || fs->ops->open == NULL ||
			fs->ops->close == NULL)
			return false;

		thread_t* thread = current_thread();
		if (thread == NULL || next_descriptor_id == 0 ||
			next_descriptor_id == UINT64_MAX)
			return false;

		open_file_t* file = fs->ops->open(path);
		if (file == NULL)
			return false;

		file_descriptor* desc =
			(file_descriptor*)malloc(sizeof(file_descriptor));
		if (desc == NULL) {
			fs->ops->close(file);
			return false;
		}

		desc->fd = next_descriptor_id++;
		desc->owner_pid = thread->owner->pid;
		desc->flags = 0;
		desc->fsid = fsid;
		desc->file = file;
		desc->next = descriptors;
		descriptors = desc;

		*fdret = desc->fd;
		return true;
	}

	bool read(uint64_t fd, void* buf, size_t bufsize) {
		if (!initialized || fd == 0)
			return false;
		if (!valid_user_buffer(buf, bufsize, true))
			return false;

		file_descriptor* desc = find_descriptor(fd);
		if (desc == NULL || !descriptor_owned_by_current(desc))
			return false;

		mounted_fs* fs = find_filesystem(desc->fsid);
		if (fs == NULL || fs->ops == NULL || fs->ops->read == NULL)
			return false;
		return fs->ops->read(desc->file, buf, bufsize);
	}

	bool write(uint64_t fd, void* buf, size_t bufsize, bool user) {
		if (!initialized)
			return false;

		if (fd == 0) {
			if (!valid_kernel_or_user_buffer(buf, bufsize, user, false))
				return false;
			const char* output = (const char*)buf;
			for (size_t i = 0; i < bufsize; ++i)
				putchar_ft(output[i]);
			return true;
		}

		if (!valid_kernel_or_user_buffer(buf, bufsize, user, false))
			return false;

		file_descriptor* desc = find_descriptor(fd);
		if (desc == NULL || !descriptor_owned_by_current(desc))
			return false;

		mounted_fs* fs = find_filesystem(desc->fsid);
		if (fs == NULL || fs->ops == NULL || fs->ops->write == NULL)
			return false;
		return fs->ops->write(desc->file, buf, bufsize);
	}

	bool close(uint64_t fd) {
		if (!initialized || fd == 0)
			return false;

		file_descriptor* desc = find_descriptor(fd);
		if (desc == NULL || !descriptor_owned_by_current(desc))
			return false;

		mounted_fs* fs = find_filesystem(desc->fsid);
		if (fs == NULL || fs->ops == NULL || fs->ops->close == NULL)
			return false;

		// Do not unlink the descriptor until the filesystem confirms close.
		if (!fs->ops->close(desc->file))
			return false;

		file_descriptor** link = &descriptors;
		while (*link != NULL && *link != desc)
			link = &(*link)->next;
		if (*link == NULL)
			return false;

		*link = desc->next;
		free(desc);
		return true;
	}

	bool registerfs(uint64_t* fsid, fs_ops* ops) {
		if (fsid == NULL || ops == NULL || ops->open == NULL ||
			ops->read == NULL || ops->write == NULL || ops->close == NULL)
			return false;
		if (!range_is_mapped((uintptr_t)fsid, sizeof(*fsid)))
			return false;
		if (next_filesystem_id == UINT64_MAX)
			return false;

		mounted_fs* mount =
			(mounted_fs*)malloc(sizeof(mounted_fs));
		if (mount == NULL)
			return false;

		mount->id = next_filesystem_id++;
		mount->isramfs = false;
		mount->ops = ops;
		mount->next = NULL;

		if (filesystems == NULL) {
			filesystems = mount;
		} else {
			mounted_fs* tail = filesystems;
			while (tail->next != NULL)
				tail = tail->next;
			tail->next = mount;
		}

		*fsid = mount->id;
		return true;
	}
}
