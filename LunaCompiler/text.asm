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
CMP EBX, 0
JE label1
MOV ECX, EAX
MOV EAX, [y]
MUL EAX
RET
JMP label2
label1:
MOV EAX, EBX
MUL EBX
RET
label2:
PUSH y
ADD EBX, EAX
PUSH EBX
CALL foo
SUB EAX, 5
MOV EDX, [z]
ADD EDX, EAX
MOV EAX, EDX
RET
end main
