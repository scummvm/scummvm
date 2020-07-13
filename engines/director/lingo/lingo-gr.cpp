/* A Bison parser, made by GNU Bison 3.6.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.6.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 54 "engines/director/lingo/lingo-gr.y"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/endian.h"
#include "common/hash-str.h"
#include "common/rect.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-object.h"

extern int yylex();
extern int yyparse();
extern void lex_unput(int c);
extern bool lex_check_parens();

using namespace Director;

static void yyerror(const char *s) {
	g_lingo->_hadError = true;
	warning("######################  LINGO: %s at line %d col %d", s, g_lingo->_linenumber, g_lingo->_colnumber);
}

static void checkEnd(Common::String *token, const char *expect, bool required) {
	if (required) {
		if (token->compareToIgnoreCase(expect)) {
			Common::String err = Common::String::format("end mismatch. Expected %s but got %s", expect, token->c_str());
			yyerror(err.c_str());
		}
	}
}

static void inArgs() { g_lingo->_indefStore = g_lingo->_indef; g_lingo->_indef = kStateInArgs; }
static void inDef()  { g_lingo->_indefStore = g_lingo->_indef; g_lingo->_indef = kStateInDef; }
static void inNone() { g_lingo->_indefStore = g_lingo->_indef; g_lingo->_indef = kStateNone; }
static void inLast() { g_lingo->_indef = g_lingo->_indefStore; }

static void startDef() {
	inArgs();
	g_lingo->_methodVarsStash = g_lingo->_methodVars;
	g_lingo->_methodVars = new VarTypeHash;

	for (VarTypeHash::iterator i = g_lingo->_methodVarsStash->begin(); i != g_lingo->_methodVarsStash->end(); ++i) {
		if (i->_value == kVarGlobal || i->_value == kVarProperty)
			(*g_lingo->_methodVars)[i->_key] = i->_value;
	}
	if (g_lingo->_inFactory) {
		for (DatumHash::iterator i = g_lingo->_assemblyContext->_properties.begin(); i != g_lingo->_assemblyContext->_properties.end(); ++i) {
			(*g_lingo->_methodVars)[i->_key] = kVarInstance;
		}
	}
}

static void endDef() {
	g_lingo->clearArgStack();
	inNone();

	delete g_lingo->_methodVars;
	g_lingo->_methodVars = g_lingo->_methodVarsStash;
	g_lingo->_methodVarsStash = nullptr;
}

static void startRepeat() {
	g_lingo->_repeatStack.push_back(new RepeatBlock);
}

static void endRepeat(uint exitPos, uint nextPos) {
	RepeatBlock *block = g_lingo->_repeatStack.back();
	g_lingo->_repeatStack.pop_back();
	for (uint i = 0; i < block->exits.size(); i++) {
		uint32 pos = block->exits[i];
		inst exit = 0;
		WRITE_UINT32(&exit, exitPos - (pos - 1));
		(*g_lingo->_currentAssembly)[pos] = exit;
	}
	for (uint i = 0; i < block->nexts.size(); i++) {
		uint32 pos = block->nexts[i];
		inst next = 0;
		WRITE_UINT32(&next, nextPos - (pos - 1));
		(*g_lingo->_currentAssembly)[pos] = next;
	}
	delete block;
}

static VarType globalCheck() {
	// If in a definition, assume variables are local unless
	// they were declared global with `global varname`
	if (g_lingo->_indef == kStateInDef) {
		return kVarLocal;
	}
	return kVarGlobal;
}

static void mVar(Common::String *s, VarType type) {
	if (!g_lingo->_methodVars->contains(*s)) {
		(*g_lingo->_methodVars)[*s] = type;
		if (type == kVarProperty || type == kVarInstance) {
			g_lingo->_assemblyContext->_properties[*s] = Datum();
		} else if (type == kVarGlobal) {
			g_lingo->varCreate(*s, true);
		}
	}
}


#line 179 "engines/director/lingo/lingo-gr.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED
# define YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    UNARY = 258,                   /* UNARY  */
    CASTREF = 259,                 /* CASTREF  */
    VOID = 260,                    /* VOID  */
    VAR = 261,                     /* VAR  */
    POINT = 262,                   /* POINT  */
    RECT = 263,                    /* RECT  */
    ARRAY = 264,                   /* ARRAY  */
    OBJECT = 265,                  /* OBJECT  */
    FIELDREF = 266,                /* FIELDREF  */
    LEXERROR = 267,                /* LEXERROR  */
    PARRAY = 268,                  /* PARRAY  */
    INT = 269,                     /* INT  */
    ARGC = 270,                    /* ARGC  */
    ARGCNORET = 271,               /* ARGCNORET  */
    THEENTITY = 272,               /* THEENTITY  */
    THEENTITYWITHID = 273,         /* THEENTITYWITHID  */
    THEMENUITEMENTITY = 274,       /* THEMENUITEMENTITY  */
    THEMENUITEMSENTITY = 275,      /* THEMENUITEMSENTITY  */
    FLOAT = 276,                   /* FLOAT  */
    BLTIN = 277,                   /* BLTIN  */
    FBLTIN = 278,                  /* FBLTIN  */
    RBLTIN = 279,                  /* RBLTIN  */
    THEFBLTIN = 280,               /* THEFBLTIN  */
    ID = 281,                      /* ID  */
    STRING = 282,                  /* STRING  */
    HANDLER = 283,                 /* HANDLER  */
    SYMBOL = 284,                  /* SYMBOL  */
    ENDCLAUSE = 285,               /* ENDCLAUSE  */
    tPLAYACCEL = 286,              /* tPLAYACCEL  */
    tMETHOD = 287,                 /* tMETHOD  */
    THEOBJECTPROP = 288,           /* THEOBJECTPROP  */
    tDOWN = 289,                   /* tDOWN  */
    tELSE = 290,                   /* tELSE  */
    tELSIF = 291,                  /* tELSIF  */
    tEXIT = 292,                   /* tEXIT  */
    tGLOBAL = 293,                 /* tGLOBAL  */
    tGO = 294,                     /* tGO  */
    tGOLOOP = 295,                 /* tGOLOOP  */
    tIF = 296,                     /* tIF  */
    tIN = 297,                     /* tIN  */
    tINTO = 298,                   /* tINTO  */
    tMACRO = 299,                  /* tMACRO  */
    tMOVIE = 300,                  /* tMOVIE  */
    tNEXT = 301,                   /* tNEXT  */
    tOF = 302,                     /* tOF  */
    tPREVIOUS = 303,               /* tPREVIOUS  */
    tPUT = 304,                    /* tPUT  */
    tREPEAT = 305,                 /* tREPEAT  */
    tSET = 306,                    /* tSET  */
    tTHEN = 307,                   /* tTHEN  */
    tTO = 308,                     /* tTO  */
    tWHEN = 309,                   /* tWHEN  */
    tWITH = 310,                   /* tWITH  */
    tWHILE = 311,                  /* tWHILE  */
    tFACTORY = 312,                /* tFACTORY  */
    tOPEN = 313,                   /* tOPEN  */
    tPLAY = 314,                   /* tPLAY  */
    tINSTANCE = 315,               /* tINSTANCE  */
    tGE = 316,                     /* tGE  */
    tLE = 317,                     /* tLE  */
    tEQ = 318,                     /* tEQ  */
    tNEQ = 319,                    /* tNEQ  */
    tAND = 320,                    /* tAND  */
    tOR = 321,                     /* tOR  */
    tNOT = 322,                    /* tNOT  */
    tMOD = 323,                    /* tMOD  */
    tAFTER = 324,                  /* tAFTER  */
    tBEFORE = 325,                 /* tBEFORE  */
    tCONCAT = 326,                 /* tCONCAT  */
    tCONTAINS = 327,               /* tCONTAINS  */
    tSTARTS = 328,                 /* tSTARTS  */
    tCHAR = 329,                   /* tCHAR  */
    tITEM = 330,                   /* tITEM  */
    tLINE = 331,                   /* tLINE  */
    tWORD = 332,                   /* tWORD  */
    tSPRITE = 333,                 /* tSPRITE  */
    tINTERSECTS = 334,             /* tINTERSECTS  */
    tWITHIN = 335,                 /* tWITHIN  */
    tTELL = 336,                   /* tTELL  */
    tPROPERTY = 337,               /* tPROPERTY  */
    tON = 338,                     /* tON  */
    tENDIF = 339,                  /* tENDIF  */
    tENDREPEAT = 340,              /* tENDREPEAT  */
    tENDTELL = 341                 /* tENDTELL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 162 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Director::DatumArray *arr;

	struct {
		Common::String *obj;
		Common::String *prop;
	} objectprop;

#line 330 "engines/director/lingo/lingo-gr.cpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_UNARY = 3,                      /* UNARY  */
  YYSYMBOL_CASTREF = 4,                    /* CASTREF  */
  YYSYMBOL_VOID = 5,                       /* VOID  */
  YYSYMBOL_VAR = 6,                        /* VAR  */
  YYSYMBOL_POINT = 7,                      /* POINT  */
  YYSYMBOL_RECT = 8,                       /* RECT  */
  YYSYMBOL_ARRAY = 9,                      /* ARRAY  */
  YYSYMBOL_OBJECT = 10,                    /* OBJECT  */
  YYSYMBOL_FIELDREF = 11,                  /* FIELDREF  */
  YYSYMBOL_LEXERROR = 12,                  /* LEXERROR  */
  YYSYMBOL_PARRAY = 13,                    /* PARRAY  */
  YYSYMBOL_INT = 14,                       /* INT  */
  YYSYMBOL_ARGC = 15,                      /* ARGC  */
  YYSYMBOL_ARGCNORET = 16,                 /* ARGCNORET  */
  YYSYMBOL_THEENTITY = 17,                 /* THEENTITY  */
  YYSYMBOL_THEENTITYWITHID = 18,           /* THEENTITYWITHID  */
  YYSYMBOL_THEMENUITEMENTITY = 19,         /* THEMENUITEMENTITY  */
  YYSYMBOL_THEMENUITEMSENTITY = 20,        /* THEMENUITEMSENTITY  */
  YYSYMBOL_FLOAT = 21,                     /* FLOAT  */
  YYSYMBOL_BLTIN = 22,                     /* BLTIN  */
  YYSYMBOL_FBLTIN = 23,                    /* FBLTIN  */
  YYSYMBOL_RBLTIN = 24,                    /* RBLTIN  */
  YYSYMBOL_THEFBLTIN = 25,                 /* THEFBLTIN  */
  YYSYMBOL_ID = 26,                        /* ID  */
  YYSYMBOL_STRING = 27,                    /* STRING  */
  YYSYMBOL_HANDLER = 28,                   /* HANDLER  */
  YYSYMBOL_SYMBOL = 29,                    /* SYMBOL  */
  YYSYMBOL_ENDCLAUSE = 30,                 /* ENDCLAUSE  */
  YYSYMBOL_tPLAYACCEL = 31,                /* tPLAYACCEL  */
  YYSYMBOL_tMETHOD = 32,                   /* tMETHOD  */
  YYSYMBOL_THEOBJECTPROP = 33,             /* THEOBJECTPROP  */
  YYSYMBOL_tDOWN = 34,                     /* tDOWN  */
  YYSYMBOL_tELSE = 35,                     /* tELSE  */
  YYSYMBOL_tELSIF = 36,                    /* tELSIF  */
  YYSYMBOL_tEXIT = 37,                     /* tEXIT  */
  YYSYMBOL_tGLOBAL = 38,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 39,                       /* tGO  */
  YYSYMBOL_tGOLOOP = 40,                   /* tGOLOOP  */
  YYSYMBOL_tIF = 41,                       /* tIF  */
  YYSYMBOL_tIN = 42,                       /* tIN  */
  YYSYMBOL_tINTO = 43,                     /* tINTO  */
  YYSYMBOL_tMACRO = 44,                    /* tMACRO  */
  YYSYMBOL_tMOVIE = 45,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 46,                     /* tNEXT  */
  YYSYMBOL_tOF = 47,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 48,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 49,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 50,                   /* tREPEAT  */
  YYSYMBOL_tSET = 51,                      /* tSET  */
  YYSYMBOL_tTHEN = 52,                     /* tTHEN  */
  YYSYMBOL_tTO = 53,                       /* tTO  */
  YYSYMBOL_tWHEN = 54,                     /* tWHEN  */
  YYSYMBOL_tWITH = 55,                     /* tWITH  */
  YYSYMBOL_tWHILE = 56,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 57,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 58,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 59,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 60,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 61,                       /* tGE  */
  YYSYMBOL_tLE = 62,                       /* tLE  */
  YYSYMBOL_tEQ = 63,                       /* tEQ  */
  YYSYMBOL_tNEQ = 64,                      /* tNEQ  */
  YYSYMBOL_tAND = 65,                      /* tAND  */
  YYSYMBOL_tOR = 66,                       /* tOR  */
  YYSYMBOL_tNOT = 67,                      /* tNOT  */
  YYSYMBOL_tMOD = 68,                      /* tMOD  */
  YYSYMBOL_tAFTER = 69,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 70,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 71,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 72,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 73,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 74,                     /* tCHAR  */
  YYSYMBOL_tITEM = 75,                     /* tITEM  */
  YYSYMBOL_tLINE = 76,                     /* tLINE  */
  YYSYMBOL_tWORD = 77,                     /* tWORD  */
  YYSYMBOL_tSPRITE = 78,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 79,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 80,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 81,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 82,                 /* tPROPERTY  */
  YYSYMBOL_tON = 83,                       /* tON  */
  YYSYMBOL_tENDIF = 84,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 85,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 86,                  /* tENDTELL  */
  YYSYMBOL_87_ = 87,                       /* '<'  */
  YYSYMBOL_88_ = 88,                       /* '>'  */
  YYSYMBOL_89_ = 89,                       /* '&'  */
  YYSYMBOL_90_ = 90,                       /* '+'  */
  YYSYMBOL_91_ = 91,                       /* '-'  */
  YYSYMBOL_92_ = 92,                       /* '*'  */
  YYSYMBOL_93_ = 93,                       /* '/'  */
  YYSYMBOL_94_ = 94,                       /* '%'  */
  YYSYMBOL_95_n_ = 95,                     /* '\n'  */
  YYSYMBOL_96_ = 96,                       /* '('  */
  YYSYMBOL_97_ = 97,                       /* ')'  */
  YYSYMBOL_98_ = 98,                       /* ','  */
  YYSYMBOL_99_ = 99,                       /* '['  */
  YYSYMBOL_100_ = 100,                     /* ']'  */
  YYSYMBOL_101_ = 101,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 102,                 /* $accept  */
  YYSYMBOL_program = 103,                  /* program  */
  YYSYMBOL_programline = 104,              /* programline  */
  YYSYMBOL_asgn = 105,                     /* asgn  */
  YYSYMBOL_stmtoneliner = 106,             /* stmtoneliner  */
  YYSYMBOL_stmt = 107,                     /* stmt  */
  YYSYMBOL_108_1 = 108,                    /* $@1  */
  YYSYMBOL_109_2 = 109,                    /* $@2  */
  YYSYMBOL_110_3 = 110,                    /* $@3  */
  YYSYMBOL_111_4 = 111,                    /* $@4  */
  YYSYMBOL_112_5 = 112,                    /* $@5  */
  YYSYMBOL_113_6 = 113,                    /* $@6  */
  YYSYMBOL_114_7 = 114,                    /* $@7  */
  YYSYMBOL_115_8 = 115,                    /* $@8  */
  YYSYMBOL_116_9 = 116,                    /* $@9  */
  YYSYMBOL_startrepeat = 117,              /* startrepeat  */
  YYSYMBOL_tellstart = 118,                /* tellstart  */
  YYSYMBOL_ifstmt = 119,                   /* ifstmt  */
  YYSYMBOL_elseifstmtlist = 120,           /* elseifstmtlist  */
  YYSYMBOL_elseifstmt = 121,               /* elseifstmt  */
  YYSYMBOL_jumpifz = 122,                  /* jumpifz  */
  YYSYMBOL_jump = 123,                     /* jump  */
  YYSYMBOL_varassign = 124,                /* varassign  */
  YYSYMBOL_if = 125,                       /* if  */
  YYSYMBOL_lbl = 126,                      /* lbl  */
  YYSYMBOL_stmtlist = 127,                 /* stmtlist  */
  YYSYMBOL_simpleexpr = 128,               /* simpleexpr  */
  YYSYMBOL_expr = 129,                     /* expr  */
  YYSYMBOL_130_10 = 130,                   /* $@10  */
  YYSYMBOL_chunkexpr = 131,                /* chunkexpr  */
  YYSYMBOL_reference = 132,                /* reference  */
  YYSYMBOL_proc = 133,                     /* proc  */
  YYSYMBOL_134_11 = 134,                   /* $@11  */
  YYSYMBOL_135_12 = 135,                   /* $@12  */
  YYSYMBOL_136_13 = 136,                   /* $@13  */
  YYSYMBOL_globallist = 137,               /* globallist  */
  YYSYMBOL_propertylist = 138,             /* propertylist  */
  YYSYMBOL_instancelist = 139,             /* instancelist  */
  YYSYMBOL_gotofunc = 140,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 141,                /* gotomovie  */
  YYSYMBOL_playfunc = 142,                 /* playfunc  */
  YYSYMBOL_143_14 = 143,                   /* $@14  */
  YYSYMBOL_defn = 144,                     /* defn  */
  YYSYMBOL_145_15 = 145,                   /* $@15  */
  YYSYMBOL_146_16 = 146,                   /* $@16  */
  YYSYMBOL_on = 147,                       /* on  */
  YYSYMBOL_148_17 = 148,                   /* $@17  */
  YYSYMBOL_argdef = 149,                   /* argdef  */
  YYSYMBOL_endargdef = 150,                /* endargdef  */
  YYSYMBOL_argstore = 151,                 /* argstore  */
  YYSYMBOL_macro = 152,                    /* macro  */
  YYSYMBOL_arglist = 153,                  /* arglist  */
  YYSYMBOL_nonemptyarglist = 154,          /* nonemptyarglist  */
  YYSYMBOL_list = 155,                     /* list  */
  YYSYMBOL_valuelist = 156,                /* valuelist  */
  YYSYMBOL_linearlist = 157,               /* linearlist  */
  YYSYMBOL_proplist = 158,                 /* proplist  */
  YYSYMBOL_proppair = 159                  /* proppair  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  116
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2623

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  102
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  58
/* YYNRULES -- Number of rules.  */
#define YYNRULES  182
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  385

