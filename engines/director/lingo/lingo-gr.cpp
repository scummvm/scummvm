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

	delete g_lingo->_methodVars;
	g_lingo->_methodVars = g_lingo->_methodVarsStash;
	g_lingo->_methodVarsStash = nullptr;
}

static void mVar(Common::String *s, VarType type) {
	if (!g_lingo->_methodVars->contains(*s))
		(*g_lingo->_methodVars)[*s] = type;
}


#line 131 "engines/director/lingo/lingo-gr.cpp"

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
#line 114 "engines/director/lingo/lingo-gr.y"

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

#line 289 "engines/director/lingo/lingo-gr.cpp"

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
  YYSYMBOL_stmt = 109,                     /* stmt  */
  YYSYMBOL_110_1 = 110,                    /* $@1  */
  YYSYMBOL_111_2 = 111,                    /* $@2  */
  YYSYMBOL_112_3 = 112,                    /* $@3  */
  YYSYMBOL_113_4 = 113,                    /* $@4  */
  YYSYMBOL_114_5 = 114,                    /* $@5  */
  YYSYMBOL_115_6 = 115,                    /* $@6  */
  YYSYMBOL_116_7 = 116,                    /* $@7  */
  YYSYMBOL_117_8 = 117,                    /* $@8  */
  YYSYMBOL_118_9 = 118,                    /* $@9  */
  YYSYMBOL_tellstart = 119,                /* tellstart  */
  YYSYMBOL_ifstmt = 120,                   /* ifstmt  */
  YYSYMBOL_elseifstmtlist = 121,           /* elseifstmtlist  */
  YYSYMBOL_elseifstmt = 122,               /* elseifstmt  */
  YYSYMBOL_jumpifz = 123,                  /* jumpifz  */
  YYSYMBOL_jump = 124,                     /* jump  */
  YYSYMBOL_varassign = 125,                /* varassign  */
  YYSYMBOL_if = 126,                       /* if  */
  YYSYMBOL_begin = 127,                    /* begin  */
  YYSYMBOL_end = 128,                      /* end  */
  YYSYMBOL_stmtlist = 129,                 /* stmtlist  */
  YYSYMBOL_simpleexpr = 130,               /* simpleexpr  */
  YYSYMBOL_expr = 131,                     /* expr  */
  YYSYMBOL_chunkexpr = 132,                /* chunkexpr  */
  YYSYMBOL_reference = 133,                /* reference  */
  YYSYMBOL_proc = 134,                     /* proc  */
  YYSYMBOL_135_10 = 135,                   /* $@10  */
  YYSYMBOL_136_11 = 136,                   /* $@11  */
  YYSYMBOL_137_12 = 137,                   /* $@12  */
  YYSYMBOL_globallist = 138,               /* globallist  */
  YYSYMBOL_propertylist = 139,             /* propertylist  */
  YYSYMBOL_instancelist = 140,             /* instancelist  */
  YYSYMBOL_gotofunc = 141,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 142,                /* gotomovie  */
  YYSYMBOL_playfunc = 143,                 /* playfunc  */
  YYSYMBOL_144_13 = 144,                   /* $@13  */
  YYSYMBOL_defn = 145,                     /* defn  */
  YYSYMBOL_146_14 = 146,                   /* $@14  */
  YYSYMBOL_147_15 = 147,                   /* $@15  */
  YYSYMBOL_148_16 = 148,                   /* $@16  */
  YYSYMBOL_on = 149,                       /* on  */
  YYSYMBOL_150_17 = 150,                   /* $@17  */
  YYSYMBOL_argdef = 151,                   /* argdef  */
  YYSYMBOL_endargdef = 152,                /* endargdef  */
  YYSYMBOL_argstore = 153,                 /* argstore  */
  YYSYMBOL_macro = 154,                    /* macro  */
  YYSYMBOL_arglist = 155,                  /* arglist  */
  YYSYMBOL_nonemptyarglist = 156,          /* nonemptyarglist  */
  YYSYMBOL_list = 157,                     /* list  */
  YYSYMBOL_valuelist = 158,                /* valuelist  */
  YYSYMBOL_linearlist = 159,               /* linearlist  */
  YYSYMBOL_proplist = 160,                 /* proplist  */
  YYSYMBOL_proppair = 161                  /* proppair  */
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
#define YYLAST   2541

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  58
/* YYNRULES -- Number of rules.  */
#define YYNRULES  175
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  364

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
       0,   170,   170,   171,   173,   174,   175,   177,   184,   188,
     199,   200,   201,   208,   215,   222,   229,   235,   242,   253,
     260,   261,   262,   264,   265,   270,   282,   286,   289,   281,
     313,   317,   320,   312,   346,   352,   358,   345,   385,   387,
     390,   394,   399,   403,   410,   418,   419,   421,   427,   431,
     435,   439,   442,   444,   446,   447,   448,   450,   453,   456,
     460,   464,   468,   476,   477,   478,   489,   490,   491,   494,
     497,   500,   503,   509,   514,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   539,   540,   541,   542,   544,   545,
     546,   547,   548,   549,   550,   551,   553,   556,   558,   559,
     560,   561,   562,   563,   563,   564,   564,   565,   565,   566,
     569,   572,   573,   575,   580,   586,   591,   597,   602,   615,
     616,   617,   618,   622,   626,   631,   632,   634,   638,   642,
     646,   646,   676,   676,   676,   682,   683,   683,   689,   697,
     703,   703,   706,   707,   708,   710,   711,   712,   714,   716,
     724,   725,   726,   728,   729,   731,   733,   734,   735,   736,
     738,   739,   741,   742,   744,   748
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
  "$accept", "program", "programline", "asgn", "stmtoneliner", "stmt",
  "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9",
  "tellstart", "ifstmt", "elseifstmtlist", "elseifstmt", "jumpifz", "jump",
  "varassign", "if", "begin", "end", "stmtlist", "simpleexpr", "expr",
  "chunkexpr", "reference", "proc", "$@10", "$@11", "$@12", "globallist",
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

