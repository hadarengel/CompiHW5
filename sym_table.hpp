#ifndef PARSER_UTILITY
#define PARSER_UTILITY

#include <vector>
#include <string>
#include <unordered_map>
#include "hw5_output.hpp"



class Sym_table
{
private:

    /*   Private Classes*/
    class sType;
    class funcType;
    class varType;
    class scope;



    vector<scope> stack_scope;
    unordered_map<string, sType*> symbol_table;
    int global_offset;
    int num_of_while_scopes;
    bool is_main_exist;
    bool print_scopes;

    /*   Private Functions   */
    void checkMainExist();
    void checkFuncIsMain(const funcType& func);
    void checkNameAvailable(const string& name);

    

public:
    Sym_table(bool print_scopes = false);
    ~Sym_table();

    //Open new scope
    void openScope(bool isWhile);

    //Close scope and print his symbols
    void closeScope();

    //Add function to symbol table and global scope
    void addFunc(const std::string& name, const std::string& ret_type,  const std::vector<std::string>& params);

    //Insert variable to symbol table and current scope
    void addVar(const std::string& name, const std::string& type, bool is_const);

    //Insert argoment of function to symbol table and current scope
    void addArg(const std::string& name, const std::string& type, bool is_const);

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
    
    //destroy parser data and call exit(exit_val)
    void abortParser(int exit_val);

    
};




//Merge 2 vectors to the first one
void mergeVectors(std::vector<std::string>& v1, std::vector<std::string>& v2);









#endif