#define YYMAXUTOK   341


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      95,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    94,    89,     2,
      96,    97,    92,    90,    98,    91,     2,    93,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   101,     2,
      87,     2,    88,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    99,     2,   100,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   214,   214,   215,   217,   218,   219,   221,   228,   232,
     243,   250,   253,   260,   263,   270,   277,   284,   291,   297,
     304,   315,   326,   333,   341,   342,   343,   345,   346,   351,
     364,   368,   371,   363,   398,   402,   405,   397,   432,   439,
     445,   431,   475,   483,   486,   487,   489,   491,   493,   500,
     508,   509,   511,   517,   521,   525,   529,   532,   534,   535,
     536,   538,   541,   544,   548,   552,   556,   564,   570,   571,
     572,   583,   584,   587,   590,   593,   596,   602,   602,   607,
     610,   613,   619,   620,   621,   622,   623,   624,   625,   626,
     627,   628,   629,   630,   631,   632,   633,   634,   635,   636,
     637,   638,   639,   640,   641,   643,   644,   645,   646,   647,
     648,   649,   650,   652,   655,   657,   658,   659,   660,   668,
     669,   669,   670,   670,   671,   671,   672,   675,   678,   681,
     685,   688,   692,   695,   699,   702,   713,   714,   715,   716,
     720,   724,   729,   730,   732,   736,   740,   744,   744,   774,
     774,   780,   781,   781,   787,   795,   801,   801,   803,   804,
     805,   807,   808,   809,   811,   813,   821,   822,   823,   825,
     826,   828,   830,   831,   832,   833,   835,   836,   838,   839,
     841,   845,   849
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  static const char *const yy_sname[] =
  {
  "end of file", "error", "invalid token", "UNARY", "CASTREF", "VOID",
  "VAR", "POINT", "RECT", "ARRAY", "OBJECT", "FIELDREF", "LEXERROR",
  "PARRAY", "INT", "ARGC", "ARGCNORET", "THEENTITY", "THEENTITYWITHID",
  "THEMENUITEMENTITY", "THEMENUITEMSENTITY", "FLOAT", "BLTIN", "FBLTIN",
  "RBLTIN", "THEFBLTIN", "ID", "STRING", "HANDLER", "SYMBOL", "ENDCLAUSE",
  "tPLAYACCEL", "tMETHOD", "THEOBJECTPROP", "tDOWN", "tELSE", "tELSIF",
  "tEXIT", "tGLOBAL", "tGO", "tGOLOOP", "tIF", "tIN", "tINTO", "tMACRO",
  "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET",
  "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tFACTORY", "tOPEN", "tPLAY",
  "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT", "tMOD",
  "tAFTER", "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR", "tITEM",
  "tLINE", "tWORD", "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL",
  "tPROPERTY", "tON", "tENDIF", "tENDREPEAT", "tENDTELL", "'<'", "'>'",
  "'&'", "'+'", "'-'", "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "','",
  "'['", "']'", "':'", "$accept", "program", "programline", "asgn",
  "stmtoneliner", "stmt", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7",
  "$@8", "$@9", "startrepeat", "tellstart", "ifstmt", "elseifstmtlist",
  "elseifstmt", "jumpifz", "jump", "varassign", "if", "lbl", "stmtlist",
  "simpleexpr", "expr", "$@10", "chunkexpr", "reference", "proc", "$@11",
  "$@12", "$@13", "globallist", "propertylist", "instancelist", "gotofunc",
  "gotomovie", "playfunc", "$@14", "defn", "$@15", "$@16", "on", "$@17",
  "argdef", "endargdef", "argstore", "macro", "arglist", "nonemptyarglist",
  "list", "valuelist", "linearlist", "proplist", "proppair", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,    60,    62,    38,
      43,    45,    42,    47,    37,    10,    40,    41,    44,    91,
      93,    58
};
#endif

