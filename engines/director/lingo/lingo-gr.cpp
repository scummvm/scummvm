/* A Bison parser, made by GNU Bison 3.7.  */

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
#define YYBISON_VERSION "3.7"

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

using namespace Director;

static void yyerror(const char *s) {
	g_lingo->_hadError = true;
	warning("######################  LINGO: %s at line %d col %d", s, g_lingo->_linenumber, g_lingo->_colnumber);
	if (g_lingo->_lines[2] != g_lingo->_lines[1])
		warning("# %3d: %s", g_lingo->_linenumber - 2, Common::String(g_lingo->_lines[2], g_lingo->_lines[1] - 1).c_str());

	if (g_lingo->_lines[1] != g_lingo->_lines[0])
		warning("# %3d: %s", g_lingo->_linenumber - 1, Common::String(g_lingo->_lines[1], g_lingo->_lines[0] - 1).c_str());

	const char *ptr = g_lingo->_lines[0];

	while (*ptr && *ptr != '\n')
		ptr++;

	warning("# %3d: %s", g_lingo->_linenumber, Common::String(g_lingo->_lines[0], ptr).c_str());

	Common::String arrow;
	for (uint i = 0; i < g_lingo->_colnumber; i++)
		arrow += ' ';

	warning("#      %s^ about here", arrow.c_str());
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


#line 195 "engines/director/lingo/lingo-gr.cpp"

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

#include "lingo-gr.h"
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
  YYSYMBOL_THEFUNC = 22,                   /* THEFUNC  */
  YYSYMBOL_THEFUNCINOF = 23,               /* THEFUNCINOF  */
  YYSYMBOL_VARID = 24,                     /* VARID  */
  YYSYMBOL_STRING = 25,                    /* STRING  */
  YYSYMBOL_SYMBOL = 26,                    /* SYMBOL  */
  YYSYMBOL_ENDCLAUSE = 27,                 /* ENDCLAUSE  */
  YYSYMBOL_tPLAYACCEL = 28,                /* tPLAYACCEL  */
  YYSYMBOL_tMETHOD = 29,                   /* tMETHOD  */
  YYSYMBOL_THEOBJECTPROP = 30,             /* THEOBJECTPROP  */
  YYSYMBOL_tCAST = 31,                     /* tCAST  */
  YYSYMBOL_tFIELD = 32,                    /* tFIELD  */
  YYSYMBOL_tSCRIPT = 33,                   /* tSCRIPT  */
  YYSYMBOL_tWINDOW = 34,                   /* tWINDOW  */
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
  YYSYMBOL_ID = 106,                       /* ID  */
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
  YYSYMBOL_startrepeat = 119,              /* startrepeat  */
  YYSYMBOL_tellstart = 120,                /* tellstart  */
  YYSYMBOL_ifstmt = 121,                   /* ifstmt  */
  YYSYMBOL_elseifstmtlist = 122,           /* elseifstmtlist  */
  YYSYMBOL_elseifstmt = 123,               /* elseifstmt  */
  YYSYMBOL_jumpifz = 124,                  /* jumpifz  */
  YYSYMBOL_jump = 125,                     /* jump  */
  YYSYMBOL_varassign = 126,                /* varassign  */
  YYSYMBOL_if = 127,                       /* if  */
  YYSYMBOL_lbl = 128,                      /* lbl  */
  YYSYMBOL_stmtlist = 129,                 /* stmtlist  */
  YYSYMBOL_stmtlistline = 130,             /* stmtlistline  */
  YYSYMBOL_simpleexprnoparens = 131,       /* simpleexprnoparens  */
  YYSYMBOL_132_10 = 132,                   /* $@10  */
  YYSYMBOL_simpleexpr = 133,               /* simpleexpr  */
  YYSYMBOL_expr = 134,                     /* expr  */
  YYSYMBOL_chunkexpr = 135,                /* chunkexpr  */
  YYSYMBOL_reference = 136,                /* reference  */
  YYSYMBOL_proc = 137,                     /* proc  */
  YYSYMBOL_138_11 = 138,                   /* $@11  */
  YYSYMBOL_139_12 = 139,                   /* $@12  */
  YYSYMBOL_140_13 = 140,                   /* $@13  */
  YYSYMBOL_141_14 = 141,                   /* $@14  */
  YYSYMBOL_globallist = 142,               /* globallist  */
  YYSYMBOL_propertylist = 143,             /* propertylist  */
  YYSYMBOL_instancelist = 144,             /* instancelist  */
  YYSYMBOL_gotofunc = 145,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 146,                /* gotomovie  */
  YYSYMBOL_playfunc = 147,                 /* playfunc  */
  YYSYMBOL_148_15 = 148,                   /* $@15  */
  YYSYMBOL_defn = 149,                     /* defn  */
  YYSYMBOL_150_16 = 150,                   /* $@16  */
  YYSYMBOL_151_17 = 151,                   /* $@17  */
  YYSYMBOL_on = 152,                       /* on  */
  YYSYMBOL_153_18 = 153,                   /* $@18  */
  YYSYMBOL_argdef = 154,                   /* argdef  */
  YYSYMBOL_endargdef = 155,                /* endargdef  */
  YYSYMBOL_argstore = 156,                 /* argstore  */
  YYSYMBOL_arglist = 157,                  /* arglist  */
  YYSYMBOL_nonemptyarglist = 158,          /* nonemptyarglist  */
  YYSYMBOL_list = 159,                     /* list  */
  YYSYMBOL_valuelist = 160,                /* valuelist  */
  YYSYMBOL_linearlist = 161,               /* linearlist  */
  YYSYMBOL_proplist = 162,                 /* proplist  */
  YYSYMBOL_proppair = 163                  /* proppair  */
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
#define YYFINAL  100
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1811

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  103
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  61
/* YYNRULES -- Number of rules.  */
#define YYNRULES  190
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  390

/* YYMAXUTOK -- Last valid token kind.  */
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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   232,   232,   233,   235,   236,   237,   239,   240,   241,
     242,   243,   245,   252,   256,   267,   274,   277,   284,   287,
     294,   301,   308,   315,   321,   328,   339,   350,   357,   365,
     366,   368,   369,   374,   387,   391,   394,   386,   421,   425,
     428,   420,   455,   462,   468,   454,   498,   506,   509,   510,
     511,   513,   515,   517,   524,   532,   533,   535,   541,   545,
     549,   553,   556,   558,   559,   561,   562,   564,   567,   570,
     574,   578,   579,   580,   581,   582,   590,   596,   599,   602,
     608,   609,   610,   611,   617,   617,   622,   625,   634,   635,
     637,   638,   639,   640,   641,   642,   643,   644,   645,   646,
     647,   648,   649,   650,   651,   652,   653,   654,   656,   659,
     660,   661,   662,   663,   664,   665,   666,   668,   669,   672,
     675,   679,   680,   681,   682,   690,   691,   691,   692,   692,
     693,   693,   694,   697,   700,   706,   706,   711,   714,   718,
     721,   725,   728,   732,   735,   746,   747,   748,   749,   753,
     757,   762,   763,   765,   769,   773,   777,   777,   807,   807,
     813,   814,   814,   820,   828,   834,   834,   836,   837,   838,
     840,   841,   842,   844,   846,   847,   848,   850,   851,   853,
     855,   856,   857,   858,   860,   861,   863,   864,   866,   870,
     874
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
  "THEMENUITEMENTITY", "THEMENUITEMSENTITY", "FLOAT", "THEFUNC",
  "THEFUNCINOF", "VARID", "STRING", "SYMBOL", "ENDCLAUSE", "tPLAYACCEL",
  "tMETHOD", "THEOBJECTPROP", "tCAST", "tFIELD", "tSCRIPT", "tWINDOW",
  "tDOWN", "tELSE", "tELSIF", "tEXIT", "tGLOBAL", "tGO", "tGOLOOP", "tIF",
  "tIN", "tINTO", "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT",
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE",
  "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tENDIF",
  "tENDREPEAT", "tENDTELL", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'\\n'", "'('", "')'", "','", "'['", "']'", "':'",
  "$accept", "program", "programline", "ID", "asgn", "stmtoneliner",
  "stmt", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9",
  "startrepeat", "tellstart", "ifstmt", "elseifstmtlist", "elseifstmt",
  "jumpifz", "jump", "varassign", "if", "lbl", "stmtlist", "stmtlistline",
  "simpleexprnoparens", "$@10", "simpleexpr", "expr", "chunkexpr",
  "reference", "proc", "$@11", "$@12", "$@13", "$@14", "globallist",
  "propertylist", "instancelist", "gotofunc", "gotomovie", "playfunc",
  "$@15", "defn", "$@16", "$@17", "on", "$@18", "argdef", "endargdef",
  "argstore", "arglist", "nonemptyarglist", "list", "valuelist",
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
     335,   336,   337,   338,   339,   340,   341,   342,    60,    62,
      38,    43,    45,    42,    47,    37,    10,    40,    41,    44,
      91,    93,    58
};
#endif

