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
  YYSYMBOL_VOID = 4,                       /* VOID  */
  YYSYMBOL_VAR = 5,                        /* VAR  */
  YYSYMBOL_POINT = 6,                      /* POINT  */
  YYSYMBOL_RECT = 7,                       /* RECT  */
  YYSYMBOL_ARRAY = 8,                      /* ARRAY  */
  YYSYMBOL_OBJECT = 9,                     /* OBJECT  */
  YYSYMBOL_LEXERROR = 10,                  /* LEXERROR  */
  YYSYMBOL_PARRAY = 11,                    /* PARRAY  */
  YYSYMBOL_CASTREF = 12,                   /* CASTREF  */
  YYSYMBOL_FIELDREF = 13,                  /* FIELDREF  */
  YYSYMBOL_CHUNKREF = 14,                  /* CHUNKREF  */
  YYSYMBOL_INT = 15,                       /* INT  */
  YYSYMBOL_ARGC = 16,                      /* ARGC  */
  YYSYMBOL_ARGCNORET = 17,                 /* ARGCNORET  */
  YYSYMBOL_THEENTITY = 18,                 /* THEENTITY  */
  YYSYMBOL_THEENTITYWITHID = 19,           /* THEENTITYWITHID  */
  YYSYMBOL_THEMENUITEMENTITY = 20,         /* THEMENUITEMENTITY  */
  YYSYMBOL_THEMENUITEMSENTITY = 21,        /* THEMENUITEMSENTITY  */
  YYSYMBOL_FLOAT = 22,                     /* FLOAT  */
  YYSYMBOL_THEFUNC = 23,                   /* THEFUNC  */
  YYSYMBOL_THEFUNCINOF = 24,               /* THEFUNCINOF  */
  YYSYMBOL_VARID = 25,                     /* VARID  */
  YYSYMBOL_STRING = 26,                    /* STRING  */
  YYSYMBOL_SYMBOL = 27,                    /* SYMBOL  */
  YYSYMBOL_ENDCLAUSE = 28,                 /* ENDCLAUSE  */
  YYSYMBOL_tPLAYACCEL = 29,                /* tPLAYACCEL  */
  YYSYMBOL_tMETHOD = 30,                   /* tMETHOD  */
  YYSYMBOL_THEOBJECTPROP = 31,             /* THEOBJECTPROP  */
  YYSYMBOL_tCAST = 32,                     /* tCAST  */
  YYSYMBOL_tFIELD = 33,                    /* tFIELD  */
  YYSYMBOL_tSCRIPT = 34,                   /* tSCRIPT  */
  YYSYMBOL_tWINDOW = 35,                   /* tWINDOW  */
  YYSYMBOL_tDOWN = 36,                     /* tDOWN  */
  YYSYMBOL_tELSE = 37,                     /* tELSE  */
  YYSYMBOL_tELSIF = 38,                    /* tELSIF  */
  YYSYMBOL_tEXIT = 39,                     /* tEXIT  */
  YYSYMBOL_tGLOBAL = 40,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 41,                       /* tGO  */
  YYSYMBOL_tGOLOOP = 42,                   /* tGOLOOP  */
  YYSYMBOL_tIF = 43,                       /* tIF  */
  YYSYMBOL_tIN = 44,                       /* tIN  */
  YYSYMBOL_tINTO = 45,                     /* tINTO  */
  YYSYMBOL_tMACRO = 46,                    /* tMACRO  */
  YYSYMBOL_tMOVIE = 47,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 48,                     /* tNEXT  */
  YYSYMBOL_tOF = 49,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 50,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 51,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 52,                   /* tREPEAT  */
  YYSYMBOL_tSET = 53,                      /* tSET  */
  YYSYMBOL_tTHEN = 54,                     /* tTHEN  */
  YYSYMBOL_tTO = 55,                       /* tTO  */
  YYSYMBOL_tWHEN = 56,                     /* tWHEN  */
  YYSYMBOL_tWITH = 57,                     /* tWITH  */
  YYSYMBOL_tWHILE = 58,                    /* tWHILE  */
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
  YYSYMBOL_tASSERTERROR = 89,              /* tASSERTERROR  */
  YYSYMBOL_90_ = 90,                       /* '<'  */
  YYSYMBOL_91_ = 91,                       /* '>'  */
  YYSYMBOL_92_ = 92,                       /* '&'  */
  YYSYMBOL_93_ = 93,                       /* '+'  */
  YYSYMBOL_94_ = 94,                       /* '-'  */
  YYSYMBOL_95_ = 95,                       /* '*'  */
  YYSYMBOL_96_ = 96,                       /* '/'  */
  YYSYMBOL_97_ = 97,                       /* '%'  */
  YYSYMBOL_98_n_ = 98,                     /* '\n'  */
  YYSYMBOL_99_ = 99,                       /* '('  */
  YYSYMBOL_100_ = 100,                     /* ')'  */
  YYSYMBOL_101_ = 101,                     /* ','  */
  YYSYMBOL_102_ = 102,                     /* '['  */
  YYSYMBOL_103_ = 103,                     /* ']'  */
  YYSYMBOL_104_ = 104,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 105,                 /* $accept  */
  YYSYMBOL_program = 106,                  /* program  */
  YYSYMBOL_programline = 107,              /* programline  */
  YYSYMBOL_ID = 108,                       /* ID  */
  YYSYMBOL_asgn = 109,                     /* asgn  */
  YYSYMBOL_stmtoneliner = 110,             /* stmtoneliner  */
  YYSYMBOL_stmt = 111,                     /* stmt  */
  YYSYMBOL_112_1 = 112,                    /* $@1  */
  YYSYMBOL_113_2 = 113,                    /* $@2  */
  YYSYMBOL_114_3 = 114,                    /* $@3  */
  YYSYMBOL_115_4 = 115,                    /* $@4  */
  YYSYMBOL_116_5 = 116,                    /* $@5  */
  YYSYMBOL_117_6 = 117,                    /* $@6  */
  YYSYMBOL_118_7 = 118,                    /* $@7  */
  YYSYMBOL_119_8 = 119,                    /* $@8  */
  YYSYMBOL_120_9 = 120,                    /* $@9  */
  YYSYMBOL_startrepeat = 121,              /* startrepeat  */
  YYSYMBOL_tellstart = 122,                /* tellstart  */
  YYSYMBOL_asserterrorstart = 123,         /* asserterrorstart  */
  YYSYMBOL_ifstmt = 124,                   /* ifstmt  */
  YYSYMBOL_elseifstmtlist = 125,           /* elseifstmtlist  */
  YYSYMBOL_elseifstmt = 126,               /* elseifstmt  */
  YYSYMBOL_jumpifz = 127,                  /* jumpifz  */
  YYSYMBOL_jump = 128,                     /* jump  */
  YYSYMBOL_varassign = 129,                /* varassign  */
  YYSYMBOL_if = 130,                       /* if  */
  YYSYMBOL_lbl = 131,                      /* lbl  */
  YYSYMBOL_stmtlist = 132,                 /* stmtlist  */
  YYSYMBOL_stmtlistline = 133,             /* stmtlistline  */
  YYSYMBOL_simpleexprnoparens = 134,       /* simpleexprnoparens  */
  YYSYMBOL_135_10 = 135,                   /* $@10  */
  YYSYMBOL_simpleexpr = 136,               /* simpleexpr  */
  YYSYMBOL_expr = 137,                     /* expr  */
  YYSYMBOL_chunkexpr = 138,                /* chunkexpr  */
  YYSYMBOL_reference = 139,                /* reference  */
  YYSYMBOL_proc = 140,                     /* proc  */
  YYSYMBOL_141_11 = 141,                   /* $@11  */
  YYSYMBOL_142_12 = 142,                   /* $@12  */
  YYSYMBOL_143_13 = 143,                   /* $@13  */
  YYSYMBOL_144_14 = 144,                   /* $@14  */
  YYSYMBOL_globallist = 145,               /* globallist  */
  YYSYMBOL_propertylist = 146,             /* propertylist  */
  YYSYMBOL_instancelist = 147,             /* instancelist  */
  YYSYMBOL_gotofunc = 148,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 149,                /* gotomovie  */
  YYSYMBOL_playfunc = 150,                 /* playfunc  */
  YYSYMBOL_151_15 = 151,                   /* $@15  */
  YYSYMBOL_defn = 152,                     /* defn  */
  YYSYMBOL_153_16 = 153,                   /* $@16  */
  YYSYMBOL_154_17 = 154,                   /* $@17  */
  YYSYMBOL_on = 155,                       /* on  */
  YYSYMBOL_156_18 = 156,                   /* $@18  */
  YYSYMBOL_argname = 157,                  /* argname  */
  YYSYMBOL_argdef = 158,                   /* argdef  */
  YYSYMBOL_endargdef = 159,                /* endargdef  */
  YYSYMBOL_argstore = 160,                 /* argstore  */
  YYSYMBOL_arglist = 161,                  /* arglist  */
  YYSYMBOL_nonemptyarglist = 162,          /* nonemptyarglist  */
  YYSYMBOL_list = 163,                     /* list  */
  YYSYMBOL_valuelist = 164,                /* valuelist  */
  YYSYMBOL_linearlist = 165,               /* linearlist  */
  YYSYMBOL_proplist = 166,                 /* proplist  */
  YYSYMBOL_proppair = 167                  /* proppair  */
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
#define YYFINAL  102
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2006

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  105
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  63
/* YYNRULES -- Number of rules.  */
#define YYNRULES  197
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  394

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   344


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
      98,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    97,    92,     2,
      99,   100,    95,    93,   101,    94,     2,    96,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   104,     2,
      90,     2,    91,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   102,     2,   103,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   236,   236,   237,   239,   240,   241,   243,   244,   245,
     246,   247,   249,   256,   260,   271,   278,   281,   288,   291,
     298,   305,   312,   319,   325,   332,   343,   354,   361,   369,
     370,   372,   373,   378,   391,   395,   398,   390,   425,   429,
     432,   424,   459,   466,   472,   458,   502,   510,   513,   514,
     515,   516,   518,   520,   522,   524,   531,   539,   540,   542,
     548,   552,   556,   560,   563,   565,   566,   568,   569,   571,
     574,   577,   581,   585,   586,   587,   588,   589,   597,   603,
     606,   609,   615,   616,   617,   618,   624,   624,   629,   632,
     641,   642,   644,   645,   646,   647,   648,   649,   650,   651,
     652,   653,   654,   655,   656,   657,   658,   659,   660,   661,
     663,   666,   669,   670,   671,   672,   673,   674,   675,   676,
     678,   679,   682,   686,   689,   690,   691,   699,   700,   700,
     701,   701,   702,   702,   703,   706,   709,   715,   715,   720,
     723,   727,   728,   731,   735,   736,   739,   743,   744,   747,
     758,   759,   760,   761,   765,   769,   774,   775,   777,   781,
     785,   789,   789,   819,   819,   825,   826,   826,   836,   844,
     850,   850,   852,   854,   855,   856,   858,   859,   860,   862,
     864,   865,   866,   868,   869,   870,   872,   874,   875,   876,
     877,   879,   880,   882,   883,   885,   889,   893
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
  "end of file", "error", "invalid token", "UNARY", "VOID", "VAR",
  "POINT", "RECT", "ARRAY", "OBJECT", "LEXERROR", "PARRAY", "CASTREF",
  "FIELDREF", "CHUNKREF", "INT", "ARGC", "ARGCNORET", "THEENTITY",
  "THEENTITYWITHID", "THEMENUITEMENTITY", "THEMENUITEMSENTITY", "FLOAT",
  "THEFUNC", "THEFUNCINOF", "VARID", "STRING", "SYMBOL", "ENDCLAUSE",
  "tPLAYACCEL", "tMETHOD", "THEOBJECTPROP", "tCAST", "tFIELD", "tSCRIPT",
  "tWINDOW", "tDOWN", "tELSE", "tELSIF", "tEXIT", "tGLOBAL", "tGO",
  "tGOLOOP", "tIF", "tIN", "tINTO", "tMACRO", "tMOVIE", "tNEXT", "tOF",
  "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH",
  "tWHILE", "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ",
  "tNEQ", "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tENDIF",
  "tENDREPEAT", "tENDTELL", "tASSERTERROR", "'<'", "'>'", "'&'", "'+'",
  "'-'", "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "','", "'['", "']'",
  "':'", "$accept", "program", "programline", "ID", "asgn", "stmtoneliner",
  "stmt", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9",
  "startrepeat", "tellstart", "asserterrorstart", "ifstmt",
  "elseifstmtlist", "elseifstmt", "jumpifz", "jump", "varassign", "if",
  "lbl", "stmtlist", "stmtlistline", "simpleexprnoparens", "$@10",
  "simpleexpr", "expr", "chunkexpr", "reference", "proc", "$@11", "$@12",
  "$@13", "$@14", "globallist", "propertylist", "instancelist", "gotofunc",
  "gotomovie", "playfunc", "$@15", "defn", "$@16", "$@17", "on", "$@18",
  "argname", "argdef", "endargdef", "argstore", "arglist",
  "nonemptyarglist", "list", "valuelist", "linearlist", "proplist",
  "proppair", YY_NULLPTR
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
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
      60,    62,    38,    43,    45,    42,    47,    37,    10,    40,
      41,    44,    91,    93,    58
};
#endif

