#include "stack.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

static int stack_dyn_arr_add(struct StackDynArr *arr, struct StackView stack) {
    if (arr->cap <= arr->cnt) {
        arr->cap *= 2;
        struct StackView  *const tmp = realloc(arr->stacks, arr->cap * sizeof(arr->stacks[0]));
        if (!tmp) {
            return ENOMEM;
        }
        arr->stacks = tmp;
    }
    arr->stacks[arr->cnt++] = stack;
    return 0;
}

static inline struct StackView stack_view_alloc(size_t size) {
    struct StackView ret_val;
    ret_val.size = size;
    ret_val.stack = mmap(/* address */ NULL, /* size */ ret_val.size,
    /* protection */PROT_READ | PROT_WRITE,
    /* flags */MAP_PRIVATE | MAP_ANONYMOUS,
    /* fd */-1, /* offset */ 0);
    return ret_val;
}

struct StackView stack_pool_get_stack(struct StackPool *pool) {
    if (pool->vacant.cnt == 0) {
        return stack_view_alloc(2 * 1024);
    }
    return pool->vacant.stacks[--(pool->vacant.cnt)];
}

void stack_pool_return_stack(struct StackPool *pool, struct StackView stack) {
    stack_dyn_arr_add(&pool->vacant, stack);
}

int stack_pool_init(struct StackPool *pool, size_t initial_cnt, size_t initial_cap, size_t stack_size) {
    assert(initial_cap >= initial_cnt);
    memset(pool, 0, sizeof(pool[0]));
    pool->vacant.cap = initial_cap;
    pool->vacant.cnt = initial_cnt;
    pool->vacant.stacks = malloc(sizeof(pool->vacant.stacks[0]) * pool->vacant.cap);
    if (pool->vacant.stacks == NULL) {
        perror("allocation");
        return ENOMEM;
    }
    for (size_t i=0; i < pool->vacant.cnt; i++) {
        pool->vacant.stacks[i] = stack_view_alloc(stack_size);
    }
    return 0;
}

void stack_pool_deinit(struct StackPool *pool) {
    for (size_t i=0; i<pool->vacant.cnt; i++) {
        munmap(pool->vacant.stacks[i].stack, pool->vacant.stacks[i].size);
    }
    free(pool->vacant.stacks);
}