#define YYPACT_NINF (-330)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-66)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     523,   -81,  -330,  -330,  -330,  -330,  -330,  -330,  -330,   -28,
    -330,   925,  -330,  -330,  -330,     9,  1039,    -3,   133,    81,
      81,  1120,  1006,  -330,  1120,  -330,  -330,     7,  -330,  1146,
    -330,  -330,  -330,  -330,  1120,  -330,  -330,  -330,  -330,  -330,
    -330,  1120,  -330,  -330,    81,  -330,  -330,  1120,  -330,    20,
    1120,  -330,  -330,  -330,  1227,  1227,  1227,  1227,  1120,  -330,
       2,  -330,  1120,  1120,  1120,  1120,  1120,  1120,  1120,  1120,
    1120,   886,   -23,  -330,  -330,  1285,  -330,  -330,  -330,  -330,
      81,  -330,    81,  1252,    81,  -330,   -43,  1120,  1120,   -38,
      -9,    24,  -330,  1621,  1285,  -330,    81,  1582,    81,    81,
    -330,   523,  1120,  1704,   -20,  1704,    81,   -20,    81,  -330,
     -16,  -330,  1120,  -330,  -330,  -330,  -330,  -330,  1704,  1120,
    -330,  1318,  1351,  1384,  1417,  1671,  -330,  -330,  1634,   -13,
      -5,  -330,   -73,  1704,    21,    30,    38,  -330,  1120,  1120,
    1120,  1120,  1120,  1120,  1120,  1120,  1120,  1120,  1120,  1120,
    1120,  1120,  1120,  1120,  1120,  1120,  -330,  -330,  1120,   232,
     232,   232,   -21,  1120,  1120,  1120,     5,    33,  1120,  1120,
    1120,  1120,  1120,  1120,  -330,  -330,    50,  -330,  -330,  -330,
      56,  -330,  -330,    29,  1634,    12,  1120,   103,  -330,   -69,
     -35,    81,  -330,  1704,  1120,  1120,  1120,  1120,  1120,  1120,
    1120,  1120,  1120,  1120,  -330,  1120,  1120,  1120,  -330,  1120,
      61,    34,    41,   -59,   -59,   -59,   -59,  1717,  1717,  -330,
     -55,   -59,   -59,   -59,   -59,   -55,   -52,   -52,  -330,  -330,
      81,   114,  -330,  -330,  -330,  -330,  -330,  -330,  1120,  1120,
    1704,  1704,  1704,  1120,  1120,    81,  1704,  1704,  1704,  1704,
    1704,  1704,    81,   307,   683,    81,  -330,  -330,  -330,  1704,
     161,  -330,    81,  -330,  -330,  -330,  1450,  -330,  1483,  -330,
    1516,  -330,  1549,  -330,  -330,  1704,  1704,  1704,  1704,   -13,
      -5,    66,  -330,  -330,  -330,  -330,    -8,  1120,  1704,  1200,
    -330,  1704,  1704,  1120,  -330,  -330,  -330,  -330,    73,  -330,
    1120,  -330,   596,  -330,   649,  1120,  1120,  1120,  1120,  1120,
    -330,  1704,  -330,  -330,  -330,   749,     8,  -330,    83,   448,
    1704,    47,  -330,   144,  -330,  -330,  -330,  -330,  -330,    49,
     649,  -330,  -330,  -330,  -330,  1120,  1120,  -330,  -330,  -330,
    1120,   117,    81,  -330,  -330,  -330,   119,   140,    91,  1704,
    1704,  1704,   810,  1120,  -330,    94,  -330,    84,  -330,  1120,
     128,  -330,  -330,  1704,  -330,    81,  -330,  1704,  1120,    99,
     134,  -330,   749,  -330,  1704,  -330,   161,   100,  -330,  -330,
    -330,  -330,   749,  -330,  -330,   102,   749,  -330,   104,  -330
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     7,   156,   161,     8,     9,    10,    11,   125,
     126,     0,   145,    61,   158,     0,     0,     0,     0,     0,
       0,     0,     0,   130,     0,   128,   165,     0,     2,   174,
      29,    31,     6,    32,     0,    30,   122,   123,     5,    62,
      50,   174,    62,   124,     0,    67,    75,     0,    68,     0,
       0,    70,    69,    79,     8,     9,    10,    11,     0,   146,
       0,   147,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   180,    87,    88,    90,   148,   117,    74,   150,    82,
       0,    46,     0,   121,     0,    62,     0,     0,     0,     0,
       0,     0,   160,   133,   153,   155,     0,     0,     0,     0,
       1,     0,   174,   175,   138,    58,   167,   157,   167,   139,
     127,    76,     0,    77,   118,   108,   119,   120,   152,     0,
      73,     0,     0,     0,     0,     0,    71,    72,     0,    70,
      69,   181,    87,   184,     0,   183,   182,   186,   174,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   149,    62,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   154,   143,   131,    52,    52,   141,
     129,   166,     3,    87,   175,     0,     0,     0,   168,     0,
       0,     0,    78,   151,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    89,     0,     0,     0,   179,     0,
       0,    87,     0,   100,   101,    98,    99,   102,   103,    95,
     105,   106,   107,    97,    96,   104,    91,    92,    93,    94,
     167,     0,    12,    13,    15,    16,    17,    18,     0,     0,
      58,    22,    20,     0,     0,     0,    27,    28,    21,    19,
      47,   132,     0,     0,     0,     0,   134,   135,   137,   176,
       0,   173,     0,   173,   140,   109,     0,   111,     0,   113,
       0,   115,     0,    80,    81,   189,   188,   190,   185,     0,
       0,     0,   187,    83,    84,    86,     0,     0,    42,    34,
      51,    23,    24,     0,   144,    62,    66,    62,    63,   142,
       0,    59,     0,   169,     0,     0,     0,     0,     0,     0,
     173,    14,    62,    60,    60,     0,     0,    49,     0,     0,
     177,     0,    55,   164,   162,   110,   112,   114,   116,     0,
       0,    43,    35,    39,    59,     0,     0,    48,    64,   136,
       0,    62,   170,    85,   159,    58,     0,     0,     0,    25,
      26,   178,     0,     0,    56,     0,   171,   163,    44,     0,
       0,    33,    62,    58,    53,     0,    51,    36,     0,     0,
       0,   172,     0,    58,    40,    54,     0,     0,    51,    58,
      59,    45,     0,    51,    57,     0,     0,    37,     0,    41
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -330,  -330,    95,     0,  -330,   -49,     3,  -330,  -330,  -330,
    -330,  -330,  -330,  -330,  -330,  -330,  -308,    27,  -330,  -330,
    -330,  -238,  -329,  -108,  -330,   -36,  -252,  -330,    62,  -330,
     -22,   258,   -17,  -330,  -330,  -330,  -330,  -330,  -330,  -330,
    -330,  -330,  -330,   -18,  -330,  -330,  -330,  -330,  -330,  -330,
    -330,  -107,  -330,  -254,   -29,  -102,  -330,  -330,  -330,  -330,
      -1
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    27,    28,    72,    30,    31,   296,   313,   346,   373,
     314,   347,   379,   312,   345,   366,   315,   253,    33,   341,
     354,   187,   322,   332,    34,   106,   297,   298,    73,   309,
      74,   103,    76,    77,    35,    44,    98,    96,   300,   110,
     180,   176,    36,    78,    37,    41,    38,    80,    42,    39,
      99,   189,   357,   302,   104,   321,    79,   134,   135,   136,
     137
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      29,   190,   290,    32,    95,   348,   108,   100,   301,   304,
     145,   164,   107,   146,   145,    40,   168,   145,    90,    91,
      92,   165,   238,    43,   138,   111,   169,   261,   113,   207,
     262,   151,   152,   153,   154,   155,   152,   153,   154,   155,
     120,   154,   155,   239,   109,   170,   126,   127,   119,   163,
     323,   384,   324,    84,    85,   171,   330,   156,   372,   243,
      81,   263,   335,   334,   262,   166,   167,   338,   112,   244,
     382,   132,   336,   185,   138,   386,   174,   172,   344,   186,
     157,   245,   158,   191,   162,     2,   279,   280,   310,   205,
     192,   262,     5,     6,     7,     8,   175,   206,   179,   181,
     362,    29,   183,   101,    32,     2,   188,   358,   188,   212,
     258,   186,     5,     6,     7,     8,   114,   115,   116,   117,
     377,   230,   208,   286,   380,   370,   138,   256,   257,   209,
     385,   138,   283,   284,   388,   378,   231,   210,   211,   285,
     186,   383,   233,   235,   237,   339,   340,   343,   340,   252,
      86,    87,    88,   352,   353,   255,   260,     2,   287,   232,
     234,   236,     1,    89,     5,     6,     7,     8,   207,   319,
     337,   342,   265,   359,   267,   360,   269,   361,   271,   364,
     273,   274,   368,   365,   375,     2,   381,   376,   387,     3,
     389,   264,     5,     6,     7,     8,   182,   -65,   -65,     9,
      10,    11,    12,    13,   295,   254,   333,   329,    15,   282,
     281,    16,    17,    18,     0,     0,    19,     0,     0,     0,
      21,    22,    23,     0,     0,     0,     0,     0,     0,     0,
     188,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    24,    25,   293,   -65,     0,     0,     0,
       0,     0,   294,    29,    29,   299,     2,   -65,     0,   317,
      29,   318,   303,     5,    55,     7,     8,     0,     0,    75,
       0,   316,     0,     0,    83,     0,   331,     0,     0,    93,
      94,     0,    97,   325,   326,   327,   328,     0,     0,     0,
       0,     0,   105,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    29,     0,    29,   355,     0,    63,    64,    65,
      66,     0,     0,     0,     0,    29,   118,     0,     0,    29,
       0,   121,   122,   123,   124,   125,   369,     0,   128,   133,
      29,     2,     0,     0,     0,     3,     0,     0,     5,     6,
       7,     8,   356,     0,     0,     9,    10,    11,    12,     0,
       0,     0,    29,     0,     0,     0,     0,    16,     0,    18,
     184,     0,     0,     0,     0,   371,    21,    22,    23,     0,
       0,     0,    29,     0,     0,     0,    29,   193,     0,     0,
       0,     0,    29,     0,     0,     0,    29,     0,     0,     0,
      25,     0,     0,     0,     0,     0,     0,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,     0,     0,     0,     0,     0,     0,
       0,   240,   241,   242,     0,     0,   246,   247,   248,   249,
     250,   251,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   259,     0,     0,     0,   -65,     1,
       0,     0,     0,   266,     0,   268,     0,   270,     0,   272,
       0,     0,     0,   275,   276,   277,     0,   278,     0,     0,
       0,     0,     2,     0,     0,   -65,     3,     0,     0,     5,
       6,     7,     8,     0,   -65,   -65,     9,    10,    11,    12,
      13,     0,     0,     0,     0,    15,   288,   289,    16,    17,
      18,   291,   292,    19,     0,     0,     0,    21,    22,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    -4,     1,     0,     0,     0,     0,     0,
      24,    25,     0,   -65,   -65,   -65,     0,     0,     0,     0,
       0,     0,     0,     0,   -65,   311,     0,     2,     0,     0,
       0,     3,     4,     0,     5,     6,     7,     8,   320,     0,
       0,     9,    10,    11,    12,    13,     0,   320,    14,     0,
      15,     0,     0,    16,    17,    18,     0,     0,    19,     0,
       0,    20,    21,    22,    23,     0,     0,     0,     0,     0,
       0,     0,     0,   349,   350,     0,   -65,     1,   351,     0,
       0,     0,     0,     0,     0,    24,    25,    26,     0,     0,
       0,   363,     0,     0,     0,     0,     0,   367,     0,    -4,
       2,     0,     0,   -65,     3,     0,   374,     5,     6,     7,
       8,     0,     0,     0,     9,    10,    11,    12,    13,     0,
       0,     0,     0,    15,     0,     0,    16,    17,    18,   -65,
       1,    19,     0,     0,     0,    21,    22,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     2,     0,     0,     0,     3,    24,    25,
       5,     6,     7,     8,     1,     0,     0,     9,    10,    11,
      12,    13,   -65,     0,     0,     0,    15,     0,     0,    16,
      17,    18,     0,     0,    19,     0,     0,     2,    21,    22,
      23,     3,     0,     0,     5,     6,     7,     8,     0,     0,
       0,     9,    10,    11,    12,    13,     0,     0,     0,     0,
      15,    24,    25,    16,    17,    18,     0,     0,    19,     0,
       0,     0,    21,    22,    23,   -65,     0,     0,     0,     0,
       1,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    24,    25,     0,     0,     0,
     -65,     0,     0,     2,     0,     0,     0,     3,     0,   -65,
       5,     6,     7,     8,     0,     0,     0,     9,    10,    11,
      12,    13,     0,     0,     0,     0,    15,     0,     0,    16,
      17,    18,     0,     0,    19,     0,     0,     0,    21,    22,
      23,     1,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    24,    25,     0,     2,   -65,     0,     0,     3,     0,
       0,     5,     6,     7,     8,   -65,     0,     0,     9,    10,
      11,    12,    13,     0,     0,     0,     0,    15,     0,     0,
      16,    17,    18,     0,     0,    19,     0,     0,     0,    21,
      22,    23,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,     0,   -65,     0,     0,     0,     0,
      45,     0,     0,    46,    47,     0,   -65,    48,    49,    50,
       2,   129,   130,     0,     0,     0,    53,    54,    55,    56,
      57,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    45,
       0,     0,    46,    47,     0,     0,    48,    49,    50,     2,
      51,    52,     0,     0,    62,    53,    54,    55,    56,    57,
       0,    63,    64,    65,    66,    67,     0,     0,     0,     0,
       0,    58,    59,    60,    61,     0,     0,    68,    69,     0,
       0,     0,     0,    70,     0,     0,    71,     0,   131,     0,
       0,     0,     0,    62,     0,     0,     0,     0,     0,     0,
      63,    64,    65,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    68,    69,     0,     0,
      45,     0,    70,    46,    47,    71,     0,    48,    49,    50,
       2,    51,    52,     0,     0,     0,    53,    54,    55,    56,
      57,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    58,    45,    60,     0,    46,    47,     0,    82,
      48,    49,    50,     2,    51,    52,     0,     0,     0,    53,
      54,    55,    56,    57,    62,     0,     0,     0,     0,     0,
       0,    63,    64,    65,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    68,    69,     0,
       0,     0,     0,    70,     0,     0,    71,    62,     0,     0,
       0,     0,     0,     0,    63,    64,    65,    66,    67,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      68,    69,     0,     0,    45,     0,    70,    46,    47,    71,
       0,    48,    49,    50,     2,    51,    52,     0,     0,     0,
      53,    54,    55,    56,    57,     0,     0,     0,     0,     0,
      45,     0,     0,    46,    47,     0,     0,    48,    49,    50,
       2,    51,    52,     0,     0,     0,    53,    54,    55,    56,
      57,     0,     0,     0,     0,     0,     0,     0,    62,     0,
       0,     0,     0,     0,     0,    63,    64,    65,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    68,    69,     0,    62,     0,     0,    70,     0,     0,
      71,    63,    64,    65,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   -38,     0,    68,    69,     0,
       0,    45,     0,   102,    46,    47,    71,     0,    48,    49,
      50,     2,    51,    52,     0,     0,     0,    53,    54,    55,
      56,    57,   139,   140,   141,   142,   143,   144,     0,   145,
       0,     0,   146,   147,   148,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   149,   150,
     151,   152,   153,   154,   155,    62,   159,     0,     0,     0,
       0,     0,    63,    64,    65,    66,    67,     0,     0,     0,
       0,     0,     0,     0,   139,   140,   141,   142,   143,   144,
       0,   145,   160,   161,   146,   147,   148,    71,     0,     0,
       0,    58,     0,    60,     0,     0,     0,     0,     0,     0,
     149,   150,   151,   152,   153,   154,   155,   139,   140,   141,
     142,   143,   144,     0,   145,     0,     0,   146,   147,   148,
       0,     0,     0,     0,     0,     0,   194,     0,     0,     0,
       0,     0,   195,   149,   150,   151,   152,   153,   154,   155,
     139,   140,   141,   142,   143,   144,     0,   145,     0,     0,
     146,   147,   148,     0,     0,     0,     0,     0,     0,   196,
       0,     0,     0,     0,     0,   197,   149,   150,   151,   152,
     153,   154,   155,   139,   140,   141,   142,   143,   144,     0,
     145,     0,     0,   146,   147,   148,     0,     0,     0,     0,
       0,     0,   198,     0,     0,     0,     0,     0,   199,   149,
     150,   151,   152,   153,   154,   155,   139,   140,   141,   142,
     143,   144,     0,   145,     0,     0,   146,   147,   148,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
       0,   201,   149,   150,   151,   152,   153,   154,   155,   139,
     140,   141,   142,   143,   144,     0,   145,     0,     0,   146,
     147,   148,     0,     0,     0,     0,     0,     0,   305,     0,
       0,     0,     0,     0,     0,   149,   150,   151,   152,   153,
     154,   155,   139,   140,   141,   142,   143,   144,     0,   145,
       0,     0,   146,   147,   148,     0,     0,     0,     0,     0,
       0,   306,     0,     0,     0,     0,     0,     0,   149,   150,
     151,   152,   153,   154,   155,   139,   140,   141,   142,   143,
     144,     0,   145,     0,     0,   146,   147,   148,     0,     0,
       0,     0,     0,     0,   307,     0,     0,     0,     0,     0,
       0,   149,   150,   151,   152,   153,   154,   155,   139,   140,
     141,   142,   143,   144,     0,   145,     0,     0,   146,   147,
     148,     0,     0,     0,     0,     0,     0,   308,     0,     0,
       0,     0,     0,     0,   149,   150,   151,   152,   153,   154,
     155,   139,   140,   141,   142,   143,   144,     0,   145,     0,
       0,   146,   147,   148,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   177,   149,   150,   151,
     152,   153,   154,   155,   139,   140,   141,   142,   143,   144,
       0,   145,     0,     0,   146,   147,   148,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     149,   150,   151,   152,   153,   154,   155,   173,   178,     0,
       0,     0,     0,   139,   140,   141,   142,   143,   144,     0,
     145,     0,     0,   146,   147,   148,   139,   140,   141,   142,
     143,   144,     0,   145,     0,     0,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,     0,     0,     0,     0,
       0,     0,   149,   150,   151,   152,   153,   154,   155,     0,
       0,     0,   204,   139,   140,   141,   142,   143,   144,     0,
     145,     0,     0,   146,   147,   148,     0,     0,     0,     0,
       0,   202,   203,     0,     0,     0,     0,     0,     0,   149,
     150,   151,   152,   153,   154,   155,   139,   140,   141,   142,
     143,   144,     0,   145,     0,     0,   146,   147,   148,   139,
     140,   141,   142,     0,     0,     0,   145,     0,     0,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,     0,
       0,     0,     0,     0,     0,   149,   150,   151,   152,   153,
     154,   155
};

