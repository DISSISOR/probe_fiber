#include "fiber.h"

#include <stdlib.h>
#include <sys/mman.h>

#include "execution_context.h"
#include "scheduler.h"

void fiber_yield(void) {
    struct Scheduler *scheduler = get_current_scheduler();
    scheduler->current_fiber->state = FiberStateSuspended;
    execution_context_switch(&scheduler->current_fiber->ctx, &scheduler->ctx, NULL);
    return;
}

struct FullContext {
    void *rsp, *rbx, *rbp, *r12, *r13, *r14, *r15;
    void *rip;
};

void *fiber_run(fiberCode code, void *data) {
    struct Scheduler *const scheduler = get_current_scheduler();
    size_t stack_size = 1024  * 1024 * 10;
    volatile void *const stack = mmap(/* address */ NULL, /* size */ stack_size,
    /* protection */PROT_READ | PROT_WRITE,
    /* flags */MAP_PRIVATE | MAP_ANONYMOUS,
    /* fd */-1, /* offset */ 0);
    void *stack_base = (char*)stack + stack_size;
    struct Fiber fiber = {
        .ctx = {
            .rsp = stack_base - sizeof(struct FullContext),
        },
        .procedure = code,
        .data = data,
    };
    void *ret_addr = stack_base - sizeof(struct FullContext) + sizeof(void*) + 40;
    *(fiberCode*)ret_addr = code;
    const size_t queue_cap = 100;
    struct Fiber *queue = malloc(queue_cap * sizeof(*queue));
    // FIXME: hanlde properly
    if (!queue) exit(1);
    *scheduler = (struct Scheduler) {
        .current_fiber = &fiber,
        .capacity = queue_cap,
        .len = 0,
        .fibers = queue,
    };
    execution_context_switch(&scheduler->ctx, &fiber.ctx, fiber.data);
}