#define YYPACT_NINF (-359)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-68)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     585,   -72,  -359,  -359,  -359,  -359,  -359,  -359,  -359,   -30,
    -359,  1079,  -359,  -359,  -359,    -8,  1200,   -42,   261,    54,
      54,  1282,  1161,  -359,  1282,  -359,  -359,  -359,    13,  -359,
    1309,  -359,  -359,  -359,  -359,  1282,  -359,  -359,  -359,  -359,
    -359,  -359,  1282,  -359,  -359,    54,  -359,  -359,  1282,  -359,
     -10,  1282,  -359,  -359,  -359,  1391,  1391,  1391,  1391,  1282,
    -359,    -1,  -359,  1282,  1282,  1282,  1282,  1282,  1282,  1282,
    1282,  1282,  1040,   -40,  -359,  -359,  1382,  -359,  -359,  -359,
    -359,    54,  -359,    54,   507,    54,  -359,   -24,  1282,  1282,
     -12,     5,    17,  -359,  1739,  1382,  -359,    54,  1683,    54,
      54,   405,  -359,   585,  1282,  1786,  -359,  1910,    54,  -359,
      54,   -17,  -359,  -359,  1282,  -359,  -359,  -359,  -359,  -359,
    1910,  1282,  -359,  1431,  1465,  1499,  1533,  1876,  -359,  -359,
    1838,   -14,    -6,  -359,   -76,  1910,   -37,    -9,     1,  -359,
    1282,  1282,  1282,  1282,  1282,  1282,  1282,  1282,  1282,  1282,
    1282,  1282,  1282,  1282,  1282,  1282,  1282,  1282,  -359,  -359,
    1282,   159,   159,   159,   -27,  1282,  1282,  1282,  1725,    63,
    1282,  1282,  1282,  1282,  1282,  1282,  -359,    23,  -359,  -359,
    -359,    25,  -359,  -359,  -359,  -359,   -26,  1773,    14,  1282,
      80,  -359,    31,    38,    40,    54,  -359,  1910,  1282,  1282,
    1282,  1282,  1282,  1282,  1282,  1282,  1282,  1282,  -359,  1282,
    1282,  1282,  -359,  1282,    96,    -5,    42,   -59,   -59,   -59,
     -59,   267,   267,  -359,   -38,   -59,   -59,   -59,   -59,   -38,
     -45,   -45,  -359,  -359,    54,    99,  -359,  -359,  -359,  -359,
    -359,  -359,  1282,  1282,  1910,  1910,  1910,  1282,  1282,    54,
    1910,  1910,  1910,  1910,  1910,  1910,    54,   405,   834,    54,
    -359,  -359,  -359,  -359,   772,    54,  -359,  -359,  -359,  -359,
    1567,  -359,  1601,  -359,  1635,  -359,  1669,  -359,  -359,  1910,
    1910,  1910,  1910,   -14,    -6,    41,  -359,  -359,  -359,  -359,
      48,  1282,  1910,   133,  -359,  1910,    78,  1282,  -359,  -359,
    -359,  -359,    51,  -359,  1282,  -359,  -359,   652,   719,  1282,
    1282,  1282,  1282,  1282,  -359,  1910,  -359,  -359,  -359,   896,
      12,  -359,    62,   478,  1825,    52,  -359,   125,  -359,  -359,
    -359,  -359,  -359,    55,   719,  -359,  -359,  -359,  -359,  1282,
    1282,  -359,  -359,  1282,  -359,    24,    54,  -359,  -359,  -359,
     101,   123,    73,  1910,  1910,  -359,   963,  1282,  -359,    81,
    -359,    67,  -359,  1282,   120,  -359,  -359,  1910,  -359,    54,
    -359,  1910,  1282,    92,   126,  -359,   896,  -359,  1910,  -359,
     772,    95,  -359,  -359,  -359,  -359,   896,  -359,  -359,   115,
     896,  -359,   117,  -359
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     7,   161,   166,     8,     9,    10,    11,   127,
     128,     0,   150,    63,   163,     0,     0,     0,     0,     0,
       0,     0,     0,   132,     0,   130,   170,    54,     0,     2,
     180,    29,    31,     6,    32,     0,    30,   124,   125,     5,
      64,    51,   180,    64,   126,   141,    69,    77,     0,    70,
       0,     0,    72,    71,    81,     8,     9,    10,    11,     0,
     151,     0,   152,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   187,    89,    90,    92,   153,   120,    76,   155,
      84,     0,    46,     0,   123,     0,    64,     0,     0,     0,
       0,     0,     0,   165,   135,   158,   160,   147,     0,   144,
       0,     0,     1,     0,   180,   181,   140,    60,   173,   162,
     173,   142,   129,    78,     0,    79,   111,   110,   121,   122,
     157,     0,    75,     0,     0,     0,     0,     0,    73,    74,
       0,    72,    71,   188,    89,   191,     0,   190,   189,   193,
     180,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   154,    64,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   159,   148,   133,    53,
      53,   145,   131,   171,    50,     3,    89,     0,     0,   180,
       0,   172,   174,     0,     0,   141,    80,   156,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    91,     0,
       0,     0,   186,     0,     0,    89,     0,   102,   103,   100,
     101,   104,   105,    97,   107,   108,   109,    99,    98,   106,
      93,    94,    95,    96,   173,     0,    12,    13,    15,    16,
      17,    18,     0,     0,    60,    22,    20,     0,     0,     0,
      27,    28,    21,    19,    47,   134,   147,     0,     0,   144,
     136,   137,   139,   182,     0,   173,   179,   179,   143,   112,
       0,   114,     0,   116,     0,   118,     0,    82,    83,   196,
     195,   197,   192,     0,     0,     0,   194,    85,    86,    88,
       0,     0,    42,    34,    52,    23,   100,     0,   149,    64,
      68,    64,    65,   146,     0,    61,   175,     0,     0,     0,
       0,     0,     0,     0,   179,    14,    64,    62,    62,     0,
       0,    49,     0,     0,   183,     0,    57,   169,   167,   113,
     115,   117,   119,     0,     0,    43,    35,    39,    61,     0,
       0,    48,    66,   184,   138,    64,   176,    87,   164,    60,
       0,     0,     0,    25,    26,   185,     0,     0,    58,     0,
     177,   168,    44,     0,     0,    33,    64,    60,    55,     0,
      52,    36,     0,     0,     0,   178,     0,    60,    40,    56,
       0,     0,    52,    60,    61,    45,     0,    52,    59,     0,
       0,    37,     0,    41
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -359,  -359,   102,     0,  -359,  -100,     6,  -359,  -359,  -359,
    -359,  -359,  -359,  -359,  -359,  -359,  -358,    29,  -359,  -359,
    -359,  -359,  -242,  -330,  -108,  -359,   -39,  -243,  -359,   132,
    -359,   -21,   250,   -44,  -359,  -359,  -359,  -359,  -359,  -359,
      16,   -47,   -43,  -359,   -13,  -359,  -359,  -359,  -359,  -359,
    -359,  -359,  -359,  -107,  -359,  -262,   -35,  -303,  -359,  -359,
    -359,  -359,     2
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    28,    29,    73,    31,    32,   300,   317,   350,   377,
     318,   351,   383,   316,   349,   370,   319,   257,   101,    34,
     345,   358,   190,   326,   336,    35,   108,   301,   302,    74,
     313,    75,   105,    77,    78,    36,    45,    99,    97,   304,
     112,   182,   178,    37,    79,    38,    42,    39,    81,    43,
      40,   100,   192,   193,   361,   307,   106,   325,    80,   136,
     137,   138,   139
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,   184,   294,   194,   110,   308,    33,   109,   352,    96,
     333,   147,   376,   102,   148,    85,    86,   242,    91,    92,
      93,   305,    44,   140,   386,   147,    41,   113,   211,   390,
     115,   166,   147,   153,   154,   155,   156,   157,   243,   114,
     355,   167,   122,   170,    82,   111,   121,   165,   128,   129,
     156,   157,   334,   171,   388,   154,   155,   156,   157,   140,
     172,   356,   357,   158,   327,   328,   212,   339,   169,   188,
     173,   174,   134,   140,   260,   261,   338,   340,   -24,     2,
     342,   159,   176,   160,   195,   164,     5,     6,     7,     8,
     209,   348,   213,   196,   140,   287,   288,   177,   210,   181,
     183,    30,   214,    30,   186,   216,   -24,   362,   191,    33,
     191,   103,   249,   366,   262,   -24,   -24,   237,   239,   241,
     234,     2,   283,   284,   256,   374,   259,   290,     5,     6,
       7,     8,   265,   381,   264,   382,   266,   384,   267,   235,
     215,   387,   289,   389,   291,   211,   314,   392,   147,   323,
     341,   148,   344,   346,   263,   347,   363,   299,   306,   364,
     365,   236,   238,   240,   -24,   -24,   -24,   368,   369,   -38,
     153,   154,   155,   156,   157,   372,   -24,   269,   379,   271,
     380,   273,   385,   275,     2,   277,   278,   116,   117,   118,
     119,    55,    56,     7,     8,   111,   141,   142,   143,   144,
     145,   146,   391,   147,   393,   185,   148,   149,   150,   258,
     337,   268,   303,   298,   285,     0,   286,     0,     0,     0,
       0,     0,     0,   151,   152,   153,   154,   155,   156,   157,
       0,     0,     0,     0,   191,    64,    65,    66,    67,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   297,
       0,     0,     0,     0,     0,     0,   177,    30,    30,   181,
     321,    76,   322,     0,    30,   191,    84,     0,     0,     0,
       0,    94,    95,     0,    98,     0,   320,   335,     0,    87,
      88,    89,     0,     0,     0,   107,     2,     0,   329,   330,
     331,   332,    90,     5,     6,     7,     8,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   359,    30,    30,   120,
       0,     0,     0,     0,   123,   124,   125,   126,   127,    30,
       0,   130,   135,    30,     0,     0,     0,   373,     0,     0,
     141,   142,   143,   144,    30,     0,     0,   147,   168,     0,
     148,   149,   150,     0,     0,     0,   360,     0,     0,     0,
       0,     0,     0,     0,   187,     0,    30,   151,   152,   153,
     154,   155,   156,   157,     0,     0,     0,     0,     0,   375,
       0,   197,     0,     0,     0,     0,    30,     0,     0,     0,
      30,     0,     0,     0,     0,     0,    30,     0,     0,     0,
      30,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,     0,     0,
       0,     0,     0,     0,     0,   244,   245,   246,     0,     0,
     250,   251,   252,   253,   254,   255,     0,     0,     0,     0,
       2,     0,     0,     0,     3,     0,     0,     5,     6,     7,
       8,     0,     0,     0,     9,    10,    11,    12,     0,   270,
       0,   272,     0,   274,     0,   276,    16,     0,    18,   279,
     280,   281,     0,   282,     0,    21,    22,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   -67,     1,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
       0,     0,   292,   293,     0,     0,     0,   295,   296,     0,
       0,     0,     0,     2,     0,     0,   -67,     3,     0,     0,
       5,     6,     7,     8,     0,   -67,   -67,     9,    10,    11,
      12,    13,     0,     0,     0,     0,    15,     0,     0,    16,
      17,    18,     0,     0,    19,     0,     0,     0,    21,    22,
      23,   315,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   161,     0,   324,     0,     0,     0,     0,     0,
       0,    24,    25,   324,   -67,   -67,   -67,    27,     0,     0,
     141,   142,   143,   144,   145,   146,   -67,   147,   162,   163,
     148,   149,   150,     0,     0,    -4,     1,     0,     0,   353,
     354,     0,     0,   324,     0,     0,     0,   151,   152,   153,
     154,   155,   156,   157,     0,     0,     0,   367,     0,     0,
       2,     0,     0,   371,     3,     4,     0,     5,     6,     7,
       8,     0,   378,     0,     9,    10,    11,    12,    13,     0,
       0,    14,     0,    15,     0,     0,    16,    17,    18,     0,
       0,    19,     0,     0,    20,    21,    22,    23,     0,     0,
       0,     0,   -67,     1,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
      26,     0,     0,     0,    27,     0,     0,     2,     0,     0,
     -67,     3,     0,    -4,     5,     6,     7,     8,     0,     0,
       0,     9,    10,    11,    12,    13,     0,     0,     0,     0,
      15,     0,     0,    16,    17,    18,     0,     0,    19,     0,
       0,     0,    21,    22,    23,     0,     0,     0,     0,   -67,
       1,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    24,    25,     0,     0,     0,
       0,    27,     0,     0,     2,     0,     0,     0,     3,     0,
     -67,     5,     6,     7,     8,     0,     0,     0,     9,    10,
      11,    12,    13,     0,     0,     0,     0,    15,     0,     0,
      16,    17,    18,     1,     0,    19,     0,     0,     0,    21,
      22,    23,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     2,     0,     0,
       0,     3,    24,    25,     5,     6,     7,     8,    27,   -67,
     -67,     9,    10,    11,    12,    13,     0,   -67,     0,     0,
      15,     0,     0,    16,    17,    18,     0,     0,    19,     0,
       0,     0,    21,    22,    23,     1,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    24,    25,     0,   -67,     2,
       0,    27,     0,     3,     0,     0,     5,     6,     7,     8,
     -67,     0,     0,     9,    10,    11,    12,    13,     0,     0,
       0,     0,    15,     0,     0,    16,    17,    18,     0,     0,
      19,     0,     0,     0,    21,    22,    23,     1,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    24,    25,     0,
       0,     2,   -67,    27,     0,     3,     0,     0,     5,     6,
       7,     8,   -67,     0,     0,     9,    10,    11,    12,    13,
       0,     0,     0,     0,    15,     0,     0,    16,    17,    18,
       0,     0,    19,     0,     0,     0,    21,    22,    23,     0,
       0,     0,     0,     0,     1,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
      25,     0,     0,   -67,     0,    27,     0,     0,     2,     0,
       0,     0,     3,     0,   -67,     5,     6,     7,     8,     0,
       0,     0,     9,    10,    11,    12,    13,     0,     0,     0,
       0,    15,     0,     0,    16,    17,    18,     0,     0,    19,
       0,     0,     0,    21,    22,    23,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    25,     0,   -67,
       0,     0,    27,     0,     0,    46,     0,     0,    47,    48,
       0,   -67,    49,    50,    51,     2,   131,   132,     0,     0,
       0,    54,    55,    56,    57,    58,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,    49,    50,    51,     2,    52,    53,     0,     0,    63,
      54,    55,    56,    57,    58,     0,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,    59,    60,    61,    62,
       0,     0,     0,    69,    70,     0,     0,     0,     0,    71,
       0,     0,    72,     0,   133,     0,     0,     0,    63,     0,
       0,     0,     0,     0,     0,    64,    65,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,    70,     0,     0,    46,     0,    71,    47,
      48,    72,     0,    49,    50,    51,     2,    52,    53,     0,
       0,     0,    54,    55,    56,    57,    58,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,     0,
      61,     0,     0,     0,     0,    46,     0,     0,    47,    48,
       0,    83,    49,    50,    51,     2,    52,    53,     0,     0,
      63,    54,    55,    56,    57,    58,     0,    64,    65,    66,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    69,    70,     0,     0,     0,     0,
      71,     0,     0,    72,     0,     0,     0,     0,     0,    63,
       0,     0,     0,     0,     0,     0,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,    70,     0,     0,    46,     0,    71,
      47,    48,    72,     0,    49,    50,    51,     2,    52,    53,
       0,     0,     0,    54,    55,    56,    57,    58,     0,     0,
       0,     0,     0,     0,    46,     0,     0,    47,    48,     0,
       0,    49,    50,    51,     2,    52,    53,     0,     0,     0,
      54,    55,    56,    57,    58,     0,     0,     0,     0,     0,
       0,    63,     0,     0,     0,     0,     0,     0,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,    70,     0,    63,     0,
       0,    71,     0,     0,    72,    64,    65,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,    70,     0,     0,    46,     0,   104,    47,
      48,    72,     0,    49,    50,    51,     2,    52,    53,     0,
       0,     0,    54,    55,    56,    57,    58,     0,     0,    59,
       0,    61,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   141,   142,   143,   144,   145,
     146,     0,   147,     0,     0,   148,   149,   150,     0,     0,
      63,     0,     0,     0,     0,     0,     0,    64,    65,    66,
      67,    68,   151,   152,   153,   154,   155,   156,   157,     0,
     198,     0,     0,     0,     0,     0,   199,     0,     0,     0,
       0,     0,     0,    72,   141,   142,   143,   144,   145,   146,
       0,   147,     0,     0,   148,   149,   150,     0,     0,     0,
       0,     0,     0,     0,   200,     0,     0,     0,     0,     0,
     201,   151,   152,   153,   154,   155,   156,   157,   141,   142,
     143,   144,   145,   146,     0,   147,     0,     0,   148,   149,
     150,     0,     0,     0,     0,     0,     0,     0,   202,     0,
       0,     0,     0,     0,   203,   151,   152,   153,   154,   155,
     156,   157,   141,   142,   143,   144,   145,   146,     0,   147,
       0,     0,   148,   149,   150,     0,     0,     0,     0,     0,
       0,     0,   204,     0,     0,     0,     0,     0,   205,   151,
     152,   153,   154,   155,   156,   157,   141,   142,   143,   144,
     145,   146,     0,   147,     0,     0,   148,   149,   150,     0,
       0,     0,     0,     0,     0,     0,   309,     0,     0,     0,
       0,     0,     0,   151,   152,   153,   154,   155,   156,   157,
     141,   142,   143,   144,   145,   146,     0,   147,     0,     0,
     148,   149,   150,     0,     0,     0,     0,     0,     0,     0,
     310,     0,     0,     0,     0,     0,     0,   151,   152,   153,
     154,   155,   156,   157,   141,   142,   143,   144,   145,   146,
       0,   147,     0,     0,   148,   149,   150,     0,     0,     0,
       0,     0,     0,     0,   311,     0,     0,     0,     0,     0,
       0,   151,   152,   153,   154,   155,   156,   157,   141,   142,
     143,   144,   145,   146,     0,   147,     0,     0,   148,   149,
     150,     0,     0,     0,     0,     0,     0,     0,   312,     0,
       0,     0,     0,     0,     0,   151,   152,   153,   154,   155,
     156,   157,   141,   142,   143,   144,   145,   146,   179,   147,
       0,     0,   148,   149,   150,     0,   141,   142,   143,   144,
     145,   146,     0,   147,     0,     0,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,     0,     0,     0,     0,
       0,     0,     0,   151,   152,   153,   154,   155,   156,   157,
     247,   180,     0,     0,     0,     0,     0,     0,   141,   142,
     248,   144,   145,   146,     0,   147,   175,     0,   148,   149,
     150,     0,   141,   142,   143,   144,   145,   146,     0,   147,
       0,     0,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,     0,     0,     0,     0,     0,     0,     0,   151,
     152,   153,   154,   155,   156,   157,   141,   142,   143,   144,
     145,   146,     0,   147,     0,     0,   148,   149,   150,   141,
     142,   143,   144,   145,   146,     0,   147,     0,     0,   148,
     149,   150,     0,   151,   152,   153,   154,   155,   156,   157,
       0,     0,     0,   208,   189,     0,   151,   152,   153,   154,
     155,   156,   157,     0,     0,     0,     0,   189,   141,   142,
     143,   144,   145,   146,     0,   147,     0,     0,   148,   149,
     150,   141,   142,   143,   144,   145,   146,     0,   147,     0,
       0,   148,   149,   150,     0,   151,   152,   153,   154,   155,
     156,   157,     0,     0,     0,     0,   343,     0,   151,   152,
     153,   154,   155,   156,   157,     0,     0,     0,   208,   141,
     142,   143,   144,   145,   146,     0,   147,     0,     0,   148,
     149,   150,     0,     0,     0,     0,     0,   206,   207,     0,
       0,     0,     0,     0,     0,     0,   151,   152,   153,   154,
     155,   156,   157,   141,   142,   143,   144,   145,   146,     0,
     147,     0,     0,   148,   149,   150,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     151,   152,   153,   154,   155,   156,   157
};