#define YYPACT_NINF (-295)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-167)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACTSTATE-NUM -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     772,   -83,  -295,  -295,    88,  -295,   594,   310,    88,   -27,
     495,  -295,  -295,  -295,  -295,  -295,  -295,   -17,  -295,  1792,
    -295,  -295,  -295,   -12,  1948,    27,    41,    30,    53,   224,
    1829,  -295,   224,   224,   224,   224,   224,   224,   224,  -295,
    -295,   224,   224,   224,   405,    81,   -56,  -295,  -295,  -295,
    -295,   224,  -295,  2433,  -295,  -295,  -295,  -295,  -295,  -295,
    -295,  -295,  -295,  -295,  -295,  -295,   -11,  1948,  1911,  2433,
       4,  1911,     4,  -295,    88,  1911,  2433,     6,   683,  -295,
    -295,    77,   224,  -295,    61,  -295,  2020,  -295,    82,  -295,
      84,  1987,    85,  -295,   -49,    88,    88,    -8,    58,    60,
    -295,  2330,  2020,  -295,    90,  -295,  2053,  2086,  2119,  2152,
    2400,  2317,    92,    93,  -295,  -295,  2363,    17,    36,  -295,
    2433,    40,    43,    45,  -295,  -295,   772,  2433,   224,   224,
     224,   224,   224,   224,   224,   224,   224,   224,   224,   224,
     224,   224,   224,   224,   224,   120,  1911,  1987,  2363,   -14,
     224,    -9,  -295,    -3,   224,     4,   120,  -295,    47,  2433,
     224,  -295,  -295,    88,    16,   224,   224,   -10,   224,   224,
     224,    -1,   101,   224,   224,   224,   224,   224,  -295,  -295,
      50,   224,   224,   224,   224,   224,   224,   224,   224,   224,
     224,  -295,  -295,  -295,    51,  -295,  -295,    88,    88,  -295,
     224,    -7,  -295,    99,   117,   117,   117,   117,  2446,  2446,
    -295,   -45,   117,   117,   117,   117,   -45,   -51,   -51,  -295,
    -295,  -295,   -46,  -295,  2433,  -295,  -295,  2433,   -29,   127,
    2433,  -295,   112,  -295,  -295,  2433,  2433,   224,   224,  2433,
    2433,   117,   224,   224,   131,  2433,   117,  2433,  2433,  2433,
     132,  2433,  2185,  2433,  2218,  2433,  2251,  2433,  2284,  2433,
    2433,  1710,  -295,   135,  -295,  -295,  2433,    17,    36,  -295,
    -295,  -295,   139,  -295,  -295,   120,   224,  2433,   378,  -295,
    2433,   117,    88,  -295,   224,   224,   224,   224,  -295,  1199,
    -295,  1113,  -295,  -295,  -295,   -22,  2433,  -295,  -295,  -295,
    1284,   119,  2433,  2433,  2433,  2433,  -295,  -295,  -295,    86,
    -295,   858,   943,  -295,  -295,  -295,  -295,    98,   224,  -295,
      64,   165,  -295,  -295,   138,   158,  -295,  2433,  -295,   224,
    -295,   109,  -295,    96,  1028,  -295,   224,   143,  -295,  2433,
    -295,   173,  -295,  2433,   224,  1369,   149,  -295,  1455,  -295,
    2433,   121,  -295,  -295,  -295,  -295,  -295,  1113,  1540,  -295,
    -295,  -295,  1625,  -295
};

  /* YYDEFACTSTATE-NUM -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    57,    62,     0,    58,     0,     0,     0,     0,
       0,    60,    59,   140,   146,    73,    74,   112,   113,     0,
     129,    51,   142,     0,     0,     0,     0,     0,     0,     0,
       0,   117,     0,     0,     0,     0,     0,     0,     0,   115,
     150,     0,     0,     0,     0,     0,     2,    75,    23,     6,
      24,     0,    66,    21,   107,    67,    22,   109,   110,     5,
      52,    20,    64,    65,    61,    72,    61,     0,     0,   161,
     120,     0,    69,   106,     0,     0,   163,   159,     0,    52,
     111,     0,     0,   130,     0,   131,   132,   134,     0,    38,
       0,   108,     0,    52,     0,     0,     0,     0,     0,     0,
     145,   122,   137,   139,     0,    89,     0,     0,     0,     0,
       0,     0,     0,     0,    94,    95,     0,    60,    59,   167,
     170,     0,   168,   169,   172,     1,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   152,     0,     0,   161,     0,
       0,     0,    71,     0,     0,   141,   152,   123,   114,   136,
       0,   133,   143,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   138,   127,
     118,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    42,    42,   125,   116,   151,    63,     0,     0,   165,
       0,     0,     3,     0,    85,    86,    83,    84,    87,    88,
      80,    91,    92,    93,    82,    81,    90,    76,    77,    78,
      79,   153,     0,   119,   162,    68,    70,   164,     0,     0,
     135,    52,     0,     7,     8,    10,    11,     0,     0,    48,
      15,    13,     0,     0,     0,    14,    12,    19,    39,   121,
       0,    98,     0,   100,     0,   102,     0,   104,     0,    96,
      97,     0,    54,     0,   175,   174,   171,     0,     0,   173,
      54,   158,     0,   158,   124,   152,     0,    34,    26,    54,
      16,    17,     0,   128,     0,     0,     0,     0,    53,     0,
     126,     0,    54,   154,    54,     0,     9,    52,    50,    50,
       0,     0,    99,   101,   103,   105,    41,    55,    56,     0,
      45,     0,     0,   158,    35,    27,    31,     0,     0,    40,
      52,   155,    54,    48,     0,     0,    25,    18,    52,     0,
      46,     0,   156,   148,     0,    36,     0,     0,    54,    48,
      43,     0,    54,    28,     0,     0,     0,   157,     0,    48,
      32,     0,    54,    37,    54,    48,    44,     0,     0,    54,
      47,    29,     0,    33
};

  /* YYPGOTONTERM-NUM.  */
static const yytype_int16 yypgoto[] =
{
    -295,    79,  -295,  -295,   -58,    15,  -295,  -295,  -295,  -295,
    -295,  -295,  -295,  -295,  -295,    14,  -295,  -295,  -295,  -151,
    -294,   -86,  -295,   -76,   -75,  1654,     3,    -6,  -295,    52,
    -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,   -20,  -295,
    -295,  -295,  -295,  -295,  -295,  -295,  -295,  -154,  -295,  -261,
    -295,     2,  -295,  -295,  -295,  -295,  -295,    18
};

  /* YYDEFGOTONTERM-NUM.  */
