#ifndef SCP_OBJECTS
#define SCP_OBJECTS
#include"errors.hpp"
#include"defs.hpp"
#include"utils.hpp"
namespace scrypt{
    const refObject Null(nullptr);
    const refObject None(new Object());
    class Class{
        protected:
            Dict<> fields;
            str name;
            refProcedure ctor;
            Dict<> inheritFields(refClass inhfrom,const Dict<> existing){
                if(inhfrom==nullptr){
                    return existing;
                }
                Dict<> nw = existing;
                nw.addOrSetFrom(inhfrom->fields);
                return nw;
            }
        private:
            void pre_construct(Scope scop,refObject& obj) const{
                obj->attrs.addOrSetFrom(fields);
                obj->setClass(this);
            }
            void post_construct(Scope scop,refObject& obj) const{
                obj->attrs.forEach([&obj](const str& attrname,refObject& attrval){
                    if(attrval->callable()){
                        attrval = refObject(attrval->copy());
                        attrval->bindMthTo(obj);
                    }
                });
            }
        protected:
            virtual void initialize(Scope& s,refObject& o,const List<refExpression>& args) const{
                ctor->invoke(s,eNew<Constant>(o)+args);
            }
            virtual void initialize(Scope& s,refObject& o,const Data& x) const{
                std::wcerr << L"Warning: construct(Data) called on an unsupported class." << std::endl;
                std::terminate();
            }
        public:
            Class(refClass inherits,const Dict<>& fields,str name,refProcedure ctor)
            : fields(inheritFields(inherits,fields)), name(name), ctor(ctor){
            }
            str getName() const{
                return name;
            }

