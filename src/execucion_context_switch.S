# 1. push calee saved registers on top
#  of `from` stack(rip is implicitly pushed by function call).
# 2. load to.rsp into rsp
# 3. pop registers in the reverse order. rip pops by ret implicitly,
#  too.
# rsp, rbx, rbp, r12, r13, r14, r15

.globl execution_context_switch

.text
# void execution_context_switch(struct ExecutionContext *from, struct ExecutionContext *to);
execution_context_switch:

pushq %rbp
pushq %rbx
pushq %r12
pushq %r13
pushq %r14
pushq %r15

movq  %rsp, (%rdi)
# загрузить новый rsp
movq (%rsi), %rsp

popq %r15
popq %r14
popq %r13
popq %r12
popq %rbx
popq %rbp

mov %rdx, %rdi

retq
