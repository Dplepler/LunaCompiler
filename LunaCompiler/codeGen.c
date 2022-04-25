#include "codeGen.h"
#include "template.h"

/*
init_asm_frontend initializes the asm frontend
Input: Program symbol table, Head of the TAC instructions list, target name for the produced file (.asm)
Output: The asm frontend
*/
asm_frontend* init_asm_frontend(table_T* table, TAC* head, char* targetName) {

    asm_frontend* frontend = mcalloc(1, sizeof(asm_frontend));

    frontend->registers = mcalloc(REG_AMOUNT, sizeof(register_T*));
    frontend->labelList = mcalloc(1, sizeof(label_list));
    
    // Allocate all registers for the frontend
    for (unsigned int i = 0; i < REG_AMOUNT; i++) {

        frontend->registers[i] = mcalloc(1, sizeof(register_T));
        frontend->registers[i]->reg = i;        // Assigning each register it's name
    }  
    
    frontend->table = table;
    frontend->instruction = head;

    frontend->targetProg = fopen(targetName, "w");

    return frontend;
}

/*
descriptor_push pushes an argument to the register descriptor
Input: Register to push to, argument to push
Output: None
*/
void descriptor_push(register_T* reg, arg_T* descriptor) {

    reg->regDescList = mrealloc(reg->regDescList, sizeof(arg_T*) * ++reg->size);
    reg->regDescList[reg->size - 1] = descriptor;
}

/*
descriptor_push_tac pushes a temporary into the register
Input: Backend, desired register, instruction to push
Output: None
*/
void descriptor_push_tac(asm_frontend* frontend, register_T* reg, TAC* instruction) {

    descriptor_reset(frontend, reg);        
    descriptor_push(reg, init_arg(instruction, TEMP_P));
}

/*
descriptor_reset resets the register descriptor to not include any arguments
Input: Backend, register to reset
Output: None
*/
void descriptor_reset(asm_frontend* frontend, register_T* r) {

    entry_T* entry = NULL;

    // Free all arguments
    for (unsigned int i = 0; i < r->size; i++) {

        if (r->regDescList[i]->type == TEMP_P) {

            r->regDescList[i]->value = NULL;
            free(r->regDescList[i]);
            r->regDescList[i] = NULL;
        }
        else if (r->regDescList[i]->type == CHAR_P && (entry = table_search_entry(frontend->table, r->regDescList[i]->value))) {

            address_remove_register(entry, r);
        }
    }

    // Free the register descriptor list itself
    if (r->regDescList) {

        free(r->regDescList);
        r->regDescList = NULL;
        r->size = 0;
    }
}

/*
descriptor_reset_all_registers just automatically resets all register descriptors from all general purpose registers
Input: Backend
Output: None
*/
void descriptor_reset_all_registers(asm_frontend* frontend) {

    for (unsigned int i = 0; i < GENERAL_REG_AMOUNT; i++) {
        descriptor_reset(frontend, frontend->registers[i]);
    }    
}

/*
generate_global_vars generates all the global variables of the program at the .data segment of the assembly file
Input: Backend, head of TAC instructions
Output: None
*/
void generate_global_vars(asm_frontend* frontend, TAC* triple) {

    table_T* table = frontend->table;

    while (triple) {

        switch (triple->op) {

        case TOKEN_LBRACE:

            table->tableIndex++;
            table = table->nestedScopes[table->tableIndex - 1];
            break;

        case TOKEN_RBRACE:

            table->tableIndex = 0;
            table = table->prev;
            break;

        case AST_VARIABLE_DEC:

            if (!(table_search_table(table, triple->arg1->value)->prev)) {

                // Declaring and assigning the global var the value of the next operation (which will be an assignment)
                fprintf(frontend->targetProg, "%s %s %s\n", (char*)triple->arg1->value, (char*)triple->arg2->value, (char*)triple->next->arg2->value);
                triple = triple->next;
            }
        
            break;
        }

        triple = triple->next;
    }
}

/*
write_asm is the main code generator function that produces all the Assembly code
Input: Symbol table, head of TAC list, target name for the file we want to produce
*/
void write_asm(table_T* table, TAC* head, char* targetName) {

    asm_frontend* frontend = init_asm_frontend(table, head, targetName);    // Initialize frontend
    TAC* triple = head;
    TAC* mainStart = NULL;
    int mainTableIndex = 0;

    // Print the template for the MASM Assembly program
    for (unsigned int i = 0; i < TEMPLATE_SIZE; i++) {
        fprintf(frontend->targetProg, "%s\n", asm_template[i]);
    }
    
    // Generate the global variables in the .data section of the Assembly file
    fprintf(frontend->targetProg, ".data\n");
    generate_global_vars(frontend, triple);

    fprintf(frontend->targetProg, ".code\n");

    frontend->table->tableIndex = 0;

    // Main loop to generate code
    while (frontend->instruction) {

        // Save main function start
        if (frontend->instruction->op == AST_FUNCTION && !strcmp(frontend->instruction->arg1->value, "main")) {
            mainStart = frontend->instruction;
        }
            
        generate_asm(frontend);
        frontend->instruction = frontend->instruction->next;
    }

    frontend->instruction = mainStart;

    if (mainStart) {

        generate_main(frontend);
    }
    else {

        printf("[Error]: No main file to start executing from");
        exit(1);
    }
        
    fclose(frontend->targetProg);
    free_registers(frontend);
}

