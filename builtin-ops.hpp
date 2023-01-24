#ifndef SCP_BUILTIN_OPS
#define SCP_BUILTIN_OPS
#include"defs.hpp"
#include"objects.hpp"
#include"expression.hpp"
namespace scrypt{
    struct BinOpData{
        using invoke_t = std::function<refObject(Scope,refExpression,refExpression)>;
        invoke_t func;
        size_t pty;
        BinOpData(size_t pty,invoke_t func) : func(func), pty(pty){}
    };
    namespace priorities{
        using pty_t = const size_t;
        pty_t PTY_MAX = 50000;
        pty_t EXPNT = 8200;
        pty_t DIVMUL = 8000;
        pty_t ADDSUB = 7800;
    }
    refObject plus(Scope s,refExpression lhs,refExpression rhs){
        refObject a0 = lhs->evaluate(s);
        refObject a1 = rhs->evaluate(s);
        if(a0->hasattr(L"__add__")){
            refObject addf = a0->getattr(L"__add__");
            if(!addf->callable()){
                throw OperatorError(L"operator +: __add__ present but not callable for object "+a0->stringify(s));
            }
            try{
                return addf->invoke(s,{eNew<Constant>(a1)});
            }catch(OperatorError&){
                //falls back to __radd__
            }
        }
        if(a1->hasattr(L"__radd__")){
            refObject addf = a1->getattr(L"__add__");
            if(!addf->callable()){
                throw OperatorError(L"operator +: __radd__ present but not callable for object "+a1->stringify(s));
            }
            try{
                return addf->invoke(s,{eNew<Constant>(a0)});
            }catch(OperatorError&){
                //cannot add
            }
        }
        throw OperatorError(L"operator +: Cannot add together a "+a0->getClass().getName()+L" and a "+a1->getClass().getName());
    }
    Dict<str,BinOpData> builtin_operators(
        Dict<str,BinOpData>::of(
            L"+",{priorities::ADDSUB,plus}
        )
    );
    BinOpData* oplookup(const str& x){
        return &builtin_operators.get(x);
    }
}
#endif
