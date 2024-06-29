#ifndef EXECUTION_CONTEXT_H_INCLUDED
#define EXECUTION_CONTEXT_H_INCLUDED

struct ExecutionContext {
    // On top of the stack there will be placed callee saved registers
    // rsp, rbx, rbp, r12, r13, r14, r15 
    void *rsp;
};

extern void execution_context_switch(struct ExecutionContext *from, struct ExecutionContext *to, void* data);

#endif // EXECUTION_CONTEXT_H_INCLUDED
