#ifndef SCP_TYPEDEF
#define SCP_TYPEDEF
//Centralized header includes: improves preprocessor performance
#include<algorithm>
#include<complex>
#include<optional>
#include<cfenv>
#include<cmath>
#include<string>
#include<memory>
#include<unordered_map>
#include<deque>
#include<iostream>
#include<functional>
#include<exception>
#include<stdexcept>
#include<cstdlib>
#include<cstring>
#include<string>
#include<vector>
#include<deque>
#include<any>
#include"cppp.hpp"
using cppp::wtos;
using cppp::stow;
using cppp::isinstanceof;
namespace scrypt{
    typedef int64_t default_int_t;
    typedef long double default_float_t;
    static_assert(sizeof(default_float_t)>=8);
    #define ref(x) typedef std::shared_ptr<x> ref##x
    using str = std::wstring;
    /*
    SCRYPT_POTENTIAL_ISSUE 0002 The <cassert> header include
    Originally, the <cassert> header was included. This posed a problem,
    as it (despite the "migrated-to-C++ <cxxx> name") does NOT put the
    `assert` macro in the std namespace(because macros do not respect
    namespaces), which pollutes the global namespace.
    Therefore, a replacement "assert" function is included here.
    */
    #ifdef NDEBUG
    void assert(bool must_be_true,str optmsg=L""){}
    #else
    void assert(bool must_be_true,str optmsg=L"<no info>"){
        if(!must_be_true){
            throw std::logic_error("Assertion failed: "+wtos(optmsg));
        }
    }
    #endif
    class Object;
    ref(Object);
    extern const refObject Null;
    extern const refObject None;
    class Expression;
    ref(Expression);
    class Statement;
    ref(Statement);
    class Class;
    ref(Class);
    class Procedure;
    ref(Procedure);
    namespace priorities{
        using pty_t = const size_t;
        pty_t PTY_MAX = 50000;
        pty_t EXPNT = 8200;
        pty_t DIVMUL = 8000;
        pty_t ADDSUB = 7800;
    }
    namespace op_specs{
        struct op_spec{
            str st;
            str sym;
            op_spec(str st,str sym) : st(st), sym(sym){}
            str dunder() const{return L"__"+st+L"__";}
            str rdunder() const{return L"__r"+st+L"__";}
        };
        op_spec add(L"add",L"+");
    }
    #undef ref
}
#endif
