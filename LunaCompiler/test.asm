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
youShallPass PROC x:DWORD
PUSHA
fnc StdOut, "Hello: "
POPA
PUSHA
MOV EAX, [x]
fnc StdOut, str$(EAX)
POPA
MOV EAX, [x]
SUB EAX, 32
RET
youShallPass ENDP
main PROC 
LOCAL magic:DWORD
MOV EAX, 42
MOV [magic], EAX
PUSH magic
CALL youShallPass
MOV EBX, 10
CMP EAX, EBX
JNE label1
PUSHA
fnc StdOut, "Worked!"
POPA
label1:
MOV EAX, 1
CMP EAX, 0
JE label2
PUSHA
fnc StdOut, "0!\n"
POPA
label2:
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
