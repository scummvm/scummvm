/* A Bison parser, made by GNU Bison 3.6.2.  */

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
#define YYBISON_VERSION "3.6.2"

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

static void inArgs() { g_lingo->_indef = kStateInArgs; }
static void inDef()  { g_lingo->_indef = kStateInDef; }
static void inNone() { g_lingo->_indef = kStateNone; }

static void startDef() {
	inArgs();
	g_lingo->_methodVarsStash = g_lingo->_methodVars;
	g_lingo->_methodVars = new Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>();
}

static void endDef() {
	g_lingo->clearArgStack();
	inNone();
	g_lingo->_ignoreMe = false;

	for (Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator i = g_lingo->_methodVars->begin(); i != g_lingo->_methodVars->end(); ++i) {
		if (i->_value == kVarInstance) {
			if (g_lingo->_currentFactory != nullptr) {
				g_lingo->_currentFactory->properties[i->_key] = Symbol();
				g_lingo->_currentFactory->properties[i->_key].name = new Common::String(i->_key);
			} else {
				warning("Instance var '%s' defined outside factory", i->_key.c_str());
			}
		}
	}

	delete g_lingo->_methodVars;
	g_lingo->_methodVars = g_lingo->_methodVarsStash;
	g_lingo->_methodVarsStash = nullptr;
}

static void mVar(Common::String *s, VarType type) {
	if (!g_lingo->_methodVars->contains(*s))
		(*g_lingo->_methodVars)[*s] = type;
}


#line 142 "engines/director/lingo/lingo-gr.cpp"

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
    REFERENCE = 266,               /* REFERENCE  */
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
    THEOBJECTFIELD = 288,          /* THEOBJECTFIELD  */
    THEOBJECTREF = 289,            /* THEOBJECTREF  */
    tDOWN = 290,                   /* tDOWN  */
    tELSE = 291,                   /* tELSE  */
    tELSIF = 292,                  /* tELSIF  */
    tEXIT = 293,                   /* tEXIT  */
    tGLOBAL = 294,                 /* tGLOBAL  */
    tGO = 295,                     /* tGO  */
    tGOLOOP = 296,                 /* tGOLOOP  */
    tIF = 297,                     /* tIF  */
    tIN = 298,                     /* tIN  */
    tINTO = 299,                   /* tINTO  */
    tMACRO = 300,                  /* tMACRO  */
    tMOVIE = 301,                  /* tMOVIE  */
    tNEXT = 302,                   /* tNEXT  */
    tOF = 303,                     /* tOF  */
    tPREVIOUS = 304,               /* tPREVIOUS  */
    tPUT = 305,                    /* tPUT  */
    tREPEAT = 306,                 /* tREPEAT  */
    tSET = 307,                    /* tSET  */
    tTHEN = 308,                   /* tTHEN  */
    tTO = 309,                     /* tTO  */
    tWHEN = 310,                   /* tWHEN  */
    tWITH = 311,                   /* tWITH  */
    tWHILE = 312,                  /* tWHILE  */
    tFACTORY = 313,                /* tFACTORY  */
    tOPEN = 314,                   /* tOPEN  */
    tPLAY = 315,                   /* tPLAY  */
    tINSTANCE = 316,               /* tINSTANCE  */
    tGE = 317,                     /* tGE  */
    tLE = 318,                     /* tLE  */
    tEQ = 319,                     /* tEQ  */
    tNEQ = 320,                    /* tNEQ  */
    tAND = 321,                    /* tAND  */
    tOR = 322,                     /* tOR  */
    tNOT = 323,                    /* tNOT  */
    tMOD = 324,                    /* tMOD  */
    tAFTER = 325,                  /* tAFTER  */
    tBEFORE = 326,                 /* tBEFORE  */
    tCONCAT = 327,                 /* tCONCAT  */
    tCONTAINS = 328,               /* tCONTAINS  */
    tSTARTS = 329,                 /* tSTARTS  */
    tCHAR = 330,                   /* tCHAR  */
    tITEM = 331,                   /* tITEM  */
    tLINE = 332,                   /* tLINE  */
    tWORD = 333,                   /* tWORD  */
    tSPRITE = 334,                 /* tSPRITE  */
    tINTERSECTS = 335,             /* tINTERSECTS  */
    tWITHIN = 336,                 /* tWITHIN  */
    tTELL = 337,                   /* tTELL  */
    tPROPERTY = 338,               /* tPROPERTY  */
    tON = 339,                     /* tON  */
    tENDIF = 340,                  /* tENDIF  */
    tENDREPEAT = 341,              /* tENDREPEAT  */
    tENDTELL = 342                 /* tENDTELL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 125 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Director::DatumArray *arr;

	struct {
		Common::String *os;
		int oe;
	} objectfield;

	struct {
		Common::String *obj;
		Common::String *field;
	} objectref;

#line 299 "engines/director/lingo/lingo-gr.cpp"

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
  YYSYMBOL_REFERENCE = 11,                 /* REFERENCE  */
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
  YYSYMBOL_THEOBJECTFIELD = 33,            /* THEOBJECTFIELD  */
  YYSYMBOL_THEOBJECTREF = 34,              /* THEOBJECTREF  */
  YYSYMBOL_tDOWN = 35,                     /* tDOWN  */
  YYSYMBOL_tELSE = 36,                     /* tELSE  */
  YYSYMBOL_tELSIF = 37,                    /* tELSIF  */
  YYSYMBOL_tEXIT = 38,                     /* tEXIT  */
  YYSYMBOL_tGLOBAL = 39,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 40,                       /* tGO  */
  YYSYMBOL_tGOLOOP = 41,                   /* tGOLOOP  */
  YYSYMBOL_tIF = 42,                       /* tIF  */
  YYSYMBOL_tIN = 43,                       /* tIN  */
  YYSYMBOL_tINTO = 44,                     /* tINTO  */
  YYSYMBOL_tMACRO = 45,                    /* tMACRO  */
  YYSYMBOL_tMOVIE = 46,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 47,                     /* tNEXT  */
  YYSYMBOL_tOF = 48,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 49,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 50,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 51,                   /* tREPEAT  */
  YYSYMBOL_tSET = 52,                      /* tSET  */
  YYSYMBOL_tTHEN = 53,                     /* tTHEN  */
  YYSYMBOL_tTO = 54,                       /* tTO  */
  YYSYMBOL_tWHEN = 55,                     /* tWHEN  */
  YYSYMBOL_tWITH = 56,                     /* tWITH  */
  YYSYMBOL_tWHILE = 57,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 58,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 59,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 60,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 61,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 62,                       /* tGE  */
  YYSYMBOL_tLE = 63,                       /* tLE  */
  YYSYMBOL_tEQ = 64,                       /* tEQ  */
  YYSYMBOL_tNEQ = 65,                      /* tNEQ  */
  YYSYMBOL_tAND = 66,                      /* tAND  */
  YYSYMBOL_tOR = 67,                       /* tOR  */
  YYSYMBOL_tNOT = 68,                      /* tNOT  */
  YYSYMBOL_tMOD = 69,                      /* tMOD  */
  YYSYMBOL_tAFTER = 70,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 71,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 72,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 73,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 74,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 75,                     /* tCHAR  */
  YYSYMBOL_tITEM = 76,                     /* tITEM  */
  YYSYMBOL_tLINE = 77,                     /* tLINE  */
  YYSYMBOL_tWORD = 78,                     /* tWORD  */
  YYSYMBOL_tSPRITE = 79,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 80,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 81,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 82,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 83,                 /* tPROPERTY  */
  YYSYMBOL_tON = 84,                       /* tON  */
  YYSYMBOL_tENDIF = 85,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 86,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 87,                  /* tENDTELL  */
  YYSYMBOL_88_ = 88,                       /* '<'  */
  YYSYMBOL_89_ = 89,                       /* '>'  */
  YYSYMBOL_90_ = 90,                       /* '&'  */
  YYSYMBOL_91_ = 91,                       /* '+'  */
  YYSYMBOL_92_ = 92,                       /* '-'  */
  YYSYMBOL_93_ = 93,                       /* '*'  */
  YYSYMBOL_94_ = 94,                       /* '/'  */
  YYSYMBOL_95_ = 95,                       /* '%'  */
  YYSYMBOL_96_n_ = 96,                     /* '\n'  */
  YYSYMBOL_97_ = 97,                       /* '('  */
  YYSYMBOL_98_ = 98,                       /* ')'  */
  YYSYMBOL_99_ = 99,                       /* ','  */
  YYSYMBOL_100_ = 100,                     /* '['  */
  YYSYMBOL_101_ = 101,                     /* ']'  */
  YYSYMBOL_102_ = 102,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 103,                 /* $accept  */
  YYSYMBOL_program = 104,                  /* program  */
  YYSYMBOL_programline = 105,              /* programline  */
  YYSYMBOL_asgn = 106,                     /* asgn  */
  YYSYMBOL_stmtoneliner = 107,             /* stmtoneliner  */
  YYSYMBOL_stmt = 108,                     /* stmt  */
  YYSYMBOL_109_1 = 109,                    /* $@1  */
  YYSYMBOL_110_2 = 110,                    /* $@2  */
  YYSYMBOL_111_3 = 111,                    /* $@3  */
  YYSYMBOL_112_4 = 112,                    /* $@4  */
  YYSYMBOL_113_5 = 113,                    /* $@5  */
  YYSYMBOL_114_6 = 114,                    /* $@6  */
  YYSYMBOL_115_7 = 115,                    /* $@7  */
  YYSYMBOL_116_8 = 116,                    /* $@8  */
  YYSYMBOL_117_9 = 117,                    /* $@9  */
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
  YYSYMBOL_chunkexpr = 130,                /* chunkexpr  */
  YYSYMBOL_reference = 131,                /* reference  */
  YYSYMBOL_proc = 132,                     /* proc  */
  YYSYMBOL_133_10 = 133,                   /* $@10  */
  YYSYMBOL_134_11 = 134,                   /* $@11  */
  YYSYMBOL_135_12 = 135,                   /* $@12  */
  YYSYMBOL_globallist = 136,               /* globallist  */
  YYSYMBOL_propertylist = 137,             /* propertylist  */
  YYSYMBOL_instancelist = 138,             /* instancelist  */
  YYSYMBOL_gotofunc = 139,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 140,                /* gotomovie  */
  YYSYMBOL_playfunc = 141,                 /* playfunc  */
  YYSYMBOL_142_13 = 142,                   /* $@13  */
  YYSYMBOL_defn = 143,                     /* defn  */
  YYSYMBOL_144_14 = 144,                   /* $@14  */
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
#define YYFINAL  125
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2546

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  103
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  57
/* YYNRULES -- Number of rules.  */
#define YYNRULES  174
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  363

