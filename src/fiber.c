#include "fiber.h"

#include <stdlib.h>
#include <sys/mman.h>

#include "execution_context.h"
#include "scheduler.h"

void fiber_yield(void) {
    struct Scheduler *scheduler = get_current_scheduler();
    if (scheduler->current_fiber->state != FiberStateTerminated) {
        scheduler->current_fiber->state = FiberStateSuspended;
    }
    execution_context_switch(&scheduler->current_fiber->ctx, &scheduler->ctx, NULL);
    return;
}

struct FullContext {
    void *rsp, *rbx, *rbp, *r12, *r13, *r14, *r15;
    void *rip;
};

static void proxy_ctx_switch() {
    struct Scheduler *const sch = get_current_scheduler();
    sch->current_fiber->procedure(sch->current_fiber->data);
    sch->current_fiber->state = FiberStateTerminated;
    execution_context_switch(&sch->current_fiber->ctx, &sch->ctx, NULL);
}

static inline struct FiberListNode* reschedule(struct Scheduler *sch, struct Fiber fiber) {
    struct FiberListNode *new_node = malloc(sizeof *new_node);
    // FIXME: hanlde properly
    if (!new_node) exit(1);
    new_node->fiber = fiber;
    struct FiberListNode *prev_tail = sch->fiber_queue.tail;
    if (!prev_tail) {
        sch->fiber_queue.head = sch->fiber_queue.tail = new_node;
    } else {
        prev_tail->next = new_node;
        sch->fiber_queue.tail = new_node;
    }
    sch->fiber_queue.len++;
    return new_node;
}

void fiber_run(fiberCode code, void *data) {
    struct Scheduler *const sch = get_current_scheduler();
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
    void *ret_addr = stack_base - 16;
    // void (*)(void);
    // *( void(*)(void) )ret_addr = proxy_ctx_switch;
    // *ret_addr = proxy_ctx_switch;
    * (( void(**)(void) ) ret_addr) = proxy_ctx_switch;
    const size_t queue_cap = 100;
    struct Fiber *queue = malloc(queue_cap * sizeof(*queue));
    // FIXME: hanlde properly
    if (!queue) exit(1);
    *sch = (struct Scheduler) {
        .current_fiber = &fiber,
        .fiber_queue = {
            .head = NULL,
            .tail = NULL,
            .len = 0,
        }
    };
    execution_context_switch(&sch->ctx, &fiber.ctx, fiber.data);
    for (;;) {
        switch (sch->current_fiber->state) {
            case FiberStateSuspended:
                // Reschedule and get next fiber
                (void) reschedule(sch, *sch->current_fiber);
                struct FiberList *queue = &sch->fiber_queue;
                sch->current_fiber = &queue->head->fiber;
                queue->len--;
                if (queue->len == 0) {
                    queue->head = queue->head->next;
                } else {
                    goto finalize;
                }
                break;
            case FiberStateRunning:
                unreachable();
                break;
            case FiberStateTerminated:
                queue = &sch->fiber_queue;
                sch->current_fiber = &queue->head->fiber;
                queue->head = queue->head->next;
                queue->len--;
                // goto finalize;
                break;
            default:
                unreachable();
        }
        execution_context_switch(&sch->ctx, &sch->current_fiber->ctx, sch->current_fiber->data);
    }
    // Clear resources
    finalize:
}

struct FiberJoinHandle fiber_add(fiberCode code, void* data) {
    struct Scheduler *const sch = get_current_scheduler();
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
    void *ret_addr = stack_base - 16;
    // *(fiberCode**)ret_addr = code;
    * (( void(**)(void) ) ret_addr) = proxy_ctx_switch;
    struct FiberListNode* new_node = reschedule(sch, fiber);
    return (struct FiberJoinHandle){
        .fiber = &new_node->fiber,
    };
}

void fiber_join(struct FiberJoinHandle handle) {
    while(handle.fiber->state != FiberStateTerminated) {
        fiber_yield();
    }
}