static const yytype_int16 yycheck[] =
{
       0,   108,   240,     0,    22,   334,    42,     0,   260,   263,
      69,    54,    41,    72,    69,    96,    54,    69,    18,    19,
      20,    64,    43,    51,    97,    47,    64,    96,    50,   102,
      99,    90,    91,    92,    93,    94,    91,    92,    93,    94,
      62,    93,    94,    64,    44,    54,    68,    69,    46,    85,
     302,   380,   304,    56,    57,    64,   310,    75,   366,    54,
      51,    96,    54,   315,    99,    87,    88,   319,    48,    64,
     378,    71,    64,   102,    97,   383,    94,    53,   330,    99,
      80,    48,    82,    99,    84,    24,    25,    26,    96,   102,
     112,    99,    31,    32,    33,    34,    96,   102,    98,    99,
     352,   101,   102,    96,   101,    24,   106,   345,   108,   138,
      98,    99,    31,    32,    33,    34,    54,    55,    56,    57,
     372,   157,   101,   230,   376,   363,    97,    98,    99,    99,
     382,    97,    98,    99,   386,   373,   158,    99,   138,    98,
      99,   379,   159,   160,   161,    98,    99,    98,    99,    99,
      17,    18,    19,    36,    37,    99,    53,    24,    44,   159,
     160,   161,     1,    30,    31,    32,    33,    34,   102,    96,
      87,    27,   194,    54,   196,    35,   198,    86,   200,    85,
     202,   203,    54,    99,    85,    24,    86,    53,    86,    28,
      86,   191,    31,    32,    33,    34,   101,    36,    37,    38,
      39,    40,    41,    42,   253,   178,   314,   309,    47,   210,
     210,    50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,
      59,    60,    61,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     230,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    82,    83,   245,    85,    -1,    -1,    -1,
      -1,    -1,   252,   253,   254,   255,    24,    96,    -1,   295,
     260,   297,   262,    31,    32,    33,    34,    -1,    -1,    11,
      -1,   293,    -1,    -1,    16,    -1,   312,    -1,    -1,    21,
      22,    -1,    24,   305,   306,   307,   308,    -1,    -1,    -1,
      -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   302,    -1,   304,   341,    -1,    75,    76,    77,
      78,    -1,    -1,    -1,    -1,   315,    58,    -1,    -1,   319,
      -1,    63,    64,    65,    66,    67,   362,    -1,    70,    71,
     330,    24,    -1,    -1,    -1,    28,    -1,    -1,    31,    32,
      33,    34,   342,    -1,    -1,    38,    39,    40,    41,    -1,
      -1,    -1,   352,    -1,    -1,    -1,    -1,    50,    -1,    52,
     102,    -1,    -1,    -1,    -1,   365,    59,    60,    61,    -1,
      -1,    -1,   372,    -1,    -1,    -1,   376,   119,    -1,    -1,
      -1,    -1,   382,    -1,    -1,    -1,   386,    -1,    -1,    -1,
      83,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   163,   164,   165,    -1,    -1,   168,   169,   170,   171,
     172,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,     0,     1,
      -1,    -1,    -1,   195,    -1,   197,    -1,   199,    -1,   201,
      -1,    -1,    -1,   205,   206,   207,    -1,   209,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    27,    28,    -1,    -1,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    -1,    47,   238,   239,    50,    51,
      52,   243,   244,    55,    -1,    -1,    -1,    59,    60,    61,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     0,     1,    -1,    -1,    -1,    -1,    -1,
      82,    83,    -1,    85,    86,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    96,   287,    -1,    24,    -1,    -1,
      -1,    28,    29,    -1,    31,    32,    33,    34,   300,    -1,
      -1,    38,    39,    40,    41,    42,    -1,   309,    45,    -1,
      47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,
      -1,    58,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   335,   336,    -1,     0,     1,   340,    -1,
      -1,    -1,    -1,    -1,    -1,    82,    83,    84,    -1,    -1,
      -1,   353,    -1,    -1,    -1,    -1,    -1,   359,    -1,    96,
      24,    -1,    -1,    27,    28,    -1,   368,    31,    32,    33,
      34,    -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,     0,
       1,    55,    -1,    -1,    -1,    59,    60,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    -1,    -1,    -1,    28,    82,    83,
      31,    32,    33,    34,     1,    -1,    -1,    38,    39,    40,
      41,    42,    96,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    -1,    -1,    55,    -1,    -1,    24,    59,    60,
      61,    28,    -1,    -1,    31,    32,    33,    34,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,
      47,    82,    83,    50,    51,    52,    -1,    -1,    55,    -1,
      -1,    -1,    59,    60,    61,    96,    -1,    -1,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    82,    83,    -1,    -1,    -1,
      87,    -1,    -1,    24,    -1,    -1,    -1,    28,    -1,    96,
      31,    32,    33,    34,    -1,    -1,    -1,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    -1,    -1,    55,    -1,    -1,    -1,    59,    60,
      61,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    82,    83,    -1,    24,    86,    -1,    -1,    28,    -1,
      -1,    31,    32,    33,    34,    96,    -1,    -1,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,    59,
      60,    61,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    82,    83,    -1,    85,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    17,    18,    -1,    96,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    30,    31,    32,    33,
      34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    14,
      -1,    -1,    17,    18,    -1,    -1,    21,    22,    23,    24,
      25,    26,    -1,    -1,    68,    30,    31,    32,    33,    34,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    46,    47,    48,    49,    -1,    -1,    91,    92,    -1,
      -1,    -1,    -1,    97,    -1,    -1,   100,    -1,   102,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    -1,    -1,
      14,    -1,    97,    17,    18,   100,    -1,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    30,    31,    32,    33,
      34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    14,    48,    -1,    17,    18,    -1,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    30,
      31,    32,    33,    34,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    -1,
      -1,    -1,    -1,    97,    -1,    -1,   100,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    -1,    -1,    14,    -1,    97,    17,    18,   100,
      -1,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    17,    18,    -1,    -1,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    30,    31,    32,    33,
      34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    -1,    68,    -1,    -1,    97,    -1,    -1,
     100,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    -1,    91,    92,    -1,
      -1,    14,    -1,    97,    17,    18,   100,    -1,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    31,    32,
      33,    34,    62,    63,    64,    65,    66,    67,    -1,    69,
      -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    68,    44,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    73,    74,   100,    -1,    -1,
      -1,    46,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    62,    63,    64,
      65,    66,    67,    -1,    69,    -1,    -1,    72,    73,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    54,    88,    89,    90,    91,    92,    93,    94,
      62,    63,    64,    65,    66,    67,    -1,    69,    -1,    -1,
      72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    54,    88,    89,    90,    91,
      92,    93,    94,    62,    63,    64,    65,    66,    67,    -1,
      69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    54,    88,
      89,    90,    91,    92,    93,    94,    62,    63,    64,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,
      -1,    54,    88,    89,    90,    91,    92,    93,    94,    62,
      63,    64,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    62,    63,    64,    65,    66,    67,    -1,    69,
      -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    62,    63,    64,    65,    66,
      67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    62,    63,
      64,    65,    66,    67,    -1,    69,    -1,    -1,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    62,    63,    64,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    88,    89,    90,
      91,    92,    93,    94,    62,    63,    64,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    56,    96,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    66,    67,    -1,
      69,    -1,    -1,    72,    73,    74,    62,    63,    64,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    88,
      89,    90,    91,    92,    93,    94,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    -1,
      -1,    -1,    98,    62,    63,    64,    65,    66,    67,    -1,
      69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,
      -1,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    62,    63,    64,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    -1,    -1,    72,
      73,    74,    88,    89,    90,    91,    92,    93,    94,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    24,    28,    29,    31,    32,    33,    34,    38,
      39,    40,    41,    42,    45,    47,    50,    51,    52,    55,
      58,    59,    60,    61,    82,    83,    84,   104,   105,   106,
     107,   108,   109,   121,   127,   137,   145,   147,   149,   152,
      96,   148,   151,    51,   138,    14,    17,    18,    21,    22,
      23,    25,    26,    30,    31,    32,    33,    34,    46,    47,
      48,    49,    68,    75,    76,    77,    78,    79,    91,    92,
      97,   100,   106,   131,   133,   134,   135,   136,   146,   159,
     150,    51,    20,   134,    56,    57,    17,    18,    19,    30,
     106,   106,   106,   134,   134,   146,   140,   134,   139,   153,
       0,    96,    97,   134,   157,   134,   128,   157,   128,   106,
     142,   133,    48,   133,   131,   131,   131,   131,   134,    46,
     133,   134,   134,   134,   134,   134,   133,   133,   134,    25,
      26,   102,   106,   134,   160,   161,   162,   163,    97,    62,
      63,    64,    65,    66,    67,    69,    72,    73,    74,    88,
      89,    90,    91,    92,    93,    94,   146,   106,   106,    44,
      70,    71,   106,   128,    54,    64,   133,   133,    54,    64,
      54,    64,    53,    56,   146,   106,   144,    54,    96,   106,
     143,   106,   105,   106,   134,   157,    99,   124,   106,   154,
     154,    99,   133,   134,    48,    54,    48,    54,    48,    54,
      48,    54,    80,    81,    98,   102,   102,   102,   101,    99,
      99,   106,   157,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     128,   133,   106,   135,   106,   135,   106,   135,    43,    64,
     134,   134,   134,    54,    64,    48,   134,   134,   134,   134,
     134,   134,    99,   120,   120,    99,    98,    99,    98,   134,
      53,    96,    99,    96,   106,   133,   134,   133,   134,   133,
     134,   133,   134,   133,   133,   134,   134,   134,   134,    25,
      26,   106,   163,    98,    99,    98,   154,    44,   134,   134,
     124,   134,   134,   106,   106,   108,   109,   129,   130,   106,
     141,   129,   156,   106,   156,    48,    48,    48,    48,   132,
      96,   134,   116,   110,   113,   119,   133,   128,   128,    96,
     134,   158,   125,   129,   129,   133,   133,   133,   133,   158,
     156,   128,   126,   126,   129,    54,    64,    87,   129,    98,
      99,   122,    27,    98,   129,   117,   111,   114,   125,   134,
     134,   134,    36,    37,   123,   128,   106,   155,   124,    54,
      35,    86,   129,   134,    85,    99,   118,   134,    54,   128,
     124,   106,   119,   112,   134,    85,    53,   129,   124,   115,
     129,    86,   119,   124,   125,   129,   119,    86,   129,    86
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   103,   104,   104,   105,   105,   105,   106,   106,   106,
     106,   106,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   108,
     108,   109,   109,   109,   110,   111,   112,   109,   113,   114,
     115,   109,   116,   117,   118,   109,   109,   109,   109,   109,
     109,   119,   120,   121,   121,   122,   122,   123,   124,   125,
     126,   127,   128,   129,   129,   130,   130,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   132,   131,   131,   131,   133,   133,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   136,   136,   136,
     136,   137,   137,   137,   137,   137,   138,   137,   139,   137,
     140,   137,   137,   137,   137,   141,   137,   137,   137,   142,
     142,   143,   143,   144,   144,   145,   145,   145,   145,   145,
     145,   146,   146,   147,   147,   147,   148,   147,   150,   149,
     149,   151,   149,   149,   149,   153,   152,   154,   154,   154,
     155,   155,   155,   156,   157,   157,   157,   158,   158,   159,
     160,   160,   160,   160,   161,   161,   162,   162,   163,   163,
     163
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     4,     4,     6,     4,     4,     4,     4,     4,
       4,     4,     4,     5,     5,     8,     8,     4,     4,     1,
       1,     1,     1,     9,     0,     0,     0,    15,     0,     0,
       0,    16,     0,     0,     0,    13,     2,     4,     7,     6,
       2,     0,     0,     9,    11,     0,     2,     6,     0,     0,
       0,     1,     0,     1,     3,     0,     1,     1,     1,     1,
       1,     2,     2,     2,     1,     1,     2,     2,     3,     1,
       4,     4,     1,     4,     0,     7,     4,     1,     1,     3,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     4,
       6,     4,     6,     4,     6,     4,     6,     1,     2,     2,
       2,     2,     1,     1,     2,     1,     0,     3,     0,     3,
       0,     3,     4,     2,     4,     0,     7,     4,     2,     1,
       3,     1,     3,     1,     3,     1,     2,     2,     2,     3,
       2,     3,     2,     2,     3,     2,     0,     3,     0,     8,
       2,     0,     7,     8,     6,     0,     3,     0,     1,     3,
       0,     1,     3,     0,     0,     1,     3,     1,     3,     3,
       0,     1,     1,     1,     1,     3,     1,     3,     3,     3,
       3
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
    case YYSYMBOL_THEFUNC: /* THEFUNC  */
#line 228 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1714 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_THEFUNCINOF: /* THEFUNCINOF  */
#line 228 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1720 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_VARID: /* VARID  */
#line 228 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1726 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_STRING: /* STRING  */
#line 228 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1732 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_SYMBOL: /* SYMBOL  */
#line 228 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1738 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ENDCLAUSE: /* ENDCLAUSE  */
#line 228 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1744 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tPLAYACCEL: /* tPLAYACCEL  */
#line 228 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1750 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tMETHOD: /* tMETHOD  */
#line 228 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1756 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 228 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1762 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_on: /* on  */
#line 228 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1768 "engines/director/lingo/lingo-gr.cpp"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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
  case 8: /* ID: tCAST  */
#line 240 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2036 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9: /* ID: tFIELD  */
#line 241 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2042 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* ID: tSCRIPT  */
#line 242 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2048 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* ID: tWINDOW  */
#line 243 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2054 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* asgn: tPUT expr tINTO ID  */
#line 245 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2066 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* asgn: tPUT expr tINTO chunkexpr  */
#line 252 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2074 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* asgn: tPUT THEMENUITEMSENTITY ID simpleexpr tINTO expr  */
#line 256 "engines/director/lingo/lingo-gr.y"
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
#line 2090 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* asgn: tPUT expr tAFTER ID  */
#line 267 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_putafter);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2102 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* asgn: tPUT expr tAFTER chunkexpr  */
#line 274 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_putafter);
		(yyval.code) = (yyvsp[-2].code); }
