global _start
BITS 64

_start:
    mov rdi, 0
    lea rsi, [rel initbinary]
    mov rdx, initbinary_size
    int 0x70
    
    ; If the loop somehow exits we just jmp $
    jmp $

initbinary db "userland/init"
initbinary_size equ $ - initbinary