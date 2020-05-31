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
	g_lingo->_methodVars.clear();
}

static void endDef() {
	g_lingo->clearArgStack();
	inNone();
	g_lingo->_ignoreMe = false;

	g_lingo->_methodVars.clear();
}

static void mArg(Common::String *s, VarType type) {
	if (!g_lingo->_methodVars.contains(*s))
		g_lingo->_methodVars[*s] = type;
}


#line 128 "engines/director/lingo/lingo-gr.cpp"

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
    tNLELSE = 313,                 /* tNLELSE  */
    tFACTORY = 314,                /* tFACTORY  */
    tOPEN = 315,                   /* tOPEN  */
    tPLAY = 316,                   /* tPLAY  */
    tINSTANCE = 317,               /* tINSTANCE  */
    tGE = 318,                     /* tGE  */
    tLE = 319,                     /* tLE  */
    tEQ = 320,                     /* tEQ  */
    tNEQ = 321,                    /* tNEQ  */
    tAND = 322,                    /* tAND  */
    tOR = 323,                     /* tOR  */
    tNOT = 324,                    /* tNOT  */
    tMOD = 325,                    /* tMOD  */
    tAFTER = 326,                  /* tAFTER  */
    tBEFORE = 327,                 /* tBEFORE  */
    tCONCAT = 328,                 /* tCONCAT  */
    tCONTAINS = 329,               /* tCONTAINS  */
    tSTARTS = 330,                 /* tSTARTS  */
    tCHAR = 331,                   /* tCHAR  */
    tITEM = 332,                   /* tITEM  */
    tLINE = 333,                   /* tLINE  */
    tWORD = 334,                   /* tWORD  */
    tSPRITE = 335,                 /* tSPRITE  */
    tINTERSECTS = 336,             /* tINTERSECTS  */
    tWITHIN = 337,                 /* tWITHIN  */
    tTELL = 338,                   /* tTELL  */
    tPROPERTY = 339,               /* tPROPERTY  */
    tON = 340,                     /* tON  */
    tENDIF = 341,                  /* tENDIF  */
    tENDREPEAT = 342,              /* tENDREPEAT  */
    tENDTELL = 343                 /* tENDTELL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 111 "engines/director/lingo/lingo-gr.y"

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

#line 286 "engines/director/lingo/lingo-gr.cpp"

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
  YYSYMBOL_tNLELSE = 58,                   /* tNLELSE  */
  YYSYMBOL_tFACTORY = 59,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 60,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 61,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 62,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 63,                       /* tGE  */
  YYSYMBOL_tLE = 64,                       /* tLE  */
  YYSYMBOL_tEQ = 65,                       /* tEQ  */
  YYSYMBOL_tNEQ = 66,                      /* tNEQ  */
  YYSYMBOL_tAND = 67,                      /* tAND  */
  YYSYMBOL_tOR = 68,                       /* tOR  */
  YYSYMBOL_tNOT = 69,                      /* tNOT  */
  YYSYMBOL_tMOD = 70,                      /* tMOD  */
  YYSYMBOL_tAFTER = 71,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 72,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 73,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 74,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 75,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 76,                     /* tCHAR  */
  YYSYMBOL_tITEM = 77,                     /* tITEM  */
  YYSYMBOL_tLINE = 78,                     /* tLINE  */
  YYSYMBOL_tWORD = 79,                     /* tWORD  */
  YYSYMBOL_tSPRITE = 80,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 81,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 82,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 83,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 84,                 /* tPROPERTY  */
  YYSYMBOL_tON = 85,                       /* tON  */
  YYSYMBOL_tENDIF = 86,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 87,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 88,                  /* tENDTELL  */
  YYSYMBOL_89_ = 89,                       /* '<'  */
  YYSYMBOL_90_ = 90,                       /* '>'  */
  YYSYMBOL_91_ = 91,                       /* '&'  */
  YYSYMBOL_92_ = 92,                       /* '+'  */
  YYSYMBOL_93_ = 93,                       /* '-'  */
  YYSYMBOL_94_ = 94,                       /* '*'  */
  YYSYMBOL_95_ = 95,                       /* '/'  */
  YYSYMBOL_96_ = 96,                       /* '%'  */
  YYSYMBOL_97_n_ = 97,                     /* '\n'  */
  YYSYMBOL_98_ = 98,                       /* '('  */
  YYSYMBOL_99_ = 99,                       /* ')'  */
  YYSYMBOL_100_ = 100,                     /* ','  */
  YYSYMBOL_101_ = 101,                     /* '['  */
  YYSYMBOL_102_ = 102,                     /* ']'  */
  YYSYMBOL_103_ = 103,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 104,                 /* $accept  */
  YYSYMBOL_program = 105,                  /* program  */
  YYSYMBOL_programline = 106,              /* programline  */
  YYSYMBOL_asgn = 107,                     /* asgn  */
  YYSYMBOL_stmtoneliner = 108,             /* stmtoneliner  */
  YYSYMBOL_stmtonelinerwithif = 109,       /* stmtonelinerwithif  */
  YYSYMBOL_stmt = 110,                     /* stmt  */
  YYSYMBOL_111_1 = 111,                    /* $@1  */
  YYSYMBOL_112_2 = 112,                    /* $@2  */
  YYSYMBOL_113_3 = 113,                    /* $@3  */
  YYSYMBOL_114_4 = 114,                    /* $@4  */
  YYSYMBOL_115_5 = 115,                    /* $@5  */
  YYSYMBOL_116_6 = 116,                    /* $@6  */
  YYSYMBOL_117_7 = 117,                    /* $@7  */
  YYSYMBOL_118_8 = 118,                    /* $@8  */
  YYSYMBOL_119_9 = 119,                    /* $@9  */
  YYSYMBOL_tellstart = 120,                /* tellstart  */
  YYSYMBOL_ifstmt = 121,                   /* ifstmt  */
  YYSYMBOL_elseifstmtlist = 122,           /* elseifstmtlist  */
  YYSYMBOL_elseifstmt = 123,               /* elseifstmt  */
  YYSYMBOL_ifoneliner = 124,               /* ifoneliner  */
  YYSYMBOL_jumpifz = 125,                  /* jumpifz  */
  YYSYMBOL_jump = 126,                     /* jump  */
  YYSYMBOL_varassign = 127,                /* varassign  */
  YYSYMBOL_if = 128,                       /* if  */
  YYSYMBOL_begin = 129,                    /* begin  */
  YYSYMBOL_end = 130,                      /* end  */
  YYSYMBOL_stmtlist = 131,                 /* stmtlist  */
  YYSYMBOL_when = 132,                     /* when  */
  YYSYMBOL_simpleexpr = 133,               /* simpleexpr  */
  YYSYMBOL_expr = 134,                     /* expr  */
  YYSYMBOL_chunkexpr = 135,                /* chunkexpr  */
  YYSYMBOL_reference = 136,                /* reference  */
  YYSYMBOL_proc = 137,                     /* proc  */
  YYSYMBOL_138_10 = 138,                   /* $@10  */
  YYSYMBOL_139_11 = 139,                   /* $@11  */
  YYSYMBOL_140_12 = 140,                   /* $@12  */
  YYSYMBOL_globallist = 141,               /* globallist  */
  YYSYMBOL_propertylist = 142,             /* propertylist  */
  YYSYMBOL_instancelist = 143,             /* instancelist  */
  YYSYMBOL_gotofunc = 144,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 145,                /* gotomovie  */
  YYSYMBOL_playfunc = 146,                 /* playfunc  */
  YYSYMBOL_147_13 = 147,                   /* $@13  */
  YYSYMBOL_defn = 148,                     /* defn  */
  YYSYMBOL_149_14 = 149,                   /* $@14  */
  YYSYMBOL_150_15 = 150,                   /* $@15  */
  YYSYMBOL_151_16 = 151,                   /* $@16  */
  YYSYMBOL_on = 152,                       /* on  */
  YYSYMBOL_153_17 = 153,                   /* $@17  */
  YYSYMBOL_argdef = 154,                   /* argdef  */
  YYSYMBOL_endargdef = 155,                /* endargdef  */
  YYSYMBOL_argstore = 156,                 /* argstore  */
  YYSYMBOL_macro = 157,                    /* macro  */
  YYSYMBOL_arglist = 158,                  /* arglist  */
  YYSYMBOL_nonemptyarglist = 159,          /* nonemptyarglist  */
  YYSYMBOL_list = 160,                     /* list  */
  YYSYMBOL_valuelist = 161,                /* valuelist  */
  YYSYMBOL_linearlist = 162,               /* linearlist  */
  YYSYMBOL_proplist = 163,                 /* proplist  */
  YYSYMBOL_proppair = 164                  /* proppair  */
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
#define YYFINAL  126
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2656

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  61
/* YYNRULES -- Number of rules.  */
#define YYNRULES  182
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  383

