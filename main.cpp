#include <iostream>
#include <string>
#include "antlr4-runtime.h"
#include "parser/SQLVisitor.h"
#include "parser/SQLLexer.h"
#include "parser/MyVisitor.h"
using namespace std;
using namespace antlr4;

auto parse(std::string sSQL, MyVisitor *visitor){
    ANTLRInputStream sInputStream(sSQL);
    SQLLexer iLexer(&sInputStream);
    CommonTokenStream sTokenStream(&iLexer);
    SQLParser iParser(&sTokenStream);
    auto iTree = iParser.program();
    auto iRes = visitor->visit(iTree);
    return iRes;
}


int main(){
    MyVisitor *iVisitor = new MyVisitor();
    cout << "Input 'quit;' or 'QUIT;' to exit program!" << endl;
    string sql;
    std::getline(std::cin, sql);
    while(1){
        if(sql == "QUIT;" || sql == "quit;"){
            iVisitor->writeBack();
            break;
        }
        parse(sql, iVisitor);
        std::getline(std::cin, sql);
    }
    return 0;
}