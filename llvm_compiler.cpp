

#include "llvm_compiler.hpp"

extern int yylineno;


/*----------- Public Functions -----------*/
union_class Llvm_compiler::add_label(){
    union_class ret;
    ret.label = code_bp.genLabel();
    return ret;
}

void Llvm_compiler::handle_var_decl(std::string& type, std::string& name){
    symbol_table.addVar(name,type);
    std::string reg_ptr = generate_reg() + "_address_to_" + name;
    symbol_table.updateReg(name,reg_ptr);
    std::string code = reg_ptr + " = getelementptr i32, i32* " + stack_ptr_reg + " , i32 " + to_string(symbol_table.getOffset(name));
    code_bp.emit(code);
    code = "store i32 0, i32* " + reg_ptr;
    code_bp.emit(code);

}

void Llvm_compiler::handle_var_decl(bool is_const, std::string& type, std::string& name, union_class& assign_exp){
    symbol_table.checkValidAssign(type,assign_exp.type);
    
    if(is_const && assign_exp.is_literal){
        symbol_table.addVar(name,type,is_const,true);
        symbol_table.updateReg(name,assign_exp.data);
    }
    else{
        symbol_table.addVar(name,type,is_const,false);
        std::string reg_ptr = generate_reg() + "_address_to_" + name;
        symbol_table.updateReg(name,reg_ptr);
        std::string code = reg_ptr + " = getelementptr i32, i32* " + stack_ptr_reg + " , i32 " + to_string(symbol_table.getOffset(name));
        code_bp.emit(code);
        std::string assign_data = assign_exp.data;
        std::string assign_type = typeSize(assign_exp.type);
        if(assign_type != "i32" && !assign_exp.is_literal){
            if(assign_type == "i1"){
                assign_data = assign_bool(assign_exp);
            }
            else{
                assign_data = zext(assign_data,assign_type,"i32");
            }
        }
        code = "store i32 " + assign_data + " , i32* " + reg_ptr;
        code_bp.emit(code);
    }
}


void Llvm_compiler::handle_assign(std::string dest_id, union_class& assign_exp){
    if(symbol_table.checkValidVar(dest_id) == true){    //check dest_id is existing variable and not const
        output::errorConstMismatch(yylineno);
        symbol_table.abortParser(1);
    }
    symbol_table.checkValidAssign(symbol_table.typeOfSymbol(dest_id),assign_exp.type);
    std::string assign_data = assign_exp.data;
    std::string assign_type = typeSize(assign_exp.type);
    
    if(assign_type != "i32" && !assign_exp.is_literal){
        if(assign_type == "i1"){
            assign_data = assign_bool(assign_exp);
        }
        else{
            assign_data = zext(assign_data,assign_type,"i32");
        }
    }
    std::string code = "store i32 " + assign_data + " , i32* " + symbol_table.getReg(dest_id);
    code_bp.emit(code);
}

union_class Llvm_compiler::handle_var(std::string var_id){
    bool is_const = symbol_table.checkValidVar(var_id);
    union_class new_exp;
    new_exp.type = symbol_table.typeOfSymbol(var_id);
    new_exp.is_const = is_const;
    std::string var_ptr = symbol_table.getReg(var_id);
    if(symbol_table.checkLiteral(var_id)){
        new_exp.is_literal = true;
        new_exp.data = var_ptr; //var_ptr contain the literal value
    }
    else{
        new_exp.is_literal = false;
        new_exp.data = generate_reg();
        std::string code = new_exp.data + " = load " + typeSize(new_exp.type) + " , i32* " + var_ptr; 
        code_bp.emit(code);
    }
    return new_exp;
}

//TODO add overflow handle  
union_class Llvm_compiler::handle_add(union_class& exp_1, union_class& exp_2){
    union_class res_exp;
    res_exp.type = symbol_table.findBinopType(exp_1.type,exp_2.type);
    res_exp.data = generate_reg();
    std::string data1 = exp_1.data;
    std::string data2 = exp_2.data;
    std::string code;

    if(res_exp.type == "INT"){
        if(exp_1.type == "BYTE"){
            data1 = zext(data1,"i8","i32");
        }
        if(exp_2.type == "BYTE"){
            data2 = zext(data2,"i8","i32");
        }
    }
    
    code = res_exp.data + " = add " + typeSize(res_exp.type) + " " +data1 + " , " + data2;
    code_bp.emit(code);

    return res_exp;
    
}