#define YYMAXUTOK   342


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
      96,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    95,    90,     2,
      97,    98,    93,    91,    99,    92,     2,    94,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   102,     2,
      88,     2,    89,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   100,     2,   101,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87
};

#if YYDEBUG
  /* YYRLINEYYN -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   184,   184,   185,   187,   188,   189,   191,   198,   202,
     213,   214,   215,   222,   229,   236,   243,   249,   256,   267,
     274,   275,   276,   278,   279,   284,   296,   300,   303,   295,
     327,   331,   334,   326,   360,   367,   373,   359,   401,   403,
     406,   407,   409,   411,   418,   426,   427,   429,   435,   439,
     443,   447,   450,   452,   453,   454,   456,   459,   462,   466,
     470,   474,   482,   488,   489,   490,   501,   502,   503,   506,
     509,   512,   515,   520,   526,   527,   528,   529,   530,   531,
     532,   533,   534,   535,   536,   537,   538,   539,   540,   541,
     542,   543,   544,   545,   546,   547,   548,   550,   551,   552,
     553,   554,   555,   556,   557,   559,   562,   564,   565,   566,
     567,   568,   569,   569,   570,   570,   571,   571,   572,   575,
     578,   579,   581,   586,   592,   597,   603,   606,   617,   618,
     619,   620,   624,   628,   633,   634,   636,   640,   644,   648,
     648,   678,   678,   678,   684,   685,   685,   691,   699,   705,
     705,   708,   709,   710,   712,   713,   714,   716,   718,   726,
     727,   728,   730,   731,   733,   735,   736,   737,   738,   740,
     741,   743,   744,   746,   750
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
  "VAR", "POINT", "RECT", "ARRAY", "OBJECT", "REFERENCE", "LEXERROR",
  "PARRAY", "INT", "ARGC", "ARGCNORET", "THEENTITY", "THEENTITYWITHID",
  "THEMENUITEMENTITY", "THEMENUITEMSENTITY", "FLOAT", "BLTIN", "FBLTIN",
  "RBLTIN", "THEFBLTIN", "ID", "STRING", "HANDLER", "SYMBOL", "ENDCLAUSE",
  "tPLAYACCEL", "tMETHOD", "THEOBJECTFIELD", "THEOBJECTREF", "tDOWN",
  "tELSE", "tELSIF", "tEXIT", "tGLOBAL", "tGO", "tGOLOOP", "tIF", "tIN",
  "tINTO", "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT",
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE",
  "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tENDIF",
  "tENDREPEAT", "tENDTELL", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'\\n'", "'('", "')'", "','", "'['", "']'", "':'",
  "$accept", "program", "programline", "asgn", "stmtoneliner", "stmt",
  "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9",
  "tellstart", "ifstmt", "elseifstmtlist", "elseifstmt", "jumpifz", "jump",
  "varassign", "if", "lbl", "stmtlist", "simpleexpr", "expr", "chunkexpr",
  "reference", "proc", "$@10", "$@11", "$@12", "globallist",
  "propertylist", "instancelist", "gotofunc", "gotomovie", "playfunc",
  "$@13", "defn", "$@14", "$@15", "$@16", "on", "$@17", "argdef",
  "endargdef", "argstore", "macro", "arglist", "nonemptyarglist", "list",
  "valuelist", "linearlist", "proplist", "proppair", YY_NULLPTR
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
     335,   336,   337,   338,   339,   340,   341,   342,    60,    62,
      38,    43,    45,    42,    47,    37,    10,    40,    41,    44,
      91,    93,    58
};
#endif

#define YYPACT_NINF (-293)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-166)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACTSTATE-NUM -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     767,   -91,  -293,  -293,    53,  -293,   591,   310,    53,   -38,
     493,  -293,  -293,  -293,  -293,  -293,  -293,   -37,  -293,   225,
    -293,  -293,  -293,   -26,  1899,    36,   173,    26,    33,  1929,
    1775,  -293,  1929,  1929,  1929,  1929,  1929,  1929,  1929,  -293,
    -293,  1929,  1929,  1929,   404,    39,   -28,  -293,  -293,  -293,
    -293,  1929,  -293,  2439,  -293,  -293,  -293,  -293,  -293,  -293,
    -293,  -293,  -293,  -293,  -293,  -293,   -12,  1899,  1812,  2439,
     -10,  1812,   -10,  -293,    53,  1812,  2439,    -2,   679,  -293,
    -293,    87,  1929,  -293,    68,  -293,  2020,  -293,    89,  -293,
      90,  1987,    91,  -293,   -20,    53,    53,   -11,    64,    66,
    -293,  2356,  2020,  -293,    94,  -293,  2053,  2086,  2119,  2152,
    2406,  2317,   113,   116,  -293,  -293,  2369,    41,    43,  -293,
    2439,    45,    48,    50,  -293,  -293,   767,  2439,  1929,  1929,
    1929,  1929,  1929,  1929,  1929,  1929,  1929,  1929,  1929,  1929,
    1929,  1929,  1929,  1929,  1929,   125,  1812,  1987,  2369,   -51,
    1929,    -3,  -293,    11,  1929,   -10,   125,  -293,    57,  2439,
    1929,  -293,  -293,    53,    25,  1929,  1929,   -22,  1929,  1929,
    1929,    -4,   104,  1929,  1929,  1929,  1929,  1929,  -293,  -293,
      58,  1929,  1929,  1929,  1929,  1929,  1929,  1929,  1929,  1929,
    1929,  -293,  -293,  -293,    59,  -293,  -293,    53,    53,  -293,
    1929,    46,  -293,   108,    14,    14,    14,    14,  2452,  2452,
    -293,   -36,    14,    14,    14,    14,   -36,   -53,   -53,  -293,
    -293,  -293,   -87,  -293,  2439,  -293,  -293,  2439,   -33,   139,
    2439,  -293,   129,  -293,  -293,  2439,  2439,  1929,  1929,  2439,
    2439,    14,  1929,  1929,   148,  2439,    14,  2439,  2439,  2439,
     159,  2439,  2185,  2439,  2218,  2439,  2251,  2439,  2284,  2439,
    2439,  1694,  -293,   160,  -293,  -293,  2439,    41,    43,  -293,
    -293,  -293,   161,  -293,  -293,   125,  1929,  2439,   377,  -293,
    2439,    14,    53,  -293,  1929,  1929,  1929,  1929,  -293,  1189,
    -293,  1104,  -293,  -293,  -293,    -9,  2439,  -293,  -293,  -293,
    1273,   135,  2439,  2439,  2439,  2439,  -293,  -293,  -293,   106,
    -293,   852,   936,  -293,  -293,  -293,  -293,   109,  1929,  -293,
      75,   170,  -293,  -293,   143,   167,  -293,  2439,  -293,  1929,
    -293,   118,  -293,   110,  1020,  -293,  1929,   151,  1357,  2439,
    -293,   181,  -293,  2439,  1929,   123,   157,  -293,  1442,  -293,
    2439,  -293,  -293,  -293,  -293,  -293,  1104,  1526,  -293,  -293,
    -293,  1610,  -293
};

  /* YYDEFACTSTATE-NUM -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    56,    61,     0,    57,     0,     0,     0,     0,
       0,    59,    58,   139,   145,    72,    73,   111,   112,     0,
     128,    51,   141,     0,     0,     0,     0,     0,     0,     0,
       0,   116,     0,     0,     0,     0,     0,     0,     0,   114,
     149,     0,     0,     0,     0,     0,     2,    74,    23,     6,
      24,     0,    66,    21,   106,    67,    22,   108,   109,     5,
      52,    20,    64,    65,    60,    62,    60,     0,     0,   160,
     119,     0,    69,   105,     0,     0,   162,   158,     0,    52,
     110,     0,     0,   129,     0,   130,   131,   133,     0,    38,
       0,   107,     0,    52,     0,     0,     0,     0,     0,     0,
     144,   121,   136,   138,     0,    88,     0,     0,     0,     0,
       0,     0,     0,     0,    93,    94,     0,    59,    58,   166,
     169,     0,   167,   168,   171,     1,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   151,     0,     0,   160,     0,
       0,     0,    71,     0,     0,   140,   151,   122,   113,   135,
       0,   132,   142,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   137,   126,
     117,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    42,    42,   124,   115,   150,    63,     0,     0,   164,
       0,     0,     3,     0,    84,    85,    82,    83,    86,    87,
      79,    90,    91,    92,    81,    80,    89,    75,    76,    77,
      78,   152,     0,   118,   161,    68,    70,   163,     0,     0,
     134,    52,     0,     7,     8,    10,    11,     0,     0,    48,
      15,    13,     0,     0,     0,    14,    12,    19,    39,   120,
       0,    97,     0,    99,     0,   101,     0,   103,     0,    95,
      96,     0,    53,     0,   174,   173,   170,     0,     0,   172,
      53,   157,     0,   157,   123,   151,     0,    34,    26,    53,
      16,    17,     0,   127,     0,     0,     0,     0,    52,     0,
     125,     0,    53,   153,    53,     0,     9,    52,    50,    50,
       0,     0,    98,   100,   102,   104,    41,    54,    55,     0,
      45,     0,     0,   157,    35,    27,    31,     0,     0,    40,
      52,   154,    53,    48,     0,     0,    25,    18,    53,     0,
      46,     0,   155,   147,     0,    36,     0,     0,     0,    48,
      43,     0,    53,    28,     0,     0,     0,   156,     0,    48,
      32,    44,    53,    37,    53,    48,     0,     0,    53,    47,
      29,     0,    33
};

  /* YYPGOTONTERM-NUM.  */
