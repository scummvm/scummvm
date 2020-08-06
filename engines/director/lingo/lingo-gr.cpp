/* A Bison parser, made by GNU Bison 3.7.1.  */

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
#define YYBISON_VERSION "3.7.1"

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
	warning("######################  LINGO: %s at line %d col %d in %s id: %d",
		s, g_lingo->_linenumber, g_lingo->_colnumber, scriptType2str(g_lingo->_assemblyContext->_scriptType),
		g_lingo->_assemblyContext->_id);
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


#line 197 "engines/director/lingo/lingo-gr.cpp"

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
  YYSYMBOL_argname = 154,                  /* argname  */
  YYSYMBOL_argdef = 155,                   /* argdef  */
  YYSYMBOL_endargdef = 156,                /* endargdef  */
  YYSYMBOL_argstore = 157,                 /* argstore  */
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
#define YYFINAL  100
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1886

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  103
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  62
/* YYNRULES -- Number of rules.  */
#define YYNRULES  195
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  391

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
       0,   234,   234,   235,   237,   238,   239,   241,   242,   243,
     244,   245,   247,   254,   258,   269,   276,   279,   286,   289,
     296,   303,   310,   317,   323,   330,   341,   352,   359,   367,
     368,   370,   371,   376,   389,   393,   396,   388,   423,   427,
     430,   422,   457,   464,   470,   456,   500,   508,   511,   512,
     513,   515,   517,   519,   526,   534,   535,   537,   543,   547,
     551,   555,   558,   560,   561,   563,   564,   566,   569,   572,
     576,   580,   581,   582,   583,   584,   592,   598,   601,   604,
     610,   611,   612,   613,   619,   619,   624,   627,   636,   637,
     639,   640,   641,   642,   643,   644,   645,   646,   647,   648,
     649,   650,   651,   652,   653,   654,   655,   656,   658,   661,
     664,   665,   666,   667,   668,   669,   670,   671,   673,   674,
     677,   681,   684,   685,   686,   694,   695,   695,   696,   696,
     697,   697,   698,   701,   704,   710,   710,   715,   718,   722,
     723,   726,   730,   731,   734,   738,   739,   742,   753,   754,
     755,   756,   760,   764,   769,   770,   772,   776,   780,   784,
     784,   814,   814,   820,   821,   821,   827,   835,   841,   841,
     843,   845,   846,   847,   849,   850,   851,   853,   855,   856,
     857,   859,   860,   861,   863,   865,   866,   867,   868,   870,
     871,   873,   874,   876,   880,   884
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
  "$@15", "defn", "$@16", "$@17", "on", "$@18", "argname", "argdef",
  "endargdef", "argstore", "arglist", "nonemptyarglist", "list",
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

