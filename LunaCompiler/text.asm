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
foo PROC x:DWORD, y:DWORD
MOV EAX, [x]
MOV EBX, EAX
MOV EAX, [y]
MUL EAX
foo ENDP
main PROC 
LOCAL x:DWORD
LOCAL z:DWORD
LOCAL y:DWORD
MOV ECX, [z]
ADD ECX, 5
MOV EDX, 2
MOV EAX, EBX
MUL EDX
MOV EAX, [y]
ADD EAX, 1
MOV ECX, [z]
ADD ECX, EAX
main ENDP