static const yytype_int16 yydefgoto[] =
{
      -1,    45,    46,    47,    48,   308,   298,   324,   349,   299,
     325,   355,   297,   323,   342,   261,    50,   320,   330,   203,
     310,   315,    51,   145,   306,   289,    52,    53,    54,    55,
      56,    81,   112,   104,   158,   194,   180,    57,    87,    58,
      78,    59,    88,   231,    79,    60,   113,   222,   333,   292,
      61,   153,    77,    62,   121,   122,   123,   124
};

  /* YYTABLEYYPACT[STATE-NUM] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      69,    69,   228,   156,    76,   169,   317,    65,    70,    72,
     103,    73,   294,    86,    63,    49,   170,   168,    91,   134,
     267,    74,   268,   101,   102,   134,   105,   106,   107,   108,
     109,   110,   111,   237,    80,   114,   115,   116,   120,    89,
       8,   126,   233,   143,   144,   127,   173,   141,   142,   143,
     144,   271,   322,   242,   272,   238,    99,   174,    94,    95,
      96,   147,   148,   360,   243,   148,   161,    97,   273,   148,
     149,   272,    69,   151,    98,   313,   159,   152,   272,   100,
     155,   125,   178,    92,    93,   223,   150,   146,   279,     1,
     225,   150,    33,    34,    35,    36,   226,   150,   171,   172,
     328,   329,     2,   157,   150,     3,   154,   160,   162,     5,
     163,   167,   175,   176,    64,    11,   179,    12,   193,   195,
     197,   295,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   198,
      69,    49,   199,   200,   224,   201,   221,   229,   227,   244,
     250,   263,   270,   274,   230,   275,   276,   282,   283,   235,
     236,   290,   239,   240,   241,   293,   232,   245,   246,   247,
     248,   249,   335,   318,   319,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   326,    43,   134,   346,    44,
     135,   332,   336,   337,   266,   340,   341,   344,   354,   347,
     264,   265,   352,   288,   359,   202,   262,   356,   140,   141,
     142,   143,   144,   316,   309,     0,   234,     0,     0,   269,
       0,   314,     0,     0,     0,     1,     0,     0,     0,     0,
       0,   277,   278,     0,     0,     0,   280,   281,     2,     0,
       0,     3,     4,     0,   331,     5,     0,     7,     8,     9,
      66,    11,   338,    12,     0,     0,     0,    15,    16,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   351,
     296,     0,     0,     0,    67,     0,    26,     0,   302,   303,
     304,   305,     0,     0,     0,   301,     0,     0,     0,     0,
       0,     0,     0,    32,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    36,    37,     0,     0,     0,     0,     0,
    -160,     1,   327,     0,     0,     0,    41,    42,     0,     0,
       0,     0,    43,   339,     2,    44,     0,     3,     4,     0,
     343,     5,  -160,     7,     8,     9,    66,    11,   350,    12,
    -160,  -160,     0,    15,    16,  -160,  -160,  -160,  -160,  -160,
    -160,  -160,  -160,     0,  -160,     0,  -160,  -160,  -160,     0,
      67,  -160,    26,  -160,  -160,  -160,  -160,     0,     0,     0,
    -160,  -160,  -160,  -160,  -160,  -160,  -160,  -160,  -160,    32,
    -160,  -160,  -160,  -160,  -160,  -160,    33,    34,    35,    36,
      37,  -160,  -160,  -160,  -160,     0,  -160,  -160,  -160,  -160,
    -160,  -160,    41,    42,  -160,  -160,     1,  -160,    71,  -160,
    -160,    44,  -160,   -30,     0,     0,     0,     0,     0,     2,
       0,     0,     3,     4,     0,     0,     5,     0,     7,     8,
       9,    66,   117,     0,   118,     0,     0,     0,    15,    16,
       0,   128,   129,   130,   131,   132,   133,     0,   134,     0,
       0,   135,   136,   137,     0,    67,     0,    26,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   138,   139,   140,
     141,   142,   143,   144,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   -61,     1,    41,    42,     0,
       0,     0,     0,    43,     0,     0,    44,  -166,   119,     2,
       0,     0,     3,     4,     0,     0,     5,   -61,     7,     8,
       9,    66,    11,     0,    12,   -61,   -61,     0,    15,    16,
       0,   -61,   -61,   -61,   -61,   -61,   -61,   -61,     0,     0,
       0,     0,   -61,     0,     0,    67,   -61,    26,     0,     0,
     -61,     0,     0,     0,     0,   -61,   -61,   -61,   -61,   -61,
     -61,   -61,   -61,   -61,    32,   -61,     0,     0,   -61,   -61,
     -61,    33,    34,    35,    36,    37,     0,     0,   -61,   -61,
       0,   -61,   -61,   -61,   -61,   -61,   -61,    41,    42,   -61,
     -61,     0,   -61,    75,  -160,     1,    44,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     2,     0,
       0,     3,     4,     0,     0,     5,  -160,     7,     8,     9,
      66,    11,     0,    12,  -160,  -160,     0,    15,    16,     0,
    -160,  -160,  -160,  -160,  -160,  -160,  -160,     0,     0,     0,
       0,  -160,     0,     0,    67,  -160,    26,     0,     0,  -160,
       0,     0,     0,     0,  -160,  -160,  -160,     0,     0,     0,
       0,     0,     0,    32,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    36,    37,     0,     0,  -160,  -160,     0,
    -160,  -160,  -160,  -160,     1,     0,    41,    42,     0,     0,
       0,  -160,    68,     0,  -160,    44,     0,     2,     0,     0,
       3,     4,     0,     0,     5,  -160,     7,     8,     9,    66,
      11,     0,    12,  -160,  -160,     0,    15,    16,     0,  -160,
    -160,  -160,  -160,  -160,  -160,  -160,     0,     0,     0,     0,
    -160,     0,     0,    67,  -160,    26,     0,     0,  -160,     0,
       0,     0,     0,  -160,  -160,  -160,     0,     0,     0,     0,
       0,     0,    32,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,    37,     0,     0,  -160,  -160,     0,  -160,
    -160,  -160,    -4,     1,     0,    41,    42,     0,     0,     0,
    -160,    43,     0,  -160,    44,     0,     2,     0,     0,     3,
       4,     0,     0,     5,     6,     7,     8,     9,    10,    11,
       0,    12,     0,    13,    14,    15,    16,     0,     0,     0,
      17,    18,    19,    20,    21,     0,     0,    22,     0,    23,
       0,     0,    24,    25,    26,     0,     0,    27,     0,     0,
       0,    28,    29,    30,    31,     0,     0,     0,     0,     0,
       0,    32,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     0,     0,    38,    39,    40,  -149,     1,
       0,     0,     0,     0,    41,    42,     0,     0,     0,    -4,
      43,     0,     2,    44,     0,     3,     4,     0,     0,     5,
       6,     7,     8,     9,    10,    11,     0,    12,   321,    13,
       0,    15,    16,     0,     0,     0,    17,    18,    19,    20,
      21,     0,     0,     0,     0,    23,     0,     0,    24,    25,
      26,     0,     0,    27,     0,     0,     0,     0,    29,    30,
      31,     0,     0,     0,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,    38,    39,  -147,     1,     0,     0,     0,     0,     0,
      41,    42,     0,     0,     0,   307,    43,     2,     0,    44,
       3,     4,     0,     0,     5,     6,     7,     8,     9,    10,
      11,     0,    12,     0,    13,     0,    15,    16,     0,     0,
       0,    17,    18,    19,    20,    21,     0,     0,     0,     0,
      23,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    29,    30,    31,     0,     0,     0,     0,
       0,     0,    32,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,    37,     0,     0,    38,    39,  -144,     1,
       0,     0,     0,     0,     0,    41,    42,     0,     0,     0,
     307,    43,     2,     0,    44,     3,     4,     0,     0,     5,
       6,     7,     8,     9,    10,    11,     0,    12,     0,    13,
       0,    15,    16,     0,     0,     0,    17,    18,    19,    20,
      21,     0,     0,     0,     0,    23,     0,     0,    24,    25,
      26,     0,     0,    27,     0,     0,     0,     0,    29,    30,
      31,     0,     0,     0,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,    38,    39,     0,     1,     0,     0,     0,     0,     0,
      41,    42,     0,     0,     0,   307,    43,     2,     0,    44,
       3,     4,     0,     0,     5,     6,     7,     8,     9,    10,
      11,     0,    12,     0,    13,     0,    15,    16,     0,   -49,
     -49,    17,    18,    19,    20,    21,     0,     0,     0,     0,
      23,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    29,    30,    31,     0,     0,     0,     0,
       0,     0,    32,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,    37,     0,     0,    38,    39,     0,   -49,
       1,     0,     0,     0,     0,    41,    42,     0,     0,     0,
     307,    43,     0,     2,    44,     0,     3,     4,     0,     0,
       5,     6,     7,     8,     9,    10,    11,     0,    12,     0,
      13,     0,    15,    16,     0,     0,     0,    17,    18,    19,
      20,    21,     0,     0,     0,     0,    23,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    29,
      30,    31,     0,     0,     0,     0,     0,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,    38,    39,     0,     1,     0,   -53,     0,     0,
       0,    41,    42,     0,     0,     0,   307,    43,     2,     0,
      44,     3,     4,     0,     0,     5,     6,     7,     8,     9,
      10,    11,     0,    12,     0,    13,     0,    15,    16,     0,
       0,     0,    17,    18,    19,    20,    21,     0,     0,     0,
       0,    23,     0,     0,    24,    25,    26,     0,     0,    27,
       0,     0,     0,     0,    29,    30,    31,     0,     0,     0,
       0,     0,     0,    32,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    36,    37,     0,     0,    38,    39,     0,
       1,   -49,     0,     0,     0,     0,    41,    42,     0,     0,
       0,   307,    43,     2,     0,    44,     3,     4,     0,     0,
       5,     6,     7,     8,     9,    10,    11,     0,    12,     0,
      13,     0,    15,    16,     0,     0,     0,    17,    18,    19,
      20,    21,     0,     0,     0,     0,    23,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    29,
      30,    31,     0,     0,     0,     0,     0,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,    38,    39,     0,   -52,     1,     0,     0,     0,
       0,    41,    42,     0,     0,     0,   307,    43,     0,     2,
      44,     0,     3,     4,     0,     0,     5,     6,     7,     8,
       9,    10,    11,     0,    12,     0,    13,     0,    15,    16,
       0,     0,     0,    17,    18,    19,    20,    21,     0,     0,
       0,     0,    23,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    29,    30,    31,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,    38,    39,
       0,     1,   353,     0,     0,     0,     0,    41,    42,     0,
       0,     0,   307,    43,     2,     0,    44,     3,     4,     0,
       0,     5,     6,     7,     8,     9,    10,    11,     0,    12,
       0,    13,     0,    15,    16,     0,     0,     0,    17,    18,
      19,    20,    21,     0,     0,     0,     0,    23,     0,     0,
      24,    25,    26,     0,     0,    27,     0,     0,     0,     0,
      29,    30,    31,     0,     0,     0,     0,     0,     0,    32,
       0,     0,     0,     0,     0,     0,    33,    34,    35,    36,
      37,     0,     0,    38,    39,     0,     1,   361,     0,     0,
       0,     0,    41,    42,     0,     0,     0,   307,    43,     2,
       0,    44,     3,     4,     0,     0,     5,     6,     7,     8,
       9,    10,    11,     0,    12,     0,    13,     0,    15,    16,
       0,     0,     0,    17,    18,    19,    20,    21,     0,     0,
       0,     0,    23,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    29,    30,    31,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,    38,    39,
       0,     1,   363,     0,     0,     0,     0,    41,    42,     0,
       0,     0,   307,    43,     2,     0,    44,     3,     4,     0,
       0,     5,     6,     7,     8,     9,    10,    11,     0,    12,
       0,    13,     0,    15,    16,     0,     0,     0,    17,    18,
      19,    20,     0,     0,     0,     0,     0,     0,     0,     0,
      24,     0,    26,     0,     0,     0,     0,     0,     0,     0,
      29,    30,    31,     0,     0,     0,     0,     0,     0,    32,
       0,     0,     0,     0,     0,     0,    33,    34,    35,    36,
      37,     0,     0,     1,    39,     0,     0,     0,     0,     0,
       0,     0,    41,    42,     0,     0,     2,     0,    43,     3,
       4,    44,     0,     5,     0,     7,     8,     9,    66,    11,
       0,    12,     0,     0,     0,    15,    16,     0,     0,     0,
       1,     0,     0,     0,     0,     0,     0,     0,    82,    83,
      84,    85,    67,     2,    26,     0,     3,     4,     0,     0,
       5,     0,     7,     8,     9,    66,    11,     0,    12,     0,
       0,    32,    15,    16,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     0,     0,    82,     0,    84,     0,    67,
       0,    26,     0,     0,    41,    42,     0,     0,     0,     0,
      43,     0,     0,    44,     0,     0,     0,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,    41,    42,     0,   291,     2,     0,    43,     3,     4,
      44,     0,     5,   300,     7,     8,     9,    66,    11,     0,
      12,     0,     0,     0,    15,    16,   311,     0,   312,     1,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    67,     2,    26,     0,     3,     4,     0,    90,     5,
       0,     7,     8,     9,    66,    11,   334,    12,     0,     0,
      32,    15,    16,     0,     0,     0,     0,    33,    34,    35,
      36,    37,   345,     0,     0,     0,   348,     0,    67,     0,
      26,     0,     0,    41,    42,     0,   357,     0,   358,    43,
    -160,  -160,    44,   362,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,   164,     0,     0,     0,     0,     0,     0,     0,     0,
      41,    42,     0,     0,     0,     0,    43,     0,     0,    44,
     128,   129,   130,   131,   132,   133,     0,   134,   165,   166,
     135,   136,   137,     0,     0,     0,    82,     0,    84,     0,
       0,     0,     0,     0,     0,     0,   138,   139,   140,   141,
     142,   143,   144,   128,   129,   130,   131,   132,   133,     0,
     134,     0,     0,   135,   136,   137,     0,     0,     0,     0,
       0,   181,     0,     0,     0,     0,     0,   182,     0,   138,
     139,   140,   141,   142,   143,   144,   128,   129,   130,   131,
     132,   133,     0,   134,     0,     0,   135,   136,   137,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
     184,     0,   138,   139,   140,   141,   142,   143,   144,   128,
     129,   130,   131,   132,   133,     0,   134,     0,     0,   135,
     136,   137,     0,     0,     0,     0,     0,   185,     0,     0,
       0,     0,     0,   186,     0,   138,   139,   140,   141,   142,
     143,   144,   128,   129,   130,   131,   132,   133,     0,   134,
       0,     0,   135,   136,   137,     0,     0,     0,     0,     0,
     187,     0,     0,     0,     0,     0,   188,     0,   138,   139,
     140,   141,   142,   143,   144,   128,   129,   130,   131,   132,
     133,     0,   134,     0,     0,   135,   136,   137,     0,     0,
       0,     0,     0,   284,     0,     0,     0,     0,     0,     0,
       0,   138,   139,   140,   141,   142,   143,   144,   128,   129,
     130,   131,   132,   133,     0,   134,     0,     0,   135,   136,
     137,     0,     0,     0,     0,     0,   285,     0,     0,     0,
       0,     0,     0,     0,   138,   139,   140,   141,   142,   143,
     144,   128,   129,   130,   131,   132,   133,     0,   134,     0,
       0,   135,   136,   137,     0,     0,     0,     0,     0,   286,
       0,     0,     0,     0,     0,     0,     0,   138,   139,   140,
     141,   142,   143,   144,   128,   129,   130,   131,   132,   133,
       0,   134,     0,     0,   135,   136,   137,     0,     0,     0,
       0,     0,   287,     0,     0,     0,     0,     0,     0,     0,
     138,   139,   140,   141,   142,   143,   144,   128,   129,   130,
     131,   132,   133,     0,   134,     0,     0,   135,   136,   137,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   191,     0,   138,   139,   140,   141,   142,   143,   144,
     128,   129,   130,   131,   132,   133,   177,   134,     0,     0,
     135,   136,   137,   128,   129,   130,   131,   132,   133,     0,
     134,     0,     0,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,     0,   192,     0,     0,     0,     0,   138,
     139,   140,   141,   142,   143,   144,   128,   129,   130,   131,
     132,   133,     0,   134,     0,     0,   135,   136,   137,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   138,   139,   140,   141,   142,   143,   144,     0,
       0,     0,   196,   128,   129,   130,   131,   132,   133,     0,
     134,     0,     0,   135,   136,   137,     0,     0,     0,     0,
       0,   189,   190,     0,     0,     0,     0,     0,     0,   138,
     139,   140,   141,   142,   143,   144,   128,   129,   130,   131,
     132,   133,     0,   134,     0,     0,   135,   136,   137,   128,
     129,   130,   131,     0,     0,     0,   134,     0,     0,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,     0,
       0,     0,     0,     0,     0,   138,   139,   140,   141,   142,
     143,   144
};

static const yytype_int16 yycheck[] =
{
       6,     7,   156,    79,    10,    54,   300,     4,     6,     7,
      30,     8,   273,    19,    97,     0,    65,    93,    24,    70,
      27,    48,    29,    29,    30,    70,    32,    33,    34,    35,
      36,    37,    38,    43,    51,    41,    42,    43,    44,    51,
      24,    97,    26,    94,    95,    51,    54,    92,    93,    94,
      95,    97,   313,    54,   100,    65,    26,    65,    17,    18,
      19,    67,    68,   357,    65,    71,    86,    26,    97,    75,
      68,   100,    78,    71,    33,    97,    82,    74,   100,    26,
      78,     0,   102,    56,    57,    99,   100,    98,   239,     1,
      99,   100,    76,    77,    78,    79,    99,   100,    95,    96,
      36,    37,    14,    26,   100,    17,   100,    46,    26,    21,
      26,    26,    54,    53,    26,    27,    26,    29,    26,    26,
     103,   275,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   103,
     146,   126,   102,   100,   150,   100,    26,   100,   154,    48,
     100,   100,    53,    26,   160,   231,    44,    26,    26,   165,
     166,    26,   168,   169,   170,    26,   163,   173,   174,   175,
     176,   177,   323,    54,    88,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,    87,    98,    70,   339,   101,
      73,    26,    54,    35,   200,    86,   100,    54,   349,    26,
     197,   198,    53,   261,   355,   126,   192,    86,    91,    92,
      93,    94,    95,   299,   289,    -1,   164,    -1,    -1,   201,
      -1,   297,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,
      -1,   237,   238,    -1,    -1,    -1,   242,   243,    14,    -1,
      -1,    17,    18,    -1,   320,    21,    -1,    23,    24,    25,
      26,    27,   328,    29,    -1,    -1,    -1,    33,    34,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   345,
     276,    -1,    -1,    -1,    50,    -1,    52,    -1,   284,   285,
     286,   287,    -1,    -1,    -1,   282,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,
       0,     1,   318,    -1,    -1,    -1,    92,    93,    -1,    -1,
      -1,    -1,    98,   329,    14,   101,    -1,    17,    18,    -1,
     336,    21,    22,    23,    24,    25,    26,    27,   344,    29,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    -1,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,     1,    97,    98,    99,
     100,   101,   102,    35,    -1,    -1,    -1,    -1,    -1,    14,
      -1,    -1,    17,    18,    -1,    -1,    21,    -1,    23,    24,
      25,    26,    27,    -1,    29,    -1,    -1,    -1,    33,    34,
      -1,    63,    64,    65,    66,    67,    68,    -1,    70,    -1,
      -1,    73,    74,    75,    -1,    50,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     0,     1,    92,    93,    -1,
      -1,    -1,    -1,    98,    -1,    -1,   101,   102,   103,    14,
      -1,    -1,    17,    18,    -1,    -1,    21,    22,    23,    24,
      25,    26,    27,    -1,    29,    30,    31,    -1,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    42,    -1,    -1,
      -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    -1,    73,    74,
      75,    76,    77,    78,    79,    80,    -1,    -1,    83,    84,
      -1,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    98,     0,     1,   101,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,
      -1,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    30,    31,    -1,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    60,    61,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    -1,    -1,    83,    84,    -1,
      86,    87,    88,     0,     1,    -1,    92,    93,    -1,    -1,
      -1,    97,    98,    -1,   100,   101,    -1,    14,    -1,    -1,
      17,    18,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    60,    61,    62,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    -1,    -1,    83,    84,    -1,    86,
      87,    88,     0,     1,    -1,    92,    93,    -1,    -1,    -1,
      97,    98,    -1,   100,   101,    -1,    14,    -1,    -1,    17,
      18,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      -1,    29,    -1,    31,    32,    33,    34,    -1,    -1,    -1,
      38,    39,    40,    41,    42,    -1,    -1,    45,    -1,    47,
      -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,
      -1,    59,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    -1,    -1,    83,    84,    85,     0,     1,
      -1,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    97,
      98,    -1,    14,   101,    -1,    17,    18,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    30,    31,
      -1,    33,    34,    -1,    -1,    -1,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,
      52,    -1,    -1,    55,    -1,    -1,    -1,    -1,    60,    61,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,
      -1,    83,    84,     0,     1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    -1,    -1,    -1,    97,    98,    14,    -1,   101,
      17,    18,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    -1,    31,    -1,    33,    34,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    60,    61,    62,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    -1,    -1,    83,    84,     0,     1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,
      97,    98,    14,    -1,   101,    17,    18,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    -1,    31,
      -1,    33,    34,    -1,    -1,    -1,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,
      52,    -1,    -1,    55,    -1,    -1,    -1,    -1,    60,    61,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,
      -1,    83,    84,    -1,     1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    -1,    -1,    -1,    97,    98,    14,    -1,   101,
      17,    18,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    -1,    31,    -1,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    60,    61,    62,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    -1,    -1,    83,    84,    -1,    86,
       1,    -1,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,
      97,    98,    -1,    14,   101,    -1,    17,    18,    -1,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    29,    -1,
      31,    -1,    33,    34,    -1,    -1,    -1,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,    60,
      61,    62,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      -1,    -1,    83,    84,    -1,     1,    -1,    88,    -1,    -1,
      -1,    92,    93,    -1,    -1,    -1,    97,    98,    14,    -1,
     101,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    -1,    31,    -1,    33,    34,    -1,
      -1,    -1,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    60,    61,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    -1,    -1,    83,    84,    -1,
       1,    87,    -1,    -1,    -1,    -1,    92,    93,    -1,    -1,
      -1,    97,    98,    14,    -1,   101,    17,    18,    -1,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    29,    -1,
      31,    -1,    33,    34,    -1,    -1,    -1,    38,    39,    40,
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
      -1,     1,    87,    -1,    -1,    -1,    -1,    92,    93,    -1,
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
      -1,     1,    87,    -1,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    97,    98,    14,    -1,   101,    17,    18,    -1,
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
      -1,    92,    93,    -1,   270,    14,    -1,    98,    17,    18,
     101,    -1,    21,   279,    23,    24,    25,    26,    27,    -1,
      29,    -1,    -1,    -1,    33,    34,   292,    -1,   294,     1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    14,    52,    -1,    17,    18,    -1,    20,    21,
      -1,    23,    24,    25,    26,    27,   322,    29,    -1,    -1,
      69,    33,    34,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,   338,    -1,    -1,    -1,   342,    -1,    50,    -1,
      52,    -1,    -1,    92,    93,    -1,   352,    -1,   354,    98,
      99,   100,   101,   359,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    -1,    -1,    -1,    -1,    98,    -1,    -1,   101,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    46,    -1,    48,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    63,    64,    65,    66,    67,    68,    -1,
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
      48,    -1,    -1,    -1,    -1,    -1,    54,    -1,    89,    90,
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
      -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    63,    64,    65,
      66,    67,    68,    -1,    70,    -1,    -1,    73,    74,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    89,    90,    91,    92,    93,    94,    95,
      63,    64,    65,    66,    67,    68,    56,    70,    -1,    -1,
      73,    74,    75,    63,    64,    65,    66,    67,    68,    -1,
      70,    -1,    -1,    73,    74,    75,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    63,    64,    65,    66,
      67,    68,    -1,    70,    -1,    -1,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    -1,
      -1,    -1,    99,    63,    64,    65,    66,    67,    68,    -1,
      70,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    63,    64,    65,    66,
      67,    68,    -1,    70,    -1,    -1,    73,    74,    75,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    -1,    -1,    73,
      74,    75,    89,    90,    91,    92,    93,    94,    95,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95
};

  /* YYSTOSSTATE-NUM -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    14,    17,    18,    21,    22,    23,    24,    25,
      26,    27,    29,    31,    32,    33,    34,    38,    39,    40,
      41,    42,    45,    47,    50,    51,    52,    55,    59,    60,
      61,    62,    69,    76,    77,    78,    79,    80,    83,    84,
      85,    92,    93,    98,   101,   105,   106,   107,   108,   109,
     120,   126,   130,   131,   132,   133,   134,   141,   143,   145,
     149,   154,   157,    97,    26,   130,    26,    50,    98,   131,
     155,    98,   155,   130,    48,    98,   131,   156,   144,   148,
      51,   135,    46,    47,    48,    49,   131,   142,   146,    51,
      20,   131,    56,    57,    17,    18,    19,    26,    33,    26,
      26,   131,   131,   142,   137,   131,   131,   131,   131,   131,
     131,   131,   136,   150,   131,   131,   131,    27,    29,   103,
     131,   158,   159,   160,   161,     0,    97,   131,    63,    64,
      65,    66,    67,    68,    70,    73,    74,    75,    89,    90,
      91,    92,    93,    94,    95,   127,    98,   131,   131,   155,
     100,   155,   130,   155,   100,   155,   127,    26,   138,   131,
      46,   142,    26,    26,    44,    71,    72,    26,   127,    54,
      65,   130,   130,    54,    65,    54,    53,    56,   142,    26,
     140,    48,    54,    48,    54,    48,    54,    48,    54,    81,
      82,    54,    97,    26,   139,    26,    99,   103,   103,   102,
     100,   100,   105,   123,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,    26,   151,    99,   131,    99,    99,   131,   151,   100,
     131,   147,   130,    26,   133,   131,   131,    43,    65,   131,
     131,   131,    54,    65,    48,   131,   131,   131,   131,   131,
     100,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   119,   119,   100,   130,   130,   131,    27,    29,   161,
      53,    97,   100,    97,    26,   127,    44,   131,   131,   123,
     131,   131,    26,    26,    48,    48,    48,    48,   108,   129,
      26,   129,   153,    26,   153,   151,   131,   116,   110,   113,
     129,   130,   131,   131,   131,   131,   128,    97,   109,   128,
     124,   129,   129,    97,   127,   125,   125,   124,    54,    88,
     121,    30,   153,   117,   111,   114,    87,   131,    36,    37,
     122,   127,    26,   152,   129,   123,    54,    35,   127,   131,
      86,   100,   118,   131,    54,   129,   123,    26,   129,   112,
     131,   127,    53,    87,   123,   115,    86,   129,   129,   123,
     124,    87,   129,    87
};

  /* YYR1YYN -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   104,   105,   105,   106,   106,   106,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     108,   108,   108,   109,   109,   109,   110,   111,   112,   109,
     113,   114,   115,   109,   116,   117,   118,   109,   109,   109,
     109,   109,   119,   120,   120,   121,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   129,   129,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   132,   132,
     132,   132,   132,   132,   132,   132,   133,   133,   134,   134,
     134,   134,   134,   135,   134,   136,   134,   137,   134,   134,
     134,   134,   134,   138,   138,   139,   139,   140,   140,   141,
     141,   141,   141,   141,   141,   142,   142,   143,   143,   143,
     144,   143,   146,   147,   145,   145,   148,   145,   145,   145,
     150,   149,   151,   151,   151,   152,   152,   152,   153,   154,
     155,   155,   155,   156,   156,   157,   158,   158,   158,   158,
     159,   159,   160,   160,   161,   161
};

  /* YYR2YYN -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     4,     4,     6,
       4,     4,     4,     4,     4,     4,     5,     5,     8,     4,
       1,     1,     1,     1,     1,     8,     0,     0,     0,    14,
       0,     0,     0,    15,     0,     0,     0,    12,     2,     4,
       7,     6,     0,     9,    12,     0,     2,     6,     0,     0,
       0,     1,     0,     0,     0,     2,     2,     1,     1,     1,
       1,     1,     1,     3,     1,     2,     1,     1,     4,     2,
       4,     3,     2,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     2,     2,     4,     4,     4,     6,
       4,     6,     4,     6,     4,     6,     2,     1,     2,     1,
       1,     2,     1,     0,     3,     0,     3,     0,     3,     4,
       2,     4,     2,     1,     3,     1,     3,     1,     3,     1,
       2,     2,     2,     3,     2,     3,     2,     2,     3,     2,
       0,     3,     0,     0,     9,     2,     0,     7,     8,     6,
       0,     3,     0,     1,     3,     0,     1,     3,     0,     2,
       0,     1,     3,     1,     3,     3,     0,     1,     1,     1,
       1,     3,     1,     3,     3,     3
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
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1926 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* FBLTIN  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1932 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* RBLTIN  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1938 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* THEFBLTIN  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1944 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* ID  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1950 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* STRING  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1956 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* HANDLER  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1962 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* SYMBOL  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1968 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 30: /* ENDCLAUSE  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1974 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 31: /* tPLAYACCEL  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1980 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 32: /* tMETHOD  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1986 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 33: /* THEOBJECTFIELD  */
#line 166 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).os; }
#line 1992 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 149: /* on  */
#line 165 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1998 "engines/director/lingo/lingo-gr.cpp"
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
#line 177 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarLocal);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2285 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 184 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2293 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 188 "engines/director/lingo/lingo-gr.y"
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
#line 2309 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 199 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_after); }
#line 2315 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 200 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_before); }
#line 2321 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 201 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), kVarLocal);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 208 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 215 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), kVarLocal);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2357 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 222 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2369 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 229 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2380 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 235 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2391 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 242 "engines/director/lingo/lingo-gr.y"
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
#line 2407 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 253 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).oe);
		delete (yyvsp[-2].objectfield).os;
		(yyval.code) = (yyvsp[0].code); }
