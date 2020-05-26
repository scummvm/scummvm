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

static void mArg(Common::String *s) {
	g_lingo->_methodVars[*s] = true;
}


#line 127 "engines/director/lingo/lingo-gr.cpp"

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
#line 110 "engines/director/lingo/lingo-gr.y"

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

#line 285 "engines/director/lingo/lingo-gr.cpp"

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
  YYSYMBOL_tellstart = 111,                /* tellstart  */
  YYSYMBOL_ifstmt = 112,                   /* ifstmt  */
  YYSYMBOL_elseifstmtlist = 113,           /* elseifstmtlist  */
  YYSYMBOL_elseifstmt = 114,               /* elseifstmt  */
  YYSYMBOL_ifoneliner = 115,               /* ifoneliner  */
  YYSYMBOL_repeatwhile = 116,              /* repeatwhile  */
  YYSYMBOL_repeatwith = 117,               /* repeatwith  */
  YYSYMBOL_jumpifz = 118,                  /* jumpifz  */
  YYSYMBOL_jump = 119,                     /* jump  */
  YYSYMBOL_if = 120,                       /* if  */
  YYSYMBOL_begin = 121,                    /* begin  */
  YYSYMBOL_end = 122,                      /* end  */
  YYSYMBOL_stmtlist = 123,                 /* stmtlist  */
  YYSYMBOL_when = 124,                     /* when  */
  YYSYMBOL_simpleexpr = 125,               /* simpleexpr  */
  YYSYMBOL_expr = 126,                     /* expr  */
  YYSYMBOL_chunkexpr = 127,                /* chunkexpr  */
  YYSYMBOL_reference = 128,                /* reference  */
  YYSYMBOL_proc = 129,                     /* proc  */
  YYSYMBOL_130_1 = 130,                    /* $@1  */
  YYSYMBOL_131_2 = 131,                    /* $@2  */
  YYSYMBOL_132_3 = 132,                    /* $@3  */
  YYSYMBOL_globallist = 133,               /* globallist  */
  YYSYMBOL_propertylist = 134,             /* propertylist  */
  YYSYMBOL_instancelist = 135,             /* instancelist  */
  YYSYMBOL_gotofunc = 136,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 137,                /* gotomovie  */
  YYSYMBOL_playfunc = 138,                 /* playfunc  */
  YYSYMBOL_139_4 = 139,                    /* $@4  */
  YYSYMBOL_defn = 140,                     /* defn  */
  YYSYMBOL_141_5 = 141,                    /* $@5  */
  YYSYMBOL_142_6 = 142,                    /* $@6  */
  YYSYMBOL_143_7 = 143,                    /* $@7  */
  YYSYMBOL_on = 144,                       /* on  */
  YYSYMBOL_145_8 = 145,                    /* $@8  */
  YYSYMBOL_argdef = 146,                   /* argdef  */
  YYSYMBOL_endargdef = 147,                /* endargdef  */
  YYSYMBOL_argstore = 148,                 /* argstore  */
  YYSYMBOL_macro = 149,                    /* macro  */
  YYSYMBOL_arglist = 150,                  /* arglist  */
  YYSYMBOL_nonemptyarglist = 151,          /* nonemptyarglist  */
  YYSYMBOL_list = 152,                     /* list  */
  YYSYMBOL_valuelist = 153,                /* valuelist  */
  YYSYMBOL_linearlist = 154,               /* linearlist  */
  YYSYMBOL_proplist = 155,                 /* proplist  */
  YYSYMBOL_proppair = 156                  /* proppair  */
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
#define YYFINAL  128
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2388

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  53
/* YYNRULES -- Number of rules.  */
#define YYNRULES  174
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  378

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
       0,   165,   165,   166,   168,   169,   170,   172,   178,   182,
     193,   194,   195,   201,   208,   214,   221,   227,   234,   245,
     252,   253,   254,   256,   257,   258,   259,   261,   262,   267,
     278,   295,   307,   318,   320,   325,   329,   334,   338,   345,
     353,   354,   356,   362,   368,   374,   376,   382,   386,   390,
     393,   395,   397,   398,   399,   401,   407,   410,   413,   417,
     421,   425,   433,   434,   435,   446,   447,   448,   451,   454,
     457,   460,   466,   471,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   501,   502,   503,
     504,   505,   506,   507,   508,   510,   513,   515,   516,   517,
     518,   519,   520,   520,   521,   521,   522,   522,   523,   526,
     529,   530,   532,   537,   543,   548,   554,   559,   572,   573,
     574,   575,   579,   583,   588,   589,   591,   595,   599,   603,
     603,   633,   633,   633,   639,   640,   640,   646,   654,   660,
     660,   663,   664,   665,   667,   668,   669,   671,   673,   681,
     682,   683,   685,   686,   688,   690,   691,   692,   693,   695,
     696,   698,   699,   701,   705
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
  "stmtonelinerwithif", "stmt", "tellstart", "ifstmt", "elseifstmtlist",
  "elseifstmt", "ifoneliner", "repeatwhile", "repeatwith", "jumpifz",
  "jump", "if", "begin", "end", "stmtlist", "when", "simpleexpr", "expr",
  "chunkexpr", "reference", "proc", "$@1", "$@2", "$@3", "globallist",
  "propertylist", "instancelist", "gotofunc", "gotomovie", "playfunc",
  "$@4", "defn", "$@5", "$@6", "$@7", "on", "$@8", "argdef", "endargdef",
  "argstore", "macro", "arglist", "nonemptyarglist", "list", "valuelist",
  "linearlist", "proplist", "proppair", YY_NULLPTR
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

