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
LOCAL hello[5]:BYTE
MOV EAX, 10
MOV EBX, 5
MUL EBX
MOV ECX, 3
ADD ECX, 2
XCHG EAX, ECX
MOV EBX, 3
PUSH EDX
XOR EDX, EDX
DIV EBX
POP EDX
SUB ECX, EAX
MOV EDX, 50
PUSHA
fnc lstrcpy, ADDR hello, "Hi\n"
POPA
PUSH ECX
PUSH EDX
fnc StdOut, str$(ECX)
fnc StdOut, "\n"
POP EDX
POP ECX
PUSH EDX
fnc StdOut, str$(EDX)
fnc StdOut, "\n"
POP EDX
fnc StdOut, ADDR hello
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