#line 2418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25:
#line 270 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-5].code) - (yyvsp[-1].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code) + 2);
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentScript)[(yyvsp[-1].code)] = start; }
#line 2429 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26:
#line 282 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), kVarLocal); }
#line 2437 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27:
#line 286 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2444 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28:
#line 289 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_le); }
#line 2450 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 289 "engines/director/lingo/lingo-gr.y"
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
#line 2473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 313 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), kVarLocal); }
#line 2481 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 317 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2488 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 320 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_ge); }
#line 2494 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 321 "engines/director/lingo/lingo-gr.y"
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
#line 2517 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 346 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  g_lingo->codeFunc(new Common::String("count"), 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
#line 2527 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 352 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
#line 2537 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 358 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get list
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(1);
				  g_lingo->codeFunc(new Common::String("getAt"), 2);
				  g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-6].s)->c_str());
				  mVar((yyvsp[-6].s), kVarLocal);
				  g_lingo->code1(LC::c_assign); }
#line 2551 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 367 "engines/director/lingo/lingo-gr.y"
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
#line 2573 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 385 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_nextRepeat); }
#line 2580 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 387 "engines/director/lingo/lingo-gr.y"
                              {
		g_lingo->code1(LC::c_whencode);
		g_lingo->codeString((yyvsp[-2].s)->c_str()); }
