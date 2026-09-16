BITS 64
ORG 0x10000

mov rdi, message
int 0x70
xor rdi, rdi
int 0x70
jmp $

message db "Hello, User World!", 0x0A, 0