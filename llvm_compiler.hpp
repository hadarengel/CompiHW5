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
    	std::string name;
        std::vector<std::string> params_types;
        bool is_const;
};

#define YYSTYPE union_class

class Llvm_compiler
{
private:
    /* data */
    CodeBuffer& code_bp;
    Sym_table symbol_table;

    unsigned int regs_counter = 0;

    /*-----Private Functions-----*/

    //generate new register name
    std::string generate_reg();


public:
    Llvm_compiler();
    ~Llvm_compiler();

    union_class handle_add(union_class& exp_1, union_class& exp_2); 
};


#endif