static const yytype_int16 yypgoto[] =
{
    -293,    85,  -293,  -293,   -47,    15,  -293,  -293,  -293,  -293,
    -293,  -293,  -293,  -293,  -293,    23,  -293,  -293,  -293,  -151,
    -292,   -83,  -293,   -76,  1690,     3,    -6,  -293,    54,  -293,
    -293,  -293,  -293,  -293,  -293,  -293,  -293,    -8,  -293,  -293,
    -293,  -293,  -293,  -293,  -293,  -293,  -154,  -293,  -267,  -293,
      13,  -293,  -293,  -293,  -293,  -293,    16
};

  /* YYDEFGOTONTERM-NUM.  */
static const yytype_int16 yydefgoto[] =
{
      -1,    45,    46,    47,    48,   308,   298,   324,   349,   299,
     325,   355,   297,   323,   342,   261,    50,   320,   330,   203,
     310,   315,    51,   145,   289,    52,    53,    54,    55,    56,
      81,   112,   104,   158,   194,   180,    57,    87,    58,    78,
      59,    88,   231,    79,    60,   113,   222,   333,   292,    61,
     153,    77,    62,   121,   122,   123,   124
};

  /* YYTABLEYYPACT[STATE-NUM] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      69,    69,   228,   156,    76,    63,   294,    65,   317,   271,
      74,    73,   272,    86,    80,    49,   134,   168,    91,    70,
      72,   237,   103,   101,   102,    89,   105,   106,   107,   108,
     109,   110,   111,   134,   169,   114,   115,   116,   120,   125,
     143,   144,   238,   173,   170,   127,   322,   223,   150,     8,
     242,   233,    99,   174,     1,   141,   142,   143,   144,   100,
     243,   147,   148,   273,   359,   148,   272,     2,   126,   148,
       3,     4,    69,   267,     5,   268,   159,   152,   161,    64,
      11,   149,    12,   134,   151,   146,   135,   313,   279,   150,
     272,   155,    92,    93,   178,   225,   150,   154,   171,   172,
      33,    34,    35,    36,   140,   141,   142,   143,   144,   226,
     150,   328,   329,   157,   160,   162,   163,   167,   175,   176,
     179,   295,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   193,
      69,    49,   195,   197,   224,   198,   199,   200,   227,   201,
      43,   221,   244,    44,   230,   275,   229,   250,   263,   235,
     236,   270,   239,   240,   241,   274,   232,   245,   246,   247,
     248,   249,   335,   276,   282,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   283,   290,   293,   346,   318,
      94,    95,    96,   319,   266,   326,   332,   336,   354,    97,
     264,   265,   337,   340,   358,   344,    98,   347,   351,   341,
     352,   202,   306,   309,   288,   262,   316,   269,   234,     0,
       0,   314,     0,     0,     0,     0,     1,     0,     0,     0,
       0,   277,   278,     0,     0,     0,   280,   281,     0,     2,
       0,     0,     3,     4,   331,     0,     5,     0,     7,     8,
       9,    66,    11,     0,    12,     0,     0,     0,    15,    16,
       0,     0,   345,     0,     0,     0,     0,     0,     0,     0,
     296,    82,    83,    84,    85,    67,     0,    26,   302,   303,
     304,   305,     0,     0,     0,   301,     0,     0,     0,     0,
       0,     0,     0,    32,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    36,    37,     0,     0,     0,     0,     0,
    -159,     1,   327,     0,     0,     0,    41,    42,     0,     0,
       0,     0,    43,   339,     2,    44,     0,     3,     4,     0,
     343,     5,  -159,     7,     8,     9,    66,    11,   350,    12,
    -159,  -159,     0,    15,    16,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,  -159,     0,  -159,     0,  -159,  -159,  -159,     0,
      67,  -159,    26,  -159,  -159,  -159,  -159,     0,     0,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,    32,  -159,
    -159,  -159,  -159,  -159,  -159,    33,    34,    35,    36,    37,
    -159,  -159,  -159,  -159,     0,  -159,  -159,  -159,  -159,  -159,
    -159,    41,    42,  -159,  -159,     1,  -159,    71,  -159,  -159,
      44,  -159,   -30,     0,     0,     0,     0,     0,     2,     0,
       0,     3,     4,     0,     0,     5,     0,     7,     8,     9,
      66,   117,     0,   118,     0,     0,     0,    15,    16,   128,
     129,   130,   131,   132,   133,     0,   134,     0,     0,   135,
     136,   137,     0,     0,    67,     0,    26,     0,     0,     0,
       0,     0,     0,     0,     0,   138,   139,   140,   141,   142,
     143,   144,    32,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   -60,     1,    41,    42,     0,     0,     0,
       0,    43,     0,     0,    44,  -165,   119,     2,     0,     0,
       3,     4,     0,     0,     5,   -60,     7,     8,     9,    66,
      11,     0,    12,   -60,   -60,     0,    15,    16,     0,   -60,
     -60,   -60,   -60,   -60,   -60,   -60,     0,     0,     0,     0,
     -60,     0,     0,    67,   -60,    26,     0,     0,   -60,     0,
       0,     0,   -60,   -60,   -60,   -60,   -60,   -60,   -60,   -60,
     -60,    32,   -60,     0,     0,   -60,   -60,   -60,    33,    34,
      35,    36,    37,     0,     0,   -60,   -60,     0,   -60,   -60,
     -60,   -60,   -60,   -60,    41,    42,   -60,   -60,     0,   -60,
      75,  -159,     1,    44,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,     0,     0,     3,     4,
       0,     0,     5,  -159,     7,     8,     9,    66,    11,     0,
      12,  -159,  -159,     0,    15,    16,     0,  -159,  -159,  -159,
    -159,  -159,  -159,  -159,     0,     0,     0,     0,  -159,     0,
       0,    67,  -159,    26,     0,     0,  -159,     0,     0,     0,
    -159,  -159,  -159,     0,     0,     0,     0,     0,     0,    32,
       0,     0,     0,     0,     0,     0,    33,    34,    35,    36,
      37,     0,     0,  -159,  -159,     0,  -159,  -159,  -159,  -159,
       1,     0,    41,    42,     0,     0,     0,  -159,    68,     0,
    -159,    44,     0,     2,     0,     0,     3,     4,     0,     0,
       5,  -159,     7,     8,     9,    66,    11,     0,    12,  -159,
    -159,     0,    15,    16,     0,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,     0,     0,     0,     0,  -159,     0,     0,    67,
    -159,    26,     0,     0,  -159,     0,     0,     0,  -159,  -159,
    -159,     0,     0,     0,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,  -159,  -159,     0,  -159,  -159,  -159,    -4,     1,     0,
      41,    42,     0,     0,     0,  -159,    43,     0,  -159,    44,
       0,     2,     0,     0,     3,     4,     0,     0,     5,     6,
       7,     8,     9,    10,    11,     0,    12,     0,    13,    14,
      15,    16,     0,     0,     0,    17,    18,    19,    20,    21,
       0,     0,    22,     0,    23,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,    28,    29,    30,    31,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,    38,
      39,    40,  -148,     1,     0,     0,     0,     0,    41,    42,
       0,     0,     0,    -4,    43,     0,     2,    44,     0,     3,
       4,     0,     0,     5,     6,     7,     8,     9,    10,    11,
       0,    12,   321,    13,     0,    15,    16,     0,     0,     0,
      17,    18,    19,    20,    21,     0,     0,     0,     0,    23,
       0,     0,    24,    25,    26,     0,     0,    27,     0,     0,
       0,    29,    30,    31,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,    38,    39,  -146,     1,     0,     0,
       0,     0,     0,    41,    42,     0,     0,     0,   307,    43,
       2,     0,    44,     3,     4,     0,     0,     5,     6,     7,
       8,     9,    10,    11,     0,    12,     0,    13,     0,    15,
      16,     0,     0,     0,    17,    18,    19,    20,    21,     0,
       0,     0,     0,    23,     0,     0,    24,    25,    26,     0,
       0,    27,     0,     0,     0,    29,    30,    31,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,    38,    39,
    -143,     1,     0,     0,     0,     0,     0,    41,    42,     0,
       0,     0,   307,    43,     2,     0,    44,     3,     4,     0,
       0,     5,     6,     7,     8,     9,    10,    11,     0,    12,
       0,    13,     0,    15,    16,     0,     0,     0,    17,    18,
      19,    20,    21,     0,     0,     0,     0,    23,     0,     0,
      24,    25,    26,     0,     0,    27,     0,     0,     0,    29,
      30,    31,     0,     0,     0,     0,     0,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,    38,    39,     0,     1,     0,     0,     0,     0,
       0,    41,    42,     0,     0,     0,   307,    43,     2,     0,
      44,     3,     4,     0,     0,     5,     6,     7,     8,     9,
      10,    11,     0,    12,     0,    13,     0,    15,    16,     0,
     -49,   -49,    17,    18,    19,    20,    21,     0,     0,     0,
       0,    23,     0,     0,    24,    25,    26,     0,     0,    27,
       0,     0,     0,    29,    30,    31,     0,     0,     0,     0,
       0,     0,    32,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,    37,     0,     0,    38,    39,     0,   -49,
       1,     0,     0,     0,     0,    41,    42,     0,     0,     0,
     307,    43,     0,     2,    44,     0,     3,     4,     0,     0,
       5,     6,     7,     8,     9,    10,    11,     0,    12,     0,
      13,     0,    15,    16,     0,     0,     0,    17,    18,    19,
      20,    21,     0,     0,     0,     0,    23,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,    29,    30,
      31,     0,     0,     0,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,    38,    39,     0,     1,     0,   -52,     0,     0,     0,
      41,    42,     0,     0,     0,   307,    43,     2,     0,    44,
       3,     4,     0,     0,     5,     6,     7,     8,     9,    10,
      11,     0,    12,     0,    13,     0,    15,    16,     0,     0,
       0,    17,    18,    19,    20,    21,     0,     0,     0,     0,
      23,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,    29,    30,    31,     0,     0,     0,     0,     0,
       0,    32,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     0,     0,    38,    39,     0,     1,   -49,
       0,     0,     0,     0,    41,    42,     0,     0,     0,   307,
      43,     2,     0,    44,     3,     4,     0,     0,     5,     6,
       7,     8,     9,    10,    11,     0,    12,     0,    13,     0,
      15,    16,     0,     0,     0,    17,    18,    19,    20,    21,
       0,     0,     0,     0,    23,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,    29,    30,    31,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,    38,
      39,     0,   -52,     1,     0,     0,     0,     0,    41,    42,
       0,     0,     0,   307,    43,     0,     2,    44,     0,     3,
       4,     0,     0,     5,     6,     7,     8,     9,    10,    11,
       0,    12,     0,    13,     0,    15,    16,     0,     0,     0,
      17,    18,    19,    20,    21,     0,     0,     0,     0,    23,
       0,     0,    24,    25,    26,     0,     0,    27,     0,     0,
       0,    29,    30,    31,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,    38,    39,     0,     1,   353,     0,
       0,     0,     0,    41,    42,     0,     0,     0,   307,    43,
       2,     0,    44,     3,     4,     0,     0,     5,     6,     7,
       8,     9,    10,    11,     0,    12,     0,    13,     0,    15,
      16,     0,     0,     0,    17,    18,    19,    20,    21,     0,
       0,     0,     0,    23,     0,     0,    24,    25,    26,     0,
       0,    27,     0,     0,     0,    29,    30,    31,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,    38,    39,
       0,     1,   360,     0,     0,     0,     0,    41,    42,     0,
       0,     0,   307,    43,     2,     0,    44,     3,     4,     0,
       0,     5,     6,     7,     8,     9,    10,    11,     0,    12,
       0,    13,     0,    15,    16,     0,     0,     0,    17,    18,
      19,    20,    21,     0,     0,     0,     0,    23,     0,     0,
      24,    25,    26,     0,     0,    27,     0,     0,     0,    29,
      30,    31,     0,     0,     0,     0,     0,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,    38,    39,     0,     1,   362,     0,     0,     0,
       0,    41,    42,     0,     0,     0,   307,    43,     2,     0,
      44,     3,     4,     0,     0,     5,     6,     7,     8,     9,
      10,    11,     0,    12,     0,    13,     0,    15,    16,     0,
       0,     0,    17,    18,    19,    20,     0,     0,     0,     0,
       0,     0,     0,     0,    24,     0,    26,     0,     0,     0,
       0,     0,     0,    29,    30,    31,     0,     0,     0,     0,
       0,     0,    32,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,    37,     0,     0,     1,    39,     0,     0,
       0,     0,     0,     0,     0,    41,    42,     0,     0,     2,
       0,    43,     3,     4,    44,     0,     5,     0,     7,     8,
       9,    66,    11,     0,    12,     0,     0,     0,    15,    16,
       0,     0,     0,     1,     0,     0,     0,     0,     0,     0,
       0,    82,     0,    84,     0,    67,     2,    26,     0,     3,
       4,     0,     0,     5,     0,     7,     8,     9,    66,    11,
       0,    12,     0,    32,     0,    15,    16,     0,     0,     0,
      33,    34,    35,    36,    37,     0,     0,     0,     0,     0,
       0,     0,    67,     0,    26,     0,    41,    42,     0,     0,
       0,     0,    43,     0,     0,    44,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       1,     0,     0,    41,    42,     0,     0,     0,     0,    43,
    -159,  -159,    44,     2,     0,     0,     3,     4,     0,    90,
       5,     0,     7,     8,     9,    66,    11,     0,    12,     0,
       1,     0,    15,    16,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     2,     0,     0,     3,     4,     0,    67,
       5,    26,     7,     8,     9,    66,    11,     0,    12,     0,
     291,     0,    15,    16,     0,     0,     0,    32,     0,   300,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    67,
       0,    26,   311,     0,   312,     0,     0,     0,     0,     0,
      41,    42,     0,     0,     0,     0,    43,    32,     0,    44,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,     0,   334,     0,     0,     0,     0,     0,   338,     0,
      41,    42,     0,     0,     0,     0,    43,     0,     0,    44,
       0,   164,   348,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   356,     0,   357,     0,     0,     0,   361,   128,
     129,   130,   131,   132,   133,     0,   134,   165,   166,   135,
     136,   137,     0,     0,     0,     0,    82,     0,    84,     0,
       0,     0,     0,     0,     0,   138,   139,   140,   141,   142,
     143,   144,   128,   129,   130,   131,   132,   133,     0,   134,
       0,     0,   135,   136,   137,     0,     0,     0,     0,     0,
       0,   181,     0,     0,     0,     0,     0,   182,   138,   139,
     140,   141,   142,   143,   144,   128,   129,   130,   131,   132,
     133,     0,   134,     0,     0,   135,   136,   137,     0,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
     184,   138,   139,   140,   141,   142,   143,   144,   128,   129,
     130,   131,   132,   133,     0,   134,     0,     0,   135,   136,
     137,     0,     0,     0,     0,     0,     0,   185,     0,     0,
       0,     0,     0,   186,   138,   139,   140,   141,   142,   143,
     144,   128,   129,   130,   131,   132,   133,     0,   134,     0,
       0,   135,   136,   137,     0,     0,     0,     0,     0,     0,
     187,     0,     0,     0,     0,     0,   188,   138,   139,   140,
     141,   142,   143,   144,   128,   129,   130,   131,   132,   133,
       0,   134,     0,     0,   135,   136,   137,     0,     0,     0,
       0,     0,     0,   284,     0,     0,     0,     0,     0,     0,
     138,   139,   140,   141,   142,   143,   144,   128,   129,   130,
     131,   132,   133,     0,   134,     0,     0,   135,   136,   137,
       0,     0,     0,     0,     0,     0,   285,     0,     0,     0,
       0,     0,     0,   138,   139,   140,   141,   142,   143,   144,
     128,   129,   130,   131,   132,   133,     0,   134,     0,     0,
     135,   136,   137,     0,     0,     0,     0,     0,     0,   286,
       0,     0,     0,     0,     0,     0,   138,   139,   140,   141,
     142,   143,   144,   128,   129,   130,   131,   132,   133,     0,
     134,     0,     0,   135,   136,   137,     0,     0,     0,     0,
       0,     0,   287,     0,     0,     0,     0,     0,     0,   138,
     139,   140,   141,   142,   143,   144,   128,   129,   130,   131,
     132,   133,     0,   134,     0,     0,   135,   136,   137,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   191,   138,   139,   140,   141,   142,   143,   144,   128,
     129,   130,   131,   132,   133,     0,   134,     0,     0,   135,
     136,   137,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   138,   139,   140,   141,   142,
     143,   144,   177,   192,     0,     0,     0,     0,   128,   129,
     130,   131,   132,   133,     0,   134,     0,     0,   135,   136,
     137,   128,   129,   130,   131,   132,   133,     0,   134,     0,
       0,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,     0,     0,     0,     0,     0,     0,   138,   139,   140,
     141,   142,   143,   144,     0,     0,     0,   196,   128,   129,
     130,   131,   132,   133,     0,   134,     0,     0,   135,   136,
     137,     0,     0,     0,     0,     0,   189,   190,     0,     0,
       0,     0,     0,     0,   138,   139,   140,   141,   142,   143,
     144,   128,   129,   130,   131,   132,   133,     0,   134,     0,
       0,   135,   136,   137,   128,   129,   130,   131,     0,     0,
       0,   134,     0,     0,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,     0,     0,     0,     0,     0,     0,
     138,   139,   140,   141,   142,   143,   144
};

static const yytype_int16 yycheck[] =
{
       6,     7,   156,    79,    10,    96,   273,     4,   300,    96,
      48,     8,    99,    19,    51,     0,    69,    93,    24,     6,
       7,    43,    30,    29,    30,    51,    32,    33,    34,    35,
      36,    37,    38,    69,    54,    41,    42,    43,    44,     0,
      93,    94,    64,    54,    64,    51,   313,    98,    99,    24,
      54,    26,    26,    64,     1,    91,    92,    93,    94,    26,
      64,    67,    68,    96,   356,    71,    99,    14,    96,    75,
      17,    18,    78,    27,    21,    29,    82,    74,    86,    26,
      27,    68,    29,    69,    71,    97,    72,    96,   239,    99,
      99,    78,    56,    57,   102,    98,    99,    99,    95,    96,
      75,    76,    77,    78,    90,    91,    92,    93,    94,    98,
      99,    36,    37,    26,    46,    26,    26,    26,    54,    53,
      26,   275,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,    26,
     146,   126,    26,   102,   150,   102,   101,    99,   154,    99,
      97,    26,    48,   100,   160,   231,    99,    99,    99,   165,
     166,    53,   168,   169,   170,    26,   163,   173,   174,   175,
     176,   177,   323,    44,    26,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,    26,    26,    26,   339,    54,
      17,    18,    19,    87,   200,    86,    26,    54,   349,    26,
     197,   198,    35,    85,   355,    54,    33,    26,    85,    99,
      53,   126,   288,   289,   261,   192,   299,   201,   164,    -1,
      -1,   297,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
      -1,   237,   238,    -1,    -1,    -1,   242,   243,    -1,    14,
      -1,    -1,    17,    18,   320,    -1,    21,    -1,    23,    24,
      25,    26,    27,    -1,    29,    -1,    -1,    -1,    33,    34,
      -1,    -1,   338,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     276,    46,    47,    48,    49,    50,    -1,    52,   284,   285,
     286,   287,    -1,    -1,    -1,   282,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
       0,     1,   318,    -1,    -1,    -1,    91,    92,    -1,    -1,
      -1,    -1,    97,   329,    14,   100,    -1,    17,    18,    -1,
     336,    21,    22,    23,    24,    25,    26,    27,   344,    29,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    -1,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    -1,    -1,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     1,    96,    97,    98,    99,
     100,   101,    35,    -1,    -1,    -1,    -1,    -1,    14,    -1,
      -1,    17,    18,    -1,    -1,    21,    -1,    23,    24,    25,
      26,    27,    -1,    29,    -1,    -1,    -1,    33,    34,    62,
      63,    64,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    -1,    -1,    50,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    68,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     0,     1,    91,    92,    -1,    -1,    -1,
      -1,    97,    -1,    -1,   100,   101,   102,    14,    -1,    -1,
      17,    18,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,
      -1,    -1,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    -1,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    -1,    96,
      97,     0,     1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    30,    31,    -1,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,
      59,    60,    61,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    -1,    85,    86,    87,     0,
       1,    -1,    91,    92,    -1,    -1,    -1,    96,    97,    -1,
      99,   100,    -1,    14,    -1,    -1,    17,    18,    -1,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    29,    30,
      31,    -1,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    -1,    -1,    55,    -1,    -1,    -1,    59,    60,
      61,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    -1,    85,    86,    87,     0,     1,    -1,
      91,    92,    -1,    -1,    -1,    96,    97,    -1,    99,   100,
      -1,    14,    -1,    -1,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    -1,    31,    32,
      33,    34,    -1,    -1,    -1,    38,    39,    40,    41,    42,
      -1,    -1,    45,    -1,    47,    -1,    -1,    50,    51,    52,
      -1,    -1,    55,    -1,    -1,    58,    59,    60,    61,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    84,     0,     1,    -1,    -1,    -1,    -1,    91,    92,
      -1,    -1,    -1,    96,    97,    -1,    14,   100,    -1,    17,
      18,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    34,    -1,    -1,    -1,
      38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,
      -1,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,     0,     1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    -1,    -1,    -1,    96,    97,
      14,    -1,   100,    17,    18,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    29,    -1,    31,    -1,    33,
      34,    -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,
      -1,    55,    -1,    -1,    -1,    59,    60,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
       0,     1,    -1,    -1,    -1,    -1,    -1,    91,    92,    -1,
      -1,    -1,    96,    97,    14,    -1,   100,    17,    18,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    29,
      -1,    31,    -1,    33,    34,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,    59,
      60,    61,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    -1,     1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    -1,    -1,    -1,    96,    97,    14,    -1,
     100,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    -1,    31,    -1,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,
      -1,    -1,    -1,    59,    60,    61,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    -1,    85,
       1,    -1,    -1,    -1,    -1,    91,    92,    -1,    -1,    -1,
      96,    97,    -1,    14,   100,    -1,    17,    18,    -1,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    29,    -1,
      31,    -1,    33,    34,    -1,    -1,    -1,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    -1,    -1,    55,    -1,    -1,    -1,    59,    60,
      61,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    -1,     1,    -1,    87,    -1,    -1,    -1,
      91,    92,    -1,    -1,    -1,    96,    97,    14,    -1,   100,
      17,    18,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    -1,    31,    -1,    33,    34,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,
      -1,    -1,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    -1,     1,    86,
      -1,    -1,    -1,    -1,    91,    92,    -1,    -1,    -1,    96,
      97,    14,    -1,   100,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    -1,    31,    -1,
      33,    34,    -1,    -1,    -1,    38,    39,    40,    41,    42,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,
      -1,    -1,    55,    -1,    -1,    -1,    59,    60,    61,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    -1,    85,     1,    -1,    -1,    -1,    -1,    91,    92,
      -1,    -1,    -1,    96,    97,    -1,    14,   100,    -1,    17,
      18,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      -1,    29,    -1,    31,    -1,    33,    34,    -1,    -1,    -1,
      38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,
      -1,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    -1,     1,    86,    -1,
      -1,    -1,    -1,    91,    92,    -1,    -1,    -1,    96,    97,
      14,    -1,   100,    17,    18,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    29,    -1,    31,    -1,    33,
      34,    -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,
      -1,    55,    -1,    -1,    -1,    59,    60,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      -1,     1,    86,    -1,    -1,    -1,    -1,    91,    92,    -1,
      -1,    -1,    96,    97,    14,    -1,   100,    17,    18,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    29,
      -1,    31,    -1,    33,    34,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,    59,
      60,    61,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    -1,     1,    86,    -1,    -1,    -1,
      -1,    91,    92,    -1,    -1,    -1,    96,    97,    14,    -1,
     100,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    -1,    31,    -1,    33,    34,    -1,
      -1,    -1,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,    61,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    -1,    -1,     1,    83,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    92,    -1,    -1,    14,
      -1,    97,    17,    18,   100,    -1,    21,    -1,    23,    24,
      25,    26,    27,    -1,    29,    -1,    -1,    -1,    33,    34,
      -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    -1,    48,    -1,    50,    14,    52,    -1,    17,
      18,    -1,    -1,    21,    -1,    23,    24,    25,    26,    27,
      -1,    29,    -1,    68,    -1,    33,    34,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    52,    -1,    91,    92,    -1,    -1,
      -1,    -1,    97,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,    91,    92,    -1,    -1,    -1,    -1,    97,
      98,    99,   100,    14,    -1,    -1,    17,    18,    -1,    20,
      21,    -1,    23,    24,    25,    26,    27,    -1,    29,    -1,
       1,    -1,    33,    34,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    -1,    -1,    17,    18,    -1,    50,
      21,    52,    23,    24,    25,    26,    27,    -1,    29,    -1,
     270,    -1,    33,    34,    -1,    -1,    -1,    68,    -1,   279,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    50,
      -1,    52,   292,    -1,   294,    -1,    -1,    -1,    -1,    -1,
      91,    92,    -1,    -1,    -1,    -1,    97,    68,    -1,   100,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    -1,   322,    -1,    -1,    -1,    -1,    -1,   328,    -1,
      91,    92,    -1,    -1,    -1,    -1,    97,    -1,    -1,   100,
      -1,    44,   342,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   352,    -1,   354,    -1,    -1,    -1,   358,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    -1,    46,    -1,    48,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    62,    63,    64,    65,    66,    67,    -1,    69,
      -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    -1,    -1,    -1,    54,    88,    89,
      90,    91,    92,    93,    94,    62,    63,    64,    65,    66,
      67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,
      54,    88,    89,    90,    91,    92,    93,    94,    62,    63,
      64,    65,    66,    67,    -1,    69,    -1,    -1,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    54,    88,    89,    90,    91,    92,    93,
      94,    62,    63,    64,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    -1,    -1,    -1,    54,    88,    89,    90,
      91,    92,    93,    94,    62,    63,    64,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    62,    63,    64,
      65,    66,    67,    -1,    69,    -1,    -1,    72,    73,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      62,    63,    64,    65,    66,    67,    -1,    69,    -1,    -1,
      72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    62,    63,    64,    65,    66,    67,    -1,
      69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    62,    63,    64,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    88,    89,    90,    91,    92,    93,    94,    62,
      63,    64,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    56,    96,    -1,    -1,    -1,    -1,    62,    63,
      64,    65,    66,    67,    -1,    69,    -1,    -1,    72,    73,
      74,    62,    63,    64,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    88,    89,    90,    91,    92,    93,
      94,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    98,    62,    63,
      64,    65,    66,    67,    -1,    69,    -1,    -1,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    62,    63,    64,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    -1,    -1,    72,    73,    74,    88,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94
};

  /* YYSTOSSTATE-NUM -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    14,    17,    18,    21,    22,    23,    24,    25,
      26,    27,    29,    31,    32,    33,    34,    38,    39,    40,
      41,    42,    45,    47,    50,    51,    52,    55,    58,    59,
      60,    61,    68,    75,    76,    77,    78,    79,    82,    83,
      84,    91,    92,    97,   100,   104,   105,   106,   107,   108,
     119,   125,   128,   129,   130,   131,   132,   139,   141,   143,
     147,   152,   155,    96,    26,   128,    26,    50,    97,   129,
     153,    97,   153,   128,    48,    97,   129,   154,   142,   146,
      51,   133,    46,    47,    48,    49,   129,   140,   144,    51,
      20,   129,    56,    57,    17,    18,    19,    26,    33,    26,
      26,   129,   129,   140,   135,   129,   129,   129,   129,   129,
     129,   129,   134,   148,   129,   129,   129,    27,    29,   102,
     129,   156,   157,   158,   159,     0,    96,   129,    62,    63,
      64,    65,    66,    67,    69,    72,    73,    74,    88,    89,
      90,    91,    92,    93,    94,   126,    97,   129,   129,   153,
      99,   153,   128,   153,    99,   153,   126,    26,   136,   129,
      46,   140,    26,    26,    44,    70,    71,    26,   126,    54,
      64,   128,   128,    54,    64,    54,    53,    56,   140,    26,
     138,    48,    54,    48,    54,    48,    54,    48,    54,    80,
      81,    54,    96,    26,   137,    26,    98,   102,   102,   101,
      99,    99,   104,   122,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,    26,   149,    98,   129,    98,    98,   129,   149,    99,
     129,   145,   128,    26,   131,   129,   129,    43,    64,   129,
     129,   129,    54,    64,    48,   129,   129,   129,   129,   129,
      99,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   118,   118,    99,   128,   128,   129,    27,    29,   159,
      53,    96,    99,    96,    26,   126,    44,   129,   129,   122,
     129,   129,    26,    26,    48,    48,    48,    48,   107,   127,
      26,   127,   151,    26,   151,   149,   129,   115,   109,   112,
     127,   128,   129,   129,   129,   129,   126,    96,   108,   126,
     123,   127,   127,    96,   126,   124,   124,   123,    54,    87,
     120,    30,   151,   116,   110,   113,    86,   129,    36,    37,
     121,   126,    26,   150,   127,   122,    54,    35,   127,   129,
      85,    99,   117,   129,    54,   126,   122,    26,   127,   111,
     129,    85,    53,    86,   122,   114,   127,   127,   122,   123,
      86,   127,    86
};

  /* YYR1YYN -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   103,   104,   104,   105,   105,   105,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     107,   107,   107,   108,   108,   108,   109,   110,   111,   108,
     112,   113,   114,   108,   115,   116,   117,   108,   108,   108,
     108,   108,   118,   119,   119,   120,   120,   121,   122,   123,
     124,   125,   126,   127,   127,   127,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   130,   130,   130,
     130,   130,   130,   130,   130,   131,   131,   132,   132,   132,
     132,   132,   133,   132,   134,   132,   135,   132,   132,   132,
     132,   132,   136,   136,   137,   137,   138,   138,   139,   139,
     139,   139,   139,   139,   140,   140,   141,   141,   141,   142,
     141,   144,   145,   143,   143,   146,   143,   143,   143,   148,
     147,   149,   149,   149,   150,   150,   150,   151,   152,   153,
     153,   153,   154,   154,   155,   156,   156,   156,   156,   157,
     157,   158,   158,   159,   159
};

  /* YYR2YYN -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     4,     4,     6,
       4,     4,     4,     4,     4,     4,     5,     5,     8,     4,
       1,     1,     1,     1,     1,     8,     0,     0,     0,    14,
       0,     0,     0,    15,     0,     0,     0,    12,     2,     4,
       7,     6,     0,     9,    11,     0,     2,     6,     0,     0,
       0,     1,     0,     0,     2,     2,     1,     1,     1,     1,
       1,     1,     2,     3,     1,     2,     1,     1,     4,     2,
       4,     3,     1,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     2,     2,     4,     4,     4,     6,     4,
       6,     4,     6,     4,     6,     2,     1,     2,     1,     1,
       2,     1,     0,     3,     0,     3,     0,     3,     4,     2,
       4,     2,     1,     3,     1,     3,     1,     3,     1,     2,
       2,     2,     3,     2,     3,     2,     2,     3,     2,     0,
       3,     0,     0,     9,     2,     0,     7,     8,     6,     0,
       3,     0,     1,     3,     0,     1,     3,     0,     2,     0,
       1,     3,     1,     3,     3,     0,     1,     1,     1,     1,
       3,     1,     3,     3,     3
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
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1934 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* FBLTIN  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1940 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* RBLTIN  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1946 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* THEFBLTIN  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1952 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* ID  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1958 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* STRING  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1964 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* HANDLER  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1970 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* SYMBOL  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1976 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 30: /* ENDCLAUSE  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1982 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 31: /* tPLAYACCEL  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1988 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 32: /* tMETHOD  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1994 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 33: /* THEOBJECTFIELD  */
#line 180 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).os; }
#line 2000 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 147: /* on  */
#line 179 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2006 "engines/director/lingo/lingo-gr.cpp"
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
#line 191 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarLocal);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2293 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 198 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2301 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 202 "engines/director/lingo/lingo-gr.y"
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
#line 2317 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 213 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_after); }
#line 2323 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 214 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_before); }
#line 2329 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 215 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), kVarLocal);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2341 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 222 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2353 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 229 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), kVarLocal);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2365 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 236 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2377 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 243 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 249 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2399 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 256 "engines/director/lingo/lingo-gr.y"
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
#line 2415 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 267 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).oe);
		delete (yyvsp[-2].objectfield).os;
		(yyval.code) = (yyvsp[0].code); }
