#ifndef SCP_PARSUTILS
#define SCP_PARSUTILS
#include"..\defs.hpp"
#include"../expression.hpp"
#include"../statements.hpp"
namespace scrypt{
    namespace{
        const size_t OK=-1,DONE_INDEX=1;
    }
    Opmap gen_opmap(const List<str>& ops){
        Opmap opmap{{},Dict<wint_t,size_t>::of(WEOF,OK)};
        size_t state,ostate;
        wchar_t ch;
        for(const auto& op : ops){
            state = 0;
            for(size_t i=0;i<op.size();i++){
                ch = op[i];
                if(opmap[state].has(ch)){
                    ostate = state;
                    state = opmap[state].get(ch);
                    if(state==DONE_INDEX){
                        state = opmap.size();
                        opmap[ostate].addOrSet(ch,state);
                        opmap.append(Dict<wint_t,size_t>::of(WEOF,OK));
                    }else if(i==(op.size()-1)){
                        opmap[state].addOrSet(WEOF,OK);
                    }
                }else{
                    if(i==(op.size()-1)){
                        opmap[state].addOrSet(ch,DONE_INDEX);
                        break;
                    }
                    opmap[state].addOrSet(ch,opmap.size());
                    state = opmap.size();
                    opmap.append({});
                    if(i==0){
                        opmap[0].addOrSet(ch,state);
                    }
                }
            }
        }
        return opmap;
    }
    const List<wchar_t> whitespaces
    {L'\t',L' ',L'\0'};
    class Scanner{
        using i_t = std::make_signed_t<size_t>;
        str data;
        i_t loc;
        public:
            size_t pos() const{
                return loc;
            }
            Scanner(const str& dat) : data(dat), loc(0){}
            Scanner() : Scanner(L""){}
            void reset(str nw,i_t npos=0ull){
                data = nw;
                seek(npos);
            }
            bool seek(i_t pos){
                loc = std::max<i_t>(0,std::min<i_t>(pos,data.length()));
                return loc==pos;
            }
            size_t consume_lwspcs(const List<wchar_t>& whtspcs=whitespaces){
                size_t i=0;
                while(whtspcs.contains((*this)())){advance();++i;}
                return i;
            }
            bool advance(i_t dta=1){
                return seek(loc+dta);
            }
            bool done() const{
                return loc==i_t(data.length());
            }
            wint_t operator()(i_t fwd=0) const{
                i_t nwp = loc+fwd;
                if(nwp<0||nwp>=i_t(data.length()))return WEOF;
                return data[nwp];
            }
            str next(const std::function<bool(wchar_t, size_t)>& pred){
                size_t i = 0;
                str mtchd;
                while(!done()&&pred((*this)(),i)){mtchd+=(*this)();advance();++i;}
                return mtchd;
            }
            wint_t next(const List<wchar_t>& options){
                wint_t tmp = (*this)();
                if(tmp==WEOF)return WEOF;
                for(const wchar_t& option : options){
                    if(option==tmp){
                        advance();
                        return option;
                    }
                }
                return WEOF;
            }
            str next(const str& st){
                size_t adv=0;
                for(const wchar_t& ch : st){
                    if((*this)(adv)!=static_cast<wint_t>(ch))return L"";
                    ++adv;
                }
                advance(adv);
                return st;
            }
            bool next(wint_t ch){
                if(done())return false;
                bool x = ((*this)()==ch);
                if(x){
                    advance();
                }
                return x;
            }
            str next(const Opmap& mp){
                size_t state = 0;
                List<size_t> exitpoints;
                size_t mov=0;
                wint_t ch;
                str matched;
                while(((ch=(*this)(mov))!=WEOF)&&(mp[state].has(ch))){
                    state = mp[state].get(ch);
                    ++mov;
                    matched += ch;
                    if(mp[state].get(WEOF,-2)==size_t(-1)){
                        exitpoints.append(mov);
                    }
                }
                if(exitpoints.empty()){
                    return L"";
                }
                advance(exitpoints[-1]);
                return matched.substr(0,exitpoints[-1]);
            }
    };
    const List<wchar_t> alphabet(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    const List<wchar_t> validIDbegin = alphabet+L'_';
    const List<wchar_t> validIDbody(validIDbegin+List<wchar_t>(L"0123456789"));
    Opmap ops(gen_opmap({L"+"}));
    bool is_ident(wchar_t ch,size_t i){
        return ((i==0)?validIDbegin:validIDbody).contains(ch);
    }
    const List<wchar_t> stmt_end({L'\n',L';'});
    /*
    NOTE: The following functions all have `noexcept` specified.
    */
    //returns the next identifier, or L"" if none.
    str inline expect_ident(Scanner& scn) noexcept(true){
        return scn.next(is_ident);
    }
    //returns the next closed string, and throws ParseError if none/malformed
    refExpression inline expect_string(Scanner& scn) noexcept(false){
        scn.consume_lwspcs();
        static const List<wchar_t> QUOTES
        ({L'"',L'"'});
        wint_t quote_t = scn.next(QUOTES);
        if(quote_t==WEOF){
            throw NoParse(L"String expected",scn.pos());
        }
        wchar_t quote_c = quote_t;
        wchar_t chr;
        str rslt;
        bool escaped=false;
        bool closed=false;
        while(!scn.done()){
            chr = scn();
            if(escaped){
                escaped = false;
                rslt += chr;
            }else if(chr==L'\\'){
                escaped = true;
            }else if(chr==quote_c){
                scn.advance();
                closed = true;
                break;
            }else{
                rslt += chr;
            }
            scn.advance();
        }
        if(!closed){
            throw ParseError(L"String not closed",scn.pos());
        }
        return eNew<Constant>(String->construct(Scope::null(),rslt));
    }
    //returns the next literal or identifier, and throws ParseError if none/malformed.
    refExpression inline expect_value(Scanner& scn) noexcept(false){
        scn.consume_lwspcs();
        size_t bgn = scn.pos();
        if(scn.done()){
            throw ParseError(L"Expected value",scn.pos());
        }
        if(scn()==L'"'){
            try{
                return expect_string(scn);
            }catch(NoParse&){
                //ignore the exception
            }
        }
        static const List<wchar_t> NUMBODY
        {L'-',L'0',L'1',L'2',L'3',L'4',L'5',L'6',L'7',L'8',L'9',L'.',L'e',L'E'};
        str id = expect_ident(scn);
        if(id.length())return eNew<Access>(id);
        wint_t chr;
        //dummy scope
        Scope ds;
        str num;
        bool isdcml=false;
        bool isscnt=false;
        bool invertable=true;
        while((chr = scn.next(NUMBODY))!=WEOF){
            if(chr==L'-'){
                if(!invertable){
                    break;//probably a subtraction operator
                }
                invertable = false;
            }else if(chr==L'.'){
                if(num.empty()){
                    num += L'0';
                }
                if(isdcml){
                    throw ParseError("Multiple decimal points detected in a numeric literal",scn.pos());
                }
                isdcml = true;
            }else if(chr==L'e'){
                if(num.empty()){
                    throw ParseError("A numeric literal cannot begin with a \"e\"",scn.pos());
                }
                if(num.back()==L'.'){
                    num += L'0';
                }
                if(isscnt){
                    throw ParseError("Multiple e's detected in a numeric literal",scn.pos());
                }
                isscnt = true;
                invertable = true;
            }
            num += chr;
        }
        if(!num.empty()){
            while(num.back()==L'-'){
                num.pop_back();
                scn.advance(-1);
            }
            if(num.back()==L'e'){
                throw ParseError(L"A numeric literal cannot end with an \"e\"",scn.pos());
            }
            if(num.back()==L'.'){
                num += L'0';
            }
            if(!scn.done() && (scn.next(alphabet)!=WEOF)){
                throw ParseError(L"Unsupported numeric suffix",scn.pos());
            }
            try{
                if(isdcml||isscnt){
                    return eNew<Constant>(lpfloat()->construct(ds,parse_num<default_float_t>(num)));
                }
                return eNew<Constant>(lpint()->construct(ds,parse_num<default_int_t>(num)));
            }catch(std::out_of_range& e){
                throw ParseError(L"Number is too big or too small!("+stow(std::string(e.what()))+L')',scn.pos());
            }
        }
        scn.seek(bgn);
        throw NoParse(L"Value expected",bgn);
    }
    List<refExpression> expect_commasep_expr(Scanner&,bool=false) noexcept(false);
    refExpression expect_expr(Scanner& scn) noexcept(false){
        scn.consume_lwspcs();
        refExpression curr;
        if(scn.next(L'(')){
            try{
                curr = expect_expr(scn);
            }catch(NoParse& e){
                throw ParseError(L"Expression expected ("+stow(e.what())+L")",scn.pos());
            }
            curr->make_parenthesized();
            if(!scn.next(L')')){
                throw ParseError(L"Closing parenthese expected",scn.pos());
            }
        }else{
            try{
                curr = expect_value(scn);
            }catch(NoParse& e){
                //TODO: Add support for unary prefix operators
                throw NoParse(L"Expected value for start of expression ("+stow(e.what())+L")",scn.pos());
            }
        }
        str matched;
        while(true){
            scn.consume_lwspcs();
            if(!(matched = scn.next(builtin_omp)).empty()){
                const auto& opdata = builtin_operators.get(matched);
                auto other = expect_expr(scn);
                curr = eNew<BinOp>(opdata,curr,other);
            }else if(scn.next(L'=')){
                curr = eNew<Assign>(curr,expect_expr(scn));
            }else if(scn.next(L'(')){
                auto args = expect_commasep_expr(scn);
                if(!scn.next(L')')){
                    throw ParseError("Missing ) at end of parameter list",scn.pos());
                }
                curr = eNew<FuncCall>(curr,args);
            }else{
                break;
            }
        }
        return curr;
    }
    //expects a terminated statement, and throws ParseError if none/malformed
    refStatement expect_statement(Scanner& scn) noexcept(false){
        refExpression bgn=nullptr;
        bool es=true;
        const ParseError* ex;
        try{
            bgn = expect_expr(scn);
        }catch(const NoParse& e){
            es = false;
            ex = &e;
        }
        if(es){
            if(!scn.done()&&scn.next(stmt_end)==WEOF){
                throw ParseError(L"Expected end of statement",scn.pos());
            }
            refStatement smt = sNew<ExprStmt>(bgn);
            smt->pos = scn.pos();
            return smt;
        }
        throw *ex;
       //TODO: add statements
    }
    List<refExpression> expect_commasep_expr(Scanner& scn,bool require_one/*=false(from previous declaration)*/) noexcept(false){
        scn.consume_lwspcs();
        size_t bgn = scn.pos();
        List<refExpression> lst;
        refExpression expr;
        while(true){
            try{
                expr = expect_expr(scn);
            }catch(NoParse&){
                break;
            }
            lst.append(expr);
            scn.consume_lwspcs();
            if(!scn.next(L',')){
                break;
            }
        }
        if(require_one&&lst.empty()){
            throw NoParse(L"Comma-seperated list of expressions expected",bgn);
        }
        return lst;
    }
}
#endif
