#include "stdfunctions.h"
#include "vfs.h"

VFSNode vfs_root = { NULL, NULL, NULL, NULL, "", NULL, NULL, NULL };

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

VFSNode *vfs_node_find_by_path(const char *path, const char **rest) {
	VFSNode *node = &vfs_root;
	char *slash;
	do {
		if (*path != '/') {
			return NULL;
		} else {
			path++;
		}
		slash = strchr(path, '/');
		if (slash) {
			*slash = '\0';
		}
		VFSNode *child = vfs_node_find_child(node, path);
		if (slash) {
			*slash = '/';
		}
		if (!child && rest) {
			*rest = path;
			return node;
		}
		path = slash;
		node = child;
	} while (node && slash);
	if (rest) {
		*rest = NULL;
	}
	return node;
}

VFSStream *vfs_node_open(VFSNode *node) {
	return node && node->open ? node->open(node) : NULL;
}

VFSStream *vfs_node_open_file(VFSNode *node, const char *path) {
	return node && node->open_file ? node->open_file(node, path) : NULL;
}

VFSStream *vfs_stream_create(VFSNode *node, void *driver_data) {
	VFSStream *stream = calloc(sizeof(VFSNode), 1);
	if (stream) {
		stream->node = node;
		stream->driver_data = driver_data;
	}
	return stream;
}

VFSStream *vfs_stream_open(const char *path) {
	const char *rest;
	VFSNode *node = vfs_node_find_by_path(path, &rest);
	if (rest) {
		return vfs_node_open_file(node, rest);
	} else {
		return vfs_node_open(node);
	}
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
