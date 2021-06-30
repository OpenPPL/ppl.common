.globl __do_cpuid
#.type __do_cpuid, @function
# edi: input eax value.
# esi: input ecx value.
# rdx: output address.
__do_cpuid:
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %r11
    movl %edi, %eax
    movl %esi, %ecx
    movq %rdx, %r11
    cpuid
    movl %eax, (%r11)
    movl %ebx, 4(%r11)
    movl %ecx, 8(%r11)
    movl %edx, 12(%r11)
    popq %r11
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    ret