static const yytype_int16 yycheck[] =
{
       0,   101,   244,   110,    43,   267,     0,    42,   338,    22,
     313,    70,   370,     0,    73,    57,    58,    44,    18,    19,
      20,   264,    52,    99,   382,    70,    98,    48,   104,   387,
      51,    55,    70,    92,    93,    94,    95,    96,    65,    49,
     343,    65,    63,    55,    52,    45,    47,    86,    69,    70,
      95,    96,   314,    65,   384,    93,    94,    95,    96,    99,
      55,    37,    38,    76,   307,   308,   103,    55,    89,   104,
      65,    54,    72,    99,   100,   101,   319,    65,     0,    25,
     323,    81,    95,    83,   101,    85,    32,    33,    34,    35,
     104,   334,   101,   114,    99,   100,   101,    97,   104,    99,
     100,   101,   101,   103,   104,   140,    28,   349,   108,   103,
     110,    98,    49,   356,   100,    37,    38,   161,   162,   163,
     159,    25,    26,    27,   101,   367,   101,   234,    32,    33,
      34,    35,   101,   376,    54,   377,    98,   380,    98,   160,
     140,   383,   100,   386,    45,   104,    98,   390,    70,    98,
      88,    73,   100,    28,   189,   100,    55,   257,   265,    36,
      87,   161,   162,   163,    86,    87,    88,    86,   101,    36,
      92,    93,    94,    95,    96,    55,    98,   198,    86,   200,
      54,   202,    87,   204,    25,   206,   207,    55,    56,    57,
      58,    32,    33,    34,    35,   195,    63,    64,    65,    66,
      67,    68,    87,    70,    87,   103,    73,    74,    75,   180,
     318,   195,   259,   256,   214,    -1,   214,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      -1,    -1,    -1,    -1,   234,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   249,
      -1,    -1,    -1,    -1,    -1,    -1,   256,   257,   258,   259,
     299,    11,   301,    -1,   264,   265,    16,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,   297,   316,    -1,    18,
      19,    20,    -1,    -1,    -1,    35,    25,    -1,   309,   310,
     311,   312,    31,    32,    33,    34,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   345,   307,   308,    59,
      -1,    -1,    -1,    -1,    64,    65,    66,    67,    68,   319,
      -1,    71,    72,   323,    -1,    -1,    -1,   366,    -1,    -1,
      63,    64,    65,    66,   334,    -1,    -1,    70,    88,    -1,
      73,    74,    75,    -1,    -1,    -1,   346,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,   356,    90,    91,    92,
      93,    94,    95,    96,    -1,    -1,    -1,    -1,    -1,   369,
      -1,   121,    -1,    -1,    -1,    -1,   376,    -1,    -1,    -1,
     380,    -1,    -1,    -1,    -1,    -1,   386,    -1,    -1,    -1,
     390,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   165,   166,   167,    -1,    -1,
     170,   171,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
      25,    -1,    -1,    -1,    29,    -1,    -1,    32,    33,    34,
      35,    -1,    -1,    -1,    39,    40,    41,    42,    -1,   199,
      -1,   201,    -1,   203,    -1,   205,    51,    -1,    53,   209,
     210,   211,    -1,   213,    -1,    60,    61,    62,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     0,     1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    84,
      -1,    -1,   242,   243,    -1,    -1,    -1,   247,   248,    -1,
      -1,    -1,    -1,    25,    -1,    -1,    28,    29,    -1,    -1,
      32,    33,    34,    35,    -1,    37,    38,    39,    40,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    -1,    51,
      52,    53,    -1,    -1,    56,    -1,    -1,    -1,    60,    61,
      62,   291,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,   304,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    84,   313,    86,    87,    88,    89,    -1,    -1,
      63,    64,    65,    66,    67,    68,    98,    70,    71,    72,
      73,    74,    75,    -1,    -1,     0,     1,    -1,    -1,   339,
     340,    -1,    -1,   343,    -1,    -1,    -1,    90,    91,    92,
      93,    94,    95,    96,    -1,    -1,    -1,   357,    -1,    -1,
      25,    -1,    -1,   363,    29,    30,    -1,    32,    33,    34,
      35,    -1,   372,    -1,    39,    40,    41,    42,    43,    -1,
      -1,    46,    -1,    48,    -1,    -1,    51,    52,    53,    -1,
      -1,    56,    -1,    -1,    59,    60,    61,    62,    -1,    -1,
      -1,    -1,     0,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,    84,
      85,    -1,    -1,    -1,    89,    -1,    -1,    25,    -1,    -1,
      28,    29,    -1,    98,    32,    33,    34,    35,    -1,    -1,
      -1,    39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    51,    52,    53,    -1,    -1,    56,    -1,
      -1,    -1,    60,    61,    62,    -1,    -1,    -1,    -1,     0,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    84,    -1,    -1,    -1,
      -1,    89,    -1,    -1,    25,    -1,    -1,    -1,    29,    -1,
      98,    32,    33,    34,    35,    -1,    -1,    -1,    39,    40,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      51,    52,    53,     1,    -1,    56,    -1,    -1,    -1,    60,
      61,    62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    -1,
      -1,    29,    83,    84,    32,    33,    34,    35,    89,    37,
      38,    39,    40,    41,    42,    43,    -1,    98,    -1,    -1,
      48,    -1,    -1,    51,    52,    53,    -1,    -1,    56,    -1,
      -1,    -1,    60,    61,    62,     1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    84,    -1,    86,    25,
      -1,    89,    -1,    29,    -1,    -1,    32,    33,    34,    35,
      98,    -1,    -1,    39,    40,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    51,    52,    53,    -1,    -1,
      56,    -1,    -1,    -1,    60,    61,    62,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,    84,    -1,
      -1,    25,    88,    89,    -1,    29,    -1,    -1,    32,    33,
      34,    35,    98,    -1,    -1,    39,    40,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    51,    52,    53,
      -1,    -1,    56,    -1,    -1,    -1,    60,    61,    62,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,
      84,    -1,    -1,    87,    -1,    89,    -1,    -1,    25,    -1,
      -1,    -1,    29,    -1,    98,    32,    33,    34,    35,    -1,
      -1,    -1,    39,    40,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    51,    52,    53,    -1,    -1,    56,
      -1,    -1,    -1,    60,    61,    62,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    83,    84,    -1,    86,
      -1,    -1,    89,    -1,    -1,    15,    -1,    -1,    18,    19,
      -1,    98,    22,    23,    24,    25,    26,    27,    -1,    -1,
      -1,    31,    32,    33,    34,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    -1,    -1,    18,    19,    -1,
      -1,    22,    23,    24,    25,    26,    27,    -1,    -1,    69,
      31,    32,    33,    34,    35,    -1,    76,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    50,
      -1,    -1,    -1,    93,    94,    -1,    -1,    -1,    -1,    99,
      -1,    -1,   102,    -1,   104,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    93,    94,    -1,    -1,    15,    -1,    99,    18,
      19,   102,    -1,    22,    23,    24,    25,    26,    27,    -1,
      -1,    -1,    31,    32,    33,    34,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,
      49,    -1,    -1,    -1,    -1,    15,    -1,    -1,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    -1,
      69,    31,    32,    33,    34,    35,    -1,    76,    77,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    93,    94,    -1,    -1,    -1,    -1,
      99,    -1,    -1,   102,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    94,    -1,    -1,    15,    -1,    99,
      18,    19,   102,    -1,    22,    23,    24,    25,    26,    27,
      -1,    -1,    -1,    31,    32,    33,    34,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    -1,    -1,    18,    19,    -1,
      -1,    22,    23,    24,    25,    26,    27,    -1,    -1,    -1,
      31,    32,    33,    34,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    93,    94,    -1,    69,    -1,
      -1,    99,    -1,    -1,   102,    76,    77,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    93,    94,    -1,    -1,    15,    -1,    99,    18,
      19,   102,    -1,    22,    23,    24,    25,    26,    27,    -1,
      -1,    -1,    31,    32,    33,    34,    35,    -1,    -1,    47,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,
      68,    -1,    70,    -1,    -1,    73,    74,    75,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    90,    91,    92,    93,    94,    95,    96,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,   102,    63,    64,    65,    66,    67,    68,
      -1,    70,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      55,    90,    91,    92,    93,    94,    95,    96,    63,    64,
      65,    66,    67,    68,    -1,    70,    -1,    -1,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    55,    90,    91,    92,    93,    94,
      95,    96,    63,    64,    65,    66,    67,    68,    -1,    70,
      -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    55,    90,
      91,    92,    93,    94,    95,    96,    63,    64,    65,    66,
      67,    68,    -1,    70,    -1,    -1,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      63,    64,    65,    66,    67,    68,    -1,    70,    -1,    -1,
      73,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,
      93,    94,    95,    96,    63,    64,    65,    66,    67,    68,
      -1,    70,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    90,    91,    92,    93,    94,    95,    96,    63,    64,
      65,    66,    67,    68,    -1,    70,    -1,    -1,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    63,    64,    65,    66,    67,    68,    55,    70,
      -1,    -1,    73,    74,    75,    -1,    63,    64,    65,    66,
      67,    68,    -1,    70,    -1,    -1,    73,    74,    75,    90,
      91,    92,    93,    94,    95,    96,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      55,    98,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,
      65,    66,    67,    68,    -1,    70,    57,    -1,    73,    74,
      75,    -1,    63,    64,    65,    66,    67,    68,    -1,    70,
      -1,    -1,    73,    74,    75,    90,    91,    92,    93,    94,
      95,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    95,    96,    63,    64,    65,    66,
      67,    68,    -1,    70,    -1,    -1,    73,    74,    75,    63,
      64,    65,    66,    67,    68,    -1,    70,    -1,    -1,    73,
      74,    75,    -1,    90,    91,    92,    93,    94,    95,    96,
      -1,    -1,    -1,   100,   101,    -1,    90,    91,    92,    93,
      94,    95,    96,    -1,    -1,    -1,    -1,   101,    63,    64,
      65,    66,    67,    68,    -1,    70,    -1,    -1,    73,    74,
      75,    63,    64,    65,    66,    67,    68,    -1,    70,    -1,
      -1,    73,    74,    75,    -1,    90,    91,    92,    93,    94,
      95,    96,    -1,    -1,    -1,    -1,   101,    -1,    90,    91,
      92,    93,    94,    95,    96,    -1,    -1,    -1,   100,    63,
      64,    65,    66,    67,    68,    -1,    70,    -1,    -1,    73,
      74,    75,    -1,    -1,    -1,    -1,    -1,    81,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,    93,
      94,    95,    96,    63,    64,    65,    66,    67,    68,    -1,
      70,    -1,    -1,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    91,    92,    93,    94,    95,    96
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    25,    29,    30,    32,    33,    34,    35,    39,
      40,    41,    42,    43,    46,    48,    51,    52,    53,    56,
      59,    60,    61,    62,    83,    84,    85,    89,   106,   107,
     108,   109,   110,   111,   124,   130,   140,   148,   150,   152,
     155,    98,   151,   154,    52,   141,    15,    18,    19,    22,
      23,    24,    26,    27,    31,    32,    33,    34,    35,    47,
      48,    49,    50,    69,    76,    77,    78,    79,    80,    93,
      94,    99,   102,   108,   134,   136,   137,   138,   139,   149,
     163,   153,    52,    21,   137,    57,    58,    18,    19,    20,
      31,   108,   108,   108,   137,   137,   149,   143,   137,   142,
     156,   123,     0,    98,    99,   137,   161,   137,   131,   161,
     131,   108,   145,   136,    49,   136,   134,   134,   134,   134,
     137,    47,   136,   137,   137,   137,   137,   137,   136,   136,
     137,    26,    27,   104,   108,   137,   164,   165,   166,   167,
      99,    63,    64,    65,    66,    67,    68,    70,    73,    74,
      75,    90,    91,    92,    93,    94,    95,    96,   149,   108,
     108,    45,    71,    72,   108,   131,    55,    65,   137,   136,
      55,    65,    55,    65,    54,    57,   149,   108,   147,    55,
      98,   108,   146,   108,   110,   107,   108,   137,   161,   101,
     127,   108,   157,   158,   158,   101,   136,   137,    49,    55,
      49,    55,    49,    55,    49,    55,    81,    82,   100,   104,
     104,   104,   103,   101,   101,   108,   161,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   131,   136,   108,   138,   108,   138,
     108,   138,    44,    65,   137,   137,   137,    55,    65,    49,
     137,   137,   137,   137,   137,   137,   101,   122,   122,   101,
     100,   101,   100,   161,    54,   101,    98,    98,   145,   136,
     137,   136,   137,   136,   137,   136,   137,   136,   136,   137,
     137,   137,   137,    26,    27,   108,   167,   100,   101,   100,
     158,    45,   137,   137,   127,   137,   137,   108,   147,   110,
     111,   132,   133,   146,   144,   132,   158,   160,   160,    49,
      49,    49,    49,   135,    98,   137,   118,   112,   115,   121,
     136,   131,   131,    98,   137,   162,   128,   132,   132,   136,
     136,   136,   136,   162,   160,   131,   129,   129,   132,    55,
      65,    88,   132,   101,   100,   125,    28,   100,   132,   119,
     113,   116,   128,   137,   137,   162,    37,    38,   126,   131,
     108,   159,   127,    55,    36,    87,   132,   137,    86,   101,
     120,   137,    55,   131,   127,   108,   121,   114,   137,    86,
      54,   132,   127,   117,   132,    87,   121,   127,   128,   132,
     121,    87,   132,    87
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   105,   106,   106,   107,   107,   107,   108,   108,   108,
     108,   108,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   110,
     110,   111,   111,   111,   112,   113,   114,   111,   115,   116,
     117,   111,   118,   119,   120,   111,   111,   111,   111,   111,
     111,   111,   121,   122,   123,   124,   124,   125,   125,   126,
     127,   128,   129,   130,   131,   132,   132,   133,   133,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   135,   134,   134,   134,
     136,   136,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     139,   139,   139,   140,   140,   140,   140,   140,   141,   140,
     142,   140,   143,   140,   140,   140,   140,   144,   140,   140,
     140,   145,   145,   145,   146,   146,   146,   147,   147,   147,
     148,   148,   148,   148,   148,   148,   149,   149,   150,   150,
     150,   151,   150,   153,   152,   152,   154,   152,   152,   152,
     156,   155,   157,   158,   158,   158,   159,   159,   159,   160,
     161,   161,   161,   162,   162,   162,   163,   164,   164,   164,
     164,   165,   165,   166,   166,   167,   167,   167
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     4,     4,     6,     4,     4,     4,     4,     4,
       4,     4,     4,     5,     5,     8,     8,     4,     4,     1,
       1,     1,     1,     9,     0,     0,     0,    15,     0,     0,
       0,    16,     0,     0,     0,    13,     2,     4,     7,     6,
       3,     2,     0,     0,     0,     9,    11,     0,     2,     6,
       0,     0,     0,     1,     0,     1,     3,     0,     1,     1,
       1,     1,     1,     2,     2,     2,     1,     1,     2,     2,
       3,     1,     4,     4,     1,     4,     0,     7,     4,     1,
       1,     3,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     4,     6,     4,     6,     4,     6,     4,     6,
       1,     2,     2,     2,     1,     1,     2,     1,     0,     3,
       0,     3,     0,     3,     4,     2,     4,     0,     7,     4,
       2,     0,     1,     3,     0,     1,     3,     0,     1,     3,
       1,     2,     2,     2,     3,     2,     3,     2,     2,     3,
       2,     0,     3,     0,     8,     2,     0,     7,     8,     6,
       0,     3,     1,     0,     1,     3,     0,     1,     3,     0,
       0,     1,     3,     1,     2,     3,     3,     0,     1,     1,
       1,     1,     3,     1,     3,     3,     3,     3
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
#line 232 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1762 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_THEFUNCINOF: /* THEFUNCINOF  */
#line 232 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1768 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_VARID: /* VARID  */
#line 232 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1774 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_STRING: /* STRING  */
#line 232 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1780 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_SYMBOL: /* SYMBOL  */
#line 232 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1786 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ENDCLAUSE: /* ENDCLAUSE  */
#line 232 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1792 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tPLAYACCEL: /* tPLAYACCEL  */
#line 232 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1798 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tMETHOD: /* tMETHOD  */
#line 232 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1804 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 232 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1810 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_on: /* on  */
#line 232 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1816 "engines/director/lingo/lingo-gr.cpp"
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
#line 244 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2084 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9: /* ID: tFIELD  */
#line 245 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2090 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* ID: tSCRIPT  */
#line 246 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2096 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* ID: tWINDOW  */
#line 247 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2102 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* asgn: tPUT expr tINTO ID  */
#line 249 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2114 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* asgn: tPUT expr tINTO chunkexpr  */
#line 256 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2122 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* asgn: tPUT THEMENUITEMSENTITY ID simpleexpr tINTO expr  */
#line 260 "engines/director/lingo/lingo-gr.y"
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
#line 2138 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* asgn: tPUT expr tAFTER ID  */
#line 271 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_putafter);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2150 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* asgn: tPUT expr tAFTER chunkexpr  */
#line 278 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_putafter);
		(yyval.code) = (yyvsp[-2].code); }
