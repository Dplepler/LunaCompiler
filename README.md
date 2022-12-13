# LunaCompiler
LunaCompiler is a very simple compiler that can understand both English and Hebrew, translating from a C like programming language to Assembly x86 (MASM syntax)! 

# To run this program
You can download it and compile with Visual Studio, to run the program simply input the .luna file you want to compile and a parameter indicating if the file is in Hebrew or English
-e for English and -h for Hebrew

Note that you also have to have MASM32 sdk installed in your C:\ drive
Link to a MASM download page: http://www.codingcrew.de/masm32/index.php

# Example
LunaCompiler.exe myCode.luna -e

# Note
The compiler contains a Lexer, Parser, Traversal (to create IR), Code Generator with somewhat good register allocation and a Symbol Table.
