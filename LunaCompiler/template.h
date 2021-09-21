/*
This is the Assembly MASM template that includes all necessary libraries and sets all the correct modes
*/


const char* asm_template[] = { ".386",
".model flat, stdcall",
"option casemap: none",
"include \\masm32\\include\\windows.inc",
"include \\masm32\\macros\\macros.asm",
"include \\masm32\\include\\kernel32.inc", 
"include \\masm32\\include\\masm32.inc",
"include \\masm32\\include\\gdi32.inc",
"include \\masm32\\include\\user32.inc",
"includelib \\masm32\\lib\\gdi32.lib",
"includelib \\masm32\\lib\\user32.lib",
"includelib \\masm32\\lib\\kernel32.lib",
"includelib \\masm32\\lib\\masm32.lib" };
