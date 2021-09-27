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
globalVar DWORD 0
anotherGlobal DWORD 0
bruhbruh DWORD 0
.code
foo PROC x:DWORD, y:DWORD
MOV EAX, [y]
MOV EBX, [x]
MUL EBX
RET
foo ENDP
main PROC 
LOCAL m:DWORD
LOCAL y:DWORD
LOCAL counter:DWORD
LOCAL str1[15]:BYTE
LOCAL str2[11]:BYTE
XOR EAX, EAX
MOV [m], EAX
MOV [y], EAX
MOV [counter], EAX
PUSHA
fnc lstrcpy, ADDR str1, "Hello, world\n"
POPA
PUSHA
fnc lstrcpy, ADDR str2, "Hi again\n"
POPA
label1:
MOV EAX, 10
MOV EBX, [m]
CMP EBX, EAX
JGE label2
label3:
MOV EAX, 10
MOV EBX, [y]
CMP EBX, EAX
JGE label4
PUSHA
MOV EDX, [counter]
fnc StdOut, str$(EDX)
fnc StdOut, "\n"
POPA
MOV EAX, [counter]
INC EAX
MOV EBX, [y]
INC EBX
MOV [counter], EAX
MOV [y], EBX
JMP label3
label4:
MOV EAX, [m]
INC EAX
XOR EBX, EBX
MOV [m], EAX
MOV [y], EBX
JMP label1
label2:
MOV EAX, 10
MOV [m], EAX
CMP EAX, EAX
JNE label5
ADD EAX, EAX
MOV [y], EAX
JMP label6
label5:
MOV EAX, [m]
MUL EAX
MOV [y], EAX
label6:
PUSHA
MOV EAX, [y]
fnc StdOut, str$(EAX)
POPA
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