#define YYPACT_NINF (-357)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-66)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     362,   -82,  -357,  -357,  -357,  -357,  -357,  -357,  -357,   -34,
    -357,   892,  -357,  -357,  -357,   -30,  1012,   -14,   188,    55,
      55,  1038,   931,  -357,  1038,  -357,  -357,     8,  -357,  1119,
    -357,  -357,  -357,  -357,  1038,  -357,  -357,  -357,  -357,  -357,
    -357,  1038,  -357,  -357,    55,  -357,  -357,  1038,  -357,   -12,
    1038,  -357,  -357,  -357,  1145,  1145,  1145,  1145,  1038,  -357,
      17,  -357,  1038,  1038,  1038,  1038,  1038,  1038,  1038,  1038,
    1038,   148,   -63,  -357,  -357,  1230,  -357,  -357,  -357,  -357,
      55,  -357,    55,  1197,    55,  -357,     5,  1038,  1038,     6,
      27,     1,  -357,  1607,  1230,  -357,    55,  1527,    55,    55,
    -357,   362,  1038,  1658,  -357,  1779,    55,  -357,    55,    -9,
    -357,  -357,  1038,  -357,  -357,  -357,  -357,  -357,  1779,  1038,
    -357,  1263,  1296,  1329,  1362,  1746,  -357,  -357,  1709,    11,
      12,  -357,   -87,  1779,    19,    23,    24,  -357,  1038,  1038,
    1038,  1038,  1038,  1038,  1038,  1038,  1038,  1038,  1038,  1038,
    1038,  1038,  1038,  1038,  1038,  1038,  -357,  -357,  1038,    -2,
      -2,    -2,   -31,  1038,  1038,  1038,  1568,    78,  1038,  1038,
    1038,  1038,  1038,  1038,  -357,    29,  -357,  -357,  -357,    35,
    -357,  -357,  -357,   -32,  1620,     9,  1038,    83,  -357,    41,
      46,    48,    55,  -357,  1779,  1038,  1038,  1038,  1038,  1038,
    1038,  1038,  1038,  1038,  1038,  -357,  1038,  1038,  1038,  -357,
    1038,   204,    59,    47,   -45,   -45,   -45,   -45,  1792,  1792,
    -357,   -53,   -45,   -45,   -45,   -45,   -53,   -43,   -43,  -357,
    -357,    55,   103,  -357,  -357,  -357,  -357,  -357,  -357,  1038,
    1038,  1779,  1779,  1779,  1038,  1038,    55,  1779,  1779,  1779,
    1779,  1779,  1779,    55,   493,   716,    55,  -357,  -357,  -357,
    -357,   650,    55,  -357,  -357,  -357,  -357,  1395,  -357,  1428,
    -357,  1461,  -357,  1494,  -357,  -357,  1779,  1779,  1779,  1779,
      11,    12,    51,  -357,  -357,  -357,  -357,    53,  1038,  1779,
    1164,  -357,  1779,    25,  1038,  -357,  -357,  -357,  -357,    67,
    -357,  1038,  -357,  -357,   563,   616,  1038,  1038,  1038,  1038,
    1038,  -357,  1779,  -357,  -357,  -357,   750,    31,  -357,    80,
     455,  1671,    70,  -357,   156,  -357,  -357,  -357,  -357,  -357,
      86,   616,  -357,  -357,  -357,  -357,  1038,  1038,  -357,  -357,
    1038,  -357,    56,    55,  -357,  -357,  -357,   131,   151,   101,
    1779,  1779,  -357,   816,  1038,  -357,   104,  -357,    91,  -357,
    1038,   137,  -357,  -357,  1779,  -357,    55,  -357,  1779,  1038,
     113,   146,  -357,   750,  -357,  1779,  -357,   650,   116,  -357,
    -357,  -357,  -357,   750,  -357,  -357,   117,   750,  -357,   118,
    -357
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     7,   159,   164,     8,     9,    10,    11,   125,
     126,     0,   148,    61,   161,     0,     0,     0,     0,     0,
       0,     0,     0,   130,     0,   128,   168,     0,     2,   178,
      29,    31,     6,    32,     0,    30,   122,   123,     5,    62,
      50,   178,    62,   124,   139,    67,    75,     0,    68,     0,
       0,    70,    69,    79,     8,     9,    10,    11,     0,   149,
       0,   150,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   185,    87,    88,    90,   151,   118,    74,   153,    82,
       0,    46,     0,   121,     0,    62,     0,     0,     0,     0,
       0,     0,   163,   133,   156,   158,   145,     0,   142,     0,
       1,     0,   178,   179,   138,    58,   171,   160,   171,   140,
     127,    76,     0,    77,   109,   108,   119,   120,   155,     0,
      73,     0,     0,     0,     0,     0,    71,    72,     0,    70,
      69,   186,    87,   189,     0,   188,   187,   191,   178,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   152,    62,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   157,   146,   131,    52,    52,   143,
     129,   169,     3,    87,     0,     0,   178,     0,   170,   172,
       0,     0,   139,    78,   154,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    89,     0,     0,     0,   184,
       0,     0,    87,     0,   100,   101,    98,    99,   102,   103,
      95,   105,   106,   107,    97,    96,   104,    91,    92,    93,
      94,   171,     0,    12,    13,    15,    16,    17,    18,     0,
       0,    58,    22,    20,     0,     0,     0,    27,    28,    21,
      19,    47,   132,   145,     0,     0,   142,   134,   135,   137,
     180,     0,   171,   177,   177,   141,   110,     0,   112,     0,
     114,     0,   116,     0,    80,    81,   194,   193,   195,   190,
       0,     0,     0,   192,    83,    84,    86,     0,     0,    42,
      34,    51,    23,    98,     0,   147,    62,    66,    62,    63,
     144,     0,    59,   173,     0,     0,     0,     0,     0,     0,
       0,   177,    14,    62,    60,    60,     0,     0,    49,     0,
       0,   181,     0,    55,   167,   165,   111,   113,   115,   117,
       0,     0,    43,    35,    39,    59,     0,     0,    48,    64,
     182,   136,    62,   174,    85,   162,    58,     0,     0,     0,
      25,    26,   183,     0,     0,    56,     0,   175,   166,    44,
       0,     0,    33,    62,    58,    53,     0,    51,    36,     0,
       0,     0,   176,     0,    58,    40,    54,     0,     0,    51,
      58,    59,    45,     0,    51,    57,     0,     0,    37,     0,
      41
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -357,  -357,   107,     0,  -357,   -44,     2,  -357,  -357,  -357,
    -357,  -357,  -357,  -357,  -357,  -357,  -356,    36,  -357,  -357,
    -357,  -237,  -326,  -106,  -357,    -7,  -248,  -357,    76,  -357,
     277,   130,    16,  -357,  -357,  -357,  -357,  -357,  -357,    21,
     -41,   -20,  -357,   -17,  -357,  -357,  -357,  -357,  -357,  -357,
    -357,  -357,  -107,  -357,  -258,   -38,  -303,  -357,  -357,  -357,
    -357,    30
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    27,    28,    72,    30,    31,   297,   314,   347,   374,
     315,   348,   380,   313,   346,   367,   316,   254,    33,   342,
     355,   187,   323,   333,    34,   106,   298,   299,    73,   310,
      74,   103,    76,    77,    35,    44,    98,    96,   301,   110,
     180,   176,    36,    78,    37,    41,    38,    80,    42,    39,
      99,   189,   190,   358,   304,   104,   322,    79,   134,   135,
     136,   137
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      29,   191,    32,   107,   291,    95,   305,   330,   100,   349,
     138,   373,   239,   302,    40,   208,   145,    43,    90,    91,
      92,    81,     2,   383,   145,   -24,   145,   146,   387,    54,
      55,     7,     8,   240,   138,   108,   112,   352,   152,   153,
     154,   155,    84,    85,   109,   151,   152,   153,   154,   155,
     154,   155,   -24,   331,   172,   385,   324,   325,   156,   164,
     168,   -24,   -24,   119,   185,   138,   257,   258,   335,   165,
     169,   132,   339,    63,    64,    65,    66,   174,   163,     2,
     157,   170,   158,   345,   162,   336,     5,     6,     7,     8,
     192,   171,   353,   354,   145,   337,   175,   146,   179,   181,
     213,    29,   183,    32,   101,   363,   188,   259,   188,   359,
     -24,   -24,   -24,   206,   207,   151,   152,   153,   154,   155,
     209,   -24,   210,   211,   287,   378,   246,   371,   253,   381,
     114,   115,   116,   117,   256,   386,   261,   379,   212,   389,
     262,    75,   263,   384,   264,   286,    83,   288,   260,   311,
     231,    93,    94,   208,    97,   303,   138,   284,   285,   233,
     235,   237,    45,   320,   105,    46,    47,   338,   341,    48,
      49,    50,     2,   129,   130,   234,   236,   238,    53,    54,
      55,    56,    57,   343,   344,   360,   361,   362,   118,   365,
     366,   369,   109,   121,   122,   123,   124,   125,   376,   377,
     128,   133,   382,   388,   390,    86,    87,    88,   182,   334,
     296,   282,     2,   265,   255,   300,    62,   166,    89,     5,
       6,     7,     8,    63,    64,    65,    66,    67,     2,   280,
     281,   188,   184,   295,     0,     5,     6,     7,     8,    68,
      69,   283,     0,     0,     0,    70,   294,     0,    71,   194,
     131,     0,     0,   175,    29,    29,   179,     0,     0,     0,
       0,    29,   188,     0,     0,     0,     0,     0,     0,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,     0,     0,     0,   318,
       0,   319,     0,   241,   242,   243,     0,     0,   247,   248,
     249,   250,   251,   252,    29,    29,   332,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    29,     0,     0,     0,
      29,     0,     0,     0,   111,     0,   267,   113,   269,     0,
     271,    29,   273,     0,     0,   356,   276,   277,   278,   120,
     279,     0,     0,   357,     0,   126,   127,     0,     0,     0,
       0,     0,     0,    29,     0,     0,   370,     0,     0,     0,
       0,     0,    -4,     1,     0,   167,   372,     0,     0,   289,
     290,     0,     0,    29,   292,   293,     0,    29,     0,     0,
       0,     0,     0,    29,     0,     0,     2,    29,     0,   193,
       3,     4,     0,     5,     6,     7,     8,     0,     0,     0,
       9,    10,    11,    12,    13,     0,     0,    14,     0,    15,
       0,     0,    16,    17,    18,     0,     0,    19,   312,     0,
      20,    21,    22,    23,     0,     0,     0,     0,     0,     0,
       0,   321,     0,     0,     0,   232,     0,     0,     0,     0,
     321,     0,     0,     0,    24,    25,    26,     0,     0,     0,
       0,     0,     0,     0,     0,   -65,     1,     0,    -4,     0,
       0,     0,     0,     0,     0,     0,   350,   351,     0,     0,
     321,     0,   266,     0,   268,     0,   270,     0,   272,     2,
     274,   275,   -65,     3,   364,     0,     5,     6,     7,     8,
     368,   -65,   -65,     9,    10,    11,    12,    13,     0,   375,
       0,     0,    15,     0,     0,    16,    17,    18,     0,     0,
      19,     0,     0,     0,    21,    22,    23,     2,     0,     0,
       0,     3,     0,     0,     5,     6,     7,     8,     0,     0,
       0,     9,    10,    11,    12,     0,     0,    24,    25,     0,
     -65,   -65,   -65,    16,     0,    18,     0,     0,     0,     0,
       0,   -65,    21,    22,    23,     0,     0,     0,     0,     0,
       0,     0,     0,   -65,     1,     0,     0,     0,     0,     0,
       0,   317,     0,     0,     0,     0,    25,     0,     0,     0,
       0,     0,     0,   326,   327,   328,   329,     2,     0,     0,
     -65,     3,     0,     0,     5,     6,     7,     8,     0,     0,
       0,     9,    10,    11,    12,    13,     0,     0,     0,     0,
      15,     0,     0,    16,    17,    18,   -65,     1,    19,     0,
       0,     0,    21,    22,    23,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       2,     0,     0,     0,     3,    24,    25,     5,     6,     7,
       8,     1,     0,     0,     9,    10,    11,    12,    13,   -65,
       0,     0,     0,    15,     0,     0,    16,    17,    18,     0,
       0,    19,     0,     0,     2,    21,    22,    23,     3,     0,
       0,     5,     6,     7,     8,     0,   -65,   -65,     9,    10,
      11,    12,    13,     0,     0,     0,     0,    15,    24,    25,
      16,    17,    18,     0,     0,    19,     0,     0,     0,    21,
      22,    23,   -65,     0,     0,     0,     0,     1,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,     0,   -65,     0,     0,     0,     0,
       2,     0,     0,     0,     3,     0,   -65,     5,     6,     7,
       8,     1,     0,     0,     9,    10,    11,    12,    13,     0,
       0,     0,     0,    15,     0,     0,    16,    17,    18,     0,
       0,    19,     0,     0,     2,    21,    22,    23,     3,     0,
       0,     5,     6,     7,     8,     0,     0,     0,     9,    10,
      11,    12,    13,     0,     0,     0,     0,    15,    24,    25,
      16,    17,    18,   -65,     0,    19,     0,     0,     0,    21,
      22,    23,   -65,     0,     0,     0,     0,     1,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,     0,     0,   -65,     0,     0,     0,
       2,     0,     0,     0,     3,     0,   -65,     5,     6,     7,
       8,     0,     0,     0,     9,    10,    11,    12,    13,     0,
       0,     0,     0,    15,     0,     0,    16,    17,    18,     0,
       0,    19,     0,     0,     0,    21,    22,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
       0,   -65,     0,     0,     0,     0,    45,     0,     0,    46,
      47,     0,   -65,    48,    49,    50,     2,    51,    52,     0,
       0,     0,    53,    54,    55,    56,    57,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,    61,     0,     0,     0,    45,     0,     0,    46,    47,
       0,     0,    48,    49,    50,     2,    51,    52,     0,     0,
      62,    53,    54,    55,    56,    57,     0,    63,    64,    65,
      66,    67,     0,     0,     0,     0,     0,    58,     0,    60,
       0,     0,     0,    68,    69,     0,     0,     0,     0,    70,
       0,     0,    71,     0,     0,     0,     0,     0,     0,    62,
       0,     0,     0,     0,     0,     0,    63,    64,    65,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    68,    69,     0,     0,    45,     0,    70,    46,
      47,    71,    82,    48,    49,    50,     2,    51,    52,     0,
       0,     0,    53,    54,    55,    56,    57,     0,     0,     0,
       0,     0,    45,     0,     0,    46,    47,     0,     0,    48,
      49,    50,     2,    51,    52,     0,     0,     0,    53,    54,
      55,    56,    57,     0,     0,     0,     0,     0,     0,     0,
      62,     0,     0,     0,     0,     0,     0,    63,    64,    65,
      66,    67,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    68,    69,     0,    62,     0,     0,    70,
       0,     0,    71,    63,    64,    65,    66,    67,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    68,
      69,     0,     0,    45,     0,    70,    46,    47,    71,     0,
      48,    49,    50,     2,    51,    52,     0,     0,     0,    53,
      54,    55,    56,    57,     0,     0,     0,     0,     0,    45,
       0,     0,    46,    47,     0,     0,    48,    49,    50,     2,
      51,    52,     0,     0,     0,    53,    54,    55,    56,    57,
       0,     0,     0,     0,     0,     0,     0,    62,     0,     0,
       0,     0,     0,     0,    63,    64,    65,    66,    67,   -38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      68,    69,     0,    62,     0,     0,   102,     0,     0,    71,
      63,    64,    65,    66,    67,     0,   139,   140,   141,   142,
     143,   144,     0,   145,     0,     0,   146,   147,   148,     0,
       0,   159,     0,     0,     0,    71,     0,     0,     0,     0,
       0,     0,   149,   150,   151,   152,   153,   154,   155,   139,
     140,   141,   142,   143,   144,     0,   145,   160,   161,   146,
     147,   148,     0,     0,     0,     0,    58,     0,    60,     0,
       0,     0,     0,     0,     0,   149,   150,   151,   152,   153,
     154,   155,   139,   140,   141,   142,   143,   144,     0,   145,
       0,     0,   146,   147,   148,     0,     0,     0,     0,     0,
       0,   195,     0,     0,     0,     0,     0,   196,   149,   150,
     151,   152,   153,   154,   155,   139,   140,   141,   142,   143,
     144,     0,   145,     0,     0,   146,   147,   148,     0,     0,
       0,     0,     0,     0,   197,     0,     0,     0,     0,     0,
     198,   149,   150,   151,   152,   153,   154,   155,   139,   140,
     141,   142,   143,   144,     0,   145,     0,     0,   146,   147,
     148,     0,     0,     0,     0,     0,     0,   199,     0,     0,
       0,     0,     0,   200,   149,   150,   151,   152,   153,   154,
     155,   139,   140,   141,   142,   143,   144,     0,   145,     0,
       0,   146,   147,   148,     0,     0,     0,     0,     0,     0,
     201,     0,     0,     0,     0,     0,   202,   149,   150,   151,
     152,   153,   154,   155,   139,   140,   141,   142,   143,   144,
       0,   145,     0,     0,   146,   147,   148,     0,     0,     0,
       0,     0,     0,   306,     0,     0,     0,     0,     0,     0,
     149,   150,   151,   152,   153,   154,   155,   139,   140,   141,
     142,   143,   144,     0,   145,     0,     0,   146,   147,   148,
       0,     0,     0,     0,     0,     0,   307,     0,     0,     0,
       0,     0,     0,   149,   150,   151,   152,   153,   154,   155,
     139,   140,   141,   142,   143,   144,     0,   145,     0,     0,
     146,   147,   148,     0,     0,     0,     0,     0,     0,   308,
       0,     0,     0,     0,     0,     0,   149,   150,   151,   152,
     153,   154,   155,   139,   140,   141,   142,   143,   144,     0,
     145,     0,     0,   146,   147,   148,     0,     0,     0,     0,
       0,     0,   309,     0,     0,     0,     0,     0,     0,   149,
     150,   151,   152,   153,   154,   155,   139,   140,   141,   142,
     143,   144,     0,   145,     0,     0,   146,   147,   148,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   177,   149,   150,   151,   152,   153,   154,   155,   139,
     140,   141,   142,   143,   144,     0,   145,     0,     0,   146,
     147,   148,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   149,   150,   151,   152,   153,
     154,   155,   244,   178,     0,     0,     0,     0,     0,     0,
     139,   140,   245,   142,   143,   144,     0,   145,     0,     0,
     146,   147,   148,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   149,   150,   151,   152,
     153,   154,   155,   173,     0,     0,     0,     0,     0,   139,
     140,   141,   142,   143,   144,     0,   145,     0,     0,   146,
     147,   148,   139,   140,   141,   142,   143,   144,     0,   145,
       0,     0,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,     0,     0,     0,     0,     0,     0,   149,   150,
     151,   152,   153,   154,   155,     0,     0,     0,   205,   186,
     139,   140,   141,   142,   143,   144,     0,   145,     0,     0,
     146,   147,   148,   139,   140,   141,   142,   143,   144,     0,
     145,     0,     0,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,     0,     0,     0,     0,   186,     0,   149,
     150,   151,   152,   153,   154,   155,     0,     0,     0,     0,
     340,   139,   140,   141,   142,   143,   144,     0,   145,     0,
       0,   146,   147,   148,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   149,   150,   151,
     152,   153,   154,   155,     0,     0,     0,   205,   139,   140,
     141,   142,   143,   144,     0,   145,     0,     0,   146,   147,
     148,     0,     0,     0,     0,     0,   203,   204,     0,     0,
       0,     0,     0,     0,   149,   150,   151,   152,   153,   154,
     155,   139,   140,   141,   142,   143,   144,     0,   145,     0,
       0,   146,   147,   148,   139,   140,   141,   142,     0,     0,
       0,   145,     0,     0,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,     0,     0,     0,     0,     0,     0,
     149,   150,   151,   152,   153,   154,   155
};

