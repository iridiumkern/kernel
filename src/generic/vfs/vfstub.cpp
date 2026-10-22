/**
 * @file vfstub.cpp
 * @author Apixeldev
 * @brief A simple VFS abstraction (stub for other filesystems, smart eh?), WIP!
 * @version 0.1
 * @date 2026-10-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */

// Right now we only implement stubs for the public facing functions.
// These are very simple, but will be expanded into a proper VFS later.
// The VFS is going to not be UNIX like and instead inspired by windows.

#include <sec/memory.hpp>
#include <stdlib.hpp>
#include <panic.hpp>
#include <stdio.hpp>
#include <debug.hpp>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

using namespace security::memory;

namespace {
	// A simple RAMFS
	// Used to model the VFS around
	namespace ramfs {
		// Going to be expanded over time as the kernel gets new features.
		// This should represent the "cutting edge" of what the VFS can do
		// If the VFS can do it, its because of the RAMFS.
		// So warning, here be dragons, bad code, and implementations that likely WILL NOT work.
		// You have been warned.

		/*
		Creators notes!
		The RAMFS is really simple, its pretty much just nodes, and a linked list, with some backtracking, the rest is planned to be handeled by a VFS layer.
		Abilities communications is also planned to be done but hell if I know lol.

		Some explanation about using UID 1 instead of UID 0!
		UID 0 is root, it handles basically everything, however the kernel needs a way to say "This is owned by the kernel, not the user" and stop standard userland (outside of PID0) from touching it, so UID1 was created (mostly in my head)
		Its entire point is to hide away a bunch of different things so that the kernel has some form of userland presence, that is the point of PID0, to start the init binary and to do what the kernel needs that is too risky to shove into RING0
		But this is all theory right now since I barely even have an API.
		More notes to come!

		P.S. please dont touch this code unless you are willing to fight god, I have spent a lot of time messing with this already and fought so many senseless bugs lol. The node shit in the RAMFS is weird as hell ngl.
		But eventually all will be abstracted into my own version of fs_ops.
		-- 2026-10-20 APixelDev
		*/

		/**
		 * @brief Provides info about a node outside of "is it a folder" and such things.
		 * 
		 */
		struct nodeinfo {
			uint64_t uid;
		};

		/**
		 * @brief RAMFS file/folder node. Will be expanded with time.
		 * 
		 */
		struct node {
			// 512 character long name max.
			// Root node (eg no parent node) makes this the filesystem name
			// Can be ignored if flag is set
			// ignorename flag is only respected if its the root node.
			char name[512];
			size_t namesize;

			bool ignorename;
			bool is_folder;

			struct nodeinfo info;

			struct node *parentnode;
			struct node *childnoderoot;
			struct node *nextnode;

			unsigned char *data; // Only applies to files, pointer
			size_t size; // Size of data, file only. Otherwise set to 0
		};

		struct node* rootnode = NULL;
		bool init = false;

		/**
		 * @brief Gets the root node of the RAMFS
		 * 
		 * @return struct node* The RAMFS root node.
		 */
		struct node* getrootnode(void) {
			if (!init) return NULL;

			return rootnode;
		}

		/**
		 * @brief Sets up the RAMFS
		 * 
		 * @return true Init worked
		 * @return false Init failed
		 */
		bool ramfs_init(void) {
			if (init || rootnode != NULL) return false;

			struct node* newroot = (struct node*)malloc(sizeof(struct node));
			if (newroot == NULL) {
				return false;
			}

			memset(newroot, 0, sizeof(struct node));

			newroot->namesize = 0;
			newroot->ignorename = true;
			newroot->is_folder = true; // Root node aka the root directory is a folder
			newroot->data = NULL;
			newroot->size = 0;

			newroot->parentnode = NULL;
			newroot->childnoderoot = NULL;
			newroot->nextnode = NULL;

			rootnode = newroot;
			init = true;
			return true;
		}

		/**
		 * @brief Create a node object
		 * 
		 * @param parent The parent node, can ONLY be a folder.
		 * @param name The name of the new file/folder
		 * @param namelen The length of the name
		 * @param is_folder Is it a folder
		 * @return struct node* The new node.
		 */
		struct node* create_node(struct node* parent, const char *name, size_t namelen, bool is_folder) {
			if (!init || parent == NULL || name == NULL) {
				return NULL;
			}

			if (!parent->is_folder || namelen == 0 || namelen >= sizeof(((struct node*)0)->name)) {
				return NULL;
			}

			if (!range_is_mapped((uintptr_t)name, namelen)) {
				return NULL;
			}

			// A node name cannot contain a path separator or an embedded null byte.
			for (size_t i = 0; i < namelen; ++i) {
				if (name[i] == '/' || name[i] == '\0') {
					return NULL;
				}
			}

			// Nodes in the same directory must have unique names.
			struct node* existing = parent->childnoderoot;
			while (existing != NULL) {
				if (existing->namesize == namelen &&
					memcmp(existing->name, name, namelen) == 0) {
					return NULL;
				}

				existing = existing->nextnode;
			}

			struct node* newnode = (struct node*)malloc(sizeof(struct node));
			if (newnode == NULL) {
				return NULL;
			}

			memset(newnode, 0, sizeof(struct node));

