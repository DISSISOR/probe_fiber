#include "coro.h"

#include <assert.h>

#include "execution_context.h"
#include "stack.h"

_Noreturn static void coro_trampoline(struct Coro* coro);

struct FullContext {
    void *rsp, *rbx, *rbp, *r12, *r13, *r14, *r15;
    void *rip;
};

int coro_init(struct Coro *coro, CoroCode code, struct StackPool *stack_pool, void *arg) {
    coro->stack_pool = stack_pool;
    struct StackView stack = stack_pool_get_stack(stack_pool);
    void *const stack_base = (void*)((char*)stack.stack + stack.size);
    coro->ctx.rsp = stack_base - sizeof(struct FullContext);
    coro->code = code;
    coro->state = CORO_STATE_SUSPENDED;
#if 0
    void *ret_addr = stack_base - 16;
#else
    void *ret_addr = stack_base;
#endif
    * (( void(**)(struct Coro*) ) ret_addr) = coro_trampoline;
    return 0;
}

int coro_resume(struct Coro *coro) {
    struct ExecutionContext current_context;
    execution_context_switch(&current_context, &coro->ctx, coro);
    return coro->state == CORO_STATE_FINISHED;
}

_Noreturn static void coro_trampoline(struct Coro *coro) {
    coro->code(coro);
    coro->state = CORO_STATE_FINISHED;
    struct ExecutionContext current_context;
    // execution_context_switch(&current_context, &coro->ctx, NULL);
    execution_context_switch(&coro->ctx, &current_context, NULL);
    assert(0 && "UNREACHABLE");
}


void coro_yield(struct Coro *coro) {
    coro->state = CORO_STATE_SUSPENDED;
    struct ExecutionContext current_context;
    execution_context_switch(&current_context, &coro->ctx, NULL);
}

