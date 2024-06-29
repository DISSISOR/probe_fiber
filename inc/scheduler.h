#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include <stddef.h>

#include "execution_context.h"

struct Fiber;

struct Scheduler {
    struct Fiber *fibers;
    size_t len;
    size_t capacity;
    struct ExecutionContext ctx;
    struct Fiber *current_fiber;
};

struct Scheduler* get_current_scheduler();
#endif // SCHEDULER_H_INCLUDED