#line 2110 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* asgn: tPUT expr tBEFORE ID  */
#line 277 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_putbefore);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2122 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18: /* asgn: tPUT expr tBEFORE chunkexpr  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_putbefore);
		(yyval.code) = (yyvsp[-2].code); }
#line 2130 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* asgn: tSET ID tEQ expr  */
#line 287 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* asgn: tSET THEENTITY tEQ expr  */
#line 294 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2154 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* asgn: tSET ID tTO expr  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2166 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* asgn: tSET THEENTITY tTO expr  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2178 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* asgn: tSET THEENTITYWITHID simpleexpr tTO expr  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2189 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* asgn: tSET THEENTITYWITHID simpleexpr tEQ expr  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2200 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* asgn: tSET THEMENUITEMENTITY simpleexpr tOF ID simpleexpr tTO expr  */
#line 328 "engines/director/lingo/lingo-gr.y"
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
#line 2216 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* asgn: tSET THEMENUITEMENTITY simpleexpr tOF ID simpleexpr tEQ expr  */
#line 339 "engines/director/lingo/lingo-gr.y"
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
#line 2232 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* asgn: tSET THEOBJECTPROP tTO expr  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2244 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* asgn: tSET THEOBJECTPROP tEQ expr  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2256 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* stmt: tREPEAT tWHILE lbl expr jumpifz startrepeat stmtlist jump tENDREPEAT  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-6].code) - (yyvsp[-1].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-4].code) + 2);
		(*g_lingo->_currentAssembly)[(yyvsp[-4].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentAssembly)[(yyvsp[-1].code)] = start;	/* looping back */
		endRepeat((yyvsp[-1].code) + 1, (yyvsp[-6].code));	}