#define YYMAXUTOK   343


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
      97,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    96,    91,     2,
      98,    99,    94,    92,   100,    93,     2,    95,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   103,     2,
      89,     2,    90,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   101,     2,   102,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88
};

#if YYDEBUG
  /* YYRLINEYYN -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   167,   167,   168,   170,   171,   172,   174,   181,   185,
     196,   197,   198,   205,   212,   219,   226,   232,   239,   250,
     257,   258,   259,   261,   262,   263,   264,   266,   267,   272,
     284,   288,   291,   283,   315,   319,   322,   314,   348,   354,
     360,   347,   387,   389,   394,   398,   403,   407,   414,   422,
     423,   425,   431,   437,   443,   447,   451,   455,   458,   460,
     462,   463,   464,   466,   472,   475,   478,   482,   486,   490,
     498,   499,   500,   511,   512,   513,   516,   519,   522,   525,
     531,   536,   542,   543,   544,   545,   546,   547,   548,   549,
     550,   551,   552,   553,   554,   555,   556,   557,   558,   559,
     560,   561,   562,   563,   564,   566,   567,   568,   569,   570,
     571,   572,   573,   575,   578,   580,   581,   582,   583,   584,
     585,   585,   586,   586,   587,   587,   588,   591,   594,   595,
     597,   602,   608,   613,   619,   624,   637,   638,   639,   640,
     644,   648,   653,   654,   656,   660,   664,   668,   668,   698,
     698,   698,   704,   705,   705,   711,   719,   725,   725,   728,
     729,   730,   732,   733,   734,   736,   738,   746,   747,   748,
     750,   751,   753,   755,   756,   757,   758,   760,   761,   763,
     764,   766,   770
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
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tNLELSE",
  "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tENDIF",
  "tENDREPEAT", "tENDTELL", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'\\n'", "'('", "')'", "','", "'['", "']'", "':'",
  "$accept", "program", "programline", "asgn", "stmtoneliner",
  "stmtonelinerwithif", "stmt", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6",
  "$@7", "$@8", "$@9", "tellstart", "ifstmt", "elseifstmtlist",
  "elseifstmt", "ifoneliner", "jumpifz", "jump", "varassign", "if",
  "begin", "end", "stmtlist", "when", "simpleexpr", "expr", "chunkexpr",
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
     335,   336,   337,   338,   339,   340,   341,   342,   343,    60,
      62,    38,    43,    45,    42,    47,    37,    10,    40,    41,
      44,    91,    93,    58
};
#endif

#define YYPACT_NINF (-278)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-174)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACTSTATE-NUM -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     788,   -92,  -278,  -278,    55,  -278,   610,   326,    55,   -33,
     511,  -278,  -278,  -278,  -278,  -278,  -278,    -8,  -278,  1890,
    -278,  -278,  -278,    -1,  2046,   -47,    35,    29,    33,  2128,
    1927,  -278,  2128,  2128,  2128,  2128,  2128,  2128,  2128,  -278,
    -278,  2128,  2128,  2128,   421,    74,    24,  -278,  -278,  -278,
    -278,  2128,  1726,  -278,  2548,  -278,  -278,  -278,  -278,  -278,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,    17,  2046,  2009,
    2548,    23,  2009,    23,  -278,    55,  2009,  2548,    26,   699,
    -278,  -278,   101,  2128,  -278,    82,  -278,  2120,  -278,   122,
    -278,   123,   393,   124,  -278,    -7,    55,    55,    40,    98,
     104,  -278,  2445,  2120,  -278,   128,  -278,  2168,  2201,  2234,
    2267,  2515,  2432,   132,   133,  -278,  -278,  2478,    61,    62,
    -278,  2548,    58,    68,    72,  -278,  -278,   788,  2548,  -278,
    -278,  2128,  2548,  -278,  -278,  2128,  2128,  2128,  2128,  2128,
    2128,  2128,  2128,  2128,  2128,  2128,  2128,  2128,  2128,  2128,
    2128,  2128,   151,  2009,   393,  2478,   -35,  2128,   -20,  -278,
      18,  2128,    23,   151,  -278,    79,  2548,  2128,  -278,  -278,
      55,    25,  2128,  2128,   -31,  2128,  2128,  2128,    57,   144,
    2128,  2128,  2128,  -278,  2128,  -278,  -278,    80,  2128,  2128,
    2128,  2128,  2128,  2128,  2128,  2128,  2128,  2128,  -278,  -278,
    -278,    94,  -278,  -278,    55,    55,  -278,  2128,    85,  -278,
     142,  -278,  2548,    15,    15,    15,    15,  2561,  2561,  -278,
      -3,    15,    15,    15,    15,    -3,   -53,   -53,  -278,  -278,
    -278,   -75,  -278,  2548,  -278,  -278,  2548,   -40,   170,  2548,
    -278,   153,  -278,  -278,  2548,  2548,  2128,  2128,  2548,  2548,
      15,  2128,  2128,   172,  2548,    15,  2548,  2548,   173,  2548,
    2300,  2548,  2333,  2548,  2366,  2548,  2399,  2548,  2548,  1808,
    -278,   177,  -278,  -278,  2548,    61,    62,  -278,  -278,   152,
    -278,   178,  -278,  -278,   151,  2128,  2548,   240,  -278,  2548,
      15,    55,  -278,  2128,  2128,  2128,  2128,  -278,  1215,  -278,
    1129,  1808,  -278,  -278,  -278,    16,  2548,  -278,  -278,  -278,
    1300,   155,  2548,  2548,  2548,  2548,  -278,  -278,  -278,   118,
    -278,   125,   874,   959,  -278,  -278,  -278,  -278,   126,  2128,
    -278,    60,   174,   129,   188,  -278,  -278,   162,   182,  -278,
    2548,  -278,  2128,  -278,   134,  -278,  -278,  -278,   119,  1044,
    -278,  2128,   164,  -278,  2548,  -278,  1808,   195,  -278,  2548,
    2128,  1385,   169,  -278,  -278,  1471,  -278,  2548,   137,  -278,
     138,  -278,  -278,  -278,  -278,  1129,  -278,  1556,  -278,  -278,
    -278,  1641,  -278
};

  /* YYDEFACTSTATE-NUM -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    64,    69,     0,    65,     0,     0,     0,     0,
       0,    67,    66,   147,   153,    80,    81,   119,   120,     0,
     136,    57,   149,     0,     0,     0,     0,     0,     0,     0,
       0,   124,     0,     0,     0,     0,     0,     0,     0,   122,
     157,     0,     0,     0,     0,     0,     2,    82,    27,     6,
      28,     0,     0,    73,    21,   114,    74,    22,   116,   117,
       5,    58,    20,    71,    72,    68,    79,    68,     0,     0,
     168,   127,     0,    76,   113,     0,     0,   170,   166,     0,
      58,   118,     0,     0,   137,     0,   138,   139,   141,     0,
      42,     0,   115,     0,    58,     0,     0,     0,     0,     0,
       0,   152,   129,   144,   146,     0,    96,     0,     0,     0,
       0,     0,     0,     0,     0,   101,   102,     0,    67,    66,
     174,   177,     0,   175,   176,   179,     1,     0,    54,    59,
      26,     0,    24,    25,    23,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   159,     0,     0,   168,     0,     0,     0,    78,
       0,     0,   148,   159,   130,   121,   143,     0,   140,   150,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    63,     0,   145,   134,   125,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    46,    46,
     132,   123,   158,    70,     0,     0,   172,     0,     0,     3,
       0,    43,    54,    92,    93,    90,    91,    94,    95,    87,
      98,    99,   100,    89,    88,    97,    83,    84,    85,    86,
     160,     0,   126,   169,    75,    77,   171,     0,     0,   142,
      58,     0,     7,     8,    10,    11,     0,     0,    54,    15,
      13,     0,     0,     0,    14,    12,    19,   128,     0,   105,
       0,   107,     0,   109,     0,   111,     0,   103,   104,     0,
      60,     0,   182,   181,   178,     0,     0,   180,    60,     0,
     165,     0,   165,   131,   159,     0,    38,    30,    60,    16,
      17,     0,   135,     0,     0,     0,     0,    59,     0,   133,
       0,     0,    60,   161,    60,     0,     9,    58,    56,    56,
       0,     0,   106,   108,   110,   112,    45,    61,    62,     0,
      49,    55,     0,     0,   165,    39,    31,    35,     0,     0,
      44,    58,     0,     0,   162,    60,    54,     0,     0,    29,
      18,    58,     0,    50,     0,    58,    53,   163,   155,     0,
      40,     0,     0,    60,    54,    47,     0,     0,    60,    32,
       0,     0,     0,    58,   164,     0,    54,    36,     0,    60,
       0,    41,    60,    54,    48,     0,    52,     0,    60,    51,
      33,     0,    37
};

  /* YYPGOTONTERM-NUM.  */