#define YYPACT_NINF (-319)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-173)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     894,   -50,  -319,  -319,    55,  -319,   720,   341,   442,   -11,
     623,  -319,  -319,  -319,  -319,  -319,    18,  -319,  1890,  -319,
    -319,  -319,    24,  2074,   -17,   190,    73,    79,  2110,  1926,
    -319,  2110,  2110,  2110,  -319,  -319,  2110,  2110,  2110,   535,
     107,    17,  -319,  -319,  -319,  -319,  2110,  -319,  2517,  -319,
    -319,  -319,  -319,  -319,  -319,  -319,  -319,  -319,  -319,    37,
    2074,  1982,  2517,    45,  1982,    45,  1982,    45,    55,  2018,
    2517,    47,   807,  -319,  -319,   113,  2110,  -319,   108,  -319,
    2131,  -319,   132,  -319,   142,   508,   145,  -319,   -47,    55,
      55,   -42,   -12,   124,  -319,  2434,  2131,  -319,   156,  -319,
    2484,  1792,   162,   170,  -319,  -319,  2447,   -37,    96,    97,
    -319,  2517,   100,   114,   115,  -319,  -319,   894,  2517,  2110,
    2110,  2110,  2110,  2110,  2110,  2110,  2110,  2110,  2110,  2110,
    2110,  2110,  2110,  2110,  2110,  2110,   180,  2018,   508,  2447,
     -45,  2110,    13,    40,  -319,   -78,    69,  2110,    45,   180,
    -319,   116,  2517,  2110,  -319,  -319,    55,    11,   110,   118,
     -29,  2110,  2110,  2110,    38,   163,  2110,  2110,  2110,  2110,
    2110,  2110,  -319,  -319,   117,  2110,  2110,  -319,  -319,  -319,
     119,  -319,  -319,  2110,  2110,  2110,  -319,  2110,   123,  -319,
     159,   112,   112,   112,   112,  2530,  2530,  -319,   -44,   112,
     112,   112,   112,   -44,    -3,    -3,  -319,  -319,  -319,   -15,
    -319,  2517,  -319,  -319,  -319,  -319,  -319,  2517,     0,   192,
    2517,   180,   184,    55,  -319,  2110,  2110,  2110,  2110,  -319,
    -319,  -319,  -319,  -319,  -319,  2110,  2110,  2517,  2517,   112,
    2110,  2110,   205,  2517,   112,  2517,   112,  2517,  2517,   206,
    2517,  2517,  1810,  -319,   207,  2517,  2517,  2517,  2517,   135,
      96,    97,  -319,  -319,  -319,   211,  2110,  -319,  -319,     8,
    2110,  -319,  2164,  2197,  2230,  2263,  2517,   244,  -319,  2517,
     112,    55,  -319,  -319,  1311,  -319,  1227,  -319,  -319,    76,
    -319,  -319,  2517,  2110,  2110,  2110,  2110,  2110,  2110,  2110,
    2110,  -319,  -319,  -319,  -319,    41,  -319,  -319,  -319,   152,
    -319,   978,  -319,  1061,  -319,  2517,  2296,  2517,  2329,  2517,
    2362,  2517,  2395,  -319,  -319,  -319,  1394,  2110,  2110,  -319,
      43,   213,  1144,  2110,  2110,  2110,  2110,  -319,   187,   209,
     160,  2517,   112,  -319,  2110,  -319,   157,  -319,   146,  2517,
    2517,  2517,  2517,  -319,  2110,   193,  -319,  1477,  2517,  -319,
     221,  -319,  2517,  2110,   164,   197,  -319,  -319,  -319,  2517,
    -319,  -319,  1561,  -319,  -319,  1227,  -319,  -319,  -319,  -319,
    1644,  -319,  -319,  1727,  -319
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    61,    66,     0,    62,     0,     0,     0,     0,
       0,    64,    63,   147,   152,    81,   119,   120,     0,   136,
      56,   149,     0,     0,     0,     0,     0,     0,     0,     0,
     124,     0,     0,     0,   122,   156,     0,     0,     0,     0,
       0,     2,    82,    27,     6,    28,     0,    71,    25,    26,
     116,   117,     5,    57,    24,    69,    70,    65,    67,    65,
       0,     0,   167,   127,     0,    73,     0,    75,     0,     0,
     169,   165,     0,    57,   118,     0,     0,   137,     0,   138,
     139,   141,     0,    42,     0,   115,     0,    57,     0,     0,
       0,     0,     0,     0,   151,   129,   144,   146,     0,    96,
       0,     0,     0,     0,   101,   102,     0,    65,    64,    63,
     173,   176,     0,   175,   174,   178,     1,     0,    53,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   158,     0,     0,   167,
       0,     0,     0,     0,    80,    65,     0,     0,   148,   158,
     130,   121,   143,     0,   140,    57,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   145,   134,   125,     0,     0,    47,    47,   132,
     123,   157,    68,     0,     0,     0,   171,     0,     0,     3,
       0,    92,    93,    90,    91,    94,    95,    87,    98,    99,
     100,    89,    88,    97,    83,    84,    85,    86,   159,     0,
     126,   168,    72,    74,    76,    77,    79,   170,     0,     0,
     142,   158,     0,     0,     7,     0,     0,     0,     0,   114,
       8,    10,    11,    12,    13,     0,     0,    53,    17,    15,
       0,     0,     0,    16,    14,    22,    23,    43,   128,     0,
     103,   104,     0,    58,     0,   182,   181,   180,   177,     0,
       0,     0,   179,    58,   164,     0,     0,   164,   131,     0,
       0,   113,     0,     0,     0,     0,    38,    30,    46,    18,
      19,     0,   135,    57,     0,   133,     0,    58,   160,     0,
      58,   164,     9,     0,     0,     0,     0,     0,     0,     0,
       0,    57,    55,    55,    58,     0,    45,    59,    60,     0,
      50,     0,    78,     0,    58,   105,     0,   107,     0,   109,
       0,   111,     0,    39,    31,    35,     0,     0,     0,    44,
      57,   161,     0,     0,     0,     0,     0,    53,     0,     0,
       0,    20,    21,    58,     0,    51,     0,   162,   154,   106,
     108,   110,   112,    40,     0,     0,    29,     0,    53,    48,
       0,    46,    32,     0,     0,     0,   163,    58,    53,    36,
      49,    58,     0,    46,    53,     0,    41,    58,    46,    52,
       0,    58,    33,     0,    37
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -319,   133,  -319,  -319,    -1,    15,  -319,  -319,  -319,  -319,
    -319,  -319,  -319,  -319,  -319,  -311,    75,  -319,  -319,  -319,
    -228,  -318,   -49,  -319,   -59,  -115,     3,    -6,  -319,  -319,
      32,  -319,  -319,  -319,  -319,  -319,  -319,  -319,  -319,   -19,
    -319,  -319,  -319,  -319,  -319,  -319,  -319,  -146,  -319,  -262,
    -319,    36,    -9,  -319,  -319,  -319,  -319,    67
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    40,    41,    42,    43,   308,   302,   338,   368,   303,
     339,   374,   301,   337,   361,   304,   252,    45,   330,   345,
     190,   310,   324,    46,   136,   284,    47,    48,   266,   229,
     230,    49,    75,   102,    98,   151,   180,   174,    50,    81,
      51,    72,    52,    82,    73,    53,   103,   209,   348,   287,
      54,   146,    71,    55,   112,   113,   114,   115
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      62,    62,    62,   218,    70,   290,   162,    58,   340,   278,
      97,   166,    80,   235,   149,    44,   163,    85,   137,   214,
     215,   167,    95,    96,   125,    99,   100,   101,   161,   314,
     104,   105,   106,   111,   236,   223,    68,   224,    86,    87,
     118,   168,    63,    65,    67,    56,   132,   133,   134,   135,
     367,   169,   210,   141,   138,   139,     1,   379,   139,   137,
     139,   154,   377,   139,   183,   125,    62,   381,    74,     2,
     152,   144,     3,     4,    83,   269,     5,   172,   343,   344,
     264,    57,    11,   265,    12,   225,   226,   227,   228,   134,
     135,   240,   164,   165,   327,   267,   221,   140,   265,    93,
     142,   241,   143,   291,   328,    94,   265,   116,   148,   353,
     212,   141,   117,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     365,    62,    44,   137,   223,   211,   231,   213,   141,   150,
     373,   217,   223,   141,   233,   147,   378,   220,   286,   259,
     260,    38,   261,   153,    39,   237,   238,   239,   155,   222,
     243,   244,   245,   246,   247,   248,   216,   141,   156,   250,
     251,   160,   311,   312,   147,   313,   170,   255,   256,   257,
     125,   258,   173,   126,   225,   226,   227,   228,   179,   326,
     232,   234,   225,   226,   227,   228,   181,   184,   185,   332,
     186,   131,   132,   133,   134,   135,   208,    88,    89,    90,
     242,   263,   187,   188,   219,   249,    91,   254,   268,   272,
     273,   274,   275,    92,   306,   309,   271,   270,   357,   276,
     277,   281,   282,   285,   279,   280,   183,   288,   329,   347,
     354,   359,   323,   355,   360,   356,   363,   366,   370,   371,
     189,   283,   372,   253,   325,   262,   375,   289,     0,     0,
      70,     0,   380,     0,   292,     0,   383,     0,     0,     0,
       0,   346,     0,     0,     0,     0,     0,     0,   -34,     0,
       0,     0,     0,     0,   305,     0,     0,   315,   316,   317,
     318,   319,   320,   321,   322,     0,     0,     0,   364,     0,
       0,     0,     0,     0,     0,   119,   120,   121,   122,   123,
     124,     0,   125,     0,     0,   126,   127,   128,     0,     0,
       0,   341,   342,     0,     0,     0,     0,   349,   350,   351,
     352,   129,   130,   131,   132,   133,   134,   135,   358,     0,
       0,  -166,     1,     0,     0,     0,     0,     0,   362,     0,
       0,     0,     0,     0,     0,     2,     0,   369,     3,     4,
       0,     0,     5,  -166,     7,     8,     9,    59,    11,     0,
      12,  -166,  -166,     0,    15,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,     0,  -166,     0,  -166,  -166,  -166,     0,
      60,  -166,    25,  -166,  -166,  -166,  -166,     0,     0,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,    31,  -166,
    -166,  -166,  -166,  -166,  -166,     0,     0,     0,     0,    32,
    -166,  -166,  -166,  -166,     0,  -166,  -166,  -166,  -166,  -166,
    -166,    36,    37,  -166,  -166,     0,  -166,    64,  -166,  -166,
      39,  -166,  -166,     1,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     2,     0,     0,     3,
       4,     0,     0,     5,  -166,     7,     8,     9,    59,    11,
       0,    12,  -166,  -166,     0,    15,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,     0,  -166,     0,  -166,  -166,  -166,
       0,    60,  -166,    25,  -166,  -166,  -166,  -166,     0,     0,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,    31,
    -166,  -166,  -166,  -166,  -166,  -166,     0,     0,     0,     0,
      32,  -166,  -166,  -166,  -166,     0,  -166,  -166,  -166,  -166,
    -166,  -166,    36,    37,  -166,  -166,     1,  -166,    66,  -166,
    -166,    39,  -166,     0,     0,     0,     0,     0,     0,     2,
       0,   157,     3,     4,     0,     0,     5,     0,     7,     8,
       9,   107,   108,     0,   109,     0,     0,     0,    15,   119,
     120,   121,   122,   123,   124,     0,   125,   158,   159,   126,
     127,   128,     0,     0,    60,     0,    25,     0,     0,     0,
       0,     0,     0,     0,     0,   129,   130,   131,   132,   133,
     134,   135,    31,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    32,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   -65,     1,    36,    37,     0,     0,     0,
       0,    38,     0,     0,    39,  -172,   110,     2,     0,     0,
       3,     4,     0,     0,     5,   -65,     7,     8,     9,    59,
      11,     0,    12,   -65,   -65,     0,    15,     0,   -65,   -65,
     -65,   -65,   -65,   -65,   -65,     0,     0,     0,     0,   -65,
       0,     0,    60,   -65,    25,     0,     0,   -65,     0,     0,
       0,   -65,   -65,   -65,   -65,   -65,   -65,   -65,   -65,   -65,
      31,   -65,     0,     0,   -65,   -65,   -65,     0,     0,     0,
       0,    32,     0,     0,   -65,   -65,     0,   -65,   -65,   -65,
     -65,   -65,   -65,    36,    37,   -65,   -65,     0,   -65,    69,
    -166,     1,    39,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     2,     0,     0,     3,     4,     0,
       0,     5,  -166,     7,     8,     9,    59,    11,     0,    12,
    -166,  -166,     0,    15,     0,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,     0,     0,     0,     0,  -166,     0,     0,    60,
    -166,    25,     0,     0,  -166,     0,     0,     0,  -166,  -166,
    -166,     0,     0,     0,     0,     0,     0,    31,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    32,     0,
       0,  -166,  -166,     0,  -166,  -166,  -166,  -166,     1,     0,
      36,    37,     0,     0,     0,  -166,    61,     0,  -166,    39,
       0,     2,     0,     0,     3,     4,     0,     0,     5,  -166,
       7,     8,     9,    59,    11,     0,    12,  -166,  -166,     0,
      15,     0,  -166,  -166,  -166,  -166,  -166,  -166,  -166,     0,
       0,     0,     0,  -166,     0,     0,    60,  -166,    25,     0,
       0,  -166,     0,     0,     0,  -166,  -166,  -166,     0,     0,
       0,     0,     0,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    32,     0,     0,  -166,  -166,
       0,  -166,  -166,  -166,    -4,     1,     0,    36,    37,     0,
       0,     0,  -166,    38,     0,  -166,    39,     0,     2,     0,
       0,     3,     4,     0,     0,     5,     6,     7,     8,     9,
      10,    11,     0,    12,     0,    13,    14,    15,     0,     0,
       0,    16,    17,    18,    19,    20,     0,     0,    21,     0,
      22,     0,     0,    23,    24,    25,     0,     0,    26,     0,
       0,    27,    28,    29,    30,     0,     0,     0,     0,     0,
       0,    31,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    32,     0,     0,    33,    34,    35,  -155,     1,
       0,     0,     0,     0,    36,    37,     0,     0,     0,    -4,
      38,     0,     2,    39,     0,     3,     4,     0,     0,     5,
       6,     7,     8,     9,    10,    11,     0,    12,   331,    13,
       0,    15,     0,     0,     0,    16,    17,    18,    19,    20,
       0,     0,     0,     0,    22,     0,     0,    23,    24,    25,
       0,     0,    26,     0,     0,     0,    28,    29,    30,     0,
       0,     0,     0,     0,     0,    31,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    32,     0,     0,    33,
      34,  -153,     1,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,   307,    38,     2,     0,    39,     3,     4,
       0,     0,     5,     6,     7,     8,     9,    10,    11,     0,
      12,     0,    13,     0,    15,     0,     0,     0,    16,    17,
      18,    19,    20,     0,     0,     0,     0,    22,     0,     0,
      23,    24,    25,     0,     0,    26,     0,     0,     0,    28,
      29,    30,     0,     0,     0,     0,     0,     0,    31,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    32,
       0,     0,    33,    34,  -150,     1,     0,     0,     0,     0,
       0,    36,    37,     0,     0,     0,   307,    38,     2,     0,
      39,     3,     4,     0,     0,     5,     6,     7,     8,     9,
      10,    11,     0,    12,     0,    13,     0,    15,     0,     0,
       0,    16,    17,    18,    19,    20,     0,     0,     0,     0,
      22,     0,     0,    23,    24,    25,     0,     0,    26,     0,
       0,     0,    28,    29,    30,     0,     0,     0,     0,     0,
       0,    31,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    32,     0,     0,    33,    34,     0,     1,     0,
       0,     0,     0,     0,    36,    37,     0,     0,     0,   307,
      38,     2,     0,    39,     3,     4,     0,     0,     5,     6,
       7,     8,     9,    10,    11,     0,    12,     0,    13,     0,
      15,     0,   -54,   -54,    16,    17,    18,    19,    20,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    25,     0,
       0,    26,     0,     0,     0,    28,    29,    30,     0,     0,
       0,     0,     0,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    32,     0,     0,    33,    34,
       0,   -54,     1,     0,     0,     0,     0,    36,    37,     0,
       0,     0,   307,    38,     0,     2,    39,     0,     3,     4,
       0,     0,     5,     6,     7,     8,     9,    10,    11,     0,
      12,     0,    13,     0,    15,     0,     0,     0,    16,    17,
      18,    19,    20,     0,     0,     0,     0,    22,     0,     0,
      23,    24,    25,     0,     0,    26,     0,     0,     0,    28,
      29,    30,     0,     0,     0,     0,     0,     0,    31,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    32,
       0,     0,    33,    34,     0,     1,     0,   -57,     0,     0,
       0,    36,    37,     0,     0,     0,   307,    38,     2,     0,
      39,     3,     4,     0,     0,     5,     6,     7,     8,     9,
      10,    11,     0,    12,     0,    13,     0,    15,     0,     0,
       0,    16,    17,    18,    19,    20,     0,     0,     0,     0,
      22,     0,     0,    23,    24,    25,     0,     0,    26,     0,
       0,     0,    28,    29,    30,     0,     0,     0,     0,     0,
       0,    31,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    32,     0,     0,    33,    34,     0,     1,   -54,
       0,     0,     0,     0,    36,    37,     0,     0,     0,   307,
      38,     2,     0,    39,     3,     4,     0,     0,     5,     6,
       7,     8,     9,    10,    11,     0,    12,     0,    13,     0,
      15,     0,     0,     0,    16,    17,    18,    19,    20,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    25,     0,
       0,    26,     0,     0,     0,    28,    29,    30,     0,     0,
       0,     0,     0,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    32,     0,     0,    33,    34,
       0,   -57,     1,     0,     0,     0,     0,    36,    37,     0,
       0,     0,   307,    38,     0,     2,    39,     0,     3,     4,
       0,     0,     5,     6,     7,     8,     9,    10,    11,     0,
      12,     0,    13,     0,    15,     0,     0,     0,    16,    17,
      18,    19,    20,     0,     0,     0,     0,    22,     0,     0,
      23,    24,    25,     0,     0,    26,     0,     0,     0,    28,
      29,    30,     0,     0,     0,     0,     0,     0,    31,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    32,
       0,     0,    33,    34,     0,     1,   376,     0,     0,     0,
       0,    36,    37,     0,     0,     0,   307,    38,     2,     0,
      39,     3,     4,     0,     0,     5,     6,     7,     8,     9,
      10,    11,     0,    12,     0,    13,     0,    15,     0,     0,
       0,    16,    17,    18,    19,    20,     0,     0,     0,     0,
      22,     0,     0,    23,    24,    25,     0,     0,    26,     0,
       0,     0,    28,    29,    30,     0,     0,     0,     0,     0,
       0,    31,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    32,     0,     0,    33,    34,     0,     1,   382,
       0,     0,     0,     0,    36,    37,     0,     0,     0,   307,
      38,     2,     0,    39,     3,     4,     0,     0,     5,     6,
       7,     8,     9,    10,    11,     0,    12,     0,    13,     0,
      15,     0,     0,     0,    16,    17,    18,    19,    20,     0,
       0,     0,     0,    22,     0,     0,    23,    24,    25,     0,
       0,    26,     0,     0,     0,    28,    29,    30,     0,     0,
       0,     0,     0,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    32,     0,     0,    33,    34,
       0,     1,   384,     0,     0,     0,     0,    36,    37,     0,
       0,     0,   307,    38,     2,     0,    39,     3,     4,     0,
       0,     5,     6,     7,     8,     9,    10,    11,     0,    12,
       0,    13,     0,    15,     0,   177,     0,    16,    17,    18,
      19,     0,     0,   119,   120,   121,   122,   123,   124,    23,
     125,    25,     0,   126,   127,   128,     0,     0,    28,    29,
      30,     0,     0,     0,     0,     0,     0,    31,     0,   129,
     130,   131,   132,   133,   134,   135,     0,   178,    32,     0,
       0,     1,    34,     0,     0,     0,     0,     0,     0,     0,
      36,    37,     0,     0,     2,     0,    38,     3,     4,    39,
       0,     5,     0,     7,     8,     9,    59,    11,     0,    12,
       0,     0,     0,    15,     0,     0,     0,     1,     0,     0,
       0,     0,     0,     0,     0,    76,    77,    78,    79,    60,
       2,    25,     0,     3,     4,     0,     0,     5,     0,     7,
       8,     9,    59,    11,     0,    12,     0,    31,     0,    15,
       0,     0,     0,     0,     0,     0,     0,     0,    32,     0,
       0,    76,     0,    78,     0,    60,     0,    25,     0,     0,
      36,    37,     0,     1,     0,     0,    38,     0,     0,    39,
       0,     0,     0,    31,     0,     0,     2,     0,     0,     3,
       4,     0,     0,     5,    32,     7,     8,     9,    59,    11,
       0,    12,     0,     0,     0,    15,    36,    37,     0,     1,
       0,     0,    38,     0,     0,    39,     0,     0,     0,     0,
       0,    60,     2,    25,     0,     3,     4,     0,     0,     5,
       0,     7,     8,     9,   145,    11,     0,    12,     0,    31,
       0,    15,     0,     0,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    60,     0,    25,
       0,     0,    36,    37,     0,     1,     0,     0,    38,  -166,
    -166,    39,     0,     0,     0,    31,     0,     0,     2,     0,
       0,     3,     4,     0,    84,     5,    32,     7,     8,     9,
      59,    11,     0,    12,     0,     0,     0,    15,    36,    37,
       0,     1,     0,     0,    38,  -166,  -166,    39,     0,     0,
       0,     0,     0,    60,     2,    25,     0,     3,     4,     0,
       0,     5,     0,     7,     8,     9,    59,    11,     0,    12,
       0,    31,     0,    15,     0,     0,     0,     0,     0,     0,
       0,     0,    32,     0,     0,     0,     0,     0,     0,    60,
       0,    25,     0,     0,    36,    37,     0,     0,     0,     0,
      38,     0,     0,    39,     0,     0,    76,    31,    78,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    32,     0,
       0,     0,   119,   120,   121,   122,   123,   124,     0,   125,
      36,    37,   126,   127,   128,     0,    38,     0,     0,    39,
       0,   293,     0,     0,     0,     0,     0,   294,   129,   130,
     131,   132,   133,   134,   135,   119,   120,   121,   122,   123,
     124,     0,   125,     0,     0,   126,   127,   128,     0,     0,
       0,     0,     0,     0,   295,     0,     0,     0,     0,     0,
     296,   129,   130,   131,   132,   133,   134,   135,   119,   120,
     121,   122,   123,   124,     0,   125,     0,     0,   126,   127,
     128,     0,     0,     0,     0,     0,     0,   297,     0,     0,
       0,     0,     0,   298,   129,   130,   131,   132,   133,   134,
     135,   119,   120,   121,   122,   123,   124,     0,   125,     0,
       0,   126,   127,   128,     0,     0,     0,     0,     0,     0,
     299,     0,     0,     0,     0,     0,   300,   129,   130,   131,
     132,   133,   134,   135,   119,   120,   121,   122,   123,   124,
       0,   125,     0,     0,   126,   127,   128,     0,     0,     0,
       0,     0,     0,   333,     0,     0,     0,     0,     0,     0,
     129,   130,   131,   132,   133,   134,   135,   119,   120,   121,
     122,   123,   124,     0,   125,     0,     0,   126,   127,   128,
       0,     0,     0,     0,     0,     0,   334,     0,     0,     0,
       0,     0,     0,   129,   130,   131,   132,   133,   134,   135,
     119,   120,   121,   122,   123,   124,     0,   125,     0,     0,
     126,   127,   128,     0,     0,     0,     0,     0,     0,   335,
       0,     0,     0,     0,     0,     0,   129,   130,   131,   132,
     133,   134,   135,   119,   120,   121,   122,   123,   124,     0,
     125,     0,     0,   126,   127,   128,     0,     0,     0,     0,
       0,     0,   336,     0,     0,     0,     0,     0,     0,   129,
     130,   131,   132,   133,   134,   135,   119,   120,   121,   122,
     123,   124,     0,   125,     0,     0,   126,   127,   128,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   129,   130,   131,   132,   133,   134,   135,   171,
       0,     0,     0,     0,     0,   119,   120,   121,   122,   123,
     124,     0,   125,     0,     0,   126,   127,   128,   119,   120,
     121,   122,   123,   124,     0,   125,     0,     0,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,     0,     0,
       0,     0,     0,     0,   129,   130,   131,   132,   133,   134,
     135,     0,     0,     0,   182,   119,   120,   121,   122,   123,
     124,     0,   125,     0,     0,   126,   127,   128,     0,     0,
       0,     0,     0,   175,   176,     0,     0,     0,     0,     0,
       0,   129,   130,   131,   132,   133,   134,   135,   119,   120,
     121,   122,   123,   124,     0,   125,     0,     0,   126,   127,
     128,   119,   120,   121,   122,     0,     0,     0,   125,     0,
       0,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,     0,     0,     0,     0,     0,     0,   129,   130,   131,
     132,   133,   134,   135
};

