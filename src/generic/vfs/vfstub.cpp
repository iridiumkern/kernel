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
#include <stdio.hpp>
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
			return rootnode;
		}

		/**
		 * @brief Sets up the RAMFS
		 * 
		 * @return true Init worked
		 * @return false Init failed
		 */
		bool ramfs_init(void) {
			if (init) return false;

			rootnode = (struct node*)malloc(sizeof(struct node));
			if (rootnode == NULL) {
				return false;
			}
			// Nukes the rootnode to nothing for now.
			memset((void*)rootnode, 0, sizeof(struct node));
			
			rootnode->ignorename = true;
			rootnode->is_folder = true; // Root node aka the root directory is a folder
			rootnode->data = NULL;
			rootnode->size = 0;

			rootnode->parentnode = NULL;
			rootnode->childnoderoot = NULL;
			rootnode->nextnode = NULL;
			
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
			if (parent == NULL || name == NULL ||
				((uint64_t)is_folder != 0 && (uint64_t)is_folder != 1)) {
				return NULL;
			}
		
			if (!parent->is_folder || namelen == 0 || namelen >= sizeof(((struct node*)0)->name)) {
				return NULL;
			}
		
			if (!range_is_mapped((uintptr_t)name, namelen)) {
				return NULL;
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
		 * @brief Tokenizes a string
		 * 
		 * @param str 
		 * @param tokens 
		 * @return size_t 
		 */
		size_t tokenize(const char *str, char **tokens) {
			if (str == NULL || tokens == NULL)
				return 0;
		
			size_t token_count = 0;
			const char *start = str;
		
			for (const char *p = str; ; ++p) {
				if (*p != '/' && *p != '\0')
					continue;
			
				size_t len = (size_t)(p - start);
			
				if (len > 0) {
					char *token = (char*)malloc(len + 1);
				
					if (token == NULL) {
						for (size_t i = 0; i < token_count; ++i)
							free(tokens[i]);
					
						return 0;
					}
				
					memcpy(token, start, len);
					token[len] = '\0';
					tokens[token_count++] = token;
				}
			
				if (*p == '\0')
					return token_count;
			
				start = p + 1;
			}
		
			return token_count;
		}

		/**
		 * @brief Takes the path, finds its file, then finds its parent.
		 * 
		 * @param name The path/file/folder name.
		 * @return struct node* The node that it is the parent of.
		 */
		struct node* backtrack_from_path(const char* name) {
			if (name[0] == '/' && name[1] == '\0') {
				// This is the root directory, return rootnode
				return rootnode;
			}

			// Failed to find any response.
			return NULL;
		}
	}
}

namespace vfs {
	bool vfs_init(void) {
		// Sets up the RAMFS.
		if (!ramfs::ramfs_init()) return false;

		struct ramfs::node* rootnode = ramfs::getrootnode();

		// Create a folder in the root.
		// Used for kernel logs and other info.
		struct ramfs::node *kernelinfo = ramfs::create_node(rootnode, "/kernelinfo", 12, true);

		kernelinfo->info.uid = 1;

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