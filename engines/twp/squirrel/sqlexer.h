/*  see copyright notice in squirrel.h */
#ifndef _SQLEXER_H_
#define _SQLEXER_H_

typedef unsigned char LexChar;

struct SQLexer
{
    SQLexer();
    ~SQLexer();
    void Init(SQSharedState *ss,SQLEXREADFUNC rg,SQUserPointer up,CompilerErrorFunc efunc,void *ed);
    void Error(const SQChar *err);
    SQInteger Lex();
    const SQChar *Tok2Str(SQInteger tok);
private:
    SQInteger GetIDType(const SQChar *s,SQInteger len);
    SQInteger ReadString(SQInteger ndelim,bool verbatim);
    SQInteger ReadNumber();
    void LexBlockComment();
    void LexLineComment();
    SQInteger ReadID();
    void Next();

    SQInteger AddUTF8(SQUnsignedInteger ch);
    SQInteger ProcessStringHexEscape(SQChar *dest, SQInteger maxdigits);
    SQInteger _curtoken = 0;
    SQTable *_keywords = nullptr;
    SQBool _reached_eof = SQFalse;
public:
    SQInteger _prevtoken = 0;
    SQInteger _currentline = 0;
    SQInteger _lasttokenline = 0;
    SQInteger _currentcolumn = 0;
    const SQChar *_svalue = nullptr;
    SQInteger _nvalue = 0;
    SQFloat _fvalue = 0.f;
    SQLEXREADFUNC _readf = nullptr;
    SQUserPointer _up = nullptr;
    LexChar _currdata = 0;
    SQSharedState *_sharedstate = nullptr;
    sqvector<SQChar> _longstr;
    CompilerErrorFunc _errfunc = nullptr;
    void *_errtarget = nullptr;
};

#endif
