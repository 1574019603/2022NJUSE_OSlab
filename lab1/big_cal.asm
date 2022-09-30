;已定义数据区
section .data
tip1: db "please input the operator X and Y:",0Ah

space: db 0
tip2: db "invalid input"


;未定义数据区
section .bss
input: resb 255     ;键盘输入
;下为操作数及其长度
operator1: resb 255    
len1: resb 255
operator2: resb 255
len2: resb 255
caozuofu: resb 255;操作符






section .text
    global main ;程序入口

main:
  getin:
    mov eax,tip1
    call printStr ;输出提示语句

    mov ecx,input;ecx=input
    call getInput ;提取输入

    ;如果是q，跳转到结束模块
    cmp byte[input],'q'
    jz endin

    ;否则进入后续处理
    ;获取操作符,存入caozuofu
    call getOperatorAndCaozuofu


  endin:
;结束程序
    mov eax, 1 
    mov ebx, 0 
    int 80h 



getOperatorAndCaozuofu:
;此函数用于获取操作符,输出：caozuofu
    pushad
    mov byte[caozuofu],0
    mov eax,input
    mov ebx,eax
    mov ecx,operator1
    mov edx,operator2
    mov esi,len1
    mov edi,len2
    mov byte[esi],0
    mov byte[edi],0

 ; .find




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
    ret


;用于获取字符串长度以输出
getStrLen:
    push ebx
    push eax
    mov ebx,eax;两者地址相同用于后续相减
  getNext:
    cmp byte[eax],0
    jz endLoop ;以标志位为1时跳转
    inc eax ;eax自增
    jmp getNext

  endLoop:
    sub eax,ebx;相减得出长度
    mov edx,eax
    pop eax
    pop ebx
    ret



getInput:
;获得键盘输入
    pushad
    mov edx,255
    mov eax,3
    mov ebx,0
    int 80h
    popad
    ret
