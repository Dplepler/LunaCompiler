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
main:
LOCAL x:DWORD
LOCAL z:DWORD
LOCAL y:DWORD
LOCAL str[14]:BYTE
FN LSTRCPY, ADDR str, "Hello, world!"
LOCAL str2[9]:BYTE
FN LSTRCPY, ADDR str2, "Hi again"
MOV EAX, 100
MOV EBX, [x]
CMP EBX, EAX
JLE label1
invoke StdOut, ADDR str2
invoke StdOut, ADDR str
label1:
MOV EAX, [x]
CMP EAX, 0
JE label2
label3:
MOV EAX, 3
MOV EBX, [x]
CMP EBX, EAX
JLE label4
MOV EDX, EAX
MOV EAX, [y]
MUL EAX
DEC EBX
JMP label3
label4:
MOV EAX, 3
MOV EBX, [x]
CMP EBX, EAX
JGE label5
MOV EDX, [y]
MOV [y], EDX
MOV [x], EDX
JMP label6
label5:
MOV EAX, [x]
JMP label6
label2:
MOV EAX, [x]
MUL EAX
label6:
PUSH y
MOV EAX, [x]
MOV EBX, [y]
ADD EAX, EBX
PUSH EAX
CALL foo
MOV ECX, [x]
SUB ECX, 5
end main