#line 2588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 390 "engines/director/lingo/lingo-gr.y"
                                                          {
		inst end;
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code));
		(*g_lingo->_currentScript)[(yyvsp[-3].code) + 1] = end; }
#line 2597 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 394 "engines/director/lingo/lingo-gr.y"
                                                    {
		inst end;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2606 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 399 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }
#line 2614 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 403 "engines/director/lingo/lingo-gr.y"
                                                                                           {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2626 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 410 "engines/director/lingo/lingo-gr.y"
                                                                                                                  {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-6].code) + 1 - (yyvsp[-9].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-9].code)] = else1;		/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-6].code), (yyvsp[-1].code)); }
#line 2638 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 421 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentScript)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2648 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 427 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, STOP);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2656 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 431 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, STOP);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2664 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 435 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentScript->size() - 1; }
#line 2672 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 439 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2679 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 442 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2685 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 444 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 446 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2697 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 450 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 453 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2713 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 456 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2722 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 460 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2731 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 464 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2740 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 468 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 476 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 478 "engines/director/lingo/lingo-gr.y"
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
#line 2774 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 489 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2780 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 491 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2788 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 494 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2796 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 497 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2804 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 500 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2812 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 503 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2823 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 509 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).oe);
		delete (yyvsp[0].objectfield).os; }