static const yytype_int16 yycheck[] =
{
       6,     7,     8,   149,    10,   267,    53,     4,   326,   237,
      29,    53,    18,    42,    73,     0,    63,    23,    96,    97,
      98,    63,    28,    29,    68,    31,    32,    33,    87,   291,
      36,    37,    38,    39,    63,    24,    47,    26,    55,    56,
      46,    53,     6,     7,     8,    95,    90,    91,    92,    93,
     361,    63,    97,    98,    60,    61,     1,   375,    64,    96,
      66,    80,   373,    69,   101,    68,    72,   378,    50,    14,
      76,    68,    17,    18,    50,   221,    21,    96,    35,    36,
      95,    26,    27,    98,    29,    74,    75,    76,    77,    92,
      93,    53,    89,    90,    53,    95,   155,    61,    98,    26,
      64,    63,    66,    95,    63,    26,    98,     0,    72,   337,
      97,    98,    95,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     358,   137,   117,    96,    24,   141,    26,    97,    98,    26,
     368,   147,    24,    98,    26,    98,   374,   153,   263,    26,
      27,    96,    29,    45,    99,   161,   162,   163,    26,   156,
     166,   167,   168,   169,   170,   171,    97,    98,    26,   175,
     176,    26,   287,    97,    98,   290,    52,   183,   184,   185,
      68,   187,    26,    71,    74,    75,    76,    77,    26,   304,
     158,   159,    74,    75,    76,    77,    26,   101,   101,   314,
     100,    89,    90,    91,    92,    93,    26,    17,    18,    19,
      47,    52,    98,    98,    98,    98,    26,    98,    26,   225,
     226,   227,   228,    33,   283,   284,   223,    43,   343,   235,
     236,    26,    26,    26,   240,   241,   101,    26,    86,    26,
      53,    84,   301,    34,    98,    85,    53,    26,    84,    52,
     117,   252,   367,   178,   303,   188,   371,   266,    -1,    -1,
     266,    -1,   377,    -1,   270,    -1,   381,    -1,    -1,    -1,
      -1,   330,    -1,    -1,    -1,    -1,    -1,    -1,    34,    -1,
      -1,    -1,    -1,    -1,   281,    -1,    -1,   293,   294,   295,
     296,   297,   298,   299,   300,    -1,    -1,    -1,   357,    -1,
      -1,    -1,    -1,    -1,    -1,    61,    62,    63,    64,    65,
      66,    -1,    68,    -1,    -1,    71,    72,    73,    -1,    -1,
      -1,   327,   328,    -1,    -1,    -1,    -1,   333,   334,   335,
     336,    87,    88,    89,    90,    91,    92,    93,   344,    -1,
      -1,     0,     1,    -1,    -1,    -1,    -1,    -1,   354,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    -1,   363,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    -1,    43,    -1,    45,    46,    47,    -1,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    -1,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    -1,    95,    96,    97,    98,
      99,   100,     0,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    17,
      18,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    43,    -1,    45,    46,    47,
      -1,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    -1,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,     1,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,    -1,    -1,    -1,    14,
      -1,    43,    17,    18,    -1,    -1,    21,    -1,    23,    24,
      25,    26,    27,    -1,    29,    -1,    -1,    -1,    33,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      72,    73,    -1,    -1,    49,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     0,     1,    90,    91,    -1,    -1,    -1,
      -1,    96,    -1,    -1,    99,   100,   101,    14,    -1,    -1,
      17,    18,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    -1,    35,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      -1,    -1,    49,    50,    51,    -1,    -1,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    -1,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    -1,    -1,    81,    82,    -1,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    -1,    95,    96,
       0,     1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    14,    -1,    -1,    17,    18,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    29,
      30,    31,    -1,    33,    -1,    35,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    -1,    -1,    49,
      50,    51,    -1,    -1,    54,    -1,    -1,    -1,    58,    59,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    81,    82,    -1,    84,    85,    86,     0,     1,    -1,
      90,    91,    -1,    -1,    -1,    95,    96,    -1,    98,    99,
      -1,    14,    -1,    -1,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    30,    31,    -1,
      33,    -1,    35,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    -1,
      -1,    54,    -1,    -1,    -1,    58,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    81,    82,
      -1,    84,    85,    86,     0,     1,    -1,    90,    91,    -1,
      -1,    -1,    95,    96,    -1,    98,    99,    -1,    14,    -1,
      -1,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    -1,    31,    32,    33,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    -1,    -1,    44,    -1,
      46,    -1,    -1,    49,    50,    51,    -1,    -1,    54,    -1,
      -1,    57,    58,    59,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    81,    82,    83,     0,     1,
      -1,    -1,    -1,    -1,    90,    91,    -1,    -1,    -1,    95,
      96,    -1,    14,    99,    -1,    17,    18,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    30,    31,
      -1,    33,    -1,    -1,    -1,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,
      -1,    -1,    54,    -1,    -1,    -1,    58,    59,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    81,
      82,     0,     1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      -1,    -1,    -1,    95,    96,    14,    -1,    99,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    -1,    31,    -1,    33,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    -1,    -1,
      49,    50,    51,    -1,    -1,    54,    -1,    -1,    -1,    58,
      59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    -1,    81,    82,     0,     1,    -1,    -1,    -1,    -1,
      -1,    90,    91,    -1,    -1,    -1,    95,    96,    14,    -1,
      99,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    -1,    31,    -1,    33,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    -1,    -1,    49,    50,    51,    -1,    -1,    54,    -1,
      -1,    -1,    58,    59,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    81,    82,    -1,     1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    -1,    -1,    -1,    95,
      96,    14,    -1,    99,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    -1,    31,    -1,
      33,    -1,    35,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    -1,
      -1,    54,    -1,    -1,    -1,    58,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    81,    82,
      -1,    84,     1,    -1,    -1,    -1,    -1,    90,    91,    -1,
      -1,    -1,    95,    96,    -1,    14,    99,    -1,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    -1,    31,    -1,    33,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    -1,    -1,
      49,    50,    51,    -1,    -1,    54,    -1,    -1,    -1,    58,
      59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    -1,    81,    82,    -1,     1,    -1,    86,    -1,    -1,
      -1,    90,    91,    -1,    -1,    -1,    95,    96,    14,    -1,
      99,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    -1,    31,    -1,    33,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    -1,    -1,    49,    50,    51,    -1,    -1,    54,    -1,
      -1,    -1,    58,    59,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    81,    82,    -1,     1,    85,
      -1,    -1,    -1,    -1,    90,    91,    -1,    -1,    -1,    95,
      96,    14,    -1,    99,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    -1,    31,    -1,
      33,    -1,    -1,    -1,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    -1,
      -1,    54,    -1,    -1,    -1,    58,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    81,    82,
      -1,    84,     1,    -1,    -1,    -1,    -1,    90,    91,    -1,
      -1,    -1,    95,    96,    -1,    14,    99,    -1,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    -1,    31,    -1,    33,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    -1,    -1,
      49,    50,    51,    -1,    -1,    54,    -1,    -1,    -1,    58,
      59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    -1,    81,    82,    -1,     1,    85,    -1,    -1,    -1,
      -1,    90,    91,    -1,    -1,    -1,    95,    96,    14,    -1,
      99,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    -1,    31,    -1,    33,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    -1,    -1,    49,    50,    51,    -1,    -1,    54,    -1,
      -1,    -1,    58,    59,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    81,    82,    -1,     1,    85,
      -1,    -1,    -1,    -1,    90,    91,    -1,    -1,    -1,    95,
      96,    14,    -1,    99,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    -1,    31,    -1,
      33,    -1,    -1,    -1,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    -1,
      -1,    54,    -1,    -1,    -1,    58,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    81,    82,
      -1,     1,    85,    -1,    -1,    -1,    -1,    90,    91,    -1,
      -1,    -1,    95,    96,    14,    -1,    99,    17,    18,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    29,
      -1,    31,    -1,    33,    -1,    53,    -1,    37,    38,    39,
      40,    -1,    -1,    61,    62,    63,    64,    65,    66,    49,
      68,    51,    -1,    71,    72,    73,    -1,    -1,    58,    59,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    78,    -1,
      -1,     1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    91,    -1,    -1,    14,    -1,    96,    17,    18,    99,
      -1,    21,    -1,    23,    24,    25,    26,    27,    -1,    29,
      -1,    -1,    -1,    33,    -1,    -1,    -1,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    48,    49,
      14,    51,    -1,    17,    18,    -1,    -1,    21,    -1,    23,
      24,    25,    26,    27,    -1,    29,    -1,    67,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    45,    -1,    47,    -1,    49,    -1,    51,    -1,    -1,
      90,    91,    -1,     1,    -1,    -1,    96,    -1,    -1,    99,
      -1,    -1,    -1,    67,    -1,    -1,    14,    -1,    -1,    17,
      18,    -1,    -1,    21,    78,    23,    24,    25,    26,    27,
      -1,    29,    -1,    -1,    -1,    33,    90,    91,    -1,     1,
      -1,    -1,    96,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,    49,    14,    51,    -1,    17,    18,    -1,    -1,    21,
      -1,    23,    24,    25,    26,    27,    -1,    29,    -1,    67,
      -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    51,
      -1,    -1,    90,    91,    -1,     1,    -1,    -1,    96,    97,
      98,    99,    -1,    -1,    -1,    67,    -1,    -1,    14,    -1,
      -1,    17,    18,    -1,    20,    21,    78,    23,    24,    25,
      26,    27,    -1,    29,    -1,    -1,    -1,    33,    90,    91,
      -1,     1,    -1,    -1,    96,    97,    98,    99,    -1,    -1,
      -1,    -1,    -1,    49,    14,    51,    -1,    17,    18,    -1,
      -1,    21,    -1,    23,    24,    25,    26,    27,    -1,    29,
      -1,    67,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    -1,    -1,    90,    91,    -1,    -1,    -1,    -1,
      96,    -1,    -1,    99,    -1,    -1,    45,    67,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    61,    62,    63,    64,    65,    66,    -1,    68,
      90,    91,    71,    72,    73,    -1,    96,    -1,    -1,    99,
      -1,    47,    -1,    -1,    -1,    -1,    -1,    53,    87,    88,
      89,    90,    91,    92,    93,    61,    62,    63,    64,    65,
      66,    -1,    68,    -1,    -1,    71,    72,    73,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,    -1,    -1,
      53,    87,    88,    89,    90,    91,    92,    93,    61,    62,
      63,    64,    65,    66,    -1,    68,    -1,    -1,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      -1,    -1,    -1,    53,    87,    88,    89,    90,    91,    92,
      93,    61,    62,    63,    64,    65,    66,    -1,    68,    -1,
      -1,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    -1,    -1,    53,    87,    88,    89,
      90,    91,    92,    93,    61,    62,    63,    64,    65,    66,
      -1,    68,    -1,    -1,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    93,    61,    62,    63,
      64,    65,    66,    -1,    68,    -1,    -1,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    88,    89,    90,    91,    92,    93,
      61,    62,    63,    64,    65,    66,    -1,    68,    -1,    -1,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    93,    61,    62,    63,    64,    65,    66,    -1,
      68,    -1,    -1,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    89,    90,    91,    92,    93,    61,    62,    63,    64,
      65,    66,    -1,    68,    -1,    -1,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    93,    55,
      -1,    -1,    -1,    -1,    -1,    61,    62,    63,    64,    65,
      66,    -1,    68,    -1,    -1,    71,    72,    73,    61,    62,
      63,    64,    65,    66,    -1,    68,    -1,    -1,    71,    72,
      73,    87,    88,    89,    90,    91,    92,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      93,    -1,    -1,    -1,    97,    61,    62,    63,    64,    65,
      66,    -1,    68,    -1,    -1,    71,    72,    73,    -1,    -1,
      -1,    -1,    -1,    79,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    88,    89,    90,    91,    92,    93,    61,    62,
      63,    64,    65,    66,    -1,    68,    -1,    -1,    71,    72,
      73,    61,    62,    63,    64,    -1,    -1,    -1,    68,    -1,
      -1,    71,    72,    73,    87,    88,    89,    90,    91,    92,
      93,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,
      90,    91,    92,    93
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    14,    17,    18,    21,    22,    23,    24,    25,
      26,    27,    29,    31,    32,    33,    37,    38,    39,    40,
      41,    44,    46,    49,    50,    51,    54,    57,    58,    59,
      60,    67,    78,    81,    82,    83,    90,    91,    96,    99,
     103,   104,   105,   106,   107,   119,   125,   128,   129,   133,
     140,   142,   144,   147,   152,   155,    95,    26,   128,    26,
      49,    96,   129,   153,    96,   153,    96,   153,    47,    96,
     129,   154,   143,   146,    50,   134,    45,    46,    47,    48,
     129,   141,   145,    50,    20,   129,    55,    56,    17,    18,
      19,    26,    33,    26,    26,   129,   129,   141,   136,   129,
     129,   129,   135,   148,   129,   129,   129,    26,    27,    29,
     101,   129,   156,   157,   158,   159,     0,    95,   129,    61,
      62,    63,    64,    65,    66,    68,    71,    72,    73,    87,
      88,    89,    90,    91,    92,    93,   126,    96,   129,   129,
     153,    98,   153,   153,   128,    26,   153,    98,   153,   126,
      26,   137,   129,    45,   141,    26,    26,    43,    69,    70,
      26,   126,    53,    63,   128,   128,    53,    63,    53,    63,
      52,    55,   141,    26,   139,    79,    80,    53,    95,    26,
     138,    26,    97,   101,   101,   101,   100,    98,    98,   103,
     122,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,    26,   149,
      97,   129,    97,    97,    97,    98,    97,   129,   149,    98,
     129,   126,   128,    24,    26,    74,    75,    76,    77,   131,
     132,    26,   132,    26,   132,    42,    63,   129,   129,   129,
      53,    63,    47,   129,   129,   129,   129,   129,   129,    98,
     129,   129,   118,   118,    98,   129,   129,   129,   129,    26,
      27,    29,   159,    52,    95,    98,   130,    95,    26,   149,
      43,   128,   129,   129,   129,   129,   129,   129,   122,   129,
     129,    26,    26,   106,   127,    26,   127,   151,    26,   154,
     151,    95,   129,    47,    53,    47,    53,    47,    53,    47,
      53,   114,   108,   111,   117,   128,   126,    95,   107,   126,
     123,   127,    97,   127,   151,   129,   129,   129,   129,   129,
     129,   129,   129,   126,   124,   124,   127,    53,    63,    86,
     120,    30,   127,    47,    47,    47,    47,   115,   109,   112,
     123,   129,   129,    35,    36,   121,   126,    26,   150,   129,
     129,   129,   129,   122,    53,    34,    85,   127,   129,    84,
      98,   116,   129,    53,   126,   122,    26,   117,   110,   129,
      84,    52,   127,   122,   113,   127,    85,   117,   122,   123,
     127,   117,    85,   127,    85
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   102,   103,   103,   104,   104,   104,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   106,   106,   106,   107,   107,   107,
     108,   109,   110,   107,   111,   112,   113,   107,   114,   115,
     116,   107,   107,   107,   107,   107,   117,   118,   119,   119,
     120,   120,   121,   122,   123,   124,   125,   126,   127,   127,
     127,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   129,   129,   129,   129,   129,   129,   130,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   131,   131,   131,   131,   131,
     131,   131,   131,   132,   132,   133,   133,   133,   133,   133,
     134,   133,   135,   133,   136,   133,   133,   133,   133,   133,
     137,   137,   138,   138,   139,   139,   140,   140,   140,   140,
     140,   140,   141,   141,   142,   142,   142,   143,   142,   145,
     144,   144,   146,   144,   144,   144,   148,   147,   149,   149,
     149,   150,   150,   150,   151,   152,   153,   153,   153,   154,
     154,   155,   156,   156,   156,   156,   157,   157,   158,   158,
     159,   159,   159
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     4,     4,     6,
       4,     4,     4,     4,     4,     4,     4,     4,     5,     5,
       8,     8,     4,     4,     1,     1,     1,     1,     1,     9,
       0,     0,     0,    15,     0,     0,     0,    16,     0,     0,
       0,    13,     2,     4,     7,     6,     0,     0,     9,    11,
       0,     2,     6,     0,     0,     0,     1,     0,     0,     2,
       2,     1,     1,     1,     1,     1,     1,     2,     3,     1,
       2,     1,     4,     2,     4,     2,     4,     0,     7,     4,
       3,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     3,     3,
       3,     2,     2,     4,     4,     4,     6,     4,     6,     4,
       6,     4,     6,     2,     1,     2,     1,     1,     2,     1,
       0,     3,     0,     3,     0,     3,     4,     2,     4,     2,
       1,     3,     1,     3,     1,     3,     1,     2,     2,     2,
       3,     2,     3,     2,     2,     3,     2,     0,     3,     0,
       8,     2,     0,     7,     8,     6,     0,     3,     0,     1,
       3,     0,     1,     3,     0,     2,     0,     1,     3,     1,
       3,     3,     0,     1,     1,     1,     1,     3,     1,     3,
       3,     3,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




/* The kind of the lookahead of this context.  */
static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx) YY_ATTRIBUTE_UNUSED;