/*
generate_asm checks for the operation in a TAC instruction and generates Assembly code for it
Input: Backend
Output: None
*/
void generate_asm(asm_frontend* frontend) {

    register_T* reg1 = NULL;
    register_T* reg2 = NULL;
    
    char* op = NULL;
    char* arg1 = NULL;
    char* arg2 = NULL;

    // We do not want to generate code for statements made outside of a function
    if (frontend->instruction->op != AST_VARIABLE_DEC && frontend->instruction->op != TOKEN_LBRACE
        && frontend->instruction->op != TOKEN_RBRACE && frontend->instruction->op != AST_FUNCTION && !frontend->table->prev) {

        return;
    }
        
    op = typeToString(frontend->instruction->op);    // Get type of operation in a string form

    // Binary operations
    if (frontend->instruction->op == AST_ADD || frontend->instruction->op == AST_SUB) {

        generate_binop(frontend);
    }
    // Multiplications and divisions are different in Assembly 8086 sadly
    else if (frontend->instruction->op == AST_MUL || frontend->instruction->op == AST_DIV) {

        generate_mul_div(frontend);
    }
    else if (frontend->instruction->op == TOKEN_LESS || frontend->instruction->op == TOKEN_MORE
        || frontend->instruction->op == TOKEN_ELESS || frontend->instruction->op == TOKEN_EMORE
        || frontend->instruction->op == TOKEN_DEQUAL || frontend->instruction->op == TOKEN_NEQUAL) {

        generate_condition(frontend);
    }
    else {

        switch (frontend->instruction->op) {
        
        case TOKEN_LBRACE:
            // If we reached the start of a new block, go to the fitting symbol table for that block
            frontend->table->tableIndex++;
            frontend->table = frontend->table->nestedScopes[frontend->table->tableIndex - 1];
            break;

        case TOKEN_RBRACE: generate_block_exit(frontend);

            descriptor_reset_all_registers(frontend);
            // When done with a block, reset the index of the table and go to the previous one
            frontend->table->tableIndex = 0;
            frontend->table = frontend->table->prev;
            break;

        // For each of the operation cases, generate fitting Assembly instructions
        case AST_FUNCTION: generate_function(frontend); break;
        case AST_ASSIGNMENT: generate_assignment(frontend); break;
        case AST_VARIABLE_DEC: generate_var_dec(frontend); break;
        case AST_IFZ: generate_if_false(frontend); break;
        case AST_GOTO: generate_unconditional_jump(frontend); break;
        case AST_LABEL: fprintf(frontend->targetProg, "%s:\n", generate_get_label(frontend, frontend->instruction)); descriptor_reset_all_registers(frontend); break;
        case AST_LOOP_LABEL: fprintf(frontend->targetProg, "%s:\n", generate_get_label(frontend, frontend->instruction)); descriptor_reset_all_registers(frontend); break;
        case AST_FUNC_CALL: generate_func_call(frontend); break;
        case AST_PRINT: generate_print(frontend); break;
        case AST_RETURN: generate_return(frontend); break;
    
        
        }
    }
}

/*
generate_binop generates Assembly code for addition and substruction
Input: Backend
Output: None
*/
void generate_binop(asm_frontend* frontend) {

    register_T* reg1 = NULL;
    register_T* reg2 = NULL;

    char* arg1 = NULL;
    char* arg2 = NULL;

    // If the operation is add or sub by 0 then we can do nothing 
    if (frontend->instruction->arg2->type == CHAR_P && !strcmp(frontend->instruction->arg2->value, "0")) {

        descriptor_push_tac(frontend, generate_move_to_register(frontend, frontend->instruction->arg1), frontend->instruction);
        return;
    }
    if (frontend->instruction->op == AST_ADD && frontend->instruction->arg1->type == CHAR_P && !strcmp(frontend->instruction->arg1->value, "0")) {

        descriptor_push_tac(frontend, generate_move_to_register(frontend, frontend->instruction->arg2), frontend->instruction);
        return;
    }

    // Addition by 1 can be replaced by the instruction INC
    if (frontend->instruction->arg2->type == CHAR_P && !strcmp(frontend->instruction->arg2->value, "1")) {

        reg1 = generate_move_to_register(frontend, frontend->instruction->arg1);
        descriptor_push_tac(frontend, reg1, frontend->instruction);
        frontend->instruction->op == AST_ADD ? fprintf(frontend->targetProg, "INC %s\n", generate_get_register_name(reg1)) : fprintf(frontend->targetProg, "DEC %s\n", generate_get_register_name(reg1));
        return;
    }
    // Substruction by 1 can be replaced by the instruction DEC
    if (frontend->instruction->op == AST_ADD && frontend->instruction->arg1->type == CHAR_P && !strcmp(frontend->instruction->arg1->value, "1")) {

        reg1 = generate_move_to_register(frontend, frontend->instruction->arg2);
        descriptor_push_tac(frontend, reg1, frontend->instruction);
        fprintf(frontend->targetProg, "INC %s\n", generate_get_register_name(reg1));
        return;
    }

    reg1 = generate_move_to_register(frontend, frontend->instruction->arg1);    // Get the first argument in a register

    arg1 = generate_assign_reg(reg1, frontend->instruction->arg1->value);
    arg2 = NULL;
    
    // If the value in the second argument is a variable or temp, we want to use a register
    if (table_search_entry(frontend->table, frontend->instruction->arg2->value) || frontend->instruction->arg2->type == TAC_P) {

        reg1->regLock = true;
        reg2 = generate_move_to_register(frontend, frontend->instruction->arg2);
        reg1->regLock = false;

        arg2 = generate_assign_reg(reg2, frontend->instruction->arg2->value);
    }
    // If the value of the second argument is a number, we can treat it as a const instead of putting it
    // in a new register
    else {

        arg2 = frontend->instruction->arg2->value;
    }

    descriptor_push_tac(frontend, reg1, frontend->instruction);            // We treat the whole TAC as a temporary variable that is now in the register

    fprintf(frontend->targetProg, "%s %s, %s\n", typeToString(frontend->instruction->op), arg1, arg2);
}

