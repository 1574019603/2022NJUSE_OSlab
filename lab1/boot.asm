section .data
    msg db "Hello OS world!",0xa
    len equ $-msg

section .text

    global main

main:

    mov edx len
    mov ecx msg
    mov ebx 1
    mov eax 4
    int 0x80

    mov ebx,0
    mov eax,1
    int 0x80

    
