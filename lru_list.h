#ifndef _LRU_H_
#define _LRU_H_

#include <stddef.h>

typedef struct list_head
{
	struct list_head *prev;
	struct list_head *next;
} list_head_t;

#define container_of(ptr, type, member) ({ \
	void *__mptr = (void *)(ptr); \
	((type *)(__mptr - offsetof(type, member))); })

#define LRU_LIST_ENTRY(ptr, type, member) \
	container_of(ptr, type, member)

#define LRU_LIST_FIRST_ENTRY(ptr, type, member) \
	LRU_LIST_ENTRY((ptr)->next, type, member)

#define LRU_LIST_LAST_ENTRY(ptr, type, member) \
	LRU_LIST_ENTRY((ptr)->prev, type, member)

#define LRU_LIST_NEXT_ENTRY(pos, member) \
	LRU_LIST_ENTRY((pos)->member.next, typeof(*(pos)), member)

#define LRU_LIST_FOREACH(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define LRU_LIST_FOREACH_ENTRY(pos, head, member)                \
	for (pos = LRU_LIST_FIRST_ENTRY(head, typeof(*pos), member); \
		 &pos->member != (head);                                 \
		 pos = LRU_LIST_NEXT_ENTRY(pos, member))

#define LRU_LIST_FOREACH_ENTRY_SAFE(pos, n, head, member)        \
	for (pos = LRU_LIST_FIRST_ENTRY(head, typeof(*pos), member), \
		n = LRU_LIST_NEXT_ENTRY(pos, member);                    \
		 &pos->member != (head);                                 \
		 pos = n, n = LRU_LIST_NEXT_ENTRY(n, member))

static inline void LRU_LIST_INIT(list_head_t *list)
{
	list->prev = list;
	list->next = list;
}

static inline void LRU_LIST_ADD(list_head_t *node, list_head_t *list)
{
	node->next = list->next;
	node->prev = list;
	list->next->prev = node;
	list->next = node;
}

static inline void LRU_LIST_DEL(list_head_t *node)
{
	list_head_t *prev = node->prev;
	list_head_t *next = node->next;
	prev->next = next;
	next->prev = prev;
}

/* LRU policy: last used node move to the head of list */
static inline void LRU_LIST_UPDATE(list_head_t *node, list_head_t *list)
{
	LRU_LIST_DEL(node);
	LRU_LIST_ADD(node, list);
}

#endif // _LRU_H_