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
y DWORD 10
.code
main PROC 
LOCAL x:DWORD
MOV EAX, 10
MOV [x], EAX
PUSHA
MUL EAX
MOV EBX, [y]
MUL EBX
MOV ECX, 10
MUL ECX
MUL ECX
fnc StdOut, str$(EAX)
POPA
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
