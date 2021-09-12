MOV AX, 0

foo PROC x:DWORD, y:DWORD
	MOV BX, y
	MOV CX, x
	MUL BX, CX
foo ENDP

main PROC 
	ADD AX, 5
	MUL CX, 2
	MOV DX, y
	ADD DX, 1
main ENDP

MOV AX, z
ADD AX, y