            refObject construct(Scope scop,List<refExpression> args) const{
                refObject obj(new Object());
                pre_construct(scop,obj);
                initialize(scop,obj,args);
                post_construct(scop,obj);
                return obj;
            }
            refObject construct(Scope scop,const Data& dat) const{
                refObject obj(new Object());
                pre_construct(scop,obj);
                initialize(scop,obj,dat);
                post_construct(scop,obj);
                return obj;
            }
            virtual str toStr(Scope s,const Object& obj) const{
                if(obj.hasattr(L"__str__")){
                    return obj.getattr(L"__str__")->invoke(s,{})->stringify(s);
                }
                if(obj.callable()){
                    return L"<Function\""+obj.onCall()->getName()+L"\">";
                }
                return L"<Object@"+std::to_wstring(size_t(&obj))+L'>';
            }
            virtual ~Class(){}
    };
    str Object::stringify(Scope s) const{
        return cls->toStr(s,*this);
    }
    #define clsDef(nm) const refClass nm(new nm ## Type())
    class ObjectClassType : public Class{
        public:
            ObjectClassType() : Class(nullptr,Dict<>::of(),L"Object",std::make_shared<Procedure>(Scope::null(),List<refStatement>{},List<str>{L"this"})){}
    };
    clsDef(ObjectClass);
    class numeric_operator{
        public:
            using di = default_int_t;
            using df = default_float_t;
            virtual di operator()(di a,di b) = 0;
            virtual df operator()(df a,df b) = 0;
            virtual ~numeric_operator(){}
    };
    namespace numop_impl{
        class _add : public numeric_operator{
            public:
                virtual di operator()(di a,di b){return a+b;}
                virtual df operator()(df a,df b){return a+b;}
        };
        _add adder;
    }
    template<typename _It>
    class DataType;
    static DataType<default_float_t>* lpfloat();
    static DataType<default_int_t>* lpint();
    namespace{
        template<typename T>
        str towstr(T x){
            return std::to_wstring(x);
        }
        template<>
        str towstr<str>(str x){
            return x;
        }
    }
    template<typename _It>
    class DataType : public Class{
        protected:
            virtual str toStr(Scope s,const Object& obj) const override{
                return towstr(obj.data.as<_It>());
            }
            static refProcedure boilerplate_arith_wrapper(numeric_operator* x){
                refProcedure cachedImpl(new NativeFunc(2,0,
                [x](Scope s,List<refExpression> args){
                    refObject this_ = args[0]->evaluate(s);
                    refObject other = args[1]->evaluate(s);
                    if(!isinstanceof<DataType<_It>>(this_->getClass())){
                        throw OperatorError(L"Stolen operators don't work");
                    }
                    constexpr bool thisflt=std::is_floating_point_v<_It>;
                    bool otherflt=false;
                    if(isinstanceof<DataType<default_float_t>>(other->getClass())){
                        otherflt=true;
                    }else if(!isinstanceof<DataType<default_int_t>>(other->getClass())){
                        throw OperatorError(L"Numeric operation attempted against nonnumeric type");
                    }
                    bool flt=thisflt||otherflt;
                    if(flt){
                        default_float_t tft = (
                    thisflt
                        ?this_->data.as<default_float_t>()
                        :static_cast<default_float_t>(this_->data.as<default_int_t>())
                        );
                        default_float_t oft = (
                        otherflt
                        ?other->data.as<default_float_t>()
                        :static_cast<default_float_t>(other->data.as<default_int_t>())
                        );
                        default_float_t rslt = (*x)(tft,oft);
                        return lpfloat()->construct(s,rslt);
                    }else{
                        default_int_t tft = this_->data.as<default_int_t>();
                        default_int_t oft = other->data.as<default_int_t>();
                        default_int_t rslt = (*x)(tft,oft);
                        return lpint()->construct(s,rslt);
                    }
                }
                ));
                return cachedImpl;
            }
            Dict<> getrequiredfields(){
                Dict<> fields;
                if(std::is_arithmetic_v<_It>){
                    fields.addOrSet(
                        op_specs::add.dunder(),
                        Object::FuncObj(boilerplate_arith_wrapper(auto(&numop_impl::adder)))
                    );
                }
                return fields;
            }
        public:
            _It get(refObject x){
                assert(isinstanceof<DataType<_It>>(x));
                return x->data.as<_It>();
            }
            using Class::Class;
            DataType(str name) : Class(
                ObjectClass,
                getrequiredfields(),
                name,
                refProcedure(
                    new NativeFunc(1,0,
                    [](Scope s,List<refExpression> args){
                        return Null;
                    }
                    ,name+L"@constructor")
                )
            ){}
            virtual void initialize(Scope& s,refObject& obj, const Data& dt) const override{
                try{
                    verifydt(dt);
                }catch(std::logic_error&){
                    throw std::logic_error(wtos(name)+":const Data& constructor internal error: invalid data type");
                }
                obj->attrs.addOrSetFrom(fields);
                obj->data = dt;
                obj->setClass(this);
            }
            void verifydt(const Data& dt) const{
                dt.as<_It>();
            }
    };
    DataType<default_float_t>* lpfloat(){
        static DataType<default_float_t> obj(L"float");
        return &obj;
    }
    DataType<default_int_t>* lpint(){
        static DataType<default_int_t> obj(L"int");
        return &obj;
    }
    class StringType : public DataType<str>{
        public:
            static str sget(const Object& obj){
                assert(isinstanceof<StringType>(obj.getClass()));
                return obj.data.as<str>();
            }
            StringType() : DataType(
                ObjectClass,
                Dict<>::of(L"__add__",Object::FuncObj(refProcedure(new NativeFunc(2,0,[](Scope s,List<refExpression> args){
                refObject this_ = args[0]->evaluate(s);
                refObject that_ = args[1]->evaluate(s);
                if(&that_->getClass()!=&this_->getClass()){
                    throw OperatorError(L"Unsupported type");
                }
                return this_->getClass().construct(s,this_->data.as<str>()+that_->data.as<str>());
            })))),
                L"str",
                refProcedure(
                    new NativeFunc(1,0,[](Scope s,List<refExpression> args){
                        args[0]->evaluate(s)->data = args[1]->evaluate(s)->stringify(s);
                        return Null;
                    })
                )
            ){}
            str toStr(Scope s,const Object& obj) const override{
                return (obj.data).as<str>();
            }
    };
    clsDef(String);
    #undef clsDef
    inline refObject Object::FuncObj(refProcedure proc){
        refObject obj(ObjectClass->construct(Scope::null(),noArgs));
        obj->invokeproc = proc;
        return obj;
    }
}
#endif
