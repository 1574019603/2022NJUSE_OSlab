section .data
	red:			db 1Bh,"[31m"
	redSize: 		equ $-red
	backgrould:		db 1Bh,"[0m"
	bgSize:			equ $-backgrould

section .text
	GLOBAL asm_printNormal
	GLOBAL asm_printRed

asm_printNormal:
	mov eax,4
	mov ebx,1
	mov ecx,backgrould
	mov edx,bgSize
	int 80h


	mov eax,4
	mov ebx,1
	mov ecx,[esp+4]
	;esp+4指向第一个参数
	mov edx,[esp+8]
	int 80h
	ret

asm_printRed:
	mov eax,4
	mov ebx,1
	mov ecx,red
	mov edx,redSize
	int 80h

	mov eax,4
	mov ebx,1
	mov ecx,[esp+4]
	;esp+4指向第一个参数
	mov edx,[esp+8]
	int 80h
	ret