#line 2426 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25:
#line 284 "engines/director/lingo/lingo-gr.y"
                                                                                {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-5].code) - (yyvsp[-1].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code) + 2);
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentScript)[(yyvsp[-1].code)] = start; }
#line 2437 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26:
#line 296 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), kVarLocal); }
#line 2445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27:
#line 300 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2452 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28:
#line 303 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_le); }
#line 2458 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 303 "engines/director/lingo/lingo-gr.y"
                                                                                          {

		g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[-11].s)->c_str());
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_add);
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-11].s)->c_str());
		g_lingo->code1(LC::c_assign);
		g_lingo->code2(LC::c_jump, 0);
		int pos = g_lingo->_currentScript->size() - 1;

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-7].code) - pos + 2);
		WRITE_UINT32(&end, pos - (yyvsp[-2].code) + 2);
		(*g_lingo->_currentScript)[pos] = loop;		/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-2].code)] = end;	}
#line 2481 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 327 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), kVarLocal); }
#line 2489 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 331 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2496 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 334 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_ge); }
#line 2502 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 335 "engines/director/lingo/lingo-gr.y"
                                                    {

		g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[-12].s)->c_str());
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_sub);
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-12].s)->c_str());
		g_lingo->code1(LC::c_assign);
		g_lingo->code2(LC::c_jump, 0);
		int pos = g_lingo->_currentScript->size() - 1;

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-8].code) - pos + 2);
		WRITE_UINT32(&end, pos - (yyvsp[-2].code) + 2);
		(*g_lingo->_currentScript)[pos] = loop;		/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-2].code)] = end;	}
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 360 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  Common::String count("count");
				  g_lingo->codeFunc(&count, 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
#line 2536 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 367 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
#line 2546 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 373 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get list
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(1);
				  Common::String getAt("getAt");
				  g_lingo->codeFunc(&getAt, 2);
				  g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-6].s)->c_str());
				  mVar((yyvsp[-6].s), kVarLocal);
				  g_lingo->code1(LC::c_assign); }
