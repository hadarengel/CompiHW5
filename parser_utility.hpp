#ifndef PARSER_UTILITY
#define PARSER_UTILITY

#include <vector>
#include <string>
#include <unordered_map>
#include "hw3_output.hpp"




class union_class{
	public:
        int val;
        bool is_const;
        std::string type;
    	std::string name;
        std::vector<std::string> params_types;
};

#define YYSTYPE union_class


class sType    //class symbols in symbol table
{
    public:
    std::string name;    /* name of symbol          */
    std::string type;    /* type of symbol          */
    int offset;	 /* offset of symbol          */

    sType(){}
    sType( const std::string& n, const std::string& t,  int of): name(n), type(t),  offset(of) {}
    virtual void printSymbol() =0;

};

class funcType : public sType //class for function symbols
{

public:
    
    std::string ret_type;            /* type of return value                      */
    std::vector<std::string> params;      /* types of the function parameters          */
    

    funcType(){}
    funcType(const std::string& n, const std::string& t,  int of, const std::string& rt, const std::vector<std::string>& p): sType(n,t,of) , ret_type(rt), params(p){}
    void printSymbol(){output::printID(name, offset, output::makeFunctionType(ret_type, params));}
};

class varType: public sType // class for variable symbols
{

public:
    bool is_const;          

    varType(){}
    varType(const std::string& n, const std::string& t,  int of, bool is_const_t): sType(n,t,of), is_const(is_const_t){}

    void printSymbol(){output::printID(name, offset, type);}
};






class scope // class for scope properties
{
	public:
    int initial_offset;
	std::vector<std::string> symbols_list;
    bool is_while_scope;

    scope(int of, bool isWs = false):
    initial_offset(of), is_while_scope(isWs){}
    
  
};


//Initialize the parser global data and add print and printi functions
void InitParser();

//Check if valid main function was declared and close the global scope
void FinishParser();

//Open new scope
void openScope(bool isWhile);

//Close scope and print his symbols
void closeScope();

//Add function to symbol table and global scope
void addFunc(const std::string& name, const std::string& ret_type, const std::vector<std::string>& params);

//Insert variable to symbol table and current scope
void addVar(const std::string& name, const std::string& type, bool is_const);

//Insert argoment of function to symbol table and current scope
void addArg(const std::string& name, const std::string& type, bool is_const);

//Merge 2 vectors to the first one
void mergeVectors(std::vector<std::string>& v1, std::vector<std::string>& v2);

//Check if types are valid in the action
void checkValidAssign(const std::string& type_1, const std::string& type_2);

//Check if a Variable is define and return if it constant
bool checkValidVar(const std::string& name);

//Return the type of the decleared function
std::string typeOfSymbol(const std::string& name);

//check if the return type is identical to what was decleared in the function
void checkRetType(const std::string& type);

// Check for unexpected break or continue
void checkUnexpected(const std::string& unexp);

//Check if t is bool type
void checkBoolType(const std::string& t);

//Check if t is a numeric type
void checkNumberType(const std::string& t);

//Check if byte is overflow
void checkOverFlowByte(int num);

//Check if the function was decleared and its call is valid,will return the function ret_type
std::string checkFuncDecl(const std::string& name, const std::vector<std::string>& params_types);

//Check if the casting is valid
void checkValidCast(const std::string& t1, const std::string& t2);

//Find the currect type of the Binop operation
std::string findBinopType(const std::string& t1, const std::string& t2);


#endif