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
  YYSYMBOL_111_1 = 111,                    /* $@1  */
  YYSYMBOL_112_2 = 112,                    /* $@2  */
  YYSYMBOL_113_3 = 113,                    /* $@3  */
  YYSYMBOL_tellstart = 114,                /* tellstart  */
  YYSYMBOL_ifstmt = 115,                   /* ifstmt  */
  YYSYMBOL_elseifstmtlist = 116,           /* elseifstmtlist  */
  YYSYMBOL_elseifstmt = 117,               /* elseifstmt  */
  YYSYMBOL_ifoneliner = 118,               /* ifoneliner  */
  YYSYMBOL_repeatwhile = 119,              /* repeatwhile  */
  YYSYMBOL_repeatwith = 120,               /* repeatwith  */
  YYSYMBOL_jumpifz = 121,                  /* jumpifz  */
  YYSYMBOL_jump = 122,                     /* jump  */
  YYSYMBOL_varassign = 123,                /* varassign  */
  YYSYMBOL_if = 124,                       /* if  */
  YYSYMBOL_begin = 125,                    /* begin  */
  YYSYMBOL_end = 126,                      /* end  */
  YYSYMBOL_stmtlist = 127,                 /* stmtlist  */
  YYSYMBOL_when = 128,                     /* when  */
  YYSYMBOL_simpleexpr = 129,               /* simpleexpr  */
  YYSYMBOL_expr = 130,                     /* expr  */
  YYSYMBOL_chunkexpr = 131,                /* chunkexpr  */
  YYSYMBOL_reference = 132,                /* reference  */
  YYSYMBOL_proc = 133,                     /* proc  */
  YYSYMBOL_134_4 = 134,                    /* $@4  */
  YYSYMBOL_135_5 = 135,                    /* $@5  */
  YYSYMBOL_136_6 = 136,                    /* $@6  */
  YYSYMBOL_globallist = 137,               /* globallist  */
  YYSYMBOL_propertylist = 138,             /* propertylist  */
  YYSYMBOL_instancelist = 139,             /* instancelist  */
  YYSYMBOL_gotofunc = 140,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 141,                /* gotomovie  */
  YYSYMBOL_playfunc = 142,                 /* playfunc  */
  YYSYMBOL_143_7 = 143,                    /* $@7  */
  YYSYMBOL_defn = 144,                     /* defn  */
  YYSYMBOL_145_8 = 145,                    /* $@8  */
  YYSYMBOL_146_9 = 146,                    /* $@9  */
  YYSYMBOL_147_10 = 147,                   /* $@10  */
  YYSYMBOL_on = 148,                       /* on  */
  YYSYMBOL_149_11 = 149,                   /* $@11  */
  YYSYMBOL_argdef = 150,                   /* argdef  */
  YYSYMBOL_endargdef = 151,                /* endargdef  */
  YYSYMBOL_argstore = 152,                 /* argstore  */
  YYSYMBOL_macro = 153,                    /* macro  */
  YYSYMBOL_arglist = 154,                  /* arglist  */
  YYSYMBOL_nonemptyarglist = 155,          /* nonemptyarglist  */
  YYSYMBOL_list = 156,                     /* list  */
  YYSYMBOL_valuelist = 157,                /* valuelist  */
  YYSYMBOL_linearlist = 158,               /* linearlist  */
  YYSYMBOL_proplist = 159,                 /* proplist  */
  YYSYMBOL_proppair = 160                  /* proppair  */
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
#define YYLAST   2498

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  57
/* YYNRULES -- Number of rules.  */
#define YYNRULES  178
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
       0,   166,   166,   167,   169,   170,   171,   173,   179,   183,
     194,   195,   196,   202,   209,   215,   222,   228,   235,   246,
     253,   254,   255,   257,   258,   259,   260,   262,   263,   268,
     280,   283,   286,   279,   309,   326,   337,   339,   344,   348,
     353,   357,   364,   372,   373,   375,   381,   387,   393,   395,
     401,   405,   409,   413,   416,   418,   420,   421,   422,   424,
     430,   433,   436,   440,   444,   448,   456,   457,   458,   469,
     470,   471,   474,   477,   480,   483,   489,   494,   500,   501,
     502,   503,   504,   505,   506,   507,   508,   509,   510,   511,
     512,   513,   514,   515,   516,   517,   518,   519,   520,   521,
     522,   524,   525,   526,   527,   528,   529,   530,   531,   533,
     536,   538,   539,   540,   541,   542,   543,   543,   544,   544,
     545,   545,   546,   549,   552,   553,   555,   560,   566,   571,
     577,   582,   595,   596,   597,   598,   602,   606,   611,   612,
     614,   618,   622,   626,   626,   656,   656,   656,   662,   663,
     663,   669,   677,   683,   683,   686,   687,   688,   690,   691,
     692,   694,   696,   704,   705,   706,   708,   709,   711,   713,
     714,   715,   716,   718,   719,   721,   722,   724,   728
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
  "stmtonelinerwithif", "stmt", "$@1", "$@2", "$@3", "tellstart", "ifstmt",
  "elseifstmtlist", "elseifstmt", "ifoneliner", "repeatwhile",
  "repeatwith", "jumpifz", "jump", "varassign", "if", "begin", "end",
  "stmtlist", "when", "simpleexpr", "expr", "chunkexpr", "reference",
  "proc", "$@4", "$@5", "$@6", "globallist", "propertylist",
  "instancelist", "gotofunc", "gotomovie", "playfunc", "$@7", "defn",
  "$@8", "$@9", "$@10", "on", "$@11", "argdef", "endargdef", "argstore",
  "macro", "arglist", "nonemptyarglist", "list", "valuelist", "linearlist",
  "proplist", "proppair", YY_NULLPTR
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

