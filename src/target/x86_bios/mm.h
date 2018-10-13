#pragma once

#include "stdfunctions.h"

void mark_memory_as_free(uint64_t base, uint64_t length);

void *malloc(size_t count);
void *calloc(size_t size, size_t count);
void free(void *ptr);

size_t get_free_memory_size(void);
void debug_dump_heap(void);