static const yytype_int16 yypgoto[] =
{
    -278,   100,  -278,  -278,  -261,  -278,    19,  -278,  -278,  -278,
    -278,  -278,  -278,  -278,  -278,  -278,    31,  -278,  -278,  -278,
    -278,  -173,  -277,   -84,   179,   -78,  -178,   -76,  -278,     3,
      -6,  -278,    63,   180,  -278,  -278,  -278,  -278,  -278,  -278,
    -278,   -16,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,
    -160,  -278,  -276,   181,    14,  -278,  -278,  -278,  -278,  -278,
      27
};

  /* YYDEFGOTONTERM-NUM.  */
static const yytype_int16 yydefgoto[] =
{
      -1,    45,    46,    47,    48,   129,   318,   308,   337,   366,
     309,   338,   373,   307,   336,   358,   269,    50,   331,   343,
     130,   210,   320,   326,    51,   152,   211,   298,    52,    53,
      54,    55,    56,    57,    82,   113,   105,   165,   201,   187,
      58,    88,    59,    79,    60,    89,   240,    80,    61,   114,
     231,   348,   302,    62,   160,    78,    63,   122,   123,   124,
     125
};

  /* YYTABLEYYPACT[STATE-NUM] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      70,    70,   163,   237,    77,    64,   304,    66,   297,    93,
      94,    74,   246,    87,   104,    75,   175,   141,    92,    49,
      71,    73,   280,   102,   103,   281,   106,   107,   108,   109,
     110,   111,   112,   328,   247,   115,   116,   117,   121,   279,
     321,   150,   151,    81,   332,   128,   132,   176,   335,     8,
      90,   242,    95,    96,    97,   100,     1,   282,   177,   101,
     281,    98,   154,   155,   232,   157,   155,   141,    99,     2,
     155,   168,     3,    70,   126,   288,     5,   166,   159,   234,
     157,    65,    11,   156,    12,   141,   158,   185,   142,   148,
     149,   150,   151,   162,   180,   363,   341,   342,   379,   178,
     179,    33,    34,    35,    36,   181,   147,   148,   149,   150,
     151,   251,   275,   324,   276,   153,   281,   235,   157,   316,
     319,   127,   252,   157,   305,   212,   161,   164,   167,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,    49,    70,   169,   170,
     174,   233,   182,    43,   186,   236,    44,   183,   200,   202,
     206,   239,   284,   350,   204,   205,   244,   245,   207,   248,
     249,   250,   208,   241,   254,   255,   256,   230,   257,   238,
     258,   362,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   253,   372,   271,   278,   283,   285,   291,   292,
     378,   274,   300,   299,   303,   301,   330,   272,   273,   329,
     345,   -58,   310,   339,   347,   346,   351,   352,   360,   357,
     355,   364,   369,   374,   376,   327,   322,   209,   323,   325,
     270,   131,   133,   134,   243,   277,     0,     0,     0,     0,
     286,   287,     0,   333,     0,   289,   290,     0,     0,     0,
       0,     0,     0,   344,     0,     0,     0,     0,     0,   349,
       0,     0,     0,   353,     0,     0,     0,   356,     0,     0,
       0,     0,     0,     0,     0,   -34,     0,   361,     0,   306,
       0,     0,   365,   368,     0,   370,     0,   312,   313,   314,
     315,     0,     0,   375,   311,     0,   377,     0,     0,     0,
       0,     0,   381,   135,   136,   137,   138,   139,   140,     0,
     141,     0,     0,   142,   143,   144,     0,     0,     0,     0,
       0,     0,     0,   340,     0,     0,  -167,     1,     0,   145,
     146,   147,   148,   149,   150,   151,   354,     0,     0,     0,
       2,     0,     0,     3,     4,   359,     0,     5,  -167,     7,
       8,     9,    67,    11,   367,    12,  -167,  -167,     0,    15,
      16,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,     0,
    -167,     0,  -167,  -167,  -167,     0,    68,  -167,    26,  -167,
    -167,  -167,  -167,     0,     0,     0,  -167,  -167,  -167,  -167,
    -167,  -167,  -167,  -167,  -167,    32,  -167,  -167,  -167,  -167,
    -167,  -167,    33,    34,    35,    36,    37,  -167,  -167,  -167,
    -167,     0,  -167,  -167,  -167,  -167,  -167,  -167,    41,    42,
    -167,  -167,     1,  -167,    72,  -167,  -167,    44,  -167,     0,
       0,     0,     0,     0,     0,     2,     0,   171,     3,     4,
       0,     0,     5,     0,     7,     8,     9,    67,   118,     0,
     119,     0,     0,     0,    15,    16,   135,   136,   137,   138,
     139,   140,     0,   141,   172,   173,   142,   143,   144,     0,
       0,    68,     0,    26,     0,     0,     0,     0,     0,     0,
       0,     0,   145,   146,   147,   148,   149,   150,   151,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   -68,     1,    41,    42,     0,     0,     0,     0,    43,
       0,     0,    44,  -173,   120,     2,     0,     0,     3,     4,
       0,     0,     5,   -68,     7,     8,     9,    67,    11,     0,
      12,   -68,   -68,     0,    15,    16,     0,   -68,   -68,   -68,
     -68,   -68,   -68,   -68,     0,     0,     0,     0,   -68,     0,
       0,    68,   -68,    26,     0,     0,   -68,     0,     0,     0,
       0,   -68,   -68,   -68,   -68,   -68,   -68,   -68,   -68,   -68,
      32,   -68,     0,     0,   -68,   -68,   -68,    33,    34,    35,
      36,    37,     0,     0,   -68,   -68,     0,   -68,   -68,   -68,
     -68,   -68,   -68,    41,    42,   -68,   -68,     0,   -68,    76,
    -167,     1,    44,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     2,     0,     0,     3,     4,     0,
       0,     5,  -167,     7,     8,     9,    67,    11,     0,    12,
    -167,  -167,     0,    15,    16,     0,  -167,  -167,  -167,  -167,
    -167,  -167,  -167,     0,     0,     0,     0,  -167,     0,     0,
      68,  -167,    26,     0,     0,  -167,     0,     0,     0,     0,
    -167,  -167,  -167,     0,     0,     0,     0,     0,     0,    32,
       0,     0,     0,     0,     0,     0,    33,    34,    35,    36,
      37,     0,     0,  -167,  -167,     0,  -167,  -167,  -167,  -167,
       1,     0,    41,    42,     0,     0,     0,  -167,    69,     0,
    -167,    44,     0,     2,     0,     0,     3,     4,     0,     0,
       5,  -167,     7,     8,     9,    67,    11,     0,    12,  -167,
    -167,     0,    15,    16,     0,  -167,  -167,  -167,  -167,  -167,
    -167,  -167,     0,     0,     0,     0,  -167,     0,     0,    68,
    -167,    26,     0,     0,  -167,     0,     0,     0,     0,  -167,
    -167,  -167,     0,     0,     0,     0,     0,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,  -167,  -167,     0,  -167,  -167,  -167,    -4,     1,
       0,    41,    42,     0,     0,     0,  -167,    43,     0,  -167,
      44,     0,     2,     0,     0,     3,     4,     0,     0,     5,
       6,     7,     8,     9,    10,    11,     0,    12,     0,    13,
      14,    15,    16,     0,     0,     0,    17,    18,    19,    20,
      21,     0,     0,    22,     0,    23,     0,     0,    24,    25,
      26,     0,     0,    27,     0,     0,     0,    28,    29,    30,
      31,     0,     0,     0,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,    38,    39,    40,  -156,     1,     0,     0,     0,     0,
      41,    42,     0,     0,     0,    -4,    43,     0,     2,    44,
       0,     3,     4,     0,     0,     5,     6,     7,     8,     9,
      10,    11,     0,    12,   334,    13,     0,    15,    16,     0,
       0,     0,    17,    18,    19,    20,    21,     0,     0,     0,
       0,    23,     0,     0,    24,    25,    26,     0,     0,    27,
       0,     0,     0,     0,    29,    30,    31,     0,     0,     0,
       0,     0,     0,    32,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    36,    37,     0,     0,    38,    39,  -154,
       1,     0,     0,     0,     0,     0,    41,    42,     0,     0,
       0,   317,    43,     2,     0,    44,     3,     4,     0,     0,
       5,     6,     7,     8,     9,    10,    11,     0,    12,     0,
      13,     0,    15,    16,     0,     0,     0,    17,    18,    19,
      20,    21,     0,     0,     0,     0,    23,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    29,
      30,    31,     0,     0,     0,     0,     0,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,    38,    39,  -151,     1,     0,     0,     0,     0,
       0,    41,    42,     0,     0,     0,   317,    43,     2,     0,
      44,     3,     4,     0,     0,     5,     6,     7,     8,     9,
      10,    11,     0,    12,     0,    13,     0,    15,    16,     0,
       0,     0,    17,    18,    19,    20,    21,     0,     0,     0,
       0,    23,     0,     0,    24,    25,    26,     0,     0,    27,
       0,     0,     0,     0,    29,    30,    31,     0,     0,     0,
       0,     0,     0,    32,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    36,    37,     0,     0,    38,    39,     0,
       1,     0,     0,     0,     0,     0,    41,    42,     0,     0,
       0,   317,    43,     2,     0,    44,     3,     4,     0,     0,
       5,     6,     7,     8,     9,    10,    11,     0,    12,     0,
      13,     0,    15,    16,     0,   -55,   -55,    17,    18,    19,
      20,    21,     0,     0,     0,     0,    23,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    29,
      30,    31,     0,     0,     0,     0,     0,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,    38,    39,     0,   -55,     1,     0,     0,     0,
       0,    41,    42,     0,     0,     0,   317,    43,     0,     2,
      44,     0,     3,     4,     0,     0,     5,     6,     7,     8,
       9,    10,    11,     0,    12,     0,    13,     0,    15,    16,
       0,     0,     0,    17,    18,    19,    20,    21,     0,     0,
       0,     0,    23,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    29,    30,    31,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,    38,    39,
       0,     1,     0,   -59,     0,     0,     0,    41,    42,     0,
       0,     0,   317,    43,     2,     0,    44,     3,     4,     0,
       0,     5,     6,     7,     8,     9,    10,    11,     0,    12,
       0,    13,     0,    15,    16,     0,     0,     0,    17,    18,
      19,    20,    21,     0,     0,     0,     0,    23,     0,     0,
      24,    25,    26,     0,     0,    27,     0,     0,     0,     0,
      29,    30,    31,     0,     0,     0,     0,     0,     0,    32,
       0,     0,     0,     0,     0,     0,    33,    34,    35,    36,
      37,     0,     0,    38,    39,     0,     1,   -55,     0,     0,
       0,     0,    41,    42,     0,     0,     0,   317,    43,     2,
       0,    44,     3,     4,     0,     0,     5,     6,     7,     8,
       9,    10,    11,     0,    12,     0,    13,     0,    15,    16,
       0,     0,     0,    17,    18,    19,    20,    21,     0,     0,
       0,     0,    23,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    29,    30,    31,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,    38,    39,
       0,   -58,     1,     0,     0,     0,     0,    41,    42,     0,
       0,     0,   317,    43,     0,     2,    44,     0,     3,     4,
       0,     0,     5,     6,     7,     8,     9,    10,    11,     0,
      12,     0,    13,     0,    15,    16,     0,     0,     0,    17,
      18,    19,    20,    21,     0,     0,     0,     0,    23,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    29,    30,    31,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,    38,    39,     0,     1,   371,     0,
       0,     0,     0,    41,    42,     0,     0,     0,   317,    43,
       2,     0,    44,     3,     4,     0,     0,     5,     6,     7,
       8,     9,    10,    11,     0,    12,     0,    13,     0,    15,
      16,     0,     0,     0,    17,    18,    19,    20,    21,     0,
       0,     0,     0,    23,     0,     0,    24,    25,    26,     0,
       0,    27,     0,     0,     0,     0,    29,    30,    31,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,    38,
      39,     0,     1,   380,     0,     0,     0,     0,    41,    42,
       0,     0,     0,   317,    43,     2,     0,    44,     3,     4,
       0,     0,     5,     6,     7,     8,     9,    10,    11,     0,
      12,     0,    13,     0,    15,    16,     0,     0,     0,    17,
      18,    19,    20,    21,     0,     0,     0,     0,    23,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    29,    30,    31,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,    38,    39,     0,     1,   382,     0,
       0,     0,     0,    41,    42,     0,     0,     0,   317,    43,
       2,     0,    44,     3,     4,     0,     0,     5,     6,     7,
       8,     9,    10,    11,     0,    12,     0,    13,     0,    15,
      16,     0,     0,     0,    17,    18,    19,    20,    21,     0,
       0,     0,     0,     0,     0,     0,    24,     0,    26,     0,
       0,     0,     0,     0,     0,     0,    29,    30,    31,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,     1,
      39,     0,     0,     0,     0,     0,     0,     0,    41,    42,
       0,     0,     2,     0,    43,     3,     4,    44,     0,     5,
       6,     7,     8,     9,    10,    11,     0,    12,     0,    13,
       0,    15,    16,     0,     0,     0,    17,    18,    19,    20,
       0,     0,     0,     0,     0,     0,     0,     0,    24,     0,
      26,     0,     0,     0,     0,     0,     0,     0,    29,    30,
      31,     0,     0,     0,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,     1,    39,     0,     0,     0,     0,     0,     0,     0,
      41,    42,     0,     0,     2,     0,    43,     3,     4,    44,
       0,     5,     0,     7,     8,     9,    67,    11,     0,    12,
       0,     0,     0,    15,    16,     0,     0,     0,     1,     0,
       0,     0,     0,     0,     0,     0,    83,    84,    85,    86,
      68,     2,    26,     0,     3,     4,     0,     0,     5,     0,
       7,     8,     9,    67,    11,     0,    12,     0,     0,    32,
      15,    16,     0,     0,     0,     0,    33,    34,    35,    36,
      37,     0,     0,    83,     0,    85,     0,    68,     0,    26,
       0,     0,    41,    42,     0,     0,     0,     0,    43,     0,
       0,    44,     0,     0,     0,     0,    32,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,    37,     0,     0,
       1,     0,     0,     0,     0,     0,     0,     0,     0,    41,
      42,     0,     0,     2,     0,    43,     3,     4,    44,     0,
       5,     0,     7,     8,     9,    67,    11,     0,    12,     0,
       0,     0,    15,    16,     0,     0,     0,     1,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    68,
       2,    26,     0,     3,     4,     0,    91,     5,     0,     7,
       8,     9,    67,    11,     0,    12,     0,     0,    32,    15,
      16,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,     0,     0,     0,     0,    68,     0,    26,     0,
       0,    41,    42,     0,     0,     0,     0,    43,  -167,  -167,
      44,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,     1,
       0,     0,     0,     0,     0,     0,     0,     0,    41,    42,
       0,     0,     2,     0,    43,     3,     4,    44,     0,     5,
       0,     7,     8,     9,    67,    11,     0,    12,     0,     0,
       0,    15,    16,     0,     0,     0,    83,     0,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    68,     0,
      26,     0,     0,   135,   136,   137,   138,   139,   140,     0,
     141,     0,     0,   142,   143,   144,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,   145,
     146,   147,   148,   149,   150,   151,   188,     0,     0,     0,
      41,    42,   189,     0,     0,     0,    43,     0,     0,    44,
       0,   135,   136,   137,   138,   139,   140,     0,   141,     0,
       0,   142,   143,   144,     0,     0,     0,     0,     0,   190,
       0,     0,     0,     0,     0,   191,     0,   145,   146,   147,
     148,   149,   150,   151,   135,   136,   137,   138,   139,   140,
       0,   141,     0,     0,   142,   143,   144,     0,     0,     0,
       0,     0,   192,     0,     0,     0,     0,     0,   193,     0,
     145,   146,   147,   148,   149,   150,   151,   135,   136,   137,
     138,   139,   140,     0,   141,     0,     0,   142,   143,   144,
       0,     0,     0,     0,     0,   194,     0,     0,     0,     0,
       0,   195,     0,   145,   146,   147,   148,   149,   150,   151,
     135,   136,   137,   138,   139,   140,     0,   141,     0,     0,
     142,   143,   144,     0,     0,     0,     0,     0,   293,     0,
       0,     0,     0,     0,     0,     0,   145,   146,   147,   148,
     149,   150,   151,   135,   136,   137,   138,   139,   140,     0,
     141,     0,     0,   142,   143,   144,     0,     0,     0,     0,
       0,   294,     0,     0,     0,     0,     0,     0,     0,   145,
     146,   147,   148,   149,   150,   151,   135,   136,   137,   138,
     139,   140,     0,   141,     0,     0,   142,   143,   144,     0,
       0,     0,     0,     0,   295,     0,     0,     0,     0,     0,
       0,     0,   145,   146,   147,   148,   149,   150,   151,   135,
     136,   137,   138,   139,   140,     0,   141,     0,     0,   142,
     143,   144,     0,     0,     0,     0,     0,   296,     0,     0,
       0,     0,     0,     0,     0,   145,   146,   147,   148,   149,
     150,   151,   135,   136,   137,   138,   139,   140,     0,   141,
       0,     0,   142,   143,   144,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   198,     0,   145,   146,
     147,   148,   149,   150,   151,   135,   136,   137,   138,   139,
     140,   184,   141,     0,     0,   142,   143,   144,   135,   136,
     137,   138,   139,   140,     0,   141,     0,     0,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,     0,   199,
       0,     0,     0,     0,   145,   146,   147,   148,   149,   150,
     151,   135,   136,   137,   138,   139,   140,     0,   141,     0,
       0,   142,   143,   144,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   145,   146,   147,
     148,   149,   150,   151,     0,     0,     0,   203,   135,   136,
     137,   138,   139,   140,     0,   141,     0,     0,   142,   143,
     144,     0,     0,     0,     0,     0,   196,   197,     0,     0,
       0,     0,     0,     0,   145,   146,   147,   148,   149,   150,
     151,   135,   136,   137,   138,   139,   140,     0,   141,     0,
       0,   142,   143,   144,   135,   136,   137,   138,     0,     0,
       0,   141,     0,     0,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,     0,     0,     0,     0,     0,     0,
     145,   146,   147,   148,   149,   150,   151
};

static const yytype_int16 yycheck[] =
{
       6,     7,    80,   163,    10,    97,   282,     4,   269,    56,
      57,     8,    43,    19,    30,    48,    94,    70,    24,     0,
       6,     7,    97,    29,    30,   100,    32,    33,    34,    35,
      36,    37,    38,   310,    65,    41,    42,    43,    44,   212,
     301,    94,    95,    51,   321,    51,    52,    54,   324,    24,
      51,    26,    17,    18,    19,    26,     1,    97,    65,    26,
     100,    26,    68,    69,    99,   100,    72,    70,    33,    14,
      76,    87,    17,    79,     0,   248,    21,    83,    75,    99,
     100,    26,    27,    69,    29,    70,    72,   103,    73,    92,
      93,    94,    95,    79,    54,   356,    36,    37,   375,    96,
      97,    76,    77,    78,    79,    65,    91,    92,    93,    94,
      95,    54,    27,    97,    29,    98,   100,    99,   100,   297,
     298,    97,    65,   100,   284,   131,   100,    26,    46,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   127,   153,    26,    26,
      26,   157,    54,    98,    26,   161,   101,    53,    26,    26,
     102,   167,   240,   336,   103,   103,   172,   173,   100,   175,
     176,   177,   100,   170,   180,   181,   182,    26,   184,   100,
     100,   354,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    48,   366,   100,    53,    26,    44,    26,    26,
     373,   207,   278,    26,    26,    53,    88,   204,   205,    54,
      36,    86,   288,    87,    26,    86,    54,    35,    54,   100,
      86,    26,    53,    86,    86,   309,   302,   127,   304,   307,
     199,    52,    52,    52,   171,   208,    -1,    -1,    -1,    -1,
     246,   247,    -1,   321,    -1,   251,   252,    -1,    -1,    -1,
      -1,    -1,    -1,   331,    -1,    -1,    -1,    -1,    -1,   335,
      -1,    -1,    -1,   341,    -1,    -1,    -1,   345,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    -1,   353,    -1,   285,
      -1,    -1,   358,   361,    -1,   363,    -1,   293,   294,   295,
     296,    -1,    -1,   369,   291,    -1,   372,    -1,    -1,    -1,
      -1,    -1,   378,    63,    64,    65,    66,    67,    68,    -1,
      70,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   329,    -1,    -1,     0,     1,    -1,    89,
      90,    91,    92,    93,    94,    95,   342,    -1,    -1,    -1,
      14,    -1,    -1,    17,    18,   351,    -1,    21,    22,    23,
      24,    25,    26,    27,   360,    29,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    -1,
      44,    -1,    46,    47,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    -1,    -1,    -1,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,     1,    97,    98,    99,   100,   101,   102,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    -1,    44,    17,    18,
      -1,    -1,    21,    -1,    23,    24,    25,    26,    27,    -1,
      29,    -1,    -1,    -1,    33,    34,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    75,    -1,
      -1,    50,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     0,     1,    92,    93,    -1,    -1,    -1,    -1,    98,
      -1,    -1,   101,   102,   103,    14,    -1,    -1,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    30,    31,    -1,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    -1,    73,    74,    75,    76,    77,    78,
      79,    80,    -1,    -1,    83,    84,    -1,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
       0,     1,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    14,    -1,    -1,    17,    18,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    29,
      30,    31,    -1,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    -1,    -1,    83,    84,    -1,    86,    87,    88,     0,
       1,    -1,    92,    93,    -1,    -1,    -1,    97,    98,    -1,
     100,   101,    -1,    14,    -1,    -1,    17,    18,    -1,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    29,    30,
      31,    -1,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,    60,
      61,    62,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      -1,    -1,    83,    84,    -1,    86,    87,    88,     0,     1,
      -1,    92,    93,    -1,    -1,    -1,    97,    98,    -1,   100,
     101,    -1,    14,    -1,    -1,    17,    18,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    -1,    31,
      32,    33,    34,    -1,    -1,    -1,    38,    39,    40,    41,
      42,    -1,    -1,    45,    -1,    47,    -1,    -1,    50,    51,
      52,    -1,    -1,    55,    -1,    -1,    -1,    59,    60,    61,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,
      -1,    83,    84,    85,     0,     1,    -1,    -1,    -1,    -1,
      92,    93,    -1,    -1,    -1,    97,    98,    -1,    14,   101,
      -1,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    30,    31,    -1,    33,    34,    -1,
      -1,    -1,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    60,    61,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    -1,    -1,    83,    84,     0,
       1,    -1,    -1,    -1,    -1,    -1,    92,    93,    -1,    -1,
      -1,    97,    98,    14,    -1,   101,    17,    18,    -1,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    29,    -1,
      31,    -1,    33,    34,    -1,    -1,    -1,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,    60,
      61,    62,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      -1,    -1,    83,    84,     0,     1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    -1,    -1,    -1,    97,    98,    14,    -1,
     101,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    -1,    31,    -1,    33,    34,    -1,
      -1,    -1,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    60,    61,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    -1,    -1,    83,    84,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    92,    93,    -1,    -1,
      -1,    97,    98,    14,    -1,   101,    17,    18,    -1,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    29,    -1,
      31,    -1,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,    60,
      61,    62,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      -1,    -1,    83,    84,    -1,    86,     1,    -1,    -1,    -1,
      -1,    92,    93,    -1,    -1,    -1,    97,    98,    -1,    14,
     101,    -1,    17,    18,    -1,    -1,    21,    22,    23,    24,
      25,    26,    27,    -1,    29,    -1,    31,    -1,    33,    34,
      -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,    -1,
      -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    60,    61,    62,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    -1,    -1,    83,    84,
      -1,     1,    -1,    88,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    97,    98,    14,    -1,   101,    17,    18,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    29,
      -1,    31,    -1,    33,    34,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    -1,    -1,    83,    84,    -1,     1,    87,    -1,    -1,
      -1,    -1,    92,    93,    -1,    -1,    -1,    97,    98,    14,
      -1,   101,    17,    18,    -1,    -1,    21,    22,    23,    24,
      25,    26,    27,    -1,    29,    -1,    31,    -1,    33,    34,
      -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,    -1,
      -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    60,    61,    62,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    -1,    -1,    83,    84,
      -1,    86,     1,    -1,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    97,    98,    -1,    14,   101,    -1,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    -1,    31,    -1,    33,    34,    -1,    -1,    -1,    38,
      39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    -1,    -1,    83,    84,    -1,     1,    87,    -1,
      -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    97,    98,
      14,    -1,   101,    17,    18,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    29,    -1,    31,    -1,    33,
      34,    -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    60,    61,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    -1,    -1,    83,
      84,    -1,     1,    87,    -1,    -1,    -1,    -1,    92,    93,
      -1,    -1,    -1,    97,    98,    14,    -1,   101,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    -1,    31,    -1,    33,    34,    -1,    -1,    -1,    38,
      39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    -1,    -1,    83,    84,    -1,     1,    87,    -1,
      -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    97,    98,
      14,    -1,   101,    17,    18,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    29,    -1,    31,    -1,    33,
      34,    -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    -1,    -1,     1,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      -1,    -1,    14,    -1,    98,    17,    18,   101,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    -1,    31,
      -1,    33,    34,    -1,    -1,    -1,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,
      -1,     1,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    -1,    -1,    14,    -1,    98,    17,    18,   101,
      -1,    21,    -1,    23,    24,    25,    26,    27,    -1,    29,
      -1,    -1,    -1,    33,    34,    -1,    -1,    -1,     1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    14,    52,    -1,    17,    18,    -1,    -1,    21,    -1,
      23,    24,    25,    26,    27,    -1,    29,    -1,    -1,    69,
      33,    34,    -1,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    -1,    -1,    46,    -1,    48,    -1,    50,    -1,    52,
      -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    98,    -1,
      -1,   101,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    -1,    -1,    14,    -1,    98,    17,    18,   101,    -1,
      21,    -1,    23,    24,    25,    26,    27,    -1,    29,    -1,
      -1,    -1,    33,    34,    -1,    -1,    -1,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      14,    52,    -1,    17,    18,    -1,    20,    21,    -1,    23,
      24,    25,    26,    27,    -1,    29,    -1,    -1,    69,    33,
      34,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    52,    -1,
      -1,    92,    93,    -1,    -1,    -1,    -1,    98,    99,   100,
     101,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    -1,    -1,     1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      -1,    -1,    14,    -1,    98,    17,    18,   101,    -1,    21,
      -1,    23,    24,    25,    26,    27,    -1,    29,    -1,    -1,
      -1,    33,    34,    -1,    -1,    -1,    46,    -1,    48,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      52,    -1,    -1,    63,    64,    65,    66,    67,    68,    -1,
      70,    -1,    -1,    73,    74,    75,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    89,
      90,    91,    92,    93,    94,    95,    48,    -1,    -1,    -1,
      92,    93,    54,    -1,    -1,    -1,    98,    -1,    -1,   101,
      -1,    63,    64,    65,    66,    67,    68,    -1,    70,    -1,
      -1,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    54,    -1,    89,    90,    91,
      92,    93,    94,    95,    63,    64,    65,    66,    67,    68,
      -1,    70,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      89,    90,    91,    92,    93,    94,    95,    63,    64,    65,
      66,    67,    68,    -1,    70,    -1,    -1,    73,    74,    75,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    89,    90,    91,    92,    93,    94,    95,
      63,    64,    65,    66,    67,    68,    -1,    70,    -1,    -1,
      73,    74,    75,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    63,    64,    65,    66,    67,    68,    -1,
      70,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    63,    64,    65,    66,
      67,    68,    -1,    70,    -1,    -1,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    63,
      64,    65,    66,    67,    68,    -1,    70,    -1,    -1,    73,
      74,    75,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    63,    64,    65,    66,    67,    68,    -1,    70,
      -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    89,    90,
      91,    92,    93,    94,    95,    63,    64,    65,    66,    67,
      68,    56,    70,    -1,    -1,    73,    74,    75,    63,    64,
      65,    66,    67,    68,    -1,    70,    -1,    -1,    73,    74,
      75,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    63,    64,    65,    66,    67,    68,    -1,    70,    -1,
      -1,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    -1,    -1,    -1,    99,    63,    64,
      65,    66,    67,    68,    -1,    70,    -1,    -1,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    81,    82,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    63,    64,    65,    66,    67,    68,    -1,    70,    -1,
      -1,    73,    74,    75,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    -1,    -1,    73,    74,    75,    89,    90,    91,
      92,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95
};

  /* YYSTOSSTATE-NUM -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    14,    17,    18,    21,    22,    23,    24,    25,
      26,    27,    29,    31,    32,    33,    34,    38,    39,    40,
      41,    42,    45,    47,    50,    51,    52,    55,    59,    60,
      61,    62,    69,    76,    77,    78,    79,    80,    83,    84,
      85,    92,    93,    98,   101,   105,   106,   107,   108,   110,
     121,   128,   132,   133,   134,   135,   136,   137,   144,   146,
     148,   152,   157,   160,    97,    26,   133,    26,    50,    98,
     134,   158,    98,   158,   133,    48,    98,   134,   159,   147,
     151,    51,   138,    46,    47,    48,    49,   134,   145,   149,
      51,    20,   134,    56,    57,    17,    18,    19,    26,    33,
      26,    26,   134,   134,   145,   140,   134,   134,   134,   134,
     134,   134,   134,   139,   153,   134,   134,   134,    27,    29,
     103,   134,   161,   162,   163,   164,     0,    97,   134,   109,
     124,   128,   134,   137,   157,    63,    64,    65,    66,    67,
      68,    70,    73,    74,    75,    89,    90,    91,    92,    93,
      94,    95,   129,    98,   134,   134,   158,   100,   158,   133,
     158,   100,   158,   129,    26,   141,   134,    46,   145,    26,
      26,    44,    71,    72,    26,   129,    54,    65,   133,   133,
      54,    65,    54,    53,    56,   145,    26,   143,    48,    54,
      48,    54,    48,    54,    48,    54,    81,    82,    54,    97,
      26,   142,    26,    99,   103,   103,   102,   100,   100,   105,
     125,   130,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
      26,   154,    99,   134,    99,    99,   134,   154,   100,   134,
     150,   133,    26,   136,   134,   134,    43,    65,   134,   134,
     134,    54,    65,    48,   134,   134,   134,   134,   100,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   120,
     120,   100,   133,   133,   134,    27,    29,   164,    53,   125,
      97,   100,    97,    26,   129,    44,   134,   134,   125,   134,
     134,    26,    26,    48,    48,    48,    48,   108,   131,    26,
     131,    53,   156,    26,   156,   154,   134,   117,   111,   114,
     131,   133,   134,   134,   134,   134,   130,    97,   110,   130,
     126,   108,   131,   131,    97,   129,   127,   127,   126,    54,
      88,   122,   126,   129,    30,   156,   118,   112,   115,    87,
     134,    36,    37,   123,   129,    36,    86,    26,   155,   131,
     125,    54,    35,   129,   134,    86,   129,   100,   119,   134,
      54,   131,   125,   108,    26,   131,   113,   134,   129,    53,
     129,    87,   125,   116,    86,   131,    86,   131,   125,   126,
      87,   131,    87
};

  /* YYR1YYN -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   104,   105,   105,   106,   106,   106,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     108,   108,   108,   109,   109,   109,   109,   110,   110,   110,
     111,   112,   113,   110,   114,   115,   116,   110,   117,   118,
     119,   110,   110,   110,   110,   110,   120,   121,   121,   122,
     122,   123,   124,   124,   125,   126,   127,   128,   129,   130,
     131,   131,   131,   132,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   135,   135,   135,   135,   135,
     135,   135,   135,   136,   136,   137,   137,   137,   137,   137,
     138,   137,   139,   137,   140,   137,   137,   137,   137,   137,
     141,   141,   142,   142,   143,   143,   144,   144,   144,   144,
     144,   144,   145,   145,   146,   146,   146,   147,   146,   149,
     150,   148,   148,   151,   148,   148,   148,   153,   152,   154,
     154,   154,   155,   155,   155,   156,   157,   158,   158,   158,
     159,   159,   160,   161,   161,   161,   161,   162,   162,   163,
     163,   164,   164
};

  /* YYR2YYN -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     4,     4,     6,
       4,     4,     4,     4,     4,     4,     5,     5,     8,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     8,
       0,     0,     0,    14,     0,     0,     0,    15,     0,     0,
       0,    12,     2,     3,     7,     6,     0,     9,    12,     0,
       2,     6,    11,     7,     0,     0,     0,     1,     0,     0,
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       3,     1,     2,     1,     1,     4,     2,     4,     3,     2,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     3,     3,
       3,     2,     2,     4,     4,     4,     6,     4,     6,     4,
       6,     4,     6,     2,     1,     2,     1,     1,     2,     1,
       0,     3,     0,     3,     0,     3,     4,     2,     4,     2,
       1,     3,     1,     3,     1,     3,     1,     2,     2,     2,
       3,     2,     3,     2,     2,     3,     2,     0,     3,     0,
       0,     9,     2,     0,     7,     8,     6,     0,     3,     0,
       1,     3,     0,     1,     3,     0,     2,     0,     1,     3,
       1,     3,     3,     0,     1,     1,     1,     1,     3,     1,
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
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1960 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* FBLTIN  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1966 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* RBLTIN  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1972 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* THEFBLTIN  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1978 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* ID  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1984 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* STRING  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1990 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* HANDLER  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1996 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* SYMBOL  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2002 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 30: /* ENDCLAUSE  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2008 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 31: /* tPLAYACCEL  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2014 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 32: /* tMETHOD  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2020 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 33: /* THEOBJECTFIELD  */
#line 163 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).os; }
#line 2026 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 152: /* on  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2032 "engines/director/lingo/lingo-gr.cpp"
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
#line 174 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s), kVarLocal);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2319 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 181 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2327 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 185 "engines/director/lingo/lingo-gr.y"
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
#line 2343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 196 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_after); }
#line 2349 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 197 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_before); }
#line 2355 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 198 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mArg((yyvsp[-2].s), kVarLocal);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2367 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 205 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 212 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mArg((yyvsp[-2].s), kVarLocal);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2391 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 219 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2403 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 226 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2414 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 232 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2425 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 239 "engines/director/lingo/lingo-gr.y"
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
#line 2441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 250 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).oe);
		delete (yyvsp[-2].objectfield).os;
		(yyval.code) = (yyvsp[0].code); }
