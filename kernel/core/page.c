// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "page.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "fmem.h"

#define PAGE_SHIFT 10
#define PAGE_MAGIC 0xDEAD5A5A
#define MAX_ORDER 7

struct fnode {
    struct fnode far *prev;
    struct fnode far *next;
};

static inline void flist_init(struct fnode far *node) {
    node->prev = node->next = node;
}

static inline bool flist_is_empty(struct fnode far *node) {
    return (node->next == node);
}

static inline void flist_delete(struct fnode far *item) {
    item->next->prev = item->prev;
    item->prev->next = item->next;
    item->prev = item->next = 0;
}

static inline void flist_push(struct fnode far *list, struct fnode far *item) {
    item->next = list->next;
    item->prev = list;
    list->next->prev = item;
    list->next = item;
}

static inline void flist_queue(struct fnode far *list, struct fnode far *item) {
    item->next = list;
    item->prev = list->prev;
    list->prev->next = item;
    list->prev = item;
}

static inline struct fnode far *flist_pop(struct fnode far *list) {
    if (flist_is_empty(list)) {
        return NULL;
    }
    struct fnode far *node = list->next;
    flist_delete(node);
    return node;
}

#define flist_pop_type(list, type, elem)           \
    ({                                             \
        type far *__t = (type far *)0;             \
        struct fnode far *__nod = flist_pop(list); \
        if (__nod) {                               \
            __t = fcontainerof(__nod, type, elem); \
        }                                          \
        __t;                                       \
    })

struct free_area {
    // Node in the free list.
    struct fnode node;
    // Helper to check if a buddy is in the free list.
    uint32_t magic;
};

// Set of free areas.
struct fnode free_areas[MAX_ORDER];

static void_fptr_t page_buddy(void_fptr_t addr, size_t order) {
    uint32_t ptr = (uint32_t)addr;
    uint32_t base = ptr & 0xffff0000;
    return (void_fptr_t)(base + ((1 << (order + PAGE_SHIFT)) ^ (ptr - base)));
}

static void page_push(void_fptr_t addr, size_t order) {
    struct free_area far *new = addr;
    new->magic = PAGE_MAGIC;
    flist_push(&free_areas[order], &new->node);
}

static void page_queue(void_fptr_t addr, size_t order) {
    struct free_area far *new = addr;
    new->magic = PAGE_MAGIC;
    flist_queue(&free_areas[order], &new->node);
}

static struct free_area far *page_pop(size_t order) {
    struct free_area far *area =
        flist_pop_type(&free_areas[order], struct free_area, node);
    if (area) {
        area->magic = 0;
    }
    return area;
}

struct page *page_alloc(size_t order) {
    if (order >= MAX_ORDER) {
        printf("mem: %u exceeding maximum order\n", order);
        return NULL;
    }

    size_t index = order;
    struct free_area far *zone = page_pop(index);
    while (!zone && index < MAX_ORDER) {
        printf("mem: zone=%lx order=%u prev=%lx next=%lx\n", (uint32_t)zone,
               index, (uint32_t)free_areas[index].prev,
               (uint32_t)free_areas[index].next);
        zone = page_pop(++index);
    }

    if (!zone) {
        // No area found, no more memory available.
        printf("mem: no area found\n");
        return NULL;
    }

    while (--index > order) {
        page_push(page_buddy(zone, index), index);
    }

    struct page *p = calloc(1, sizeof(*p));
    p->addr = zone;
    p->order = order;
    return p;
}

static inline bool page_is_before(void_fptr_t a, void_fptr_t b) {
    return (uint32_t)a <= (uint32_t)b;
}

void page_free(struct page *p) {
    void_fptr_t addr;
    size_t order;
    struct free_area far *buddy;

    if (!p) {
        return;
    }

    addr = p->addr;
    order = p->order;
    free(p);

    buddy = page_buddy(addr, order);
    if (buddy->magic != PAGE_MAGIC) {
        // The buddy is not free, just insert the block.
        page_push(addr, order);
        return;
    }

    // TODO: once the block are double list linked.
    while (order < MAX_ORDER) {
        addr = page_is_before(addr, buddy) ? addr : buddy;
        buddy = page_buddy(addr, ++order);
    }
    page_push(addr, order);

    free(p);
}

void page_add(void_fptr_t area, size_t order) {
    if (order >= MAX_ORDER) {
        return;
    }
    page_queue(area, order);
}

void page_initialize(void) {
    for (size_t i = 0; i < MAX_ORDER; i++) {
        flist_init(&free_areas[i]);
    }
}