static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx)
{
  return yyctx->yytoken;
}



/* User defined function to report a syntax error.  */
static int
yyreport_syntax_error (const yypcontext_t *yyctx);

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case 22: /* BLTIN  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1989 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* FBLTIN  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1995 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* RBLTIN  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2001 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* THEFBLTIN  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2007 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* ID  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2013 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* STRING  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2019 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* HANDLER  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2025 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* SYMBOL  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2031 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 30: /* ENDCLAUSE  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2037 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 31: /* tPLAYACCEL  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2043 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 32: /* tMETHOD  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2049 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 147: /* on  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2055 "engines/director/lingo/lingo-gr.cpp"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize;

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yynerrs = 0;
  yystate = 0;
  yyerrstatus = 0;

  yystacksize = YYINITDEPTH;
  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;


  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 7:
#line 221 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2342 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 228 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2350 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 232 "engines/director/lingo/lingo-gr.y"
                                                                {
		if (!(yyvsp[-3].s)->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		g_lingo->code1(LC::c_themenuentitypush);
		g_lingo->codeInt((yyvsp[-4].e)[0]);
		g_lingo->codeInt((yyvsp[-4].e)[1]);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code); }
#line 2366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 243 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_after);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2378 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 250 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_after);
		(yyval.code) = (yyvsp[-2].code); }
#line 2386 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 253 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_before);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2398 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 260 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_before);
		(yyval.code) = (yyvsp[-2].code); }
#line 2406 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 263 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 270 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2430 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 277 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2442 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 284 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2454 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 291 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2465 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 297 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2476 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20:
#line 304 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if (!(yyvsp[-3].s)->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		warning("STUB: menuItem entity");
		g_lingo->code1(LC::c_themenuitementityassign);
		g_lingo->codeInt((yyvsp[-6].e)[0]);
		g_lingo->codeInt((yyvsp[-6].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21:
#line 315 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if (!(yyvsp[-3].s)->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		warning("STUB: menuItem entity");
		g_lingo->code1(LC::c_themenuitementityassign);
		g_lingo->codeInt((yyvsp[-6].e)[0]);
		g_lingo->codeInt((yyvsp[-6].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2508 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22:
#line 326 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2520 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23:
#line 333 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2532 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 351 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-6].code) - (yyvsp[-1].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-4].code) + 2);
		(*g_lingo->_currentAssembly)[(yyvsp[-4].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentAssembly)[(yyvsp[-1].code)] = start;	/* looping back */
		endRepeat((yyvsp[-1].code) + 1, (yyvsp[-6].code));	}
