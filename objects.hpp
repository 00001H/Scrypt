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
            static refObject bind_opt_func(const str& name,const refObject& v){
                if(v->callable()){
                    return Object::FuncObj(Procedure::partial(v->onCall(),{eNew<Constant>(v)}));
                }
                return v;
            }
            Dict<> inheritFields(refClass inhfrom,const Dict<> existing){
                if(inhfrom==nullptr){
                    return existing;
                }
                Dict<> nw = existing;
                nw.addOrSetFrom(inhfrom->fields);
                nw.reevaluateEach(bind_opt_func);
                return nw;
            }
        public:
            Class(refClass inherits,const Dict<>& fields,str name,refProcedure ctor)
            : fields(inheritFields(inherits,fields)), name(name), ctor(ctor){
            }
            str getName() const{
                return name;
            }

            virtual refObject construct(Scope global,List<refExpression> args) const{
                refObject obj(new Object());
                obj->attrs.addOrSetFrom(fields);
                ctor->invoke(global,eNew<Constant>(obj)+args);
                obj->setClass(this);
                return obj;
            }
            virtual refObject construct(Scope s,const Data& x) const{
                std::wcerr << L"Warning: construct(Data) called on an unsupported class." << std::endl;
                std::terminate();
            }
            virtual str toStr(Scope s,const Object& obj) const{
                if(obj.hasattr(L"__str__")){
                    return obj.getattr(L"__str__")->invoke(s,{})->stringify(s);
                }
                return L"<Object@"+std::to_wstring(numberAddr(&obj))+L'>';
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
    class DataType : public Class{
        public:
            using Class::Class;
            virtual refObject construct(Scope s,const Data& dt) const override{
                try{
                    verifydt(dt);
                }catch(std::logic_error&){
                    throw std::logic_error(wtos(name)+":const Data& constructor internal error: invalid data type");
                }
                refObject obj(ObjectClass->construct(s,noArgs));
                obj->attrs.addOrSetFrom(fields);
                obj->data = dt;
                obj->setClass(this);
                return obj;
            }
            virtual void verifydt(const Data& dt) const = 0;
            template<typename _Ct>
            static refProcedure default_add_impl();
    };
    template<typename _Ct>
    refProcedure DataType::default_add_impl(){
        static refProcedure cachedFunc(new NativeFunc(2,0,[](Scope s,List<refExpression> args){
            refObject this_ = args[0]->evaluate(s);
            refObject freshnew = this_->getClass().construct(s,this_->data.as<_Ct>()+args[1]->evaluate(s)->data.as<_Ct>());
            return None;
        }));//one for each specialization
        return cachedFunc;
    };
    class StringType : public DataType{
        public:
            StringType() : DataType(
                ObjectClass,
                Dict<>::of(L"__add__",Object::FuncObj(DataType::default_add_impl<str>())),
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
            virtual void verifydt(const Data& dt) const override{
                dt.require(Data::S);
            }
    };
    clsDef(String);
    template<typename T>
    refProcedure pcd_convert_to(std::function<T(const str&)> from_str,Class* owner){
        static_assert(std::is_integral_v<T>||std::is_floating_point_v<T>);
        static refProcedure cachedFunc(new NativeFunc(2,0,[&,from_str,owner](Scope s,List<refExpression> args){
            refObject this_ = args[0]->evaluate(s);
            this_->setClass(owner);
            refObject tocvt = args[0]->evaluate(s);
            if(isinstanceof<DataType>(tocvt->getClass())){
                const Data& dt = tocvt->data;
                if(dt.is(Data::S)){
                    this_->data = from_str(dt.as<str>());
                }else if(dt.is(Data::I)){
                    this_->data = static_cast<T>(dt.as<long long>());
                }else if(dt.is(Data::D)){
                    this_->data = static_cast<T>(dt.as<long double>());
                }else{
                    throw ValueError(L"Unsupported conversion to number from a "+tocvt->getClass().getName());
                }
            }else{
                throw ValueError(L"Unsupported conversion to number from a "+tocvt->getClass().getName());
            }
            return Null;
        }));//one for each specialization
        return cachedFunc;
    }
    class LPFloatType : public DataType{
        static long double st2f(const str& st){
            return parse_num<long double>(st,wstold);
        }
        static const refProcedure add_impl;
        public:
            long double get(const Object& obj) const{
                assert(isinstanceof<LPFloatType>(obj.getClass()));
                return obj.data.as<long double>();
            }
            LPFloatType() : DataType(
                ObjectClass,
                Dict<>::of(L"__add__",Object::FuncObj(add_impl)),
                L"float",
                pcd_convert_to<long double>(st2f,this)
            ){}
            str toStr(Scope s,const Object& obj) const override{
                return std::to_wstring((obj.data).as<long double>());
            }
            virtual void verifydt(const Data& dt) const override{
                dt.require(Data::D);
            }
    };
    clsDef(LPFloat);
    class LPIntType : public DataType{
        static long long st2i(const str& st){
            return parse_num<long long>(st,wstoll);
        }
        static const refProcedure add_impl;
        public:
            long long get(const Object& obj) const{
                assert(isinstanceof<LPIntType>(obj.getClass()));
                return obj.data.as<long long>();
            }
            LPIntType() : DataType(
                ObjectClass,
                Dict<>::of(L"__add__",Object::FuncObj(add_impl)),
                L"int",
                pcd_convert_to<long long>(st2i,this)
            ){}
            str toStr(Scope s,const Object& obj) const override{
                return std::to_wstring((obj.data).as<long long>());
            }
            virtual void verifydt(const Data& dt) const override{
                dt.require(Data::I);
            }
    };
    clsDef(LPInt);
    const refProcedure LPIntType::add_impl(
        new NativeFunc(2,0,[](Scope s,List<refExpression> args){
            refObject this_ = args[0]->evaluate(s);
            refObject that_ = args[1]->evaluate(s);
            refObject dest;
            const Class& thatcls = that_->getClass();
            if(isinstanceof<LPIntType>(thatcls)){
                dest = that_;
            }else if(isinstanceof<LPFloatType>(thatcls)){
                return that_->getattr(L"__add__")->invoke(s,{eNew<Constant>(this_)});
            }else{
                throw OperatorError(L"unknown target type");
            }
            const LPIntType& lpf = dynamic_cast<const LPIntType&>(*LPFloat);
            return LPFloat->construct(s,lpf.get(*this_)+lpf.get(*that_));
        })
    );
    const refProcedure LPFloatType::add_impl(
        new NativeFunc(2,0,[](Scope s,List<refExpression> args){
            refObject this_ = args[0]->evaluate(s);
            refObject that_ = args[1]->evaluate(s);
            refObject dest;
            const Class& thatcls = that_->getClass();
            if(isinstanceof<LPFloatType>(thatcls)){
                dest = that_;
            }else if(isinstanceof<LPIntType>(thatcls)){
                const LPIntType& thati = dynamic_cast<const LPIntType&>(thatcls);
                dest = LPFloat->construct(s,static_cast<long double>(thati.get(*that_)));
            }else{
                throw OperatorError(L"unknown target type");
            }
            const LPFloatType& lpf = dynamic_cast<const LPFloatType&>(*LPFloat);
            return LPFloat->construct(s,lpf.get(*this_)+lpf.get(*that_));
        })
    );
    #undef clsDef
    inline refObject Object::FuncObj(refProcedure proc){
        refObject obj(ObjectClass->construct(Scope::null(),noArgs));
        obj->invokeproc = proc;
        return obj;
    }
}
#endif