			newnode->parentnode = parent;
			newnode->is_folder = is_folder;
			newnode->size = 0;
			newnode->data = NULL;

			memcpy(newnode->name, name, namelen);
			newnode->name[namelen] = '\0';
			newnode->namesize = namelen;

			// Only link the node after all validation and initialization succeeds.
			if (parent->childnoderoot == NULL) {
				parent->childnoderoot = newnode;
			} else {
				struct node* appendnode = parent->childnoderoot;

				while (appendnode->nextnode != NULL) {
					appendnode = appendnode->nextnode;
				}

				appendnode->nextnode = newnode;
			}

			return newnode;
		}

		/**
		 * @brief Tokenizes a string, tokens seperated by '/' character
		 * 
		 * @param str The string to tokenize
		 * @param tokens The tokens
		 * @return size_t The amount of tokens found
		 */
		size_t tokenize(const char *str, char ***tokens) {
			const size_t max_path_len = 4096;

			if (tokens == NULL)
				return 0;

			*tokens = NULL;

			if (str == NULL)
				return 0;

			// Find the path length without reading unmapped memory.
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
			const char *start = str;

			for (const char *p = str; ; ++p) {
				if (*p != '/' && *p != '\0')
					continue;

				size_t len = (size_t)(p - start);

				if (len > 0) {
					char *token = (char*)malloc(len + 1);

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

						char **new_tokens = (char**)malloc(new_capacity * sizeof(*new_tokens));

						if (new_tokens == NULL) {
							free(token);
							goto error;
						}

						if (count > 0)
							memcpy(new_tokens, *tokens, count * sizeof(*new_tokens));

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

		/**
		 * @brief Takes a path and finds the node matching it.
		 * 
		 * @param name The path/file/folder name.
		 * @return struct node* The matching node, or NULL if not found.
		 */
		struct node* backtrack_from_path(const char* name) {
			if (!init || rootnode == NULL || name == NULL) {
				return NULL;
			}

			// This is the root directory, return rootnode.
			if (name[0] == '/' && name[1] == '\0') {
				return rootnode;
			}

			// An empty path does not identify a node.
			if (name[0] == '\0') {
				return NULL;
			}

			char** tokens = NULL;
			size_t count = tokenize(name, &tokens);

			if (count == 0 || tokens == NULL) {
				return NULL;
			}

			// Search from the root namespace downwards until finding the node.
			// Userland might want to optimize this somehow with a service to solve repeated seraches
			// No idea though. Honestly this is prolly a "it works but its slow as hell", stupid simple but not the fastest lol.
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

				// Every intermediate component must be a folder.
				if (i + 1 < count && !found->is_folder) {
					current = NULL;
					break;
				}

				current = found;
			}

			// Free the allocated path tokens.
			for (size_t i = 0; i < count; ++i)
				free(tokens[i]);
			free(tokens);

			return current;
		}
	}
}

namespace vfs {
	bool vfs_init(void) {
		// Sets up the RAMFS.
		if (!ramfs::ramfs_init()) return false;

		struct ramfs::node* rootnode = ramfs::getrootnode();
		if (rootnode == NULL) {
			kpanic("FAILED TO GET RAMFS ROOT NODE!");
			return false;
		}

		// Create a folder in the root.
		// Used for kernel logs and other info.
		struct ramfs::node *kernelinfo = ramfs::create_node(rootnode, "kernelinfo", 10, true);
		if (kernelinfo == NULL) {
			kpanic("FAILED TO CREATE KERNELINFO!");
			return false;
		}

		kernelinfo->info.uid = 1;

		// Sanity test.
		if (ramfs::backtrack_from_path("kernelinfo") != kernelinfo) {
			kpanic("RAMFS BACKTRACK CODE FAILED!");
			return false;
		}

		// Create a small log file!
		struct ramfs::node *testlog = ramfs::create_node(kernelinfo, "test.log", 8, false);
		if (testlog == NULL) {
			kpanic("FAILED TO MAKE TESTLOG!");
			return false;
		}

		const char *teststr = "Test Log!\n";
		size_t teststrlen = strlen(teststr);

		uint8_t *data = (uint8_t*)malloc(teststrlen);
		if (data == NULL) {
			kpanic("FAILED TO ALLOCATE TESTLOG DATA!");
			return false;
		}

		memcpy(data, teststr, teststrlen);
		testlog->data = data;
		testlog->size = teststrlen;

		// Backtrack to find it and print out its log data
		struct ramfs::node *log = ramfs::backtrack_from_path("kernelinfo/test.log");
		if (log == NULL || log != testlog) {
			kpanic("Backtrack code failed when grabbing test log!");
			return false;
		}

		debug::hexdump(log->data, log->size);

		return true;
	}

	bool write(uint64_t fd, void* buf, size_t bufsize, bool user) {
		// Print to STDOUT
		if (fd == 0) {
			if (range_is_mapped((uintptr_t)buf, bufsize)) {
				// Check if the range is meant to be user memory
				if (user && !range_is_user((uintptr_t)buf, bufsize)) {
					return false;
				}
				const char* printbuf = (const char*)buf;
				for (uint64_t i = 0; i < bufsize; i++) {
					putchar_ft(*printbuf++);
				}

				return true;
			}
		}

		return false;
	}
}