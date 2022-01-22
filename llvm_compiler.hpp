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
        std::vector<std::string> params_types;
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
    CodeBuffer& code_bp;
    Sym_table symbol_table;

    unsigned int regs_counter = 0;
    std::string stack_ptr_reg;

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

    //add label in code
    //note: use it for generating mid labels in boolean ops and pre labels in loops
    union_class add_label();

    /*------------------------------Statements Handlers-----------------------------------*/
    //Add var name to the symbol table and generate decleration code to the code buffer
    void handle_var_decl(std::string& type, std::string& name);
    void handle_var_decl(bool is_const, std::string& type, std::string& name, union_class& assign_exp);
    
    //Generate code for assigning assign_exp data in dest_id variable
    void handle_assign(std::string dest_id, union_class& assign_exp);

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

            /*----------------------- Boolean Handlers -------------------*/
    union_class handle_not(union_class& exp);
    union_class handle_or(union_class& exp_1, union_class& exp_2);
};


#endif