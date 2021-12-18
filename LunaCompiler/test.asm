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
main PROC 
LOCAL x:DWORD
LOCAL y:DWORD
LOCAL z:DWORD
XOR EAX, EAX
MOV [x], EAX
MOV EBX, 10
MOV [y], EBX
MOV ECX, 20
MOV [z], ECX
SUB EBX, ECX
MOV ECX, EAX
MOV EAX, 3
MUL EBX
PUSHA
fnc StdOut, str$(EAX)
POPA
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
