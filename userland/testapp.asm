BITS 64
ORG 0x10000

mov rdi, 0
mov rsi, message
int 0x70

rdtsc

; Entire TSC is now in RDX
shl rdx, 32
or  rdx, rax

mov rdi, 255
mov rsi, 0
int 0x70

; If the loop somehow exits we just jmp $
jmp $

message db "Entered Ring 3!", 0x0A, 0