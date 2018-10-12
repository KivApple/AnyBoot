#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s input-file.bin output-file.txt\n", argv[0]);
		return 0;
	}
	FILE *input_file = strcmp(argv[1], "-") == 0 ? stdin : fopen(argv[1], "rb");
	if (input_file) {
		FILE *output_file = strcmp(argv[2], "-") == 0 ? stdout : fopen(argv[2], "w");
		if (output_file) {
			unsigned long long count = 0;
			while (!feof(input_file)) {
				char byte;
				if (fread(&byte, 1, 1, input_file) > 0) {
					fprintf(output_file, !count || (count % 16 < 15) ? "0x%02X," : "0x%02X,\n", (unsigned char) byte);
					count++;
				} else {
					break;
				}
			}
			if (!count || (count % 16)) {
				fprintf(output_file, "\n");
			}
		} else {
			fprintf(stderr, "Failed to open output file %s: %s\n", argv[2], strerror(errno));
			return 1;
		}
		fclose(input_file);
	} else {
		fprintf(stderr, "Failed to open input file %s: %s\n", argv[1], strerror(errno));
		return 1;
	}
	return 0;
}
