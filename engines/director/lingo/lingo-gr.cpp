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
#define YYFINAL  191
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4608

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  105
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  333
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  621

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
     342,   343,   344,   345,   346,   347,   348,   349,   350,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   374,   375,
     376,   379,   383,   393,   394,   397,   398,   399,   400,   401,
     402,   405,   406,   407,   410,   411,   412,   413,   414,   415,
     416,   417,   422,   423,   424,   425,   426,   427,   430,   433,
     438,   442,   448,   453,   456,   461,   465,   471,   471,   477,
     482,   488,   494,   500,   508,   509,   510,   511,   514,   514,
     516,   517,   518,   521,   525,   529,   535,   539,   543,   547,
     548,   550,   552,   554,   556,   560,   564,   568,   570,   571,
     575,   581,   588,   589,   592,   593,   597,   603,   610,   611,
     617,   618,   619,   620,   621,   622,   623,   624,   625,   626,
     627,   628,   629,   632,   634,   635,   638,   639,   642,   643,
     644,   646,   648,   650,   652,   654,   656,   658,   660,   663,
     664,   665,   666,   669,   670,   673,   678,   681,   686,   692,
     693,   694,   695,   696,   699,   700,   701,   702,   703,   706,
     708,   709,   710,   711,   712,   713,   714,   715,   716,   717,
     721,   722,   723,   724,   725,   726,   727,   728,   731,   731,
     733,   734,   737,   738,   739,   740,   741,   744,   745,   746,
     752,   756,   759,   764,   765,   766,   767,   768,   771,   772,
     775,   776,   780,   781,   782,   783,   784,   785,   786,   787,
     788,   789,   790,   791,   792,   793,   794,   795,   796,   797,
     798,   805,   806,   807,   808,   809,   810,   811,   812,   813,
     814,   815,   816,   817,   818,   819,   820,   821,   822,   823,
     826,   827,   828,   829,   830,   831,   832,   833,   834,   835,
     836,   837,   838,   839,   840,   841,   842,   843,   846,   847,
     850,   851,   854,   858
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

