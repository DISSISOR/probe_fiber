#ifndef FIBER_H_INCLUDED
#define FIBER_H_INCLUDED

#include "execution_context.h"
#include "stack.h"

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
    void *data;
    struct ExecutionContext ctx;
    enum FiberState state;
    struct StackView stack_view;
};

struct FiberJoinHandle {
    struct Fiber *fiber;
};

void fiber_yield(void);
int fiber_run(fiberCode code, void *data);
int fiber_add(struct FiberJoinHandle *handle, fiberCode code, void* data);
void fiber_join(struct FiberJoinHandle handle);

#endif // FIBER_H_INCLUDED
