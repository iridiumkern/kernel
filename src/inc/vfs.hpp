#pragma once

#include <stdint.h>
#include <stddef.h>

namespace vfs {
    bool write(uint64_t fd, void* buf, size_t bufsize, bool user);
}