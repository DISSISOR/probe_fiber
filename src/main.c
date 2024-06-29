#include <stdio.h>

#include "fiber.h"
#include "scheduler.h"

static int res_idx = 0;
enum { RES_SIZE = 5 };

static void *fst_fiber_code(void *payload);
static void *snd_fiber_code(void *payload);
static void *trd_fiber_code(void *payload);

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    int res[RES_SIZE] = { 0, };
    void *ret_val = fiber_run(fst_fiber_code, (void*)res);
    (void) ret_val;
    for (int i=0; i < RES_SIZE; i++) {
        if (res[i] != i) {
            fprintf(stderr, "WRONG: res[%d]=%d\n", res[i], i);
        }
    }
    return 0;
}

static void *fst_fiber_code(void *payload) {
    int *res = (int*)payload;
    // struct Fiber snd = fiber_add(snd_fiber_code, payload);
    res[res_idx++] = 0;
    fiber_yield();
    res[res_idx++] = 2;
    // struct Fiber trd = fiber_add(trd_fiber_code, payload);

    // (void)fiber_join(&snd);
    // (void)fiber_join(&trd);
    return NULL;
}

static void *snd_fiber_code(void *payload) {
    int *res = (int*)payload;
    res[res_idx++] = 1;
    fiber_yield();
    res[res_idx++] = 3;
}

static void *trd_fiber_code(void *payload) {
    int *res = (int*)payload;
    res[res_idx++] = 4;
}
