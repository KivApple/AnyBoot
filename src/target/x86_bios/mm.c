#include "mm.h"

#define MEMORY_BLOCK_MAGIC 0xC0DEAA55
#define MEMORY_BLOCK_BASE_SIZE 32

typedef struct MemoryBlock MemoryBlock;
struct MemoryBlock {
	uint32_t magic1;
	MemoryBlock *next;
	MemoryBlock *prev;
	size_t size;
	uint8_t padding[MEMORY_BLOCK_BASE_SIZE - 2 * sizeof(uint32_t) - 3 * sizeof(size_t)];
	uint32_t magic2;
};

static MemoryBlock *first_free_memory_block = NULL;

void mark_memory_as_free(uint64_t base, uint64_t length) {
#ifndef __x86_64__  
	if (base > 0xFFFFFFFF) return;
	if (base + length > 0xFFFFFFFF) {
		length = 0xFFFFFFFF - base;
	}
#endif
	if (length < sizeof(MemoryBlock) * 2) return;
	length = length & ~(MEMORY_BLOCK_BASE_SIZE - 1);
	MemoryBlock *block = (MemoryBlock*) (size_t) base;
	block->magic1 = MEMORY_BLOCK_MAGIC;
	block->magic2 = MEMORY_BLOCK_MAGIC;
	block->next = NULL;
	block->prev = NULL;
	block->size = length - sizeof(MemoryBlock);
	free(block + 1);
}

void *malloc(size_t count) {
	count = (count + MEMORY_BLOCK_BASE_SIZE - 1) & ~(MEMORY_BLOCK_BASE_SIZE - 1);
	MemoryBlock *block = first_free_memory_block;
	if (block) {
		do {
			if (block->magic1 != MEMORY_BLOCK_MAGIC || block->magic2 != MEMORY_BLOCK_MAGIC) {
				panic("PANIC: Heap corruption\r\n");
			}
			if (block->size == count) {
				if (block == first_free_memory_block) {
					first_free_memory_block = block->next;
					if (block == first_free_memory_block) {
						first_free_memory_block = NULL;
					}
				}
				block->prev->next = block->next;
				block->next->prev = block->prev;
				block->next = NULL;
				block->prev = NULL;
				return block + 1;
			} else if (block->size > (count + sizeof(MemoryBlock))) {
				block->size -= count + sizeof(MemoryBlock);
				MemoryBlock *new_block = (MemoryBlock*) ((char*) (block + 1) + block->size);
				new_block->magic1 = MEMORY_BLOCK_MAGIC;
				new_block->magic2 = MEMORY_BLOCK_MAGIC;
				new_block->next = NULL;
				new_block->prev = NULL;
				new_block->size = count;
				return new_block + 1;
			}
			block = block->next;
		} while (block != first_free_memory_block);
	}
	return NULL;
}

void free(void *ptr) {
	if (!ptr) return;
	MemoryBlock *block = ((MemoryBlock*) ptr) - 1;
	if (block->magic1 != MEMORY_BLOCK_MAGIC || block->magic2 != MEMORY_BLOCK_MAGIC) {
		panic("PANIC: Heap corruption\r\n");
	}
	if (block->next || block->prev) {
		panic("PANIC: Double free\r\n");
	}
	if (!first_free_memory_block) {
		block->next = block;
		block->prev = block;
		first_free_memory_block = block;
		return;
	}
	MemoryBlock *block_end = (MemoryBlock*) ((char*) (block + 1) + block->size);
	MemoryBlock *cur_block = first_free_memory_block;
	do {
		MemoryBlock *cur_block_end = (MemoryBlock*) ((char*) (cur_block + 1) + cur_block->size);
		if (block == cur_block_end) {
			cur_block->size += sizeof(MemoryBlock) + block->size;
			block->magic1 = 0;
			block->magic2 = 0;
			cur_block_end = (MemoryBlock*) ((char*) (cur_block + 1) + cur_block->size);
			if (cur_block->next == cur_block_end) {
				cur_block->size += sizeof(MemoryBlock) + cur_block->next->size;
				cur_block->next = cur_block->next->next;
				cur_block->next->prev = cur_block;
			}
			return;
		} else if (cur_block == block_end) {
			block->size += sizeof(MemoryBlock) + cur_block->size;
			block->next = cur_block->next != cur_block ? cur_block->next : block;
			block->prev = cur_block->prev != cur_block ? cur_block->prev : block;
			block->next->prev = block;
			block->prev->next = block;
			if (block_end < first_free_memory_block) {
				first_free_memory_block = block;
			}
			cur_block->magic1 = 0;
			cur_block->magic2 = 0;
			return;
		} else if (cur_block > block_end) {
			break;
		}
		cur_block = cur_block->next;
	} while (cur_block != first_free_memory_block);
	block->next = cur_block;
	block->prev = cur_block->prev;
	block->next->prev = block;
	block->prev->next = block;
	if (block_end < first_free_memory_block) {
		first_free_memory_block = block;
	}
}