#line 2561 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 383 "engines/director/lingo/lingo-gr.y"
                                            {

		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_add);			// Increment counter

		int jump = g_lingo->code2(LC::c_jump, 0);

		int end2 = g_lingo->code1(LC::c_stackdrop);	// remove list, size, counter
		g_lingo->codeInt(3);

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-5].code) - jump);
		WRITE_UINT32(&end, end2 - (yyvsp[-3].code) + 1);

		(*g_lingo->_currentScript)[jump + 1] = loop;		/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = end;	}
#line 2583 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 401 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_nextRepeat); }
#line 2590 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 403 "engines/director/lingo/lingo-gr.y"
                              {
		g_lingo->code1(LC::c_whencode);
		g_lingo->codeString((yyvsp[-2].s)->c_str()); }
#line 2598 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 406 "engines/director/lingo/lingo-gr.y"
                                                          { g_lingo->code1(LC::c_telldone); }
#line 2604 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 407 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->code1(LC::c_telldone); }
#line 2610 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 409 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_tell); }
#line 2616 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 411 "engines/director/lingo/lingo-gr.y"
                                                                                         {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2628 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 418 "engines/director/lingo/lingo-gr.y"
                                                                                                          {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].code) + 1 - (yyvsp[-8].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-8].code)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-5].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-5].code), (yyvsp[-1].code)); }
