#include "stdfunctions.h"
#include "vfs.h"
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

typedef struct DiskDriverData {
	DriveParameters params;
} DiskDriverData;

typedef struct DiskStreamDriverData {
	VFSStreamOffset offset;
} DiskStreamDriverData;

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

static void bios_drive_close(VFSStream *stream) {
	free(stream->driver_data);
}

static size_t bios_drive_read(VFSStream *stream, void *buffer, size_t count) {
	DiskStreamDriverData *data = (DiskStreamDriverData*) stream->driver_data;
	DiskDriverData *node_data = (DiskDriverData*) stream->node->driver_data;
	size_t i;
	for (i = 0; i < count; i += 512) {
		if (!pm_functions->read_sector(&node_data->params, data->offset / 512, (char*) buffer + i)) {
			break;
		}
		data->offset += 512;
	}
	return i;
}

static void bios_drive_seek(VFSStream *stream, VFSStreamOffset offset) {
	DiskStreamDriverData *data = (DiskStreamDriverData*) stream->driver_data;
	data->offset = offset;
}

static VFSStream *bios_drive_open(VFSNode *node) {
	DiskStreamDriverData *data = malloc(sizeof(DiskStreamDriverData));
	if (data) {
		data->offset = 0;
		VFSStream *stream = vfs_stream_create(node, data);
		if (stream) {
			stream->close = bios_drive_close;
			stream->read = bios_drive_read;
			stream->seek = bios_drive_seek;
			return stream;
		}
		free(data);
	}
	return NULL;
}

static void process_detected_drive(const DriveParameters *params) {
	char buffer[16];
	if (params->id & 0x80) {
		snprintf(buffer, sizeof(buffer), "hd%u", params->id & ~0x80);
	} else {
		snprintf(buffer, sizeof(buffer), "fd%u", params->id);
	}
	DiskDriverData *data = malloc(sizeof(DiskDriverData));
	if (data) {
		memcpy(&data->params, params, sizeof(*params));
		VFSNode *node = vfs_node_create(NULL, buffer, data);
		if (node) {
			node->open = bios_drive_open;
			if (params->id == pm_functions->get_boot_drive_id()) {
				vfs_node_create_link(NULL, "boot_drive", node);
			}
		} else {
			free(data);
		}
	}
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
	main();
}
