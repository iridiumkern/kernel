.extern apisyscall
.global apistub

.section .text

apistub:
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    mov r12, rsp
    and rsp, -16
    call apisyscall
    mov rsp, r12

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx

    iretq