#define YYPACT_NINF (-283)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-170)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACTSTATE-NUM -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     785,   -81,  -283,  -283,    60,  -283,   607,   323,    60,   -27,
     508,  -283,  -283,  -283,  -283,  -283,  -283,   -26,  -283,  1802,
    -283,  -283,  -283,    12,  1958,    34,   280,    17,    52,  2040,
    1839,  -283,  2040,  2040,  2040,  2040,  2040,  2040,  2040,  -283,
    -283,  2040,  2040,  2040,   418,    70,   -17,  -283,  -283,  -283,
    -283,  2040,   -24,  2040,  1638,  -283,  2390,  -283,  -283,  -283,
    -283,  -283,  -283,  -283,  -283,  -283,  -283,  -283,  -283,   -14,
    1958,  1921,  2390,    -7,  1921,    -7,  -283,    60,  1921,  2390,
      -5,   696,  -283,  -283,    71,  2040,  -283,    57,  -283,  2032,
    -283,    90,  -283,    92,   390,    98,  -283,   -45,    60,    60,
     -43,    72,    58,  -283,  2324,  2032,  -283,   101,  -283,  2080,
    2113,  2146,  2179,  2357,   191,   102,   103,  -283,  -283,   133,
      30,    48,  -283,  2390,    53,    54,    59,  -283,  -283,   785,
    2390,  -283,  -283,  2390,  -283,  -283,  2040,  2390,  -283,  -283,
    2040,  2040,  2040,  2040,  2040,  2040,  2040,  2040,  2040,  2040,
    2040,  2040,  2040,  2040,  2040,  2040,  2040,   131,  1921,   390,
     133,     0,  2040,     7,  -283,    10,  2040,    -7,   131,  -283,
      62,  2390,  2040,  -283,  -283,    60,    36,  2040,  2040,    99,
    2040,  2040,    -8,   115,  2040,  2040,  2040,  -283,  2040,  -283,
    -283,    65,  2040,  2040,  2040,  2040,  2040,  2040,  2040,  2040,
    2040,  2040,  -283,  -283,  -283,    67,  -283,  -283,    60,    60,
    -283,  2040,    44,  -283,  -283,  2040,  2040,   120,  -283,  2390,
     143,   143,   143,   143,  2403,  2403,  -283,   148,   143,   143,
     143,   143,   148,   -28,   -28,  -283,  -283,  -283,   -85,  -283,
    2390,  -283,  -283,  2390,   -48,   150,  2390,  -283,   140,  -283,
    -283,  2390,  2390,  2040,  2390,   143,  2040,  2040,   151,  2390,
     143,  2390,  2390,   176,  2390,  2212,  2390,  2245,  2390,  2278,
    2390,  2311,  2390,  2390,  1720,  -283,   178,  -283,  -283,  2390,
      30,    48,  -283,  1212,  2390,  2390,  -283,   158,  -283,   186,
    -283,  -283,   131,  2040,  2390,  2390,   143,    60,  -283,  2040,
    2040,  2040,  2040,  -283,  1297,  -283,  -283,  -283,   130,  -283,
     184,  1126,  1720,  -283,  -283,  -283,   -44,  2390,  -283,   166,
    2390,  2390,  2390,  2390,  -283,   142,  -283,  1382,   167,  -283,
     145,   871,   956,  -283,  -283,  2040,  -283,   146,  2040,    85,
     208,   160,   222,  -283,   195,  2390,  -283,  2390,  -283,  2040,
    -283,   174,  -283,  -283,  -283,   152,  1041,  2040,  -283,  -283,
    2390,  -283,  1720,   237,  2390,  1382,  1467,   214,  -283,  -283,
    -283,   181,   183,  -283,   187,  -283,  -283,  -283,  1126,  -283,
    1553,  -283,  -283
};

  /* YYDEFACTSTATE-NUM -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    60,    65,     0,    61,     0,     0,     0,     0,
       0,    63,    62,   143,   149,    76,    77,   115,   116,     0,
     132,    53,   145,     0,     0,     0,     0,     0,     0,     0,
       0,   120,     0,     0,     0,     0,     0,     0,     0,   118,
     153,     0,     0,     0,     0,     0,     2,    78,    27,     6,
      28,     0,     0,     0,     0,    69,    21,   110,    70,    22,
     112,   113,     5,    54,    20,    67,    68,    64,    75,    64,
       0,     0,   164,   123,     0,    72,   109,     0,     0,   166,
     162,     0,    54,   114,     0,     0,   133,     0,   134,   135,
     137,     0,    36,     0,   111,     0,    48,     0,     0,     0,
       0,     0,     0,   148,   125,   140,   142,     0,    92,     0,
       0,     0,     0,     0,     0,     0,     0,    97,    98,     0,
      63,    62,   170,   173,     0,   171,   172,   175,     1,     0,
      50,    54,    54,    50,    55,    26,     0,    24,    25,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   155,     0,     0,
     164,     0,     0,     0,    74,     0,     0,   144,   155,   126,
     117,   139,     0,   136,   146,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    59,     0,   141,
     130,   121,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    40,    40,   128,   119,   154,    66,     0,     0,
     168,     0,     0,     3,    56,     0,     0,     0,    37,    50,
      88,    89,    86,    87,    90,    91,    83,    94,    95,    96,
      85,    84,    93,    79,    80,    81,    82,   156,     0,   122,
     165,    71,    73,   167,     0,     0,   138,    54,     0,     7,
       8,    10,    11,     0,    15,    13,     0,     0,     0,    14,
      12,    19,   124,     0,   101,     0,   103,     0,   105,     0,
     107,     0,    99,   100,     0,    56,     0,   178,   177,   174,
       0,     0,   176,     0,    55,    55,    56,     0,   161,     0,
     161,   127,   155,     0,    30,    16,    17,     0,   131,     0,
       0,     0,     0,    55,     0,   129,    57,    58,     0,    56,
       0,     0,     0,    56,   157,    56,     0,     9,    52,     0,
     102,   104,   106,   108,    39,     0,    29,     0,     0,    43,
      51,     0,     0,   161,    31,     0,    38,     0,     0,    54,
       0,     0,   158,    56,     0,    18,    35,    55,    54,     0,
      44,     0,    54,    47,   159,   151,     0,     0,    56,    56,
      50,    41,     0,     0,    32,     0,     0,     0,    54,   160,
      50,     0,     0,    56,     0,    56,    34,    42,     0,    46,
       0,    45,    33
};

  /* YYPGOTONTERM-NUM.  */
