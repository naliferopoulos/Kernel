#include <ds/list.h>

void list_init(list_node_t* node) 
{
  node->next = node;
  node->prev = node;
}

bool list_is_empty(list_node_t* list) 
{
  return list->next == list ? true : false;
}

void list_insert_front(list_node_t* item, list_node_t* list) 
{
  item->next = list->next;
  item->prev = list;
  list->next->prev = item;
  list->next = item;
}

void list_insert_back(list_node_t* item, list_node_t* list) 
{
  item->next = list;
  item->prev = list->prev;
  list->prev->next = item;
  list->prev = item;
}

void list_append_front(list_node_t* dst, list_node_t* src) 
{
  if(list_is_empty(src))
    return;
  
  src->prev->next = dst->next;
  dst->next->prev = src->prev;
  dst->next = src->next;
  src->next->prev = dst;
  list_init(src);
}

void list_append_back(list_node_t* dst, list_node_t* src) 
{
  if(list_is_empty(src))
    return;
  
  src->next->prev = dst->prev;
  dst->prev->next = src->next;
  dst->prev = src->prev;
  src->prev->next = dst;
  list_init(src);
}

void list_rotate_forward(list_node_t* list) 
{
  if(list_is_empty(list))
    return;
  
  list_node_t *next = list->next;
  list_remove(list);
  list_insert_front(list, next);
}

void list_rotate_backward(list_node_t* list) 
{
  if(list_is_empty(list))
    return;
  
  list_node_t *prev = list->prev;
  list_remove(list);
  list_insert_back(list, prev);
}

void list_remove(list_node_t* item) 
{
  item->prev->next = item->next;
  item->next->prev = item->prev;
  list_init(item);
}

list_node_t* list_pop(list_node_t* list) 
{
  if(list_is_empty(list))
    return NULL;
   
  list_node_t *first = list->next;
  list_remove(first);
  return first;
}
