#include <stdio.h>

#include "stack.h"
#include "coro.h"

static int res_idx = 0;
enum { RES_SIZE = 5 };

static void fst_coro_code(struct Coro *coro);

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    int res = 0;
    struct Coro fst_coro;
    struct StackPool stack_pool;
    stack_pool_init(&stack_pool, 10, 10, 200);
    coro_init(&fst_coro, fst_coro_code, &stack_pool, &res);
    int c = getc(stdin);
    while ( c != EOF && c != 'q') {
        coro_resume(&fst_coro);
        printf("%d\n", res);
        c = getc(stdin);
    }
    return 0;
}

static void fst_coro_code(struct Coro *coro) {
    int n;
    for (;;) {
        *((int*)coro->arg) = n++;
        coro_yield(coro);
    }
}