#define YYPACT_NINF (-318)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-166)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACTSTATE-NUM -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     775,   -63,  -318,  -318,   199,  -318,   597,   313,   199,    25,
     498,  -318,  -318,  -318,  -318,  -318,  -318,    41,  -318,  1622,
    -318,  -318,  -318,    52,  1778,   -36,    43,    80,    83,  1860,
    1659,  -318,  1860,  1860,  1860,  1860,  1860,  1860,  1860,  -318,
    -318,  1860,  1860,  1860,   408,   114,    19,  -318,  -318,  -318,
    -318,  1860,   -24,  1860,  1458,  -318,  2280,  -318,  -318,  -318,
    -318,  -318,  -318,  -318,  -318,  -318,  -318,  -318,  -318,    17,
    1778,  1741,  2280,    20,  1741,    20,  -318,   199,  1741,  2280,
      21,   686,  -318,  -318,    91,  1860,  -318,    72,  -318,  1852,
    -318,    93,  -318,    96,   380,    97,  -318,    -8,   199,   199,
       5,    70,    74,  -318,  2177,  1852,  -318,    99,  -318,  1900,
    1933,  1966,  1999,  2247,  2164,   102,   103,  -318,  -318,  2210,
      29,    30,  -318,  2280,    51,    54,    55,  -318,  -318,   775,
    2280,  -318,  -318,  2280,  -318,  -318,  1860,  2280,  -318,  -318,
    1860,  1860,  1860,  1860,  1860,  1860,  1860,  1860,  1860,  1860,
    1860,  1860,  1860,  1860,  1860,  1860,  1860,   135,  1741,   380,
    2210,   -10,  1860,     8,  -318,    11,  1860,    20,   135,  -318,
      62,  2280,  1860,  -318,  -318,   199,    18,  1860,  1860,  -318,
    1860,  1860,    13,   115,  1860,  1860,  1860,  -318,  1860,  -318,
    -318,    64,  1860,  1860,  1860,  1860,  1860,  1860,  1860,  1860,
    1860,  1860,  -318,  -318,  -318,    65,  -318,  -318,   199,   199,
    -318,  1860,    22,  -318,  -318,  1860,  1860,   116,  -318,  2280,
       7,     7,     7,     7,  2293,  2293,  -318,    -7,     7,     7,
       7,     7,    -7,   -55,   -55,  -318,  -318,  -318,   -75,  -318,
    2280,  -318,  -318,  2280,   -44,   141,  2280,  -318,   124,  -318,
    -318,  2280,  2280,  2280,     7,  1860,  1860,   147,  2280,     7,
    2280,  2280,   157,  2280,  2032,  2280,  2065,  2280,  2098,  2280,
    2131,  2280,  2280,  1540,  -318,   158,  -318,  -318,  2280,    29,
      30,  -318,  1202,  2280,  2280,  -318,   144,  -318,   172,  -318,
    -318,   135,  1860,  2280,     7,   199,  -318,  1860,  1860,  1860,
    1860,  -318,  1287,  -318,  -318,  -318,   117,  -318,    -2,  1116,
    1540,  -318,  -318,  -318,   -26,  2280,   148,  2280,  2280,  2280,
    2280,  -318,   113,  -318,  1202,   149,  1860,  -318,   120,   861,
     946,  -318,  1860,  -318,   121,  1860,  2280,    76,   171,   126,
     191,  -318,  2280,  -318,  2280,  -318,  -318,  1860,  -318,   137,
    -318,  -318,  -318,   127,  1031,  -318,  1202,  -318,  2280,  -318,
    1540,   195,  1202,   142,  1372,   177,  -318,  -318,   145,  -318,
     150,  -318,   152,  -318,  -318,  1116,  -318,  -318
};

  /* YYDEFACTSTATE-NUM -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    56,    61,     0,    57,     0,     0,     0,     0,
       0,    59,    58,   139,   145,    72,    73,   111,   112,     0,
     128,    49,   141,     0,     0,     0,     0,     0,     0,     0,
       0,   116,     0,     0,     0,     0,     0,     0,     0,   114,
     149,     0,     0,     0,     0,     0,     2,    74,    27,     6,
      28,     0,     0,     0,     0,    65,    21,   106,    66,    22,
     108,   109,     5,    50,    20,    63,    64,    60,    71,    60,
       0,     0,   160,   119,     0,    68,   105,     0,     0,   162,
     158,     0,    50,   110,     0,     0,   129,     0,   130,   131,
     133,     0,    33,     0,   107,     0,    45,     0,     0,     0,
       0,     0,     0,   144,   121,   136,   138,     0,    88,     0,
       0,     0,     0,     0,     0,     0,     0,    93,    94,     0,
      59,    58,   166,   169,     0,   167,   168,   171,     1,     0,
      51,    50,    50,    47,    51,    26,     0,    24,    25,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   151,     0,     0,
     160,     0,     0,     0,    70,     0,     0,   140,   151,   122,
     113,   135,     0,   132,   142,     0,     0,     0,     0,    46,
       0,     0,     0,     0,     0,     0,     0,    55,     0,   137,
     126,   117,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    37,    37,   124,   115,   150,    62,     0,     0,
     164,     0,     0,     3,    52,     0,     0,     0,    34,    47,
      84,    85,    82,    83,    86,    87,    79,    90,    91,    92,
      81,    80,    89,    75,    76,    77,    78,   152,     0,   118,
     161,    67,    69,   163,     0,     0,   134,    50,     0,     7,
       8,    10,    11,    15,    13,     0,     0,     0,    14,    12,
      19,   120,     0,    97,     0,    99,     0,   101,     0,   103,
       0,    95,    96,     0,    52,     0,   174,   173,   170,     0,
       0,   172,     0,    51,    51,    52,     0,   157,     0,   157,
     123,   151,     0,    16,    17,     0,   127,     0,     0,     0,
       0,    51,     0,   125,    53,    54,     0,    52,     0,     0,
       0,    52,   153,    52,     0,     9,     0,    98,   100,   102,
     104,    36,     0,    29,     0,     0,     0,    40,    48,     0,
       0,   157,     0,    35,     0,     0,    51,    50,     0,     0,
     154,    52,    18,    32,    51,    52,    50,     0,    41,     0,
      50,    44,   155,   147,     0,    52,     0,    52,    47,    38,
       0,     0,     0,     0,     0,     0,    50,   156,     0,    30,
       0,    52,     0,    31,    39,     0,    43,    42
};

  /* YYPGOTONTERM-NUM.  */
static const yytype_int16 yypgoto[] =
{
    -318,   104,  -318,  -318,  -267,  -318,     2,    31,  -318,  -318,
    -318,  -318,  -318,  -318,  -207,  -317,   181,   -77,  -125,   -89,
    -318,     6,    -6,  -318,    63,   186,  -318,  -318,  -318,  -318,
    -318,  -318,  -318,   -23,  -318,  -318,  -318,  -318,  -318,  -318,
    -318,  -318,  -165,  -318,  -281,   187,    10,  -318,  -318,  -318,
    -318,  -318,    32
};

  /* YYDEFGOTONTERM-NUM.  */