#line 2268 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* $@1: %empty  */
#line 387 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* $@2: %empty  */
#line 391 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2283 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* $@3: %empty  */
#line 394 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_le); }
#line 2289 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* stmt: tREPEAT tWITH ID tEQ expr $@1 varassign $@2 tTO expr $@3 jumpifz startrepeat stmtlist tENDREPEAT  */
#line 394 "engines/director/lingo/lingo-gr.y"
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
#line 2314 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* $@4: %empty  */
#line 421 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* $@5: %empty  */
#line 425 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2329 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* $@6: %empty  */
#line 428 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_ge); }
#line 2335 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* stmt: tREPEAT tWITH ID tEQ expr $@4 varassign $@5 tDOWN tTO expr $@6 jumpifz startrepeat stmtlist tENDREPEAT  */
#line 429 "engines/director/lingo/lingo-gr.y"
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
#line 2360 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* $@7: %empty  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  Common::String count("count");
				  g_lingo->codeFunc(&count, 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
#line 2371 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* $@8: %empty  */
#line 462 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
#line 2381 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* $@9: %empty  */
#line 468 "engines/director/lingo/lingo-gr.y"
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
#line 2396 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* stmt: tREPEAT tWITH ID tIN expr $@7 lbl $@8 jumpifz $@9 startrepeat stmtlist tENDREPEAT  */
#line 478 "engines/director/lingo/lingo-gr.y"
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
#line 2420 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* stmt: tNEXT tREPEAT  */
#line 498 "engines/director/lingo/lingo-gr.y"
                                        {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->nexts.push_back(pos);
		} else {
			warning("# LINGO: next repeat not inside repeat block");
		} }