union_class Llvm_compiler::handle_sub(union_class& exp_1, union_class& exp_2){
    union_class res_exp;
    res_exp.type = symbol_table.findBinopType(exp_1.type,exp_2.type);
    res_exp.data = generate_reg();
    std::string data1 = exp_1.data;
    std::string data2 = exp_2.data;
    std::string code;

    if(res_exp.type == "INT"){
        if(exp_1.type == "BYTE"){
            data1 = zext(data1,"i8","i32");
        }
        if(exp_2.type == "BYTE"){
            data2 = zext(data2,"i8","i32");
        }
    }
    
    code = res_exp.data + " = sub " + typeSize(res_exp.type) + " " +data1 + " , " + data2;
    code_bp.emit(code);

    return res_exp;
}

union_class Llvm_compiler::handle_mul(union_class& exp_1, union_class& exp_2){
    union_class res_exp;
    res_exp.type = symbol_table.findBinopType(exp_1.type,exp_2.type);
    res_exp.data = generate_reg();
    std::string data1 = exp_1.data;
    std::string data2 = exp_2.data;
    std::string code;

    if(res_exp.type == "INT"){
        if(exp_1.type == "BYTE"){
            data1 = zext(data1,"i8","i32");
        }
        if(exp_2.type == "BYTE"){
            data2 = zext(data2,"i8","i32");
        }
    }
    
    code = res_exp.data + " = mul " + typeSize(res_exp.type) + " " +data1 + " , " + data2;
    code_bp.emit(code);

    return res_exp;
}

union_class Llvm_compiler::handle_div(union_class& exp_1, union_class& exp_2){
    union_class res_exp;
    res_exp.type = symbol_table.findBinopType(exp_1.type,exp_2.type);
    res_exp.data = generate_reg();
    std::string op = res_exp.type == "BYTE" ? "udiv" : "sdiv";
    std::string data1 = exp_1.data;
    std::string data2 = exp_2.data;
    std::string code;
    /*--TODO add check div by zero --*/



    /*-------------------------------*/

    if(res_exp.type == "INT"){
        if(exp_1.type == "BYTE"){
            data1 = zext(data1,"i8","i32");
        }
        if(exp_2.type == "BYTE"){
            data2 = zext(data2,"i8","i32");
        }
    }
    
    code = res_exp.data + " = " + op +" " + typeSize(res_exp.type) + " " +data1 + " , " + data2;
    code_bp.emit(code);

    return res_exp;
    
}

union_class Llvm_compiler::handle_relop_equalop(union_class& exp_1,std::string op, union_class& exp_2){
    symbol_table.checkNumberType(exp_1.type);
    symbol_table.checkNumberType(exp_2.type);
    std::string op_type = symbol_table.findBinopType(exp_1.type,exp_2.type);
    string llvm_op = "";
    if (op == "==") 
        llvm_op = "eq";
    else if (op == "!=")
         llvm_op = "ne";
    else if (op == ">")
         llvm_op =  op_type == "BYTE" ? "ugt" :"sgt";
    else if (op == "<") 
        llvm_op =  op_type == "BYTE" ? "ult" : "slt";
    else if (op == ">=")
         llvm_op =  op_type == "BYTE" ?"uge" : "sge";
    else if (op == "<=") 
        llvm_op =  op_type == "BYTE" ? "ule" : "sle";

    std::string exp1_data = exp_1.data;
    std::string exp2_data = exp_2.data;
   
    if(op_type != exp_1.type && !exp_1.is_literal){
        exp1_data = zext(exp1_data,typeSize(exp_1.type),typeSize(op_type));
    }
    if(op_type != exp_2.type && !exp_2.is_literal){
        exp2_data = zext(exp2_data,typeSize(exp_2.type),typeSize(op_type));
    }
    union_class res_exp;

    res_exp.type = "BOOL";
    res_exp.data = generate_reg();
    std::string code = res_exp.data + " = icmp " + llvm_op + " " + typeSize(op_type) + " " + exp1_data + " , " + exp2_data;
    code_bp.emit(code);

    return res_exp;
}

union_class Llvm_compiler::handle_literal(union_class& exp, std::string type){
    int value = atoi(exp.data.c_str());
    if(type == "BYTE"){
        symbol_table.checkOverFlowByte(value);    
    }
    union_class res_exp;
    res_exp.data = exp.data;
    res_exp.is_literal = true;
    res_exp.type = type;
    return res_exp;
}

union_class Llvm_compiler::handle_cast(std::string cast_type, union_class& exp){
    symbol_table.checkValidCast(cast_type,exp.type);
    union_class casted_exp = exp;
    casted_exp.type = cast_type;
    if(exp.is_literal || casted_exp.type == exp.type){
        return casted_exp;
    }
    else{ // must be (int)byte or (byte)int
        if(casted_exp.type == "INT"){
            casted_exp.data = zext(exp.data,typeSize(exp.type),typeSize(casted_exp.type));
        }
        else{
            casted_exp.data = trunc(exp.data,typeSize(exp.type),typeSize(casted_exp.type));
        }
    }
    return casted_exp;
}


