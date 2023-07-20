/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 53 "engines/director/lingo/lingo-gr.y"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/endian.h"
#include "common/hash-str.h"
#include "common/rect.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-the.h"

extern int yylex();
extern int yyparse();

using namespace Director;

static void yyerror(const char *s) {
	LingoCompiler *compiler = g_lingo->_compiler;
	compiler->_hadError = true;
	warning("######################  LINGO: %s at line %d col %d in %s id: %d",
		s, compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
		compiler->_assemblyContext->_id);
	if (compiler->_lines[2] != compiler->_lines[1])
		warning("# %3d: %s", compiler->_linenumber - 2, Common::String(compiler->_lines[2], compiler->_lines[1] - 1).c_str());

	if (compiler->_lines[1] != compiler->_lines[0])
		warning("# %3d: %s", compiler->_linenumber - 1, Common::String(compiler->_lines[1], compiler->_lines[0] - 1).c_str());

	const char *ptr = compiler->_lines[0];

	while (*ptr && *ptr != '\n')
		ptr++;

	warning("# %3d: %s", compiler->_linenumber, Common::String(compiler->_lines[0], ptr).c_str());

	Common::String arrow;
	for (uint i = 0; i < compiler->_colnumber; i++)
		arrow += ' ';

	warning("#      %s^ about here", arrow.c_str());
}

static void checkEnd(Common::String *token, Common::String *expect, bool required) {
	if (required) {
		if (token->compareToIgnoreCase(*expect)) {
			Common::String err = Common::String::format("end mismatch. Expected %s but got %s", expect->c_str(), token->c_str());
			yyerror(err.c_str());
		}
	}
}