#line 2433 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* stmt: tWHEN ID tTHEN expr  */
#line 506 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_whencode);
		g_lingo->codeString((yyvsp[-2].s)->c_str()); }
#line 2441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* stmt: tTELL expr '\n' tellstart stmtlist lbl tENDTELL  */
#line 509 "engines/director/lingo/lingo-gr.y"
                                                          { g_lingo->code1(LC::c_telldone); }
#line 2447 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* stmt: tTELL expr tTO tellstart stmtoneliner lbl  */
#line 510 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->code1(LC::c_telldone); }
#line 2453 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* stmt: error '\n'  */
#line 511 "engines/director/lingo/lingo-gr.y"
                                        { yyerrok; }
#line 2459 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* startrepeat: %empty  */
#line 513 "engines/director/lingo/lingo-gr.y"
                                { startRepeat(); }
#line 2465 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* tellstart: %empty  */
#line 515 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_tell); }
#line 2471 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* ifstmt: if expr jumpifz tTHEN stmtlist jump elseifstmtlist lbl tENDIF  */
#line 517 "engines/director/lingo/lingo-gr.y"
                                                                                         {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2483 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* ifstmt: if expr jumpifz tTHEN stmtlist jump elseifstmtlist tELSE stmtlist lbl tENDIF  */
#line 524 "engines/director/lingo/lingo-gr.y"
                                                                                                          {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].code) + 1 - (yyvsp[-8].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-8].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-5].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-5].code), (yyvsp[-1].code)); }