#line 2544 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 364 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2552 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 368 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2559 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 371 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_le); }
#line 2565 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 371 "engines/director/lingo/lingo-gr.y"
                                                                                                      {

		int nextPos = g_lingo->_currentAssembly->size();
		g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[-12].s)->c_str());
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_add);
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-12].s)->c_str());
		g_lingo->code1(LC::c_assign);
		g_lingo->code2(LC::c_jump, 0);
		int pos = g_lingo->_currentAssembly->size() - 1;

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-8].code) - pos + 2);
		WRITE_UINT32(&end, pos - (yyvsp[-3].code) + 2);
		(*g_lingo->_currentAssembly)[pos] = loop;		/* final count value */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end;	/* end, if cond fails */
		endRepeat(pos + 1, nextPos); }
#line 2590 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 398 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2598 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 402 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 405 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_ge); }
#line 2611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 406 "engines/director/lingo/lingo-gr.y"
                                                                {

		int nextPos = g_lingo->_currentAssembly->size();
		g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[-13].s)->c_str());
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_sub);
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-13].s)->c_str());
		g_lingo->code1(LC::c_assign);
		g_lingo->code2(LC::c_jump, 0);
		int pos = g_lingo->_currentAssembly->size() - 1;

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-9].code) - pos + 2);
		WRITE_UINT32(&end, pos - (yyvsp[-3].code) + 2);
		(*g_lingo->_currentAssembly)[pos] = loop;		/* final count value */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end;	/* end, if cond fails */
		endRepeat(pos + 1, nextPos); }
