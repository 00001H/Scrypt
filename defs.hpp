#ifndef SCP_DEFS
#define SCP_DEFS
#include"defx\typedef.hpp"
#include"defx\dstruct.hpp"
#include"utils.hpp"
namespace scrypt{
    inline const List<refExpression> noArgs{};
    namespace builtins{}
    using namespace builtins;
    class Procedure{
        protected:
            List<refStatement> stmts;
            List<str> argnames;
            str file;
            str name;
            Scope closure;
        public:
            Procedure(Scope s,List<refStatement> stmts={},List<str> params={},str name=L"<Null>",str file=L"<builtin>") : stmts(stmts), argnames(params), file(file), name(name), closure(s){}
            static refProcedure partial(refProcedure x,List<refExpression> params);
            str getName() const{
                return name;
            }
            virtual size_t nArgs() const{
                return argnames.size();
            }
            virtual size_t nDArgs() const{
                return 0;//TODO
            }
            virtual refObject invoke(Scope outer,List<refExpression> args);
            virtual ~Procedure(){}
    };
    class NativeFunc : public Procedure{
        using funType = std::function<refObject(Scope,List<refExpression>)>;
        funType fun;
        size_t argc,defc;
        public:
            NativeFunc(size_t ac,size_t dc,funType fun,str name=L"<builtin-func>")
            : Procedure(Scope::null(),{},{},name){
                this->fun = fun;
                this->argc = ac;
                this->defc = dc;
            }
            size_t nArgs() const override{
                return argc;
            }
            size_t nDArgs() const override{
                return defc;
            }
            virtual refObject invoke(Scope outer,List<refExpression> args) override{
                if(args.size()<(argc-defc)||args.size()>argc){
                    throw FuncCallError(subst(L"Calling builtin-function "+name+L" which accepts $ arguments with $ arguments",{
                        std::to_wstring(argc),
                        std::to_wstring(args.size())
                        }));
                }
                while(args.size()<argc){
                    args.append(nullptr);
                }
                return fun(outer,args);
            }
    };
    class Object{
        refProcedure invokeproc;
        const Class* cls;
        public:
            Dict<> attrs;
            Data data;
            const Class& getClass() const{
                return *cls;
            }
            void setClass(const Class* newcls){
                cls = newcls;
            }
            //returns a pointer created by `new` to the copied object. Can be passed to a smart pointer constructor.
            Object* copy(){
                Object* newObj = new Object();
                newObj->invokeproc = invokeproc;
                newObj->cls = cls;
                newObj->attrs = attrs;//copy-by-value
                newObj->data = data;
                return newObj;
            }
            void bindMthTo(const refObject obj);
            Object() : invokeproc(nullptr), cls(nullptr), attrs({}), data(){}
            static inline refObject FuncObj(refProcedure proc);
            bool callable() const{
                return invokeproc != nullptr;
            }
            refProcedure onCall() const{
                return invokeproc;
            }
            virtual refObject invoke(Scope s,List<refExpression> args){
                if(invokeproc==nullptr){
                    throw UncallableError(L"Attempting to call a non-callable");
                }
                return invokeproc->invoke(s,args);
            }
            virtual void setattr(str attr,refObject val){
                attrs.addOrSet(attr,val);
            }
            virtual bool hasattr(str attr) const{
                return attrs.has(attr);
            }
            virtual refObject getattr(str attr) const{
                return attrs.get(attr);
            }
            virtual str stringify(Scope s) const;
            virtual ~Object(){}
    };
    class Expression{
        bool has_parens = false;
        public:
            void inline make_parenthesized(bool parens=true){
                has_parens = parens;
            }
            bool inline parenthesized() const{
                return has_parens;
            }
            virtual refObject evaluate(Scope s) = 0;
            virtual ~Expression(){}
            virtual bool is_lval() const{return false;}
            virtual void assignTo(Scope s,refObject expr){
                if(is_lval()){
                    throw std::logic_error("INTERNAL ERROR: lvalue assign handler missing");
                }
            }
    };
    class Statement{
        protected:
            virtual void execute(Scope s) = 0;
        public:
            size_t pos=-1;
            virtual void invoke(Scope s) final{
                execute(s);
            }
            virtual ~Statement(){}
    };
    class Constant : public Expression{
        refObject value;
        public:
            Constant(refObject value) : value(value){}
            virtual refObject evaluate(Scope s) override{
                return value;
            }
    };

    template<typename T,typename... Args>
    refProcedure pNew(Args... args);
    template<typename T,typename... Args>
    refExpression eNew(Args... args);
    template<typename T,typename... Args>
    refStatement sNew(Args... args);

    typedef List<Dict<wint_t,long long>> Opmap;
    extern const Opmap builtin_omp;
    Opmap gen_opmap(const List<str>& ops);

    void inline Object::bindMthTo(const refObject obj){
        if(callable()){
            invokeproc = Procedure::partial(invokeproc,{eNew<Constant>(obj)});
        }
    }
    using basic_op_t = std::function<refObject(Scope,refExpression,refExpression)>;
    struct BinOpData{
        basic_op_t func;
        size_t pty;
        BinOpData(size_t pty,basic_op_t func) : func(func), pty(pty){}
    };
}
#endif
