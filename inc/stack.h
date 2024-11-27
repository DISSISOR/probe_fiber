#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <stddef.h>

struct StackView {
    void *stack;
    size_t size;
};

struct StackDynArr {
    struct StackView *stacks;
    size_t cnt;
    size_t cap;
};

struct StackPool {
    struct StackDynArr vacant;
};

int stack_pool_init(struct StackPool *pool, size_t initial_cnt, size_t initial_cap, size_t stack_size);
void stack_pool_deinit(struct StackPool *pool);
struct StackView stack_pool_get_stack(struct StackPool *pool);
void stack_pool_return_stack(struct StackPool *pool, struct StackView stack);

#endif // STACK_H_INCLUDED