#define YYPACT_NINF (-533)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    3053,  -533,  3553,  -533,  -533,  -533,  -533,    32,  -533,   -19,
    -533,  3553,  1968,    32,  2067,  -533,  -533,  3553,  -533,   -17,
    -533,  -533,  2166,    58,  3635,  -533,   -75,  -533,  -533,  3553,
    2166,  1968,  3553,  -533,  -533,  -533,  -533,  -533,  -533,  -533,
    -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,
    -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,
    -533,  -533,  -533,  -533,  -533,  -533,  2067,  3553,  3553,   -64,
    3880,  -533,    46,  3053,  -533,  -533,  -533,  -533,  2166,  -533,
    -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,
    -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,
    -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,  -533,
    -533,  -533,  -533,    12,  -533,  -533,  2265,  2265,  2067,  2067,
    2067,  2067,   -37,   -43,   -40,  -533,  -533,   -10,   -38,  -533,
    -533,  -533,  -533,  2265,  2265,  2265,  2265,  2067,  2067,  2364,
    2067,  2067,  2067,  2067,  3717,  2067,  2364,  2364,  1473,   778,
      -8,    28,    42,  -533,  -533,  -533,  -533,  -533,  -533,  -533,
    -533,  -533,  -533,   879,  -533,    67,  2067,  4112,  3553,    71,
      78,   978,  3553,  2067,  3553,  -533,  -533,     3,  -533,  -533,
      82,    84,  1077,    91,    92,    96,  4055,    97,  3553,  -533,
    -533,  -533,  -533,    98,  1176,  -533,  3553,  1572,  -533,  -533,
    -533,   443,   601,   671,  3929,   109,  -533,  -533,  -533,  3553,
    -533,  -533,  1275,  4425,  -533,   -23,    29,    57,    86,    95,
     109,    53,    73,  4399,  -533,  -533,  2067,  -533,  4076,    89,
      94,   100,   102,    99,   -56,   -28,  -533,  4425,   104,   101,
    1671,  -533,  -533,   173,  2067,  2067,  2067,  2067,  2760,  2760,
    2958,  2067,  2859,  2859,  2067,  2067,  2067,  2067,  2067,  2067,
    2067,  2067,  -533,  -533,  4196,  -533,  -533,  4168,  3135,  2067,
    2067,  2067,  2067,  2067,   110,  -533,  -533,  3799,  3799,  3799,
       6,  4228,   181,  -533,  -533,  2067,    -2,  -533,  2067,  -533,
    -533,  -533,  3880,  3217,  -533,   112,  -533,  -533,  -533,  4136,
    2364,  2067,  2364,  2067,  2364,  2067,  2364,  2067,  -533,  -533,
    -533,  -533,   115,  -533,   185,  4455,  -533,  -533,  -533,  -533,
    -533,  -533,  -533,  -533,  -533,  -533,  1374,  2463,  2364,  2364,
     113,  1770,  -533,  2067,  2067,  2067,  2067,  -533,  2067,  2562,
    -533,  -533,  2067,  -533,   113,  2067,   -31,   -31,   -31,   -31,
    4481,  4481,  -533,    33,   -31,   -31,   -31,   -31,    33,    20,
      20,  -533,  -533,   113,  2067,  2067,  2067,  2067,  2067,  2067,
    2067,  2067,  2067,  2067,  2067,  2067,  2067,  2067,  2067,  2067,
    2067,  2067,  -533,  3463,   197,  3217,   -37,  -533,   116,  -533,
     117,   118,  2067,  2067,  3217,  2661,  4254,  3553,  3553,  -533,
    -533,  -533,    -2,  -533,  4285,  -533,  -533,  -533,   128,  3217,
    -533,  3217,  1869,  -533,  3949,  -533,  3962,  -533,  3985,  -533,
    4019,  -533,  -533,  2364,  2067,   115,   115,   115,   115,  2364,
    -533,  2364,   115,  -533,  -533,  2364,  2364,  -533,  -533,  -533,
    -533,  -533,  -533,  2067,   119,  -533,   113,  4425,  4425,  4425,
    4425,  4425,  -533,  4425,  4425,   120,  4425,  -533,   -31,   -31,
     -31,   -31,  4481,  4481,  -533,    33,   -31,   -31,   -31,   -31,
      33,    20,    20,  -533,  -533,   113,  -533,  -533,    -1,  3463,
    -533,  3299,  -533,  -533,  -533,  -533,  4311,   531,   131,  2067,
    2067,  2067,  2067,  -533,  -533,  -533,    25,  3553,  -533,  -533,
     124,  -533,   214,  -533,   113,  2364,  2364,  2364,  2364,  -533,
    4425,  2364,  2364,  2364,  2364,  -533,   196,   155,  -533,  -533,
    -533,   127,  -533,  -533,  3381,   133,  -533,  -533,  3463,  -533,
    3217,   195,  2067,   134,  -533,   350,  -533,  4041,   350,   350,
    -533,   135,  -533,  3553,   136,  -533,  -533,  -533,  -533,  -533,
    -533,  -533,  -533,   164,  2364,  -533,  -533,  3463,  -533,  -533,
     149,   172,  2067,  4342,  -533,  2067,  2067,  2067,  2067,  2067,
    2067,  2067,  2067,  2067,  2067,  2067,  2067,  2067,  2067,  2067,
    2067,   191,  3217,    75,  -533,  2364,   149,  -533,   178,  4368,
    3217,    51,    51,    51,  4511,  4511,  -533,    55,    51,    51,
      51,    51,    55,    60,    60,  -533,  -533,  2067,  -533,  -533,
    -533,  -533,  -533,  3217,   182,   350,   183,   179,   180,  -533,
    -533
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    26,    23,    33,    40,    64,    74,    38,    39,     0,
      41,    98,   128,    42,     0,    43,    45,     0,    56,    57,
      60,    62,   128,    63,     0,    72,     0,    75,    73,     0,
     128,   128,    98,    31,    61,    58,    55,    30,    32,    36,
      69,    34,    35,    46,    47,    49,    50,    77,    78,    27,
      28,    29,    51,    66,    37,    48,    52,    53,    54,    59,
      70,    71,    67,    68,    44,    76,     0,    98,     0,     0,
      65,     5,     0,     2,     3,     6,     7,     8,   128,     9,
     103,   105,   111,   112,   113,   106,   107,   108,   109,   110,
      81,    38,    80,    82,    84,    85,    42,    86,    88,    57,
      94,    63,    95,    97,    83,    91,    92,    87,    96,    93,
      90,    89,    65,     0,    79,    24,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   124,   101,     0,    99,   180,
     181,   183,   182,    33,    40,    64,    74,    41,    56,    58,
      34,    46,    49,    77,    70,    68,     0,     0,     0,   330,
     193,     0,     0,   270,   188,   189,   190,   191,   222,   223,
     192,   271,   272,   137,   273,     0,     0,     0,    98,     0,
       0,   137,     0,     0,    70,   193,   196,     0,   197,   167,
       0,     0,   137,     0,     0,     0,     0,     0,    98,   104,
     127,     1,     4,     0,   137,    10,     0,     0,   199,   215,
     198,     0,     0,     0,     0,     0,   125,   123,   150,   100,
     213,   214,   139,   140,   184,    27,    28,    29,    51,    66,
      48,    59,   219,     0,   268,   269,    26,   133,     0,   180,
     181,   183,   182,     0,   193,     0,   260,   332,     0,   331,
       0,   116,   117,    60,     0,     0,     0,     0,    31,    61,
      55,     0,    36,    69,     0,     0,     0,     0,     0,     0,
       0,   138,   129,   291,   137,   292,   126,     0,     0,     0,
       0,     0,     0,     0,     0,   122,   115,    45,    30,    32,
       0,     0,   250,   148,   149,     0,    14,   120,    75,   118,
     119,   152,     0,   168,   151,     0,   114,    25,   216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   209,   211,
     212,   210,     0,   102,    60,   143,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,     0,     0,     0,     0,
     137,     0,   187,     0,     0,     0,     0,   258,     0,     0,
     259,   257,     0,   185,   137,     0,   283,   284,   281,   282,
     285,   286,   278,   288,   289,   290,   280,   279,   287,   274,
     275,   276,   277,   137,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   138,   131,   174,   153,   168,    70,   194,     0,   195,
       0,     0,     0,     0,   168,     0,     0,    23,     0,    18,
      19,    12,    15,    16,     0,   165,   172,   173,     0,   169,
     170,   168,     0,   200,     0,   204,     0,   206,     0,   202,
       0,   248,   249,     0,     0,    35,    47,    50,    78,    52,
     245,    53,    54,   246,   247,    67,    68,   221,   225,   224,
     220,   328,   329,   138,     0,   134,   137,   266,   267,   265,
     263,   264,   261,   262,   333,     0,   142,   130,   302,   303,
     300,   301,   304,   305,   297,   307,   308,   309,   299,   298,
     306,   293,   294,   295,   296,   137,   178,   179,   159,   175,
     176,     0,    11,   144,   145,   146,     0,     0,     0,    52,
      53,    67,    68,   251,   252,   147,     0,    98,    17,   121,
       0,   171,    22,   217,   137,     0,     0,     0,     0,   208,
     141,     0,     0,     0,     0,   229,     0,     0,   227,   228,
     136,     0,   186,   132,     0,     0,   154,   177,   174,   155,
     168,     0,     0,     0,   310,   253,   311,     0,   255,   256,
      20,     0,   166,    23,     0,   201,   205,   207,   203,   240,
     242,   243,   241,     0,     0,   244,   135,   174,   157,   160,
     159,     0,     0,     0,   161,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   168,     0,   218,     0,   159,   156,     0,     0,
     168,   320,   321,   319,   322,   323,   316,   325,   326,   327,
     318,   317,   324,   312,   313,   314,   315,     0,    13,    21,
     226,   158,   164,   168,     0,   254,     0,     0,     0,   162,
     163
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -533,  -533,  -533,   189,  -533,  -533,  -533,  -533,  -533,  -130,
    -533,  -384,     0,     9,   -27,  -533,     1,  -373,   -66,  -533,
       5,  -245,   249,  -533,  -533,  -533,  -533,  -533,  -532,  -533,
    -533,  -533,  -355,  -533,  -128,  -506,  -533,  -196,  -104,   -22,
    -100,  -533,    -4,  -533,  -533,   -15,  -533,   -42,  -233,  -533,
    -533,  -240,  -533,  -533,  -533,  -533,   -50,  -533,   174,    -6,
      88,   -58,  -114,  -533,  -209
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    72,    73,    74,    75,    76,   400,   401,   402,   403,
      77,   113,   114,   150,   127,   128,   407,    80,    81,    82,
     151,   262,   152,    83,   285,    84,    85,    86,   526,    87,
      88,    89,   408,   409,   410,   478,   479,   480,   153,   154,
     388,   177,   155,   312,   156,   198,   157,   437,   438,   158,
     159,   423,   178,   493,   160,   235,   236,   161,   162,   237,
     264,   535,   164,   238,   239
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      78,    79,   176,   123,   190,   185,   163,   397,   167,   165,
     477,   115,   124,   496,   169,   524,   171,   330,   271,   382,
     126,   251,   560,   179,   182,   163,   168,   170,   587,   392,
     482,   344,   205,   175,   189,   181,   183,   283,   180,   488,
     187,   126,   116,   117,   240,   316,   191,   284,   338,   265,
     393,   586,   363,   317,   611,   206,   502,   265,   207,   263,
     186,   209,   256,   257,   258,   259,   260,   263,   265,   271,
      78,   339,   194,    78,    79,   340,   126,   188,   263,   125,
     265,   326,   271,   193,   398,   444,   525,   118,   208,   119,
     263,   120,   240,   121,   172,   173,   399,   318,   265,   455,
     570,   327,   200,   571,   570,   319,   477,   122,   263,   570,
     195,   196,   201,   202,   203,   204,   259,   260,   457,   200,
     210,   211,   446,   540,   196,   320,   241,   257,   258,   259,
     260,   212,   213,   321,   201,   202,   203,   204,   421,   223,
     242,   274,   228,   422,   576,   577,   578,   579,   580,   577,
     578,   579,   580,   222,   322,   477,   579,   580,   234,   583,
     267,   295,   323,   324,   308,   266,   309,   281,   310,   275,
     311,   325,   475,   609,   196,   561,   276,   126,   390,   391,
     286,   280,   287,   282,   477,   511,   512,   513,   514,   289,
     290,   299,   517,   333,   291,   294,   296,   126,   334,   345,
     342,   521,   337,   504,   335,   297,   336,   341,   385,   395,
     411,   424,   443,   481,   483,   484,   485,   500,   313,   533,
     520,   522,   542,   543,   553,   554,   405,   608,   556,   562,
     523,   559,   564,   582,   585,   614,   525,   584,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   387,   387,   387,   616,   544,
     588,   607,   192,   350,   351,   352,   354,   355,    78,   384,
     617,   618,   498,   389,   389,   389,   612,   619,   620,   396,
     184,   501,   404,   527,   555,   440,   175,   175,   175,   452,
     199,   199,    78,    78,     0,   414,     0,   416,     0,   418,
     315,   420,     0,     0,     0,     0,     0,   199,   199,   199,
     199,     0,     0,   214,     0,     0,     0,     0,     0,     0,
     224,   225,     0,     0,     0,     0,     0,   447,   448,   449,
     450,     0,   451,   453,     0,     0,   454,     0,     0,   456,
       0,     0,     0,     0,     0,     0,     0,     0,   234,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   536,   536,   536,   536,     0,
       0,     0,     0,    78,     0,    78,   486,   487,     0,     0,
       0,     0,   565,   566,    78,   567,   568,   569,     0,   570,
       0,     0,   571,   572,   573,     0,   115,   497,     0,    78,
       0,    78,     0,     0,     0,     0,     0,     0,   510,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   537,   538,   539,     0,     0,   454,     0,     0,
       0,   574,   575,   576,   577,   578,   579,   580,     0,     0,
       0,   536,   536,   536,   536,   536,   536,   536,   536,   536,
     536,   536,   536,   536,   536,   536,   536,     0,     0,     0,
     541,   300,     0,     0,   413,     0,   415,   301,   417,    78,
     419,    78,   529,     0,     0,   244,   245,   246,   247,   269,
     270,     0,   271,   536,     0,   251,   272,   273,     0,     0,
     439,   439,   441,   442,     0,     0,   126,   591,   592,   593,
     594,   595,   596,   597,   598,   599,   600,   601,   602,   603,
     604,   605,   606,     0,    78,   558,   563,     0,    78,     0,
      78,     0,     0,     0,   254,   255,   256,   257,   258,   259,
     260,     0,     0,     0,     0,     0,   531,     0,     0,   615,
       0,     0,   115,     0,     0,     0,   589,    78,     0,     0,
       0,     0,     0,     0,     0,   532,     0,     0,     0,   494,
       0,     0,     0,   244,   245,   246,   247,   269,   270,     0,
     271,     0,    78,   251,   272,   273,     0,     0,     0,     0,
      78,     0,     0,     0,     0,     0,     0,   509,     0,     0,
       0,     0,     0,   515,     0,   516,     0,     0,     0,   518,
     519,     0,     0,    78,     0,     0,     0,     0,     0,     0,
       0,     0,   254,   255,   256,   257,   258,   259,   260,   302,
       0,     0,     0,     0,     0,   303,     0,     0,     0,     0,
       0,     0,     0,   244,   245,   246,   247,   269,   270,     0,
     271,     0,     0,   251,   272,   273,     0,     0,     0,     0,
       0,     0,     0,   534,   534,   534,   534,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   545,
     546,   547,   548,     0,     0,   549,   550,   551,   552,     0,
       0,     0,   254,   255,   256,   257,   258,   259,   260,   304,
       0,     0,     0,     0,     0,   305,     0,     0,     0,     0,
       0,     0,     0,   244,   245,   246,   247,   269,   270,     0,
     271,     0,     0,   251,   272,   273,     0,     0,   515,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   534,
     534,   534,   534,   534,   534,   534,   534,   534,   534,   534,
     534,   534,   534,   534,   534,     0,     0,     0,     0,   610,
       0,     0,   254,   255,   256,   257,   258,   259,   260,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   534,   229,   230,     1,   231,   232,    90,   133,   134,
     135,   136,    91,     8,    92,    93,    10,    94,    95,    96,
      97,    15,    16,    98,    18,    99,    20,    21,   100,   101,
     102,   103,    25,     0,    27,    28,   104,   105,   106,   107,
       0,     0,     0,     0,    33,    34,   139,    36,    37,    38,
       0,    39,    40,   140,    42,   141,    44,   142,    46,   143,
      48,    49,    50,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,   144,    61,     0,     0,    62,   145,    64,
      65,   108,   109,   110,   111,     0,     0,     0,   112,     0,
       0,     0,   146,   147,     0,     0,     0,     0,   166,     0,
     149,     0,   233,   129,   130,     1,   131,   132,    90,   133,
     134,   135,   136,    91,     8,    92,    93,    10,    94,    95,
      96,    97,    15,    16,    98,    18,    99,   243,    21,   100,
     101,   102,   103,    25,     0,    27,    28,   104,   105,   106,
     107,   244,   245,   246,   247,   248,   249,   139,   250,    37,
      38,   251,   252,   253,   140,    42,   141,    44,   142,    46,
     143,    48,    49,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,   144,    61,     0,     0,    62,   145,
      64,    65,   108,   109,   110,   111,     0,     0,     0,   112,
     254,   255,   256,   257,   258,   259,   260,     0,   261,   166,
       0,   149,   129,   130,     1,   131,   132,    90,   133,   134,
     135,   136,    91,     8,    92,    93,    10,    94,    95,    96,
      97,    15,   277,    98,    18,    99,    20,    21,   100,   101,
     102,   103,    25,     0,    27,    28,   104,   105,   106,   107,
     244,   245,   246,   247,   248,   249,   139,   250,   278,   279,
     251,   252,   253,   140,    42,   141,    44,   142,    46,   143,
      48,    49,    50,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,   144,    61,     0,     0,    62,   145,    64,
      65,   108,   109,   110,   111,     0,     0,     0,   112,   254,
     255,   256,   257,   258,   259,   260,     0,   261,   166,     0,
     149,   129,   130,     1,   131,   132,    90,   133,   134,   135,
     136,    91,     8,    92,    93,    10,    94,    95,    96,    97,
      15,    16,    98,    18,    99,    20,    21,   100,   101,   102,
     103,    25,     0,   288,    28,   104,   105,   106,   107,   244,
     245,   246,   247,   248,   249,   139,   250,    37,    38,   251,
     252,   253,   140,    42,   141,    44,   142,    46,   143,    48,
      49,    50,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,   144,    61,     0,     0,    62,   145,    64,    65,
     108,   109,   110,   111,     0,     0,     0,   112,   254,   255,
     256,   257,   258,   259,   260,     0,   261,   166,     0,   149,
     129,   130,     1,   131,   132,    90,   133,   134,   135,   136,
      91,     8,    92,    93,    10,    94,    95,    96,    97,    15,
      16,    98,    18,    99,    20,    21,   100,   101,   102,   103,
      25,     0,    27,    28,   104,   105,   106,   107,   244,   245,
     246,   247,   248,   249,   139,   250,    37,    38,   251,   252,
     253,   140,    42,   141,    44,   142,    46,   143,    48,    49,
      50,    51,    52,    53,    54,    55,    56,     0,    57,    58,
      59,   144,    61,     0,     0,    62,   145,    64,    65,   108,
     109,   110,   111,     0,     0,     0,   112,   254,   255,   256,
     257,   258,   259,   260,     0,   261,   166,     0,   149,   129,
     130,     1,   131,   132,    90,   133,   134,   135,   136,    91,
       8,    92,    93,    10,    94,    95,    96,    97,    15,    16,
      98,    18,    99,   314,    21,   100,   101,   102,   103,    25,
       0,    27,    28,   104,   105,   106,   107,   244,   245,   246,
     247,   248,   249,   139,   250,    37,    38,   251,   252,   253,
     140,    42,   141,    44,   142,    46,   143,    48,    49,    50,
      51,    52,    53,    54,    55,    56,     0,    57,    58,    59,
     144,    61,     0,     0,    62,   145,    64,    65,   108,   109,
     110,   111,     0,     0,     0,   112,   254,   255,   256,   257,
     258,   259,   260,     0,     0,   166,     0,   149,   129,   130,
       1,   131,   132,    90,   133,   134,   135,   136,    91,     8,
      92,    93,    10,    94,    95,    96,    97,    15,    16,    98,
      18,    99,    20,    21,   100,   101,   102,   103,    25,     0,
      27,    28,   104,   105,   106,   107,     0,     0,     0,     0,
      33,    34,   139,    36,    37,    38,     0,    39,    40,   140,
     425,   141,   426,   142,   427,   143,   428,    49,    50,    51,
      52,    53,    54,    55,   429,   430,   431,   432,    59,   144,
      61,   433,   434,   435,   436,    64,    65,   108,   109,   110,
     111,     0,     0,     0,   112,     0,     0,     0,   146,   147,
       0,     0,     0,     0,   166,     0,   149,   129,   130,   226,
     131,   132,    90,   133,   134,   135,   136,    91,     8,    92,
      93,    10,    94,    95,    96,    97,    15,    16,    98,    18,
      99,    20,    21,   100,   101,   102,   103,    25,     0,    27,
      28,   104,   105,   106,   107,     0,     0,     0,     0,    33,
      34,   139,    36,    37,    38,     0,    39,    40,   140,    42,
     141,    44,   142,    46,   143,    48,    49,    50,    51,    52,
      53,    54,    55,    56,     0,    57,    58,    59,   144,    61,
       0,     0,    62,   145,    64,    65,   108,   109,   110,   111,
       0,     0,     0,   112,     0,     0,     0,   146,   147,     0,
       0,     0,     0,   166,   227,   149,   129,   130,     1,   131,
     132,    90,   133,   134,   135,   136,    91,     8,    92,    93,
      10,    94,    95,    96,    97,    15,    16,    98,    18,    99,
      20,    21,   100,   101,   102,   103,    25,     0,    27,    28,
     104,   105,   106,   107,     0,     0,     0,     0,    33,    34,
     139,    36,    37,    38,     0,    39,    40,   140,    42,   141,
      44,   142,    46,   143,    48,    49,    50,    51,    52,    53,
      54,    55,    56,     0,    57,    58,    59,   144,    61,     0,
       0,    62,   145,    64,    65,   108,   109,   110,   111,     0,
       0,     0,   112,     0,     0,     0,   146,   147,     0,     0,
       0,     0,   166,   298,   149,   129,   130,     1,   131,   132,
      90,   133,   134,   135,   136,    91,     8,    92,    93,    10,
      94,    95,    96,    97,    15,    16,    98,    18,    99,    20,
      21,   100,   101,   102,   103,    25,     0,    27,    28,   104,
     105,   106,   107,     0,     0,     0,     0,    33,    34,   139,
      36,    37,    38,     0,    39,    40,   140,    42,   141,    44,
     142,    46,   143,    48,    49,    50,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,   144,    61,     0,     0,
      62,   145,    64,    65,   108,   109,   110,   111,     0,     0,
       0,   112,     0,     0,     0,   146,   147,     0,     0,     0,
       0,   166,   343,   149,   129,   130,     1,   131,   132,    90,
     133,   134,   135,   136,    91,     8,    92,    93,    10,    94,
      95,    96,    97,    15,    16,    98,    18,    99,    20,    21,
     100,   101,   102,   103,    25,     0,    27,    28,   104,   105,
     106,   107,     0,     0,     0,     0,    33,    34,   139,    36,
      37,    38,     0,    39,    40,   140,    42,   141,    44,   142,
      46,   143,    48,    49,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,   144,    61,     0,     0,    62,
     145,    64,    65,   108,   109,   110,   111,     0,     0,     0,
     112,     0,     0,     0,   146,   147,     0,     0,     0,     0,
     166,   445,   149,   129,   130,     1,   131,   132,    90,   133,
     134,   135,   136,    91,     8,    92,    93,    10,    94,    95,
      96,    97,    15,    16,    98,    18,    99,    20,    21,   100,
     101,   102,   103,    25,     0,    27,    28,   104,   105,   106,
     107,     0,     0,     0,     0,    33,    34,   139,    36,    37,
      38,     0,    39,    40,   140,    42,   141,    44,   142,    46,
     143,    48,    49,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,   144,    61,     0,     0,    62,   145,
      64,    65,   108,   109,   110,   111,     0,     0,     0,   112,
       0,     0,     0,   146,   147,     0,     0,     0,     0,   166,
     503,   149,   129,   130,     1,   131,   132,    90,   133,   134,
     135,   136,    91,     8,    92,    93,   137,    94,    95,    96,
      97,    15,    16,    98,   138,    99,    20,    21,   100,   101,
     102,   103,    25,     0,    27,    28,   104,   105,   106,   107,
       0,     0,     0,     0,    33,    34,   139,    36,    37,    38,
       0,    39,    40,   140,    42,   141,    44,   142,    46,   143,
      48,    49,    50,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,   144,    61,     0,     0,    62,   145,    64,
      65,   108,   109,   110,   111,     0,     0,     0,   112,     0,
       0,     0,   146,   147,     0,     0,     0,     0,   148,     0,
     149,   129,   130,     1,   131,   132,    90,   133,   134,   135,
     136,    91,     8,    92,    93,    10,    94,    95,    96,    97,
      15,    16,    98,    18,    99,    20,    21,   100,   101,   102,
     103,    25,     0,    27,    28,   104,   105,   106,   107,     0,
       0,     0,     0,    33,    34,   139,    36,    37,    38,     0,
      39,    40,   140,    42,   141,    44,   142,    46,   143,    48,
      49,    50,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,   144,    61,     0,     0,    62,   145,    64,    65,
     108,   109,   110,   111,     0,     0,     0,   112,     0,     0,
       0,   146,   147,     0,     0,     0,     0,   166,     0,   149,
     129,   130,     1,   131,   132,    90,   133,   134,   135,   136,
      91,     8,    92,    93,    10,    94,    95,    96,    97,    15,
      16,    98,    18,    99,    20,    21,   100,   101,   102,   103,
      25,     0,    27,    28,   104,   105,   106,   107,     0,     0,
       0,     0,    33,    34,   139,    36,    37,    38,     0,    39,
      40,   140,    42,   141,    44,   142,    46,   143,    48,    49,
      50,    51,    52,    53,    54,    55,    56,     0,    57,    58,
      59,   144,    61,     0,     0,    62,   145,    64,    65,   108,
     109,   110,   111,     0,     0,     0,   112,     0,     0,     0,
     146,   147,     0,     0,     0,     0,   148,     0,   149,   129,
     130,     1,   131,   132,    90,   133,   134,   135,   136,    91,
       8,    92,    93,    10,    94,    95,    96,    97,    15,    16,
      98,    18,    99,    20,    21,   100,   101,   102,   103,    25,
       0,    27,    28,   104,   105,   106,   107,     0,     0,     0,
       0,    33,    34,   139,    36,    37,    38,     0,    39,    40,
     140,    42,   141,    44,   142,    46,   143,    48,    49,    50,
      51,    52,    53,    54,    55,    56,     0,    57,    58,    59,
     144,    61,     0,     0,    62,    63,    64,    65,   108,   109,
     110,   111,     0,     0,     0,   112,     0,     0,     0,   146,
     147,     0,     0,     0,     0,   197,     0,   149,   129,   130,
       1,   131,   132,    90,   133,   134,   135,   136,    91,     8,
      92,    93,    10,    94,    95,    96,    97,    15,    16,    98,
      18,    99,    20,    21,   100,   101,   102,   103,    25,     0,
      27,    28,   104,   105,   106,   107,     0,     0,     0,     0,
      33,    34,   139,    36,    37,    38,     0,    39,    40,   140,
      42,   141,    44,   142,    46,   143,    48,    49,    50,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,   144,
      61,     0,     0,    62,    63,    64,    65,   108,   109,   110,
     111,     0,     0,     0,   112,     0,     0,     0,   146,   147,
       0,     0,     0,     0,   166,     0,   149,   129,   130,     1,
     131,   132,    90,   133,   134,   135,   136,    91,     8,    92,
      93,    10,    94,    95,    96,    97,    15,    16,    98,    18,
      99,    20,    21,   100,   101,   102,   103,    25,     0,    27,
      28,   104,   105,   106,   107,     0,     0,     0,     0,    33,
      34,   139,    36,    37,    38,     0,    39,    40,   140,    42,
     141,    44,   142,    46,   143,    48,    49,    50,    51,    52,
      53,    54,    55,   429,     0,   431,    58,    59,   144,    61,
       0,     0,   435,   436,    64,    65,   108,   109,   110,   111,
       0,     0,     0,   112,     0,     0,     0,   146,   147,     0,
       0,     0,     0,   166,     0,   149,   229,   230,     1,   231,
     232,    90,   133,   134,   135,   136,    91,     8,    92,    93,
      10,    94,    95,    96,    97,    15,    16,    98,    18,    99,
      20,    21,   100,   101,   102,   103,    25,     0,    27,    28,
     104,   105,   106,   107,     0,     0,     0,     0,    33,    34,
     139,    36,    37,    38,     0,    39,    40,   140,    42,   141,
      44,   142,    46,   143,    48,    49,    50,    51,    52,    53,
      54,    55,    56,     0,    57,    58,    59,   144,    61,     0,
       0,    62,   145,    64,    65,   108,   109,   110,   111,     0,
       0,     0,   112,     0,     0,     0,   146,   147,     0,     0,
       0,     0,   166,     0,   149,   129,   130,     1,   131,   132,
      90,   133,   134,   135,   136,    91,     8,    92,    93,    10,
      94,    95,    96,    97,    15,    16,    98,    18,    99,    20,
      21,   100,   101,   102,   103,    25,     0,    27,    28,   104,
     105,   106,   107,     0,     0,     0,     0,    33,    34,   139,
      36,    37,    38,     0,    39,    40,   140,    42,   141,    44,
     142,    46,   143,    48,    49,    50,    51,    52,    53,    54,
      55,   489,     0,   490,    58,    59,   144,    61,     0,     0,
     491,   492,    64,    65,   108,   109,   110,   111,     0,     0,
       0,   112,     0,     0,     0,   146,   147,     0,     0,     0,
       0,   166,     0,   149,   129,   130,     1,   131,   132,    90,
     133,   134,   135,   136,    91,     8,    92,    93,    10,    94,
      95,    96,    97,    15,    16,    98,    18,    99,    20,    21,
     100,   101,   102,   103,    25,     0,    27,    28,   104,   105,
     106,   107,     0,     0,     0,     0,     0,     0,   139,    36,
      37,    38,     0,    39,    40,   140,    42,   141,    44,   142,
      46,   143,    48,    49,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,   144,    61,     0,     0,    62,
     145,    64,    65,   108,   109,   110,   111,     0,     0,     0,
     112,     0,     0,     0,   146,   147,     0,     0,     0,     0,
     166,     0,   149,   129,   130,     1,   131,   132,    90,   133,
     134,   135,   136,    91,     8,    92,    93,    10,    94,    95,
      96,    97,    15,    16,    98,    18,    99,    20,    21,   100,
     101,   102,   103,    25,     0,    27,    28,   104,   105,   106,
     107,     0,     0,     0,     0,     0,     0,   139,    36,    37,
      38,     0,     0,     0,   140,    42,   141,    44,   142,    46,
     143,    48,    49,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,   144,    61,     0,     0,    62,   145,
      64,    65,   108,   109,   110,   111,     0,     0,     0,   112,
       0,     0,     0,   146,   147,     0,     0,     0,     0,   166,
       0,   149,   129,   130,     1,   131,   132,    90,   133,   134,
     135,   136,    91,     8,    92,    93,    10,    94,    95,    96,
      97,    15,    16,    98,    18,    99,    20,    21,   100,   101,
     102,   103,    25,     0,    27,    28,   104,   105,   106,   107,
       0,     0,     0,     0,     0,     0,   139,     0,    37,    38,
       0,     0,     0,   140,    42,   141,    44,   142,    46,   143,
      48,    49,    50,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,   144,    61,     0,     0,    62,   145,    64,
      65,   108,   109,   110,   111,     0,     0,     0,   112,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   166,     1,
     149,     0,     2,     3,     4,     5,     6,     7,     8,     0,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,     0,    57,    58,    59,    60,    61,
       0,     0,    62,    63,    64,    65,    66,    67,    68,     0,
      69,     1,     0,    70,     0,     3,     4,     5,     6,     7,
       8,    71,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,    28,     0,    30,    31,    32,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,     0,    57,    58,    59,
      60,    61,     0,     0,    62,    63,    64,    65,    66,    67,
       0,     0,    69,     1,     0,    70,     0,     3,     4,     5,
       6,     7,     8,   383,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,    28,     0,    30,    31,    32,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,     0,     0,    62,    63,    64,    65,
      66,    67,     0,     0,    69,     1,     0,    70,     0,     3,
       4,     5,     6,     7,     8,   406,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,     0,    30,    31,
      32,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,    60,    61,     0,     0,    62,    63,
      64,    65,    66,    67,     0,     0,    69,     1,     0,    70,
       0,     3,     4,     5,     6,     7,     8,   528,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,     0,
      30,    31,    32,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,    60,    61,     0,     0,
      62,    63,    64,    65,    66,    67,     0,     0,    69,     1,
       0,    70,     0,     3,     4,     5,     6,     7,     8,   557,
       9,    10,    11,    12,    13,    14,    15,    16,     0,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
      28,     0,    30,    31,    32,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,     0,    57,    58,    59,    60,    61,
       0,     0,    62,    63,    64,    65,    66,    67,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,     0,     1,
       0,   476,    90,     3,     4,     5,     6,    91,     8,    92,
      93,    10,    94,    95,    96,    97,    15,    16,    98,    18,
      99,    20,    21,   100,   101,   102,   103,    25,     0,    27,
      28,   104,   105,   106,   107,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,     0,    57,    58,    59,    60,    61,
       0,     0,    62,    63,    64,    65,   108,   109,   110,   111,
       0,     1,     0,   112,    90,     3,     4,     5,     6,    91,
       8,    92,    93,    10,    94,    95,    96,    97,    15,    16,
      98,    18,    99,    20,    21,   100,   101,   102,   103,    25,
       0,    27,    28,   104,   105,   106,   107,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,     0,    57,    58,    59,
     174,    61,     0,     0,    62,    63,    64,    65,   108,   109,
     110,   111,     0,     1,     0,   112,    90,     3,     4,     5,
       6,    91,     8,    92,    93,    10,    94,    95,    96,    97,
      15,    16,    98,    18,    99,    20,    21,   100,   101,   102,
     103,    25,     0,    27,    28,   104,   105,   106,   107,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
     215,   216,   217,   218,   219,    54,   220,    56,     0,    57,
      58,   221,    60,    61,     0,     0,    62,    63,    64,    65,
     108,   109,   110,   111,     0,     1,     0,   112,    90,   133,
     134,     5,     6,    91,     8,    92,    93,    10,    94,    95,
      96,    97,    15,    16,    98,    18,    99,    20,    21,   100,
     101,   102,   103,    25,     0,    27,    28,   104,   105,   106,
     107,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,   140,    42,   141,    44,   142,    46,
     143,    48,    49,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,   386,    61,     0,     0,    62,    63,
      64,    65,   108,   109,   110,   111,     1,     0,     0,   112,
       3,     4,     5,     6,     7,     8,     0,     9,    10,    11,
      12,    13,     0,    15,    16,     0,    18,    19,    20,    21,
      22,   101,    24,     0,    25,     0,    27,    28,     0,    30,
      31,    32,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,    60,    61,   306,     0,    62,
      63,    64,    65,   307,    67,     0,     0,     0,     0,     0,
      70,   244,   245,   246,   247,   269,   270,   505,   271,     0,
       0,   251,   272,   273,     0,     0,     0,     0,     0,     0,
     506,   244,   245,   246,   247,   269,   270,     0,   271,     0,
       0,   251,   272,   273,   244,   245,   246,   247,   269,   270,
       0,   271,     0,   507,   251,   272,   273,     0,     0,     0,
     254,   255,   256,   257,   258,   259,   260,   244,   245,   246,
     247,   269,   270,     0,   271,     0,     0,   251,   272,   273,
     254,   255,   256,   257,   258,   259,   260,   508,     0,     0,
       0,     0,     0,   254,   255,   256,   257,   258,   259,   260,
       0,   244,   245,   246,   247,   269,   270,     0,   271,   581,
       0,   251,   272,   273,     0,     0,   254,   255,   256,   257,
     258,   259,   260,   565,   566,     0,   567,   568,   569,   292,
     570,     0,     0,   571,   572,   573,     0,   244,   245,   246,
     247,   269,   270,     0,   271,     0,     0,   251,   272,   273,
     254,   255,   256,   257,   258,   259,   260,     0,   244,   245,
     246,   247,   269,   270,     0,   271,     0,     0,   251,   272,
     273,     0,   574,   575,   576,   577,   578,   579,   580,     0,
       0,     0,     0,     0,     0,   268,   254,   255,   256,   257,
     258,   259,   260,   293,   244,   245,   246,   247,   269,   270,
       0,   271,     0,     0,   251,   272,   273,   254,   255,   256,
     257,   258,   259,   260,     0,   331,     0,   332,   244,   245,
     246,   247,   269,   270,     0,   271,     0,     0,   251,   272,
     273,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   254,   255,   256,   257,   258,   259,   260,
     244,   245,   246,   247,   269,   270,     0,   271,     0,     0,
     251,   272,   273,     0,     0,     0,     0,   254,   255,   256,
     257,   258,   259,   260,     0,   412,     0,   332,   364,   365,
     366,   367,   368,   369,     0,   370,     0,     0,   371,   372,
     373,     0,     0,     0,     0,     0,     0,     0,     0,   254,
     255,   256,   257,   258,   259,   260,     0,     0,     0,   332,
     244,   245,   246,   247,   269,   270,     0,   271,     0,     0,
     251,   272,   273,     0,     0,     0,     0,   374,   375,   376,
     377,   378,   379,   380,     0,   381,   244,   245,   246,   247,
     269,   270,     0,   271,     0,     0,   251,   272,   273,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   254,
     255,   256,   257,   258,   259,   260,   394,   244,   245,   246,
     247,   269,   270,     0,   271,     0,     0,   251,   272,   273,
       0,     0,     0,     0,     0,   254,   255,   256,   257,   258,
     259,   260,   495,   244,   245,   246,   247,   269,   270,     0,
     271,     0,     0,   251,   272,   273,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   254,   255,   256,   257,
     258,   259,   260,   499,   244,   245,   246,   247,   269,   270,
       0,   271,     0,     0,   251,   272,   273,     0,     0,     0,
       0,     0,   254,   255,   256,   257,   258,   259,   260,   530,
     244,   245,   246,   247,   269,   270,     0,   271,     0,     0,
     251,   272,   273,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   254,   255,   256,   257,   258,   259,   260,
     590,   244,   245,   246,   247,   269,   270,     0,   271,     0,
       0,   251,   272,   273,     0,     0,     0,     0,     0,   254,
     255,   256,   257,   258,   259,   260,   613,   244,   245,   246,
     247,   269,   270,     0,   271,     0,     0,   251,   272,   273,
     328,   329,     0,     0,     0,     0,     0,     0,     0,     0,
     254,   255,   256,   257,   258,   259,   260,   364,   365,   366,
     367,   368,   369,     0,   370,     0,     0,   371,   372,   373,
       0,     0,     0,     0,     0,     0,   254,   255,   256,   257,
     258,   259,   260,   244,   245,   246,   247,     0,     0,     0,
     271,     0,     0,   251,   272,   273,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   374,   375,   376,   377,
     378,   379,   380,   565,   566,     0,   567,     0,     0,     0,
     570,     0,     0,   571,   572,   573,     0,     0,     0,     0,
       0,     0,   254,   255,   256,   257,   258,   259,   260,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   574,   575,   576,   577,   578,   579,   580
};

