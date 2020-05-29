struct list_head {
    struct list_head *prev, *next;
};

#define list_for_each(head, iter) \
    for (iter = (head)->next; iter != (head); iter = iter->next)

static inline void list_init(struct list_head *head) {
    head->prev = head->prev = head;
}

/* insert `node` to the end of the list */
static inline void list_add(struct list_head *head, struct list_head *node) {
    node->next = head;
    node->prev = head->prev;
    head->prev->next = node;
    head->prev = node;
}

/* remove `node` from list */
static inline void list_del(struct list_head *node) {
    node->next->prev = node->prev;
    node->prev->next = node->next; 
}