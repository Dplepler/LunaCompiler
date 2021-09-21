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
LOCAL m:DWORD
LOCAL z:DWORD
LOCAL y:DWORD
LOCAL str1[16]:BYTE
LOCAL str2[11]:BYTE
XOR EAX, EAX
MOV EBX, 101
fn lstrcpy, ADDR str1, "Hello, world!\n"
fn lstrcpy, ADDR str2, "Hi again\n"
MOV ECX, 100
MOV EDX, [m]
CMP EDX, ECX
JLE label1
invoke StdOut, ADDR str1
invoke StdOut, ADDR str2
MOV [m], EDX
label1:
MOV EAX, [m]
CMP EAX, 0
JE label2
label3:
MOV EAX, 3
MOV EBX, [m]
CMP EBX, EAX
JLE label4
MOV EDX, EAX
MOV EAX, [y]
MUL EAX
DEC EBX
JMP label3
label4:
MOV EAX, 3
MOV EBX, [m]
CMP EBX, EAX
JGE label5
MOV EDX, [y]
MOV [y], EDX
MOV [m], EDX
JMP label6
label5:
MOV EAX, [m]
JMP label6
label2:
MOV EAX, [m]
MUL EAX
label6:
PUSH y
MOV EAX, [m]
MOV EBX, [y]
ADD EAX, EBX
PUSH EAX
CALL foo
MOV ECX, [m]
SUB ECX, 5
main ENDP
main_start:
CALL main
invoke ExitProcess, 0
end main_start
