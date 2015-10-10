#include "toygc.h"

size_t tgc_collect(struct tgc_config *gc)
{
	struct tgc_node *obj, **link;
	struct tgc_node *candidates = 0;
	size_t rc = 0;

	/* stop the world collecter */

	/* mark all objs white */
	for (obj = gc->obj_set; obj; obj = obj->next)
		obj->color = WHITE;

	/* add objects in root set to candidates */
	gc->walk_root_set(gc, &candidates);
	/* depth first search */
	while ((obj = candidates)) {
		candidates = candidates->next_list;
		if (obj->color  == WHITE) {
			/* mark node as black */
			obj->color = BLACK;
			/* add neighbours to candidates */
			gc->walk_obj(obj, &candidates);
			++rc;
		}
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