/*
generate_mul_div generates Assembly code for multiplication and division
Input: Backend
Output: None
*/
void generate_mul_div(asm_frontend* frontend) {

    register_T* reg1 = NULL;
    register_T* reg2 = NULL;
    entry_T* entry = NULL;

    // If one of the operands is 1 and the operation is multiplication then do nothing as multiplication by 1 means nothing
    // Also if the second argument is 1 and the operation is division we can do nothing as well for the same reason
    if (frontend->instruction->arg2->type == CHAR_P && !strcmp(frontend->instruction->arg2->value, "1")) {

        descriptor_push_tac(frontend, generate_move_to_register(frontend, frontend->instruction->arg1), frontend->instruction);    
        return;
    }
    if (frontend->instruction->op == AST_MUL && frontend->instruction->arg1->type == CHAR_P && !strcmp(frontend->instruction->arg1->value, "1")) {

        descriptor_push_tac(frontend, generate_move_to_register(frontend, frontend->instruction->arg2), frontend->instruction);    
        return;
    }

    frontend->registers[REG_DX]->regLock = true;        // Do not temper with DX since it can hold a carry 

    reg1 = generate_move_to_ax(frontend, frontend->instruction->arg1);    // Multiplication and division must use the AX register
    
    reg1->regLock = true;
    reg2 = generate_move_to_register(frontend, frontend->instruction->arg2);
    reg1->regLock = false;

    if (frontend->instruction->op == AST_MUL) {

        fprintf(frontend->targetProg, "MUL %s\n", generate_get_register_name(reg2));
    }
    else {

        // Xoring EDX is necessary because in division we divide EDX:EAX with the other register which means any 
        // value in EDX can ruin our calculations
        fprintf(frontend->targetProg, "PUSH EDX\n");
        fprintf(frontend->targetProg, "XOR EDX, EDX\n");
        fprintf(frontend->targetProg, "DIV %s\n", generate_get_register_name(reg2));
        fprintf(frontend->targetProg, "POP EDX\n");
    }

    frontend->registers[REG_DX]->regLock = false;        // Remove lock on EDX after we're done
    descriptor_push_tac(frontend, frontend->registers[REG_AX], frontend->instruction);    // Now AX will hold the result value so we can reset it to that value
}

/*
generate_condition generates Assembly code for a condition like <, == etc
Input: Backend
Output: None
*/
void generate_condition(asm_frontend* frontend) {

    register_T* reg1 = generate_move_to_register(frontend, frontend->instruction->arg1);

    reg1->regLock = true;
    register_T* reg2 = generate_move_to_register(frontend, frontend->instruction->arg2);
    reg1->regLock = false;

    // Generate a block exit operation because with the control flow that is occuring here,
    // we don't know if the variables changed inside a loop. For if statements we don't want the
    // change to affect our else statement
    frontend->table = frontend->table->nestedScopes[frontend->table->tableIndex];
    generate_block_exit(frontend);
    frontend->table = frontend->table->prev;


    fprintf(frontend->targetProg, "CMP %s, %s\n", generate_get_register_name(reg1), generate_get_register_name(reg2));
    descriptor_push_tac(frontend, generate_get_register(frontend), frontend->instruction);
}

/*
generate_if_false generates Assembly code for IFZ operations
Input: Backend
Output: None
*/
void generate_if_false(asm_frontend* frontend) {

    char* jmpCondition = NULL;

    if (frontend->instruction->arg1->type == TAC_P || frontend->instruction->arg1->type == TEMP_P) {

        // In all our cases when it comes to comparison, we want to do the exact opposite of what is specified
        switch (((TAC*)frontend->instruction->arg1->value)->op) {

        case TOKEN_LESS: jmpCondition = "JGE"; break;
        case TOKEN_ELESS: jmpCondition = "JG"; break;
        case TOKEN_MORE: jmpCondition = "JLE"; break;
        case TOKEN_EMORE: jmpCondition = "JL"; break;
        case TOKEN_DEQUAL: jmpCondition = "JNE"; break;
        case TOKEN_NEQUAL: jmpCondition = "JE"; break;

        }

        fprintf(frontend->targetProg, "%s %s\n", jmpCondition, generate_get_label(frontend, frontend->instruction->arg2->value));
    }
    // For a number or variable, we want to skip statement if it equals 0
    else {
        fprintf(frontend->targetProg, "CMP %s, 0\n", generate_get_register_name(generate_move_to_register(frontend, frontend->instruction->arg1)));
        fprintf(frontend->targetProg, "JE %s\n", generate_get_label(frontend, frontend->instruction->arg2->value));
    }
}

/*
generate_unconditional_jump generates an Assembly code for GOTO operations
Input: Backend
Outut: None
*/
void generate_unconditional_jump(asm_frontend* frontend) {

    fprintf(frontend->targetProg, "JMP %s\n", generate_get_label(frontend, frontend->instruction->arg1->value));
}

