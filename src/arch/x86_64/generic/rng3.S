.global jump_usermode

jump_usermode:
    mov ax, (4 * 8) | 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push (4 * 8) | 3
    push rsi
    pushfq
    push (3 * 8) | 3
    push rdi
    iretq