static const yytype_int16 yydefgoto[] =
{
      -1,    45,    46,    47,    48,   134,   305,   273,    50,   337,
     348,   135,    51,    52,   217,   327,    53,   157,   214,   282,
      54,    55,    56,    57,    58,    59,    84,   115,   107,   170,
     205,   191,    60,    90,    61,    81,    62,    91,   247,    82,
      63,   116,   238,   353,   311,    64,   165,    80,    65,   124,
     125,   126,   127
};

  /* YYTABLEYYPACT[STATE-NUM] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      72,    72,    49,   244,    79,   168,   301,   106,   313,   218,
      68,   338,   286,    89,    76,   146,    73,    75,    94,   131,
      95,    96,   287,   104,   105,   288,   108,   109,   110,   111,
     112,   113,   114,   325,    66,   117,   118,   119,   123,   155,
     156,   132,     8,   328,   249,   130,   180,   133,   137,   279,
     341,   280,   326,   289,   215,   216,   288,   181,   377,   184,
      97,    98,    99,   146,   159,   160,   173,   255,   160,   100,
     185,   331,   160,    77,   288,    72,   101,   146,   256,   171,
     147,   161,   189,   164,   163,   153,   154,   155,   156,   239,
     162,   167,    83,   366,    33,    34,    35,    36,   152,   153,
     154,   155,   156,    92,   182,   183,   102,   241,   162,   103,
     242,   162,   346,   347,   128,   158,   129,   169,   172,   174,
     162,   166,   175,   179,   186,   190,   314,   187,   204,   206,
     219,    49,   208,   209,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   365,    72,   210,   211,   212,   240,   306,   307,   308,
     243,   237,   245,   257,   262,   275,   246,   290,   292,   285,
     291,   251,   252,   295,   253,   254,   321,   322,   258,   259,
     260,   248,   261,   296,   303,   302,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   309,   310,   312,   334,
       1,   333,   332,   335,   323,   278,   -50,   350,   343,   283,
     284,   345,   351,     2,   276,   277,     3,   352,   324,   355,
       5,   367,   329,   359,   330,    67,    11,   361,    12,   369,
     371,   363,   373,   213,   274,   136,   374,   368,   376,   250,
     138,   139,     0,     0,   281,     0,     0,     0,     0,   293,
     294,   339,   354,     0,     0,     0,   356,     0,     0,     0,
     349,     0,     0,     0,     0,     0,   362,     0,   364,   357,
       0,     0,     0,   360,     0,     0,     0,     0,     0,     0,
       0,     0,   375,     0,     0,     0,   315,   370,     0,   372,
       0,   317,   318,   319,   320,     0,     0,    43,     0,     0,
      44,   316,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -159,     1,     0,     0,     0,     0,     0,
     336,     0,     0,     0,     0,     0,   342,     2,     0,   344,
       3,     4,     0,     0,     5,  -159,     7,     8,     9,    69,
      11,   358,    12,  -159,  -159,     0,    15,    16,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,     0,  -159,     0,  -159,
    -159,  -159,     0,    70,  -159,    26,  -159,  -159,  -159,  -159,
       0,     0,     0,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,    32,  -159,  -159,  -159,  -159,  -159,  -159,    33,
      34,    35,    36,    37,  -159,  -159,  -159,  -159,     0,  -159,
    -159,  -159,  -159,  -159,  -159,    41,    42,  -159,  -159,     1,
    -159,    74,  -159,  -159,    44,  -159,     0,     0,     0,     0,
       0,     0,     2,     0,   176,     3,     4,     0,     0,     5,
       0,     7,     8,     9,    69,   120,     0,   121,     0,     0,
       0,    15,    16,   140,   141,   142,   143,   144,   145,     0,
     146,   177,   178,   147,   148,   149,     0,     0,    70,     0,
      26,     0,     0,     0,     0,     0,     0,     0,     0,   150,
     151,   152,   153,   154,   155,   156,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   -60,     1,
      41,    42,     0,     0,     0,     0,    43,     0,     0,    44,
    -165,   122,     2,     0,     0,     3,     4,     0,     0,     5,
     -60,     7,     8,     9,    69,    11,     0,    12,   -60,   -60,
       0,    15,    16,     0,   -60,   -60,   -60,   -60,   -60,   -60,
     -60,     0,     0,     0,     0,   -60,     0,     0,    70,   -60,
      26,     0,     0,   -60,     0,     0,     0,     0,   -60,   -60,
     -60,   -60,   -60,   -60,   -60,   -60,   -60,    32,   -60,     0,
       0,   -60,   -60,   -60,    33,    34,    35,    36,    37,     0,
       0,   -60,   -60,     0,   -60,   -60,   -60,   -60,   -60,   -60,
      41,    42,   -60,   -60,     0,   -60,    78,  -159,     1,    44,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     2,     0,     0,     3,     4,     0,     0,     5,  -159,
       7,     8,     9,    69,    11,     0,    12,  -159,  -159,     0,
      15,    16,     0,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
       0,     0,     0,     0,  -159,     0,     0,    70,  -159,    26,
       0,     0,  -159,     0,     0,     0,     0,  -159,  -159,  -159,
       0,     0,     0,     0,     0,     0,    32,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,    37,     0,     0,
    -159,  -159,     0,  -159,  -159,  -159,  -159,     1,     0,    41,
      42,     0,     0,     0,  -159,    71,     0,  -159,    44,     0,
       2,     0,     0,     3,     4,     0,     0,     5,  -159,     7,
       8,     9,    69,    11,     0,    12,  -159,  -159,     0,    15,
      16,     0,  -159,  -159,  -159,  -159,  -159,  -159,  -159,     0,
       0,     0,     0,  -159,     0,     0,    70,  -159,    26,     0,
       0,  -159,     0,     0,     0,     0,  -159,  -159,  -159,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,  -159,
    -159,     0,  -159,  -159,  -159,    -4,     1,     0,    41,    42,
       0,     0,     0,  -159,    43,     0,  -159,    44,     0,     2,
       0,     0,     3,     4,     0,     0,     5,     6,     7,     8,
       9,    10,    11,     0,    12,     0,    13,    14,    15,    16,
       0,     0,     0,    17,    18,    19,    20,    21,     0,     0,
      22,     0,    23,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,    28,    29,    30,    31,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,    38,    39,
      40,  -148,     1,     0,     0,     0,     0,    41,    42,     0,
       0,     0,    -4,    43,     0,     2,    44,     0,     3,     4,
       0,     0,     5,     6,     7,     8,     9,    10,    11,     0,
      12,   340,    13,     0,    15,    16,     0,     0,     0,    17,
      18,    19,    20,    21,     0,     0,     0,     0,    23,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    29,    30,    31,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,    38,    39,  -146,     1,     0,     0,
       0,     0,     0,    41,    42,     0,     0,     0,   304,    43,
       2,     0,    44,     3,     4,     0,     0,     5,     6,     7,
       8,     9,    10,    11,     0,    12,     0,    13,     0,    15,
      16,     0,     0,     0,    17,    18,    19,    20,    21,     0,
       0,     0,     0,    23,     0,     0,    24,    25,    26,     0,
       0,    27,     0,     0,     0,     0,    29,    30,    31,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,    38,
      39,  -143,     1,     0,     0,     0,     0,     0,    41,    42,
       0,     0,     0,   304,    43,     2,     0,    44,     3,     4,
       0,     0,     5,     6,     7,     8,     9,    10,    11,     0,
      12,     0,    13,     0,    15,    16,     0,     0,     0,    17,
      18,    19,    20,    21,     0,     0,     0,     0,    23,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    29,    30,    31,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,    38,    39,     0,     1,     0,     0,
       0,     0,     0,    41,    42,     0,     0,     0,   304,    43,
       2,     0,    44,     3,     4,     0,     0,     5,     6,     7,
       8,     9,    10,    11,     0,    12,     0,    13,     0,    15,
      16,     0,   -48,   -48,    17,    18,    19,    20,    21,     0,
       0,     0,     0,    23,     0,     0,    24,    25,    26,     0,
       0,    27,     0,     0,     0,     0,    29,    30,    31,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,    38,
      39,     0,   -48,     1,     0,     0,     0,     0,    41,    42,
       0,     0,     0,   304,    43,     0,     2,    44,     0,     3,
       4,     0,     0,     5,     6,     7,     8,     9,    10,    11,
       0,    12,     0,    13,     0,    15,    16,     0,     0,     0,
      17,    18,    19,    20,    21,     0,     0,     0,     0,    23,
       0,     0,    24,    25,    26,     0,     0,    27,     0,     0,
       0,     0,    29,    30,    31,     0,     0,     0,     0,     0,
       0,    32,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     0,     0,    38,    39,     0,     1,   -51,
       0,     0,     0,     0,    41,    42,     0,     0,     0,   304,
      43,     2,     0,    44,     3,     4,     0,     0,     5,     6,
       7,     8,     9,    10,    11,     0,    12,     0,    13,     0,
      15,    16,     0,     0,     0,    17,    18,    19,    20,    21,
       0,     0,     0,     0,    23,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    29,    30,    31,
       0,     0,     0,     0,     0,     0,    32,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,    37,     0,     0,
      38,    39,     0,     1,     0,   -51,     0,     0,     0,    41,
      42,     0,     0,     0,   304,    43,     2,     0,    44,     3,
       4,     0,     0,     5,     6,     7,     8,     9,    10,    11,
       0,    12,     0,    13,     0,    15,    16,     0,     0,     0,
      17,    18,    19,    20,    21,     0,     0,     0,     0,    23,
       0,     0,    24,    25,    26,     0,     0,    27,     0,     0,
       0,     0,    29,    30,    31,     0,     0,     0,     0,     0,
       0,    32,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     0,     0,    38,    39,     0,   -50,     1,
       0,     0,     0,     0,    41,    42,     0,     0,     0,   304,
      43,     0,     2,    44,     0,     3,     4,     0,     0,     5,
       6,     7,     8,     9,    10,    11,     0,    12,     0,    13,
       0,    15,    16,     0,     0,     0,    17,    18,    19,    20,
      21,     0,     0,     0,     0,     0,     0,     0,    24,     0,
      26,     0,     0,     0,     0,     0,     0,     0,    29,    30,
      31,     0,     0,     0,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,     1,    39,     0,     0,     0,     0,     0,     0,     0,
      41,    42,     0,     0,     2,     0,    43,     3,     4,    44,
       0,     5,     6,     7,     8,     9,    10,    11,     0,    12,
       0,    13,     0,    15,    16,     0,     0,     0,    17,    18,
      19,    20,     0,     0,     0,     0,     0,     0,     0,     0,
      24,     0,    26,     0,     0,     0,     0,     0,     0,     0,
      29,    30,    31,     0,     0,     0,     0,     0,     0,    32,
       0,     0,     0,     0,     0,     0,    33,    34,    35,    36,
      37,     0,     0,     1,    39,     0,     0,     0,     0,     0,
       0,     0,    41,    42,     0,     0,     2,     0,    43,     3,
       4,    44,     0,     5,     0,     7,     8,     9,    69,    11,
       0,    12,     0,     0,     0,    15,    16,     0,     0,     0,
       1,     0,     0,     0,     0,     0,     0,     0,    85,    86,
      87,    88,    70,     2,    26,     0,     3,     4,     0,     0,
       5,     0,     7,     8,     9,    69,    11,     0,    12,     0,
       0,    32,    15,    16,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     0,     0,    85,     0,    87,     0,    70,
       0,    26,     0,     0,    41,    42,     0,     0,     0,     0,
      43,     0,     0,    44,     0,     0,     0,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,    41,    42,     0,     0,     2,     0,    43,     3,     4,
      44,     0,     5,     0,     7,     8,     9,    69,    11,     0,
      12,     0,     0,     0,    15,    16,     0,     0,     0,     1,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    70,     2,    26,     0,     3,     4,     0,    93,     5,
       0,     7,     8,     9,    69,    11,     0,    12,     0,     0,
      32,    15,    16,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,     0,     0,     0,     0,    70,     0,
      26,     0,     0,    41,    42,     0,     0,     0,     0,    43,
    -159,  -159,    44,     0,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,     1,     0,     0,     0,     0,     0,     0,     0,     0,
      41,    42,     0,     0,     2,     0,    43,     3,     4,    44,
       0,     5,     0,     7,     8,     9,    69,    11,     0,    12,
       0,     0,     0,    15,    16,     0,     0,     0,    85,     0,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      70,     0,    26,     0,     0,   140,   141,   142,   143,   144,
     145,     0,   146,     0,     0,   147,   148,   149,     0,    32,
       0,     0,     0,     0,     0,     0,    33,    34,    35,    36,
      37,   150,   151,   152,   153,   154,   155,   156,   192,     0,
       0,     0,    41,    42,   193,     0,     0,     0,    43,     0,
       0,    44,     0,   140,   141,   142,   143,   144,   145,     0,
     146,     0,     0,   147,   148,   149,     0,     0,     0,     0,
       0,   194,     0,     0,     0,     0,     0,   195,     0,   150,
     151,   152,   153,   154,   155,   156,   140,   141,   142,   143,
     144,   145,     0,   146,     0,     0,   147,   148,   149,     0,
       0,     0,     0,     0,   196,     0,     0,     0,     0,     0,
     197,     0,   150,   151,   152,   153,   154,   155,   156,   140,
     141,   142,   143,   144,   145,     0,   146,     0,     0,   147,
     148,   149,     0,     0,     0,     0,     0,   198,     0,     0,
       0,     0,     0,   199,     0,   150,   151,   152,   153,   154,
     155,   156,   140,   141,   142,   143,   144,   145,     0,   146,
       0,     0,   147,   148,   149,     0,     0,     0,     0,     0,
     297,     0,     0,     0,     0,     0,     0,     0,   150,   151,
     152,   153,   154,   155,   156,   140,   141,   142,   143,   144,
     145,     0,   146,     0,     0,   147,   148,   149,     0,     0,
       0,     0,     0,   298,     0,     0,     0,     0,     0,     0,
       0,   150,   151,   152,   153,   154,   155,   156,   140,   141,
     142,   143,   144,   145,     0,   146,     0,     0,   147,   148,
     149,     0,     0,     0,     0,     0,   299,     0,     0,     0,
       0,     0,     0,     0,   150,   151,   152,   153,   154,   155,
     156,   140,   141,   142,   143,   144,   145,     0,   146,     0,
       0,   147,   148,   149,     0,     0,     0,     0,     0,   300,
       0,     0,     0,     0,     0,     0,     0,   150,   151,   152,
     153,   154,   155,   156,   140,   141,   142,   143,   144,   145,
       0,   146,     0,     0,   147,   148,   149,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   202,     0,
     150,   151,   152,   153,   154,   155,   156,   140,   141,   142,
     143,   144,   145,   188,   146,     0,     0,   147,   148,   149,
     140,   141,   142,   143,   144,   145,     0,   146,     0,     0,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
       0,   203,     0,     0,     0,     0,   150,   151,   152,   153,
     154,   155,   156,   140,   141,   142,   143,   144,   145,     0,
     146,     0,     0,   147,   148,   149,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   150,
     151,   152,   153,   154,   155,   156,     0,     0,     0,   207,
     140,   141,   142,   143,   144,   145,     0,   146,     0,     0,
     147,   148,   149,     0,     0,     0,     0,     0,   200,   201,
       0,     0,     0,     0,     0,     0,   150,   151,   152,   153,
     154,   155,   156,   140,   141,   142,   143,   144,   145,     0,
     146,     0,     0,   147,   148,   149,   140,   141,   142,   143,
       0,     0,     0,   146,     0,     0,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,     0,     0,     0,     0,
       0,     0,   150,   151,   152,   153,   154,   155,   156
};

static const yytype_int16 yycheck[] =
{
       6,     7,     0,   168,    10,    82,   273,    30,   289,   134,
       4,   328,   219,    19,     8,    70,     6,     7,    24,    43,
      56,    57,    97,    29,    30,   100,    32,    33,    34,    35,
      36,    37,    38,    35,    97,    41,    42,    43,    44,    94,
      95,    65,    24,   310,    26,    51,    54,    53,    54,    27,
     331,    29,    54,    97,   131,   132,   100,    65,   375,    54,
      17,    18,    19,    70,    70,    71,    89,    54,    74,    26,
      65,    97,    78,    48,   100,    81,    33,    70,    65,    85,
      73,    71,   105,    77,    74,    92,    93,    94,    95,    99,
     100,    81,    51,   360,    76,    77,    78,    79,    91,    92,
      93,    94,    95,    51,    98,    99,    26,    99,   100,    26,
      99,   100,    36,    37,     0,    98,    97,    26,    46,    26,
     100,   100,    26,    26,    54,    26,   291,    53,    26,    26,
     136,   129,   103,   103,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   358,   158,   102,   100,   100,   162,   282,   283,   284,
     166,    26,   100,    48,   100,   100,   172,    26,    44,    53,
     247,   177,   178,    26,   180,   181,   301,   302,   184,   185,
     186,   175,   188,    26,    26,   274,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   285,    53,    26,   324,
       1,    88,    54,    54,    87,   211,    86,    36,    87,   215,
     216,   336,    86,    14,   208,   209,    17,    26,   307,   344,
      21,    26,   311,    86,   313,    26,    27,   100,    29,    87,
      53,   356,    87,   129,   203,    54,    86,   362,    86,   176,
      54,    54,    -1,    -1,   212,    -1,    -1,    -1,    -1,   255,
     256,   328,   341,    -1,    -1,    -1,   345,    -1,    -1,    -1,
     337,    -1,    -1,    -1,    -1,    -1,   355,    -1,   357,   346,
      -1,    -1,    -1,   350,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   371,    -1,    -1,    -1,   292,   364,    -1,   366,
      -1,   297,   298,   299,   300,    -1,    -1,    98,    -1,    -1,
     101,   295,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     0,     1,    -1,    -1,    -1,    -1,    -1,
     326,    -1,    -1,    -1,    -1,    -1,   332,    14,    -1,   335,
      17,    18,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,   347,    29,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    -1,    44,    -1,    46,
      47,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      -1,    -1,    -1,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     1,
      97,    98,    99,   100,   101,   102,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    -1,    44,    17,    18,    -1,    -1,    21,
      -1,    23,    24,    25,    26,    27,    -1,    29,    -1,    -1,
      -1,    33,    34,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    75,    -1,    -1,    50,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     0,     1,
      92,    93,    -1,    -1,    -1,    -1,    98,    -1,    -1,   101,
     102,   103,    14,    -1,    -1,    17,    18,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    30,    31,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,
      52,    -1,    -1,    55,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      -1,    83,    84,    -1,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    97,    98,     0,     1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    14,    -1,    -1,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    30,    31,    -1,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,
      -1,    -1,    55,    -1,    -1,    -1,    -1,    60,    61,    62,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,    -1,
      83,    84,    -1,    86,    87,    88,     0,     1,    -1,    92,
      93,    -1,    -1,    -1,    97,    98,    -1,   100,   101,    -1,
      14,    -1,    -1,    17,    18,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    29,    30,    31,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    60,    61,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    -1,    -1,    83,
      84,    -1,    86,    87,    88,     0,     1,    -1,    92,    93,
      -1,    -1,    -1,    97,    98,    -1,   100,   101,    -1,    14,
      -1,    -1,    17,    18,    -1,    -1,    21,    22,    23,    24,
      25,    26,    27,    -1,    29,    -1,    31,    32,    33,    34,
      -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,    -1,
      45,    -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,
      55,    -1,    -1,    -1,    59,    60,    61,    62,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    -1,    -1,    83,    84,
      85,     0,     1,    -1,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    97,    98,    -1,    14,   101,    -1,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    30,    31,    -1,    33,    34,    -1,    -1,    -1,    38,
      39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    -1,    -1,    83,    84,     0,     1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    97,    98,
      14,    -1,   101,    17,    18,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    29,    -1,    31,    -1,    33,
      34,    -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    60,    61,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    -1,    -1,    83,
      84,     0,     1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      -1,    -1,    -1,    97,    98,    14,    -1,   101,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    -1,    31,    -1,    33,    34,    -1,    -1,    -1,    38,
      39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    -1,    -1,    83,    84,    -1,     1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    97,    98,
      14,    -1,   101,    17,    18,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    29,    -1,    31,    -1,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    60,    61,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    80,    -1,    -1,    83,
      84,    -1,    86,     1,    -1,    -1,    -1,    -1,    92,    93,
      -1,    -1,    -1,    97,    98,    -1,    14,   101,    -1,    17,
      18,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      -1,    29,    -1,    31,    -1,    33,    34,    -1,    -1,    -1,
      38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    -1,    -1,    83,    84,    -1,     1,    87,
      -1,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    97,
      98,    14,    -1,   101,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    -1,    31,    -1,
      33,    34,    -1,    -1,    -1,    38,    39,    40,    41,    42,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,
      -1,    -1,    55,    -1,    -1,    -1,    -1,    60,    61,    62,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,    -1,
      83,    84,    -1,     1,    -1,    88,    -1,    -1,    -1,    92,
      93,    -1,    -1,    -1,    97,    98,    14,    -1,   101,    17,
      18,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      -1,    29,    -1,    31,    -1,    33,    34,    -1,    -1,    -1,
      38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    -1,    -1,    83,    84,    -1,    86,     1,
      -1,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    97,
      98,    -1,    14,   101,    -1,    17,    18,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    -1,    31,
      -1,    33,    34,    -1,    -1,    -1,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,
      -1,     1,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    -1,    -1,    14,    -1,    98,    17,    18,   101,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    29,
      -1,    31,    -1,    33,    34,    -1,    -1,    -1,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    -1,    -1,     1,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    -1,    -1,    14,    -1,    98,    17,
      18,   101,    -1,    21,    -1,    23,    24,    25,    26,    27,
      -1,    29,    -1,    -1,    -1,    33,    34,    -1,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    14,    52,    -1,    17,    18,    -1,    -1,
      21,    -1,    23,    24,    25,    26,    27,    -1,    29,    -1,
      -1,    69,    33,    34,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    -1,    -1,    46,    -1,    48,    -1,    50,
      -1,    52,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,
      98,    -1,    -1,   101,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    -1,    -1,    14,    -1,    98,    17,    18,
     101,    -1,    21,    -1,    23,    24,    25,    26,    27,    -1,
      29,    -1,    -1,    -1,    33,    34,    -1,    -1,    -1,     1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    14,    52,    -1,    17,    18,    -1,    20,    21,
      -1,    23,    24,    25,    26,    27,    -1,    29,    -1,    -1,
      69,    33,    34,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      52,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    98,
      99,   100,   101,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,
      -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    -1,    -1,    14,    -1,    98,    17,    18,   101,
      -1,    21,    -1,    23,    24,    25,    26,    27,    -1,    29,
      -1,    -1,    -1,    33,    34,    -1,    -1,    -1,    46,    -1,
      48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    52,    -1,    -1,    63,    64,    65,    66,    67,
      68,    -1,    70,    -1,    -1,    73,    74,    75,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    89,    90,    91,    92,    93,    94,    95,    48,    -1,
      -1,    -1,    92,    93,    54,    -1,    -1,    -1,    98,    -1,
      -1,   101,    -1,    63,    64,    65,    66,    67,    68,    -1,
      70,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    -1,    -1,    -1,    54,    -1,    89,
      90,    91,    92,    93,    94,    95,    63,    64,    65,    66,
      67,    68,    -1,    70,    -1,    -1,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,
      54,    -1,    89,    90,    91,    92,    93,    94,    95,    63,
      64,    65,    66,    67,    68,    -1,    70,    -1,    -1,    73,
      74,    75,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    54,    -1,    89,    90,    91,    92,    93,
      94,    95,    63,    64,    65,    66,    67,    68,    -1,    70,
      -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    63,    64,    65,    66,    67,
      68,    -1,    70,    -1,    -1,    73,    74,    75,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    63,    64,
      65,    66,    67,    68,    -1,    70,    -1,    -1,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    63,    64,    65,    66,    67,    68,    -1,    70,    -1,
      -1,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    63,    64,    65,    66,    67,    68,
      -1,    70,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      89,    90,    91,    92,    93,    94,    95,    63,    64,    65,
      66,    67,    68,    56,    70,    -1,    -1,    73,    74,    75,
      63,    64,    65,    66,    67,    68,    -1,    70,    -1,    -1,
      73,    74,    75,    89,    90,    91,    92,    93,    94,    95,
      -1,    97,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    63,    64,    65,    66,    67,    68,    -1,
      70,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    -1,    -1,    -1,    99,
      63,    64,    65,    66,    67,    68,    -1,    70,    -1,    -1,
      73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    63,    64,    65,    66,    67,    68,    -1,
      70,    -1,    -1,    73,    74,    75,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    -1,    -1,    73,    74,    75,    89,
      90,    91,    92,    93,    94,    95,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95
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
     112,   116,   117,   120,   124,   125,   126,   127,   128,   129,
     136,   138,   140,   144,   149,   152,    97,    26,   125,    26,
      50,    98,   126,   150,    98,   150,   125,    48,    98,   126,
     151,   139,   143,    51,   130,    46,    47,    48,    49,   126,
     137,   141,    51,    20,   126,    56,    57,    17,    18,    19,
      26,    33,    26,    26,   126,   126,   137,   132,   126,   126,
     126,   126,   126,   126,   126,   131,   145,   126,   126,   126,
      27,    29,   103,   126,   153,   154,   155,   156,     0,    97,
     126,    43,    65,   126,   109,   115,   120,   126,   129,   149,
      63,    64,    65,    66,    67,    68,    70,    73,    74,    75,
      89,    90,    91,    92,    93,    94,    95,   121,    98,   126,
     126,   150,   100,   150,   125,   150,   100,   150,   121,    26,
     133,   126,    46,   137,    26,    26,    44,    71,    72,    26,
      54,    65,   125,   125,    54,    65,    54,    53,    56,   137,
      26,   135,    48,    54,    48,    54,    48,    54,    48,    54,
      81,    82,    54,    97,    26,   134,    26,    99,   103,   103,
     102,   100,   100,   105,   122,   121,   121,   118,   122,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,    26,   146,    99,
     126,    99,    99,   126,   146,   100,   126,   142,   125,    26,
     128,   126,   126,   126,   126,    54,    65,    48,   126,   126,
     126,   126,   100,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   111,   111,   100,   125,   125,   126,    27,
      29,   156,   123,   126,   126,    53,   118,    97,   100,    97,
      26,   121,    44,   126,   126,    26,    26,    48,    48,    48,
      48,   108,   123,    26,    97,   110,   122,   122,   122,   123,
      53,   148,    26,   148,   146,   126,   125,   126,   126,   126,
     126,   122,   122,    87,   123,    35,    54,   119,   108,   123,
     123,    97,    54,    88,   122,    54,   126,   113,   119,   121,
      30,   148,   126,    87,   126,   122,    36,    37,   114,   121,
      36,    86,    26,   147,   123,   122,   123,   121,   126,    86,
     121,   100,   123,   122,   123,   118,   108,    26,   122,    87,
     121,    53,   121,    87,    86,   123,    86,   119
};

  /* YYR1YYN -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   104,   105,   105,   106,   106,   106,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     108,   108,   108,   109,   109,   109,   109,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   111,   112,   112,
     113,   113,   114,   115,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   123,   123,   124,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   127,   127,   127,
     127,   127,   127,   127,   127,   128,   128,   129,   129,   129,
     129,   129,   130,   129,   131,   129,   132,   129,   129,   129,
     129,   129,   133,   133,   134,   134,   135,   135,   136,   136,
     136,   136,   136,   136,   137,   137,   138,   138,   138,   139,
     138,   141,   142,   140,   140,   143,   140,   140,   140,   145,
     144,   146,   146,   146,   147,   147,   147,   148,   149,   150,
     150,   150,   151,   151,   152,   153,   153,   153,   153,   154,
     154,   155,   155,   156,   156
};

  /* YYR2YYN -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     4,     4,     6,
       4,     4,     4,     4,     4,     4,     5,     5,     8,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
      11,    12,     8,     2,     3,     7,     6,     0,     9,    12,
       0,     2,     6,    11,     7,     2,     3,     0,     0,     1,
       0,     0,     0,     2,     2,     3,     1,     1,     1,     1,
       1,     1,     3,     1,     2,     1,     1,     4,     2,     4,
       3,     2,     1,     1,     1,     3,     3,     3,     3,     3,
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
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1888 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* FBLTIN  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1894 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* RBLTIN  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1900 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* THEFBLTIN  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1906 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* ID  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1912 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* STRING  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1918 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* HANDLER  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1924 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* SYMBOL  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1930 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 30: /* ENDCLAUSE  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1936 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 31: /* tPLAYACCEL  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1942 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 32: /* tMETHOD  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1948 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 33: /* THEOBJECTFIELD  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).os; }
#line 1954 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 144: /* on  */
#line 160 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1960 "engines/director/lingo/lingo-gr.cpp"
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
#line 172 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2246 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 178 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2254 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 182 "engines/director/lingo/lingo-gr.y"
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
#line 2270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 193 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_after); }
#line 2276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 194 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_before); }
#line 2282 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 195 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2293 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 201 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2305 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 208 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2316 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 214 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2328 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 221 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 227 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2350 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 234 "engines/director/lingo/lingo-gr.y"
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
#line 2366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 245 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).oe);
		delete (yyvsp[-2].objectfield).os;
		(yyval.code) = (yyvsp[0].code); }
