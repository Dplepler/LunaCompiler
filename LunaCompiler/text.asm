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
MOV EAX, [x]
CMP EAX, 0
JE label1
label2:
MOV EAX, 3
MOV EBX, [x]
CMP EBX, EAX
JLE label3
MOV EDX, EAX
MOV EAX, [y]
MUL EAX
DEC EBX
MOV [y], EAX
MOV [x], EBX
JMP label2
label3:
MOV EAX, 3
MOV EBX, [x]
CMP EBX, EAX
JGE label4
MOV EDX, [y]
MOV [x], EDX
label4:
JMP label5
label1:
MOV EAX, [x]
MUL EAX
MOV [y], EAX
label5:
MOV EAX, [x]
SUB EAX, 5
MOV EBX, [z]
ADD EBX, EAX
MOV EAX, EBX
RET
end main