/*
generate_assignment generates Assembly code for assignments
Input: Backend
Ouput: None
*/
void generate_assignment(asm_frontend* frontend) {

    register_T* reg1 = NULL;
    register_T* reg2 = NULL;
    entry_T* entry = table_search_entry(frontend->table, frontend->instruction->arg1->value);

    if (entry->dtype == DATA_STRING) {

        fprintf(frontend->targetProg, "PUSHA\n");

        // MASM macro to copy a string value onto the string array
        fprintf(frontend->targetProg, "fnc lstrcpy, ADDR %s, \"%s\"\n", (char*)frontend->instruction->arg1->value, (char*)frontend->instruction->arg2->value);

        fprintf(frontend->targetProg, "POPA\n");

        return;
    }

    // If variable equals a function call then the value will return in AX, therefore we know that the register will always be AX
    if (frontend->instruction->arg2->type == TAC_P && ((TAC*)frontend->instruction->arg2->value)->op == AST_FUNC_CALL) {
        
        reg1 = frontend->registers[REG_AX];
    }
    // Otherwise, without a function call, we can use any register
    else {
        
        reg1 = generate_move_to_register(frontend, frontend->instruction->arg2);
    }

    address_reset(entry);

    address_push(entry, reg1, ADDRESS_REG);

    // Remove variable from all registers that held it's value
    while ((reg2 = generate_check_variable_in_reg(frontend, frontend->instruction->arg1))) {
        generate_remove_descriptor(reg2, frontend->instruction->arg1);
    }
        
    descriptor_push(reg1, frontend->instruction->arg1);    
}

/*
generate_var_dec generates Assembly code for variable declarations
Input: Backend
Output: None
*/
void generate_var_dec(asm_frontend* frontend) {

    char* name = NULL;

    if (!table_search_table(frontend->table, frontend->instruction->arg1->value)->prev) { return; }
        

    name = frontend->instruction->arg1->value;

    // If the second argument is a number, that means it's the amount of bytes to put in a string data
    // For other types, just declare them normally
    isNum(frontend->instruction->arg2->value) ? fprintf(frontend->targetProg, "LOCAL %s[%s]:BYTE\n", name, (char*)frontend->instruction->arg2->value)
        : fprintf(frontend->targetProg, "LOCAL %s:%s\n", name, (char*)frontend->instruction->arg2->value);
    
}  

/*
generate_main generates Assembly code for the main function of the source code
Input: Backend
Output: None
*/
void generate_main(asm_frontend* frontend) {

    TAC* triple = NULL;

    char* name = frontend->instruction->arg1->value;

    fprintf(frontend->targetProg, "main_start:\n");
    fprintf(frontend->targetProg, "CALL main\n");    // Call the actual main procedure
    fprintf(frontend->targetProg, "invoke ExitProcess, 0\n");
    fprintf(frontend->targetProg, "end main_start\n");
}

/*
generate_function generates Assembly code for every function but the main one, which is a procedure that returns
it's value in AX
Input: Backend
Output: None
*/
void generate_function(asm_frontend* frontend) {

    TAC* triple = NULL;
    arg_T* initValue = NULL;
    entry_T* entry = NULL;

    size_t variables = 0;
    size_t counter = atoi(frontend->instruction->next->arg1->value);

    char* name = frontend->instruction->arg1->value;
    char* varName = NULL;

    fprintf(frontend->targetProg, "%s PROC ", name);    // Generating function label

    frontend->table->tableIndex++;
    frontend->table = frontend->table->nestedScopes[frontend->table->tableIndex - 1];

    // Skipping number of local vars and start of block
    frontend->instruction = frontend->instruction->next->next->next;

    triple = frontend->instruction;

    // Generate all the local variables for the function
    if (counter > 0) {
        fprintf(frontend->targetProg, "%s:%s", frontend->table->entries[0]->name, dataToAsm(frontend->table->entries[0]->dtype));
    }
        
    for (unsigned int i = 1; i < counter; i++) {
        fprintf(frontend->targetProg, ", %s:%s", frontend->table->entries[i]->name, dataToAsm(frontend->table->entries[i]->dtype));
    }

    fprintf(frontend->targetProg, "\n");

    // First generate only the variable declarations
    while (frontend->instruction->op != TOKEN_FUNC_END) {

        if (frontend->instruction->op == AST_VARIABLE_DEC) {

            if (table_search_entry(frontend->table, frontend->instruction->arg1->value)->dtype != DATA_STRING) {
                variables++;
            }
                
            generate_asm(frontend);
        }

        frontend->instruction = frontend->instruction->next;
    }

    frontend->instruction = triple;

    // Go and assign a starting value to each declared variable
    for (unsigned int i = 0; i < variables;)  {

        if (frontend->instruction->op == AST_ASSIGNMENT 
            && table_search_entry(frontend->table, frontend->instruction->arg1->value)->dtype != DATA_STRING) {

            varName = frontend->instruction->arg1->value;
            initValue = frontend->instruction->arg2;

            generate_asm(frontend);
            fprintf(frontend->targetProg, "MOV [%s], %s\n", varName, generate_get_register_name(generate_find_register(frontend, initValue)));
            
            i++;
        }
                
        frontend->instruction = frontend->instruction->next;
    }
    
    frontend->instruction = triple;

    // Loop through the function and generate code for the statements
    while (frontend->instruction->op != TOKEN_FUNC_END) {

        if (frontend->instruction->op != AST_VARIABLE_DEC) {
            generate_asm(frontend);
        }
            
        frontend->instruction = frontend->instruction->next;
    }

    fprintf(frontend->targetProg, "%s ENDP\n", name);
}

/*
generate_return generates Assembly code for return operations
Input: Backend
Output: None
*/
void generate_return(asm_frontend* frontend) {

    register_T* reg = generate_move_to_ax(frontend, frontend->instruction->arg1);        // Always return a value in AX
    fprintf(frontend->targetProg, "RET\n");
}