#line 2377 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 267 "engines/director/lingo/lingo-gr.y"
                                                                        {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[-3].code) - (yyvsp[-5].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 2] = end; }
#line 2388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 278 "engines/director/lingo/lingo-gr.y"
                                                                                                 {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[-8].code) - (yyvsp[-10].code));
		WRITE_UINT32(&finish, (yyvsp[-4].code) - (yyvsp[-10].code));
		WRITE_UINT32(&body, (yyvsp[-2].code) - (yyvsp[-10].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-10].code));
		WRITE_UINT32(&inc, 1);
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 1] = init;		/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 2] = finish;	/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 3] = body;		/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 4] = inc;		/* increment */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 5] = end; }
#line 2405 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 295 "engines/director/lingo/lingo-gr.y"
                                                                                                       {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[-9].code) - (yyvsp[-11].code));
		WRITE_UINT32(&finish, (yyvsp[-4].code) - (yyvsp[-11].code));
		WRITE_UINT32(&body, (yyvsp[-2].code) - (yyvsp[-11].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-11].code));
		WRITE_UINT32(&inc, (uint32)-1);
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 1] = init;		/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 2] = finish;	/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 3] = body;		/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 4] = inc;		/* increment */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 5] = end; }
#line 2422 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 307 "engines/director/lingo/lingo-gr.y"
                                                                            {
		inst list = 0, body = 0, end = 0;
		WRITE_UINT32(&list, (yyvsp[-5].code) - (yyvsp[-7].code));
		WRITE_UINT32(&body, (yyvsp[-2].code) - (yyvsp[-7].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-7].code));
		(*g_lingo->_currentScript)[(yyvsp[-7].code) + 1] = list;		/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[-7].code) + 2] = 0;		/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-7].code) + 3] = body;		/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-7].code) + 4] = 0;		/* increment */
		(*g_lingo->_currentScript)[(yyvsp[-7].code) + 5] = end; }