static const yytype_int16 yypgoto[] =
{
    -283,   141,  -283,  -283,  -268,  -283,     3,  -283,  -283,  -283,
      69,  -283,  -283,  -283,  -283,  -283,  -283,  -131,  -261,  -283,
     220,   -77,  -245,  -190,  -283,     6,    -6,  -283,   100,   223,
    -283,  -283,  -283,  -283,  -283,  -283,  -283,   -13,  -283,  -283,
    -283,  -283,  -283,  -283,  -283,  -283,  -161,  -283,  -282,   224,
      27,  -283,  -283,  -283,  -283,  -283,    78
};

  /* YYDEFGOTONTERM-NUM.  */
static const yytype_int16 yydefgoto[] =
{
      -1,    45,    46,    47,    48,   134,   307,   318,   344,   370,
     274,    50,   339,   350,   135,    51,    52,   214,   308,   334,
      53,   157,   218,   283,    54,    55,    56,    57,    58,    59,
      84,   115,   107,   170,   205,   191,    60,    90,    61,    81,
      62,    91,   247,    82,    63,   116,   238,   355,   313,    64,
     165,    80,    65,   124,   125,   126,   127
};

  /* YYTABLEYYPACT[STATE-NUM] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      72,    72,   217,    49,    79,   168,   303,   244,   315,   180,
      68,   184,   288,    89,    76,   289,    66,   106,    94,   131,
     181,    77,   185,   104,   105,    83,   108,   109,   110,   111,
     112,   113,   114,    73,    75,   117,   118,   119,   123,   309,
     310,   132,   146,   102,   330,   130,   256,   133,   137,   290,
     329,   343,   289,   333,   215,   216,   289,   257,   324,   325,
       8,     1,   249,    92,   159,   160,   155,   156,   160,   340,
     128,   280,   160,   281,     2,    72,   173,     3,   103,   171,
     129,     5,   337,   164,   158,   304,    67,    11,   287,    12,
      95,    96,   189,   162,   368,   166,   311,   169,   161,   239,
     162,   163,   358,   172,   182,   183,   241,   162,   167,   242,
     162,   187,    33,    34,    35,    36,   174,   381,   175,   327,
     371,   348,   349,   331,   179,   332,   186,   190,   204,   206,
     219,   316,    49,   208,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   209,    72,   356,   211,   210,   240,   237,    43,   212,
     243,    44,   245,   258,   253,   263,   246,   276,   365,   366,
     292,   251,   252,   286,   254,   255,   291,   297,   259,   260,
     261,   248,   262,   378,   293,   380,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   140,   141,   142,   143,
     144,   145,   298,   146,   305,   279,   147,   148,   149,   284,
     285,   312,   314,   146,   277,   278,   147,   326,   146,   328,
     335,   338,   150,   151,   152,   153,   154,   155,   156,   367,
     336,   -54,   207,   346,   152,   153,   154,   155,   156,   375,
     153,   154,   155,   156,   352,   202,   353,   294,   354,   357,
     295,   296,   363,   341,   140,   141,   142,   143,   144,   145,
     361,   146,   351,   369,   147,   148,   149,   373,   376,   377,
     213,   359,   275,   379,   136,   362,   250,   138,   139,     0,
     150,   151,   152,   153,   154,   155,   156,   317,   203,   372,
     282,   374,     0,   320,   321,   322,   323,    97,    98,    99,
       0,     0,     0,   319,     0,     0,   100,     0,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -163,     1,     0,     0,     0,     0,   345,
       0,     0,   347,     0,     0,     0,     0,     2,     0,     0,
       3,     4,     0,   360,     5,  -163,     7,     8,     9,    69,
      11,   364,    12,  -163,  -163,     0,    15,    16,  -163,  -163,
    -163,  -163,  -163,  -163,  -163,  -163,     0,  -163,     0,  -163,
    -163,  -163,     0,    70,  -163,    26,  -163,  -163,  -163,  -163,
       0,     0,     0,  -163,  -163,  -163,  -163,  -163,  -163,  -163,
    -163,  -163,    32,  -163,  -163,  -163,  -163,  -163,  -163,    33,
      34,    35,    36,    37,  -163,  -163,  -163,  -163,     0,  -163,
    -163,  -163,  -163,  -163,  -163,    41,    42,  -163,  -163,     1,
    -163,    74,  -163,  -163,    44,  -163,     0,     0,     0,     0,
       0,     0,     2,     0,   176,     3,     4,     0,     0,     5,
       0,     7,     8,     9,    69,   120,     0,   121,     0,     0,
       0,    15,    16,   140,   141,   142,   143,   144,   145,     0,
     146,   177,   178,   147,   148,   149,     0,     0,    70,     0,
      26,     0,     0,     0,     0,     0,     0,     0,     0,   150,
     151,   152,   153,   154,   155,   156,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   -64,     1,
      41,    42,     0,     0,     0,     0,    43,     0,     0,    44,
    -169,   122,     2,     0,     0,     3,     4,     0,     0,     5,
     -64,     7,     8,     9,    69,    11,     0,    12,   -64,   -64,
       0,    15,    16,     0,   -64,   -64,   -64,   -64,   -64,   -64,
     -64,     0,     0,     0,     0,   -64,     0,     0,    70,   -64,
      26,     0,     0,   -64,     0,     0,     0,     0,   -64,   -64,
     -64,   -64,   -64,   -64,   -64,   -64,   -64,    32,   -64,     0,
       0,   -64,   -64,   -64,    33,    34,    35,    36,    37,     0,
       0,   -64,   -64,     0,   -64,   -64,   -64,   -64,   -64,   -64,
      41,    42,   -64,   -64,     0,   -64,    78,  -163,     1,    44,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     2,     0,     0,     3,     4,     0,     0,     5,  -163,
       7,     8,     9,    69,    11,     0,    12,  -163,  -163,     0,
      15,    16,     0,  -163,  -163,  -163,  -163,  -163,  -163,  -163,
       0,     0,     0,     0,  -163,     0,     0,    70,  -163,    26,
       0,     0,  -163,     0,     0,     0,     0,  -163,  -163,  -163,
       0,     0,     0,     0,     0,     0,    32,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,    37,     0,     0,
    -163,  -163,     0,  -163,  -163,  -163,  -163,     1,     0,    41,
      42,     0,     0,     0,  -163,    71,     0,  -163,    44,     0,
       2,     0,     0,     3,     4,     0,     0,     5,  -163,     7,
       8,     9,    69,    11,     0,    12,  -163,  -163,     0,    15,
      16,     0,  -163,  -163,  -163,  -163,  -163,  -163,  -163,     0,
       0,     0,     0,  -163,     0,     0,    70,  -163,    26,     0,
       0,  -163,     0,     0,     0,     0,  -163,  -163,  -163,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,  -163,
    -163,     0,  -163,  -163,  -163,    -4,     1,     0,    41,    42,
       0,     0,     0,  -163,    43,     0,  -163,    44,     0,     2,
       0,     0,     3,     4,     0,     0,     5,     6,     7,     8,
       9,    10,    11,     0,    12,     0,    13,    14,    15,    16,
       0,     0,     0,    17,    18,    19,    20,    21,     0,     0,
      22,     0,    23,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,    28,    29,    30,    31,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,    38,    39,
      40,  -152,     1,     0,     0,     0,     0,    41,    42,     0,
       0,     0,    -4,    43,     0,     2,    44,     0,     3,     4,
       0,     0,     5,     6,     7,     8,     9,    10,    11,     0,
      12,   342,    13,     0,    15,    16,     0,     0,     0,    17,
      18,    19,    20,    21,     0,     0,     0,     0,    23,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    29,    30,    31,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,    38,    39,  -150,     1,     0,     0,
       0,     0,     0,    41,    42,     0,     0,     0,   306,    43,
       2,     0,    44,     3,     4,     0,     0,     5,     6,     7,
       8,     9,    10,    11,     0,    12,     0,    13,     0,    15,
      16,     0,     0,     0,    17,    18,    19,    20,    21,     0,
       0,     0,     0,    23,     0,     0,    24,    25,    26,     0,
       0,    27,     0,     0,     0,     0,    29,    30,    31,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,    38,
      39,  -147,     1,     0,     0,     0,     0,     0,    41,    42,
       0,     0,     0,   306,    43,     2,     0,    44,     3,     4,
       0,     0,     5,     6,     7,     8,     9,    10,    11,     0,
      12,     0,    13,     0,    15,    16,     0,     0,     0,    17,
      18,    19,    20,    21,     0,     0,     0,     0,    23,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    29,    30,    31,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     0,     0,    38,    39,     0,     1,     0,     0,
       0,     0,     0,    41,    42,     0,     0,     0,   306,    43,
       2,     0,    44,     3,     4,     0,     0,     5,     6,     7,
       8,     9,    10,    11,     0,    12,     0,    13,     0,    15,
      16,     0,   -51,   -51,    17,    18,    19,    20,    21,     0,
       0,     0,     0,    23,     0,     0,    24,    25,    26,     0,
       0,    27,     0,     0,     0,     0,    29,    30,    31,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,    38,
      39,     0,   -51,     1,     0,     0,     0,     0,    41,    42,
       0,     0,     0,   306,    43,     0,     2,    44,     0,     3,
       4,     0,     0,     5,     6,     7,     8,     9,    10,    11,
       0,    12,     0,    13,     0,    15,    16,     0,     0,     0,
      17,    18,    19,    20,    21,     0,     0,     0,     0,    23,
       0,     0,    24,    25,    26,     0,     0,    27,     0,     0,
       0,     0,    29,    30,    31,     0,     0,     0,     0,     0,
       0,    32,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     0,     0,    38,    39,     0,     1,   -51,
       0,     0,     0,     0,    41,    42,     0,     0,     0,   306,
      43,     2,     0,    44,     3,     4,     0,     0,     5,     6,
       7,     8,     9,    10,    11,     0,    12,     0,    13,     0,
      15,    16,     0,     0,     0,    17,    18,    19,    20,    21,
       0,     0,     0,     0,    23,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    29,    30,    31,
       0,     0,     0,     0,     0,     0,    32,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,    37,     0,     0,
      38,    39,     0,     1,     0,   -55,     0,     0,     0,    41,
      42,     0,     0,     0,   306,    43,     2,     0,    44,     3,
       4,     0,     0,     5,     6,     7,     8,     9,    10,    11,
       0,    12,     0,    13,     0,    15,    16,     0,     0,     0,
      17,    18,    19,    20,    21,     0,     0,     0,     0,    23,
       0,     0,    24,    25,    26,     0,     0,    27,     0,     0,
       0,     0,    29,    30,    31,     0,     0,     0,     0,     0,
       0,    32,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     0,     0,    38,    39,     0,     1,   -55,
       0,     0,     0,     0,    41,    42,     0,     0,     0,   306,
      43,     2,     0,    44,     3,     4,     0,     0,     5,     6,
       7,     8,     9,    10,    11,     0,    12,     0,    13,     0,
      15,    16,     0,     0,     0,    17,    18,    19,    20,    21,
       0,     0,     0,     0,    23,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    29,    30,    31,
       0,     0,     0,     0,     0,     0,    32,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,    37,     0,     0,
      38,    39,     0,   -54,     1,     0,     0,     0,     0,    41,
      42,     0,     0,     0,   306,    43,     0,     2,    44,     0,
       3,     4,     0,     0,     5,     6,     7,     8,     9,    10,
      11,     0,    12,     0,    13,     0,    15,    16,     0,     0,
       0,    17,    18,    19,    20,    21,     0,     0,     0,     0,
      23,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    29,    30,    31,     0,     0,     0,     0,
       0,     0,    32,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,    37,     0,     0,    38,    39,     0,     1,
     382,     0,     0,     0,     0,    41,    42,     0,     0,     0,
     306,    43,     2,     0,    44,     3,     4,     0,     0,     5,
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
    -163,  -163,    44,     0,     0,     0,     0,    32,     0,     0,
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
     299,     0,     0,     0,     0,     0,     0,     0,   150,   151,
     152,   153,   154,   155,   156,   140,   141,   142,   143,   144,
     145,     0,   146,     0,     0,   147,   148,   149,     0,     0,
       0,     0,     0,   300,     0,     0,     0,     0,     0,     0,
       0,   150,   151,   152,   153,   154,   155,   156,   140,   141,
     142,   143,   144,   145,     0,   146,     0,     0,   147,   148,
     149,     0,     0,     0,     0,     0,   301,     0,     0,     0,
       0,     0,     0,     0,   150,   151,   152,   153,   154,   155,
     156,   140,   141,   142,   143,   144,   145,     0,   146,     0,
       0,   147,   148,   149,     0,     0,     0,     0,     0,   302,
       0,     0,     0,     0,     0,     0,     0,   150,   151,   152,
     153,   154,   155,   156,   140,   141,   142,   143,   144,   145,
     188,   146,     0,     0,   147,   148,   149,   140,   141,   142,
     143,   144,   145,     0,   146,     0,     0,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,     0,     0,     0,
       0,     0,     0,   150,   151,   152,   153,   154,   155,   156,
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
       6,     7,   133,     0,    10,    82,   274,   168,   290,    54,
       4,    54,    97,    19,     8,   100,    97,    30,    24,    43,
      65,    48,    65,    29,    30,    51,    32,    33,    34,    35,
      36,    37,    38,     6,     7,    41,    42,    43,    44,   284,
     285,    65,    70,    26,   312,    51,    54,    53,    54,    97,
     311,   333,   100,    97,   131,   132,   100,    65,   303,   304,
      24,     1,    26,    51,    70,    71,    94,    95,    74,   330,
       0,    27,    78,    29,    14,    81,    89,    17,    26,    85,
      97,    21,   327,    77,    98,   275,    26,    27,   219,    29,
      56,    57,   105,   100,   362,   100,   286,    26,    71,    99,
     100,    74,   347,    46,    98,    99,    99,   100,    81,    99,
     100,    53,    76,    77,    78,    79,    26,   378,    26,   309,
     365,    36,    37,   313,    26,   315,    54,    26,    26,    26,
     136,   292,   129,   103,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   103,   158,   343,   100,   102,   162,    26,    98,   100,
     166,   101,   100,    48,    65,   100,   172,   100,   358,   359,
     247,   177,   178,    53,   180,   181,    26,    26,   184,   185,
     186,   175,   188,   373,    44,   375,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,    63,    64,    65,    66,
      67,    68,    26,    70,    26,   211,    73,    74,    75,   215,
     216,    53,    26,    70,   208,   209,    73,    87,    70,    35,
      54,    54,    89,    90,    91,    92,    93,    94,    95,   360,
      88,    86,    99,    87,    91,    92,    93,    94,    95,   370,
      92,    93,    94,    95,    36,    54,    86,   253,    26,    54,
     256,   257,   100,   330,    63,    64,    65,    66,    67,    68,
      86,    70,   339,    26,    73,    74,    75,    53,    87,    86,
     129,   348,   203,    86,    54,   352,   176,    54,    54,    -1,
      89,    90,    91,    92,    93,    94,    95,   293,    97,   366,
     212,   368,    -1,   299,   300,   301,   302,    17,    18,    19,
      -1,    -1,    -1,   297,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     0,     1,    -1,    -1,    -1,    -1,   335,
      -1,    -1,   338,    -1,    -1,    -1,    -1,    14,    -1,    -1,
      17,    18,    -1,   349,    21,    22,    23,    24,    25,    26,
      27,   357,    29,    30,    31,    -1,    33,    34,    35,    36,
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
      78,    79,    80,    -1,    -1,    83,    84,    -1,     1,    87,
      -1,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    97,
      98,    14,    -1,   101,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    -1,    31,    -1,
      33,    34,    -1,    -1,    -1,    38,    39,    40,    41,    42,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,
      -1,    -1,    55,    -1,    -1,    -1,    -1,    60,    61,    62,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,    -1,
      83,    84,    -1,    86,     1,    -1,    -1,    -1,    -1,    92,
      93,    -1,    -1,    -1,    97,    98,    -1,    14,   101,    -1,
      17,    18,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    -1,    31,    -1,    33,    34,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    60,    61,    62,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    -1,    -1,    83,    84,    -1,     1,
      87,    -1,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,
      97,    98,    14,    -1,   101,    17,    18,    -1,    -1,    21,
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
      56,    70,    -1,    -1,    73,    74,    75,    63,    64,    65,
      66,    67,    68,    -1,    70,    -1,    -1,    73,    74,    75,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
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
     115,   119,   120,   124,   128,   129,   130,   131,   132,   133,
     140,   142,   144,   148,   153,   156,    97,    26,   129,    26,
      50,    98,   130,   154,    98,   154,   129,    48,    98,   130,
     155,   143,   147,    51,   134,    46,    47,    48,    49,   130,
     141,   145,    51,    20,   130,    56,    57,    17,    18,    19,
      26,    33,    26,    26,   130,   130,   141,   136,   130,   130,
     130,   130,   130,   130,   130,   135,   149,   130,   130,   130,
      27,    29,   103,   130,   157,   158,   159,   160,     0,    97,
     130,    43,    65,   130,   109,   118,   124,   130,   133,   153,
      63,    64,    65,    66,    67,    68,    70,    73,    74,    75,
      89,    90,    91,    92,    93,    94,    95,   125,    98,   130,
     130,   154,   100,   154,   129,   154,   100,   154,   125,    26,
     137,   130,    46,   141,    26,    26,    44,    71,    72,    26,
      54,    65,   129,   129,    54,    65,    54,    53,    56,   141,
      26,   139,    48,    54,    48,    54,    48,    54,    48,    54,
      81,    82,    54,    97,    26,   138,    26,    99,   103,   103,
     102,   100,   100,   105,   121,   125,   125,   121,   126,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,    26,   150,    99,
     130,    99,    99,   130,   150,   100,   130,   146,   129,    26,
     132,   130,   130,    65,   130,   130,    54,    65,    48,   130,
     130,   130,   130,   100,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   114,   114,   100,   129,   129,   130,
      27,    29,   160,   127,   130,   130,    53,   121,    97,   100,
      97,    26,   125,    44,   130,   130,   130,    26,    26,    48,
      48,    48,    48,   108,   127,    26,    97,   110,   122,   126,
     126,   127,    53,   152,    26,   152,   150,   130,   111,   129,
     130,   130,   130,   130,   126,   126,    87,   127,    35,   122,
     108,   127,   127,    97,   123,    54,    88,   126,    54,   116,
     122,   125,    30,   152,   112,   130,    87,   130,    36,    37,
     117,   125,    36,    86,    26,   151,   127,    54,   126,   125,
     130,    86,   125,   100,   130,   127,   127,   121,   108,    26,
     113,   126,   125,    53,   125,   121,    87,    86,   127,    86,
     127,   122,    87
};

  /* YYR1YYN -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   104,   105,   105,   106,   106,   106,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     108,   108,   108,   109,   109,   109,   109,   110,   110,   110,
     111,   112,   113,   110,   110,   110,   110,   110,   110,   110,
     114,   115,   115,   116,   116,   117,   118,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   127,   127,   128,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   131,   131,   131,   131,   131,   131,   131,   131,   132,
     132,   133,   133,   133,   133,   133,   134,   133,   135,   133,
     136,   133,   133,   133,   133,   133,   137,   137,   138,   138,
     139,   139,   140,   140,   140,   140,   140,   140,   141,   141,
     142,   142,   142,   143,   142,   145,   146,   144,   144,   147,
     144,   144,   144,   149,   148,   150,   150,   150,   151,   151,
     151,   152,   153,   154,   154,   154,   155,   155,   156,   157,
     157,   157,   157,   158,   158,   159,   159,   160,   160
};

  /* YYR2YYN -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     4,     4,     6,
       4,     4,     4,     4,     4,     4,     5,     5,     8,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
       0,     0,     0,    14,    12,     8,     2,     3,     7,     6,
       0,     9,    12,     0,     2,     6,    11,     7,     2,     3,
       0,     0,     0,     1,     0,     0,     0,     2,     2,     3,
       1,     1,     1,     1,     1,     1,     3,     1,     2,     1,
       1,     4,     2,     4,     3,     2,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     3,     3,     3,     3,     2,     2,     4,
       4,     4,     6,     4,     6,     4,     6,     4,     6,     2,
       1,     2,     1,     1,     2,     1,     0,     3,     0,     3,
       0,     3,     4,     2,     4,     2,     1,     3,     1,     3,
       1,     3,     1,     2,     2,     2,     3,     2,     3,     2,
       2,     3,     2,     0,     3,     0,     0,     9,     2,     0,
       7,     8,     6,     0,     3,     0,     1,     3,     0,     1,
       3,     0,     2,     0,     1,     3,     1,     3,     3,     0,
       1,     1,     1,     1,     3,     1,     3,     3,     3
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
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1918 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* FBLTIN  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1924 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* RBLTIN  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1930 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* THEFBLTIN  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1936 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* ID  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1942 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* STRING  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1948 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* HANDLER  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1954 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* SYMBOL  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1960 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 30: /* ENDCLAUSE  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1966 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 31: /* tPLAYACCEL  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1972 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 32: /* tMETHOD  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1978 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 33: /* THEOBJECTFIELD  */
#line 162 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).os; }
#line 1984 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 148: /* on  */
#line 161 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1990 "engines/director/lingo/lingo-gr.cpp"
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
#line 173 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 179 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2284 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 183 "engines/director/lingo/lingo-gr.y"
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
#line 2300 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 194 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_after); }
#line 2306 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 195 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_before); }
#line 2312 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 196 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2323 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 202 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2335 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 209 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2346 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 215 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 222 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2369 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 228 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2380 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 235 "engines/director/lingo/lingo-gr.y"
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
#line 2396 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 246 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).oe);
		delete (yyvsp[-2].objectfield).os;
		(yyval.code) = (yyvsp[0].code); }
