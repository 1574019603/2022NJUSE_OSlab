;已定义数据区
section .data
tip1: db "please input the operator X and Y:",0Ah

space: db 0
tip2: db "invalid input",0Ah


;未定义数据区
section .bss
input: resb 255     ;键盘输入
;下为操作数及其长度
operator1: resb 255    
len1: resb 255
operator2: resb 255
len2: resb 255
caozuofu: resb 255;操作符
result: resb 255;结果






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

   

    ;检查是否符合输入规范，这里检查是否缺少操作数
    call checkIsValid

    ;开始处理相关运算
    cmp byte[caozuofu],'+'
    call bigAdd
    mov eax,result
    call format_res
    call 
    cmp byte[caozuofu],'*'
    call bigMul                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             

    jmp getin
  endin:
;结束程序
    mov eax, 1 
    mov ebx, 0 
    int 80h 

checkIsValid:
;检查是否是非法输入
    cmp byte[len1],0
    je exitProg
    cmp byte[len2],0
    je exitProg
    ;否则正常返回
    jmp return

  
  exitProg:
  ;因为异常故结束程序
  ;先输出异常信息
    pushad
    mov eax,4
    mov ebx,1
    mov ecx,tip2
    mov edx,14
    int 80h
    popad

  return:
    ret



getOperatorAndCaozuofu:
;此函数用于获取操作符,输出：caozuofu和两个操作数
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

  findcaozuoshu:
  ;看是否是操作符1
    cmp byte[eax],'+'
    je getCaozuoshu
    cmp byte[eax],'*'
    je getCaozuoshu

  ;接下来存第一个数
    mov bl,byte[eax]
    mov byte[ecx],bl
    inc eax
    inc ecx
    inc byte[esi]
    jmp findcaozuoshu
  
  getCaozuoshu:
    mov bl,byte[eax]
    mov byte[caozuofu],bl;将操作符存入内存 
    inc eax ;跳过这个操作符

  another:
  ;读第二个数
    cmp byte[eax],0
    jz end
    mov bl,byte[eax]
    mov byte[edx],bl
    inc eax
    inc edx
    inc byte[edi]
    jmp another

  end:
    dec byte[edi]
    popad
    ret


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

printInt:
;控制台输出数字,数字在eax
    push   eax
    push   ecx
    push   edx
    push   esi
    mov    ecx, 0;ecx 存位数
  
  divideLoop:
    inc    ecx       ;存有位数
    mov    edx, 0
    mov    esi, 10
    idiv   esi       ;eax=eax/10
    add    edx, 48   ;edx存有余数
    push   edx       ;edx存入栈
    cmp    eax, 0
    jnz    divideLoop
  printLoop:
    dec    ecx
    mov    eax, esp
    call   printStr
    pop    eax
    cmp    ecx, 0
    jnz    printLoop
    pop    esi
    pop    edx
    pop    ecx
    pop    eax
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



;重要：加法的实现
bigAdd:
;主要实现大数加法
    push eax
    push ebx
    push edx
    mov ecx,result;将result地址赋值给ecx，从而通过修改ecx来改变result

    mov eax,operator1
    mov ebx,operator2
    mov esi,dword[len1]
    mov edi,dword[len2]

    cmp esi,edi;对两个操作数进行比较，按不同情况进行操作
    ja o1_longer
    je normalCal
    jmp o2_longer

  o1_longer:
    sub esi,edi;此时esi保存长出来的长度
    
  o1_loop:;将o1的高位先存入结果中
    cmp esi,0
    je normalCal
    mov dl,byte[eax]
    mov byte[ecx],dl
    inc eax
    inc ecx
    dec esi
    jmp o1_loop

  o2_longer:;此时esi保存o2长出来的长度
    sub edi,esi
  o2_loop:;将o2的高位先存入结果中
    cmp edi,0
    je normalCal
    mov dl,byte[ebx]
    mov byte[ecx],dl
    inc ebx
    inc ecx
    dec edi
    jmp o2_loop


  normalCal:;真正的加法,两者位数相同时
    cmp byte[eax],0
    je finishAdd
    mov dl,byte[eax]
    add dl,byte[ebx]
    sub dl,30h;减去0的ascii值
    mov byte[ecx],dl
    inc eax
    inc ebx
    inc ecx
    jmp normalCal
    
  finishAdd:
    pop edx
    pop ebx
    pop eax
    ret


format_res:;将数字转变为10进制格式，如果发生进位会提前输出一
    push edx
    mov ebx,eax

  toEnd:
    cmp byte[eax],0
    je formatLoop
    inc eax
    jmp toEnd

  formatLoop:
    dec eax
    cmp eax,ebx
    je formatFinish

  formatLoop_1:
		cmp byte[eax],'9'
		jna formatLoop
		mov edx,eax
		dec edx
		sub byte[eax],10
		add byte[edx],1
		jmp formatLoop_1

  formatFinish:
    mov ecx,0
    cmp byte[eax],'9'
    ja extraPrint
    pop edx
    ret

  extraPrint:
    inc ecx
    sub byte[eax],10
    cmp byte[eax],'9'
    ja extraPrint
    push eax
    mov eax,ecx
    call printInt
    pop eax
    jmp formatFinish