static const yytype_int16 yycheck[] =
{
       0,   108,     0,    41,   241,    22,   264,   310,     0,   335,
      97,   367,    43,   261,    96,   102,    69,    51,    18,    19,
      20,    51,    24,   379,    69,     0,    69,    72,   384,    31,
      32,    33,    34,    64,    97,    42,    48,   340,    91,    92,
      93,    94,    56,    57,    44,    90,    91,    92,    93,    94,
      93,    94,    27,   311,    53,   381,   304,   305,    75,    54,
      54,    36,    37,    46,   102,    97,    98,    99,   316,    64,
      64,    71,   320,    75,    76,    77,    78,    94,    85,    24,
      80,    54,    82,   331,    84,    54,    31,    32,    33,    34,
      99,    64,    36,    37,    69,    64,    96,    72,    98,    99,
     138,   101,   102,   101,    96,   353,   106,    98,   108,   346,
      85,    86,    87,   102,   102,    90,    91,    92,    93,    94,
     101,    96,    99,    99,   231,   373,    48,   364,    99,   377,
      54,    55,    56,    57,    99,   383,    53,   374,   138,   387,
      99,    11,    96,   380,    96,    98,    16,    44,   186,    96,
     157,    21,    22,   102,    24,   262,    97,    98,    99,   159,
     160,   161,    14,    96,    34,    17,    18,    87,    98,    21,
      22,    23,    24,    25,    26,   159,   160,   161,    30,    31,
      32,    33,    34,    27,    98,    54,    35,    86,    58,    85,
      99,    54,   192,    63,    64,    65,    66,    67,    85,    53,
      70,    71,    86,    86,    86,    17,    18,    19,   101,   315,
     254,   211,    24,   192,   178,   256,    68,    87,    30,    31,
      32,    33,    34,    75,    76,    77,    78,    79,    24,    25,
      26,   231,   102,   253,    -1,    31,    32,    33,    34,    91,
      92,   211,    -1,    -1,    -1,    97,   246,    -1,   100,   119,
     102,    -1,    -1,   253,   254,   255,   256,    -1,    -1,    -1,
      -1,   261,   262,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,    -1,    -1,    -1,   296,
      -1,   298,    -1,   163,   164,   165,    -1,    -1,   168,   169,
     170,   171,   172,   173,   304,   305,   313,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   316,    -1,    -1,    -1,
     320,    -1,    -1,    -1,    47,    -1,   196,    50,   198,    -1,
     200,   331,   202,    -1,    -1,   342,   206,   207,   208,    62,
     210,    -1,    -1,   343,    -1,    68,    69,    -1,    -1,    -1,
      -1,    -1,    -1,   353,    -1,    -1,   363,    -1,    -1,    -1,
      -1,    -1,     0,     1,    -1,    88,   366,    -1,    -1,   239,
     240,    -1,    -1,   373,   244,   245,    -1,   377,    -1,    -1,
      -1,    -1,    -1,   383,    -1,    -1,    24,   387,    -1,   112,
      28,    29,    -1,    31,    32,    33,    34,    -1,    -1,    -1,
      38,    39,    40,    41,    42,    -1,    -1,    45,    -1,    47,
      -1,    -1,    50,    51,    52,    -1,    -1,    55,   288,    -1,
      58,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   301,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     310,    -1,    -1,    -1,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     0,     1,    -1,    96,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   336,   337,    -1,    -1,
     340,    -1,   195,    -1,   197,    -1,   199,    -1,   201,    24,
     203,   204,    27,    28,   354,    -1,    31,    32,    33,    34,
     360,    36,    37,    38,    39,    40,    41,    42,    -1,   369,
      -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,
      55,    -1,    -1,    -1,    59,    60,    61,    24,    -1,    -1,
      -1,    28,    -1,    -1,    31,    32,    33,    34,    -1,    -1,
      -1,    38,    39,    40,    41,    -1,    -1,    82,    83,    -1,
      85,    86,    87,    50,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    96,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     0,     1,    -1,    -1,    -1,    -1,    -1,
      -1,   294,    -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    -1,   306,   307,   308,   309,    24,    -1,    -1,
      27,    28,    -1,    -1,    31,    32,    33,    34,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    50,    51,    52,     0,     1,    55,    -1,
      -1,    -1,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    28,    82,    83,    31,    32,    33,
      34,     1,    -1,    -1,    38,    39,    40,    41,    42,    96,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,
      -1,    55,    -1,    -1,    24,    59,    60,    61,    28,    -1,
      -1,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    47,    82,    83,
      50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,    59,
      60,    61,    96,    -1,    -1,    -1,    -1,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    82,    83,    -1,    85,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    28,    -1,    96,    31,    32,    33,
      34,     1,    -1,    -1,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,
      -1,    55,    -1,    -1,    24,    59,    60,    61,    28,    -1,
      -1,    31,    32,    33,    34,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    47,    82,    83,
      50,    51,    52,    87,    -1,    55,    -1,    -1,    -1,    59,
      60,    61,    96,    -1,    -1,    -1,    -1,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    82,    83,    -1,    -1,    86,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    28,    -1,    96,    31,    32,    33,
      34,    -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,
      -1,    55,    -1,    -1,    -1,    59,    60,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    82,    83,
      -1,    85,    -1,    -1,    -1,    -1,    14,    -1,    -1,    17,
      18,    -1,    96,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    30,    31,    32,    33,    34,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    -1,    -1,    -1,    14,    -1,    -1,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    -1,    -1,
      68,    30,    31,    32,    33,    34,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    46,    -1,    48,
      -1,    -1,    -1,    91,    92,    -1,    -1,    -1,    -1,    97,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    -1,    -1,    14,    -1,    97,    17,
      18,   100,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    30,    31,    32,    33,    34,    -1,    -1,    -1,
      -1,    -1,    14,    -1,    -1,    17,    18,    -1,    -1,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    30,    31,
      32,    33,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    -1,    68,    -1,    -1,    97,
      -1,    -1,   100,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      92,    -1,    -1,    14,    -1,    97,    17,    18,   100,    -1,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    30,
      31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,    14,
      -1,    -1,    17,    18,    -1,    -1,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    30,    31,    32,    33,    34,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    -1,    68,    -1,    -1,    97,    -1,    -1,   100,
      75,    76,    77,    78,    79,    -1,    62,    63,    64,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,
      -1,    44,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    62,
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
      93,    94,    54,    96,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    66,    67,    -1,    69,    -1,    -1,
      72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    56,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    62,    63,    64,    65,    66,    67,    -1,    69,
      -1,    -1,    72,    73,    74,    88,    89,    90,    91,    92,
      93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    -1,    -1,    -1,    98,    99,
      62,    63,    64,    65,    66,    67,    -1,    69,    -1,    -1,
      72,    73,    74,    62,    63,    64,    65,    66,    67,    -1,
      69,    -1,    -1,    72,    73,    74,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    99,    -1,    88,
      89,    90,    91,    92,    93,    94,    -1,    -1,    -1,    -1,
      99,    62,    63,    64,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
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
      97,   100,   106,   131,   133,   134,   135,   136,   146,   160,
     150,    51,    20,   134,    56,    57,    17,    18,    19,    30,
     106,   106,   106,   134,   134,   146,   140,   134,   139,   153,
       0,    96,    97,   134,   158,   134,   128,   158,   128,   106,
     142,   133,    48,   133,   131,   131,   131,   131,   134,    46,
     133,   134,   134,   134,   134,   134,   133,   133,   134,    25,
      26,   102,   106,   134,   161,   162,   163,   164,    97,    62,
      63,    64,    65,    66,    67,    69,    72,    73,    74,    88,
      89,    90,    91,    92,    93,    94,   146,   106,   106,    44,
      70,    71,   106,   128,    54,    64,   134,   133,    54,    64,
      54,    64,    53,    56,   146,   106,   144,    54,    96,   106,
     143,   106,   105,   106,   134,   158,    99,   124,   106,   154,
     155,   155,    99,   133,   134,    48,    54,    48,    54,    48,
      54,    48,    54,    80,    81,    98,   102,   102,   102,   101,
      99,    99,   106,   158,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   128,   133,   106,   135,   106,   135,   106,   135,    43,
      64,   134,   134,   134,    54,    64,    48,   134,   134,   134,
     134,   134,   134,    99,   120,   120,    99,    98,    99,    98,
     158,    53,    99,    96,    96,   142,   133,   134,   133,   134,
     133,   134,   133,   134,   133,   133,   134,   134,   134,   134,
      25,    26,   106,   164,    98,    99,    98,   155,    44,   134,
     134,   124,   134,   134,   106,   144,   108,   109,   129,   130,
     143,   141,   129,   155,   157,   157,    48,    48,    48,    48,
     132,    96,   134,   116,   110,   113,   119,   133,   128,   128,
      96,   134,   159,   125,   129,   129,   133,   133,   133,   133,
     159,   157,   128,   126,   126,   129,    54,    64,    87,   129,
      99,    98,   122,    27,    98,   129,   117,   111,   114,   125,
     134,   134,   159,    36,    37,   123,   128,   106,   156,   124,
      54,    35,    86,   129,   134,    85,    99,   118,   134,    54,
     128,   124,   106,   119,   112,   134,    85,    53,   129,   124,
     115,   129,    86,   119,   124,   125,   129,   119,    86,   129,
      86
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
     135,   135,   135,   135,   135,   135,   135,   135,   136,   136,
     136,   137,   137,   137,   137,   137,   138,   137,   139,   137,
     140,   137,   137,   137,   137,   141,   137,   137,   137,   142,
     142,   142,   143,   143,   143,   144,   144,   144,   145,   145,
     145,   145,   145,   145,   146,   146,   147,   147,   147,   148,
     147,   150,   149,   149,   151,   149,   149,   149,   153,   152,
     154,   155,   155,   155,   156,   156,   156,   157,   158,   158,
     158,   159,   159,   159,   160,   161,   161,   161,   161,   162,
     162,   163,   163,   164,   164,   164
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
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       4,     6,     4,     6,     4,     6,     4,     6,     1,     2,
       2,     2,     1,     1,     2,     1,     0,     3,     0,     3,
       0,     3,     4,     2,     4,     0,     7,     4,     2,     0,
       1,     3,     0,     1,     3,     0,     1,     3,     1,     2,
       2,     2,     3,     2,     3,     2,     2,     3,     2,     0,
       3,     0,     8,     2,     0,     7,     8,     6,     0,     3,
       1,     0,     1,     3,     0,     1,     3,     0,     0,     1,
       3,     1,     2,     3,     3,     0,     1,     1,     1,     1,
       3,     1,     3,     3,     3,     3
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
#line 230 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1734 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_THEFUNCINOF: /* THEFUNCINOF  */
#line 230 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1740 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_VARID: /* VARID  */
#line 230 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1746 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_STRING: /* STRING  */
#line 230 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1752 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_SYMBOL: /* SYMBOL  */
#line 230 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1758 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ENDCLAUSE: /* ENDCLAUSE  */
#line 230 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1764 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tPLAYACCEL: /* tPLAYACCEL  */
#line 230 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1770 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tMETHOD: /* tMETHOD  */
#line 230 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1776 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 230 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1782 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_on: /* on  */
#line 230 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1788 "engines/director/lingo/lingo-gr.cpp"
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
#line 242 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2056 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9: /* ID: tFIELD  */
#line 243 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2062 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* ID: tSCRIPT  */
#line 244 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2068 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* ID: tWINDOW  */
#line 245 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2074 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* asgn: tPUT expr tINTO ID  */
#line 247 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2086 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* asgn: tPUT expr tINTO chunkexpr  */
#line 254 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* asgn: tPUT THEMENUITEMSENTITY ID simpleexpr tINTO expr  */
#line 258 "engines/director/lingo/lingo-gr.y"
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
#line 2110 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* asgn: tPUT expr tAFTER ID  */
#line 269 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_putafter);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2122 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* asgn: tPUT expr tAFTER chunkexpr  */
#line 276 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_putafter);
		(yyval.code) = (yyvsp[-2].code); }
