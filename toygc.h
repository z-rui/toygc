#ifndef TOYGC_H
#define TOYGC_H

#include <stddef.h>

struct tgc_node {
	int color;
	struct tgc_node *next, *next_list;
};

struct tgc_config {
	void (*finalize)(struct tgc_node *);
	void (*walk_obj)(struct tgc_node *, struct tgc_node **);
	struct tgc_node *(*walk_root_set)(void **);
	struct tgc_node *obj_set;
	int current_color;
};

extern size_t tgc_collect(struct tgc_config *, void *);
extern void tgc_add(struct tgc_config *, struct tgc_node *);

#endif /* TOYGC_H */
