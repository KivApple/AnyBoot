#include "stdfunctions.h"
#include "vfs.h"

static void dump_vfs(VFSNode *parent, int level) {
	VFSNode *child = parent->first_child;
	while (child) {
		for (int i = 0; i < level; i++) {
			puts("  ");
		}
		printf("%s\r\n", child->name);
		dump_vfs(child, level + 1);
		child = child->next;
	}
}

int main(void) {
	printf("%zu\r\n", get_free_memory_size());
	//printf("Hello world!\r\n");
	dump_vfs(&vfs_root, 0);
	VFSStream *stream = vfs_node_open(vfs_node_find_child(NULL, "boot_drive"));
	if (stream) {
		uint8_t buffer[512];
		vfs_stream_seek(stream, 0 * 512);
		size_t count = vfs_stream_read(stream, buffer, sizeof(buffer));
		printf("%zu %02x%02x\r\n", count, buffer[510], buffer[511]);
		vfs_stream_close(stream);
	}
	printf("%zu\r\n", get_free_memory_size());
	return 0;
}
