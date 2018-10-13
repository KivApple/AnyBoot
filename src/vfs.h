#pragma once

typedef struct VFSNode VFSNode;
typedef struct VFSStream VFSStream;
typedef uint64_t VFSStreamOffset;

struct VFSStream {
	VFSNode *node;
	void (*seek)(VFSStream *stream, VFSStreamOffset offset);
	size_t (*read)(VFSStream *stream, void *buffer, size_t count);
	void (*close)(VFSStream *stream);
	void *driver_data;
};

struct VFSNode {
	VFSNode *parent;
	VFSNode *next;
	VFSNode *prev;
	VFSNode *first_child;
	char *name;
	VFSStream *(*open)(VFSNode *node);
	VFSStream *(*open_file)(VFSNode *node, const char *path);
	void *driver_data;
};

extern VFSNode vfs_root;

VFSNode *vfs_node_create(VFSNode *parent, const char *name, void *driver_data);
VFSNode *vfs_node_create_link(VFSNode *parent, const char *name, VFSNode *node);
VFSNode *vfs_node_find_child(VFSNode *parent, const char *name);
VFSNode *vfs_node_find_by_path(const char *path, const char **rest);

VFSStream *vfs_node_open(VFSNode *node);
VFSStream *vfs_node_open_file(VFSNode *node, const char *path);

VFSStream *vfs_stream_create(VFSNode *node, void *driver_data);
VFSStream *vfs_stream_open(const char *path);
void vfs_stream_close(VFSStream *stream);
size_t vfs_stream_read(VFSStream *stream, void *buffer, size_t count);
void vfs_stream_seek(VFSStream *stream, VFSStreamOffset offset);
