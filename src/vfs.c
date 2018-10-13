#include "stdfunctions.h"
#include "vfs.h"

VFSNode vfs_root = { NULL, NULL, NULL, NULL, "" };

VFSNode *vfs_node_create(VFSNode *parent, const char *name, void *driver_data) {
	if (!parent) parent = &vfs_root;
	VFSNode *child = calloc(sizeof(VFSNode), 1);
	if (child) {
		child->name = strdup(name);
		child->parent = parent;
		child->next = parent->first_child;
		if (child->next) {
			child->next->prev = child;
		}
		parent->first_child = child;
		child->driver_data = driver_data;
	}
	return child;
}

static VFSStream *vfs_node_link_open(VFSNode *node) {
	return vfs_node_open((VFSNode*) node->driver_data);
}

VFSNode *vfs_node_create_link(VFSNode *parent, const char *name, VFSNode *node) {
	VFSNode *link_node = vfs_node_create(parent, name, node);
	if (link_node) {
		link_node->open = vfs_node_link_open;
	}
	return link_node;
}

VFSNode *vfs_node_find_child(VFSNode *parent, const char *name) {
	if (!parent) parent = &vfs_root;
	VFSNode *child = parent->first_child;
	while (child) {
		if (strcmp(name, child->name) == 0) {
			return child;
		}
		child = child->next;
	}
	return NULL;
}

VFSStream *vfs_node_open(VFSNode *node) {
	return node && node->open ? node->open(node) : NULL;
}

VFSStream *vfs_stream_create(VFSNode *node, void *driver_data) {
	VFSStream *stream = calloc(sizeof(VFSNode), 1);
	if (stream) {
		stream->node = node;
		stream->driver_data = driver_data;
	}
	return stream;
}

void vfs_stream_close(VFSStream *stream) {
	if (stream) {
		if (stream->close) {
			stream->close(stream);
		}
		free(stream);
	}
}

size_t vfs_stream_read(VFSStream *stream, void *buffer, size_t count) {
	if (stream && stream->read) {
		return stream->read(stream, buffer, count);
	}
	return 0;
}

void vfs_stream_seek(VFSStream *stream, VFSStreamOffset offset) {
	if (stream && stream->seek) {
		stream->seek(stream, offset);
	}
}