#line 2407 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 268 "engines/director/lingo/lingo-gr.y"
                                                                        {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-5].code) - (yyvsp[-1].code) + 2);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code) + 2);
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentScript)[(yyvsp[-1].code)] = start; }
#line 2418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 280 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->code1(LC::c_varpush);
			  g_lingo->codeString((yyvsp[-2].s)->c_str()); }
#line 2425 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 283 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->code1(LC::c_eval);
			  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2432 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 286 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->code1(LC::c_le); }
#line 2438 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 286 "engines/director/lingo/lingo-gr.y"
                                                                                  {

		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[-11].s)->c_str());
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
#line 2461 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 309 "engines/director/lingo/lingo-gr.y"
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
#line 2478 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 326 "engines/director/lingo/lingo-gr.y"
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
#line 2493 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 337 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_nextRepeat); }
#line 2500 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 339 "engines/director/lingo/lingo-gr.y"
                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2510 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 344 "engines/director/lingo/lingo-gr.y"
                                                          {
		inst end;
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code));
		(*g_lingo->_currentScript)[(yyvsp[-3].code) + 1] = end; }
#line 2519 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 348 "engines/director/lingo/lingo-gr.y"
                                                    {
		inst end;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2528 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 353 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }
#line 2536 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 357 "engines/director/lingo/lingo-gr.y"
                                                                                           {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2548 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 364 "engines/director/lingo/lingo-gr.y"
                                                                                                                  {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-6].code) + 1 - (yyvsp[-9].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-9].code)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-6].code), (yyvsp[-1].code)); }
