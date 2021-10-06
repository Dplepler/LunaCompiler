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
XOR EAX, EAX
MOV [a], EAX
MOV EBX, 5
MOV [b], EBX
PUSHA
fnc lstrcpy, ADDR btfk_pflm, "Hello, World!"
POPA
label1:
MOV EAX, [a]
MOV EBX, 10
MOV [a], EAX
CMP EAX, EBX
JG label2
PUSH EDX
XOR EDX, EDX
MUL EAX
MOV EBX, 81
PUSH EDX
XOR EDX, EDX
PUSH EDX
XOR EDX, EDX
DIV EBX
POP EDX
MOV EDX, 1
MOV [b], EAX
CMP EAX, EDX
JNE label3
PUSHA
fnc StdOut, str$(EAX)
fnc StdOut, "\n"
POPA
JMP label4
label3:
PUSHA
MOV EAX, [a]
fnc StdOut, str$(EAX)
fnc StdOut, "Weird\n"
POPA
label4:
MOV EAX, [a]
INC EAX
MOV [a], EAX
JMP label1
label2:
PUSHA
fnc StdOut, ADDR btfk_pflm
POPA
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
