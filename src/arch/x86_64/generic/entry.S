.global kentry
.extern kmain

kentry:
    mov rax, cr0
    and rax, ~(1 << 2)     # CR0.EM = 0
    or  rax,  (1 << 1)     # CR0.MP = 1
    mov cr0, rax

    mov rax, cr4
    or  rax,  (1 << 9)     # CR4.OSFXSR = 1
    or  rax,  (1 << 10)    # CR4.OSXMMEXCPT = 1
    mov cr4, rax

    fninit

    call kmain

.hang:
    cli
    hlt
    jmp .hang