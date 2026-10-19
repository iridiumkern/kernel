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
#include <stdio.hpp>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

using namespace security::memory;

namespace vfs {
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