#include "stdfunctions.h"
#include "mm.h"

typedef struct MemoryMapEntry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t attrs;
} MemoryMapEntry;

typedef struct MemoryMap {
	uint32_t entry_count;
	const MemoryMapEntry *entries;
} MemoryMap;

typedef struct PMFunctionTable {
	NO_RETURN void (*reboot)(void);
	void (*print_str)(const char*);
	const MemoryMap *(*get_memory_map)(void);
} PMFunctionTable;

static PMFunctionTable *pm_functions;

NO_RETURN void reboot(void) {
	pm_functions->reboot();
}

void puts(const char *string) {
	pm_functions->print_str(string);
}

void entry_point(PMFunctionTable *arg_pm_functions) {
	pm_functions = arg_pm_functions;
	const MemoryMap *mm = pm_functions->get_memory_map();
	for (size_t i = 0; i < mm->entry_count; i++) {
		if ((mm->entries[i].attrs & 1) && (mm->entries[i].type == 1) && (mm->entries[i].base >= 0x100000) && (mm->entries[i].length > 0)) {
			mark_memory_as_free(mm->entries[i].base, mm->entries[i].length);
		}
	}
	main();
}