#line 2130 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* asgn: tPUT expr tBEFORE ID  */
#line 279 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_putbefore);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18: /* asgn: tPUT expr tBEFORE chunkexpr  */
#line 286 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_putbefore);
		(yyval.code) = (yyvsp[-2].code); }
#line 2150 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* asgn: tSET ID tEQ expr  */
#line 289 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2162 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* asgn: tSET THEENTITY tEQ expr  */
#line 296 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2174 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* asgn: tSET ID tTO expr  */
#line 303 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2186 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* asgn: tSET THEENTITY tTO expr  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2198 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* asgn: tSET THEENTITYWITHID expr tTO expr  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                                                {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2209 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* asgn: tSET THEENTITYWITHID expr tEQ expr  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                                                {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* asgn: tSET THEMENUITEMENTITY simpleexpr tOF ID simpleexpr tTO expr  */
#line 330 "engines/director/lingo/lingo-gr.y"
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
#line 2236 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* asgn: tSET THEMENUITEMENTITY simpleexpr tOF ID simpleexpr tEQ expr  */
#line 341 "engines/director/lingo/lingo-gr.y"
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
#line 2252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* asgn: tSET THEOBJECTPROP tTO expr  */
#line 352 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2264 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* asgn: tSET THEOBJECTPROP tEQ expr  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* stmt: tREPEAT tWHILE lbl expr jumpifz startrepeat stmtlist jump tENDREPEAT  */
#line 376 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-6].code) - (yyvsp[-1].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-4].code) + 2);
		(*g_lingo->_currentAssembly)[(yyvsp[-4].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentAssembly)[(yyvsp[-1].code)] = start;	/* looping back */
		endRepeat((yyvsp[-1].code) + 1, (yyvsp[-6].code));	}
