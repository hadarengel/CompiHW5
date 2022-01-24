

#include "llvm_compiler.hpp"

extern int yylineno;


/*----------- Public Functions -----------*/

Llvm_compiler::Llvm_compiler(): code_bp(CodeBuffer::instance()){
    code_bp.emitGlobal("declare i32 @printf(i8*, ...)");
    code_bp.emitGlobal("declare void @exit(i32)");
    code_bp.emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    code_bp.emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    code_bp.emitGlobal("@.div_zero_str = constant [23 x i8] c\"Error division by zero\\00\"");
    
    symbol_table.addFunc("printi","VOID");
    symbol_table.updateFuncParams("printi",{"INT"});
    code_bp.emit("define void @printi(i32) {");
    code_bp.emit("  %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
    code_bp.emit("  call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
    code_bp.emit("  ret void");
    code_bp.emit("}");
    code_bp.emit("");

    symbol_table.addFunc("print","VOID");
    symbol_table.updateFuncParams("print",{"STRING"});
    code_bp.emit("define void @print(i8*) {");
    code_bp.emit("  %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
    code_bp.emit("  call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
    code_bp.emit("  ret void");
    code_bp.emit("}");
    code_bp.emit("");
    
}

void Llvm_compiler::printBuffers(){
    code_bp.printGlobalBuffer();
    code_bp.printCodeBuffer();
}

bool Llvm_compiler::is_main_exist(){
    return symbol_table.is_main_exist;
}

void Llvm_compiler::openScope(){
    symbol_table.openScope();
}
void Llvm_compiler::closeScope(){
    symbol_table.closeScope();
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

void Llvm_compiler::handle_return(){
    symbol_table.checkRetType("VOID");
    code_bp.emit("ret void");
}

void Llvm_compiler::handle_return(union_class& exp){
    symbol_table.checkRetType(exp.type);
    std::string ret_data = exp.data;
    if(!exp.is_literal && exp.type == "BOOL"){
        ret_data = assign_bool(exp);
    }
    std::string code = "ret " + typeSize(exp.type) + " " + ret_data;
    code_bp.emit(code);
}

union_class Llvm_compiler::begin_else(){
    std::string code = "br label @";
    int loc = code_bp.emit(code);
    symbol_table.closeScope();
    symbol_table.openScope();
    union_class else_exp;
    else_exp.label = code_bp.genLabel();
    else_exp.nextlist = code_bp.makelist({loc,FIRST});
    return else_exp;
}

union_class Llvm_compiler::end_if(union_class& exp, union_class& states){
    int loc = code_bp.emit("br label @");
    std::string next_label = code_bp.genLabel();
    code_bp.bpatch(code_bp.merge(exp.falselist,code_bp.makelist({loc,FIRST})),next_label);
    union_class res_exp;
    res_exp.nextlist = states.nextlist;
    symbol_table.closeScope();
    return res_exp;
}

union_class Llvm_compiler::end_else(union_class& exp, union_class& if_states, union_class& else_exp,union_class& else_states){
    int loc = code_bp.emit("br label @");
    std::string next_label = code_bp.genLabel();
    code_bp.bpatch(code_bp.merge(else_exp.nextlist,code_bp.makelist({loc,FIRST})),next_label);
    code_bp.bpatch(exp.falselist,else_exp.label);
    union_class res_exp;
    res_exp.nextlist = code_bp.merge(if_states.nextlist,else_states.nextlist);
    symbol_table.closeScope();
    return res_exp;
}

void Llvm_compiler::begin_while(){
    int loc = code_bp.emit("br label @");
    std::string w_label = code_bp.genLabel();
    nested_while_labels.push_back(w_label);
    code_bp.bpatch(code_bp.makelist({loc,FIRST}),w_label);
}

void Llvm_compiler::end_while(union_class& exp, union_class& states){
    std::string code = "br label %" + nested_while_labels.back();
    code_bp.emit(code);
    vector<pair<int,BranchLabelIndex>> next_list = code_bp.merge(exp.falselist,states.nextlist);
    std::string next_label = code_bp.genLabel();
    code_bp.bpatch(next_list,next_label);
    nested_while_labels.pop_back();
    symbol_table.closeScope();
}

union_class Llvm_compiler::handle_cond(union_class& exp){
    union_class res_exp;
    std::string true_label = add_br_and_label(exp);
    code_bp.bpatch(exp.truelist,true_label);
    res_exp.falselist = exp.falselist;
    symbol_table.openScope();
    return res_exp;
}

union_class Llvm_compiler::handle_break(){
    if (nested_while_labels.empty())
    {
        output::errorUnexpectedBreak(yylineno);
        symbol_table.abortParser(1);
    }
    std::string code = "br label @";
    int loc = code_bp.emit(code);
    union_class res_state;
    res_state.nextlist = code_bp.makelist({loc,FIRST});
    return res_state; 
}

void Llvm_compiler::handle_continue(){
    if (nested_while_labels.empty())
    {
        output::errorUnexpectedContinue(yylineno);
        symbol_table.abortParser(1);
    }
    std::string code = "br label %" + nested_while_labels.back();
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
        int offset = symbol_table.getOffset(var_id);
        if(offset <0){ 
            new_exp.data = "%" + to_string(-(1+offset));
        }
        else{
            new_exp.data = generate_reg();
            std::string code = new_exp.data + " = load i32 , i32* " + var_ptr; 
            code_bp.emit(code);
            if(typeSize(new_exp.type) != "i32"){
                new_exp.data = trunc(new_exp.data,"i32",typeSize(new_exp.type));
            }
        }
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
    
    /*------ check div by zero ------*/
    std::string div_reg = generate_reg();
    code_bp.emit(div_reg + " = icmp eq i32 0, " + data2);
    int loc = code_bp.emit("br i1 " + div_reg + ", label @, label @");

    std::string zero_label = code_bp.genLabel();
    std::string div_str_ptr = generate_reg();
    code_bp.emit(div_str_ptr + " = " + "getelementptr inbounds [23 x i8], [23 x i8]* @.div_zero_str, i32 0, i32 0");
    code_bp.emit("call void @print(i8* " + div_str_ptr + ")");
    code_bp.emit("call void @exit(i32 0)");

    std::string not_zero_label = code_bp.genLabel();

    code_bp.bpatch(code_bp.makelist({loc, FIRST}), zero_label);
    code_bp.bpatch(code_bp.makelist({loc, SECOND}), not_zero_label);
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

union_class Llvm_compiler::handle_literal(std::string data, std::string type){
    int value = atoi(data.c_str());
    if(type == "BYTE"){
        symbol_table.checkOverFlowByte(value);    
    }
    union_class res_exp;
    res_exp.data = data;
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

void Llvm_compiler::add_call_arg(union_class& exp){
    if(exp.type == "BOOL" && (!exp.is_literal)){
        exp.data = assign_bool(exp);
    }
    args_list.push_back(exp);
}

void Llvm_compiler::add_func_arg(union_class& exp){
    args_list.push_back(exp);
}

void Llvm_compiler::handle_func_decl(std::string ret_type,std::string func_id){
    symbol_table.addFunc(func_id,ret_type);
    symbol_table.openScope();
    std::string args_code; 
    vector<std::string> params_types;
    while(!args_list.empty()){
        union_class arg_exp = args_list.back();
        symbol_table.addArg(arg_exp.data,arg_exp.type,arg_exp.is_const);
        args_code += typeSize(arg_exp.type) + ", ";
        params_types.push_back(arg_exp.type);
        args_list.pop_back();
    }
     if(!args_code.empty()){// delete last ", "
        args_code.erase(args_code.size() - 2);
    }
    symbol_table.updateFuncParams(func_id,params_types);
    std::string code = "define " + typeSize(ret_type) + " @" + func_id + "(" + args_code + ") {";
    code_bp.emit(code);
    stack_ptr_reg = generate_reg() + "_stack_ptr_" + func_id;
    code = stack_ptr_reg + " = alloca i32, i32 50";
    code_bp.emit(code);
}

void Llvm_compiler::handle_func_end(std::string ret_type){
    std::string ret_code = "ret " + typeSize(ret_type);
    if(ret_type != "VOID"){
        ret_code += " 0";
    }
    code_bp.emit(ret_code);
    code_bp.emit("}");
    code_bp.emit("");
    symbol_table.closeScope();
}

union_class Llvm_compiler::handle_call(std::string func_id){
    std::string call_args;
    vector<std::string> params_types;
    while(!args_list.empty()){
        union_class arg_exp = args_list.back();
        call_args += typeSize(arg_exp.type) + " " + arg_exp.data + ", ";
        params_types.push_back(arg_exp.type);
        args_list.pop_back();
    }
    if(!call_args.empty()){// delete last ", "
        call_args.erase(call_args.size() - 2);
    }
    union_class call_res;
    call_res.type = symbol_table.checkFuncDecl(func_id,params_types);
    std::string code;
    if(call_res.type != "VOID"){
        call_res.data = generate_reg();
        code += call_res.data + " = ";
    }
    code += "call " + typeSize(call_res.type) + " @" + func_id + "(" + call_args + ")";
    code_bp.emit(code);
    return call_res;
}


union_class Llvm_compiler::merge_lists(union_class& uni_1, union_class& uni_2){
    union_class uni_res;
    uni_res.truelist = code_bp.merge(uni_1.truelist,uni_2.truelist);
    uni_res.falselist = code_bp.merge(uni_1.falselist,uni_2.falselist);
    uni_res.nextlist = code_bp.merge(uni_1.nextlist,uni_2.nextlist);
    return uni_res;
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
    if(type == "STRING"){
        return "i8*";
    }
    if(type == "VOID"){
        return "void";
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
    int backpatch_check =  code_bp.emit("br i1" + bool_exp.data +" label @, label @");
    std::string bool_reg = generate_reg();
    std::string true_label = code_bp.genLabel();
    int backpatch_for_true =  code_bp.emit("br label @");
    std::string false_label = code_bp.genLabel();
    int backpatch_for_false =  code_bp.emit("br label @");

    string phi_label = code_bp.genLabel();
    code_bp.emit(bool_reg + " = phi i32 [  1, %" + true_label + "], [  0, %" + false_label + "]");

    bool_exp.truelist = code_bp.merge(bool_exp.truelist,code_bp.makelist({backpatch_check,FIRST}));
    bool_exp.falselist = code_bp.merge(bool_exp.falselist,code_bp.makelist({backpatch_check,SECOND}));
    code_bp.bpatch(bool_exp.truelist, true_label);
    code_bp.bpatch(bool_exp.falselist, false_label);

    code_bp.bpatch(code_bp.makelist({backpatch_for_true, FIRST}), phi_label);
    code_bp.bpatch(code_bp.makelist({backpatch_for_false, FIRST}), phi_label);

    return bool_reg;
}