static const yytype_int16 yycheck[] =
{
       0,     0,    24,     7,    70,    32,    12,     9,    14,    13,
     383,     2,    31,   397,    31,    16,    22,   226,    49,   264,
      11,    52,   528,    98,    30,    31,    17,    22,   560,    23,
     385,   240,    69,    24,    98,    30,    31,    34,    29,   394,
      67,    32,    10,    11,   100,    68,     0,    44,   104,   163,
      44,   557,   261,    76,   586,    98,   411,   171,    98,   163,
      66,    99,    93,    94,    95,    96,    97,   171,   182,    49,
      70,    99,    78,    73,    73,   103,    67,    68,   182,    98,
     194,    28,    49,    78,    86,   330,    87,    55,    98,    57,
     194,    59,   100,    61,    36,    37,    98,    68,   212,   344,
      49,    28,   117,    52,    49,    76,   479,    75,   212,    49,
      98,    99,   118,   119,   120,   121,    96,    97,   363,   134,
     135,   136,   331,    98,    99,    68,    98,    94,    95,    96,
      97,   137,   138,    76,   140,   141,   142,   143,    23,   145,
      98,   168,   148,    28,    93,    94,    95,    96,    97,    94,
      95,    96,    97,   144,    68,   528,    96,    97,   149,   543,
     166,   188,    76,    68,    55,    98,    57,   173,    59,    98,
      61,    76,   381,    98,    99,   530,    98,   168,   278,   279,
      98,   172,    98,   174,   557,   425,   426,   427,   428,    98,
      98,   197,   432,   104,    98,    98,    98,   188,   104,    26,
      99,   446,   103,   412,   104,   196,   104,   103,    98,    28,
      98,    26,    99,    16,    98,    98,    98,    89,   209,    88,
     101,   101,    98,     9,    28,    70,   292,   582,   101,    34,
     475,    98,    98,    98,    70,   590,    87,   101,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   277,   278,   279,   613,   504,
      88,    70,    73,   269,   270,   271,   272,   273,   268,   268,
      88,    88,   402,   277,   278,   279,    98,    98,    98,   285,
      31,   409,   288,   479,   517,   327,   277,   278,   279,   339,
     116,   117,   292,   293,    -1,   301,    -1,   303,    -1,   305,
     212,   307,    -1,    -1,    -1,    -1,    -1,   133,   134,   135,
     136,    -1,    -1,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,   147,    -1,    -1,    -1,    -1,    -1,   333,   334,   335,
     336,    -1,   338,   339,    -1,    -1,   342,    -1,    -1,   345,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   339,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   489,   490,   491,   492,    -1,
      -1,    -1,    -1,   383,    -1,   385,   392,   393,    -1,    -1,
      -1,    -1,    42,    43,   394,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    -1,   397,   398,    -1,   409,
      -1,   411,    -1,    -1,    -1,    -1,    -1,    -1,   424,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   490,   491,   492,    -1,    -1,   443,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    -1,    -1,
      -1,   565,   566,   567,   568,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   578,   579,   580,    -1,    -1,    -1,
     497,    28,    -1,    -1,   300,    -1,   302,    34,   304,   479,
     306,   481,   481,    -1,    -1,    42,    43,    44,    45,    46,
      47,    -1,    49,   607,    -1,    52,    53,    54,    -1,    -1,
     326,   327,   328,   329,    -1,    -1,   497,   565,   566,   567,
     568,   569,   570,   571,   572,   573,   574,   575,   576,   577,
     578,   579,   580,    -1,   524,   524,   532,    -1,   528,    -1,
     530,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,   607,
      -1,    -1,   543,    -1,    -1,    -1,   562,   557,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,    -1,   395,
      -1,    -1,    -1,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,   582,    52,    53,    54,    -1,    -1,    -1,    -1,
     590,    -1,    -1,    -1,    -1,    -1,    -1,   423,    -1,    -1,
      -1,    -1,    -1,   429,    -1,   431,    -1,    -1,    -1,   435,
     436,    -1,    -1,   613,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    28,
      -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   489,   490,   491,   492,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   505,
     506,   507,   508,    -1,    -1,   511,   512,   513,   514,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    28,
      -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    52,    53,    54,    -1,    -1,   554,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   565,
     566,   567,   568,   569,   570,   571,   572,   573,   574,   575,
     576,   577,   578,   579,   580,    -1,    -1,    -1,    -1,   585,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   607,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,   100,    -1,
     102,    -1,   104,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    95,    96,    97,    -1,    99,   100,
      -1,   102,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    -1,    99,   100,    -1,
     102,     4,     5,     6,     7,     8,     9,    10,    11,    12,
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
      95,    96,    97,    -1,    -1,   100,    -1,   102,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,   100,    -1,   102,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,
      -1,    -1,    -1,   100,   101,   102,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,
      -1,    -1,   100,   101,   102,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,   100,   101,   102,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
     100,   101,   102,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,   100,
     101,   102,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,   100,    -1,
     102,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,
      -1,    94,    95,    -1,    -1,    -1,    -1,   100,    -1,   102,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      94,    95,    -1,    -1,    -1,    -1,   100,    -1,   102,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,
      95,    -1,    -1,    -1,    -1,   100,    -1,   102,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,   100,    -1,   102,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,
      -1,    -1,    -1,   100,    -1,   102,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,
      -1,    -1,   100,    -1,   102,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,   100,    -1,   102,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
     100,    -1,   102,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      51,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,   100,
      -1,   102,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,     6,
     102,    -1,     9,    10,    11,    12,    13,    14,    15,    -1,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    -1,
      87,     6,    -1,    90,    -1,    10,    11,    12,    13,    14,
      15,    98,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    36,    37,    -1,    39,    40,    41,    -1,    -1,    -1,
      -1,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    81,    82,    83,    84,
      -1,    -1,    87,     6,    -1,    90,    -1,    10,    11,    12,
      13,    14,    15,    98,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    36,    37,    -1,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    -1,    -1,    79,    80,    81,    82,
      83,    84,    -1,    -1,    87,     6,    -1,    90,    -1,    10,
      11,    12,    13,    14,    15,    98,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    36,    37,    -1,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    -1,    -1,    87,     6,    -1,    90,
      -1,    10,    11,    12,    13,    14,    15,    98,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    36,    37,    -1,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    -1,
      79,    80,    81,    82,    83,    84,    -1,    -1,    87,     6,
      -1,    90,    -1,    10,    11,    12,    13,    14,    15,    98,
      17,    18,    19,    20,    21,    22,    23,    24,    -1,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    36,
      37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,     6,
      -1,    98,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,     6,    -1,    90,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,     6,    -1,    90,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,     6,    -1,    90,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,     6,    -1,    -1,    90,
      10,    11,    12,    13,    14,    15,    -1,    17,    18,    19,
      20,    21,    -1,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    -1,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    28,    -1,    79,
      80,    81,    82,    34,    84,    -1,    -1,    -1,    -1,    -1,
      90,    42,    43,    44,    45,    46,    47,    28,    49,    -1,
      -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    28,    52,    53,    54,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      91,    92,    93,    94,    95,    96,    97,    28,    -1,    -1,
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    97,
      -1,    42,    43,    44,    45,    46,    47,    -1,    49,    28,
      -1,    52,    53,    54,    -1,    -1,    91,    92,    93,    94,
      95,    96,    97,    42,    43,    -1,    45,    46,    47,    34,
      49,    -1,    -1,    52,    53,    54,    -1,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      91,    92,    93,    94,    95,    96,    97,    -1,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    -1,    91,    92,    93,    94,    95,    96,    97,    -1,
      -1,    -1,    -1,    -1,    -1,    33,    91,    92,    93,    94,
      95,    96,    97,    98,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    91,    92,    93,
      94,    95,    96,    97,    -1,    99,    -1,   101,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    97,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    -1,    -1,    91,    92,    93,
      94,    95,    96,    97,    -1,    99,    -1,   101,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    96,    97,    -1,    -1,    -1,   101,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    -1,    -1,    91,    92,    93,
      94,    95,    96,    97,    -1,    99,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    96,    97,    98,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,
      96,    97,    98,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,
      95,    96,    97,    98,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    98,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    97,
      98,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    96,    97,    98,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,
      95,    96,    97,    42,    43,    44,    45,    -1,    -1,    -1,
      49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,
      95,    96,    97,    42,    43,    -1,    45,    -1,    -1,    -1,
      49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     6,     9,    10,    11,    12,    13,    14,    15,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    34,    35,    36,    37,    38,
      39,    40,    41,    46,    47,    48,    49,    50,    51,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    72,    73,    74,
      75,    76,    79,    80,    81,    82,    83,    84,    85,    87,
      90,    98,   106,   107,   108,   109,   110,   115,   117,   121,
     122,   123,   124,   128,   130,   131,   132,   134,   135,   136,
       9,    14,    16,    17,    19,    20,    21,    22,    25,    27,
      30,    31,    32,    33,    38,    39,    40,    41,    83,    84,
      85,    86,    90,   116,   117,   118,    10,    11,    55,    57,
      59,    61,    75,   147,    31,    98,   118,   119,   120,     4,
       5,     7,     8,    10,    11,    12,    13,    18,    26,    48,
      55,    57,    59,    61,    75,    80,    94,    95,   100,   102,
     118,   125,   127,   143,   144,   147,   149,   151,   154,   155,
     159,   162,   163,   164,   167,   147,   100,   164,   118,    31,
     125,   164,    36,    37,    75,   118,   144,   146,   157,    98,
     118,   125,   164,   125,   127,   119,   164,   119,   118,    98,
     123,     0,   108,   125,   164,    98,    99,   100,   150,   163,
     150,   164,   164,   164,   164,    69,    98,    98,    98,    99,
     150,   150,   164,   164,   163,    63,    64,    65,    66,    67,
      69,    74,   118,   164,   163,   163,     6,   101,   164,     4,
       5,     7,     8,   104,   118,   160,   161,   164,   168,   169,
     100,    98,    98,    28,    42,    43,    44,    45,    46,    47,
      49,    52,    53,    54,    91,    92,    93,    94,    95,    96,
      97,    99,   126,   143,   165,   167,    98,   164,    33,    46,
      47,    49,    53,    54,   119,    98,    98,    24,    50,    51,
     118,   164,   118,    34,    44,   129,    98,    98,    36,    98,
      98,    98,    34,    98,    98,   119,    98,   118,   101,   164,
      28,    34,    28,    34,    28,    34,    28,    34,    55,    57,
      59,    61,   148,   118,    28,   165,    68,    76,    68,    76,
      68,    76,    68,    76,    68,    76,    28,    28,    81,    82,
     169,    99,   101,   104,   104,   104,   104,   103,   104,    99,
     103,   103,    99,   101,   169,    26,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   169,    42,    43,    44,    45,    46,    47,
      49,    52,    53,    54,    91,    92,    93,    94,    95,    96,
      97,    99,   126,    98,   121,    98,    75,   144,   145,   147,
     145,   145,    23,    44,    98,    28,   164,     9,    86,    98,
     111,   112,   113,   114,   164,   123,    98,   121,   137,   138,
     139,    98,    99,   163,   164,   163,   164,   163,   164,   163,
     164,    23,    28,   156,    26,    56,    58,    60,    62,    70,
      71,    72,    73,    77,    78,    79,    80,   152,   153,   163,
     152,   163,   163,    99,   126,   101,   169,   164,   164,   164,
     164,   164,   161,   164,   164,   126,   164,   126,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   169,    98,   122,   140,   141,
     142,    16,   137,    98,    98,    98,   164,   164,   137,    70,
      72,    79,    80,   158,   163,    98,   116,   118,   114,    98,
      89,   139,   137,   101,   169,    28,    28,    28,    28,   163,
     164,   156,   156,   156,   156,   163,   163,   156,   163,   163,
     101,   126,   101,   126,    16,    87,   133,   142,    98,   121,
      98,    15,    34,    88,   163,   166,   167,   166,   166,   166,
      98,   119,    98,     9,   126,   163,   163,   163,   163,   163,
     163,   163,   163,    28,    70,   153,   101,    98,   121,    98,
     140,   137,    34,   164,    98,    42,    43,    45,    46,    47,
      49,    52,    53,    54,    91,    92,    93,    94,    95,    96,
      97,    28,    98,   116,   101,    70,   140,   133,    88,   164,
      98,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   166,   166,    70,   137,    98,
     163,   133,    98,    98,   137,   166,   137,    88,    88,    98,
      98
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
     117,   117,   117,   117,   117,   117,   117,   117,   117,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   119,   119,
     119,   120,   120,   121,   121,   122,   122,   122,   122,   122,
     122,   123,   123,   123,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   125,   125,
     125,   125,   125,   125,   125,   125,   125,   126,   126,   127,
     127,   127,   127,   127,   128,   128,   128,   128,   129,   129,
     130,   130,   130,   131,   131,   132,   132,   132,   132,   133,
     133,   134,   134,   134,   134,   135,   135,   136,   137,   137,
     138,   138,   139,   139,   140,   140,   141,   141,   142,   142,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   144,   145,   145,   146,   146,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   148,
     148,   148,   148,   149,   149,   150,   150,   150,   150,   151,
     151,   151,   151,   151,   152,   152,   152,   152,   152,   153,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     155,   155,   155,   155,   155,   155,   155,   155,   156,   156,
     157,   157,   158,   158,   158,   158,   158,   159,   159,   159,
     160,   160,   160,   161,   161,   161,   161,   161,   162,   162,
     163,   163,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     166,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   166,   166,   166,   167,   167,
     168,   168,   169,   169
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
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       2,     1,     3,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     5,     3,     3,     2,     3,     3,     2,     0,     2,
       4,     3,     5,     2,     4,     6,     5,     0,     1,     2,
       2,     5,     4,     3,     5,     5,     5,     5,     1,     1,
       3,     3,     3,     4,     6,     6,     8,     7,     9,     0,
       2,     7,    11,    12,     9,     4,     6,     2,     0,     1,
       1,     2,     1,     1,     0,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     2,     3,     5,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       4,     6,     4,     6,     4,     6,     4,     6,     5,     1,
       1,     1,     1,     2,     2,     1,     2,     4,     6,     2,
       4,     4,     1,     1,     1,     1,     5,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       6,     6,     6,     6,     6,     4,     4,     4,     1,     1,
       2,     4,     1,     2,     5,     2,     2,     3,     3,     3,
       1,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     4,     4,
       0,     1,     1,     3
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
#line 2322 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2328 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2334 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2340 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2346 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2352 "engines/director/lingo/lingo-gr.cpp"
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
#line 2622 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 202 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2633 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 208 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2643 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 215 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2649 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* scriptpart: tENDCLAUSE endargdef '\n'  */
#line 220 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2655 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 249 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2661 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* factory: tFACTORY ID '\n' methodlist  */
#line 253 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2667 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 255 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2673 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* methodlist: %empty  */
#line 257 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2679 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlistline  */
#line 261 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2690 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 267 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2700 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18: /* methodlistline: '\n'  */
#line 274 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2706 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* methodlistline: tENDCLAUSE endargdef '\n'  */
#line 276 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2712 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 281 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2721 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* handler: tON ID idlist '\n' stmtlist  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* endargdef: ID  */
#line 290 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2734 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* endargdef: endargdef ',' ID  */
#line 291 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2740 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* CMDID: tABBREVIATED  */
#line 299 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2746 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* CMDID: tABBREV  */
#line 300 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2752 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* CMDID: tABBR  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2758 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* CMDID: tAFTER  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2764 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* CMDID: tAND  */
#line 303 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 2770 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tBEFORE  */
#line 304 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2776 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tCAST  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2782 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tCHAR  */
#line 306 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2788 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tCHARS  */
#line 307 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2794 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tCONTAINS  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("contains"); }
#line 2800 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tDATE  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2806 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tDELETE  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 2812 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tDOWN  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tFIELD  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2824 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tFRAME  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2830 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tHILITE  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 2836 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tIN  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2842 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tINTERSECTS  */
#line 316 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2848 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tINTO  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2854 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tITEM  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2860 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tITEMS  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2866 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tLAST  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2872 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tLINE  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tLINES  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2884 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tLONG  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2890 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tMENU  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2896 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tMENUITEM  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2902 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tMENUITEMS  */
#line 326 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2908 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tMOD  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2914 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tMOVIE  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2920 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tNEXT  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2926 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tNOT  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2932 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tNUMBER  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2938 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tOF  */
#line 332 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2944 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tOR  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2950 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tPREVIOUS  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2956 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tREPEAT  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2962 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tSCRIPT  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2968 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tASSERTERROR  */
#line 337 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2974 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tSHORT  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2980 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tSOUND  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2986 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tSPRITE  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2992 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tSTARTS  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2998 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tTHE  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 3004 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tTIME  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 3010 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tTO  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 3016 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* CMDID: tWHILE  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 3022 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* CMDID: tWINDOW  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 3028 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* CMDID: tWITH  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 3034 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* CMDID: tWITHIN  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 3040 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* CMDID: tWORD  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 3046 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* CMDID: tWORDS  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 3052 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* ID: tELSE  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 3058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tENDCLAUSE  */
#line 355 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); delete (yyvsp[0].s); }
#line 3064 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tEXIT  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 3070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tFACTORY  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 3076 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tGLOBAL  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 3082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tGO  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 3088 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tIF  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 3094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tINSTANCE  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 3100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tMACRO  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3106 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tMETHOD  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3112 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tON  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3118 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tOPEN  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tPLAY  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3130 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ID: tPROPERTY  */
#line 367 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3136 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* ID: tPUT  */
#line 368 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* ID: tSET  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* ID: tTELL  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3154 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* ID: tTHEN  */
#line 371 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3160 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* idlist: %empty  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3166 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* nonemptyidlist: ID  */
#line 379 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3175 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 383 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* stmt: tENDIF '\n'  */
#line 394 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3189 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* proc: CMDID cmdargs '\n'  */
#line 410 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3195 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* proc: tPUT cmdargs '\n'  */
#line 411 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3201 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: tGO cmdargs '\n'  */
#line 412 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3207 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: tGO frameargs '\n'  */
#line 413 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tPLAY cmdargs '\n'  */
#line 414 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3219 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tPLAY frameargs '\n'  */
#line 415 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3225 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tOPEN cmdargs '\n'  */
#line 416 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3231 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tOPEN expr tWITH expr '\n'  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3241 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* proc: tNEXT tREPEAT '\n'  */
#line 422 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proc: tEXIT tREPEAT '\n'  */
#line 423 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3253 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tEXIT '\n'  */
#line 424 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proc: tDELETE chunk '\n'  */
#line 425 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* proc: tHILITE chunk '\n'  */
#line 426 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3271 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* proc: tASSERTERROR stmtoneliner  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* cmdargs: %empty  */
#line 430 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3285 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* cmdargs: expr trailingcomma  */
#line 433 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 438 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd args, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 442 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3315 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 448 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3325 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* cmdargs: '(' ')'  */
#line 453 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* cmdargs: '(' expr ',' ')'  */
#line 456 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(args,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 461 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(args, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3352 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* cmdargs: '(' tVARID nonemptyexprlist trailingcomma ')'  */
#line 465 "engines/director/lingo/lingo-gr.y"
                                                              {
		// This matches `cmd(args ...)`
		(yyvsp[-2].nodelist)->insert_at(0, new VarNode((yyvsp[-3].s)));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3361 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* frameargs: tFRAME expr  */
#line 477 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3371 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* frameargs: tMOVIE expr  */
#line 482 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3382 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 488 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3393 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* frameargs: expr tOF tMOVIE expr  */
#line 494 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3404 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* frameargs: tFRAME expr expr_nounarymath  */
#line 500 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3415 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 508 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3421 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 509 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3427 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 510 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3433 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* asgn: tSET varorthe to expr '\n'  */
#line 511 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3439 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* definevars: tGLOBAL idlist '\n'  */
#line 516 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* definevars: tPROPERTY idlist '\n'  */
#line 517 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3451 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* definevars: tINSTANCE idlist '\n'  */
#line 518 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3457 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* ifstmt: tIF expr tTHEN stmt  */
#line 521 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3466 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 525 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 3473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 529 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3484 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 535 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 3493 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3502 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 543 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 3509 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* endif: %empty  */
#line 547 "engines/director/lingo/lingo-gr.y"
                        { warning("LingoCompiler::parse: no end if"); }
#line 3515 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 550 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3522 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 552 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3529 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 554 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3536 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 556 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3543 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* tell: tTELL expr tTO stmtoneliner  */
#line 560 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3552 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 564 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3559 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* when: tWHEN '\n'  */
#line 568 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WhenNode((yyvsp[-1].w).eventName, (yyvsp[-1].w).stmt); }
#line 3565 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* stmtlist: %empty  */
#line 570 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3571 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* nonemptystmtlist: stmtlistline  */
#line 575 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 581 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3592 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* stmtlistline: '\n'  */
#line 588 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3598 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* stmtlist_insideif: %empty  */
#line 592 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 3604 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 597 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3615 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 603 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* stmtlistline_insideif: '\n'  */
#line 610 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* simpleexpr_nounarymath: tINT  */
#line 617 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* simpleexpr_nounarymath: tFLOAT  */
#line 618 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3643 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* simpleexpr_nounarymath: tSYMBOL  */
#line 619 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3649 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* simpleexpr_nounarymath: tSTRING  */
#line 620 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3655 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 621 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3661 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 622 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 3667 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 623 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 3673 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 624 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3679 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* var: ID  */
#line 632 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3685 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* chunk: tFIELD refargs  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 3691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* chunk: tCAST refargs  */
#line 643 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 3697 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* chunk: tCHAR expr tOF simpleexpr  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3704 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* chunk: tWORD expr tOF simpleexpr  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3718 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3725 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* chunk: tITEM expr tOF simpleexpr  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3732 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3739 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* chunk: tLINE expr tOF simpleexpr  */
#line 656 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3746 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 658 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 660 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 3759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* chunktype: tCHAR  */
#line 663 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 3765 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* chunktype: tWORD  */
#line 664 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 3771 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* chunktype: tITEM  */
#line 665 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 3777 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* chunktype: tLINE  */
#line 666 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 3783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* object: tSCRIPT refargs  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 3789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* object: tWINDOW refargs  */
#line 670 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 3795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* refargs: simpleexpr  */
#line 673 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3805 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* refargs: '(' ')'  */
#line 678 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 3813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* refargs: '(' expr ',' ')'  */
#line 681 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(args,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3823 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 686 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(args, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3832 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* the: tTHE ID  */
#line 692 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3838 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* the: tTHE ID tOF theobj  */
#line 693 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3844 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 221: /* the: tTHE tNUMBER tOF theobj  */
#line 694 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 3850 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 226: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 701 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3856 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* theobj: tSOUND simpleexpr  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3862 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* theobj: tSPRITE simpleexpr  */
#line 703 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 229: /* menu: tMENU simpleexpr  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 230: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 708 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3880 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 709 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3886 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* thedatetime: tTHE tABBREV tDATE  */
#line 710 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3892 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* thedatetime: tTHE tABBREV tTIME  */
#line 711 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3898 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* thedatetime: tTHE tABBR tDATE  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* thedatetime: tTHE tABBR tTIME  */
#line 713 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3910 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* thedatetime: tTHE tLONG tDATE  */
#line 714 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 3916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* thedatetime: tTHE tLONG tTIME  */
#line 715 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 3922 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* thedatetime: tTHE tSHORT tDATE  */
#line 716 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 3928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* thedatetime: tTHE tSHORT tTIME  */
#line 717 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 3934 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 721 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 3940 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 722 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 3946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 723 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 3952 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 724 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 3958 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 725 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 3964 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* thenumberof: tTHE tNUMBER tOF tMENUS  */
#line 726 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenus, nullptr); }
#line 3970 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* thenumberof: tTHE tNUMBER tOF tXTRAS  */
#line 727 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfXtras, nullptr); }
#line 3976 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* thenumberof: tTHE tNUMBER tOF tCASTLIBS  */
#line 728 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNumberOfNode(kNumberOfCastlibs, nullptr); }
#line 3982 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* writablethe: tTHE ID  */
#line 733 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3988 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* writablethe: tTHE ID tOF writabletheobj  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3994 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* writabletheobj: tMENU expr_noeq  */
#line 738 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 4000 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 739 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 4006 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* writabletheobj: tSOUND expr_noeq  */
#line 740 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 4012 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* writabletheobj: tSPRITE expr_noeq  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 4018 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* list: '[' exprlist ']'  */
#line 744 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 4024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* list: '[' ':' ']'  */
#line 745 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 4030 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* list: '[' proplist ']'  */
#line 746 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 4036 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* proplist: proppair  */
#line 752 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4045 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 261: /* proplist: proplist ',' proppair  */
#line 756 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4053 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 262: /* proplist: proplist ',' expr  */
#line 759 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 263: /* proppair: tSYMBOL ':' expr  */
#line 764 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4067 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 264: /* proppair: ID ':' expr  */
#line 765 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4073 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 265: /* proppair: tSTRING ':' expr  */
#line 766 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* proppair: tINT ':' expr  */
#line 767 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new IntNode((yyvsp[-2].i)), (yyvsp[0].node)); }
#line 4085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* proppair: tFLOAT ':' expr  */
#line 768 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new FloatNode((yyvsp[-2].f)), (yyvsp[0].node)); }
#line 4091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* unarymath: '+' simpleexpr  */
#line 771 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 4097 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* unarymath: '-' simpleexpr  */
#line 772 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 4103 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* expr: expr '+' expr  */
#line 782 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4109 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* expr: expr '-' expr  */
#line 783 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4115 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* expr: expr '*' expr  */
#line 784 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4121 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* expr: expr '/' expr  */
#line 785 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4127 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* expr: expr tMOD expr  */
#line 786 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* expr: expr '>' expr  */
#line 787 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4139 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* expr: expr '<' expr  */
#line 788 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4145 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* expr: expr tEQ expr  */
#line 789 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4151 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* expr: expr tNEQ expr  */
#line 790 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4157 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 283: /* expr: expr tGE expr  */
#line 791 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4163 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 284: /* expr: expr tLE expr  */
#line 792 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4169 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* expr: expr tAND expr  */
#line 793 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4175 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* expr: expr tOR expr  */
#line 794 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4181 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 287: /* expr: expr '&' expr  */
#line 795 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4187 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 288: /* expr: expr tCONCAT expr  */
#line 796 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4193 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 289: /* expr: expr tCONTAINS expr  */
#line 797 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4199 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 290: /* expr: expr tSTARTS expr  */
#line 798 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 807 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4211 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 294: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 808 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4217 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 295: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 809 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4223 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 296: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 810 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4229 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 811 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4235 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 812 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4241 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 813 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 814 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4253 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 301: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 815 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 302: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 816 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 303: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 817 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4271 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 304: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 818 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 819 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4283 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4289 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 307: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 821 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 308: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4301 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 309: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 823 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 312: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 828 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 313: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 829 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4319 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 314: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 830 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4325 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 315: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 831 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4331 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 832 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4337 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 833 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 834 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4349 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 835 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4355 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 836 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4361 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 837 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4367 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 322: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 838 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4373 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 323: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 839 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 324: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 840 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4385 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 325: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 841 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4391 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 326: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 842 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4397 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 327: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 843 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4403 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 328: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 846 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4409 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 329: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 847 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4415 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 330: /* exprlist: %empty  */
#line 850 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 4421 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 332: /* nonemptyexprlist: expr  */
#line 854 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4430 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 333: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 858 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4438 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 4442 "engines/director/lingo/lingo-gr.cpp"

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

#line 863 "engines/director/lingo/lingo-gr.y"


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
