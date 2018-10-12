#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define SECTOR_SIZE 512

static const char bootsector_data[] = {
#include "bootsector_data.h"
};

#define BOOT_START_SECTOR_OFFSET 0x1AA
#define BOOT_SECTOR_COUNT_OFFSET 0x1B2

static void setup_boot_sector(char *buffer, uint64_t param_start_sector, uint16_t param_sector_count) {
	for (size_t i = 0; i < SECTOR_SIZE; i++) {
		if (i >= 0 && i < 3 || i >= 90 && i < 436 || i >= 510 && i < 512) {
			buffer[i] = bootsector_data[i];
		}
	}
	buffer[BOOT_START_SECTOR_OFFSET] = param_start_sector & 0xFF;
	buffer[BOOT_START_SECTOR_OFFSET + 1] = (param_start_sector >> 8) & 0xFF;
	buffer[BOOT_START_SECTOR_OFFSET + 2] = (param_start_sector >> 16) & 0xFF;
	buffer[BOOT_START_SECTOR_OFFSET + 3] = (param_start_sector >> 24) & 0xFF;
	buffer[BOOT_START_SECTOR_OFFSET + 4] = (param_start_sector >> 32) & 0xFF;
	buffer[BOOT_START_SECTOR_OFFSET + 5] = (param_start_sector >> 40) & 0xFF;
	buffer[BOOT_START_SECTOR_OFFSET + 6] = (param_start_sector >> 48) & 0xFF;
	buffer[BOOT_START_SECTOR_OFFSET + 7] = (param_start_sector >> 56) & 0xFF;
	buffer[BOOT_SECTOR_COUNT_OFFSET] = param_sector_count & 0xFF;
	buffer[BOOT_SECTOR_COUNT_OFFSET + 1] = (param_sector_count >> 8) & 0xFF;
}

static long query_stream_size(FILE *stream) {
	fseek(stream, 0, SEEK_END);
	long result = ftell(stream);
	fseek(stream, SEEK_SET, 0);
	return result;
}

static void read_sector(FILE *device_file, uint64_t sector, char *buffer) {
	fseek(device_file, sector * SECTOR_SIZE, SEEK_SET);
	if (fread(buffer, SECTOR_SIZE, 1, device_file) != 1) {
		memset(buffer, 0, SECTOR_SIZE);
		fprintf(stderr, "Failed to read sector %llu\n", sector);
	}
}

static void write_sector(FILE *device_file, uint64_t sector, const char *buffer) {
	fseek(device_file, sector * SECTOR_SIZE, SEEK_SET);
	if (fwrite(buffer, SECTOR_SIZE, 1, device_file) != 1) {
		fprintf(stderr, "Failed to write sector %llu\n", sector);
	}
}

static void install_boot_sector(FILE *device_file, uint64_t param_start_sector, uint16_t param_sector_count) {
	char buffer[SECTOR_SIZE];
	read_sector(device_file, 0, buffer);
	setup_boot_sector(buffer, param_start_sector, param_sector_count);
	write_sector(device_file, 0, buffer);
}

static long copy_sectors(FILE *device_file, FILE *input_file, uint64_t start_sector, long bytes_count) {
	uint64_t last_sector = start_sector + (bytes_count + SECTOR_SIZE - 1) / SECTOR_SIZE - 1;
	for (uint64_t sector = start_sector; sector <= last_sector; sector++) {
		char buffer[SECTOR_SIZE];
		long offset = ftell(input_file);
		if (sector == last_sector && bytes_count % SECTOR_SIZE) {
			long count = fread(buffer, 1, SECTOR_SIZE, input_file);
			memset(buffer + count, 0, SECTOR_SIZE - count);
		} else {
			fread(buffer, SECTOR_SIZE, 1, input_file);
		}
		write_sector(device_file, sector, buffer);
	}
	return last_sector - start_sector + 1;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s disk.img boot.bin\n");
		return 1;
	}
	FILE *input_file = fopen(argv[2], "rb");
	if (!input_file) {
		fprintf(stderr, "Failed to open %s: %s\n", argv[2], strerror(errno));
		return 1;
	}
	FILE *device_file = fopen(argv[1], "rb+");
	if (!device_file) {
		fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(errno));
		return 1;
	}
	long input_size = query_stream_size(input_file);
	long sector_count = copy_sectors(device_file, input_file, 1, input_size);
	install_boot_sector(device_file, 1, sector_count);
	return 0;
}