#line 2560 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 375 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2570 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 381 "engines/director/lingo/lingo-gr.y"
                                                                                                             {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].code) + 1 - (yyvsp[-8].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-8].code)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-5].code)] = end;	}
#line 2581 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 387 "engines/director/lingo/lingo-gr.y"
                                                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-4].code) + 1);

		(*g_lingo->_currentScript)[(yyvsp[-4].code)] = end; }
#line 2591 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 393 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2597 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 395 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code3(LC::c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2607 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 401 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, STOP);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2615 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 405 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, STOP);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2623 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 409 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 413 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2638 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 416 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2644 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 418 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2650 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 420 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2656 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 424 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2666 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 430 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2674 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 433 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2682 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 436 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 440 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2700 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 444 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 448 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2722 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 456 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 458 "engines/director/lingo/lingo-gr.y"
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
#line 2743 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 469 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 471 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2757 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 474 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2765 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 477 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 480 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2781 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 483 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2792 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 489 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).oe);
		delete (yyvsp[0].objectfield).os; }
#line 2802 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 494 "engines/director/lingo/lingo-gr.y"
                       {
		g_lingo->code1(LC::c_objectrefpush);
		g_lingo->codeString((yyvsp[0].objectref).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectref).field->c_str());
		delete (yyvsp[0].objectref).obj;
		delete (yyvsp[0].objectref).field; }