#line 2495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* elseifstmt: tELSIF expr jumpifz tTHEN stmtlist jump  */
#line 535 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2505 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* jumpifz: %empty  */
#line 541 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2513 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* jump: %empty  */
#line 545 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2521 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* varassign: %empty  */
#line 549 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2529 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* if: tIF  */
#line 553 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2536 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* lbl: %empty  */
#line 556 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2542 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* simpleexprnoparens: INT  */
#line 564 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2550 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* simpleexprnoparens: FLOAT  */
#line 567 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2558 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* simpleexprnoparens: SYMBOL  */
#line 570 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2567 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* simpleexprnoparens: STRING  */
#line 574 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* simpleexprnoparens: '+' simpleexpr  */
#line 578 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[0].code); }
#line 2582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* simpleexprnoparens: '-' simpleexpr  */
#line 579 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* simpleexprnoparens: tNOT simpleexpr  */
#line 580 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_not); }
#line 2594 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* simpleexprnoparens: THEENTITY  */
#line 582 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2607 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* simpleexprnoparens: THEENTITYWITHID simpleexpr  */
#line 590 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* simpleexprnoparens: THEFUNCINOF simpleexpr  */
#line 596 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2626 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* simpleexprnoparens: THEFUNC tOF simpleexpr  */
#line 599 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2634 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* simpleexprnoparens: THEOBJECTPROP  */
#line 602 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_objectproppush);
		g_lingo->codeString((yyvsp[0].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectprop).prop->c_str());
		delete (yyvsp[0].objectprop).obj;
		delete (yyvsp[0].objectprop).prop; }
#line 2645 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* simpleexprnoparens: tSPRITE expr tINTERSECTS simpleexpr  */
#line 608 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_intersects); }
#line 2651 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* simpleexprnoparens: tSPRITE expr tWITHIN simpleexpr  */
#line 609 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 2657 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* simpleexprnoparens: ID '(' ID ')'  */
#line 611 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_lazyeval);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeFunc((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 2668 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* $@10: %empty  */
#line 617 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_lazyeval); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 2674 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* simpleexprnoparens: ID '(' ID ',' $@10 nonemptyarglist ')'  */
#line 618 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeFunc((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 2683 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* simpleexprnoparens: ID '(' arglist ')'  */
#line 622 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* simpleexprnoparens: ID  */
#line 625 "engines/director/lingo/lingo-gr.y"
                                                        {
		if (g_lingo->_builtinConsts.contains(*(yyvsp[0].s))) {
			(yyval.code) = g_lingo->code1(LC::c_constpush);
		} else {
			(yyval.code) = g_lingo->code1(LC::c_eval);
		}
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2704 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* simpleexpr: '(' expr ')'  */
#line 635 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2710 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* expr: simpleexpr  */
#line 637 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2716 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* expr: expr '+' expr  */
#line 638 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2722 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* expr: expr '-' expr  */
#line 639 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* expr: expr '*' expr  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2734 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* expr: expr '/' expr  */
#line 641 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2740 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* expr: expr tMOD expr  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2746 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* expr: expr '>' expr  */
#line 643 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2752 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* expr: expr '<' expr  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2758 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* expr: expr tEQ expr  */
#line 645 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2764 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* expr: expr tNEQ expr  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2770 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* expr: expr tGE expr  */
#line 647 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2776 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* expr: expr tLE expr  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2782 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102: /* expr: expr tAND expr  */
#line 649 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2788 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* expr: expr tOR expr  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2794 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* expr: expr '&' expr  */
#line 651 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2800 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105: /* expr: expr tCONCAT expr  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2806 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* expr: expr tCONTAINS expr  */
#line 653 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2812 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107: /* expr: expr tSTARTS expr  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* chunkexpr: tFIELD simpleexprnoparens  */
#line 656 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String field("field");
		g_lingo->codeFunc(&field, 1); }
