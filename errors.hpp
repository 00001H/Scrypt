#ifndef SCP_ERRORS
#define SCP_ERRORS
#include"defx/typedef.hpp"
#include"defx/dstruct.hpp"
#include"cppp.hpp"
namespace scrypt{
    class ParseError : public std::exception{
        size_t ch;
        const char* wht0;
        std::string wht1;
        public:
            ParseError(std::string wht,size_t place) : ch(place){
                wht1 = wht+" at "+std::to_string(place);
                wht0 = wht1.c_str();
            }
            ParseError(str wht,size_t place) : ParseError(wtos(wht),place){}
            const char* what() const noexcept{
                return wht0;
            }
    };
    class NoParse : public ParseError{
        public:
            using ParseError::ParseError;
    };
    struct StacktraceFrame{
        str file;
        str name;
        size_t lineno;
        StacktraceFrame(str file,str name,size_t pos) : file(file), name(name), lineno(pos){}
        public:
            str dump() const{
                str result = L"File "+file;
                result += L", pos "+std::to_wstring(lineno)+L"\n";
                result += L"In function "+name;
                return result;
            }
    };
    class Stacktrace{
        List<StacktraceFrame> frames;
        public:
            Stacktrace() = default;
            Stacktrace(List<StacktraceFrame> frms) : frames(frms){}
            void addFrame(const StacktraceFrame& frm){
                frames.append(frm);
            }
            str dump() const{
                str result;
                for(const auto& frame : frames){
                    result += L'\n'+frame.dump();
                }
                if(result.empty())return L"";
                return result.substr(1);
            }
    };
    class ProgramError : public std::exception{
        Stacktrace st;
        str rson;
        mutable std::string hold_ref;
        mutable bool held_ref = false;
        private:
        protected:
            virtual constexpr str clsnm() const = 0;
        public:
            ProgramError(str rson) : rson(rson){
            }
            ProgramError() : rson(L"<reason unspecified>"){
            }
            void addFrame(const StacktraceFrame& frm){
                st.addFrame(frm);
            }
            str dump() const{
                str rslt = L"\nUncaught "+clsnm()+L"("+rson+L").\nTraceback:\n";
                rslt += st.dump();
                return rslt;
            }
            const char* what() const noexcept{
                if(!held_ref){
                    hold_ref = wtos(dump());
                    held_ref = true;
                }
                return hold_ref.c_str();
            }
    };
    #define newIErr(name,inh) class name: public inh {using inh :: inh ;protected:str constexpr clsnm() const override{return L ## #name;}}
    #define newErr(name) newIErr(name,ProgramError);
    newErr(VariableNotFoundError);
    newErr(FuncCallError);
    newIErr(UncallableError,FuncCallError);
    newErr(AssignmentError);
    newErr(NumericError);
    newIErr(ArithmeticError,NumericError);
    newErr(ValueError);
    newErr(OperatorError);
    class ReturnValue : public std::exception{
        refObject value;
        public:
            ReturnValue(refObject val) : value(val){}
            refObject get() const{
                return value;
            }
    };
}

#endif