#line 2158 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* asgn: tPUT expr tBEFORE ID  */
#line 281 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_putbefore);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2170 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18: /* asgn: tPUT expr tBEFORE chunkexpr  */
#line 288 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_putbefore);
		(yyval.code) = (yyvsp[-2].code); }
#line 2178 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* asgn: tSET ID tEQ expr  */
#line 291 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* asgn: tSET THEENTITY tEQ expr  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2202 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* asgn: tSET ID tTO expr  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2214 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* asgn: tSET THEENTITY tTO expr  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2226 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* asgn: tSET THEENTITYWITHID expr tTO expr  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                                                {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2237 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* asgn: tSET THEENTITYWITHID expr tEQ expr  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                                                {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2248 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* asgn: tSET THEMENUITEMENTITY simpleexpr tOF ID simpleexpr tTO expr  */
#line 332 "engines/director/lingo/lingo-gr.y"
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
#line 2264 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* asgn: tSET THEMENUITEMENTITY simpleexpr tOF ID simpleexpr tEQ expr  */
#line 343 "engines/director/lingo/lingo-gr.y"
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
#line 2280 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* asgn: tSET THEOBJECTPROP tTO expr  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2292 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* asgn: tSET THEOBJECTPROP tEQ expr  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* stmt: tREPEAT tWHILE lbl expr jumpifz startrepeat stmtlist jump tENDREPEAT  */
#line 378 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-6].code) - (yyvsp[-1].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-4].code) + 2);
		(*g_lingo->_currentAssembly)[(yyvsp[-4].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentAssembly)[(yyvsp[-1].code)] = start;	/* looping back */
		endRepeat((yyvsp[-1].code) + 1, (yyvsp[-6].code));	}