#line 2288 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* $@1: %empty  */
#line 389 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2296 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* $@2: %empty  */
#line 393 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2303 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* $@3: %empty  */
#line 396 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_le); }
#line 2309 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* stmt: tREPEAT tWITH ID tEQ expr $@1 varassign $@2 tTO expr $@3 jumpifz startrepeat stmtlist tENDREPEAT  */
#line 396 "engines/director/lingo/lingo-gr.y"
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
#line 2334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* $@4: %empty  */
#line 423 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2342 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* $@5: %empty  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2349 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* $@6: %empty  */
#line 430 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_ge); }
#line 2355 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* stmt: tREPEAT tWITH ID tEQ expr $@4 varassign $@5 tDOWN tTO expr $@6 jumpifz startrepeat stmtlist tENDREPEAT  */
#line 431 "engines/director/lingo/lingo-gr.y"
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
#line 2380 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* $@7: %empty  */
#line 457 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  Common::String count("count");
				  g_lingo->codeFunc(&count, 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
#line 2391 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* $@8: %empty  */
#line 464 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
#line 2401 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* $@9: %empty  */
#line 470 "engines/director/lingo/lingo-gr.y"
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
#line 2416 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* stmt: tREPEAT tWITH ID tIN expr $@7 lbl $@8 jumpifz $@9 startrepeat stmtlist tENDREPEAT  */
#line 480 "engines/director/lingo/lingo-gr.y"
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
#line 2440 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* stmt: tNEXT tREPEAT  */
#line 500 "engines/director/lingo/lingo-gr.y"
                                        {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->nexts.push_back(pos);
		} else {
			warning("# LINGO: next repeat not inside repeat block");
		} }
