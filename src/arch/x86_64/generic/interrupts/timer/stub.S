.extern lapic_timer_drv
.global lapic_timer_stub

.section .text

lapic_timer_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov r12, rsp

    sub rsp, 512
    and rsp, -16

    mov r13, rsp

    fxsave [r13]

    mov rdi, r12
    mov rsi, r13

    call lapic_timer_drv

    fxrstor [r13]

    mov rsp, r12

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq