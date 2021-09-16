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
.code
foo PROC x:DWORD, y:DWORD
MOV EAX, [y]
MOV EBX, [x]
MUL EBX
foo ENDP
main:
LOCAL x:DWORD
LOCAL z:DWORD
LOCAL y:DWORD
MOV ECX, [z]
ADD ECX, 5
MOV EAX, EBX
MOV EDX, 2
MUL EDX
MOV EAX, [y]
MUL EAX
MOV ECX, [z]
ADD ECX, EAX
end main
