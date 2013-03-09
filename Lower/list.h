#ifndef LIST_H
#define LIST_H

struct list_head {
    struct list_head *prev,*next;
};

#define LIST_HEAD_INIT(name) { &(name), &(name)}

#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do {\
    (ptr)->next = ptr; (ptr)->next = ptr; \
} while(0)



#endif
