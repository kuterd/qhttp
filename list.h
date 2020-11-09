#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#define containerof(ptr, type, member)					\
  ({									\
    const typeof(((type*)0)->member) *_ptr_value = (ptr);		\
    (type*)((void*)_ptr_value - offsetof(type, member));		\
  })

struct list_head {
  struct list_head *prev, *next;
};

#define LIST_INIT(l) (l)->prev = (l)->next = l;
#define LIST_FOR_EACH(l) for (struct list_head *c = (l)->next; c != (l)->next->prev; c = c->next) 

void list_add_tail(struct list_head *lst, struct list_head *e);
void list_add(struct list_head *lst, struct list_head *e);
void list_deattach(struct list_head *elem);

#endif
