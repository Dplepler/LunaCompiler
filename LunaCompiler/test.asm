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
youShallPass PROC num:DWORD
PUSHA
MOV EAX, [num]
fnc StdOut, str$(EAX)
POPA
MOV EAX, [num]
MOV EBX, 42
CMP EAX, EBX
JNE label1
PUSHA
fnc StdOut, "Correct!\n"
fnc StdOut, "The magic number was: "
fnc StdOut, str$(EAX)
POPA
JMP label2
label1:
PUSHA
fnc StdOut, "Wrong\n"
POPA
label2:
MOV EAX, 10
RET
youShallPass ENDP
main PROC 
LOCAL magic:DWORD
MOV EAX, 42
MOV [magic], EAX
PUSH magic
CALL youShallPass
MOV EBX, EAX
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