#line 2453 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* stmt: tWHEN ID tTHEN expr  */
#line 508 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_whencode);
		g_lingo->codeString((yyvsp[-2].s)->c_str()); }
#line 2461 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* stmt: tTELL expr '\n' tellstart stmtlist lbl tENDTELL  */
#line 511 "engines/director/lingo/lingo-gr.y"
                                                          { g_lingo->code1(LC::c_telldone); }
#line 2467 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* stmt: tTELL expr tTO tellstart stmtoneliner lbl  */
#line 512 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->code1(LC::c_telldone); }
#line 2473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* stmt: error '\n'  */
#line 513 "engines/director/lingo/lingo-gr.y"
                                        { yyerrok; }
#line 2479 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* startrepeat: %empty  */
#line 515 "engines/director/lingo/lingo-gr.y"
                                { startRepeat(); }
#line 2485 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* tellstart: %empty  */
#line 517 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_tell); }
#line 2491 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* ifstmt: if expr jumpifz tTHEN stmtlist jump elseifstmtlist lbl tENDIF  */
#line 519 "engines/director/lingo/lingo-gr.y"
                                                                                         {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2503 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* ifstmt: if expr jumpifz tTHEN stmtlist jump elseifstmtlist tELSE stmtlist lbl tENDIF  */
#line 526 "engines/director/lingo/lingo-gr.y"
                                                                                                          {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].code) + 1 - (yyvsp[-8].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-8].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-5].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-5].code), (yyvsp[-1].code)); }