#line 2833 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 514 "engines/director/lingo/lingo-gr.y"
                       {
		g_lingo->code1(LC::c_objectrefpush);
		g_lingo->codeString((yyvsp[0].objectref).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectref).field->c_str());
		delete (yyvsp[0].objectref).obj;
		delete (yyvsp[0].objectref).field; }
#line 2844 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 521 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2850 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 522 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2856 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 523 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2862 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 524 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 525 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 526 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2880 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 527 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2886 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 528 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2892 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 529 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2898 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 530 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 531 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2910 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 532 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 533 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2922 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 534 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 535 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2934 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 536 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2940 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 537 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 538 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2952 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 539 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); }
#line 2958 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 540 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2964 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 541 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_intersects); }
#line 2970 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 542 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 2976 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 544 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charOf); }
#line 2982 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 545 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charToOf); }
#line 2988 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 546 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemOf); }
#line 2994 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 547 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_itemToOf); }
#line 3000 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 548 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineOf); }
#line 3006 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 549 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_lineToOf); }
#line 3012 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 550 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordOf); }
#line 3018 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 551 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_wordToOf); }
#line 3024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 553 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 3032 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 558 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_printtop); }
#line 3038 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 561 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_exitRepeat); }
#line 3044 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 562 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 3050 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 563 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3056 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 563 "engines/director/lingo/lingo-gr.y"
                                                                                 { inNone(); }
