#include "toygc.h"

static
size_t mark_obj(struct tgc_config *gc, struct tgc_node *obj)
{
	struct tgc_node *node;
	struct tgc_node *list;
	size_t rc = 0;

	/* depth first search */

	list = obj;
	obj->next_list = 0;

	while ((node = list)) {
		/* mark node as black */
		node->color = BLACK;
		/* remove node from list */
		list = node->next_list;
		/* add neighbours to list */
		gc->walk_obj(node, &list);
		++rc;
	}
	return rc;
}

size_t tgc_collect(struct tgc_config *gc, void *rs_state)
{
	struct tgc_node *obj, **link;
	size_t rc = 0;

	/* stop the world collecter */

	/* mark all objs white */
	for (obj = gc->obj_set; obj; obj = obj->next)
		obj->color = WHITE;

	/* mark live objects black */
	while ((obj = gc->walk_root_set(&rs_state))) {
		rc += mark_obj(gc, obj);
	}
	/* sweep */
	for (link = &gc->obj_set; (obj = *link); ) {
		if (obj->color == WHITE) {
			/* remove obj from obj_set */
			*link = obj->next;
			/* finalize obj */
			gc->finalize(obj);
		} else {
			link = &obj->next;
		}
	}
	return rc;
}

void tgc_add(struct tgc_config *gc, struct tgc_node *obj)
{
	obj->next = gc->obj_set;
	gc->obj_set = obj;
}