#line 2515 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* elseifstmt: tELSIF expr jumpifz tTHEN stmtlist jump  */
#line 537 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* jumpifz: %empty  */
#line 543 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2533 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* jump: %empty  */
#line 547 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2541 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* varassign: %empty  */
#line 551 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2549 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* if: tIF  */
#line 555 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2556 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* lbl: %empty  */
#line 558 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2562 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* simpleexprnoparens: INT  */
#line 566 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2570 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* simpleexprnoparens: FLOAT  */
#line 569 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2578 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* simpleexprnoparens: SYMBOL  */
#line 572 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2587 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* simpleexprnoparens: STRING  */
#line 576 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2596 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* simpleexprnoparens: '+' simpleexpr  */
#line 580 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[0].code); }
#line 2602 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* simpleexprnoparens: '-' simpleexpr  */
#line 581 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2608 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* simpleexprnoparens: tNOT simpleexpr  */
#line 582 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_not); }
#line 2614 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* simpleexprnoparens: THEENTITY  */
#line 584 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2627 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* simpleexprnoparens: THEENTITYWITHID simpleexpr  */
#line 592 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2638 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* simpleexprnoparens: THEFUNCINOF simpleexpr  */
#line 598 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2646 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* simpleexprnoparens: THEFUNC tOF simpleexpr  */
#line 601 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2654 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* simpleexprnoparens: THEOBJECTPROP  */
#line 604 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_objectproppush);
		g_lingo->codeString((yyvsp[0].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectprop).prop->c_str());
		delete (yyvsp[0].objectprop).obj;
		delete (yyvsp[0].objectprop).prop; }
#line 2665 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* simpleexprnoparens: tSPRITE expr tINTERSECTS simpleexpr  */
#line 610 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_intersects); }
#line 2671 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* simpleexprnoparens: tSPRITE expr tWITHIN simpleexpr  */
#line 611 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 2677 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* simpleexprnoparens: ID '(' ID ')'  */
#line 613 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_lazyeval);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeFunc((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 2688 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* $@10: %empty  */
#line 619 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_lazyeval); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 2694 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* simpleexprnoparens: ID '(' ID ',' $@10 nonemptyarglist ')'  */
#line 620 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeFunc((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 2703 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* simpleexprnoparens: ID '(' arglist ')'  */
#line 624 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* simpleexprnoparens: ID  */
#line 627 "engines/director/lingo/lingo-gr.y"
                                                        {
		if (g_lingo->_builtinConsts.contains(*(yyvsp[0].s))) {
			(yyval.code) = g_lingo->code1(LC::c_constpush);
		} else {
			(yyval.code) = g_lingo->code1(LC::c_eval);
		}
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2724 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* simpleexpr: '(' expr ')'  */
#line 637 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2730 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* expr: simpleexpr  */
#line 639 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2736 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* expr: expr '+' expr  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2742 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* expr: expr '-' expr  */
#line 641 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2748 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* expr: expr '*' expr  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2754 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* expr: expr '/' expr  */
#line 643 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2760 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* expr: expr tMOD expr  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2766 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* expr: expr '>' expr  */
#line 645 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2772 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* expr: expr '<' expr  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2778 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* expr: expr tEQ expr  */
#line 647 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2784 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* expr: expr tNEQ expr  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2790 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* expr: expr tGE expr  */
#line 649 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2796 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* expr: expr tLE expr  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2802 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102: /* expr: expr tAND expr  */
#line 651 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2808 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* expr: expr tOR expr  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2814 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* expr: expr '&' expr  */
#line 653 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2820 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105: /* expr: expr tCONCAT expr  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2826 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* expr: expr tCONTAINS expr  */
#line 655 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2832 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107: /* expr: expr tSTARTS expr  */
#line 656 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2838 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* chunkexpr: tFIELD simpleexprnoparens  */
#line 658 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String field("field");
		g_lingo->codeFunc(&field, 1); }
#line 2846 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* chunkexpr: tCAST simpleexprnoparens  */
#line 661 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String cast("cast");
		g_lingo->codeFunc(&cast, 1); }
#line 2854 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* chunkexpr: tCHAR expr tOF simpleexpr  */
#line 664 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_charOf); }
#line 2860 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* chunkexpr: tCHAR expr tTO expr tOF simpleexpr  */
#line 665 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charToOf); }
#line 2866 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* chunkexpr: tITEM expr tOF simpleexpr  */
#line 666 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_itemOf); }
#line 2872 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* chunkexpr: tITEM expr tTO expr tOF simpleexpr  */
#line 667 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemToOf); }
#line 2878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* chunkexpr: tLINE expr tOF simpleexpr  */
#line 668 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_lineOf); }
#line 2884 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* chunkexpr: tLINE expr tTO expr tOF simpleexpr  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineToOf); }
#line 2890 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* chunkexpr: tWORD expr tOF simpleexpr  */
#line 670 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_wordOf); }
#line 2896 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* chunkexpr: tWORD expr tTO expr tOF simpleexpr  */
#line 671 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordToOf); }
#line 2902 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* reference: tSCRIPT simpleexprnoparens  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String script("script");
		g_lingo->codeFunc(&script, 1); }