#line 2813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 501 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2819 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 502 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2825 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 503 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2831 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 504 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 505 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2843 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 506 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2849 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 507 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2855 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 508 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 509 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2867 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 510 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2873 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 511 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2879 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 512 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2885 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 513 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2891 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 514 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2897 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 515 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2903 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 516 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2909 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 517 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2915 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 518 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2921 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 519 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); }
#line 2927 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 520 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2933 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 521 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_intersects); }
#line 2939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 522 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 2945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 524 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charOf); }
#line 2951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 525 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charToOf); }
#line 2957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 526 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemOf); }
#line 2963 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 527 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_itemToOf); }
#line 2969 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 528 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineOf); }
#line 2975 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 529 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_lineToOf); }
#line 2981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 530 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordOf); }
#line 2987 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 531 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_wordToOf); }
#line 2993 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 533 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 3001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 538 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_printtop); }
#line 3007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 541 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_exitRepeat); }
#line 3013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 542 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 3019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 543 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 543 "engines/director/lingo/lingo-gr.y"
                                                                                 { inNone(); }
#line 3031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 544 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 544 "engines/director/lingo/lingo-gr.y"
                                                                                   { inNone(); }
#line 3043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 545 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 545 "engines/director/lingo/lingo-gr.y"
                                                                                   { inNone(); }
