#ifndef SCP_BUILTIN_OPS
#define SCP_BUILTIN_OPS
#include"defs.hpp"
#include"objects.hpp"
#include"expression.hpp"
namespace scrypt{
    template<op_specs::op_spec*>
    refObject basic_op(Scope,refExpression,refExpression);
    namespace{
        template<op_specs::op_spec* funcname>
        basic_op_t op_for(){
            return static_cast<basic_op_t>(basic_op<funcname>);
        }
    }
    basic_op_t addition_op = op_for<&op_specs::add>();
    template<op_specs::op_spec* funcname>
    refObject basic_op(Scope s,refExpression lhs,refExpression rhs){
        refObject a0 = lhs->evaluate(s);
        refObject a1 = rhs->evaluate(s);
        str dunder_name = funcname->dunder();
        str rdunder_name = funcname->rdunder();
        if(a0->hasattr(dunder_name)){
            refObject addf = a0->getattr(dunder_name);
            if(!addf->callable()){
                throw OperatorError(L"operator "+funcname->sym+L": "+dunder_name+L" present but not callable for object "+a0->stringify(s));
            }
            try{
                return addf->invoke(s,{eNew<Constant>(a1)});
            }catch(OperatorError& e){
                //falls back to __rxxx__
            }
        }
        if(a1->hasattr(rdunder_name)){
            refObject addf = a1->getattr(rdunder_name);
            if(!addf->callable()){
                throw OperatorError(L"operator "+funcname->sym+L": "+rdunder_name+L" present but not callable for object "+a1->stringify(s));
            }
            try{
                return addf->invoke(s,{eNew<Constant>(a0)});
            }catch(OperatorError&){
                //cannot add
            }
        }
        throw OperatorError(L"operator "+funcname->sym+L": Cannot "+funcname->st+L" a "+a0->getClass().getName()+L" and a "+a1->getClass().getName());
    }
    Dict<str,BinOpData> builtin_operators(
        Dict<str,BinOpData>::of(
            L"+",{priorities::ADDSUB,addition_op}
        )
    );
    const Opmap builtin_omp = gen_opmap(builtin_operators.keys());
    BinOpData* oplookup(const str& x){
        return &builtin_operators.get(x);
    }
}
#endif
