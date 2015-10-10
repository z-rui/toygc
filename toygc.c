#include "toygc.h"

size_t tgc_collect(struct tgc_config *gc)
{
	struct tgc_node *obj, **link;
	struct tgc_node *candidates = 0;
	size_t rc = 0;

	/* stop the world collecter */

	/* swap color */
	gc->current_color ^= 1;

	/* add objects in root set to candidates */
	gc->walk_root_set(gc, &candidates);

	/* mark */
	while ((obj = candidates)) {
		candidates = candidates->next_list;
		/* add neighbours to candidates */
		gc->walk_obj(obj, &candidates, gc->current_color);
		++rc;
	}

	/* sweep */
	for (link = &gc->obj_set; (obj = *link); ) {
		if (obj->color != gc->current_color) {
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
	obj->color = gc->current_color;
	obj->next = gc->obj_set;
	gc->obj_set = obj;
}

void tgc_add_list(struct tgc_node *obj, struct tgc_node **list, int color)
{
	obj->color = color;
	obj->next_list = *list;
	*list = obj;
}
