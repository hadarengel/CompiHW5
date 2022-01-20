

#include "llvm_compiler.hpp"




/*----------- Public Functions -----------*/

union_class Llvm_compiler::handle_add(union_class& exp_1, union_class& exp_2){
    union_class res_exp;
    res_exp.type = symbol_table.findBinopType(exp_1.type,exp_2.type);
    res_exp.data = generate_reg();


}

/*----------- Private Functions ----------*/


std::string Llvm_compiler::generate_reg(){
    return std::string("reg_") + std::to_string(regs_counter++);
}




