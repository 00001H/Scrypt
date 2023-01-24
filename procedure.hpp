#ifndef SCP_PROCEDURE
#define SCP_PROCEDURE
#include"defs.hpp"
#include"errors.hpp"
#include"statements.hpp"
#include"utils.hpp"
#include"objects.hpp"
namespace scrypt{
    #define stri std::to_wstring
    refObject Procedure::invoke(Scope outer,List<refExpression> args){
        if(args.size()!=argnames.size()){
            throw FuncCallError(subst(L"Calling function "+name+L" which accepts $ arguments with $ arguments",{
                stri(argnames.size()),
                stri(args.size())
                }));
        }
        Scope inner;
        for(size_t i=0;i<argnames.size();++i){
            inner.assign(argnames[i],args[i]->evaluate(outer));
        }
        Scope execscope = Scope::compoundScope(inner,closure);
        for(refStatement& stmt : stmts){
            try{
                stmt->invoke(execscope);
            }catch(ProgramError& pe){
                pe.addFrame({file,name,stmt->pos});
                throw;
            }
        }
        return None;
    }
    refProcedure Procedure::partial(refProcedure x,List<refExpression> params){
        return refProcedure(new NativeFunc(x->nArgs(),x->nDArgs(),
            [&x,&params](Scope s,List<refExpression> args){
                return x->invoke(s,params+args);
            }
        ,x->getName()));
    }
    namespace builtins{
        refObject printfunc(Scope s,List<refExpression> args){
            std::wcout << args[0]->evaluate(s)->stringify(s);
            return None;
        }
        refObject exitfimpl(Scope s,List<refExpression> args){
            std::exit(0);
        }
        const refProcedure print(new NativeFunc(1,0,printfunc,L"print"));
        const refProcedure exit(new NativeFunc(0,0,exitfimpl,L"exit"));
        Scope builtinScope(){
            Scope s;
            s.assign(print->getName(),Object::FuncObj(print));
            s.assign(exit->getName(),Object::FuncObj(exit));
            return s;
        }
    }
    void runProgram(refProcedure main,List<refExpression> args=noArgs){
        try{
            main->invoke(builtinScope(),args);
        }catch(ProgramError& e){
            std::wcerr << e.dump() << std::endl;
        }
    }
    template<typename T,typename... Args>
    refProcedure pNew(Args... args){
        return refProcedure(new T(args...));
    }
    #undef stri
}
#endif
