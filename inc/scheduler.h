#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include <stddef.h>

#include "execution_context.h"
#include "fiber.h"

struct FiberListNode {
    struct FiberListNode *next;
    struct Fiber fiber;
};

struct FiberList {
    struct FiberListNode *head;
    struct FiberListNode *tail;
    ptrdiff_t len;
};

struct Scheduler {
    struct ExecutionContext ctx;
    struct Fiber *current_fiber;
    struct FiberList fiber_queue;
    struct Fiber **terminated;
    size_t terminated_count;
    size_t terminated_cap;
};

struct Scheduler* get_current_scheduler();
#endif // SCHEDULER_H_INCLUDED
