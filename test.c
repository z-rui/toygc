#include <stdio.h>
#include <stdlib.h>

#include "toygc.h"

#ifndef container_of
# define container_of(x, t, f) (t *) ((char *) (x) - offsetof(t, f))
#endif

struct ll_node {
	int value;
	struct ll_node *next;
	struct tgc_node gc;
};

struct ll_root_set {
	struct ll_node *root;
};

static
void finalize(struct tgc_node *obj)
{
	struct ll_node *ll;

	ll = container_of(obj, struct ll_node, gc);
	free(ll);
}

static
void walk_obj(struct tgc_node *obj, struct tgc_node **list)
{
	struct ll_node *ll, *next;

	ll = container_of(obj, struct ll_node, gc);
	next = ll->next;

	if (next) {
		next->gc.next_list = *list;
		*list = &next->gc;
	}
}

static
struct tgc_node *walk_root_set(void **rs_state)
{
	struct ll_root_set *rs;

	rs = (struct ll_root_set *) *rs_state;
	if (rs && rs->root) {
		*rs_state = 0;
		return &rs->root->gc;
	}
	return 0;
}

static
struct ll_node *new_node(struct tgc_config *gc, int value)
{
	struct ll_node *ll;

	ll = (struct ll_node *) malloc(sizeof (struct ll_node));
	ll->value = value;
	ll->next = 0;
	tgc_add(gc, &ll->gc);
	return ll;
}

static
void dump(struct ll_root_set *rs)
{
	struct ll_node *node;

	for (node = rs->root; node; node = node->next) {
		printf("%d\n", node->value);
	}
}

static
void generate_list(struct ll_root_set *rs, struct tgc_config *gc, size_t n)
{
	struct ll_node *node;
	int i;

	/* assume n > 0 */
	node = rs->root = new_node(gc, 0);
	for (i = 1; i < n; i++)
		node = node->next = new_node(gc, i);
}

static
void truncate_list(struct ll_root_set *rs)
{
	struct ll_node **tortoise, *hare;

	hare = rs->root;
	tortoise = &rs->root;

	while (hare && hare->next) {
		hare = hare->next->next;
		tortoise = &(*tortoise)->next;
	}
	*tortoise = 0;
}

#define N 1000000

int main()
{
	struct tgc_config gc = {finalize, walk_obj, walk_root_set};
	struct ll_root_set rs;
	size_t live;

	generate_list(&rs, &gc, N);

	do {
		/*dump(&rs);*/
		live = tgc_collect(&gc, &rs);
		printf("live objects: %u\n", (unsigned) live);
		truncate_list(&rs);
	} while (live);

	return 0;
}