#line 2452 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 272 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-5].code) - (yyvsp[-1].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code) + 2);
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentScript)[(yyvsp[-1].code)] = start; }
#line 2463 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 284 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mArg((yyvsp[-2].s), kVarLocal); }
#line 2471 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 288 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2478 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 291 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_le); }
#line 2484 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 291 "engines/director/lingo/lingo-gr.y"
                                                                                          {

		g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[-11].s)->c_str());
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_add);
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-11].s)->c_str());
		g_lingo->code1(LC::c_assign);
		g_lingo->code2(LC::c_jump, STOP);
		int pos = g_lingo->_currentScript->size() - 1;

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-7].code) - pos + 2);
		WRITE_UINT32(&end, pos - (yyvsp[-2].code) + 2);
		(*g_lingo->_currentScript)[pos] = loop;		/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-2].code)] = end;	}
#line 2507 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 315 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mArg((yyvsp[-2].s), kVarLocal); }
#line 2515 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 319 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2522 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 322 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_ge); }
#line 2528 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 323 "engines/director/lingo/lingo-gr.y"
                                                    {

		g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[-12].s)->c_str());
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_sub);
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-12].s)->c_str());
		g_lingo->code1(LC::c_assign);
		g_lingo->code2(LC::c_jump, STOP);
		int pos = g_lingo->_currentScript->size() - 1;

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-8].code) - pos + 2);
		WRITE_UINT32(&end, pos - (yyvsp[-2].code) + 2);
		(*g_lingo->_currentScript)[pos] = loop;		/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-2].code)] = end;	}