#line 2636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 432 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  Common::String count("count");
				  g_lingo->codeFunc(&count, 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
#line 2647 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 439 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
#line 2657 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 445 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get list
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(1);
				  Common::String getAt("getAt");
				  g_lingo->codeFunc(&getAt, 2);
				  g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-6].s)->c_str());
				  mVar((yyvsp[-6].s), globalCheck());
				  g_lingo->code1(LC::c_assign); }
#line 2672 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 455 "engines/director/lingo/lingo-gr.y"
                                                        {

		int nextPos = g_lingo->_currentAssembly->size();
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_add);			// Increment counter

		int jump = g_lingo->code2(LC::c_jump, 0);

		int end2 = g_lingo->code1(LC::c_stackdrop);	// remove list, size, counter
		g_lingo->codeInt(3);

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-6].code) - jump);
		WRITE_UINT32(&end, end2 - (yyvsp[-4].code) + 1);

		(*g_lingo->_currentAssembly)[jump + 1] = loop;		/* final count value */
		(*g_lingo->_currentAssembly)[(yyvsp[-4].code)] = end;		/* end, if cond fails */
		endRepeat(end2, nextPos); }
#line 2696 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 475 "engines/director/lingo/lingo-gr.y"
                        {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->nexts.push_back(pos);
		} else {
			warning("# LINGO: next repeat not inside repeat block");
		} }
