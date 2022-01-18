#include "hw5_output.hpp"
#include "parser_utility.hpp"

using namespace std;

#define BASE__GLOBAL_OFFSET 0
#define MAX_BYTE_NUM 255


extern int yylineno;
vector<scope> stack_scope;
unordered_map<string, sType*> symbol_table;
int global_offset;
int num_of_while_scopes;
bool is_main_exist;

void abortParser(int exit_val);
void checkNameAvailable(const string& name);
void checkFuncIsMain(const funcType& func);
void checkMainExist();


// Initialize the parser global data and add print and printi functions
void InitParser() { 
    global_offset = BASE__GLOBAL_OFFSET;
    num_of_while_scopes = 0;
    is_main_exist = false;
    openScope(false); //open global scope
	
    addFunc("print", "VOID", {"STRING"});
	addFunc("printi", "VOID", {"INT"});
	
}

//Check if valid main function was declared and close the global scope
void FinishParser(){
    checkMainExist();
    closeScope();
}

//Delete Parser data and exit with value = exit_val
void abortParser(int exit_val){
    for (auto symbol : symbol_table){
        delete symbol.second;
    }
    exit(exit_val);
}

//check if Main was valid decleared
void checkMainExist() { 
	if (!is_main_exist) {
		output::errorMainMissing();
		abortParser(1);
	}
}

// Check if a symbol is already exists or not
void checkNameAvailable(const string& name){

	if (symbol_table.find(name) != symbol_table.end()) 
    {
        output::errorDef(yylineno, name);
        abortParser(1);
	}
}


//Open new scope
void openScope(bool isWhile){ 
    scope sc(global_offset, isWhile);
    stack_scope.push_back(sc);
	if (isWhile)
	{
		num_of_while_scopes++;
	}
	
}

//Close scope and print his symbols
void closeScope() {
	scope sc = stack_scope.back();
	output::endScope();
	for (auto& symbol_name : sc.symbols_list) {
		sType* symbol_ptr = symbol_table[symbol_name];
        symbol_ptr->printSymbol();
        delete symbol_ptr;
		symbol_table.erase(symbol_name);
	}
	global_offset = sc.initial_offset;
	if (sc.is_while_scope){
		--num_of_while_scopes;
	}
	stack_scope.pop_back();
}



//Check if given Func symbol is valid main
void checkFuncIsMain(const funcType& func) { 
	if (func.name == "main" && func.ret_type == "VOID" && func.params.empty()) 
    {
        is_main_exist = true;
    }
}

//Add function to symbol table and global scope
void addFunc(const string& name, const string& ret_type, const vector<string>& params) {
	checkNameAvailable(name);
    funcType* sym_ptr = new funcType(name, "FUNC", 0, ret_type, params);
	symbol_table[name] = sym_ptr;
    stack_scope.front().symbols_list.push_back(name);
	checkFuncIsMain(*sym_ptr);		
}

// Insert variable to symbol table and current scope
void addVar(const string& name, const string& type, bool is_const) {
	checkNameAvailable(name);
    varType* sym_ptr = new varType(name, type, global_offset++, is_const);
    symbol_table[name] = sym_ptr;
    stack_scope.back().symbols_list.push_back(name);
}

//Insert argoment of function to symbol table and current scope
void addArg(const string& name, const string& type, bool is_const){
    checkNameAvailable(name);
    varType* sym_ptr = new varType(name, type, (-(stack_scope.back().symbols_list.size()+1)) , is_const);
    symbol_table[name] = sym_ptr;
    stack_scope.back().symbols_list.push_back(name);
    
}


//Merge 2 vectors to the first one
void mergeVectors(vector<string>& v1, vector<string>& v2){
    v1.insert(v1.end(),v2.begin(),v2.end());
}

//Check if types are valid in the action
void checkValidAssign(const string& type_1, const string& type_2) { 
	if (type_1 != type_2 && !(type_1 == "INT" && type_2 == "BYTE")) {
		output::errorMismatch(yylineno);
		abortParser(1);
	}
}

//Check if a Variable is define and return if it constant
bool checkValidVar(const string& name) {
	if (symbol_table.find(name) == symbol_table.end() || symbol_table[name]->type == "FUNC"){
		output::errorUndef(yylineno, name);
        abortParser(1);
	}
    varType* var_ptr = (varType*)symbol_table[name];
    return var_ptr->is_const;
}

//Return the type of the decleared function
string typeOfSymbol(const string& name){ 
	return symbol_table[name]->type;
}

//check if the return type is identical to what was decleared in the function.
void checkRetType(const string& type) {
	funcType* func_ptr = (funcType*) symbol_table[stack_scope.front().symbols_list.back()];
	checkValidAssign(func_ptr->ret_type, type);
}


// Check for unexpected break or continue
void checkUnexpected(const string& statement) {
	if (num_of_while_scopes == 0){
		if(statement == "CONTINUE"){
            output::errorUnexpectedContinue(yylineno);
        }
	    if(statement == "BREAK"){
            output::errorUnexpectedBreak(yylineno);
		}
        abortParser(1);
	}
}

//Check if t is bool type
void checkBoolType(const string& t) { 
	if (t != "BOOL") {
		output::errorMismatch(yylineno);
		abortParser(1);
	}
}

//Check if t is a numeric type
void checkNumberType(const string& t) { 
	if (t != "INT" && t != "BYTE"  ) {
		output::errorMismatch(yylineno);
		abortParser(1);
	}
}

// Check if byte is overflow
void checkOverFlowByte(int num) { 
	if (num > MAX_BYTE_NUM) {
		output::errorByteTooLarge(yylineno, to_string(num));
		abortParser(1);
	}
}

// Check if the function was decleared and its call is valid,will return the function ret_type
string checkFuncDecl(const string& name, const vector<string>& params_types) { 
	if (symbol_table.find(name) == symbol_table.end() || symbol_table[name]->type != "FUNC") {
		output::errorUndefFunc(yylineno, name);
		abortParser(1);
	}
	funcType* sym_ptr = (funcType*)symbol_table[name];
	if (sym_ptr->params.size() != params_types.size()) {
		output::errorPrototypeMismatch(yylineno, name, sym_ptr->params);
		abortParser(1);
	}
	for (unsigned int i =0; i < params_types.size(); i++) {
		string func_param_type = sym_ptr->params[i];
        string call_param_type = params_types[i];
        if (func_param_type != call_param_type && !(func_param_type == "INT" && call_param_type == "BYTE")) {
			output::errorPrototypeMismatch(yylineno, name, sym_ptr->params);
			abortParser(1);
		}
	}
    return sym_ptr->ret_type;

}

//Check if the casting is valid
void checkValidCast(const string& t1, const string& t2) {
	if ((t1 == "INT" || t1 == "BYTE") &&
		(t2 == "INT" || t2 == "BYTE")) 
		return;
    
	output::errorMismatch(yylineno);
	abortParser(1);
}


//Find the currect type of the Binop operation
string findBinopType(const string& t1, const string& t2) { 
    checkValidCast(t1,t2);
    if (t1 == "INT" || t2 == "INT") {
		return "INT";
	} else {
		return "BYTE";
	}
}