#line 2551 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 348 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  g_lingo->codeFunc(new Common::String("count"), 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
#line 2561 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 354 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
#line 2571 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 360 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get list
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(1);
				  g_lingo->codeFunc(new Common::String("getAt"), 2);
				  g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-6].s)->c_str());
				  mArg((yyvsp[-6].s), kVarLocal);
				  g_lingo->code1(LC::c_assign); }
#line 2585 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 369 "engines/director/lingo/lingo-gr.y"
                                            {

		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_add);			// Increment counter

		int jump = g_lingo->code2(LC::c_jump, STOP);

		int end2 = g_lingo->code1(LC::c_stackdrop);	// remove list, size, counter
		g_lingo->codeInt(3);

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-5].code) - jump);
		WRITE_UINT32(&end, end2 - (yyvsp[-3].code) + 1);

		(*g_lingo->_currentScript)[jump + 1] = loop;		/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = end;	}
#line 2607 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 387 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_nextRepeat); }
#line 2614 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 389 "engines/director/lingo/lingo-gr.y"
                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2624 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 394 "engines/director/lingo/lingo-gr.y"
                                                          {
		inst end;
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code));
		(*g_lingo->_currentScript)[(yyvsp[-3].code) + 1] = end; }
#line 2633 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 398 "engines/director/lingo/lingo-gr.y"
                                                    {
		inst end;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2642 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 403 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }
#line 2650 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 407 "engines/director/lingo/lingo-gr.y"
                                                                                           {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2662 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 414 "engines/director/lingo/lingo-gr.y"
                                                                                                                  {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-6].code) + 1 - (yyvsp[-9].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-9].code)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-6].code), (yyvsp[-1].code)); }
#line 2674 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 425 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2684 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 431 "engines/director/lingo/lingo-gr.y"
                                                                                                             {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].code) + 1 - (yyvsp[-8].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-8].code)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-5].code)] = end;	}
#line 2695 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 437 "engines/director/lingo/lingo-gr.y"
                                                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-4].code) + 1);

		(*g_lingo->_currentScript)[(yyvsp[-4].code)] = end; }
#line 2705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 443 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, STOP);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2713 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 447 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, STOP);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2721 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 451 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 455 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2736 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 458 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2742 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 460 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2748 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 462 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2754 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 466 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2764 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 472 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2772 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 475 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2780 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 478 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 482 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2798 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 486 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2807 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 490 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2820 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 498 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2826 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 500 "engines/director/lingo/lingo-gr.y"
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
#line 2841 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 511 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2847 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 513 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2855 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 516 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 519 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2871 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 522 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2879 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 525 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2890 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 531 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).oe);
		delete (yyvsp[0].objectfield).os; }