#line 2316 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* $@1: %empty  */
#line 391 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2324 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* $@2: %empty  */
#line 395 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2331 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* $@3: %empty  */
#line 398 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_le); }
#line 2337 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* stmt: tREPEAT tWITH ID tEQ expr $@1 varassign $@2 tTO expr $@3 jumpifz startrepeat stmtlist tENDREPEAT  */
#line 398 "engines/director/lingo/lingo-gr.y"
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
#line 2362 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* $@4: %empty  */
#line 425 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* $@5: %empty  */
#line 429 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2377 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* $@6: %empty  */
#line 432 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_ge); }
#line 2383 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* stmt: tREPEAT tWITH ID tEQ expr $@4 varassign $@5 tDOWN tTO expr $@6 jumpifz startrepeat stmtlist tENDREPEAT  */
#line 433 "engines/director/lingo/lingo-gr.y"
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
#line 2408 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* $@7: %empty  */
#line 459 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  Common::String count("count");
				  g_lingo->codeFunc(&count, 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
#line 2419 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* $@8: %empty  */
#line 466 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
#line 2429 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* $@9: %empty  */
#line 472 "engines/director/lingo/lingo-gr.y"
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
#line 2444 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* stmt: tREPEAT tWITH ID tIN expr $@7 lbl $@8 jumpifz $@9 startrepeat stmtlist tENDREPEAT  */
#line 482 "engines/director/lingo/lingo-gr.y"
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
#line 2468 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* stmt: tNEXT tREPEAT  */
#line 502 "engines/director/lingo/lingo-gr.y"
                                        {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->nexts.push_back(pos);
		} else {
			warning("# LINGO: next repeat not inside repeat block");
		} }
