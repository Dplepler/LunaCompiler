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
LOCAL hello:DWORD
XOR EAX, EAX
MOV [hello], EAX
LOCAL m:DWORD
MOV EAX, 10
MOV [m], EAX
LOCAL z:DWORD
XOR EAX, EAX
MOV [z], EAX
LOCAL y:DWORD
XOR EAX, EAX
MOV [y], EAX
LOCAL str1[15]:BYTE
LOCAL str2[11]:BYTE
XOR EAX, EAX
MOV EBX, 10
PUSHA
fnc lstrcpy, ADDR str1, "Hello, world\n"
POPA
PUSHA
fnc lstrcpy, ADDR str2, "Hi again\n"
POPA
label1:
MOV [hello], EAX
MOV [z], EAX
MOV [y], EAX
MOV [m], EBX
CMP EBX, EAX
JLE label2
PUSHA
fnc StdOut, ADDR str1
POPA
PUSHA
fnc StdOut, ADDR str2
POPA
DEC EBX
MOV [m], EBX
JMP label1
label2:
MOV EAX, 10
MOV [m], EAX
XOR EBX, EBX
CMP EAX, EBX
JE label3
ADD EAX, EAX
MOV [y], EAX
JMP label4
label3:
MOV EAX, [m]
MUL EAX
MOV [y], EAX
label4:
PUSHA
MOV EAX, [y]
fnc StdOut, str$(EAX)
POPA
PUSH y
MOV EBX, [m]
ADD EBX, EAX
PUSH EBX
CALL foo
SUB EAX, 5
MOV ECX, EAX
XOR EAX, EAX
RET
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
