#ifndef FIBER_H_INCLUDED
#define FIBER_H_INCLUDED

#include "execution_context.h"

struct Scheduler;

enum FiberState {
    FiberStateRunning,
    FiberStateSuspended,
    FiberStateTerminated,
};

typedef void (fiberCode)(void*);
// typedef void (fiberJoinCallback)(void);

struct Fiber {
    fiberCode *procedure;
    // fiberJoinCallback *join_cb;
    void *data;
    struct ExecutionContext ctx;
    enum FiberState state;
};

struct FiberJoinHandle {
    struct Fiber *fiber;
};

void fiber_yield(void);
void fiber_run(fiberCode code, void *data);
struct FiberJoinHandle fiber_add(fiberCode code, void* data);
void fiber_join(struct FiberJoinHandle handle);

#endif // FIBER_H_INCLUDED
