.386
.model flat, stdcall
option casemap: none
include \masm32\include\windows.inc
include \masm32\macros\macros.asm
include \masm32\include\kernel32.inc
include \masm32\include\masm32.inc
include \masm32\include\gdi32.inc
include \masm32\include\user32.inc
includelib \masm32\lib\gdi32.lib
includelib \masm32\lib\user32.lib
includelib \masm32\lib\kernel32.lib
includelib \masm32\lib\masm32.lib
.data
.code
fib PROC n:DWORD
MOV EAX, [n]
MOV EBX, 1
CMP EAX, EBX
JG label1
RET
label1:
MOV EBX, [n]
DEC EBX
PUSH EBX
CALL fib
MOV ECX, [n]
SUB ECX, 2
PUSH ECX
CALL fib
ADD EDX, EAX
XCHG EAX, EDX
RET
fib ENDP
main PROC 
LOCAL x:DWORD
LOCAL ans:DWORD
MOV EAX, 9
MOV [x], EAX
MOV [ans], EBX
PUSH x
CALL fib
MOV EBX, 34
MOV [ans], EAX
CMP EAX, EBX
JNE label2
PUSHA
fnc StdOut, "Worked!\n"
POPA
PUSHA
fnc StdOut, str$(EAX)
POPA
label2:
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