#line 129 "engines/director/lingo/lingo-gr.cpp"

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
  YYSYMBOL_tUNARY = 3,                     /* tUNARY  */
  YYSYMBOL_tINT = 4,                       /* tINT  */
  YYSYMBOL_tFLOAT = 5,                     /* tFLOAT  */
  YYSYMBOL_tVARID = 6,                     /* tVARID  */
  YYSYMBOL_tSTRING = 7,                    /* tSTRING  */
  YYSYMBOL_tSYMBOL = 8,                    /* tSYMBOL  */
  YYSYMBOL_tENDCLAUSE = 9,                 /* tENDCLAUSE  */
  YYSYMBOL_tCAST = 10,                     /* tCAST  */
  YYSYMBOL_tFIELD = 11,                    /* tFIELD  */
  YYSYMBOL_tSCRIPT = 12,                   /* tSCRIPT  */
  YYSYMBOL_tWINDOW = 13,                   /* tWINDOW  */
  YYSYMBOL_tDELETE = 14,                   /* tDELETE  */
  YYSYMBOL_tDOWN = 15,                     /* tDOWN  */
  YYSYMBOL_tELSE = 16,                     /* tELSE  */
  YYSYMBOL_tEXIT = 17,                     /* tEXIT  */
  YYSYMBOL_tFRAME = 18,                    /* tFRAME  */
  YYSYMBOL_tGLOBAL = 19,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 20,                       /* tGO  */
  YYSYMBOL_tHILITE = 21,                   /* tHILITE  */
  YYSYMBOL_tIF = 22,                       /* tIF  */
  YYSYMBOL_tIN = 23,                       /* tIN  */
  YYSYMBOL_tINTO = 24,                     /* tINTO  */
  YYSYMBOL_tMACRO = 25,                    /* tMACRO  */
  YYSYMBOL_tMOVIE = 26,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 27,                     /* tNEXT  */
  YYSYMBOL_tOF = 28,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 29,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 30,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 31,                   /* tREPEAT  */
  YYSYMBOL_tSET = 32,                      /* tSET  */
  YYSYMBOL_tTHEN = 33,                     /* tTHEN  */
  YYSYMBOL_tTO = 34,                       /* tTO  */
  YYSYMBOL_tWHEN = 35,                     /* tWHEN  */
  YYSYMBOL_tWITH = 36,                     /* tWITH  */
  YYSYMBOL_tWHILE = 37,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 38,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 39,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 40,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 41,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 42,                       /* tGE  */
  YYSYMBOL_tLE = 43,                       /* tLE  */
  YYSYMBOL_tEQ = 44,                       /* tEQ  */
  YYSYMBOL_tNEQ = 45,                      /* tNEQ  */
  YYSYMBOL_tAND = 46,                      /* tAND  */
  YYSYMBOL_tOR = 47,                       /* tOR  */
  YYSYMBOL_tNOT = 48,                      /* tNOT  */
  YYSYMBOL_tMOD = 49,                      /* tMOD  */
  YYSYMBOL_tAFTER = 50,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 51,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 52,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 53,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 54,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 55,                     /* tCHAR  */
  YYSYMBOL_tCHARS = 56,                    /* tCHARS  */
  YYSYMBOL_tITEM = 57,                     /* tITEM  */
  YYSYMBOL_tITEMS = 58,                    /* tITEMS  */
  YYSYMBOL_tLINE = 59,                     /* tLINE  */
  YYSYMBOL_tLINES = 60,                    /* tLINES  */
  YYSYMBOL_tWORD = 61,                     /* tWORD  */
  YYSYMBOL_tWORDS = 62,                    /* tWORDS  */
  YYSYMBOL_tABBREVIATED = 63,              /* tABBREVIATED  */
  YYSYMBOL_tABBREV = 64,                   /* tABBREV  */
  YYSYMBOL_tABBR = 65,                     /* tABBR  */
  YYSYMBOL_tLONG = 66,                     /* tLONG  */
  YYSYMBOL_tSHORT = 67,                    /* tSHORT  */
  YYSYMBOL_tDATE = 68,                     /* tDATE  */
  YYSYMBOL_tLAST = 69,                     /* tLAST  */
  YYSYMBOL_tMENU = 70,                     /* tMENU  */
  YYSYMBOL_tMENUS = 71,                    /* tMENUS  */
  YYSYMBOL_tMENUITEM = 72,                 /* tMENUITEM  */
  YYSYMBOL_tMENUITEMS = 73,                /* tMENUITEMS  */
  YYSYMBOL_tNUMBER = 74,                   /* tNUMBER  */
  YYSYMBOL_tTHE = 75,                      /* tTHE  */
  YYSYMBOL_tTIME = 76,                     /* tTIME  */
  YYSYMBOL_tXTRAS = 77,                    /* tXTRAS  */
  YYSYMBOL_tCASTLIBS = 78,                 /* tCASTLIBS  */
  YYSYMBOL_tSOUND = 79,                    /* tSOUND  */
  YYSYMBOL_tSPRITE = 80,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 81,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 82,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 83,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 84,                 /* tPROPERTY  */
  YYSYMBOL_tON = 85,                       /* tON  */
  YYSYMBOL_tMETHOD = 86,                   /* tMETHOD  */
  YYSYMBOL_tENDIF = 87,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 88,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 89,                  /* tENDTELL  */
  YYSYMBOL_tASSERTERROR = 90,              /* tASSERTERROR  */
  YYSYMBOL_91_ = 91,                       /* '<'  */
  YYSYMBOL_92_ = 92,                       /* '>'  */
  YYSYMBOL_93_ = 93,                       /* '&'  */
  YYSYMBOL_94_ = 94,                       /* '+'  */
  YYSYMBOL_95_ = 95,                       /* '-'  */
  YYSYMBOL_96_ = 96,                       /* '*'  */
  YYSYMBOL_97_ = 97,                       /* '/'  */
  YYSYMBOL_98_n_ = 98,                     /* '\n'  */
  YYSYMBOL_99_ = 99,                       /* ','  */
  YYSYMBOL_100_ = 100,                     /* '('  */
  YYSYMBOL_101_ = 101,                     /* ')'  */
  YYSYMBOL_102_ = 102,                     /* '['  */
  YYSYMBOL_103_ = 103,                     /* ']'  */
  YYSYMBOL_104_ = 104,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 105,                 /* $accept  */
  YYSYMBOL_script = 106,                   /* script  */
  YYSYMBOL_scriptpartlist = 107,           /* scriptpartlist  */
  YYSYMBOL_scriptpart = 108,               /* scriptpart  */
  YYSYMBOL_macro = 109,                    /* macro  */
  YYSYMBOL_factory = 110,                  /* factory  */
  YYSYMBOL_method = 111,                   /* method  */
  YYSYMBOL_methodlist = 112,               /* methodlist  */
  YYSYMBOL_nonemptymethodlist = 113,       /* nonemptymethodlist  */
  YYSYMBOL_methodlistline = 114,           /* methodlistline  */
  YYSYMBOL_handler = 115,                  /* handler  */
  YYSYMBOL_endargdef = 116,                /* endargdef  */
  YYSYMBOL_CMDID = 117,                    /* CMDID  */
  YYSYMBOL_ID = 118,                       /* ID  */
  YYSYMBOL_idlist = 119,                   /* idlist  */
  YYSYMBOL_nonemptyidlist = 120,           /* nonemptyidlist  */
  YYSYMBOL_stmt = 121,                     /* stmt  */
  YYSYMBOL_stmt_insideif = 122,            /* stmt_insideif  */
  YYSYMBOL_stmtoneliner = 123,             /* stmtoneliner  */
  YYSYMBOL_proc = 124,                     /* proc  */
  YYSYMBOL_cmdargs = 125,                  /* cmdargs  */
  YYSYMBOL_trailingcomma = 126,            /* trailingcomma  */
  YYSYMBOL_frameargs = 127,                /* frameargs  */
  YYSYMBOL_asgn = 128,                     /* asgn  */
  YYSYMBOL_to = 129,                       /* to  */
  YYSYMBOL_definevars = 130,               /* definevars  */
  YYSYMBOL_ifstmt = 131,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 132,               /* ifelsestmt  */
  YYSYMBOL_endif = 133,                    /* endif  */
  YYSYMBOL_loop = 134,                     /* loop  */
  YYSYMBOL_tell = 135,                     /* tell  */
  YYSYMBOL_when = 136,                     /* when  */
  YYSYMBOL_stmtlist = 137,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 138,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 139,             /* stmtlistline  */
  YYSYMBOL_stmtlist_insideif = 140,        /* stmtlist_insideif  */
  YYSYMBOL_nonemptystmtlist_insideif = 141, /* nonemptystmtlist_insideif  */
  YYSYMBOL_stmtlistline_insideif = 142,    /* stmtlistline_insideif  */
  YYSYMBOL_simpleexpr_nounarymath = 143,   /* simpleexpr_nounarymath  */
  YYSYMBOL_var = 144,                      /* var  */
  YYSYMBOL_varorchunk = 145,               /* varorchunk  */
  YYSYMBOL_varorthe = 146,                 /* varorthe  */
  YYSYMBOL_chunk = 147,                    /* chunk  */
  YYSYMBOL_chunktype = 148,                /* chunktype  */
  YYSYMBOL_object = 149,                   /* object  */
  YYSYMBOL_refargs = 150,                  /* refargs  */
  YYSYMBOL_the = 151,                      /* the  */
  YYSYMBOL_theobj = 152,                   /* theobj  */
  YYSYMBOL_menu = 153,                     /* menu  */
  YYSYMBOL_thedatetime = 154,              /* thedatetime  */
  YYSYMBOL_thenumberof = 155,              /* thenumberof  */
  YYSYMBOL_inof = 156,                     /* inof  */
  YYSYMBOL_writablethe = 157,              /* writablethe  */
  YYSYMBOL_writabletheobj = 158,           /* writabletheobj  */
  YYSYMBOL_list = 159,                     /* list  */
  YYSYMBOL_proplist = 160,                 /* proplist  */
  YYSYMBOL_proppair = 161,                 /* proppair  */
  YYSYMBOL_unarymath = 162,                /* unarymath  */
  YYSYMBOL_simpleexpr = 163,               /* simpleexpr  */
  YYSYMBOL_expr = 164,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 165,         /* expr_nounarymath  */
  YYSYMBOL_expr_noeq = 166,                /* expr_noeq  */
  YYSYMBOL_sprite = 167,                   /* sprite  */
  YYSYMBOL_exprlist = 168,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 169          /* nonemptyexprlist  */
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

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
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
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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
#define YYFINAL  185
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4306

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  105
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  326
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  606

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   345


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
       2,     2,     2,     2,     2,     2,     2,     2,    93,     2,
     100,   101,    96,    94,    99,    95,     2,    97,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   104,     2,
      91,     2,    92,     2,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   200,   200,   202,   208,   215,   216,   217,   218,   219,
     220,   249,   253,   255,   257,   258,   261,   267,   274,   275,
     276,   281,   285,   289,   290,   291,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   368,   369,   370,   373,   377,   387,   388,   391,
     392,   393,   394,   395,   396,   399,   400,   401,   404,   405,
     406,   407,   408,   409,   410,   411,   416,   417,   418,   419,
     420,   421,   424,   427,   432,   436,   442,   447,   450,   455,
     461,   461,   467,   472,   478,   484,   490,   498,   499,   500,
     501,   504,   504,   506,   507,   508,   511,   515,   519,   525,
     529,   533,   537,   538,   540,   542,   544,   546,   550,   554,
     558,   560,   561,   565,   571,   578,   579,   582,   583,   587,
     593,   600,   601,   607,   608,   609,   610,   611,   612,   613,
     614,   615,   616,   617,   618,   619,   622,   624,   625,   628,
     629,   632,   633,   634,   636,   638,   640,   642,   644,   646,
     648,   650,   653,   654,   655,   656,   659,   660,   663,   668,
     671,   676,   682,   683,   684,   685,   686,   689,   690,   691,
     692,   693,   696,   698,   699,   700,   701,   702,   703,   704,
     705,   706,   707,   711,   712,   713,   714,   715,   716,   717,
     718,   721,   721,   723,   724,   727,   728,   729,   730,   731,
     734,   735,   736,   742,   746,   749,   754,   755,   756,   757,
     758,   761,   762,   765,   766,   770,   771,   772,   773,   774,
     775,   776,   777,   778,   779,   780,   781,   782,   783,   784,
     785,   786,   787,   788,   795,   796,   797,   798,   799,   800,
     801,   802,   803,   804,   805,   806,   807,   808,   809,   810,
     811,   812,   813,   816,   817,   818,   819,   820,   821,   822,
     823,   824,   825,   826,   827,   828,   829,   830,   831,   832,
     833,   836,   837,   840,   841,   844,   848
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
  "end of file", "error", "invalid token", "tUNARY", "tINT", "tFLOAT",
  "tVARID", "tSTRING", "tSYMBOL", "tENDCLAUSE", "tCAST", "tFIELD",
  "tSCRIPT", "tWINDOW", "tDELETE", "tDOWN", "tELSE", "tEXIT", "tFRAME",
  "tGLOBAL", "tGO", "tHILITE", "tIF", "tIN", "tINTO", "tMACRO", "tMOVIE",
  "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO",
  "tWHEN", "tWITH", "tWHILE", "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE",
  "tGE", "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT", "tMOD", "tAFTER",
  "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR", "tCHARS", "tITEM",
  "tITEMS", "tLINE", "tLINES", "tWORD", "tWORDS", "tABBREVIATED",
  "tABBREV", "tABBR", "tLONG", "tSHORT", "tDATE", "tLAST", "tMENU",
  "tMENUS", "tMENUITEM", "tMENUITEMS", "tNUMBER", "tTHE", "tTIME",
  "tXTRAS", "tCASTLIBS", "tSOUND", "tSPRITE", "tINTERSECTS", "tWITHIN",
  "tTELL", "tPROPERTY", "tON", "tMETHOD", "tENDIF", "tENDREPEAT",
  "tENDTELL", "tASSERTERROR", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'\\n'", "','", "'('", "')'", "'['", "']'", "':'", "$accept",
  "script", "scriptpartlist", "scriptpart", "macro", "factory", "method",
  "methodlist", "nonemptymethodlist", "methodlistline", "handler",
  "endargdef", "CMDID", "ID", "idlist", "nonemptyidlist", "stmt",
  "stmt_insideif", "stmtoneliner", "proc", "cmdargs", "trailingcomma",
  "frameargs", "asgn", "to", "definevars", "ifstmt", "ifelsestmt", "endif",
  "loop", "tell", "when", "stmtlist", "nonemptystmtlist", "stmtlistline",
  "stmtlist_insideif", "nonemptystmtlist_insideif",
  "stmtlistline_insideif", "simpleexpr_nounarymath", "var", "varorchunk",
  "varorthe", "chunk", "chunktype", "object", "refargs", "the", "theobj",
  "menu", "thedatetime", "thenumberof", "inof", "writablethe",
  "writabletheobj", "list", "proplist", "proppair", "unarymath",
  "simpleexpr", "expr", "expr_nounarymath", "expr_noeq", "sprite",
  "exprlist", "nonemptyexprlist", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-524)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2734,  -524,  3234,  -524,  -524,  -524,  -524,    20,  -524,   -16,
    -524,  3234,  1946,    20,  2045,  -524,  -524,  3234,  -524,    52,
    -524,  -524,  2144,    15,  3316,  -524,   -58,  -524,  -524,  3234,
    2144,  1946,  3234,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    2045,  3234,  3234,    -4,  3561,  -524,   110,  2734,  -524,  -524,
    -524,  -524,  2144,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,   -40,  -524,  -524,
    2243,  2243,  2045,  2045,  2045,  2045,    48,    24,    29,  -524,
    -524,    34,    51,  -524,  -524,  -524,  -524,  2243,  2243,  2243,
    2243,  2045,  2045,  2342,  2045,  2045,  2045,  2045,  3398,  2045,
    2342,  2342,  1451,   756,    72,    61,    78,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,   857,  -524,    80,
    2045,  3758,  3234,    81,    83,   956,  3234,  2045,  3234,  -524,
    -524,     3,  -524,  -524,    85,    86,  1055,    88,    89,    90,
     333,    91,  3234,  -524,  -524,  -524,  -524,    95,  1154,  -524,
    3234,  1550,  -524,  -524,  -524,   587,   658,  3610,  3632,   164,
    -524,  -524,  -524,  3234,  -524,  -524,  1253,  4071,  -524,   -23,
     -13,    22,    66,    75,   164,   145,   166,  4058,  -524,  -524,
    -524,  3780,    92,    93,    96,    97,    10,   -26,    57,  -524,
    4071,    99,   104,  1649,  -524,  -524,   169,  2045,  2045,  2045,
    2045,  2045,  2045,  2045,  2045,  2045,  2045,  2045,  2045,  2045,
    2045,  2045,  2045,  2045,  2045,  -524,  -524,  3874,  -524,  -524,
    3814,  2816,   112,  -524,  -524,  3480,  3480,  3480,    13,  3887,
     185,  -524,  -524,  2045,    14,  -524,  2045,  -524,  -524,  -524,
    3561,  2898,  -524,   119,  -524,  -524,  -524,  3793,  2342,  2045,
    2342,  2045,  2342,  2045,  2342,  2045,  -524,  -524,  -524,  -524,
      37,  -524,   194,  4127,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  1352,  2441,  2342,  2342,  1748,  -524,
    2045,  2045,  2045,  2045,  -524,  2045,  2540,  -524,  -524,  2045,
    -524,   125,  2045,    44,    44,    44,    44,  4153,  4153,  -524,
     -10,    44,    44,    44,    44,   -10,    23,    23,  -524,  -524,
     125,  2045,  2045,  2045,  2045,  2045,  2045,  2045,  2045,  2045,
    2045,  2045,  2045,  2045,  2045,  2045,  2045,  2045,  2045,  -524,
    3144,   210,  2898,    48,  -524,   129,  -524,   130,   132,  2045,
    2045,  2898,  2639,  3900,  3234,  3234,  -524,  -524,  -524,    14,
    -524,  3957,  -524,  -524,  -524,   163,  2898,  -524,  2898,  1847,
    -524,  3645,  -524,  3667,  -524,  3702,  -524,  3724,  -524,  -524,
    2342,  2045,    37,    37,    37,    37,  2342,  -524,  2342,    37,
    -524,  -524,  2342,  2342,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,   125,  4071,  4071,  4071,  4071,  4071,  -524,  4071,  4071,
    2045,   159,  4071,  -524,    44,    44,    44,    44,  4153,  4153,
    -524,   -10,    44,    44,    44,    44,   -10,    23,    23,  -524,
    -524,   125,  -524,  -524,    -7,  3144,  -524,  2980,  -524,  -524,
    -524,  -524,  3970,   517,   175,  2045,  2045,  2045,  2045,  -524,
    -524,  -524,    46,  3234,  -524,  -524,   168,  -524,   255,  -524,
     125,  2342,  2342,  2342,  2342,  -524,  4071,  2342,  2342,  2342,
    2342,  -524,   237,   198,  -524,  -524,   170,  -524,  -524,  3062,
     171,  -524,  -524,  3144,  -524,  2898,   238,  2045,   181,  -524,
    4140,  -524,   162,  4140,  4140,  -524,   184,  -524,  3234,   172,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,   214,  2342,
    -524,  -524,  3144,  -524,  -524,   199,   200,  2045,  3983,  -524,
    2045,  2045,  2045,  2045,  2045,  2045,  2045,  2045,  2045,  2045,
    2045,  2045,  2045,  2045,  2045,  2045,   220,  2898,    59,  -524,
    2342,   199,  -524,   193,  4040,  2898,    69,    69,    69,  4209,
    4209,  -524,     7,    69,    69,    69,    69,     7,    27,    27,
    -524,  -524,  2045,  -524,  -524,  -524,  -524,  -524,  2898,   204,
    4140,   205,   201,   206,  -524,  -524
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    26,    23,    32,    38,    59,    68,    36,    37,     0,
      39,    92,   122,    40,     0,    41,    43,     0,    53,    54,
      56,    57,   122,    58,     0,    66,     0,    69,    67,     0,
     122,   122,    92,    30,    31,    33,    34,    44,    45,    47,
      48,    71,    72,    27,    28,    29,    49,    61,    35,    46,
      50,    51,    52,    55,    64,    65,    62,    63,    42,    70,
       0,    92,     0,     0,    60,     5,     0,     2,     3,     6,
       7,     8,   122,     9,    97,    99,   105,   106,   107,   100,
     101,   102,   103,   104,    75,    36,    74,    76,    78,    79,
      40,    80,    82,    54,    88,    58,    89,    91,    77,    85,
      86,    81,    90,    87,    84,    83,    60,     0,    73,    24,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   118,
      95,     0,    93,   173,   174,   176,   175,    32,    38,    59,
      68,    39,    53,     0,    33,    44,    47,    71,    64,    63,
       0,     0,     0,   323,   186,     0,     0,   263,   181,   182,
     183,   184,   215,   216,   185,   264,   265,   130,   266,     0,
       0,     0,    92,     0,     0,   130,     0,     0,    64,   186,
     189,     0,   190,   160,     0,     0,   130,     0,     0,     0,
       0,     0,    92,    98,   121,     1,     4,     0,   130,    10,
       0,     0,   192,   208,   191,     0,     0,     0,     0,     0,
     119,   117,   143,    94,   206,   207,   132,   133,   177,    27,
      28,    29,    49,    61,    46,    55,   212,     0,   261,   262,
     127,     0,   173,   174,   176,   175,     0,   186,     0,   253,
     325,     0,   324,     0,   110,   111,    56,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   131,   123,   284,   130,   285,   120,
       0,     0,     0,   116,   109,    43,    30,    31,     0,     0,
     243,   141,   142,     0,    14,   114,    69,   112,   113,   145,
       0,   161,   144,     0,   108,    25,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   202,   204,   205,   203,
       0,    96,    56,   136,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,     0,     0,     0,     0,     0,   180,
       0,     0,     0,     0,   251,     0,     0,   252,   250,     0,
     178,   130,     0,   276,   277,   274,   275,   278,   279,   271,
     281,   282,   283,   273,   272,   280,   267,   268,   269,   270,
     130,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   131,   125,
     167,   146,   161,    64,   187,     0,   188,     0,     0,     0,
       0,   161,     0,     0,    23,     0,    18,    19,    12,    15,
      16,     0,   158,   165,   166,     0,   162,   163,   161,     0,
     193,     0,   197,     0,   199,     0,   195,     0,   241,   242,
       0,     0,    34,    45,    48,    72,    50,   238,    51,    52,
     239,   240,    62,    63,   214,   218,   217,   213,   321,   322,
     128,   130,   259,   260,   258,   256,   257,   254,   255,   326,
     131,     0,   135,   124,   295,   296,   293,   294,   297,   298,
     290,   300,   301,   302,   292,   291,   299,   286,   287,   288,
     289,   130,   171,   172,   152,   168,   169,     0,    11,   137,
     138,   139,     0,     0,     0,    50,    51,    62,    63,   244,
     245,   140,     0,    92,    17,   115,     0,   164,    22,   210,
     130,     0,     0,     0,     0,   201,   134,     0,     0,     0,
       0,   222,     0,     0,   220,   221,     0,   179,   126,     0,
       0,   147,   170,   167,   148,   161,     0,     0,     0,   303,
     246,   304,     0,   248,   249,    20,     0,   159,    23,     0,
     194,   198,   200,   196,   233,   235,   236,   234,     0,     0,
     237,   129,   167,   150,   153,   152,     0,     0,     0,   154,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   161,     0,   211,
       0,   152,   149,     0,     0,   161,   313,   314,   312,   315,
     316,   309,   318,   319,   320,   311,   310,   317,   305,   306,
     307,   308,     0,    13,    21,   219,   151,   157,   161,     0,
     247,     0,     0,     0,   155,   156
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -524,  -524,  -524,   231,  -524,  -524,  -524,  -524,  -524,   -84,
    -524,  -380,     0,     9,   -27,  -524,     1,  -360,   -62,  -524,
      -3,  -239,   275,  -524,  -524,  -524,  -524,  -524,  -523,  -524,
    -524,  -524,  -369,  -524,   -87,  -496,  -524,  -155,  -115,   -17,
    -178,  -524,    36,  -524,  -524,   -14,  -524,    -2,  -192,  -524,
    -524,  -245,  -524,  -524,  -524,  -524,    -8,  -524,   167,    -6,
     106,   -34,  -144,  -524,  -219
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    66,    67,    68,    69,    70,   387,   388,   389,   390,
      71,   107,   108,   144,   121,   122,   394,    74,    75,    76,
     145,   255,   146,    77,   273,    78,    79,    80,   511,    81,
      82,    83,   395,   396,   397,   464,   465,   466,   147,   148,
     375,   171,   149,   300,   150,   192,   151,   424,   425,   152,
     153,   410,   172,   479,   154,   228,   229,   155,   156,   230,
     257,   520,   158,   231,   232
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      72,    73,   184,   468,   482,   179,   157,   170,   161,   509,
     463,   109,   474,   258,   331,   118,   165,   545,   369,   164,
     120,   258,   572,   384,   176,   157,   162,   175,   177,   488,
     110,   111,   258,   169,   181,   350,   379,   271,   174,   243,
     173,   120,   256,   117,   258,   304,   571,   272,   596,   159,
     256,   166,   167,   305,   180,   306,   555,   380,   189,   190,
     408,   256,   258,   307,    72,   409,   188,    72,    73,   187,
     120,   182,   243,   256,   233,   112,   555,   113,   325,   114,
     510,   115,   119,   163,   250,   251,   252,   253,   377,   378,
     308,   256,   441,   243,   183,   116,   244,   194,   309,   431,
     385,   562,   563,   564,   565,   463,   195,   196,   197,   198,
     185,   443,   386,   324,   194,   204,   205,   199,   555,   252,
     253,   556,   200,   564,   565,   206,   207,   201,   195,   196,
     197,   198,   202,   217,   310,   262,   221,   249,   250,   251,
     252,   253,   311,   312,   525,   190,   546,   216,   568,   461,
     203,   313,   227,   463,   260,   283,   326,   594,   190,   234,
     327,   269,   561,   562,   563,   564,   565,   497,   498,   499,
     500,   120,   233,   314,   503,   268,   235,   270,   259,   263,
     490,   264,   463,   274,   275,   287,   277,   278,   279,   282,
     566,   120,   506,   284,   315,   332,   320,   321,   593,   285,
     322,   323,   328,   329,   550,   551,   599,   552,   553,   554,
     372,   555,   301,   382,   556,   557,   558,   398,   392,   296,
     411,   297,   508,   298,   440,   299,   467,   469,   470,   601,
     471,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   374,   374,
     374,   529,   486,   559,   560,   561,   562,   563,   564,   565,
     507,    72,   371,   518,   528,   538,   527,   383,   539,   544,
     391,   541,   547,   569,   169,   169,   169,   193,   193,   549,
      72,    72,   567,   401,   570,   403,   510,   405,   573,   407,
     592,   597,   602,   603,   193,   193,   193,   193,   186,   604,
     208,   376,   376,   376,   605,   484,   178,   218,   219,   487,
     512,   540,   303,   427,   432,   433,   434,   435,   437,   436,
     438,     0,     0,   439,     0,     0,   442,     0,     0,     0,
       0,   521,   521,   521,   521,   227,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,     0,     0,     0,     0,     0,   280,     0,     0,
      72,     0,    72,   472,   473,   237,   238,   239,   240,   241,
     242,    72,   243,     0,     0,   244,   245,   246,     0,     0,
       0,     0,     0,   109,   483,     0,    72,     0,    72,     0,
       0,     0,     0,     0,     0,   496,   521,   521,   521,   521,
     521,   521,   521,   521,   521,   521,   521,   521,   521,   521,
     521,   521,     0,     0,   247,   248,   249,   250,   251,   252,
     253,   281,     0,     0,   439,     0,     0,     0,     0,     0,
       0,     0,   522,   523,   524,     0,     0,     0,   521,     0,
       0,     0,     0,     0,     0,   400,   526,   402,     0,   404,
       0,   406,     0,     0,     0,    72,     0,    72,   514,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   426,   426,   428,   429,     0,     0,     0,     0,     0,
       0,     0,   120,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
     543,   548,     0,    72,     0,    72,   576,   577,   578,   579,
     580,   581,   582,   583,   584,   585,   586,   587,   588,   589,
     590,   591,   516,     0,     0,     0,     0,   109,     0,     0,
       0,   574,    72,     0,     0,     0,     0,     0,     0,   480,
       0,   517,     0,     0,     0,     0,     0,     0,   600,   237,
     238,   239,   240,   241,   242,     0,   243,    72,     0,   244,
     245,   246,     0,     0,     0,    72,     0,   495,     0,     0,
       0,     0,     0,   501,     0,   502,     0,     0,     0,   504,
     505,     0,     0,     0,     0,     0,     0,     0,    72,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   247,   248,
     249,   250,   251,   252,   253,   288,     0,     0,     0,     0,
       0,   289,     0,     0,     0,     0,     0,     0,     0,   237,
     238,   239,   240,   241,   242,     0,   243,     0,     0,   244,
     245,   246,   519,   519,   519,   519,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   530,   531,
     532,   533,     0,     0,   534,   535,   536,   537,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   247,   248,
     249,   250,   251,   252,   253,     0,   290,     0,     0,     0,
       0,     0,   291,     0,     0,     0,     0,     0,     0,     0,
     237,   238,   239,   240,   241,   242,   501,   243,     0,     0,
     244,   245,   246,     0,     0,     0,     0,   519,   519,   519,
     519,   519,   519,   519,   519,   519,   519,   519,   519,   519,
     519,   519,   519,     0,     0,     0,     0,   595,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   247,
     248,   249,   250,   251,   252,   253,     0,     0,     0,   519,
     222,   223,     1,   224,   225,    84,   127,   128,   129,   130,
      85,     8,    86,    87,    10,    88,    89,    90,    91,    15,
      16,    92,    18,    93,    20,    21,    94,    95,    96,    97,
      25,     0,    27,    28,    98,    99,   100,   101,     0,     0,
       0,     0,     0,     0,   133,     0,    33,    34,     0,     0,
       0,   134,    36,   135,    38,   136,    40,   137,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,   138,    55,     0,     0,    56,   139,    58,    59,   102,
     103,   104,   105,     0,     0,     0,   106,     0,     0,     0,
     140,   141,     0,     0,     0,     0,   160,     0,   143,     0,
     226,   123,   124,     1,   125,   126,    84,   127,   128,   129,
     130,    85,     8,    86,    87,    10,    88,    89,    90,    91,
      15,    16,    92,    18,    93,   236,    21,    94,    95,    96,
      97,    25,     0,    27,    28,    98,    99,   100,   101,   237,
     238,   239,   240,   241,   242,   133,   243,    33,    34,   244,
     245,   246,   134,    36,   135,    38,   136,    40,   137,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,   138,    55,     0,     0,    56,   139,    58,    59,
     102,   103,   104,   105,     0,     0,     0,   106,   247,   248,
     249,   250,   251,   252,   253,     0,   254,   160,     0,   143,
     123,   124,     1,   125,   126,    84,   127,   128,   129,   130,
      85,     8,    86,    87,    10,    88,    89,    90,    91,    15,
     265,    92,    18,    93,    20,    21,    94,    95,    96,    97,
      25,     0,    27,    28,    98,    99,   100,   101,   237,   238,
     239,   240,   241,   242,   133,   243,   266,   267,   244,   245,
     246,   134,    36,   135,    38,   136,    40,   137,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,   138,    55,     0,     0,    56,   139,    58,    59,   102,
     103,   104,   105,     0,     0,     0,   106,   247,   248,   249,
     250,   251,   252,   253,     0,   254,   160,     0,   143,   123,
     124,     1,   125,   126,    84,   127,   128,   129,   130,    85,
       8,    86,    87,    10,    88,    89,    90,    91,    15,    16,
      92,    18,    93,    20,    21,    94,    95,    96,    97,    25,
       0,   276,    28,    98,    99,   100,   101,   237,   238,   239,
     240,   241,   242,   133,   243,    33,    34,   244,   245,   246,
     134,    36,   135,    38,   136,    40,   137,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
     138,    55,     0,     0,    56,   139,    58,    59,   102,   103,
     104,   105,     0,     0,     0,   106,   247,   248,   249,   250,
     251,   252,   253,     0,   254,   160,     0,   143,   123,   124,
       1,   125,   126,    84,   127,   128,   129,   130,    85,     8,
      86,    87,    10,    88,    89,    90,    91,    15,    16,    92,
      18,    93,    20,    21,    94,    95,    96,    97,    25,     0,
      27,    28,    98,    99,   100,   101,   237,   238,   239,   240,
     241,   242,   133,   243,    33,    34,   244,   245,   246,   134,
      36,   135,    38,   136,    40,   137,    42,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,   138,
      55,     0,     0,    56,   139,    58,    59,   102,   103,   104,
     105,     0,     0,     0,   106,   247,   248,   249,   250,   251,
     252,   253,     0,   254,   160,     0,   143,   123,   124,     1,
     125,   126,    84,   127,   128,   129,   130,    85,     8,    86,
      87,    10,    88,    89,    90,    91,    15,    16,    92,    18,
      93,   302,    21,    94,    95,    96,    97,    25,     0,    27,
      28,    98,    99,   100,   101,   237,   238,   239,   240,   241,
     242,   133,   243,    33,    34,   244,   245,   246,   134,    36,
     135,    38,   136,    40,   137,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,   138,    55,
       0,     0,    56,   139,    58,    59,   102,   103,   104,   105,
       0,     0,     0,   106,   247,   248,   249,   250,   251,   252,
     253,     0,     0,   160,     0,   143,   123,   124,     1,   125,
     126,    84,   127,   128,   129,   130,    85,     8,    86,    87,
      10,    88,    89,    90,    91,    15,    16,    92,    18,    93,
      20,    21,    94,    95,    96,    97,    25,     0,    27,    28,
      98,    99,   100,   101,     0,     0,     0,     0,     0,     0,
     133,     0,    33,    34,     0,     0,     0,   134,   412,   135,
     413,   136,   414,   137,   415,    43,    44,    45,    46,    47,
      48,    49,   416,   417,   418,   419,    53,   138,    55,   420,
     421,   422,   423,    58,    59,   102,   103,   104,   105,     0,
       0,     0,   106,     0,     0,     0,   140,   141,     0,     0,
       0,     0,   160,     0,   143,   123,   124,     1,   125,   126,
      84,   127,   128,   129,   130,    85,     8,    86,    87,    10,
      88,    89,    90,    91,    15,    16,    92,    18,    93,    20,
      21,    94,    95,    96,    97,    25,     0,    27,    28,    98,
      99,   100,   101,     0,     0,     0,     0,     0,     0,   133,
       0,    33,    34,     0,     0,     0,   134,    36,   135,    38,
     136,    40,   137,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,   138,    55,     0,     0,
      56,   139,    58,    59,   102,   103,   104,   105,     0,     0,
       0,   106,     0,     0,     0,   140,   141,     0,     0,     0,
       0,   160,   220,   143,   123,   124,     1,   125,   126,    84,
     127,   128,   129,   130,    85,     8,    86,    87,    10,    88,
      89,    90,    91,    15,    16,    92,    18,    93,    20,    21,
      94,    95,    96,    97,    25,     0,    27,    28,    98,    99,
     100,   101,     0,     0,     0,     0,     0,     0,   133,     0,
      33,    34,     0,     0,     0,   134,    36,   135,    38,   136,
      40,   137,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,   138,    55,     0,     0,    56,
     139,    58,    59,   102,   103,   104,   105,     0,     0,     0,
     106,     0,     0,     0,   140,   141,     0,     0,     0,     0,
     160,   286,   143,   123,   124,     1,   125,   126,    84,   127,
     128,   129,   130,    85,     8,    86,    87,    10,    88,    89,
      90,    91,    15,    16,    92,    18,    93,    20,    21,    94,
      95,    96,    97,    25,     0,    27,    28,    98,    99,   100,
     101,     0,     0,     0,     0,     0,     0,   133,     0,    33,
      34,     0,     0,     0,   134,    36,   135,    38,   136,    40,
     137,    42,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,   138,    55,     0,     0,    56,   139,
      58,    59,   102,   103,   104,   105,     0,     0,     0,   106,
       0,     0,     0,   140,   141,     0,     0,     0,     0,   160,
     330,   143,   123,   124,     1,   125,   126,    84,   127,   128,
     129,   130,    85,     8,    86,    87,    10,    88,    89,    90,
      91,    15,    16,    92,    18,    93,    20,    21,    94,    95,
      96,    97,    25,     0,    27,    28,    98,    99,   100,   101,
       0,     0,     0,     0,     0,     0,   133,     0,    33,    34,
       0,     0,     0,   134,    36,   135,    38,   136,    40,   137,
      42,    43,    44,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,   138,    55,     0,     0,    56,   139,    58,
      59,   102,   103,   104,   105,     0,     0,     0,   106,     0,
       0,     0,   140,   141,     0,     0,     0,     0,   160,   430,
     143,   123,   124,     1,   125,   126,    84,   127,   128,   129,
     130,    85,     8,    86,    87,    10,    88,    89,    90,    91,
      15,    16,    92,    18,    93,    20,    21,    94,    95,    96,
      97,    25,     0,    27,    28,    98,    99,   100,   101,     0,
       0,     0,     0,     0,     0,   133,     0,    33,    34,     0,
       0,     0,   134,    36,   135,    38,   136,    40,   137,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,   138,    55,     0,     0,    56,   139,    58,    59,
     102,   103,   104,   105,     0,     0,     0,   106,     0,     0,
       0,   140,   141,     0,     0,     0,     0,   160,   489,   143,
     123,   124,     1,   125,   126,    84,   127,   128,   129,   130,
      85,     8,    86,    87,   131,    88,    89,    90,    91,    15,
      16,    92,   132,    93,    20,    21,    94,    95,    96,    97,
      25,     0,    27,    28,    98,    99,   100,   101,     0,     0,
       0,     0,     0,     0,   133,     0,    33,    34,     0,     0,
       0,   134,    36,   135,    38,   136,    40,   137,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,   138,    55,     0,     0,    56,   139,    58,    59,   102,
     103,   104,   105,     0,     0,     0,   106,     0,     0,     0,
     140,   141,     0,     0,     0,     0,   142,     0,   143,   123,
     124,     1,   125,   126,    84,   127,   128,   129,   130,    85,
       8,    86,    87,    10,    88,    89,    90,    91,    15,    16,
      92,    18,    93,    20,    21,    94,    95,    96,    97,    25,
       0,    27,    28,    98,    99,   100,   101,     0,     0,     0,
       0,     0,     0,   133,     0,    33,    34,     0,     0,     0,
     134,    36,   135,    38,   136,    40,   137,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
     138,    55,     0,     0,    56,   139,    58,    59,   102,   103,
     104,   105,     0,     0,     0,   106,     0,     0,     0,   140,
     141,     0,     0,     0,     0,   160,     0,   143,   123,   124,
       1,   125,   126,    84,   127,   128,   129,   130,    85,     8,
      86,    87,    10,    88,    89,    90,    91,    15,    16,    92,
      18,    93,    20,    21,    94,    95,    96,    97,    25,     0,
      27,    28,    98,    99,   100,   101,     0,     0,     0,     0,
       0,     0,   133,     0,    33,    34,     0,     0,     0,   134,
      36,   135,    38,   136,    40,   137,    42,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,   138,
      55,     0,     0,    56,   139,    58,    59,   102,   103,   104,
     105,     0,     0,     0,   106,     0,     0,     0,   140,   141,
       0,     0,     0,     0,   142,     0,   143,   123,   124,     1,
     125,   126,    84,   127,   128,   129,   130,    85,     8,    86,
      87,    10,    88,    89,    90,    91,    15,    16,    92,    18,
      93,    20,    21,    94,    95,    96,    97,    25,     0,    27,
      28,    98,    99,   100,   101,     0,     0,     0,     0,     0,
       0,   133,     0,    33,    34,     0,     0,     0,   134,    36,
     135,    38,   136,    40,   137,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,   138,    55,
       0,     0,    56,    57,    58,    59,   102,   103,   104,   105,
       0,     0,     0,   106,     0,     0,     0,   140,   141,     0,
       0,     0,     0,   191,     0,   143,   123,   124,     1,   125,
     126,    84,   127,   128,   129,   130,    85,     8,    86,    87,
      10,    88,    89,    90,    91,    15,    16,    92,    18,    93,
      20,    21,    94,    95,    96,    97,    25,     0,    27,    28,
      98,    99,   100,   101,     0,     0,     0,     0,     0,     0,
     133,     0,    33,    34,     0,     0,     0,   134,    36,   135,
      38,   136,    40,   137,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,   138,    55,     0,
       0,    56,    57,    58,    59,   102,   103,   104,   105,     0,
       0,     0,   106,     0,     0,     0,   140,   141,     0,     0,
       0,     0,   160,     0,   143,   123,   124,     1,   125,   126,
      84,   127,   128,   129,   130,    85,     8,    86,    87,    10,
      88,    89,    90,    91,    15,    16,    92,    18,    93,    20,
      21,    94,    95,    96,    97,    25,     0,    27,    28,    98,
      99,   100,   101,     0,     0,     0,     0,     0,     0,   133,
       0,    33,    34,     0,     0,     0,   134,    36,   135,    38,
     136,    40,   137,    42,    43,    44,    45,    46,    47,    48,
      49,   416,     0,   418,    52,    53,   138,    55,     0,     0,
     422,   423,    58,    59,   102,   103,   104,   105,     0,     0,
       0,   106,     0,     0,     0,   140,   141,     0,     0,     0,
       0,   160,     0,   143,   222,   223,     1,   224,   225,    84,
     127,   128,   129,   130,    85,     8,    86,    87,    10,    88,
      89,    90,    91,    15,    16,    92,    18,    93,    20,    21,
      94,    95,    96,    97,    25,     0,    27,    28,    98,    99,
     100,   101,     0,     0,     0,     0,     0,     0,   133,     0,
      33,    34,     0,     0,     0,   134,    36,   135,    38,   136,
      40,   137,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,   138,    55,     0,     0,    56,
     139,    58,    59,   102,   103,   104,   105,     0,     0,     0,
     106,     0,     0,     0,   140,   141,     0,     0,     0,     0,
     160,     0,   143,   123,   124,     1,   125,   126,    84,   127,
     128,   129,   130,    85,     8,    86,    87,    10,    88,    89,
      90,    91,    15,    16,    92,    18,    93,    20,    21,    94,
      95,    96,    97,    25,     0,    27,    28,    98,    99,   100,
     101,     0,     0,     0,     0,     0,     0,   133,     0,    33,
      34,     0,     0,     0,   134,    36,   135,    38,   136,    40,
     137,    42,    43,    44,    45,    46,    47,    48,    49,   475,
       0,   476,    52,    53,   138,    55,     0,     0,   477,   478,
      58,    59,   102,   103,   104,   105,     0,     0,     0,   106,
       0,     0,     0,   140,   141,     0,     0,     0,     0,   160,
       1,   143,     0,     2,     3,     4,     5,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,     0,    25,    26,
      27,    28,    29,    30,    31,    32,     0,     0,     0,     0,
       0,     0,     0,     0,    33,    34,     0,     0,     0,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,    54,
      55,     0,     0,    56,    57,    58,    59,    60,    61,    62,
       0,    63,     1,     0,    64,     0,     3,     4,     5,     6,
       7,     8,    65,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    18,    19,    20,    21,    22,    23,    24,     0,
      25,    26,    27,    28,     0,    30,    31,    32,     0,     0,
       0,     0,     0,     0,     0,     0,    33,    34,     0,     0,
       0,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,    54,    55,     0,     0,    56,    57,    58,    59,    60,
      61,     0,     0,    63,     1,     0,    64,     0,     3,     4,
       5,     6,     7,     8,   370,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,    28,     0,    30,    31,    32,
       0,     0,     0,     0,     0,     0,     0,     0,    33,    34,
       0,     0,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,    54,    55,     0,     0,    56,    57,    58,
      59,    60,    61,     0,     0,    63,     1,     0,    64,     0,
       3,     4,     5,     6,     7,     8,   393,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    18,    19,    20,    21,
      22,    23,    24,     0,    25,    26,    27,    28,     0,    30,
      31,    32,     0,     0,     0,     0,     0,     0,     0,     0,
      33,    34,     0,     0,     0,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,     0,     0,    56,
      57,    58,    59,    60,    61,     0,     0,    63,     1,     0,
      64,     0,     3,     4,     5,     6,     7,     8,   513,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    18,    19,
      20,    21,    22,    23,    24,     0,    25,    26,    27,    28,
       0,    30,    31,    32,     0,     0,     0,     0,     0,     0,
       0,     0,    33,    34,     0,     0,     0,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,     0,
       0,    56,    57,    58,    59,    60,    61,     0,     0,    63,
       1,     0,    64,     0,     3,     4,     5,     6,     7,     8,
     542,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      18,    19,    20,    21,    22,    23,    24,     0,    25,    26,
      27,    28,     0,    30,    31,    32,     0,     0,     0,     0,
       0,     0,     0,     0,    33,    34,     0,     0,     0,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,    54,
      55,     0,     0,    56,    57,    58,    59,    60,    61,     0,
       0,     0,     0,     0,    64,     0,     0,     0,     0,     0,
       1,     0,   462,    84,     3,     4,     5,     6,    85,     8,
      86,    87,    10,    88,    89,    90,    91,    15,    16,    92,
      18,    93,    20,    21,    94,    95,    96,    97,    25,     0,
      27,    28,    98,    99,   100,   101,     0,     0,     0,     0,
       0,     0,     0,     0,    33,    34,     0,     0,     0,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,    54,
      55,     0,     0,    56,    57,    58,    59,   102,   103,   104,
     105,     0,     1,     0,   106,    84,     3,     4,     5,     6,
      85,     8,    86,    87,    10,    88,    89,    90,    91,    15,
      16,    92,    18,    93,    20,    21,    94,    95,    96,    97,
      25,     0,    27,    28,    98,    99,   100,   101,     0,     0,
       0,     0,     0,     0,     0,     0,    33,    34,     0,     0,
       0,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,   168,    55,     0,     0,    56,    57,    58,    59,   102,
     103,   104,   105,     0,     1,     0,   106,    84,     3,     4,
       5,     6,    85,     8,    86,    87,    10,    88,    89,    90,
      91,    15,    16,    92,    18,    93,    20,    21,    94,    95,
      96,    97,    25,     0,    27,    28,    98,    99,   100,   101,
       0,     0,     0,     0,     0,     0,     0,     0,    33,    34,
       0,     0,     0,    35,    36,    37,    38,    39,    40,    41,
      42,   209,   210,   211,   212,   213,    48,   214,    50,     0,
      51,    52,   215,    54,    55,     0,     0,    56,    57,    58,
      59,   102,   103,   104,   105,     0,     1,     0,   106,    84,
     127,   128,     5,     6,    85,     8,    86,    87,    10,    88,
      89,    90,    91,    15,    16,    92,    18,    93,    20,    21,
      94,    95,    96,    97,    25,     0,    27,    28,    98,    99,
     100,   101,     0,     0,     0,     0,     0,     0,     0,     0,
      33,    34,     0,     0,     0,   134,    36,   135,    38,   136,
      40,   137,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,   373,    55,     0,     0,    56,
      57,    58,    59,   102,   103,   104,   105,     1,     0,     0,
     106,     3,     4,     5,     6,     7,     8,     0,     9,    10,
      11,    12,    13,     0,    15,    16,     0,    18,    19,    20,
      21,    22,    95,    24,     0,    25,     0,    27,    28,     0,
      30,    31,    32,     0,     0,     0,     0,     0,     0,     0,
       0,    33,    34,     0,     0,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,   292,     0,
      56,    57,    58,    59,   293,    61,     0,     0,     0,     0,
       0,    64,   237,   238,   239,   240,   241,   242,     0,   243,
     294,     0,   244,   245,   246,     0,   295,     0,     0,     0,
       0,     0,     0,   491,   237,   238,   239,   240,   241,   242,
       0,   243,     0,     0,   244,   245,   246,   237,   238,   239,
     240,   241,   242,     0,   243,   492,     0,   244,   245,   246,
       0,   247,   248,   249,   250,   251,   252,   253,     0,   237,
     238,   239,   240,   241,   242,     0,   243,     0,     0,   244,
     245,   246,     0,   247,   248,   249,   250,   251,   252,   253,
     493,     0,     0,     0,     0,     0,   247,   248,   249,   250,
     251,   252,   253,     0,   237,   238,   239,   240,   241,   242,
       0,   243,   494,     0,   244,   245,   246,     0,   247,   248,
     249,   250,   251,   252,   253,     0,   237,   238,   239,   240,
     241,   242,     0,   243,     0,     0,   244,   245,   246,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   261,     0,   247,   248,   249,   250,   251,   252,   253,
     237,   238,   239,   240,   241,   242,     0,   243,     0,     0,
     244,   245,   246,     0,     0,   247,   248,   249,   250,   251,
     252,   253,   237,   238,   239,   240,   241,   242,     0,   243,
       0,     0,   244,   245,   246,   237,   238,   239,   240,   241,
     242,     0,   243,     0,     0,   244,   245,   246,     0,   247,
     248,   249,   250,   251,   252,   253,   237,   238,   239,   240,
     241,   242,     0,   243,     0,     0,   244,   245,   246,     0,
       0,   247,   248,   249,   250,   251,   252,   253,     0,   318,
       0,   319,     0,     0,   247,   248,   249,   250,   251,   252,
     253,     0,   399,     0,   319,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   247,   248,   249,   250,   251,
     252,   253,     0,     0,     0,   319,   351,   352,   353,   354,
     355,   356,     0,   357,     0,     0,   358,   359,   360,   237,
     238,   239,   240,   241,   242,     0,   243,     0,     0,   244,
     245,   246,   237,   238,   239,   240,   241,   242,     0,   243,
       0,     0,   244,   245,   246,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   361,   362,   363,   364,   365,
     366,   367,     0,   368,     0,     0,     0,     0,   247,   248,
     249,   250,   251,   252,   253,   381,     0,     0,     0,     0,
       0,   247,   248,   249,   250,   251,   252,   253,   481,   237,
     238,   239,   240,   241,   242,     0,   243,     0,     0,   244,
     245,   246,   237,   238,   239,   240,   241,   242,     0,   243,
       0,     0,   244,   245,   246,   237,   238,   239,   240,   241,
     242,     0,   243,     0,     0,   244,   245,   246,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   247,   248,
     249,   250,   251,   252,   253,   485,     0,     0,     0,     0,
       0,   247,   248,   249,   250,   251,   252,   253,   515,     0,
       0,     0,     0,     0,   247,   248,   249,   250,   251,   252,
     253,   575,   237,   238,   239,   240,   241,   242,     0,   243,
       0,     0,   244,   245,   246,     0,     0,     0,     0,     0,
     237,   238,   239,   240,   241,   242,     0,   243,     0,     0,
     244,   245,   246,   237,   238,   239,   240,   241,   242,     0,
     243,     0,     0,   244,   245,   246,     0,     0,     0,     0,
       0,   247,   248,   249,   250,   251,   252,   253,   598,   316,
     317,     0,     0,     0,     0,     0,     0,     0,     0,   247,
     248,   249,   250,   251,   252,   253,     0,     0,     0,     0,
       0,     0,   247,   248,   249,   250,   251,   252,   253,   351,
     352,   353,   354,   355,   356,     0,   357,     0,     0,   358,
     359,   360,   550,   551,     0,   552,   553,   554,     0,   555,
       0,     0,   556,   557,   558,   237,   238,   239,   240,     0,
       0,     0,   243,     0,     0,   244,   245,   246,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   361,   362,
     363,   364,   365,   366,   367,     0,     0,     0,     0,     0,
       0,   559,   560,   561,   562,   563,   564,   565,     0,     0,
       0,     0,     0,     0,   247,   248,   249,   250,   251,   252,
     253,   550,   551,     0,   552,     0,     0,     0,   555,     0,
       0,   556,   557,   558,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     559,   560,   561,   562,   563,   564,   565
};

