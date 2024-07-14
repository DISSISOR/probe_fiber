#include "stack.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>

static void stack_dyn_arr_add(struct StackDynArr *arr, struct StackView stack) {
    if (arr->cap <= arr->cnt) {
        arr->cap *= 2;
        struct StackView  *const tmp = realloc(arr->stacks, arr->cap * sizeof(arr->stacks[0]));
        if (!tmp) {
            perror("allocation");
            exit(1);
        }
        arr->stacks = tmp;
    }
    arr->stacks[arr->cnt++] = stack;
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

struct StackPool stack_pool_init(size_t initial_cnt, size_t initial_cap, size_t stack_size) {
    assert(initial_cap >= initial_cnt);
    struct StackPool ret_val = {
        .vacant = {
            .cap = initial_cap,
            .cnt = initial_cnt,
        }
    };
    ret_val.vacant.stacks = malloc(sizeof(ret_val.vacant.stacks[0]) * ret_val.vacant.cap);
    if (ret_val.vacant.stacks == NULL) {
        perror("allocation");
        exit(1);
    }
    for (size_t i=0; i < ret_val.vacant.cnt; i++) {
        ret_val.vacant.stacks[i] = stack_view_alloc(stack_size);
    }
    return ret_val;
}

void stack_pool_deinit(struct StackPool *pool) {
    for (size_t i=0; i<pool->vacant.cnt; i++) {
        munmap(pool->vacant.stacks[i].stack, pool->vacant.stacks[i].size);
    }
    free(pool->vacant.stacks);
}