/*
generate_func_call generates Assembly code for function calls
Input: Backend
Output: None
*/
void generate_func_call(asm_frontend* frontend) {

    char* name = frontend->instruction->arg1->value;
    size_t size = atoi(frontend->instruction->arg2->value);

    descriptor_push_tac(frontend, frontend->registers[0], frontend->instruction);    // Set temporary result to AX
    
    // Push all the variables to the stack, from last to first
    for (unsigned int i = 0; i < size;) {

        frontend->instruction = frontend->instruction->next;

        // For variables and numbers we can just push them as is
        if (frontend->instruction->op == AST_PARAM && frontend->instruction->arg1->type == CHAR_P) {

            fprintf(frontend->targetProg, "PUSH %s\n", (char*)frontend->instruction->arg1->value);
            i++;
        }
        // For TAC operations we need to allocate a register before pushing
        else if (frontend->instruction->op == AST_PARAM && (frontend->instruction->arg1->type == TAC_P || frontend->instruction->arg1->type == TEMP_P)) {

            fprintf(frontend->targetProg, "PUSH %s\n", generate_get_register_name(generate_move_to_register(frontend, frontend->instruction->arg1)));
            i++;
        }
        // There can be expression operations between parameters, so we need to generate code for them
        else {
            generate_asm(frontend);
        }
    }

    fprintf(frontend->targetProg, "CALL %s\n", name);
}

/*
generate_print generates Assembly code for the built in function print
Input: Backend
Output: None
*/
void generate_print(asm_frontend* frontend) {

    entry_T* entry = NULL;;

    size_t size = atoi(frontend->instruction->arg2->value);

    arg_T** regDescListList[GENERAL_REG_AMOUNT];

    // Save the register values we will change because of the macros
    for (unsigned int i = 0; i < GENERAL_REG_AMOUNT; i++) {

        regDescListList[i] = mcalloc(1, sizeof(arg_T));

        for (unsigned int i2 = 0; i2 < frontend->registers[i]->size; i2++) {
            regDescListList[i][i2] = frontend->registers[i]->regDescList[i2];
        }
    }
    
    fprintf(frontend->targetProg, "PUSHA\n");    // fnc will change register values, save previous values before doing so
    
    bool regsChanged = false;    // Optimization to check if registers could've actually changed

    // For each pushed param, produce an fnc StdOut instruction
    for (unsigned int i = 0; i < size; i++) {

        frontend->instruction = frontend->instruction->next;

        if (frontend->instruction->op == AST_PARAM && frontend->instruction->arg1->type == CHAR_P) {
            entry = table_search_entry(frontend->table, frontend->instruction->arg1->value);
        }

        // For data literals we just want to print them as is
        if (frontend->instruction->op == AST_PARAM && !entry) {

            if (!(frontend->instruction->arg1->type == TAC_P || frontend->instruction->arg1->type == TEMP_P)) {
                fprintf(frontend->targetProg, "fnc StdOut, \"%s\"\n", (char*)frontend->instruction->arg1->value);
            }
            else {
                // We need to return registers inside here because they could've changed
                if (regsChanged) { restore_save_registers(frontend); }

                fprintf(frontend->targetProg, "fnc StdOut, str$(%s)\n", generate_get_register_name(
                    generate_move_to_register(frontend, frontend->instruction->arg1)));
            }
                 
            regsChanged = true;
        }
        // For strings being pushed, produce fitting code
        else if (frontend->instruction->op == AST_PARAM && entry->dtype == DATA_STRING) {

            fprintf(frontend->targetProg, "fnc StdOut, ADDR %s\n", entry->name);
            regsChanged = true;
        }
        // For an integer, find or allocate a register to the value and print the value using the str$ macro
        // also for temporeries
        else if (frontend->instruction->op == AST_PARAM && entry->dtype == DATA_INT) {
            // We need to return registers inside here because they could've changed
            if (regsChanged) { restore_save_registers(frontend); }
            
            fprintf(frontend->targetProg, "fnc StdOut, str$(%s)\n", generate_get_register_name(generate_move_to_register(frontend, frontend->instruction->arg1)));
            regsChanged = true;
        }
        else {

            generate_asm(frontend);
            i--;
        }
    }

    /* Return back the values before the PUSHA instruction */

    fprintf(frontend->targetProg, "POPA\n");        // Return previous values to registers in Assembly code

    descriptor_reset_all_registers(frontend);

    for (unsigned int i = 0; i < GENERAL_REG_AMOUNT; i++) {

        for (unsigned int i2 = 0; i2 < frontend->registers[i]->size; i2++) {
            descriptor_push(frontend->registers[i], regDescListList[i][i2]);
        }
            
        free(regDescListList[i]);
    }

    
}

/*
generate_assign_reg returns a valid register / const number string depending on the given register
Input: A register, an argument to return if register is null
Output: A string that can either represent the given register or the given value, depending on if the register is null or not
*/
char* generate_assign_reg(register_T* r, void* argument) {
    return r ? generate_get_register_name(r) : argument;
}

/*
generate_check_variable_in_reg checks if a variable exists in any general purpose register, and if so it returns the register
Input: Register list, variable to search
Output: First register to contain the variable, NULL if none of them do
*/
register_T* generate_check_variable_in_reg(asm_frontend* frontend, arg_T* var) {

    register_T* reg = NULL;

    for (unsigned int i = 0; i < GENERAL_REG_AMOUNT && !reg; i++) {

        for (unsigned int i2 = 0; i2 < frontend->registers[i]->size; i2++) {

            if (generate_compare_arguments(var, frontend->registers[i]->regDescList[i2])) {

                reg = frontend->registers[i];
                break;
            }
        }
    }

    return reg;
}