#line 2709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 483 "engines/director/lingo/lingo-gr.y"
                              {
		g_lingo->code1(LC::c_whencode);
		g_lingo->codeString((yyvsp[-2].s)->c_str()); }
#line 2717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 486 "engines/director/lingo/lingo-gr.y"
                                                          { g_lingo->code1(LC::c_telldone); }
#line 2723 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 487 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->code1(LC::c_telldone); }
#line 2729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 489 "engines/director/lingo/lingo-gr.y"
                                { startRepeat(); }
#line 2735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 491 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_tell); }
#line 2741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 493 "engines/director/lingo/lingo-gr.y"
                                                                                         {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 500 "engines/director/lingo/lingo-gr.y"
                                                                                                          {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].code) + 1 - (yyvsp[-8].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-8].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-5].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-5].code), (yyvsp[-1].code)); }
#line 2765 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 511 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2775 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 517 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 521 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2791 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 525 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2799 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 529 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2806 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 532 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2812 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 534 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 538 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2826 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 541 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2834 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 544 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2843 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 548 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2852 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 552 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 556 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 564 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2885 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 570 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2891 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 572 "engines/director/lingo/lingo-gr.y"
                                {
		// Director parser till D3 was forgiving for any hanging parentheses
		if (g_lingo->_ignoreError) {
			warning("# LINGO: Ignoring trailing paren before %d:%d", g_lingo->_linenumber, g_lingo->_colnumber);
			g_lingo->_ignoreError = false;
			lex_unput('\n');	// We ate '\n', so put it back, otherwise lines will be joined
		} else {
			yyerrok;
		}
	}
#line 2906 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 583 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2912 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 584 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2920 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 587 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 590 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2936 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 593 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2944 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 596 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_lazyeval);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeFunc((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 2955 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 602 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_lazyeval); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 2961 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 603 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeFunc((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 2970 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 607 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2978 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 610 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2986 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 613 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_objectproppush);
		g_lingo->codeString((yyvsp[0].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectprop).prop->c_str());
		delete (yyvsp[0].objectprop).obj;
		delete (yyvsp[0].objectprop).prop; }
#line 2997 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 620 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 3003 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 621 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 3009 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 622 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 3015 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 623 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 3021 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 624 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 3027 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 625 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 3033 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 626 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 3039 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 627 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 3045 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 628 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 3051 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 629 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 3057 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 630 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 3063 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 631 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 3069 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 632 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 3075 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 633 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 3081 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 634 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 3087 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 635 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 3093 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 636 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 3099 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 637 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 3105 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 638 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); }
#line 3111 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 639 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 3117 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 640 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_intersects); }
#line 3123 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 641 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 3129 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 643 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charOf); }
#line 3135 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 644 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charToOf); }
#line 3141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 645 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemOf); }
#line 3147 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 646 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_itemToOf); }
#line 3153 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 647 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineOf); }
#line 3159 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 648 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_lineToOf); }
#line 3165 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 649 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordOf); }
#line 3171 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 650 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_wordToOf); }
#line 3177 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 652 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 3185 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 657 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_printtop); }
#line 3191 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 660 "engines/director/lingo/lingo-gr.y"
                                                {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->exits.push_back(pos);
		} else {
			warning("# LINGO: exit repeat not inside repeat block");
		} }
#line 3204 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 668 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 3210 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 669 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3216 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 669 "engines/director/lingo/lingo-gr.y"
                                                                                 { inLast(); }
#line 3222 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 670 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 670 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 3234 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 671 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3240 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 671 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 3246 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 672 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3254 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 675 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3262 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 678 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String open("open");
		g_lingo->codeFunc(&open, 2); }
#line 3270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 681 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String open("open");
		g_lingo->codeFunc(&open, 1); }
#line 3278 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 685 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3286 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 688 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3294 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 692 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3302 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 695 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3310 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 699 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3318 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 702 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3326 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 713 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3332 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 714 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3338 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 715 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3344 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 716 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3353 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 720 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3362 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 724 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3371 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 732 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3380 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 736 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3389 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 740 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3398 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 744 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3404 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 744 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3413 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 774 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3419 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 775 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3429 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 780 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3435 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 781 "engines/director/lingo/lingo-gr.y"
                  { startDef(); (*g_lingo->_methodVars)["me"] = kVarArgument; }
#line 3441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 782 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1);
		endDef();
		delete (yyvsp[-6].s); }
#line 3451 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 787 "engines/director/lingo/lingo-gr.y"
                                                                   {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3464 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 795 "engines/director/lingo/lingo-gr.y"
                                               {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3474 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 801 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3480 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 801 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = (yyvsp[0].s); }
#line 3486 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 803 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 804 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3498 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 805 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3504 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 808 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3510 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 809 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 811 "engines/director/lingo/lingo-gr.y"
                                        { inDef(); }
#line 3522 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 813 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 3534 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 821 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3540 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 822 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3546 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 823 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3552 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 825 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3558 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 826 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3564 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 828 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3570 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172:
#line 830 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173:
#line 831 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174:
#line 832 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175:
#line 833 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3594 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176:
#line 835 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3600 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177:
#line 836 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3606 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178:
#line 838 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3612 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179:
#line 839 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180:
#line 841 "engines/director/lingo/lingo-gr.y"
                          {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3627 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181:
#line 845 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182:
#line 849 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3645 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3649 "engines/director/lingo/lingo-gr.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        if (yyreport_syntax_error (&yyctx) == 2)
          goto yyexhaustedlab;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if 1
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 855 "engines/director/lingo/lingo-gr.y"


int yyreport_syntax_error(const yypcontext_t *ctx) {
	int res = 0;

	if (lex_check_parens()) {
		g_lingo->_ignoreError = true;
		return 0;
	}

	Common::String msg = "syntax error, ";

	// Report the unexpected token.
	yysymbol_kind_t lookahead = yypcontext_token(ctx);
	if (lookahead != YYSYMBOL_YYEMPTY)
		msg += Common::String::format("unexpected %s", yysymbol_name(lookahead));

	// Report the tokens expected at this point.
	enum { TOKENMAX = 10 };
	yysymbol_kind_t expected[TOKENMAX];

	int n = yypcontext_expected_tokens(ctx, expected, TOKENMAX);
	if (n < 0)
		// Forward errors to yyparse.
		res = n;
	else
		for (int i = 0; i < n; ++i)
			msg += Common::String::format("%s %s", i == 0 ? ": expected" : " or", yysymbol_name(expected[i]));

	yyerror(msg.c_str());

	return res;
}
