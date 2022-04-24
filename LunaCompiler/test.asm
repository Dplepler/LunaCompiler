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
statement DWORD 10
.code
main PROC 
LOCAL x:DWORD
LOCAL x:DWORD
MOV EAX, 10
MOV [x], EAX
MOV EBX, 40
MOV [x], EBX
MOV ECX, EAX
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