#line 2437 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 318 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_nextRepeat); }
#line 2444 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 320 "engines/director/lingo/lingo-gr.y"
                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2454 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 325 "engines/director/lingo/lingo-gr.y"
                                                          {
		inst end;
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code));
		(*g_lingo->_currentScript)[(yyvsp[-3].code) + 1] = end; }
#line 2463 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 329 "engines/director/lingo/lingo-gr.y"
                                                    {
		inst end;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2472 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 334 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }
#line 2480 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 338 "engines/director/lingo/lingo-gr.y"
                                                                                           {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].narg) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code));
		(*g_lingo->_currentScript)[(yyvsp[-6].narg)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-3].narg)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].narg), (yyvsp[-1].code)); }
#line 2492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 345 "engines/director/lingo/lingo-gr.y"
                                                                                                                  {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-6].narg) +1);
		WRITE_UINT32(&end, (yyvsp[-1].code));
		(*g_lingo->_currentScript)[(yyvsp[-9].narg)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-6].narg)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-6].narg), (yyvsp[-1].code)); }
#line 2504 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 356 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].narg) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-3].narg)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].narg)); }
#line 2514 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 362 "engines/director/lingo/lingo-gr.y"
                                                                                                             {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].narg) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code));
		(*g_lingo->_currentScript)[(yyvsp[-8].narg)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-5].narg)] = end;	}
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 368 "engines/director/lingo/lingo-gr.y"
                                                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[-1].code));

		(*g_lingo->_currentScript)[(yyvsp[-4].narg)] = end; }