#line 2481 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* stmt: tWHEN ID tTHEN expr  */
#line 510 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_whencode);
		g_lingo->codeString((yyvsp[-2].s)->c_str()); }
#line 2489 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* stmt: tTELL expr '\n' tellstart stmtlist lbl tENDTELL  */
#line 513 "engines/director/lingo/lingo-gr.y"
                                                          { g_lingo->code1(LC::c_telldone); }
#line 2495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* stmt: tTELL expr tTO tellstart stmtoneliner lbl  */
#line 514 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->code1(LC::c_telldone); }
#line 2501 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* stmt: tASSERTERROR asserterrorstart stmtoneliner  */
#line 515 "engines/director/lingo/lingo-gr.y"
                                                     { g_lingo->code1(LC::c_asserterrordone); }
#line 2507 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* stmt: error '\n'  */
#line 516 "engines/director/lingo/lingo-gr.y"
                                        { yyerrok; }
#line 2513 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* startrepeat: %empty  */
#line 518 "engines/director/lingo/lingo-gr.y"
                                { startRepeat(); }
#line 2519 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* tellstart: %empty  */
#line 520 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_tell); }
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* asserterrorstart: %empty  */
#line 522 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_asserterror); }
#line 2531 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* ifstmt: if expr jumpifz tTHEN stmtlist jump elseifstmtlist lbl tENDIF  */
#line 524 "engines/director/lingo/lingo-gr.y"
                                                                                         {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2543 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* ifstmt: if expr jumpifz tTHEN stmtlist jump elseifstmtlist tELSE stmtlist lbl tENDIF  */
#line 531 "engines/director/lingo/lingo-gr.y"
                                                                                                          {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].code) + 1 - (yyvsp[-8].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-8].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-5].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-5].code), (yyvsp[-1].code)); }
