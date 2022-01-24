#ifndef LLVM_COMPILER
#define LLVM_COMPILER

#include <string>
#include <vector>

#include "bp.hpp"
#include "sym_table.hpp"

class union_class{
	public:
        std::string data;
        std::string type;
        std::string label;

        bool is_const = false;
        bool is_literal = false;

        vector<pair<int,BranchLabelIndex>> truelist;
        vector<pair<int,BranchLabelIndex>> falselist;
        vector<pair<int,BranchLabelIndex>> nextlist;
};

#define YYSTYPE union_class

class Llvm_compiler
{
private:
    /* data */
    static CodeBuffer& code_bp;
    Sym_table symbol_table;

    unsigned int regs_counter = 0;
    unsigned int string_counter = 0;
    std::string stack_ptr_reg;

    vector<std::string> nested_while_labels;
    vector<union_class> args_list;

    /*-----Private Functions-----*/



    //generate new register name
    std::string generate_reg();

    //return llvm type size
    std::string typeSize(std::string type);

    //generate code for trunc reg to desired size and return the new trunced reg
    std::string trunc(std::string reg , std::string cur_size , std::string res_size);

    //generate code for zext reg to desired size and return the new zext reg
    std::string zext(std::string reg , std::string cur_size , std::string res_size);

    //generate code for assigning bool type
    std::string assign_bool(union_class& bool_exp);
    


public:
    Llvm_compiler();
    ~Llvm_compiler();

    union_class merge_lists(union_class& uni_1, union_class& uni_2);

    /*------------------------------Statements Handlers-----------------------------------*/
    void openScope();
    void closeScope();
    
    //Add var name to the symbol table and generate decleration code to the code buffer
    void handle_var_decl(std::string& type, std::string& name);
    void handle_var_decl(bool is_const, std::string& type, std::string& name, union_class& assign_exp);
    
    //Generate code for assigning assign_exp data in dest_id variable
    void handle_assign(std::string dest_id, union_class& assign_exp);
    
    
    union_class begin_else();
    //Backpatch exp truelist to the next block label and closes the if scope, return states nextlist
    union_class end_if(union_class& exp, union_class& states);
    //Backpatch exp truelist to the else block label and closes the else scope, return merged if and else states nextlist
    union_class end_else(union_class& exp, union_class& if_states, union_class& else_exp,union_class& else_states);

    //Generate nested label for while scope
    void begin_while();
    //Backpatch exp truelist and states nextlist to the next block label, generate loop jump
    //and closes the while scope
    void end_while(union_class& exp, union_class& states);
    
    union_class handle_break();
    void handle_continue();

    union_class handle_cond(union_class& exp);

    union_class handle_call(std::string func_id);


    /*-------------------------------- Exp Handlers ------------------------------------*/

    //Handle use of a variable with id = var_id
    union_class handle_var(std::string var_id);

            /*----------------------- Binop Handlers ---------------------*/
    union_class handle_add(union_class& exp_1, union_class& exp_2);
    union_class handle_sub(union_class& exp_1, union_class& exp_2);
    union_class handle_mul(union_class& exp_1, union_class& exp_2);
    union_class handle_div(union_class& exp_1, union_class& exp_2);
            /*------------------------------------------------------------*/
    
    // Generate code for relop/equalop equation between exp_1 and exp_2 by given op string: "[<>]=?|[!=]="
    union_class handle_relop_equalop(union_class& exp_1,std::string op, union_class& exp_2); 

    // Handle literals
    union_class handle_literal(union_class& exp, std::string type);

    // Handle casting
    union_class handle_cast(std::string cast_type, union_class& exp);

    // Handle strings implementation
    union_class handle_string(std::string str);

            /*----------------------- Boolean Handlers -------------------*/
    union_class handle_not(union_class& exp);
    union_class handle_or(union_class& exp_1, std::string or_label, union_class& exp_2);
    union_class handle_and(union_class& exp_1, std::string or_label, union_class& exp_2);

    //Add branch to exp and generate label after br and return the label
    std::string add_br_and_label(union_class& exp);

    /*------------------------------ Function Handlers ---------------------------------*/
    void add_call_arg(union_class& exp);
    void add_func_arg(union_class& exp);
    void handle_func_decl(std::string ret_type,std::string func_id);
    void handle_func_end(std::string ret_type);
};


#endif