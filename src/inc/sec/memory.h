#pragma once

#include <stdbool.h>
#include <stdint.h>

// Will be automatically aligned to system page size if applicable on said architecture/port
bool range_is_mapped(uintptr_t start, uintptr_t end);