#line 2900 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 536 "engines/director/lingo/lingo-gr.y"
                       {
		g_lingo->code1(LC::c_objectrefpush);
		g_lingo->codeString((yyvsp[0].objectref).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectref).field->c_str());
		delete (yyvsp[0].objectref).obj;
		delete (yyvsp[0].objectref).field; }
#line 2911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 543 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 544 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 545 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2929 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 546 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 547 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2941 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 548 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 549 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2953 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 550 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 551 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 552 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2971 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 553 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 554 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 555 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 556 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 557 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 3001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 558 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 3007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 559 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 3013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 560 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 3019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 561 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); }
#line 3025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 562 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 3031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 563 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_intersects); }
#line 3037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 564 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 3043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 566 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charOf); }
#line 3049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 567 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charToOf); }
#line 3055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 568 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemOf); }
#line 3061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 569 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_itemToOf); }
#line 3067 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 570 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineOf); }
#line 3073 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 571 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_lineToOf); }
#line 3079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 572 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordOf); }
#line 3085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 573 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_wordToOf); }
#line 3091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 575 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 3099 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 580 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_printtop); }
#line 3105 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 583 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_exitRepeat); }
#line 3111 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 584 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 3117 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 585 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3123 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 585 "engines/director/lingo/lingo-gr.y"
                                                                                 { inNone(); }
