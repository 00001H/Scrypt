#include<iostream>
#include"scrypt.hpp"
using namespace scrypt;
using namespace std;
int main(){
    /*
    Procedure main(what, funct, what2){
        print(what);
        funct(what2);
    }
    */
    refProcedure main(new Procedure(builtinScope(),{
        sNew<ExprStmt>(eNew<FuncCall>(
            eNew<Access>(L"print")
            ,List<refExpression>{
            eNew<Access>(L"what")
        })),
        sNew<ExprStmt>(eNew<FuncCall>(
            eNew<Access>(L"funct")
            ,List<refExpression>{
            eNew<Access>(L"what2")
        }))
    },{L"what",L"funct",L"what2"}));
    /*
    main("Hello, World!\n", print, "Print 2");
    */
    runProgram(main,{
        eNew<Constant>(String->construct(Scope::null(),str(L"Hello, World!\n"))),
        eNew<Constant>(Object::FuncObj(print)),
        eNew<Constant>(String->construct(Scope::null(),str(L"Print 2\n")))
    });
    Opmap x = gen_opmap({L"pr",L"pra",L"prb"});
    Scanner scn(L"prpraprb");
    str mtchd;
    while(!scn.done()){
        mtchd = scn.next(x);
        if(mtchd.empty()){
            wcout << L"Error: unmatched data" << endl;
            return -1;
        }
        wcout << L"Matched: " << mtchd << endl;
    }
    return 0;
}
