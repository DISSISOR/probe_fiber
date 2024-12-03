#ifndef CORO_H_INCLUDED
#define CORO_H_INCLUDED

#include "execution_context.h"

struct Coro;
struct StackView;
struct StackPool;

typedef void (*CoroCode)(void*, struct Coro *coro);

struct Coro {
    struct ExecutionContext ctx; // own context when suspended OR caller context when resumed
    struct StackPool *stack_pool; // unowned
    enum CORO_STATE {
        CORO_STATE_SUSPENDED,
        CORO_STATE_FINISHED
    } state;
    void *arg;
};

int coro_init(struct Coro *coro,  CoroCode code, struct StackPool *stack_pool, void *arg);
int coro_resume(struct Coro *coro);
void coro_yield(struct Coro *coro);

#endif // CORO_H_INCLUDED
