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
        struct node {
            // 512 character long name max.
            // Root node (eg no parent node) makes this the filesystem name
            // Can be ignored if flag is set
            // ignorename flag is only respected if its the root node.
            char name[512];
            bool ignorename;
            bool is_folder;
        
            struct node *parentnode;
            struct node *childnoderoot;
            struct node *nextnode;
        
            unsigned char *data; // Only applies to files
            size_t size; // Size of data, file only. Otherwise set to 0
        };

        struct node* rootnode = NULL;
        bool init = false;

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

        // A stub for now.
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
    }
}

namespace vfs {
    void vfs_init(void) {
        // Sets up the RAMFS.
        ramfs::ramfs_init();
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