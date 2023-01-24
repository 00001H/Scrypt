#ifndef SCP_DSTRUCT
#define SCP_DSTRUCT
#include"typedef.hpp"
#include"cppp.hpp"
namespace scrypt{
    template<typename E=refObject>
    class List{
        std::deque<E> container;
        public:
            List() = default;
            List(std::initializer_list<E> ilist) : container(ilist){}
            List(const std::basic_string<E>& bstr){
                for(const E& elem : bstr){
                    append(elem);
                }
            }
            operator std::basic_string<E> () const{
                return std::basic_string<E>(container);
            }
            E& operator[](long long i){
                if(i<0)i += size();
                return container[i];
            }
            const E& operator[](long long i) const{
                if(i<0)i += size();
                return container[i];
            }
            bool empty() const{
                return container.empty();
            }
            void append(const E& elem){
                container.push_back(elem);
            }
            void extend(const List<E>& other){
                for(const E& e : other)
                    append(e);
            }
            template<typename _CmpType>
            void sort(_CmpType less){
                std::stable_sort(container.begin(),container.end(),less);
            }
            size_t size() const{
                return container.size();
            }
            E&& pop() const{
                E&& elem = std::move(container.back());
                container.pop_back();
                return elem;
            }
            void prepend(const E& elem){
                container.push_front(elem);
            }
            long long find(const E& elem) const{
                for(size_t i=0;i<container.size();++i){
                    if(container[i]==elem)return static_cast<long long>(i);
                }
                return -1;
            }
            bool contains(const E& elem) const{
                return find(elem)!=-1;
            }
            auto begin(){
                return container.begin();
            }
            auto cbegin() const{
                return container.cbegin();
            }
            auto begin() const{
                return cbegin();
            }
            auto end(){
                return container.end();
            }
            auto cend() const{
                return container.cend();
            }
            auto end() const{
                return cend();
            }
    };
    template<typename E>
    void operator+=(List<E>& lis,const List<E>& lis2){
        lis.extend(lis2);
    }
    template<typename E>
    List<E> operator+(const List<E>& lis,const List<E>& lis2){
        List<E> nlis = lis;
        nlis += lis2;
        return nlis;
    }
    template<typename A,typename E>
    void operator+=(List<E>& lis,const A& after){
        lis.append(after);
    }
    template<typename A,typename E>
    List<E> operator+(const List<E>& lis,const A& after){
        List<E> nlis = lis;
        nlis += after;
        return nlis;
    }
    template<typename A,typename E>
    List<E> operator+(const A& before,const List<E>& lis){
        List<E> nlis = lis;
        nlis.prepend(before);
        return nlis;
    }
    template<typename K=str,typename V=refObject>
    class Dict{
        std::unordered_map<K,V> container;
        public:
            List<K> keys() const{
                List<K> kys;
                for(const auto&[key,val] : container){
                    kys.append(key);
                }
                return kys;
            }
            size_t size() const{
                return container.size();
            }
            template<typename... Args>
            static Dict<K,V> of(const K& key,const V& val,Args... remaining){
                Dict<K,V> dic;
                dic.addOrSet(key,val);
                dic.addOrSetFrom(of(remaining...));
                return dic;
            }
            decltype(container) data(){
                return container;
            }
            static Dict<K,V> of(){
                return Dict<K,V>();
            }
            using reevaluator_type = std::function<void(const K&, V&)>;
            void reevaluateEach(reevaluator_type func){
                std::unordered_map<K,V> nc;
                for(auto&[key,val] : container){
                    func(key,val);
                }
            }
            bool addIfMissing(const K& key,const V& val){
                return container.insert(key,val).second;
            }
            void addOrSet(const K& key,const V& val){
                container.insert_or_assign(key,val);
            }
            void addOrSetFrom(const Dict<K,V>& other){
                for(const auto&[key,val] : other){
                    addOrSet(key,val);
                }
            }
            void del(const K& key){
                container.erase(key);
            }
            bool has(const K& key) const{
                return container.contains(key);
            }
            V& get(const K& key){
                return container.at(key);
            }
            const V& get(const K& key) const{
                return container.at(key);
            }
            V& get(const K& key,V& orelse){
                if(container.contains(key)){
                    return container.at(key);
                }
                return orelse;
            }
            const V& get(const K& key,const V& orelse) const{
                if(container.contains(key)){
                    return container.at(key);
                }
                return orelse;
            }
            auto begin(){
                return container.begin();
            }
            auto cbegin() const{
                return container.cbegin();
            }
            auto begin() const{
                return cbegin();
            }
            auto end(){
                return container.end();
            }
            auto cend() const{
                return container.cend();
            }
            auto end() const{
                return cend();
            }
    };
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
            Data(long long x) : Data(){
                *this = x;
            }
            Data(long double x) : Data(){
                *this = x;
            }
            Data(str x) : Data(){
                *this = x;
            }
            Data(void* x) : Data(){
                *this = x;
            }
            Data& operator =(long long x){
                what = I;
                a = x;
                return *this;
            };
            Data& operator =(long double x){
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
    long long Data::as<long long>() const{
        if(what!=I)bad();
        return std::any_cast<long long>(a);
    }
    template<>
    long double Data::as<long double>() const{
        if(what!=D)bad();
        return std::any_cast<long double>(a);
    }
}
#endif
