#pragma once

#include "stdfunctions.h"

void mark_memory_as_free(uint64_t base, uint64_t length);
void *malloc(size_t count);
void free(void *ptr);
