#include "hw5_output.hpp"
#include "sym_table.hpp"

using namespace std;

#define BASE__GLOBAL_OFFSET 0
#define MAX_BYTE_NUM 255



extern int yylineno;


/*------------------------------------- Private Classes ------------------------------------------*/
class Sym_table::sType    //class symbols in symbol table
{
	public:
	std::string name;    /* name of symbol          */
	std::string type;    /* type of symbol          */
	int offset;	 /* offset of symbol          */

	sType(){}
	sType( const std::string& n, const std::string& t,  int of): name(n), type(t),  offset(of) {}
	virtual void printSymbol() =0;

};

class Sym_table::funcType : public Sym_table::sType //class for function symbols
{

public:
	
	std::string ret_type;            	/* type of return value                     */
	std::vector<std::string> params;    /* types of the function parameters     	*/

	funcType(){}
	funcType(const std::string& n, const std::string& t,  int of, const std::string& rt, const std::vector<std::string>& p): sType(n,t,of) , ret_type(rt), params(p){}
	void printSymbol(){output::printID(name, offset, output::makeFunctionType(ret_type, params));}
};

class Sym_table::varType: public Sym_table::sType // class for variable symbols
{

public:
	bool is_const;
	bool is_literal;

	std::string ptr_reg; /* holds register name of var pointer on stack, in case of literal will hold the value itself*/

	varType(){}
	varType(const std::string& n, const std::string& t,  int of, bool is_const_t, bool is_literal_t): sType(n,t,of), is_const(is_const_t), is_literal(is_literal_t)
	{ptr_reg = nullptr;}

	void printSymbol(){output::printID(name, offset, type);}
};

class Sym_table::scope // class for scope properties
{
	public:
	int initial_offset;
	std::vector<std::string> symbols_list;;

	scope(int of):
	initial_offset(of){}
	

};
/*------------------------------------------------------------------------------------------------*/


/*----------------------------------- Sym_table Functions ----------------------------------------*/



// Initialize the parser global data and add print and printi functions
Sym_table::Sym_table(bool print_copes_t = false) { 
	stack_scope = vector<scope>();
	symbol_table = unordered_map<string, sType*>();
    global_offset = BASE__GLOBAL_OFFSET;
    is_main_exist = false;

	print_scopes = print_scopes;

    openScope(); //open global scope
	
    addFunc("print", "VOID", {"STRING"});
	addFunc("printi", "VOID", {"INT"});
	
}

//Check if valid main function was declared and close the global scope
Sym_table::~Sym_table(){
    checkMainExist();
    while(!stack_scope.empty()){
		closeScope();
	}
}

//Delete Parser data and exit with value = exit_val
void Sym_table::abortParser(int exit_val){
	print_scopes = false;
    exit(exit_val);
}

//check if Main was valid decleared
void Sym_table::checkMainExist() { 
	if (!is_main_exist) {
		output::errorMainMissing();
		abortParser(1);
	}
}

// Check if a symbol is already exists or not
void Sym_table::checkNameAvailable(const string& name){

	if (symbol_table.find(name) != symbol_table.end()) 
    {
        output::errorDef(yylineno, name);
        exit(1);
	}
}


//Open new scope
void Sym_table::openScope(){ 
    scope sc(global_offset);
    stack_scope.push_back(sc);

	
}

//Close scope and print his symbols
void Sym_table::closeScope() {
	scope sc = stack_scope.back();
	if(print_scopes){
		output::endScope();
	}
	for (auto& symbol_name : sc.symbols_list) {
		sType* symbol_ptr = symbol_table[symbol_name];
		if(print_scopes){
			symbol_ptr->printSymbol();
		}
        delete symbol_ptr;
		symbol_table.erase(symbol_name);
	}
	global_offset = sc.initial_offset;

	stack_scope.pop_back();
}



//Check if given Func symbol is valid main
void Sym_table::checkFuncIsMain(const funcType& func) { 
	if (func.name == "main" && func.ret_type == "VOID" && func.params.empty()) 
    {
        is_main_exist = true;
    }
}

//Add function to symbol table and global scope
void Sym_table::addFunc(const string& name, const string& ret_type, const vector<string>& params) {
	checkNameAvailable(name);
    funcType* sym_ptr = new funcType(name, "FUNC", 0, ret_type, params);
	symbol_table[name] = sym_ptr;
    stack_scope.front().symbols_list.push_back(name);
	checkFuncIsMain(*sym_ptr);		
}

