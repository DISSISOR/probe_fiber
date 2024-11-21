#include <stdio.h>

#include "fiber.h"

static int res_idx = 0;
enum { RES_SIZE = 5 };

static void fst_fiber_code(void *payload);
static void snd_fiber_code(void *payload);
static void trd_fiber_code(void *payload);

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    int res[RES_SIZE] = { 0, };
#if 0
    for (size_t i=0; i<10000; i++) {
        res[0] = res[1] = res[2] = res[3] = res[4] = 0;
        res_idx = 0;
        fiber_run(fst_fiber_code, res);
    }
#else
    fiber_run(fst_fiber_code, res);
    for (int i=0; i < RES_SIZE; i++) {
        if (res[i] != i) {
            fprintf(stderr, "WRONG: res[%d]=%d\n", i, res[i]);
        } else {
            fprintf(stdout, "res[%d]=%d\n", i, res[i]);
        }
    }
#endif
    return 0;
}

static void fst_fiber_code(void *payload) {
    int *res = (int*)payload;
    struct FiberJoinHandle snd = fiber_add(snd_fiber_code, payload);
    res[res_idx++] = 0;
    fiber_yield();
    res[res_idx++] = 2;
    struct FiberJoinHandle trd = fiber_add(trd_fiber_code, payload);

    fiber_join(snd);
    fiber_join(trd);
}

static void snd_fiber_code(void *payload) {
    int *const res = (int*)payload;
    res[res_idx++] = 1;
    fiber_yield();
    res[res_idx++] = 3;
}

static void trd_fiber_code(void *payload) {
    int *res = (int*)payload;
    res[res_idx++] = 4;
}
