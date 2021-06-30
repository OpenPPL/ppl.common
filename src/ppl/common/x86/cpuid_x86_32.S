.global __do_cpuid
#.type __do_cpuid, @function
__do_cpuid:
    pushl %ebp
    movl %esp, %ebp
    pushl %edi
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    movl 8(%ebp), %eax
    movl 12(%ebp), %ecx
    movl 16(%ebp), %edi
    cpuid
    movl %eax, (%edi)
    movl %ebx, 4(%edi)
    movl %ecx, 8(%edi)
    movl %edx, 12(%edi)
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    popl %edi
    popl %ebp
    ret
