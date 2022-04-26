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
MOV ECX, EAX
PUSH ECX
MOV EDX, [n]
SUB EDX, 2
PUSH EDX
CALL fib
POP ECX
ADD ECX, EAX
XCHG EAX, ECX
RET
fib ENDP
main PROC 
PUSHA
PUSH 9
CALL fib
fnc StdOut, str$(EAX)
POPA
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
