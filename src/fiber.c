#include "fiber.h"

#include <stddef.h>
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

static inline struct FiberListNode* fiber_get_node(const struct Fiber *fiber) {
    enum { OFFSET = offsetof(struct FiberListNode, fiber) };
    struct FiberListNode *node = (struct FiberListNode*)((char*)(fiber) - OFFSET);
    return node;
}

static inline void fiber_deinit(struct Fiber *fiber) {
    struct FiberListNode *node = fiber_get_node(fiber);
    munmap(fiber->stack_view.stack, fiber->stack_view.size);
    free(node);
}

static inline struct FiberListNode* reschedule(struct Scheduler *sch, struct Fiber* fiber) {
    // struct FiberListNode *new_node = malloc(sizeof *new_node);
    struct FiberListNode *node = fiber_get_node(fiber);
    struct FiberListNode *prev_tail = sch->fiber_queue.tail;
    if (!prev_tail) {
        sch->fiber_queue.head = sch->fiber_queue.tail = node;
    } else {
        prev_tail->next = node;
        sch->fiber_queue.tail = node;
    }
    sch->fiber_queue.len++;
    return node;
}

void fiber_run(fiberCode code, void *data) {
    struct Scheduler *const sch = get_current_scheduler();
    size_t stack_size = 1024  * 1024 * 10;
    volatile void *const stack = mmap(/* address */ NULL, /* size */ stack_size,
    /* protection */PROT_READ | PROT_WRITE,
    /* flags */MAP_PRIVATE | MAP_ANONYMOUS,
    /* fd */-1, /* offset */ 0);
    void *stack_base = (char*)stack + stack_size;
    struct FiberListNode *node = malloc(sizeof(node[0]));
    if (NULL == node) {
        exit(1);
    };
    node->fiber = (struct Fiber) {
        .ctx = {
            .rsp = stack_base - sizeof(struct FullContext),
        },
        .procedure = code,
        .data = data,
        .state = FiberStateSuspended,
    };
    void *ret_addr = stack_base - 16;
    * (( void(**)(void) ) ret_addr) = proxy_ctx_switch;
    *sch = (struct Scheduler) {
        .current_fiber = &node->fiber,
        .fiber_queue = {
            .head = NULL,
            .tail = NULL,
            .len = 0,
        }
    };
    sch->terminated_count = 0;
    sch->terminated_cap = 100;
    sch->terminated = malloc(sch->terminated_cap * sizeof(sch->terminated[0]));
    if (NULL == sch->terminated) {
        exit(1);
    }
    // reschedule(sch, sch->current_fiber);
    execution_context_switch(&sch->ctx, &sch->current_fiber->ctx, &sch->current_fiber->data);
    for (;;) {
        switch (sch->current_fiber->state) {
            case FiberStateSuspended:
                // Reschedule and get next fiber
                (void) reschedule(sch, sch->current_fiber);
                struct FiberList *queue = &sch->fiber_queue;
                sch->current_fiber = &queue->head->fiber;
                queue->len--;
                if (queue->len >= 0) {
                    queue->head = queue->head->next;
                } else {
                    // goto finalize;
                }
                break;
            case FiberStateRunning:
                unreachable();
                break;
            case FiberStateTerminated:
                if (sch->terminated_cap <= sch->terminated_count) {
                    sch->terminated_cap *= 2;
                    const typeof(sch->terminated) tmp = realloc(sch->terminated, sch->terminated_cap);
                    if (NULL == tmp) {
                        exit(1);
                    }
                    sch->terminated = tmp;
                }
                sch->terminated[sch->terminated_count++] = sch->current_fiber;

                queue = &sch->fiber_queue;
                sch->current_fiber = &queue->head->fiber;
                queue->len--;
                if (queue->len >= 0) {
                    queue->head = queue->head->next;
                } else {
                    goto finalize;
                }
                break;
            default:
                unreachable();
        }
        execution_context_switch(&sch->ctx, &sch->current_fiber->ctx, sch->current_fiber->data);
    }
    // Clear resources
    finalize:
    for (size_t i=0; i < sch->terminated_count; i++) {
        struct Fiber *f = sch->terminated[i];
        fiber_deinit(f);
    }
    free(sch->terminated);
}

struct FiberJoinHandle fiber_add(fiberCode code, void* data) {
    struct Scheduler *const sch = get_current_scheduler();
    size_t stack_size = 1024  * 1024 * 10;
    volatile void *const stack = mmap(/* address */ NULL, /* size */ stack_size,
    /* protection */PROT_READ | PROT_WRITE,
    /* flags */MAP_PRIVATE | MAP_ANONYMOUS,
    /* fd */-1, /* offset */ 0);
    void *stack_base = (char*)stack + stack_size;
    struct FiberListNode *node = malloc(sizeof(node[0]));
    if (NULL == node) {
        exit(1);
    }
    node->fiber = (struct Fiber) {
        .ctx = {
            .rsp = stack_base - sizeof(struct FullContext),
        },
        .procedure = code,
        .data = data,
    };
    void *ret_addr = stack_base - 16;
    // *(fiberCode**)ret_addr = code;
    * (( void(**)(void) ) ret_addr) = proxy_ctx_switch;
    struct FiberListNode* new_node = reschedule(sch, &node->fiber);
    return (struct FiberJoinHandle){
        .fiber = &new_node->fiber,
    };
}

void fiber_join(struct FiberJoinHandle handle) {
    while(handle.fiber->state != FiberStateTerminated) {
        fiber_yield();
    }
}
