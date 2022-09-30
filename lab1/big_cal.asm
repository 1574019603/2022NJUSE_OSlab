;已定义数据区
section .data
tip1: db "please input the operator X and Y:",0Ah
anotherLine: db 0Ah


;未定义数据区
section .bss
input: resb 255     ;键盘输入
;下为操作数及其长度
operator1: resb 255    
len1: resb 255
operator2: resb 255
len2: resb 255






section .text
    global main ;程序入口

main:
    mov eax,tip1
    



printStr:
;此函数用于输出字符串，其中字符串地址在eax
    push ecx
    push ebx
    push edx
    push eax
    mov ecx,eax
;输出需要edx有字符串长度
    call getStrLen
    mov eax,4
    mov ebx,1
    int 80h
    pop edx
    pop ebx
    pop ecx
    pop eax


;用于获取字符串长度以输出
getStrLen:
    push ebx
    push eax
    mov ebx,eax;两者地址相同用于后续相减
  .getNext:
    cmp byte[eax],0
    jz endLoop ;以标志位为1时跳转
    inc eax ;eax自增
    jmp getNext

  .endLoop:
    sub eax,ebx;相减得出长度
    mov edx,eax
    pop eax
    pop ebx
    ret