#line 3062 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 564 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3068 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 564 "engines/director/lingo/lingo-gr.y"
                                                                                   { inNone(); }
#line 3074 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 565 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3080 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 565 "engines/director/lingo/lingo-gr.y"
                                                                                   { inNone(); }
#line 3086 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 566 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 569 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3102 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 572 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_open); }
#line 3108 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 573 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 3114 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 575 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 580 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3134 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 586 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3144 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 591 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3154 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 597 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3164 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 602 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3174 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 615 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3180 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 616 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3186 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 617 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3192 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 618 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3201 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 622 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3210 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 626 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3219 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 634 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 638 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3237 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 642 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3246 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 646 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 646 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3261 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 676 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3267 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 676 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->_currentFactory.clear(); }
#line 3273 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 677 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-6].s); }
#line 3283 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 682 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3289 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 683 "engines/director/lingo/lingo-gr.y"
                  { startDef(); }
#line 3295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 684 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
		endDef();
		delete (yyvsp[-6].s); }
#line 3305 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 689 "engines/director/lingo/lingo-gr.y"
                                                                     {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3318 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 697 "engines/director/lingo/lingo-gr.y"
                                                 {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3328 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 703 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 703 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.s) = (yyvsp[0].s); g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 3341 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 706 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3347 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 707 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3353 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 708 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3359 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 711 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3365 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 712 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3371 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 714 "engines/director/lingo/lingo-gr.y"
                                        { inDef(); }
#line 3377 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 716 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 3389 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 724 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3395 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 725 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3401 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 726 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3407 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 728 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3413 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 729 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3419 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 731 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3425 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 733 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3431 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 734 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3437 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 735 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3443 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 736 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3449 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 738 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3455 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 739 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3461 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172:
#line 741 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3467 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173:
#line 742 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174:
#line 744 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3482 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175:
#line 748 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3491 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3495 "engines/director/lingo/lingo-gr.cpp"

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

#line 754 "engines/director/lingo/lingo-gr.y"


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
