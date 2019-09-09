#ifndef LIST_H
#define LIST_H

#include <libk/stdbool.h>
#include <libk/types.h>

// Turn anything into a list by providing a list_node object.

typedef struct list_node {
  struct list_node *next;
  struct list_node *prev;
} list_node_t;

void list_init(list_node_t* node);
bool list_is_empty(list_node_t* list);
void list_insert_front(list_node_t* item, list_node_t* list);
void list_insert_back(list_node_t* item, list_node_t* list);
void list_rotate_forward(list_node_t* list);
void list_rotate_backward(list_node_t* list);
void list_append_front(list_node_t* dst, list_node_t* src);
void list_append_back(list_node_t* dst, list_node_t* src);
void list_remove(list_node_t* item);
list_node_t* list_pop(list_node_t* list);

#define list_foreach(p, lst) for((p)=(lst)->next; (p)!=(lst); (p)=(p)->next)
#define list_foreach_safe(p, tmp, lst) for((p)=(lst)->next, (tmp)=(p)->next; (p)!=(lst); (p)=(tmp), (tmp)=(p)->next)
#define list_foreach_reverse(p, lst) for((p)=(lst)->prev; (p)!=(lst); (p)=(p)->prev)
#define list_foreach_safe_reverse(p, tmp, lst) for((p)=(lst)->prev, (tmp)=(p)->prev; (p)!=(lst); (p)=(tmp), (tmp)=(p)->prev)
#define list_entry container_of
#define list_first(lst) ((lst)->next!=(lst)?(lst)->next:NULL)
#define list_last(lst) ((lst)->prev!=(lst)?(lst)->prev:NULL)
#define list_free_all(list, type, memb, func) do { \
    list_node_t *p, *tmp; \
    list_foreach_safe(p, tmp, (list)) { \
      list_remove(_p); \
      (func)(list_entry(_p, type, memb)); \
    } \
  } while(0)
#endif