// Insert variable to symbol table and current scope
void Sym_table::addVar(const string& name, const string& type, bool is_const,bool is_literal = false) {
	checkNameAvailable(name);
    varType* sym_ptr = new varType(name, type, is_literal ? global_offset : global_offset++, is_const , is_literal);
    symbol_table[name] = sym_ptr;
    stack_scope.back().symbols_list.push_back(name);
}

//Insert argoment of function to symbol table and current scope
void Sym_table::addArg(const string& name, const string& type, bool is_const){
    checkNameAvailable(name);
    varType* sym_ptr = new varType(name, type, (-(stack_scope.back().symbols_list.size()+1)), is_const, false);
    symbol_table[name] = sym_ptr;
    stack_scope.back().symbols_list.push_back(name);
}

//Update the register containing the variable, 
//note: function assuming id is exist and variable
//		use checkValidVar before before using!
void Sym_table::updateReg(std::string& id, std::string& reg){
	varType* var_ptr = (varType*)symbol_table[id];
	var_ptr->ptr_reg = reg;
}

//Get the register containing the variable
//note: function assuming id is exist and variable
//		use checkValidVar before using!
std::string& Sym_table::getReg(std::string& id){
	varType* var_ptr = (varType*)symbol_table[id];
	return var_ptr->ptr_reg;
}

//Check if variable is literal
bool Sym_table::checkLiteral(std::string& id){
	varType* var_ptr = (varType*)symbol_table[id];
	return var_ptr->is_literal;
}

//get Offset of symbol, assume symbol exist
int Sym_table::getOffset(std::string& name){
	return symbol_table[name]->offset;
}


//Merge 2 vectors to the first one
void mergeVectors(vector<string>& v1, vector<string>& v2){
    v1.insert(v1.end(),v2.begin(),v2.end());
}

//Check if types are valid in the action
void Sym_table::checkValidAssign(const string& type_1, const string& type_2) { 
	if (type_1 != type_2 && !(type_1 == "INT" && type_2 == "BYTE")) {
		output::errorMismatch(yylineno);
		abortParser(1);
	}
}

//Check if a Variable is define and return if it constant
bool Sym_table::checkValidVar(const string& name) {
	if (symbol_table.find(name) == symbol_table.end() || symbol_table[name]->type == "FUNC"){
		output::errorUndef(yylineno, name);
        abortParser(1);
	}
    varType* var_ptr = (varType*)symbol_table[name];
    return var_ptr->is_const;
}

//Return the type of the decleared function
string Sym_table::typeOfSymbol(const string& name){ 
	return symbol_table[name]->type;
}

//check if the return type is identical to what was decleared in the function.
void Sym_table::checkRetType(const string& type) {
	funcType* func_ptr = (funcType*) symbol_table[stack_scope.front().symbols_list.back()];
	checkValidAssign(func_ptr->ret_type, type);
}

//Check if t is bool type
void Sym_table::checkBoolType(const string& t) { 
	if (t != "BOOL") {
		output::errorMismatch(yylineno);
		abortParser(1);
	}
}

//Check if t is a numeric type
void Sym_table::checkNumberType(const string& t) { 
	if (t != "INT" && t != "BYTE"  ) {
		output::errorMismatch(yylineno);
		abortParser(1);
	}
}

// Check if byte is overflow
void Sym_table::checkOverFlowByte(int num) { 
	if (num > MAX_BYTE_NUM) {
		output::errorByteTooLarge(yylineno, to_string(num));
		abortParser(1);
	}
}

// Check if the function was decleared and its call is valid,will return the function ret_type
string Sym_table::checkFuncDecl(const string& name, const vector<string>& params_types) { 
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
        if (func_param_type != call_param_type && !(func_param_type == "INT" && call_param_type == "BYTE")){
			output::errorPrototypeMismatch(yylineno, name, sym_ptr->params);
			abortParser(1);
		}
	}
    return sym_ptr->ret_type;

}

//Check if the casting is valid
void Sym_table::checkValidCast(const string& t1, const string& t2) {
	if ((t1 == "INT" || t1 == "BYTE") &&
		(t2 == "INT" || t2 == "BYTE")) 
		return;
    
	output::errorMismatch(yylineno);
	abortParser(1);
}


//Find the currect type of the Binop operation
string Sym_table::findBinopType(const string& t1, const string& t2) { 
    checkValidCast(t1,t2);
    if (t1 == "INT" || t2 == "INT") {
		return "INT";
	} else {
		return "BYTE";
	}
}