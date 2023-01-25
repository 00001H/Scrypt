#ifndef SCP_UTILS
#define SCP_UTILS
#include"defx/typedef.hpp"
#include"defx/dstruct.hpp"
#include"errors.hpp"
#include"cppp.hpp"
namespace scrypt{
    //logb(a) = log(a)/log(b)
    template<typename T>
    T log2p(T base, T number){
        return std::log(number)/std::log(base);
    }
    template<typename T>
    bool is_pow_outofrange(T base,T xpnt){
        if(base<0){
            base = -base;
            if(base<0){
                return true;
            }
        }
        if(xpnt<0){
            if(xpnt!=-1 && !std::is_floating_point_v<T>){
                throw std::logic_error("Negative exponents for integers are meaningless");
            }
            return is_pow_outofrange(base,-xpnt);
        }
        //base&xpnt >= 0
        T maxexponent = log2p(base,std::numeric_limits<T>::max());
        return xpnt>=maxexponent;
    }
    template<typename Numeric_T>
    Numeric_T safepow(Numeric_T base,Numeric_T exp){
        Numeric_T one(1);
        Numeric_T zero(0);
        Numeric_T epsilon = std::numeric_limits<Numeric_T>::epsilon();
        if(exp==zero)return one;
        if(base==zero)return zero;//ordering is important, 0^0 = 1
        if(is_pow_outofrange(base,exp)){
            if(std::numeric_limits<Numeric_T>::has_infinity){
                return std::numeric_limits<Numeric_T>::infinity();
            }
            throw ArithmeticError(L"pow() result overflow/underflow");
        }
        if(base<0){
            std::complex<Numeric_T> possibly_complex_result = 
            std::pow(std::complex<Numeric_T>(base),std::complex<Numeric_T>(exp));
            if(possibly_complex_result.imag()>epsilon){
                throw ArithmeticError(L"pow() value is not a real number");
            }
            return possibly_complex_result.real();
        }
        if(exp<zero){
            if(-exp<zero){
                throw std::runtime_error("pow() exponent underflow");
            }
            return one/safepow(base,-exp);
        }
        auto result = std::pow(base,exp);
        return result;
    }
    template<typename Numeric_T>
    Numeric_T parse_num(str s,bool is_bignum=false,Numeric_T multiplier=10,bool rev=true){
        if(!s.length()){
            throw std::invalid_argument("parse_num: no number");
        }
        bool invert = s[0]==L'-';
        if(invert){
            s = s.substr(1);
        }
        Numeric_T maxv = (is_bignum?-1:std::numeric_limits<Numeric_T>::max());
        while(s.contains(L'E')){
            s[s.find(L'E')] = L'e';
        }
        if(s.contains(L'e')){
            str bfore = s.substr(0,s.find(L'e'));
            str after = s.substr(s.find(L'e')+1);
            if(bfore.empty()||after.empty()||after.contains(L'e')){
                throw std::invalid_argument("parse_num: Invalid placement of \"e\" or multiple \"e\"s detected");
            }
            Numeric_T mn = parse_num<Numeric_T>(bfore);
            Numeric_T exp = parse_num<Numeric_T>(after);
            Numeric_T rslt = mn*safepow<Numeric_T>(10,exp);
            if(invert)return -rslt;
            return rslt;
        }
        if(s.contains('.')){
            if(!std::is_floating_point_v<Numeric_T>){
                throw std::invalid_argument("parse_num: decimal point found while parsing an integer");
            }
            str bfore = s.substr(0,s.find(L'.'));
            str after = s.substr(s.find(L'.')+1);
            if(bfore.empty()||after.empty()||after.contains(L'.')){
                throw std::invalid_argument("parse_num: Invalid placement of \".\" or multiple \".\"s detected");
            }
            Numeric_T whl = parse_num<Numeric_T>(bfore,is_bignum);
            Numeric_T dcml = parse_num<Numeric_T>(after,is_bignum,0.1,!rev);
            return whl+dcml/Numeric_T(10);
        }
        Numeric_T rslt(0);
        Numeric_T dgt;
        Numeric_T multipliere(1);
        bool bad_on_next = false;
        size_t j;
        wchar_t ch;
        for(size_t i=0;i<s.length();++i){
            j = ((rev)?(s.length()-1-i):(i));
            ch = s[j];
            if(bad_on_next){
                throw std::out_of_range("Number over/underflow");
            }
            if(ch<L'0'||ch>L'9'){
                throw std::invalid_argument("parse_num: non-numeric character "+wtos(std::wstring()+ch));
            }
            dgt = static_cast<Numeric_T>(ch-L'0');
            if(!is_bignum){
                if(multipliere>Numeric_T(1)&&((maxv/multipliere)<dgt)){
                    throw std::out_of_range("Number over/underflow");
                }
            }
            dgt *= multipliere;
            if(!is_bignum&&multiplier>1){
                if(maxv/multipliere < multiplier){
                    bad_on_next = true;
                }
            }
            multipliere *= multiplier;
            if(!is_bignum){
                if((maxv-rslt)<dgt){
                    throw std::out_of_range("Number over/underflow");
                }
            }
            rslt += dgt;
        }
        std::endl(std::wcout);
        if(invert)return -rslt;
        return rslt;
    }
    str subst(str x,List<str> values){
        size_t j=0;
        str out;
        wchar_t curr,peek;
        for(size_t i=0;i<x.size();++i){
            curr = x[i];
            if(i==(x.size()-1)){
                peek = L'\0';
            }else{
                peek = x[i+1];
            }
            if(curr==L'$'&&peek==L'$'){
                out += L'$';
                ++i;
            }else if(curr==L'$'&&peek!=L'$'){
                if(j<values.size()){
                    out += values[j];
                    ++j;
                }else{
                    out += L'$';
                }
            }else if(curr!=L'#'){
                out += curr;
            }
        }
        return out;
    }
    template<typename A>
    List<str> toStr(const List<A>& stuff){
        List<str> rslt;
        for(const A& x : stuff){
            rslt.append(std::to_string(x));
        }
        return rslt;
    }
}
#endif
