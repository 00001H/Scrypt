#ifndef SCP_EXPRESSION
#define SCP_EXPRESSION
#include"defs.hpp"
#include"objects.hpp"
namespace scrypt{
    class Assign : public Expression{
        refExpression lhs;
        refExpression rhs;
        public:
            Assign(refExpression lhs,refExpression rhs) : lhs(lhs), rhs(rhs){}
            virtual refObject evaluate(Scope s) override{
                refObject nw = rhs->evaluate(s);
                if(!lhs->is_lval()){
                    throw AssignmentError(L"Cannot assign to a rvalue");
                }
                lhs->assignTo(s,nw);
                return nw;
            }
    };
    class Access : public Expression{
        str name;
        public:
            Access(str name) : name(name){}
            virtual refObject evaluate(Scope s) override{
                try{
                    return s.get(name);
                }catch(std::out_of_range& e){
                    throw VariableNotFoundError(stow(e.what()));
                }
            }
            virtual bool is_lval() const override{return true;}
            virtual void assignTo(Scope s,refObject obj) override{
                s.assign(name,obj);
            }
    };
}
#include"builtin-ops.hpp"
/*
SCRYPT_POTENTIAL_ISSUE 0001 Operator Priority

Example 1.
> x*y+z

The current parsing system will fail to produce the correct
order of evaluation.

(Abbreviations:
Acss("x") or var"x" for Access("x")

parse()/?? for the return value of recursive parse() call
that hasn't finished yet
)

#1 parse scan: x|*y+z curr: Access("x")
#1 parse scan: x*|y+z partial: Mul(Acss("x"),parse()/??)
  -> #2 parse scan: y|+z curr: Access("y")
     #2 parse scan: y+|z curr: Add(Acss("y"),parse()/??)
       -> parse scan: z| curr: Access("z")
     #2 parse result: Add(var"y",var"z")
#1 parse result: Mul(var"x",Add(var"y",var"z"))

Should be:
Add(Mul(var"x",var"y"),var"z")

therefore, in the Mul() constructor, we check if the
RHS has a lower priority, and if it does, we replace
our RHS with the original RHS's LHS, and
the original RHS's LHS with this operator,
and change this operator to the original RHS.
*/
namespace scrypt{
    class BiOp : public Expression{
        refExpression lhs;
        refExpression rhs;
        BinOpData::invoke_t oncall;
        size_t pty;
        public:
            BiOp& operator=(const BiOp& other){
                lhs = other.lhs;
                rhs = other.rhs;
                oncall = other.oncall;
                pty = other.pty;
                return *this;
            }
            BiOp(BinOpData dt,refExpression lhs,refExpression rhs) : rhs(rhs), oncall(dt.func), pty(dt.pty){
                if(isinstanceof<BiOp>(rhs)){
                    BiOp& crhs = *dynamic_cast<BiOp*>(rhs.get());
                    size_t rpty = crhs.pty;
                    if(pty>rpty){
                        rhs = crhs.lhs;
                        crhs.lhs = eNew<BiOp>(*this);
                        (*this) = crhs;
                    }
                }
            }
            virtual refObject evaluate(Scope s) override{
                return oncall(s,lhs,rhs);
            }
    };
    class FuncCall : public Expression{
        refExpression func;
        List<refExpression> args;
        public:
            FuncCall(refExpression func,List<refExpression> args) : func(func), args(args){}
            virtual refObject evaluate(Scope s) override{
                return func->evaluate(s)->invoke(s,args);
            }
    };
    template<typename T,typename... Args>
    refExpression eNew(Args... args){
        return refExpression(new T(args...));
    }
}
#endif
