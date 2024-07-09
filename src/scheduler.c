#include "scheduler.h"
#include "fiber.h"

static struct Scheduler current_scheduler;

struct Scheduler* get_current_scheduler() {
    return &current_scheduler;
}

