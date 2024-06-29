#ifndef FIBER_H_INCLUDED
#define FIBER_H_INCLUDED

#include "execution_context.h"

struct Scheduler;

enum FiberState {
    FiberStateRunning,
    FiberStateSuspended,
    FiberStateTerminated,
};

typedef void *(*fiberCode)(void*);

struct Fiber {
    struct ExecutionContext ctx;
    void *(*procedure)(void*);
    void *data;
    enum FiberState state;
};

void fiber_yield(void);
void *fiber_run(fiberCode code, void *data);

#endif // FIBER_H_INCLUDED
