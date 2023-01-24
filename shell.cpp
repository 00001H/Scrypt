#include<iostream>
#include"scrypt.hpp"
using namespace scrypt;
using std::wcout;
using std::wcin;
using std::endl;
using std::getline;
int main(){
    str exp;
    refStatement stmt;
    Scope sp = builtinScope();
    Scanner scnr;
    while(true){
        wcout << L">";
        getline(wcin,exp);
        scnr.reset(exp);
        while(!scnr.done()){
            stmt = nullptr;
            try{
                stmt = expect_statement(scnr);
            }catch(ParseError& e){
                wcout << e.what() << endl;
                break;
            }
            if(stmt!=nullptr){
                try{
                    stmt->invoke(sp);
                }catch(ProgramError& e){
                    e.addFrame({L"<shell>",L"<shell-main>",scnr.pos()});
                    wcout << e.dump() << endl;
                    break;
                }
            }
        }
        if(!scnr.done()){
            wcout << L"[WARN] input not fully consumed." << endl;
        }
        wcout << endl;
    }
    return 0;
}
