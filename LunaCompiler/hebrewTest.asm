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
LOCAL btfk_pflm[14]:BYTE
LOCAL a:DWORD
LOCAL b:DWORD
PUSHA
fnc lstrcpy, ADDR btfk_pflm, "Hello, World!"
POPA
XOR EAX, EAX
MOV EBX, 5
label1:
MOV EAX, [a]
MOV EBX, 10
MOV [a], EAX
CMP EAX, EBX
JG label2
PUSH EAX
fnc StdOut, "Amazing!\n"
POP EAX
INC EAX
MOV [a], EAX
JMP label1
label2:
MOV EAX, [a]
fnc StdOut, str$(EAX)
fnc StdOut, ADDR btfk_pflm
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