static const yytype_int16 yycheck[] =
{
       0,     0,    64,   372,   384,    32,    12,    24,    14,    16,
     370,     2,   381,   157,   233,    31,    22,   513,   257,    22,
      11,   165,   545,     9,    30,    31,    17,    30,    31,   398,
      10,    11,   176,    24,    61,   254,    23,    34,    29,    49,
      98,    32,   157,     7,   188,    68,   542,    44,   571,    13,
     165,    36,    37,    76,    60,    68,    49,    44,    98,    99,
      23,   176,   206,    76,    64,    28,    72,    67,    67,    72,
      61,    62,    49,   188,   100,    55,    49,    57,   104,    59,
      87,    61,    98,    31,    94,    95,    96,    97,   266,   267,
      68,   206,   331,    49,    98,    75,    52,   111,    76,   318,
      86,    94,    95,    96,    97,   465,   112,   113,   114,   115,
       0,   350,    98,   103,   128,   129,   130,    69,    49,    96,
      97,    52,    98,    96,    97,   131,   132,    98,   134,   135,
     136,   137,    98,   139,    68,   162,   142,    93,    94,    95,
      96,    97,    76,    68,    98,    99,   515,   138,   528,   368,
      99,    76,   143,   513,   160,   182,    99,    98,    99,    98,
     103,   167,    93,    94,    95,    96,    97,   412,   413,   414,
     415,   162,   100,    28,   419,   166,    98,   168,    98,    98,
     399,    98,   542,    98,    98,   191,    98,    98,    98,    98,
      28,   182,   431,    98,    28,    26,   104,   104,   567,   190,
     104,   104,   103,    99,    42,    43,   575,    45,    46,    47,
      98,    49,   203,    28,    52,    53,    54,    98,   280,    55,
      26,    57,   461,    59,    99,    61,    16,    98,    98,   598,
      98,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   265,   266,
     267,   490,    89,    91,    92,    93,    94,    95,    96,    97,
     101,   261,   261,    88,     9,    28,    98,   273,    70,    98,
     276,   101,    34,   101,   265,   266,   267,   110,   111,    98,
     280,   281,    98,   289,    70,   291,    87,   293,    88,   295,
      70,    98,    88,    88,   127,   128,   129,   130,    67,    98,
     133,   265,   266,   267,    98,   389,    31,   140,   141,   396,
     465,   503,   206,   315,   320,   321,   322,   323,   326,   325,
     326,    -1,    -1,   329,    -1,    -1,   332,    -1,    -1,    -1,
      -1,   475,   476,   477,   478,   326,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,    -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,
     370,    -1,   372,   379,   380,    42,    43,    44,    45,    46,
      47,   381,    49,    -1,    -1,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,   384,   385,    -1,   396,    -1,   398,    -1,
      -1,    -1,    -1,    -1,    -1,   411,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
     564,   565,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    98,    -1,    -1,   440,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   476,   477,   478,    -1,    -1,    -1,   592,    -1,
      -1,    -1,    -1,    -1,    -1,   288,   483,   290,    -1,   292,
      -1,   294,    -1,    -1,    -1,   465,    -1,   467,   467,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   314,   315,   316,   317,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   483,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   509,
     509,   517,    -1,   513,    -1,   515,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
     564,   565,    15,    -1,    -1,    -1,    -1,   528,    -1,    -1,
      -1,   547,   542,    -1,    -1,    -1,    -1,    -1,    -1,   382,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,   592,    42,
      43,    44,    45,    46,    47,    -1,    49,   567,    -1,    52,
      53,    54,    -1,    -1,    -1,   575,    -1,   410,    -1,    -1,
      -1,    -1,    -1,   416,    -1,   418,    -1,    -1,    -1,   422,
     423,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   598,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    28,    -1,    -1,    -1,    -1,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,   475,   476,   477,   478,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   491,   492,
     493,   494,    -1,    -1,   497,   498,   499,   500,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    45,    46,    47,   539,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    -1,    -1,   550,   551,   552,
     553,   554,   555,   556,   557,   558,   559,   560,   561,   562,
     563,   564,   565,    -1,    -1,    -1,    -1,   570,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    96,    97,    -1,    -1,    -1,   592,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      94,    95,    -1,    -1,    -1,    -1,   100,    -1,   102,    -1,
     104,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    90,    91,    92,
      93,    94,    95,    96,    97,    -1,    99,   100,    -1,   102,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    91,    92,    93,
      94,    95,    96,    97,    -1,    99,   100,    -1,   102,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    97,    -1,    99,   100,    -1,   102,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    -1,    99,   100,    -1,   102,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    -1,    -1,   100,    -1,   102,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,
      -1,    -1,   100,    -1,   102,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,   100,   101,   102,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
     100,   101,   102,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,   100,
     101,   102,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,   100,   101,
     102,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,
      -1,    94,    95,    -1,    -1,    -1,    -1,   100,   101,   102,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      94,    95,    -1,    -1,    -1,    -1,   100,    -1,   102,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,
      95,    -1,    -1,    -1,    -1,   100,    -1,   102,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,   100,    -1,   102,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,
      -1,    -1,    -1,   100,    -1,   102,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,
      -1,    -1,   100,    -1,   102,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,   100,    -1,   102,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
     100,    -1,   102,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,   100,
       6,   102,    -1,     9,    10,    11,    12,    13,    14,    15,
      -1,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      -1,    87,     6,    -1,    90,    -1,    10,    11,    12,    13,
      14,    15,    98,    17,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    -1,    79,    80,    81,    82,    83,
      84,    -1,    -1,    87,     6,    -1,    90,    -1,    10,    11,
      12,    13,    14,    15,    98,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,    35,    36,    37,    -1,    39,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    -1,    79,    80,    81,
      82,    83,    84,    -1,    -1,    87,     6,    -1,    90,    -1,
      10,    11,    12,    13,    14,    15,    98,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    -1,    79,
      80,    81,    82,    83,    84,    -1,    -1,    87,     6,    -1,
      90,    -1,    10,    11,    12,    13,    14,    15,    98,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    36,    37,
      -1,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      -1,    79,    80,    81,    82,    83,    84,    -1,    -1,    87,
       6,    -1,    90,    -1,    10,    11,    12,    13,    14,    15,
      98,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      36,    37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    -1,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,
       6,    -1,    98,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,     6,    -1,    90,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,     6,    -1,    90,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,     6,    -1,    90,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,     6,    -1,    -1,
      90,    10,    11,    12,    13,    14,    15,    -1,    17,    18,
      19,    20,    21,    -1,    23,    24,    -1,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    -1,    36,    37,    -1,
      39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    28,    -1,
      79,    80,    81,    82,    34,    84,    -1,    -1,    -1,    -1,
      -1,    90,    42,    43,    44,    45,    46,    47,    -1,    49,
      28,    -1,    52,    53,    54,    -1,    34,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    42,    43,    44,
      45,    46,    47,    -1,    49,    28,    -1,    52,    53,    54,
      -1,    91,    92,    93,    94,    95,    96,    97,    -1,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    -1,    91,    92,    93,    94,    95,    96,    97,
      28,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,
      95,    96,    97,    -1,    42,    43,    44,    45,    46,    47,
      -1,    49,    28,    -1,    52,    53,    54,    -1,    91,    92,
      93,    94,    95,    96,    97,    -1,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    33,    -1,    91,    92,    93,    94,    95,    96,    97,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    91,    92,    93,    94,    95,
      96,    97,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    91,
      92,    93,    94,    95,    96,    97,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    -1,    99,
      -1,   101,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    -1,    99,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,
      96,    97,    -1,    -1,    -1,   101,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    52,    53,    54,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,
      96,    97,    -1,    99,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    98,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    98,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    98,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    96,    97,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    42,    43,    -1,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    42,    43,    44,    45,    -1,
      -1,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    42,    43,    -1,    45,    -1,    -1,    -1,    49,    -1,
      -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     6,     9,    10,    11,    12,    13,    14,    15,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    34,    35,    36,    37,    38,
      39,    40,    41,    50,    51,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    72,    73,    74,    75,    76,    79,    80,    81,    82,
      83,    84,    85,    87,    90,    98,   106,   107,   108,   109,
     110,   115,   117,   121,   122,   123,   124,   128,   130,   131,
     132,   134,   135,   136,     9,    14,    16,    17,    19,    20,
      21,    22,    25,    27,    30,    31,    32,    33,    38,    39,
      40,    41,    83,    84,    85,    86,    90,   116,   117,   118,
      10,    11,    55,    57,    59,    61,    75,   147,    31,    98,
     118,   119,   120,     4,     5,     7,     8,    10,    11,    12,
      13,    18,    26,    48,    55,    57,    59,    61,    75,    80,
      94,    95,   100,   102,   118,   125,   127,   143,   144,   147,
     149,   151,   154,   155,   159,   162,   163,   164,   167,   147,
     100,   164,   118,    31,   125,   164,    36,    37,    75,   118,
     144,   146,   157,    98,   118,   125,   164,   125,   127,   119,
     164,   119,   118,    98,   123,     0,   108,   125,   164,    98,
      99,   100,   150,   163,   150,   164,   164,   164,   164,    69,
      98,    98,    98,    99,   150,   150,   164,   164,   163,    63,
      64,    65,    66,    67,    69,    74,   118,   164,   163,   163,
     101,   164,     4,     5,     7,     8,   104,   118,   160,   161,
     164,   168,   169,   100,    98,    98,    28,    42,    43,    44,
      45,    46,    47,    49,    52,    53,    54,    91,    92,    93,
      94,    95,    96,    97,    99,   126,   143,   165,   167,    98,
     164,    33,   119,    98,    98,    24,    50,    51,   118,   164,
     118,    34,    44,   129,    98,    98,    36,    98,    98,    98,
      34,    98,    98,   119,    98,   118,   101,   164,    28,    34,
      28,    34,    28,    34,    28,    34,    55,    57,    59,    61,
     148,   118,    28,   165,    68,    76,    68,    76,    68,    76,
      68,    76,    68,    76,    28,    28,    81,    82,    99,   101,
     104,   104,   104,   104,   103,   104,    99,   103,   103,    99,
     101,   169,    26,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     169,    42,    43,    44,    45,    46,    47,    49,    52,    53,
      54,    91,    92,    93,    94,    95,    96,    97,    99,   126,
      98,   121,    98,    75,   144,   145,   147,   145,   145,    23,
      44,    98,    28,   164,     9,    86,    98,   111,   112,   113,
     114,   164,   123,    98,   121,   137,   138,   139,    98,    99,
     163,   164,   163,   164,   163,   164,   163,   164,    23,    28,
     156,    26,    56,    58,    60,    62,    70,    71,    72,    73,
      77,    78,    79,    80,   152,   153,   163,   152,   163,   163,
     101,   169,   164,   164,   164,   164,   164,   161,   164,   164,
      99,   126,   164,   126,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   169,    98,   122,   140,   141,   142,    16,   137,    98,
      98,    98,   164,   164,   137,    70,    72,    79,    80,   158,
     163,    98,   116,   118,   114,    98,    89,   139,   137,   101,
     169,    28,    28,    28,    28,   163,   164,   156,   156,   156,
     156,   163,   163,   156,   163,   163,   126,   101,   126,    16,
      87,   133,   142,    98,   121,    98,    15,    34,    88,   163,
     166,   167,   166,   166,   166,    98,   119,    98,     9,   126,
     163,   163,   163,   163,   163,   163,   163,   163,    28,    70,
     153,   101,    98,   121,    98,   140,   137,    34,   164,    98,
      42,    43,    45,    46,    47,    49,    52,    53,    54,    91,
      92,    93,    94,    95,    96,    97,    28,    98,   116,   101,
      70,   140,   133,    88,   164,    98,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   166,    70,   137,    98,   163,   133,    98,    98,   137,
     166,   137,    88,    88,    98,    98
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   105,   106,   107,   107,   108,   108,   108,   108,   108,
     108,   109,   110,   111,   112,   112,   113,   113,   114,   114,
     114,   115,   115,   116,   116,   116,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   119,   119,   119,   120,   120,   121,   121,   122,
     122,   122,   122,   122,   122,   123,   123,   123,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   125,   125,   125,   125,   125,   125,   125,   125,
     126,   126,   127,   127,   127,   127,   127,   128,   128,   128,
     128,   129,   129,   130,   130,   130,   131,   131,   132,   132,
     132,   132,   133,   133,   134,   134,   134,   134,   135,   135,
     136,   137,   137,   138,   138,   139,   139,   140,   140,   141,
     141,   142,   142,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   144,   145,   145,   146,
     146,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   148,   148,   148,   148,   149,   149,   150,   150,
     150,   150,   151,   151,   151,   151,   151,   152,   152,   152,
     152,   152,   153,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   155,   155,   155,   155,   155,   155,   155,
     155,   156,   156,   157,   157,   158,   158,   158,   158,   158,
     159,   159,   159,   160,   160,   160,   161,   161,   161,   161,
     161,   162,   162,   163,   163,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   167,   167,   168,   168,   169,   169
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       3,     5,     4,     5,     0,     1,     1,     2,     1,     1,
       3,     8,     5,     0,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     1,     2,     1,     3,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     5,     3,     3,     2,     3,
       3,     2,     0,     2,     4,     3,     5,     2,     4,     6,
       0,     1,     2,     2,     5,     4,     3,     5,     5,     5,
       5,     1,     1,     3,     3,     3,     4,     6,     6,     8,
       7,     9,     0,     2,     7,    11,    12,     9,     4,     6,
       2,     0,     1,     1,     2,     1,     1,     0,     1,     1,
       2,     1,     1,     1,     1,     1,     1,     2,     3,     5,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     4,     6,     4,     6,     4,     6,     4,
       6,     5,     1,     1,     1,     1,     2,     2,     1,     2,
       4,     6,     2,     4,     4,     1,     1,     1,     1,     5,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     6,     6,     6,     6,     6,     4,     4,
       4,     1,     1,     2,     4,     1,     2,     5,     2,     2,
       3,     3,     3,     1,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     4,     4,     0,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


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
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
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
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_tVARID: /* tVARID  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2253 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2259 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2265 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2271 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2277 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2283 "engines/director/lingo/lingo-gr.cpp"
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
    YYNOMEM;
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
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
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
  case 2: /* script: scriptpartlist  */
#line 200 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->_compiler->_assemblyAST = new ScriptNode((yyvsp[0].nodelist)); }
#line 2553 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 202 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2564 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 208 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2574 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 215 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2580 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* scriptpart: tENDCLAUSE endargdef '\n'  */
#line 220 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2586 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 249 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2592 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* factory: tFACTORY ID '\n' methodlist  */
#line 253 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2598 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 255 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2604 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* methodlist: %empty  */
#line 257 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2610 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlistline  */
#line 261 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2621 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 267 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18: /* methodlistline: '\n'  */
#line 274 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* methodlistline: tENDCLAUSE endargdef '\n'  */
#line 276 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2643 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 281 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2652 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* handler: tON ID idlist '\n' stmtlist  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2659 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* endargdef: ID  */
#line 290 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2665 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* endargdef: endargdef ',' ID  */
#line 291 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2671 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* CMDID: tABBREVIATED  */
#line 299 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2677 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* CMDID: tABBREV  */
#line 300 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2683 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* CMDID: tABBR  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2689 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* CMDID: tAFTER  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2695 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* CMDID: tBEFORE  */
#line 303 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2701 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tCAST  */
#line 304 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2707 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tCHAR  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2713 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tCHARS  */
#line 306 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2719 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tDATE  */
#line 307 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2725 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tDELETE  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 2731 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tDOWN  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2737 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tFIELD  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2743 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tFRAME  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tHILITE  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 2755 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tIN  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2761 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tINTERSECTS  */
#line 314 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2767 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tINTO  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tITEM  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2779 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tITEMS  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2785 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tLAST  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2791 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tLINE  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2797 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tLINES  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2803 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tLONG  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2809 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tMENU  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2815 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tMENUITEM  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2821 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tMENUITEMS  */
#line 324 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2827 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tMOVIE  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2833 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tNEXT  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2839 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tNUMBER  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2845 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tOF  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2851 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tPREVIOUS  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tREPEAT  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tSCRIPT  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2869 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tASSERTERROR  */
#line 332 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2875 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tSHORT  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2881 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tSOUND  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tSPRITE  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tTHE  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 2899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tTIME  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 2905 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tTO  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tWHILE  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 2917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tWINDOW  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tWITH  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 2929 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tWITHIN  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 2935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tWORD  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 2941 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tWORDS  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 2947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* ID: tELSE  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 2953 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* ID: tENDCLAUSE  */
#line 349 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); delete (yyvsp[0].s); }
#line 2959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* ID: tEXIT  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 2965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* ID: tFACTORY  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 2971 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* ID: tGLOBAL  */
#line 352 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 2977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* ID: tGO  */
#line 353 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 2983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* ID: tIF  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 2989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tINSTANCE  */
#line 355 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 2995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tMACRO  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tMETHOD  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tON  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tOPEN  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tPLAY  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tPROPERTY  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tPUT  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tSET  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tTELL  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tTHEN  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* idlist: %empty  */
#line 368 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* nonemptyidlist: ID  */
#line 373 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 377 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3078 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* stmt: tENDIF '\n'  */
#line 388 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3084 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* proc: CMDID cmdargs '\n'  */
#line 404 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3090 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* proc: tPUT cmdargs '\n'  */
#line 405 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3096 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* proc: tGO cmdargs '\n'  */
#line 406 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3102 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* proc: tGO frameargs '\n'  */
#line 407 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3108 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* proc: tPLAY cmdargs '\n'  */
#line 408 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3114 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* proc: tPLAY frameargs '\n'  */
#line 409 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3120 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* proc: tOPEN cmdargs '\n'  */
#line 410 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3126 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* proc: tOPEN expr tWITH expr '\n'  */
#line 411 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3136 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: tNEXT tREPEAT '\n'  */
#line 416 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: tEXIT tREPEAT '\n'  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tEXIT '\n'  */
#line 418 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3154 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tDELETE chunk '\n'  */
#line 419 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3160 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tHILITE chunk '\n'  */
#line 420 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3166 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tASSERTERROR stmtoneliner  */
#line 421 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* cmdargs: %empty  */
#line 424 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3180 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* cmdargs: expr trailingcomma  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 432 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd arg, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3199 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 436 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3210 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 442 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* cmdargs: '(' ')'  */
#line 447 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* cmdargs: '(' expr ',' ')'  */
#line 450 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3238 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* frameargs: tFRAME expr  */
#line 467 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3257 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* frameargs: tMOVIE expr  */
#line 472 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3268 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 478 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3279 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* frameargs: expr tOF tMOVIE expr  */
#line 484 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3290 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* frameargs: tFRAME expr expr_nounarymath  */
#line 490 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3301 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 498 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 499 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 500 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3319 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* asgn: tSET varorthe to expr '\n'  */
#line 501 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3325 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* definevars: tGLOBAL idlist '\n'  */
#line 506 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3331 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* definevars: tPROPERTY idlist '\n'  */
#line 507 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3337 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* definevars: tINSTANCE idlist '\n'  */
#line 508 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* ifstmt: tIF expr tTHEN stmt  */
#line 511 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3352 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 515 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 3359 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 519 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 525 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 3379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 529 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 533 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 3395 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* endif: %empty  */
#line 537 "engines/director/lingo/lingo-gr.y"
                        { warning("LingoCompiler::parse: no end if"); }
#line 3401 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 540 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3408 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 542 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3415 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 544 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3422 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 546 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3429 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* tell: tTELL expr tTO stmtoneliner  */
#line 550 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3438 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 554 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* when: tWHEN '\n'  */
#line 558 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WhenNode((yyvsp[-1].w).eventName, (yyvsp[-1].w).stmt); }
#line 3451 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* stmtlist: %empty  */
#line 560 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3457 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* nonemptystmtlist: stmtlistline  */
#line 565 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3468 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 571 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3478 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* stmtlistline: '\n'  */
#line 578 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3484 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* stmtlist_insideif: %empty  */
#line 582 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 3490 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 587 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3501 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 593 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3511 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* stmtlistline_insideif: '\n'  */
#line 600 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3517 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* simpleexpr_nounarymath: tINT  */
#line 607 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3523 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* simpleexpr_nounarymath: tFLOAT  */
#line 608 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3529 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* simpleexpr_nounarymath: tSYMBOL  */
#line 609 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3535 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* simpleexpr_nounarymath: tSTRING  */
#line 610 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3541 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 611 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3547 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 612 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 3553 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 613 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 3559 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 614 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3565 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* var: ID  */
#line 622 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3571 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 191: /* chunk: tFIELD refargs  */
#line 632 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 3577 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* chunk: tCAST refargs  */
#line 633 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 3583 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* chunk: tCHAR expr tOF simpleexpr  */
#line 634 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3590 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 194: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 636 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3597 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* chunk: tWORD expr tOF simpleexpr  */
#line 638 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3604 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* chunk: tITEM expr tOF simpleexpr  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* chunk: tLINE expr tOF simpleexpr  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3632 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3639 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 3645 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* chunktype: tCHAR  */
#line 653 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 3651 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* chunktype: tWORD  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 3657 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* chunktype: tITEM  */
#line 655 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 3663 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* chunktype: tLINE  */
#line 656 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 3669 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* object: tSCRIPT refargs  */
#line 659 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 3675 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* object: tWINDOW refargs  */
#line 660 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 3681 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* refargs: simpleexpr  */
#line 663 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* refargs: '(' ')'  */
#line 668 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 3699 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* refargs: '(' expr ',' ')'  */
#line 671 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 676 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3718 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* the: tTHE ID  */
#line 682 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3724 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* the: tTHE ID tOF theobj  */
#line 683 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3730 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* the: tTHE tNUMBER tOF theobj  */
#line 684 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 3736 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 691 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3742 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* theobj: tSOUND simpleexpr  */
#line 692 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3748 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 221: /* theobj: tSPRITE simpleexpr  */
#line 693 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3754 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 222: /* menu: tMENU simpleexpr  */
#line 696 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3760 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 223: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 698 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3766 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 224: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3772 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 225: /* thedatetime: tTHE tABBREV tDATE  */
#line 700 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3778 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 226: /* thedatetime: tTHE tABBREV tTIME  */
#line 701 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3784 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* thedatetime: tTHE tABBR tDATE  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3790 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* thedatetime: tTHE tABBR tTIME  */
#line 703 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3796 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 229: /* thedatetime: tTHE tLONG tDATE  */
#line 704 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 3802 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 230: /* thedatetime: tTHE tLONG tTIME  */
#line 705 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 3808 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* thedatetime: tTHE tSHORT tDATE  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 3814 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* thedatetime: tTHE tSHORT tTIME  */
#line 707 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 3820 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 711 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 3826 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 3832 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 713 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 3838 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 714 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 3844 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 715 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 3850 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* thenumberof: tTHE tNUMBER tOF tMENUS  */
#line 716 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenus, nullptr); }
#line 3856 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* thenumberof: tTHE tNUMBER tOF tXTRAS  */
#line 717 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfXtras, nullptr); }
#line 3862 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* thenumberof: tTHE tNUMBER tOF tCASTLIBS  */
#line 718 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNumberOfNode(kNumberOfCastlibs, nullptr); }
#line 3868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* writablethe: tTHE ID  */
#line 723 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* writablethe: tTHE ID tOF writabletheobj  */
#line 724 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3880 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* writabletheobj: tMENU expr_noeq  */
#line 728 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3886 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 729 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3892 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* writabletheobj: tSOUND expr_noeq  */
#line 730 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3898 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* writabletheobj: tSPRITE expr_noeq  */
#line 731 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* list: '[' exprlist ']'  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 3910 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* list: '[' ':' ']'  */
#line 735 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 3916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* list: '[' proplist ']'  */
#line 736 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 3922 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* proplist: proppair  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 3931 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* proplist: proplist ',' proppair  */
#line 746 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* proplist: proplist ',' expr  */
#line 749 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* proppair: tSYMBOL ':' expr  */
#line 754 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3953 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* proppair: ID ':' expr  */
#line 755 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* proppair: tSTRING ':' expr  */
#line 756 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* proppair: tINT ':' expr  */
#line 757 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new IntNode((yyvsp[-2].i)), (yyvsp[0].node)); }
#line 3971 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* proppair: tFLOAT ':' expr  */
#line 758 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new FloatNode((yyvsp[-2].f)), (yyvsp[0].node)); }
#line 3977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 261: /* unarymath: '+' simpleexpr  */
#line 761 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 3983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 262: /* unarymath: '-' simpleexpr  */
#line 762 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 3989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* expr: expr '+' expr  */
#line 772 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* expr: expr '-' expr  */
#line 773 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* expr: expr '*' expr  */
#line 774 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 270: /* expr: expr '/' expr  */
#line 775 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 271: /* expr: expr tMOD expr  */
#line 776 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* expr: expr '>' expr  */
#line 777 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* expr: expr '<' expr  */
#line 778 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* expr: expr tEQ expr  */
#line 779 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* expr: expr tNEQ expr  */
#line 780 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* expr: expr tGE expr  */
#line 781 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* expr: expr tLE expr  */
#line 782 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* expr: expr tAND expr  */
#line 783 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* expr: expr tOR expr  */
#line 784 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4067 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* expr: expr '&' expr  */
#line 785 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4073 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* expr: expr tCONCAT expr  */
#line 786 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* expr: expr tCONTAINS expr  */
#line 787 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 283: /* expr: expr tSTARTS expr  */
#line 788 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 797 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4097 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 287: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 798 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4103 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 288: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 799 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4109 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 289: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 800 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4115 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 290: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 801 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4121 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 291: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 802 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4127 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 292: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 803 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 804 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4139 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 294: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 805 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4145 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 295: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 806 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4151 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 296: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 807 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4157 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 808 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4163 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 809 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4169 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 810 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4175 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 811 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4181 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 301: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 812 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4187 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 302: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 813 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4193 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 818 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4199 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 819 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 307: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4211 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 308: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 821 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4217 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 309: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4223 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 310: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 823 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4229 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 311: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 824 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4235 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 312: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 825 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4241 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 313: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 826 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 314: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 827 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4253 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 315: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 828 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 829 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 830 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4271 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 831 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 832 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4283 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 833 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4289 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 836 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 322: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 837 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4301 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 323: /* exprlist: %empty  */
#line 840 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 4307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 325: /* nonemptyexprlist: expr  */
#line 844 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4316 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 326: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 848 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4324 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 4328 "engines/director/lingo/lingo-gr.cpp"

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
          YYNOMEM;
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
  ++yynerrs;

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
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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

#line 853 "engines/director/lingo/lingo-gr.y"


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