#line 2535 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 374 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code3(LC::c_repeatwhilecode, STOP, STOP); }
#line 2541 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 376 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code3(LC::c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2551 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 382 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, STOP);
		(yyval.narg) = g_lingo->_currentScript->size() - 1; }
#line 2559 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 386 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, STOP);
		(yyval.narg) = g_lingo->_currentScript->size() - 1; }
#line 2567 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 390 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2574 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 393 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2580 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 395 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2586 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 397 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2592 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 401 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2602 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 407 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2610 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 410 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 413 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2627 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 417 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 421 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2645 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 425 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2658 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 433 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2664 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 435 "engines/director/lingo/lingo-gr.y"
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
#line 2679 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 446 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2685 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 448 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2693 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 451 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2701 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 454 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 457 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 460 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 466 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).oe);
		delete (yyvsp[0].objectfield).os; }
#line 2738 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 471 "engines/director/lingo/lingo-gr.y"
                       {
		g_lingo->code1(LC::c_objectrefpush);
		g_lingo->codeString((yyvsp[0].objectref).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectref).field->c_str());
		delete (yyvsp[0].objectref).obj;
		delete (yyvsp[0].objectref).field; }
#line 2749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 478 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2755 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 479 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2761 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 480 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2767 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 481 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 482 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2779 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 483 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2785 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 484 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2791 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 485 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2797 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 486 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2803 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 487 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2809 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 488 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2815 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 489 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2821 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 490 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2827 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 491 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2833 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 492 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2839 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 493 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2845 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 494 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2851 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 495 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 496 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); }
#line 2863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 497 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2869 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 498 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_intersects); }
#line 2875 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 499 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 2881 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 501 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charOf); }
#line 2887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 502 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charToOf); }
#line 2893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 503 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemOf); }
#line 2899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 504 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_itemToOf); }
#line 2905 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 505 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineOf); }
#line 2911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 506 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_lineToOf); }
#line 2917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 507 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordOf); }
#line 2923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 508 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_wordToOf); }
#line 2929 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 510 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2937 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 515 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_printtop); }
#line 2943 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 518 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_exitRepeat); }
#line 2949 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 519 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 2955 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 520 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 2961 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 520 "engines/director/lingo/lingo-gr.y"
                                                                                 { inNone(); }
