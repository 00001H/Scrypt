#ifndef SCP_DSTRUCT
#define SCP_DSTRUCT
#include"typedef.hpp"
namespace scrypt{
    template<typename E=refObject>
    using List = cppp::List<E>;
    template<typename K=str, typename V=refObject>
    using Dict = cppp::Dict<K,V>;
    namespace detail{
        struct ScopeImpl;
        using psi = std::shared_ptr<ScopeImpl>;
        struct ScopeImpl{
            const psi outer;
            Dict<> inner;
            ScopeImpl() : outer(nullptr), inner(){}
            ScopeImpl(const Dict<>& inner,const psi outer)
            : outer(outer), inner(inner){}
            bool isSimple() const{
                return outer==nullptr;
            }
            bool has(str key) const{
                return inner.has(key)||outer->has(key);
            }
            refObject& get(str key){
                if(inner.has(key)){
                    return inner.get(key);
                }
                if(outer==nullptr){
                    throw std::out_of_range(wtos(key));
                }
                return outer->get(key);
            }
            const refObject& get(str key) const{
                if(inner.has(key)){
                    return inner.get(key);
                }
                if(outer==nullptr){
                    throw std::out_of_range(wtos(key));
                }
                return outer->get(key);
            }
            const Dict<>& simple() const{
                return inner;
            }
        };
    }
    class Scope{
        detail::psi impl;
        /*
        About _const_impl:
        Special "globally shared" Scope instances *should* be const,
        because global non-consts *is* shared state, which is usually
        very bad. However, virtual function implementations
        may not use the actual scope, but is required to have its
        function signature accept a Scope parameter. Moreover,
        some implementations may *assign* to the scope, therefore
        *all* implementations are forced to take a non-const Scope
        instance only. This poses a problem, as this makes it impossible
        to pass a shared global "dummy scope" constant to these
        virtual methods.
        This _const_impl flag solves this issue. A const-impl Scope
        can be accessed normally, but throws a std::logic_error if
        modification is attemted. This allows sharing global
        "dummy scopes" or even "scope templates", and makes sure
        these scopes are not polluted with trash data in case they
        are mistakenly passed to implementations that actually
        *does* write to the passed scope.
        */
        bool _const_impl;
        public:
            Scope() : impl(new detail::ScopeImpl()), _const_impl(false){}
            Scope(detail::psi newmpl) : impl(newmpl), _const_impl(false){}
            Scope(bool _ic) : impl(new detail::ScopeImpl()), _const_impl(_ic){}
            static Scope clone(const Scope& other);
            bool isSimple() const{
                return impl->isSimple();
            }
            const Dict<>& simple() const{
                return impl->simple();
            }
            static Scope compoundScope(const Scope& inner,const Scope& outer){
                return Scope(std::make_shared<detail::ScopeImpl>(inner.impl->simple(),outer.impl));
            }
            bool has(str key) const{
                return impl->has(key);
            }
            refObject& get(str key){
                return impl->get(key);
            }
            const refObject& get(str key) const{
                return impl->get(key);
            }
            void assign(str key,refObject val){
                if(_const_impl){
                    throw std::logic_error("Attempting to assign to a const-impl Scope");
                }
                impl->inner.addOrSet(key,val);
            }
            static inline Scope& null();
    };
    namespace detail{
        Scope nulScope{true};
    }
    Scope& Scope::null(){
        return detail::nulScope;
    }
    Scope Scope::clone(const Scope& other){
        return Scope(std::make_shared<detail::ScopeImpl>(*other.impl));
    }
    class Data{
        public:
            enum DT{
                I,D,S,A,N
            };
        private:
            std::any a;
            DT what;
            [[noreturn]] void bad() const{
                throw std::logic_error("Data: wrong underlying type!");
            }
        public:
            bool is(DT tp) const{
                return what==tp;
            }
            void require(DT type) const{
                if(what!=type){
                    bad();
                }
            }
            Data() : what(N){};
            DT which() const{
                return what;
            }
            Data(default_int_t x) : Data(){
                *this = x;
            }
            Data(default_float_t x) : Data(){
                *this = x;
            }
            Data(str x) : Data(){
                *this = x;
            }
            Data(void* x) : Data(){
                *this = x;
            }
            Data& operator =(default_int_t x){
                what = I;
                a = x;
                return *this;
            };
            Data& operator =(default_float_t x){
                what = D;
                a = x;
                return *this;
            };
            Data& operator =(str x){
                what = S;
                a = x;
                return *this;
            };
            Data& operator =(void* x){
                what = A;
                a = x;
                return *this;
            };
            ~Data(){
            }
            template<typename T>
            T as() const{
                throw std::logic_error("unsupported Data::as specialization");
            }
    };
    template<>
    str Data::as<str>() const{
        if(what!=S)bad();
        return std::any_cast<str>(a);
    }
    template<>
    void* Data::as<void*>() const{
        if(what!=A)bad();
        return std::any_cast<void*>(a);
    }
    template<>
    default_int_t Data::as<default_int_t>() const{
        if(what!=I)bad();
        return std::any_cast<default_int_t>(a);
    }
    template<>
    default_float_t Data::as<default_float_t>() const{
        if(what!=D)bad();
        return std::any_cast<default_float_t>(a);
    }
}
#endif