union_class Llvm_compiler::handle_not(union_class& exp){
    symbol_table.checkBoolType(exp.type);
    union_class res_exp = exp;
    if(exp.is_literal){
        res_exp.data = exp.data == "1" ? "0" : "1";
    }
    res_exp.truelist = exp.falselist;
    res_exp.falselist = exp.truelist;

    return res_exp;
}

union_class Llvm_compiler::handle_or(union_class& exp_1, std::string or_label, union_class& exp_2){
    symbol_table.checkBoolType(exp_1.type);
    symbol_table.checkBoolType(exp_2.type);
    union_class res_exp = exp_2;
    res_exp.is_literal = false;
    code_bp.bpatch(exp_1.falselist,or_label);
    res_exp.truelist = code_bp.merge(exp_1.truelist,exp_2.truelist);
    return res_exp;
}

union_class Llvm_compiler::handle_and(union_class& exp_1, std::string or_label, union_class& exp_2){
    symbol_table.checkBoolType(exp_1.type);
    symbol_table.checkBoolType(exp_2.type);
    union_class res_exp = exp_2;
    res_exp.is_literal = false;
    code_bp.bpatch(exp_1.truelist,or_label);
    res_exp.falselist = code_bp.merge(exp_1.falselist,exp_2.falselist);
    return res_exp;
}


std::string Llvm_compiler::add_br_and_label(union_class& exp){
    symbol_table.checkBoolType(exp.type);
    std::string code = "br i1 " + exp.data +", label @, label @";
    int loc = code_bp.emit(code);
    exp.truelist = code_bp.merge(exp.truelist,code_bp.makelist({loc,FIRST}));
    exp.falselist = code_bp.merge(exp.falselist,code_bp.makelist({loc,SECOND}));
    return code_bp.genLabel();
}


union_class Llvm_compiler::handle_string(std::string str) {

    union_class res_exp;
    res_exp.type = "STRING";
    string str_name = "@.str" + to_string(string_counter++);

    str.erase(0, 1); // delete start "
    str.erase(str.size() - 1); //delete end "
    int str_size= str.size() + 1;
    
    code_bp.emitGlobal(str_name + " = constant [" + to_string(str_size) + " x i8] c\"" + str + "\\00\"");

    res_exp.data = generate_reg();
    code_bp.emit(res_exp.data + " = getelementptr inbounds [" +to_string(str_size) + " x i8], [" + to_string(str_size) + " x i8]* " + str_name + ", i32 0, i32 0");
    
    return res_exp;
}




/*----------- Private Functions ----------*/


std::string Llvm_compiler::generate_reg(){
    return std::string("%reg_") + std::to_string(regs_counter++);
}


std::string Llvm_compiler::typeSize(std::string type){
    if(type == "BOOL"){
        return "i1";
    }
    if(type == "BYTE"){
        return "i8";
    }
    return "i32";
}


std::string Llvm_compiler::trunc(std::string reg , std::string cur_size , std::string res_size){
    std::string res = generate_reg();
    std::string code = res + " = trunc " + cur_size +" " + reg + " to " + res_size;
    code_bp.emit(code);
    return res;
}

std::string Llvm_compiler::zext(std::string reg , std::string cur_size , std::string res_size){
    std::string res = generate_reg();
    std::string code = res + " = zext " + cur_size +" " + reg + " to " + res_size;
    code_bp.emit(code);
    return res;
}


std::string Llvm_compiler::assign_bool(union_class& bool_exp) {
    std::string bool_reg = generate_reg();
    std::string true_label = code_bp.genLabel();
    int backpatch_for_true =  code_bp.emit("br label @");
    std::string false_label = code_bp.genLabel();
    int backpatch_for_false =  code_bp.emit("br label @");

    string phi_label = code_bp.genLabel();
    code_bp.emit(bool_reg + " = phi i32 [  1, %" + true_label + "], [  0, %" + false_label + "]");

    code_bp.bpatch(bool_exp.truelist, true_label);
    code_bp.bpatch(bool_exp.falselist, false_label);

    code_bp.bpatch(code_bp.makelist({backpatch_for_true, FIRST}), phi_label);
    code_bp.bpatch(code_bp.makelist({backpatch_for_false, FIRST}), phi_label);

    return bool_reg;
}

