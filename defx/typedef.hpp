#ifndef SCP_TYPEDEF
#define SCP_TYPEDEF
//Centralized header includes: improves preprocessor performance
#include<algorithm>
#include<complex>
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
namespace scrypt{
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
    void assert(bool must_be_true){}
    #else
    void assert(bool must_be_true){
        if(!must_be_true){
            throw std::logic_error("Assertion failed");
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
    #undef ref
}
#endif
