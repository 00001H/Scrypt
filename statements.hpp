#ifndef SCP_STATEMENTS
#define SCP_STATEMENTS
#include"defs.hpp"
#include"errors.hpp"
#include"objects.hpp"
#include"expression.hpp"
namespace scrypt{
    class ExprStmt : public Statement{
        refExpression expr;
        public:
            ExprStmt(refExpression expr) : expr(expr){}
            virtual void execute(Scope s) override{
                (void)expr->evaluate(s);
            }
            refExpression getExpr(){
                return expr;
            }
    };
    template<typename T,typename... Args>
    refStatement sNew(Args... args){
        return refStatement(new T(args...));
    }
}
#endif