#line 2967 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 521 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 2973 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 521 "engines/director/lingo/lingo-gr.y"
                                                                                   { inNone(); }
#line 2979 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 522 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 2985 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 522 "engines/director/lingo/lingo-gr.y"
                                                                                   { inNone(); }
#line 2991 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 523 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2999 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 526 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 529 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_open); }
#line 3013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 530 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 3019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 532 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3029 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 537 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3039 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 543 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 548 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3059 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 554 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3069 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 559 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 572 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 573 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 574 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3097 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 575 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3106 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 579 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3115 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 583 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 591 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 595 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 599 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3151 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 603 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3157 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 603 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3166 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 633 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 633 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->_currentFactory.clear(); }
#line 3178 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 634 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-6].s); }
#line 3188 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 639 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3194 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 640 "engines/director/lingo/lingo-gr.y"
                  { startDef(); }
#line 3200 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 641 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
		endDef();
		delete (yyvsp[-6].s); }
#line 3210 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 646 "engines/director/lingo/lingo-gr.y"
                                                                     {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3223 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 654 "engines/director/lingo/lingo-gr.y"
                                                 {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3233 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 660 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3239 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 660 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.s) = (yyvsp[0].s); g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 3246 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 663 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 664 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mArg((yyvsp[0].s)); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3258 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 665 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); mArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3264 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 668 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 669 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 671 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArgStore(); inDef(); }
#line 3282 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 673 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 3294 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 681 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3300 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 682 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3306 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 683 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3312 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 685 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3318 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 686 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3324 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 688 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3330 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 690 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3336 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 691 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3342 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 692 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3348 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 693 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3354 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 695 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3360 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 696 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 698 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172:
#line 699 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3378 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173:
#line 701 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3387 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174:
#line 705 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3396 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3400 "engines/director/lingo/lingo-gr.cpp"

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

#line 711 "engines/director/lingo/lingo-gr.y"


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
