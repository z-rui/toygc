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
void walk_root_set(struct tgc_config *gc, struct tgc_node **list)
{
	struct ll_root_set *rs;

	rs = container_of(gc, struct ll_root_set, gc);
	if (rs->root) {
		rs->root->gc.next_list = *list;
		*list = &rs->root->gc;
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

	for (node = rs->root; node; node = node->next) {
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
	} while (live);

	printf("%d allocs\n%d frees\n", alloc_calls, free_calls);
	return 0;
}
