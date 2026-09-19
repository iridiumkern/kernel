BITS 64
ORG 0x10000

mov rdi, 0
mov rsi, initbinary
mov rdx, initbinary_size
int 0x70

; If the loop somehow exits we just jmp $
jmp $

initbinary db "userland/init"
initbinary_size equ $ - initbinary