#line 2555 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* elseifstmt: tELSIF expr jumpifz tTHEN stmtlist jump  */
#line 542 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2565 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* jumpifz: %empty  */
#line 548 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2573 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* jump: %empty  */
#line 552 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2581 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* varassign: %empty  */
#line 556 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2589 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* if: tIF  */
#line 560 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2596 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* lbl: %empty  */
#line 563 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2602 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* simpleexprnoparens: INT  */
#line 571 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2610 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* simpleexprnoparens: FLOAT  */
#line 574 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* simpleexprnoparens: SYMBOL  */
#line 577 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2627 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* simpleexprnoparens: STRING  */
#line 581 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* simpleexprnoparens: '+' simpleexpr  */
#line 585 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[0].code); }
#line 2642 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* simpleexprnoparens: '-' simpleexpr  */
#line 586 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2648 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* simpleexprnoparens: tNOT simpleexpr  */
#line 587 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_not); }
#line 2654 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* simpleexprnoparens: THEENTITY  */
#line 589 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2667 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* simpleexprnoparens: THEENTITYWITHID simpleexpr  */
#line 597 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2678 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* simpleexprnoparens: THEFUNCINOF simpleexpr  */
#line 603 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2686 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* simpleexprnoparens: THEFUNC tOF simpleexpr  */
#line 606 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2694 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* simpleexprnoparens: THEOBJECTPROP  */
#line 609 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_objectproppush);
		g_lingo->codeString((yyvsp[0].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectprop).prop->c_str());
		delete (yyvsp[0].objectprop).obj;
		delete (yyvsp[0].objectprop).prop; }
#line 2705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* simpleexprnoparens: tSPRITE expr tINTERSECTS simpleexpr  */
#line 615 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_intersects); }
#line 2711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* simpleexprnoparens: tSPRITE expr tWITHIN simpleexpr  */
#line 616 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 2717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* simpleexprnoparens: ID '(' ID ')'  */
#line 618 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_varpush);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeFunc((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* $@10: %empty  */
#line 624 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_varpush); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 2734 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* simpleexprnoparens: ID '(' ID ',' $@10 nonemptyarglist ')'  */
#line 625 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeFunc((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 2743 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* simpleexprnoparens: ID '(' arglist ')'  */
#line 629 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2751 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* simpleexprnoparens: ID  */
#line 632 "engines/director/lingo/lingo-gr.y"
                                                        {
		if (g_lingo->_builtinConsts.contains(*(yyvsp[0].s))) {
			(yyval.code) = g_lingo->code1(LC::c_constpush);
		} else {
			(yyval.code) = g_lingo->code1(LC::c_eval);
		}
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2764 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* simpleexpr: '(' expr ')'  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2770 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* expr: simpleexpr  */
#line 644 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2776 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* expr: expr '+' expr  */
#line 645 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2782 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* expr: expr '-' expr  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2788 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* expr: expr '*' expr  */
#line 647 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2794 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* expr: expr '/' expr  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2800 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* expr: expr tMOD expr  */
#line 649 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2806 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* expr: expr '>' expr  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2812 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* expr: expr '<' expr  */
#line 651 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* expr: expr tEQ expr  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2824 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* expr: expr tNEQ expr  */
#line 653 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2830 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102: /* expr: expr tGE expr  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2836 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* expr: expr tLE expr  */
#line 655 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2842 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* expr: expr tAND expr  */
#line 656 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2848 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105: /* expr: expr tOR expr  */
#line 657 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2854 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* expr: expr '&' expr  */
#line 658 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2860 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107: /* expr: expr tCONCAT expr  */
#line 659 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2866 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* expr: expr tCONTAINS expr  */
#line 660 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2872 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* expr: expr tSTARTS expr  */
#line 661 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* chunkexpr: tFIELD simpleexprnoparens  */
#line 663 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String field("field");
		g_lingo->codeFunc(&field, 1); }