#line 2640 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 429 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2650 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 435 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, 0);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2658 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 439 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, 0);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2666 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 443 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2674 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 447 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2681 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 450 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2687 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 452 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2693 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 456 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2701 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 459 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 462 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2718 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 466 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2727 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 470 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2736 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 474 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 482 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2760 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 488 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2766 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 490 "engines/director/lingo/lingo-gr.y"
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
#line 2781 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 501 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2787 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 503 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 506 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2803 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 509 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2811 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 512 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2819 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 515 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).oe);
		delete (yyvsp[0].objectfield).os; }
#line 2829 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 520 "engines/director/lingo/lingo-gr.y"
                       {
		g_lingo->code1(LC::c_objectrefpush);
		g_lingo->codeString((yyvsp[0].objectref).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectref).field->c_str());
		delete (yyvsp[0].objectref).obj;
		delete (yyvsp[0].objectref).field; }
#line 2840 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 527 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2846 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 528 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2852 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 529 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2858 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 530 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2864 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 531 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2870 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 532 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2876 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 533 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2882 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 534 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2888 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 535 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2894 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 536 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2900 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 537 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2906 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 538 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2912 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 539 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2918 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 540 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2924 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 541 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2930 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 542 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2936 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 543 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2942 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 544 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2948 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 545 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); }
#line 2954 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 546 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2960 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 547 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_intersects); }
#line 2966 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 548 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 2972 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 550 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charOf); }
#line 2978 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 551 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charToOf); }
#line 2984 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 552 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemOf); }
#line 2990 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 553 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_itemToOf); }
#line 2996 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 554 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineOf); }
#line 3002 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 555 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_lineToOf); }
#line 3008 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 556 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordOf); }
#line 3014 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 557 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_wordToOf); }
#line 3020 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 559 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 3028 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 564 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_printtop); }
#line 3034 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 567 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_exitRepeat); }
#line 3040 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 568 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 3046 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 569 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3052 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 569 "engines/director/lingo/lingo-gr.y"
                                                                                 { inNone(); }