/*
generate_find_free_reg finds a register that currently holds no values in it's descriptor
Input: Backend
Output: Free register if it exists, 0 if it doesn't
*/
register_T* generate_find_free_reg(asm_frontend* frontend) {

    register_T* reg = NULL;

    for (unsigned int i = 0; i < GENERAL_REG_AMOUNT && !reg; i++) {

        if (!frontend->registers[i]->size && !frontend->registers[i]->regLock) {
            reg = frontend->registers[i];
        }
    }

    return reg;
}

/*
generate_find_register finds a register for an argument, if the argument does not exist in any register it loads it in one
Input: Backend struct with instruction, argument to load register for
Output: Fitting register
*/
register_T* generate_find_register(asm_frontend* frontend, arg_T* arg) {

    register_T* reg = generate_check_variable_in_reg(frontend, arg);    // Check if variable is in a register
    
    if (!reg) {
        reg = generate_get_register(frontend);
    }
        
    return reg;
}

/*
generate_get_register uses all functions that try to find an available register to use and returns it
Input: Backend
Output: Available register
*/
register_T* generate_get_register(asm_frontend* frontend) {

    register_T* reg = generate_find_free_reg(frontend);

    // Check if there's a register that can be used despite being occupied
    if (!reg) {
        reg = generate_find_used_reg(frontend);        
    }
        
    return reg;
}

/*
generate_move_to_register searches for an available register and produces Assembly code to 
store a value in that register if it's not already there
Input: Backend, Argument that contains a value we want to put in a register
Output: Available register
*/
register_T* generate_move_to_register(asm_frontend* frontend, arg_T* arg) {

    register_T* reg = generate_check_variable_in_reg(frontend, arg);        // Check if variable is in a register
    entry_T* entry = NULL;
    char* name = NULL;

    if (reg) { return reg; }
    
    entry = table_search_entry(frontend->table, arg->value);    // Search for the entry

    reg = generate_get_register(frontend);

    name = generate_get_register_name(reg);

    // Push the new variable descriptor onto the register
    descriptor_push(reg, arg);

    // If entry exists and value was not found in any register previously, store it in the found available register
    if (entry) {

        address_push(entry, reg, ADDRESS_REG);
        fprintf(frontend->targetProg, "MOV %s, [%s]\n", name, (char*)arg->value);
    }
    // Otherwise, if we want to move a number to a register, check if that number is 0, if so generate a XOR
    // instruction, and if it isn't just load it's value onto the register
    else if (arg->type == CHAR_P) {

        !strcmp(arg->value, "0") ? fprintf(frontend->targetProg, "XOR %s, %s\n", name, name)
            : fprintf(frontend->targetProg, "MOV %s, %s\n", name, (char*)arg->value);    
    }
    
    return reg;
}

/*
generate_move_new_value_to_register searches for an available register and puts the desired value in it
but it doesn't search for a register that already contains the argument value
Input: Backend, Argument to find register for
Ouput: Available register
*/
register_T* generate_move_new_value_to_register(asm_frontend* frontend, arg_T* arg) {

    register_T* reg = NULL;
    entry_T* entry = NULL;
    char* name = NULL;

    entry = table_search_entry(frontend->table, arg->value);

    reg = generate_get_register(frontend);

    name = generate_get_register_name(reg);

    // Push the new variable descriptor onto the register
    descriptor_push(reg, arg);

    if (entry) {

        address_push(entry, reg, ADDRESS_REG);
        fprintf(frontend->targetProg, "MOV %s, [%s]\n", name, (char*)arg->value);
    }
    else {

        !strcmp(arg->value, "0") ? fprintf(frontend->targetProg, "XOR %s %s\n", name, name)
            : fprintf(frontend->targetProg, "MOV %s, %s\n", name, (char*)arg->value);        
    }

    return reg;
}

/*
generate_move_to_ax moves a value only to the AX register
Input: Backend, argument to move to AX
Output: AX register
*/
register_T* generate_move_to_ax(asm_frontend* frontend, arg_T* arg) {

    register_T* reg = generate_check_variable_in_reg(frontend, arg);
    arg_T** regDescList = NULL;

    if (!reg) {

        // If the register found was not AX, copy AX's contents to it to free AX
        if ((reg = generate_get_register(frontend))->reg != REG_AX) {

            for (unsigned int i = 0; i < frontend->registers[REG_AX]->size; i++) {
                descriptor_push(reg, frontend->registers[REG_AX]->regDescList[i]);
            }
            
            fprintf(frontend->targetProg, "MOV %s, EAX\n", generate_get_register_name(reg));        // Move the value of AX to a different register

            // Reset AX
            frontend->registers[REG_AX]->size = 0;
            free(frontend->registers[REG_AX]->regDescList);
            frontend->registers[REG_AX]->regDescList = NULL;
        }

        reg = generate_move_to_register(frontend, arg);
    }
    else if (reg->reg != REG_AX) {

        fprintf(frontend->targetProg, "XCHG EAX, %s\n", generate_get_register_name(reg));

        // Switch their register descriptors
        regDescList = frontend->registers[REG_AX]->regDescList;
        frontend->registers[REG_AX]->regDescList = reg->regDescList;
        reg->regDescList = regDescList;
        
    }

    return reg;
}

/*
generate_find_lowest_values finds and returns the register with the lowest amount of values stored
Input: Register list
Output: Register with lowest variables
*/
register_T* generate_find_lowest_values(asm_frontend* frontend) {

    size_t loc = 0;

    // Start comparing to the first available register
    while (frontend->registers[loc]->regLock) { loc++; }

    // Start comparing all registers to find the lowest value
    for (unsigned int i = 1; i < GENERAL_REG_AMOUNT; i++) {

        if (frontend->registers[i]->size < frontend->registers[loc]->size
            && !frontend->registers[i]->regLock) {

            loc = i;
        }
    }

    return frontend->registers[loc];
}

