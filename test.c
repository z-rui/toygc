#include <stdio.h>
#include <stdlib.h>

#include "toygc.h"

#ifndef container_of
# define container_of(x, t, f) (t *) ((char *) (x) - offsetof(t, f))
#endif

int alloc_calls, free_calls;

static void *my_alloc(size_t size)
{
	++alloc_calls;
	return malloc(size);
}

static void my_free(void *mem)
{
	++free_calls;
	free(mem);
}

struct ll_node {
	int value;
	struct ll_node *next;
	struct tgc_node gc;
};

struct ll_root_set {
	struct tgc_config gc;
	struct ll_node *root;
};

static
void finalize(struct tgc_node *obj)
{
	struct ll_node *ll;

	ll = container_of(obj, struct ll_node, gc);
	my_free(ll);
}

static
void walk_obj(struct tgc_node *obj, struct tgc_node **list, int black)
{
	struct ll_node *ll, *next;

	ll = container_of(obj, struct ll_node, gc);
	next = ll->next;

	if (next && next->gc.color != black) {
		tgc_add_list(&next->gc, list, black);
	}
}

static
void walk_root_set(struct tgc_config *gc, struct tgc_node **list)
{
	struct ll_root_set *rs;

	rs = container_of(gc, struct ll_root_set, gc);
	if (rs->root) {
		tgc_add_list(&rs->root->gc, list, gc->current_color);
	}
}

static
struct ll_node *new_node(struct tgc_config *gc, int value)
{
	struct ll_node *ll;

	ll = (struct ll_node *) my_alloc(sizeof (struct ll_node));
	ll->value = value;
	ll->next = 0;
	tgc_add(gc, &ll->gc);
	return ll;
}

static
void dump(struct ll_root_set *rs)
{
	struct ll_node *node;
	int first = 1;

	for (node = rs->root; first || node != rs->root; node = node->next) {
		first = 0;
		printf("%d\n", node->value);
	}
}

static
void generate_list(struct ll_root_set *rs, size_t n)
{
	struct ll_node *node;
	int i;

	/* assume n > 0 */
	node = rs->root = new_node(&rs->gc, 0);
	for (i = 1; i < n; i++)
		node = node->next = new_node(&rs->gc, i);
	node->next = rs->root; /* circular list */
}

static
void truncate_list(struct ll_root_set *rs)
{
	struct ll_node **tortoise, *hare;
	int first = 1;

	hare = rs->root;
	tortoise = &rs->root;

	while ((first || hare != rs->root) && hare->next != rs->root) {
		first = 0;
		hare = hare->next->next;
		tortoise = &(*tortoise)->next;
	}
	*tortoise = rs->root;
}

#define N 1000000

int main()
{
	struct ll_root_set rs = {
		{finalize, walk_obj, walk_root_set}
	};
	int live;

	generate_list(&rs, N);

	do {
#if N <= 10
		dump(&rs);
#endif
		live = tgc_collect(&rs.gc);
		printf("live objects: %u\n", (unsigned) live);
		truncate_list(&rs);
	} while (live > 1);

	rs.root = 0;
	tgc_collect(&rs.gc);

	printf("%d allocs\n%d frees\n", alloc_calls, free_calls);
	return 0;
}
