.386
.model flat, stdcall
option casemap: none
include c:\masm32\include\kernel32.inc
include c:\masm32\include\masm32.inc
includelib c:\masm32\lib\kernel32.lib
includelib c:\masm32\lib\masm32.lib
.data
clfblj1 DWORD 0
clfblj2 DWORD 0
.code
hot_mumpfv PROC a:DWORD, b:DWORD
MOV EAX, [b]
MOV EBX, [a]
MUL EBX
RET
hot_mumpfv ENDP
main:
LOCAL a:DWORD
LOCAL b:DWORD
LOCAL c:DWORD
MOV EAX, [a]
CMP EAX, 0
JE label1
label2:
MOV EAX, 3
MOV EBX, [a]
CMP EBX, EAX
JLE label3
MOV EDX, EAX
MOV EAX, [b]
MUL EAX
DEC EBX
MOV [b], EAX
MOV [a], EBX
JMP label2
label3:
MOV EAX, 3
MOV EBX, [a]
CMP EBX, EAX
JGE label4
MOV EDX, [b]
MOV [a], EDX
JMP label5
label4:
MOV EAX, [a]
MOV [b], EAX
JMP label5
label1:
MOV EAX, [a]
MUL EAX
MOV [b], EAX
label5:
MOV EAX, [a]
SUB EAX, 5
end main