/*
generate_find_used_reg finds an available register when all registers are used
Input: Backend
Output: Newly available register
*/
register_T* generate_find_used_reg(asm_frontend* frontend) {

    register_T* reg = NULL;
    entry_T* entry = NULL;

    // Going through all the variables in all the registers and searching to see if there's a register
    // that has all it's values stored somewhere else as well, if so, we can use that register
    for (unsigned int i = 0; i < GENERAL_REG_AMOUNT && !reg; i++) {

        if (frontend->registers[i]->regLock) {
            continue;
        }
            
        reg = frontend->registers[i];

        for (unsigned int i2 = 0; i2 < frontend->registers[i]->size && reg; i2++) {

            // If there's a temporary in the register, we cannot use the register
            if (frontend->registers[i]->regDescList[i2]->type == TAC_P 
                || frontend->registers[i]->regDescList[i2]->type == TEMP_P) {

                reg = NULL;
            }
            else if (entry = table_search_entry(frontend->table, frontend->registers[i]->regDescList[i2]->value)) {

                if (entry->size <= 1) {
                    reg = NULL;
                }    
            }
        }
    }
    // Check that the variable the program is assigning to doesn't equal both operands
    // and if it doesn't, we can use a register that contains the variable
    for (unsigned int i = 0; i < GENERAL_REG_AMOUNT && !reg; i++) {

        if (frontend->instruction->next && frontend->instruction->next->op == AST_ASSIGNMENT
            && frontend->instruction->next->arg1->type != TAC_P && !frontend->registers[i]->regLock) {

            reg = generate_check_useless_value(frontend, frontend->registers[i]);
        }
    }
    
    // Going through registers and checking if there's a register that holds values that won't be used again
    for (unsigned int i = 0; i < GENERAL_REG_AMOUNT && !reg; i++) {
        if (frontend->registers[i]->regLock) {
            continue;
        }
            
        reg = generate_check_register_usability(frontend, frontend->registers[i]);    
    }
    // If there are no usable registers, we need to spill the values of one of the registers
    if (!reg) {

        reg = generate_find_lowest_values(frontend);
        generate_spill(frontend, reg);
    }
    // Now that the variable is saved somewhere else, we need to free the previous values stored in it and reset the size
    if (reg->regDescList) {

        descriptor_reset(frontend, reg);
    }
    
    return reg;
}

/*
generate_check_useless_value checks if the register holds a value that is going to disappear through an assignment instruction
Input: Backend, register to search in
Output: Returns the register if it contains useless value, NULL if it doesn't
*/
register_T* generate_check_useless_value(asm_frontend* frontend, register_T* r) {

    register_T* reg = r;
    
    // If we specified earlier to not use that register
    if (r->regLock || frontend->instruction->next && frontend->instruction->next->op != AST_ASSIGNMENT) {
        return NULL;
    }
        

    for (unsigned int i = 0; i < r->size && reg; i++) {
    
        if (frontend->instruction->next && (generate_compare_arguments(frontend->instruction->next->arg1->value, frontend->instruction->arg1->value)
            || generate_compare_arguments(frontend->instruction->next->arg1->value, frontend->instruction->arg2->value)
            || !generate_compare_arguments(frontend->instruction->next->arg1->value, r->regDescList[i]->value))) {
            reg = NULL;
        }
    }

    return reg;
}

/*
generate_check_register_usability goes through a given register's descriptor list to see if the values that a register holds will not be used again
Input: Backend, register to search in
Output: Returns the register if it contains unused values, NULL if not
*/
register_T* generate_check_register_usability(asm_frontend* frontend, register_T* r) {

    register_T* reg = NULL;

    for (unsigned int i = 0; i < r->size; i++) {
        reg = generate_check_variable_usability(frontend, r, r->regDescList[i]);

        if (!reg) {
            break;
        }    
    }

    return reg;
}

/*
generate_check_variable_usability goes through a block to see if a specific value that a register holds will not be used again
Input: Backend, register to search in
Output: Returns the register if it contains an unused value, NULL if not
*/
register_T* generate_check_variable_usability(asm_frontend* frontend, register_T* r, arg_T* arg) {

    TAC* triple = frontend->instruction;
    register_T* reg = r;

    // Loop through instructions until the end of the block to see if we can use a register that holds
    // a value that will not be used
    while (triple->op != TOKEN_RBRACE && reg) {

        // For the first argument, if we are assigning to the variable, then we are okay to use register
        if ((triple->op != AST_ASSIGNMENT && triple->arg1 && generate_compare_arguments(triple->arg1, arg))
            || (triple->arg2 && generate_compare_arguments(triple->arg2, arg)) || reg->regLock) {
            reg = NULL;
        }
        
        triple = triple->next;
    }

    return reg;
}

/*
generate spill stores the actual value of a variable in it, after being changed by operations such as assignment
Input: Backend, register to spill variables in
Output: None
*/
void generate_spill(asm_frontend* frontend, register_T* r) {

    entry_T* entry = NULL;

    // For each value, store the value of the variable in itself
    for (unsigned int i = 0; i < r->size; i++) {

        if ((entry = table_search_entry(frontend->table, r->regDescList[i]->value))) {

            fprintf(frontend->targetProg, "MOV [%s], %s\n", (char*)r->regDescList[i]->value, generate_get_register_name(r));
            address_push(entry, r->regDescList[i]->value, ADDRESS_VAR);
        }
    }
}