#line 2886 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* chunkexpr: tCAST simpleexprnoparens  */
#line 666 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String cast("cast");
		g_lingo->codeFunc(&cast, 1); }
#line 2894 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* chunkexpr: tCHAR expr tOF simpleexpr  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_charOf); }
#line 2900 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* chunkexpr: tCHAR expr tTO expr tOF simpleexpr  */
#line 670 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charToOf); }
#line 2906 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* chunkexpr: tITEM expr tOF simpleexpr  */
#line 671 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_itemOf); }
#line 2912 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* chunkexpr: tITEM expr tTO expr tOF simpleexpr  */
#line 672 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemToOf); }
#line 2918 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* chunkexpr: tLINE expr tOF simpleexpr  */
#line 673 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_lineOf); }
#line 2924 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* chunkexpr: tLINE expr tTO expr tOF simpleexpr  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineToOf); }
#line 2930 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* chunkexpr: tWORD expr tOF simpleexpr  */
#line 675 "engines/director/lingo/lingo-gr.y"
                                                                { g_lingo->code1(LC::c_wordOf); }
#line 2936 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* chunkexpr: tWORD expr tTO expr tOF simpleexpr  */
#line 676 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordToOf); }
#line 2942 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* reference: tSCRIPT simpleexprnoparens  */
#line 679 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String script("script");
		g_lingo->codeFunc(&script, 1); }
#line 2950 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* reference: tWINDOW simpleexprnoparens  */
#line 682 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String window("window");
		g_lingo->codeFunc(&window, 1); }
#line 2958 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proc: tPUT expr  */
#line 686 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String put("put");
		g_lingo->codeCmd(&put, 1); }
#line 2966 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* proc: tEXIT tREPEAT  */
#line 691 "engines/director/lingo/lingo-gr.y"
                                                {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->exits.push_back(pos);
		} else {
			warning("# LINGO: exit repeat not inside repeat block");
		} }
#line 2979 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* proc: tEXIT  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 2985 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* $@11: %empty  */
#line 700 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 2991 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* proc: tGLOBAL $@11 globallist  */
#line 700 "engines/director/lingo/lingo-gr.y"
                                                                                 { inLast(); }
#line 2997 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* $@12: %empty  */
#line 701 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3003 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* proc: tPROPERTY $@12 propertylist  */
#line 701 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 3009 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* $@13: %empty  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3015 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* proc: tINSTANCE $@13 instancelist  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 3021 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* proc: tOPEN expr tWITH expr  */
#line 703 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String open("open");
		g_lingo->codeCmd(&open, 2); }
#line 3029 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* proc: tOPEN expr  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String open("open");
		g_lingo->codeCmd(&open, 1); }
#line 3037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* proc: ID '(' ID ')'  */
#line 709 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_varpush);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeCmd((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 3048 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* $@14: %empty  */
#line 715 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_varpush); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 3054 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* proc: ID '(' ID ',' $@14 nonemptyarglist ')'  */
#line 716 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeCmd((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 3063 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* proc: ID '(' arglist ')'  */
#line 720 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeCmd((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3071 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* proc: ID arglist  */
#line 723 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeCmd((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* globallist: ID  */
#line 728 "engines/director/lingo/lingo-gr.y"
                                                        {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3087 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* globallist: ID ',' globallist  */
#line 731 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[-2].s), kVarGlobal);
		delete (yyvsp[-2].s); }
#line 3095 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* propertylist: ID  */
#line 736 "engines/director/lingo/lingo-gr.y"
                                                        {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3103 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* propertylist: ID ',' propertylist  */
#line 739 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[-2].s), kVarProperty);
		delete (yyvsp[-2].s); }
#line 3111 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* instancelist: ID  */
#line 744 "engines/director/lingo/lingo-gr.y"
                                                        {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3119 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* instancelist: ID ',' instancelist  */
#line 747 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[-2].s), kVarInstance);
		delete (yyvsp[-2].s); }
#line 3127 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* gotofunc: tGOLOOP  */
#line 758 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* gotofunc: tGO tNEXT  */
#line 759 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3139 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* gotofunc: tGO tPREVIOUS  */
#line 760 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3145 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* gotofunc: tGO expr  */
#line 761 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3154 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* gotofunc: tGO expr gotomovie  */
#line 765 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3163 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* gotofunc: tGO gotomovie  */
#line 769 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* playfunc: tPLAY expr  */
#line 777 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3181 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* playfunc: tPLAY expr gotomovie  */
#line 781 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* playfunc: tPLAY gotomovie  */
#line 785 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3199 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* $@15: %empty  */
#line 789 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* playfunc: tPLAYACCEL $@15 arglist  */
#line 789 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeCmd((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3214 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* $@16: %empty  */
#line 819 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* defn: tMACRO $@16 ID lbl argdef '\n' argstore stmtlist  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3230 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* defn: tFACTORY ID  */
#line 825 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3236 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* $@17: %empty  */
#line 826 "engines/director/lingo/lingo-gr.y"
                  {
			startDef();
			Common::String me("me");
			g_lingo->codeArg(&me);
			mVar(&me, kVarArgument);
		}
#line 3247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* defn: tMETHOD $@17 lbl argdef '\n' argstore stmtlist  */
#line 831 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->code1(LC::c_procret);
			g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1);
			endDef();
			delete (yyvsp[-6].s); }
#line 3257 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* defn: on lbl argdef '\n' argstore stmtlist ENDCLAUSE endargdef  */
#line 836 "engines/director/lingo/lingo-gr.y"
                                                                   {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* defn: on lbl argdef '\n' argstore stmtlist  */
#line 844 "engines/director/lingo/lingo-gr.y"
                                               {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3280 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* $@18: %empty  */
#line 850 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3286 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* on: tON $@18 ID  */
#line 850 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = (yyvsp[0].s); }
#line 3292 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* argname: ID  */
#line 852 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); delete (yyvsp[0].s); }
#line 3298 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* argdef: %empty  */
#line 854 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* argdef: argname  */
#line 855 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3310 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* argdef: argname ',' argdef  */
#line 856 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[0].narg) + 1; }
#line 3316 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* endargdef: ID  */
#line 859 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* endargdef: endargdef ',' ID  */
#line 860 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3328 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* argstore: %empty  */
#line 862 "engines/director/lingo/lingo-gr.y"
                                        { inDef(); }
#line 3334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* arglist: %empty  */
#line 864 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3340 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* arglist: expr  */
#line 865 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3346 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* arglist: expr ',' arglist  */
#line 866 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[0].narg) + 1; }
#line 3352 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* nonemptyarglist: expr  */
#line 868 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* nonemptyarglist: expr ','  */
#line 869 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3364 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* nonemptyarglist: expr ',' nonemptyarglist  */
#line 870 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[0].narg) + 1; }
#line 3370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* list: '[' valuelist ']'  */
#line 872 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3376 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* valuelist: %empty  */
#line 874 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3382 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* valuelist: ':'  */
#line 875 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189: /* valuelist: proplist  */
#line 876 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3394 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* valuelist: linearlist  */
#line 877 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3400 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 191: /* linearlist: expr  */
#line 879 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3406 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* linearlist: linearlist ',' expr  */
#line 880 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3412 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* proplist: proppair  */
#line 882 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 194: /* proplist: proplist ',' proppair  */
#line 883 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3424 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* proppair: SYMBOL ':' expr  */
#line 885 "engines/director/lingo/lingo-gr.y"
                          {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3433 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* proppair: STRING ':' expr  */
#line 889 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3442 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* proppair: ID ':' expr  */
#line 893 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3451 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3455 "engines/director/lingo/lingo-gr.cpp"

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

#line 899 "engines/director/lingo/lingo-gr.y"


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
