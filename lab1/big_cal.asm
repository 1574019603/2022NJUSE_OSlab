;已定义数据区
section .data
tip1: db "please input the operator X and Y:",0Ah

space: db 0
tip2: db "Invalid",0Ah
space2: db 0

line: db 0Ah


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
extra: resb 255 ;进位







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
    je addtask
    
    cmp byte[caozuofu],'*' 
    je multask



  addtask:
    call bigAdd
    mov eax,result
    call format_res;将最后的结果规范化，主要是处理进位的情况
    call printStr
    mov eax,line
    call printStr
    
    jmp restart

  multask:
    call bigMul
    mov eax,result
    call format_res
    call operatorZero;没有此步会出现1*1=01的情况
    call printStr
    mov eax,line
    call printStr
    jmp restart
  

  restart:
    call removeAll;因为程序需要连续输入，因此每一次处理过后都要将未定义的内存完全清零
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
    jmp restart

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
    pop eax
    pop edx
    pop ebx
    pop ecx
    
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
    mov ecx,extra
  toEnd:;这一步将eax移向result的尾部
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
    mov byte[ecx],0
    cmp byte[eax],'9'
    ja extraPrint
    pop edx
    ret

  extraPrint:
    inc byte[ecx]
    sub byte[eax],10
    cmp byte[eax],'9'
    ja extraPrint
    push eax
    mov eax,ecx
    add byte[eax],'0'
    call printStr
    pop eax
    jmp formatFinish




bigMul:
    pushad
    mov ecx,result
    mov edx,operator1
    mov ebx,operator2
    mov esi,0;esi 代表 o1前移的位数
    mov edi,0;edi表示o2从末尾前移的位数

    mov eax,0
    add eax,dword[len1]
    add eax,dword[len2]  ;eax存有两束长度之和

  init:;将结果result初始化为两数长度和的零
    cmp eax,0
    je toEnd_o1
    mov byte[ecx],'0'
    dec eax
    inc ecx
    jmp init

  toEnd_o1:;使edx指向o1的末尾
    cmp byte[edx+1],0
    je toEnd_o2
    inc edx
    jmp toEnd_o1

  toEnd_o2:
    cmp byte[ebx+2],0
    je multiple
    inc ebx
    jmp toEnd_o2

  multiple:;用于讲操作数一的某一位加载到al,结束条件为edx移动到首位

    push ebx;此时ebx指向o2的倒数第二个
    
    mov eax,0
    cmp edx,operator1
    jb finish_multiple
    mov al,byte[edx];al存o1的1位

    sub al,30h;使其ascii值为对应整数
    jmp multiple_loop

  mark:
    dec edx
    inc esi
    jmp multiple

  multiple_loop:;对操作数1的某一位，让操作数2从倒数第二位乘到第一位
    cmp ebx,operator2
    jb finish_multiple_loop
    mov ah,byte[ebx];ah存o2的一位

    push edx
    mov dl,al;dl存o1的一位,用于后续恢复到al以便进行后续运算
    sub ah,30h
    mul ah;现在ax中存着ah*al

    push ecx

    sub ecx,esi
    sub ecx,edi
    sub ecx,1;这里因为此时ecx位于结果最后一位再往后一位
    cmp byte[ecx],150
    ja simple_format_byte;不跳转则进入下面的代码块

  finish_simple_format_byte:
    add byte[ecx],al;将乘法结果加载入ecx
    pop ecx
    mov al,dl;恢复al为o1的某一位
    pop edx
    dec ebx;ebx（o2）前移
    inc edi;代表ebx（o2）前移的位数
    jmp multiple_loop

  finish_multiple_loop:
    mov edi,0
    pop ebx
    jmp mark

  finish_multiple:
    pop ebx
    popad
    ret

  simple_format_byte:
    sub byte[ecx],100
  	add byte[ecx-2],1
  	jmp finish_simple_format_byte





  



removeAll:   ;每次运算过后将相关内存清零
  push eax
  mov eax,input
  call allZero
  mov eax,operator1
  call allZero
  mov eax,len1
  call allZero
  mov eax,operator2
  call allZero
  mov eax,len2
  call allZero
  mov eax,caozuofu
  call allZero
  mov eax,result
  call allZero
  mov eax,extra
  call allZero
  pop eax

  ret


allZero:;清零操作

  push ebx
  push ecx
  mov ecx,eax
  mov ebx,255;因为预先给未定义数据区的大小为255
deleteLoop:
  cmp ebx,0
  je endZero
  mov byte[ecx],0
  inc ecx
  dec ebx
  jmp deleteLoop

endZero:
  pop ecx
  pop ebx
  ret


operatorZero:
    ;去除惩罚结果前面的0
    push ebx
    mov ebx,eax

  operatorZero_loop:
    cmp byte[ebx],'0'
    jne endoperatorZero_loop
    inc ebx
    jmp operatorZero_loop

  endoperatorZero_loop:
    cmp byte[ebx],0;防止出现0*0=   的情况
    je qianyi
  realend:
    mov eax,ebx
    pop ebx
    ret


  qianyi:
    dec ebx
    jmp realend
