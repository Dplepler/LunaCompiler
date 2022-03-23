# LunaCompiler
LunaCompiler is a very simple compiler that can understand both English and Hebrew, translating from a C like programming language to Assembly x86 using the MASM compiler! 

# To run this program
You can download it and compile with Visual Studio, to run the program simply input the .luna file you want to compile and a parameter indicating if the file is in Hebrew or English
-e for English and -h for Hebrew

# Example
LunaCompiler.exe myCode.luna -e

# Note
You can check out the IR created by the compiler by using the "traversal_print_instructions" routine
