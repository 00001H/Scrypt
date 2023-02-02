#ifndef SCP_UTILS
#define SCP_UTILS
#include"defx/typedef.hpp"
#include"defx/dstruct.hpp"
#include"errors.hpp"
namespace scrypt{
    //logb(a) = log(a)/log(b)
    template<typename T>
    T log2p(T base, T number){
        return std::log(number)/std::log(base);
    }
    using cppp::safepow;
    using cppp::parse_num;
    template<typename A>
    List<str> toStr(const List<A>& stuff){
        return cppp::map<str,A>([](const A& x){
            return std::to_wstring(x);
        },cppp::iter(stuff));
    }
}
#endif
