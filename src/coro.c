#include "coro.h"
#include "execution_context.h"
#include "stack.h"

_Noreturn static void coro_trampoline();

struct FullContext {
    void *rsp, *rbx, *rbp, *r12, *r13, *r14, *r15;
    void *rip;
};

int coro_init(struct Coro *coro, CoroCode code, struct StackPool *stack_pool, void *arg) {
    coro->stack_pool = stack_pool;
    struct StackView stack = stack_pool_get_stack(stack_pool);
    void *const stack_base = (char*)stack.stack + stack.size;
    coro->ctx.rsp = stack_base - sizeof(struct FullContext);
    void *ret_addr = stack_base - 16;
    * (( void(**)(void) ) ret_addr) = coro_trampoline;
    return 0;
}

int coro_resume(struct Coro *coro) {
    struct ExecutionContext current_context;
    execution_context_switch(&current_context, &coro->ctx, NULL);
}

_Noreturn static void coro_trampoline(struct Coro *coro) {
}


void coro_yield(struct Coro *coro) {
    coro->state = CORO_STATE_SUSPENDED;
}
