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
LOCAL mhtg[11]:BYTE
FN LSTRCPY, ADDR mhtg, "ulfm lkflm"
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
JMP label2
label3:
MOV EAX, 3
MOV EBX, [a]
CMP EBX, EAX
JGE label4
MOV EDX, [b]
MOV [b], EDX
MOV [a], EDX
JMP label5
label4:
MOV EAX, [a]
JMP label5
label1:
MOV EAX, [a]
MUL EAX
label5:
invoke StdOut, ADDR mhtg
MOV EAX, [a]
SUB EAX, 5
end main