#line 3058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 570 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3064 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 570 "engines/director/lingo/lingo-gr.y"
                                                                                   { inNone(); }
#line 3070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 571 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3076 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 571 "engines/director/lingo/lingo-gr.y"
                                                                                   { inNone(); }
#line 3082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 572 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3090 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 575 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3098 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 578 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_open); }
#line 3104 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 579 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 3110 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 581 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3120 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 586 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3130 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 592 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3140 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 597 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3150 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 603 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3158 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 606 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3166 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 617 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 618 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3178 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 619 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3184 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 620 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3193 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 624 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3202 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 628 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3211 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 636 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 640 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3229 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 644 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3238 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 648 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3244 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 648 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3253 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 678 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 678 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->_currentFactory = NULL; }
#line 3265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 679 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-6].s); }
#line 3275 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 684 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3281 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 685 "engines/director/lingo/lingo-gr.y"
                  { startDef(); }
#line 3287 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 686 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, g_lingo->_currentFactory);
		endDef();
		delete (yyvsp[-6].s); }
#line 3297 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 691 "engines/director/lingo/lingo-gr.y"
                                                                   {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3310 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 699 "engines/director/lingo/lingo-gr.y"
                                               {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3320 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 705 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3326 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 705 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.s) = (yyvsp[0].s); g_lingo->_currentFactory = NULL; g_lingo->_ignoreMe = true; }
#line 3333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 708 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 709 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 710 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3351 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 713 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3357 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 714 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3363 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 716 "engines/director/lingo/lingo-gr.y"
                                        { inDef(); }
#line 3369 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 718 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 3381 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 726 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3387 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 727 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3393 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 728 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3399 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 730 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3405 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 731 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3411 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 733 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3417 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 735 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3423 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 736 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3429 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 737 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3435 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 738 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 740 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3447 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 741 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3453 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 743 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3459 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172:
#line 744 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3465 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173:
#line 746 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3474 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174:
#line 750 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3483 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3487 "engines/director/lingo/lingo-gr.cpp"

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

#line 756 "engines/director/lingo/lingo-gr.y"


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