#line 2910 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* reference: tWINDOW simpleexprnoparens  */
#line 677 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String window("window");
		g_lingo->codeFunc(&window, 1); }
#line 2918 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tPUT expr  */
#line 681 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String put("put");
		g_lingo->codeCmd(&put, 1); }
#line 2926 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tEXIT tREPEAT  */
#line 686 "engines/director/lingo/lingo-gr.y"
                                                {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->exits.push_back(pos);
		} else {
			warning("# LINGO: exit repeat not inside repeat block");
		} }
#line 2939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proc: tEXIT  */
#line 694 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 2945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* $@11: %empty  */
#line 695 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 2951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* proc: tGLOBAL $@11 globallist  */
#line 695 "engines/director/lingo/lingo-gr.y"
                                                                                 { inLast(); }
#line 2957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* $@12: %empty  */
#line 696 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 2963 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* proc: tPROPERTY $@12 propertylist  */
#line 696 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 2969 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* $@13: %empty  */
#line 697 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 2975 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* proc: tINSTANCE $@13 instancelist  */
#line 697 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 2981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* proc: tOPEN expr tWITH expr  */
#line 698 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String open("open");
		g_lingo->codeCmd(&open, 2); }
#line 2989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* proc: tOPEN expr  */
#line 701 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String open("open");
		g_lingo->codeCmd(&open, 1); }
#line 2997 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* proc: ID '(' ID ')'  */
#line 704 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_lazyeval);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeCmd((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 3008 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* $@14: %empty  */
#line 710 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_lazyeval); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 3014 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* proc: ID '(' ID ',' $@14 nonemptyarglist ')'  */
#line 711 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeCmd((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 3023 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* proc: ID '(' arglist ')'  */
#line 715 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeCmd((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* proc: ID arglist  */
#line 718 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeCmd((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3039 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* globallist: ID  */
#line 723 "engines/director/lingo/lingo-gr.y"
                                                        {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3047 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* globallist: ID ',' globallist  */
#line 726 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[-2].s), kVarGlobal);
		delete (yyvsp[-2].s); }
#line 3055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* propertylist: ID  */
#line 731 "engines/director/lingo/lingo-gr.y"
                                                        {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3063 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* propertylist: ID ',' propertylist  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[-2].s), kVarProperty);
		delete (yyvsp[-2].s); }
#line 3071 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* instancelist: ID  */
#line 739 "engines/director/lingo/lingo-gr.y"
                                                        {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* instancelist: ID ',' instancelist  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[-2].s), kVarInstance);
		delete (yyvsp[-2].s); }
#line 3087 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* gotofunc: tGOLOOP  */
#line 753 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3093 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* gotofunc: tGO tNEXT  */
#line 754 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3099 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* gotofunc: tGO tPREVIOUS  */
#line 755 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3105 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* gotofunc: tGO expr  */
#line 756 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3114 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* gotofunc: tGO expr gotomovie  */
#line 760 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3123 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* gotofunc: tGO gotomovie  */
#line 764 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3132 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* playfunc: tPLAY expr  */
#line 772 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* playfunc: tPLAY expr gotomovie  */
#line 776 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3150 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* playfunc: tPLAY gotomovie  */
#line 780 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3159 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* $@15: %empty  */
#line 784 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3165 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* playfunc: tPLAYACCEL $@15 arglist  */
#line 784 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeCmd((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3174 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* $@16: %empty  */
#line 814 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3180 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* defn: tMACRO $@16 ID lbl argdef '\n' argstore stmtlist  */
#line 815 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* defn: tFACTORY ID  */
#line 820 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3196 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* $@17: %empty  */
#line 821 "engines/director/lingo/lingo-gr.y"
                  { startDef(); (*g_lingo->_methodVars)["me"] = kVarArgument; }
#line 3202 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* defn: tMETHOD $@17 lbl argdef '\n' argstore stmtlist  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1);
		endDef();
		delete (yyvsp[-6].s); }
#line 3212 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* defn: on lbl argdef '\n' argstore stmtlist ENDCLAUSE endargdef  */
#line 827 "engines/director/lingo/lingo-gr.y"
                                                                   {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3225 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* defn: on lbl argdef '\n' argstore stmtlist  */
#line 835 "engines/director/lingo/lingo-gr.y"
                                               {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3235 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* $@18: %empty  */
#line 841 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3241 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* on: tON $@18 ID  */
#line 841 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = (yyvsp[0].s); }
#line 3247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* argname: ID  */
#line 843 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); delete (yyvsp[0].s); }
#line 3253 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* argdef: %empty  */
#line 845 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* argdef: argname  */
#line 846 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* argdef: argname ',' argdef  */
#line 847 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[0].narg) + 1; }
#line 3271 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* endargdef: ID  */
#line 850 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* endargdef: endargdef ',' ID  */
#line 851 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3283 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* argstore: %empty  */
#line 853 "engines/director/lingo/lingo-gr.y"
                                        { inDef(); }
#line 3289 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* arglist: %empty  */
#line 855 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* arglist: expr  */
#line 856 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3301 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* arglist: expr ',' arglist  */
#line 857 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[0].narg) + 1; }
#line 3307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* nonemptyarglist: expr  */
#line 859 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* nonemptyarglist: expr ','  */
#line 860 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3319 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* nonemptyarglist: expr ',' nonemptyarglist  */
#line 861 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[0].narg) + 1; }
#line 3325 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* list: '[' valuelist ']'  */
#line 863 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3331 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* valuelist: %empty  */
#line 865 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3337 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* valuelist: ':'  */
#line 866 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* valuelist: proplist  */
#line 867 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3349 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* valuelist: linearlist  */
#line 868 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3355 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189: /* linearlist: expr  */
#line 870 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3361 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* linearlist: linearlist ',' expr  */
#line 871 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3367 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 191: /* proplist: proppair  */
#line 873 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3373 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* proplist: proplist ',' proppair  */
#line 874 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* proppair: SYMBOL ':' expr  */
#line 876 "engines/director/lingo/lingo-gr.y"
                          {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 194: /* proppair: STRING ':' expr  */
#line 880 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3397 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* proppair: ID ':' expr  */
#line 884 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3406 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3410 "engines/director/lingo/lingo-gr.cpp"

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

#line 890 "engines/director/lingo/lingo-gr.y"


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
