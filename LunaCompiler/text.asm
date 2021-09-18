.386
.model flat, stdcall
option casemap: none
include c:\masm32\include\windows.inc
include c:\masm32\include\user32.inc
include c:\masm32\include\kernel32.inc
includelib c:\masm32\lib\user32.lib
includelib c:\masm32\lib\kernel32.lib
.data
globalVar DWORD 0
anotherGlobal DWORD 0
bruhbruh DWORD 0
.code
foo PROC x:DWORD, z:DWORD
MOV EAX, [y]
MOV EBX, [x]
MUL EBX
RET
foo ENDP
main:
LOCAL x:DWORD
LOCAL z:DWORD
LOCAL y:DWORD
MOV EAX, [x]
CMP EAX, 0
JE label1
MOV EBX, EAX
MOV EAX, [y]
MUL EAX
MOV [y], EAX
MOV [x], EBX
JMP label2
label1:
MOV EAX, [x]
MUL EAX
MOV [y], EAX
label2:
PUSH y
MOV EAX, [x]
MOV EBX, [y]
ADD EAX, EBX
PUSH EAX
CALL foo
SUB EAX, 5
MOV ECX, [z]
ADD ECX, EAX
MOV EAX, ECX
RET
end main
