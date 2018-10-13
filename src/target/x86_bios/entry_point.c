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

typedef struct DriveParameters {
	uint8_t valid;
	uint8_t drive_count;
	uint8_t id;
	uint8_t edd_support;
	uint32_t spt;
	uint32_t head_count;
	uint32_t track_count;
} DriveParameters;

typedef struct PMFunctionTable {
	NO_RETURN void (*reboot)(void);
	void (*print_str)(const char *s);
	const MemoryMap *(*get_memory_map)(void);
	uint8_t (*get_boot_drive_id)(void);
	bool (*query_drive_parameters)(uint8_t id, DriveParameters *params);
	bool (*read_sector)(const DriveParameters *params, uint64_t index, void *buffer);
} PMFunctionTable;

static PMFunctionTable *pm_functions;

NO_RETURN void reboot(void) {
	pm_functions->reboot();
}

void puts(const char *string) {
	pm_functions->print_str(string);
}

static void init_mm(void) {
	const MemoryMap *mm = pm_functions->get_memory_map();
	for (size_t i = 0; i < mm->entry_count; i++) {
		if ((mm->entries[i].attrs & 1) && (mm->entries[i].type == 1) && (mm->entries[i].base >= 0x100000) && (mm->entries[i].length > 0)) {
			mark_memory_as_free(mm->entries[i].base, mm->entries[i].length);
		}
	}
}

static void process_detected_drive(const DriveParameters *params) {
	printf("valid=%u,id=0x%02x,drive_count=%u,edd_support=%u,spt=%u,head_count=%u,track_count=%u\r\n", 
		   params->valid, params->id, params->drive_count, params->edd_support, params->spt, params->head_count, params->track_count);
}

static void detect_drives_range(uint8_t first_id) {
	uint8_t drive_count = 1;
	for (uint8_t id = first_id; id < first_id + drive_count; id++) {
		DriveParameters params;
		if (pm_functions->query_drive_parameters(id, &params)) {
			drive_count = params.drive_count;
			process_detected_drive(&params);
		}
	}
}

static void detect_drives(void) {
	detect_drives_range(0x00);
	detect_drives_range(0x80);
}

void entry_point(PMFunctionTable *arg_pm_functions) {
	pm_functions = arg_pm_functions;
	init_mm();
	detect_drives();
	
	DriveParameters params;
	pm_functions->query_drive_parameters(pm_functions->get_boot_drive_id(), &params);
	uint8_t *buffer = malloc(512);
	if (buffer) {
		bool result = pm_functions->read_sector(&params, 0, buffer);
		printf("%02x%02x (%i)\r\n", buffer[510], buffer[511], result);
		free(buffer);
	}
	
	main();
}