#line 3129 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 586 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3135 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 586 "engines/director/lingo/lingo-gr.y"
                                                                                   { inNone(); }
#line 3141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 587 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3147 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 587 "engines/director/lingo/lingo-gr.y"
                                                                                   { inNone(); }
#line 3153 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 588 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3161 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 591 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3169 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 594 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_open); }
#line 3175 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 595 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 3181 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 597 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3191 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 602 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3201 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 608 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3211 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 613 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3221 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 619 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3231 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 624 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3241 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 637 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 638 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3253 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 639 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 640 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3268 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 644 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 648 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3286 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 656 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 660 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 664 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 668 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3319 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 668 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3328 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 698 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 698 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->_currentFactory.clear(); }
#line 3340 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 699 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-6].s); }
#line 3350 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 704 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3356 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 705 "engines/director/lingo/lingo-gr.y"
                  { startDef(); }
#line 3362 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 706 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
		endDef();
		delete (yyvsp[-6].s); }
#line 3372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 711 "engines/director/lingo/lingo-gr.y"
                                                                     {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3385 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 719 "engines/director/lingo/lingo-gr.y"
                                                 {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3395 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 725 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3401 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 725 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.s) = (yyvsp[0].s); g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 3408 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 728 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3414 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 729 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mArg((yyvsp[0].s), kVarArgument); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3420 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 730 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); mArg((yyvsp[0].s), kVarArgument); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3426 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 733 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3432 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 734 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3438 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 736 "engines/director/lingo/lingo-gr.y"
                                        { inDef(); }
#line 3444 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 738 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 3456 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 746 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3462 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 747 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3468 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 748 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3474 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 750 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3480 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 751 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3486 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172:
#line 753 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173:
#line 755 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3498 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174:
#line 756 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3504 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175:
#line 757 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3510 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176:
#line 758 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177:
#line 760 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3522 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178:
#line 761 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3528 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179:
#line 763 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3534 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180:
#line 764 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3540 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181:
#line 766 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3549 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182:
#line 770 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3558 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3562 "engines/director/lingo/lingo-gr.cpp"

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

#line 776 "engines/director/lingo/lingo-gr.y"


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