#line 3055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 546 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3063 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 549 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3071 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 552 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_open); }
#line 3077 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 553 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 3083 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 555 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3093 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 560 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3103 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 566 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3113 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 571 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3123 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 577 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 582 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mArg((yyvsp[0].s));
		delete (yyvsp[0].s); }
#line 3143 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 595 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3149 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 596 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3155 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 597 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3161 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 598 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3170 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 602 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3179 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 606 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3188 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 614 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3197 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 618 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3206 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 622 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3215 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 626 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3221 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 626 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3230 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 656 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3236 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 656 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->_currentFactory.clear(); }
#line 3242 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 657 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-6].s); }
#line 3252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 662 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3258 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 663 "engines/director/lingo/lingo-gr.y"
                  { startDef(); }
#line 3264 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 664 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
		endDef();
		delete (yyvsp[-6].s); }
#line 3274 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 669 "engines/director/lingo/lingo-gr.y"
                                                                     {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3287 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 677 "engines/director/lingo/lingo-gr.y"
                                                 {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3297 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 683 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3303 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 683 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.s) = (yyvsp[0].s); g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 3310 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 686 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3316 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 687 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mArg((yyvsp[0].s)); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 688 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); mArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3328 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 691 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 692 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3340 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 694 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArgStore(); inDef(); }
#line 3346 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 696 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 3358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 704 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3364 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 705 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 706 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3376 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 708 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3382 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 709 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 711 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3394 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 713 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3400 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 714 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3406 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 715 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3412 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172:
#line 716 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173:
#line 718 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3424 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174:
#line 719 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3430 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175:
#line 721 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3436 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176:
#line 722 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3442 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177:
#line 724 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3451 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178:
#line 728 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3460 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3464 "engines/director/lingo/lingo-gr.cpp"

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

#line 734 "engines/director/lingo/lingo-gr.y"


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