#line 2826 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* chunkexpr: tCHAR expr tOF simpleexpr  */
#line 659 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_charOf); }
#line 2832 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* chunkexpr: tCHAR expr tTO expr tOF simpleexpr  */
#line 660 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charToOf); }
#line 2838 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* chunkexpr: tITEM expr tOF simpleexpr  */
#line 661 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_itemOf); }
#line 2844 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* chunkexpr: tITEM expr tTO expr tOF simpleexpr  */
#line 662 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemToOf); }
#line 2850 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* chunkexpr: tLINE expr tOF simpleexpr  */
#line 663 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_lineOf); }
#line 2856 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* chunkexpr: tLINE expr tTO expr tOF simpleexpr  */
#line 664 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineToOf); }
#line 2862 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* chunkexpr: tWORD expr tOF simpleexpr  */
#line 665 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_wordOf); }
#line 2868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* chunkexpr: tWORD expr tTO expr tOF simpleexpr  */
#line 666 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordToOf); }
#line 2874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* reference: tCAST simpleexprnoparens  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String cast("cast");
		g_lingo->codeFunc(&cast, 1); }
#line 2882 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* reference: tSCRIPT simpleexprnoparens  */
#line 672 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String script("script");
		g_lingo->codeFunc(&script, 1); }
#line 2890 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* reference: tWINDOW simpleexprnoparens  */
#line 675 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String window("window");
		g_lingo->codeFunc(&window, 1); }
#line 2898 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tPUT expr  */
#line 679 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_printtop); }
#line 2904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tEXIT tREPEAT  */
#line 682 "engines/director/lingo/lingo-gr.y"
                                                {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->exits.push_back(pos);
		} else {
			warning("# LINGO: exit repeat not inside repeat block");
		} }
#line 2917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proc: tEXIT  */
#line 690 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 2923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* $@11: %empty  */
#line 691 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 2929 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* proc: tGLOBAL $@11 globallist  */
#line 691 "engines/director/lingo/lingo-gr.y"
                                                                                 { inLast(); }
#line 2935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* $@12: %empty  */
#line 692 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 2941 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* proc: tPROPERTY $@12 propertylist  */
#line 692 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 2947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* $@13: %empty  */
#line 693 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 2953 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* proc: tINSTANCE $@13 instancelist  */
#line 693 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 2959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* proc: tOPEN expr tWITH expr  */
#line 694 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String open("open");
		g_lingo->codeCmd(&open, 2); }
#line 2967 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* proc: tOPEN expr  */
#line 697 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String open("open");
		g_lingo->codeCmd(&open, 1); }
#line 2975 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* proc: ID '(' ID ')'  */
#line 700 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_lazyeval);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeCmd((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 2986 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* $@14: %empty  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_lazyeval); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 2992 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* proc: ID '(' ID ',' $@14 nonemptyarglist ')'  */
#line 707 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeCmd((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 3001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* proc: ID '(' arglist ')'  */
#line 711 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeCmd((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3009 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* proc: ID arglist  */
#line 714 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeCmd((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3017 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* globallist: ID  */
#line 718 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* globallist: globallist ',' ID  */
#line 721 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3033 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* propertylist: ID  */
#line 725 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3041 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* propertylist: propertylist ',' ID  */
#line 728 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* instancelist: ID  */
#line 732 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3057 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* instancelist: instancelist ',' ID  */
#line 735 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3065 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* gotofunc: tGOLOOP  */
#line 746 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3071 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* gotofunc: tGO tNEXT  */
#line 747 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3077 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* gotofunc: tGO tPREVIOUS  */
#line 748 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3083 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* gotofunc: tGO expr  */
#line 749 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3092 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* gotofunc: tGO expr gotomovie  */
#line 753 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3101 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* gotofunc: tGO gotomovie  */
#line 757 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3110 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* playfunc: tPLAY expr  */
#line 765 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3119 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* playfunc: tPLAY expr gotomovie  */
#line 769 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3128 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* playfunc: tPLAY gotomovie  */
#line 773 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3137 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* $@15: %empty  */
#line 777 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3143 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* playfunc: tPLAYACCEL $@15 arglist  */
#line 777 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeCmd((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3152 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* $@16: %empty  */
#line 807 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3158 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* defn: tMACRO $@16 ID lbl argdef '\n' argstore stmtlist  */
#line 808 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3168 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* defn: tFACTORY ID  */
#line 813 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3174 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* $@17: %empty  */
#line 814 "engines/director/lingo/lingo-gr.y"
                  { startDef(); (*g_lingo->_methodVars)["me"] = kVarArgument; }
#line 3180 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* defn: tMETHOD $@17 lbl argdef '\n' argstore stmtlist  */
#line 815 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1);
		endDef();
		delete (yyvsp[-6].s); }
#line 3190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* defn: on lbl argdef '\n' argstore stmtlist ENDCLAUSE endargdef  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                                                   {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3203 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* defn: on lbl argdef '\n' argstore stmtlist  */
#line 828 "engines/director/lingo/lingo-gr.y"
                                               {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* $@18: %empty  */
#line 834 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3219 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* on: tON $@18 ID  */
#line 834 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = (yyvsp[0].s); }
#line 3225 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* argdef: %empty  */
#line 836 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3231 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* argdef: ID  */
#line 837 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3237 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* argdef: argdef ',' ID  */
#line 838 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3243 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* endargdef: ID  */
#line 841 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3249 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* endargdef: endargdef ',' ID  */
#line 842 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3255 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* argstore: %empty  */
#line 844 "engines/director/lingo/lingo-gr.y"
                                        { inDef(); }
#line 3261 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* arglist: %empty  */
#line 846 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3267 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* arglist: expr  */
#line 847 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3273 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* arglist: arglist ',' expr  */
#line 848 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3279 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* nonemptyarglist: expr  */
#line 850 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3285 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* nonemptyarglist: nonemptyarglist ',' expr  */
#line 851 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3291 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* list: '[' valuelist ']'  */
#line 853 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3297 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* valuelist: %empty  */
#line 855 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3303 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* valuelist: ':'  */
#line 856 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3309 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* valuelist: proplist  */
#line 857 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3315 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* valuelist: linearlist  */
#line 858 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3321 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* linearlist: expr  */
#line 860 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3327 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* linearlist: linearlist ',' expr  */
#line 861 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* proplist: proppair  */
#line 863 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* proplist: proplist ',' proppair  */
#line 864 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* proppair: SYMBOL ':' expr  */
#line 866 "engines/director/lingo/lingo-gr.y"
                          {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3354 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189: /* proppair: STRING ':' expr  */
#line 870 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3363 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* proppair: ID ':' expr  */
#line 874 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3372 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3376 "engines/director/lingo/lingo-gr.cpp"

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
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
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

#line 880 "engines/director/lingo/lingo-gr.y"


int yyreport_syntax_error(const yypcontext_t *ctx) {
	int res = 0;

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
