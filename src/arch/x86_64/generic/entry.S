.global kentry
.extern kmain

kentry:
    mov rax, cr0
    and rax, ~(1 << 2)
    or  rax,  (1 << 1)
    mov cr0, rax

    mov rax, cr4
    or  rax,  (1 << 9)
    or  rax,  (1 << 10)
    mov cr4, rax

    fninit

    and rsp, -16
    call kmain

.hang:
    cli
    hlt
    jmp .hang