/*
generate_block_exit generates Assembly code for when we exit a block, it stores all later-needed values from the registers
back to the variables
Input: Backend
Output: None
*/
void generate_block_exit(asm_frontend* frontend) {

    for (unsigned int i = 0; i < GENERAL_REG_AMOUNT; i++) {
        register_block_exit(frontend, frontend->registers[i]);
    }
}
        

/*
register_block_exit checks if a variable currently holds it's correct data, if not, it loads it before exiting a scope
Input: Backend, register to check variables in
Output: None
*/
void register_block_exit(asm_frontend* frontend, register_T* reg) {

    entry_T* entry = NULL;

    // For each value stored in the register, check if that value is different from what the actual variable
    // holds, if it is, store the correct value in the variable before exiting a scope
    for (unsigned int i = 0; i < reg->size; i++) {

        entry = table_search_entry(frontend->table, reg->regDescList[i]->value);

        // Only check variables in registers, because only they can be alive on exit
        // Also if there is no entry, we can continue searching
        if (reg->regDescList[i]->type != CHAR_P || !entry) {
            continue;
        }
            
        // For values that are live on exit (values that exist in some parent symbol table)
        if (!table_search_in_specific_table(frontend->table, reg->regDescList[i]->value)
            && !entry_search_var(entry, reg->regDescList[i]->value)) {        // Here we check if the variable doesn't hold it's own value 

            fprintf(frontend->targetProg, "MOV [%s], %s\n", (char*)reg->regDescList[i]->value, generate_get_register_name(reg));
            address_remove_registers(entry);
            address_push(entry, reg->regDescList[i]->value, ADDRESS_VAR);
        }
        // We can reset the addresses for values that are not live on exit
        else if (table_search_in_specific_table(frontend->table, reg->regDescList[i]->value)) {
            address_reset(entry);
        }
    }
}

/*
generate_get_label generates a label name from a TAC instruction address
Input: Backend, label instruction
Output: Label name
*/
char* generate_get_label(asm_frontend* frontend, TAC* label) {

    char* name = NULL;
    
    // Search for label and return it
    for (unsigned int i = 0; i < frontend->labelList->size && !name; i++) {

        if (label == frontend->labelList->labels[i]) {
            name = frontend->labelList->names[i];
        }
    }
    // If label was not found, create a new one
    if (!name) {

        frontend->labelList->labels = mrealloc(frontend->labelList->labels, sizeof(TAC*) * ++frontend->labelList->size);    // Appending list
        frontend->labelList->labels[frontend->labelList->size - 1] = label;
        frontend->labelList->names = mrealloc(frontend->labelList->names, sizeof(char*) * frontend->labelList->size);
        name = mcalloc(1, strlen("label") + numOfDigits(frontend->labelList->size) + 1);
        sprintf(name, "label%zu", frontend->labelList->size);
        frontend->labelList->names[frontend->labelList->size - 1] = name;
    }

    return name;
}

/*
generate_get_register_name gets a name for a register from it's type
Input: Register to get the name of
Output: Register name
*/
char* generate_get_register_name(register_T* r) {

    // Switch types
    switch (r->reg) {

    case REG_AX: return "EAX";
    case REG_BX: return "EBX";
    case REG_CX: return "ECX";
    case REG_DX: return "EDX";
    case REG_CS: return "CS";
    case REG_DS: return "DS";
    case REG_SS: return "SS";
    case REG_SP: return "ESP";
    case REG_SI: return "ESI";
    case REG_DI: return "EDI";
    case REG_BP: return "EBP";
    
    default:     return NULL;

    }
}

/*
generate_compare_arguments takes two arguments and compares them according to their types
Input: First argument, second argument
Output: True if they are equal, otherwise false
*/
bool generate_compare_arguments(arg_T* arg1, arg_T* arg2) {

    bool flag = (arg1->type == TAC_P || arg1->type == TEMP_P) && (arg2->type == TAC_P || arg2->type == TEMP_P) && arg1->value == arg2->value;

    if (!flag) {
        flag = arg1->type == CHAR_P && arg2->type == CHAR_P && !strcmp(arg1->value, arg2->value);
    }
        
    return flag;
}

/*
generate_remove_descriptor removes a specific address from the register descriptor
Input: Register to remove from, address to remove
Output: None
*/
void generate_remove_descriptor(register_T* reg, arg_T* desc) {

    unsigned int index = 0;

    if (!reg) { return; }
        
    for (unsigned int i = 0; i < reg->size; i++) {

        if (!generate_compare_arguments(reg->regDescList[i], desc)) {

            reg->regDescList[index] = reg->regDescList[index];
            index++;
        }
    }

    reg->regDescList = mrealloc(reg->regDescList, --reg->size * sizeof(arg_T*));
}

/*
restore_save_registers saves registers in Assembly code
Input: Backend
Output: None
*/
void restore_save_registers(asm_frontend* frontend) {

    fprintf(frontend->targetProg, "POPA\n");
    fprintf(frontend->targetProg, "PUSHA\n");
}

/*
free_registers frees all the registers and their descriptors
Input: Backend
Output: None
*/
void free_registers(asm_frontend* frontend) {

    for (unsigned int i = 0; i < REG_AMOUNT; i++) {

        descriptor_reset(frontend, frontend->registers[i]);
        free(frontend->registers[i]);
    }

    free(frontend->registers);

    if (frontend->labelList) {

        for (unsigned int i = 0; i < frontend->labelList->size; i++) {
            free(frontend->labelList->names[i]);
        }
            
        free(frontend->labelList->names);
        free(frontend->labelList->labels);
        free(frontend->labelList);
    }
    
    free(frontend);
}
