/* A Bison parser, made by GNU Bison 3.7.6.  */

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
#define YYBISON 30706

/* Bison version string.  */
#define YYBISON_VERSION "3.7.6"

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
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-object.h"

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

#include "lingo-gr.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_tUNARY = 3,                     /* tUNARY  */
  YYSYMBOL_tLEXERROR = 4,                  /* tLEXERROR  */
  YYSYMBOL_tINT = 5,                       /* tINT  */
  YYSYMBOL_tFLOAT = 6,                     /* tFLOAT  */
  YYSYMBOL_tVARID = 7,                     /* tVARID  */
  YYSYMBOL_tSTRING = 8,                    /* tSTRING  */
  YYSYMBOL_tSYMBOL = 9,                    /* tSYMBOL  */
  YYSYMBOL_tENDCLAUSE = 10,                /* tENDCLAUSE  */
  YYSYMBOL_tCAST = 11,                     /* tCAST  */
  YYSYMBOL_tFIELD = 12,                    /* tFIELD  */
  YYSYMBOL_tSCRIPT = 13,                   /* tSCRIPT  */
  YYSYMBOL_tWINDOW = 14,                   /* tWINDOW  */
  YYSYMBOL_tDOWN = 15,                     /* tDOWN  */
  YYSYMBOL_tELSE = 16,                     /* tELSE  */
  YYSYMBOL_tELSIF = 17,                    /* tELSIF  */
  YYSYMBOL_tEXIT = 18,                     /* tEXIT  */
  YYSYMBOL_tFRAME = 19,                    /* tFRAME  */
  YYSYMBOL_tGLOBAL = 20,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 21,                       /* tGO  */
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
  YYSYMBOL_tCASTMEMBERS = 68,              /* tCASTMEMBERS  */
  YYSYMBOL_tDATE = 69,                     /* tDATE  */
  YYSYMBOL_tLAST = 70,                     /* tLAST  */
  YYSYMBOL_tMENU = 71,                     /* tMENU  */
  YYSYMBOL_tMENUITEM = 72,                 /* tMENUITEM  */
  YYSYMBOL_tMENUITEMS = 73,                /* tMENUITEMS  */
  YYSYMBOL_tNUMBER = 74,                   /* tNUMBER  */
  YYSYMBOL_tTHE = 75,                      /* tTHE  */
  YYSYMBOL_tTIME = 76,                     /* tTIME  */
  YYSYMBOL_tSOUND = 77,                    /* tSOUND  */
  YYSYMBOL_tSPRITE = 78,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 79,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 80,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 81,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 82,                 /* tPROPERTY  */
  YYSYMBOL_tON = 83,                       /* tON  */
  YYSYMBOL_tMETHOD = 84,                   /* tMETHOD  */
  YYSYMBOL_tENDIF = 85,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 86,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 87,                  /* tENDTELL  */
  YYSYMBOL_tASSERTERROR = 88,              /* tASSERTERROR  */
  YYSYMBOL_89_ = 89,                       /* '<'  */
  YYSYMBOL_90_ = 90,                       /* '>'  */
  YYSYMBOL_91_ = 91,                       /* '&'  */
  YYSYMBOL_92_ = 92,                       /* '+'  */
  YYSYMBOL_93_ = 93,                       /* '-'  */
  YYSYMBOL_94_ = 94,                       /* '*'  */
  YYSYMBOL_95_ = 95,                       /* '/'  */
  YYSYMBOL_96_ = 96,                       /* '%'  */
  YYSYMBOL_97_n_ = 97,                     /* '\n'  */
  YYSYMBOL_98_ = 98,                       /* ','  */
  YYSYMBOL_99_ = 99,                       /* '('  */
  YYSYMBOL_100_ = 100,                     /* ')'  */
  YYSYMBOL_101_ = 101,                     /* '['  */
  YYSYMBOL_102_ = 102,                     /* ']'  */
  YYSYMBOL_103_ = 103,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 104,                 /* $accept  */
  YYSYMBOL_script = 105,                   /* script  */
  YYSYMBOL_scriptpartlist = 106,           /* scriptpartlist  */
  YYSYMBOL_scriptpart = 107,               /* scriptpart  */
  YYSYMBOL_macro = 108,                    /* macro  */
  YYSYMBOL_factory = 109,                  /* factory  */
  YYSYMBOL_method = 110,                   /* method  */
  YYSYMBOL_methodlist = 111,               /* methodlist  */
  YYSYMBOL_nonemptymethodlist = 112,       /* nonemptymethodlist  */
  YYSYMBOL_methodlistline = 113,           /* methodlistline  */
  YYSYMBOL_handler = 114,                  /* handler  */
  YYSYMBOL_endargdef = 115,                /* endargdef  */
  YYSYMBOL_ID = 116,                       /* ID  */
  YYSYMBOL_idlist = 117,                   /* idlist  */
  YYSYMBOL_nonemptyidlist = 118,           /* nonemptyidlist  */
  YYSYMBOL_stmt = 119,                     /* stmt  */
  YYSYMBOL_stmtoneliner = 120,             /* stmtoneliner  */
  YYSYMBOL_proc = 121,                     /* proc  */
  YYSYMBOL_cmdargs = 122,                  /* cmdargs  */
  YYSYMBOL_frameargs = 123,                /* frameargs  */
  YYSYMBOL_asgn = 124,                     /* asgn  */
  YYSYMBOL_to = 125,                       /* to  */
  YYSYMBOL_definevars = 126,               /* definevars  */
  YYSYMBOL_ifstmt = 127,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 128,               /* ifelsestmt  */
  YYSYMBOL_loop = 129,                     /* loop  */
  YYSYMBOL_tell = 130,                     /* tell  */
  YYSYMBOL_when = 131,                     /* when  */
  YYSYMBOL_stmtlist = 132,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 133,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 134,             /* stmtlistline  */
  YYSYMBOL_simpleexpr_noparens_nounarymath = 135, /* simpleexpr_noparens_nounarymath  */
  YYSYMBOL_var = 136,                      /* var  */
  YYSYMBOL_varorchunk = 137,               /* varorchunk  */
  YYSYMBOL_varorthe = 138,                 /* varorthe  */
  YYSYMBOL_chunk = 139,                    /* chunk  */
  YYSYMBOL_object = 140,                   /* object  */
  YYSYMBOL_list = 141,                     /* list  */
  YYSYMBOL_proplist = 142,                 /* proplist  */
  YYSYMBOL_proppair = 143,                 /* proppair  */
  YYSYMBOL_parens = 144,                   /* parens  */
  YYSYMBOL_unarymath = 145,                /* unarymath  */
  YYSYMBOL_simpleexpr = 146,               /* simpleexpr  */
  YYSYMBOL_simpleexpr_noparens = 147,      /* simpleexpr_noparens  */
  YYSYMBOL_simpleexpr_nounarymath = 148,   /* simpleexpr_nounarymath  */
  YYSYMBOL_expr = 149,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 150,         /* expr_nounarymath  */
  YYSYMBOL_sprite = 151,                   /* sprite  */
  YYSYMBOL_exprlist = 152,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 153          /* nonemptyexprlist  */
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
#define YYFINAL  143
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3493

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  218
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  393

/* YYMAXUTOK -- Last valid token kind.  */
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
      99,   100,    94,    92,    98,    93,     2,    95,     2,     2,
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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   197,   197,   199,   205,   212,   213,   214,   215,   216,
     245,   249,   251,   253,   254,   257,   263,   270,   271,   276,
     280,   284,   285,   286,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   308,   309,   313,
     315,   316,   317,   318,   319,   320,   321,   322,   324,   325,
     326,   328,   329,   330,   331,   332,   333,   335,   336,   338,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   352,
     353,   355,   356,   357,   358,   359,   360,   363,   364,   367,
     371,   381,   382,   383,   384,   385,   386,   389,   390,   391,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     406,   409,   414,   418,   424,   429,   432,   442,   447,   453,
     459,   465,   473,   474,   475,   476,   479,   479,   481,   482,
     483,   486,   490,   494,   500,   504,   508,   512,   514,   516,
     520,   524,   528,   530,   531,   535,   541,   548,   549,   555,
     556,   557,   558,   559,   560,   561,   562,   563,   564,   567,
     569,   570,   573,   577,   581,   587,   591,   597,   598,   599,
     602,   606,   611,   612,   613,   616,   618,   619,   622,   623,
     624,   627,   628,   631,   632,   636,   637,   638,   639,   640,
     641,   642,   643,   644,   645,   646,   647,   648,   649,   650,
     651,   652,   653,   654,   661,   662,   663,   664,   665,   666,
     667,   668,   669,   670,   671,   672,   673,   674,   675,   676,
     677,   678,   679,   682,   683,   686,   687,   690,   694
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
  "end of file", "error", "invalid token", "tUNARY", "tLEXERROR", "tINT",
  "tFLOAT", "tVARID", "tSTRING", "tSYMBOL", "tENDCLAUSE", "tCAST",
  "tFIELD", "tSCRIPT", "tWINDOW", "tDOWN", "tELSE", "tELSIF", "tEXIT",
  "tFRAME", "tGLOBAL", "tGO", "tIF", "tIN", "tINTO", "tMACRO", "tMOVIE",
  "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO",
  "tWHEN", "tWITH", "tWHILE", "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE",
  "tGE", "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT", "tMOD", "tAFTER",
  "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR", "tCHARS", "tITEM",
  "tITEMS", "tLINE", "tLINES", "tWORD", "tWORDS", "tABBREVIATED",
  "tABBREV", "tABBR", "tLONG", "tSHORT", "tCASTMEMBERS", "tDATE", "tLAST",
  "tMENU", "tMENUITEM", "tMENUITEMS", "tNUMBER", "tTHE", "tTIME", "tSOUND",
  "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON",
  "tMETHOD", "tENDIF", "tENDREPEAT", "tENDTELL", "tASSERTERROR", "'<'",
  "'>'", "'&'", "'+'", "'-'", "'*'", "'/'", "'%'", "'\\n'", "','", "'('",
  "')'", "'['", "']'", "':'", "$accept", "script", "scriptpartlist",
  "scriptpart", "macro", "factory", "method", "methodlist",
  "nonemptymethodlist", "methodlistline", "handler", "endargdef", "ID",
  "idlist", "nonemptyidlist", "stmt", "stmtoneliner", "proc", "cmdargs",
  "frameargs", "asgn", "to", "definevars", "ifstmt", "ifelsestmt", "loop",
  "tell", "when", "stmtlist", "nonemptystmtlist", "stmtlistline",
  "simpleexpr_noparens_nounarymath", "var", "varorchunk", "varorthe",
  "chunk", "object", "list", "proplist", "proppair", "parens", "unarymath",
  "simpleexpr", "simpleexpr_noparens", "simpleexpr_nounarymath", "expr",
  "expr_nounarymath", "sprite", "exprlist", "nonemptyexprlist", YY_NULLPTR
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
      62,    38,    43,    45,    42,    47,    37,    10,    44,    40,
      41,    91,    93,    58
};
#endif

#define YYPACT_NINF (-297)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1883,  -297,  -297,  -297,  -297,  -297,  -297,   -21,  -297,  3086,
    1119,  1216,  -297,  -297,  3086,  -297,   -12,  -297,  -297,  1313,
      57,  3086,  -297,  3086,  -297,  -297,  3086,  -297,  1119,  3086,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  1216,  3086,  3086,  2930,  -297,    37,  1883,  -297,
    -297,  -297,  -297,  1313,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,   -54,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,   -47,   -44,  -297,  -297,  -297,  -297,  1507,  1507,
    1507,  1507,  1216,  1216,  1410,  1216,  1410,  1410,  1022,   535,
     -40,   -31,    -1,  -297,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,   634,  -297,  1216,   205,  3086,    10,    20,   731,  3086,
    1216,  -297,  -297,   -16,    36,    24,    25,    26,    27,   124,
      29,  3086,  -297,  -297,  -297,    31,   828,  -297,  -297,  3086,
    -297,  -297,  -297,  -297,  -297,  -297,   925,  3372,  -297,  3318,
    -297,  -297,  -297,   421,   -22,    28,   -11,   -83,   -62,  -297,
    3372,    32,    45,  1216,  -297,  -297,   104,  1216,  1216,  1216,
    1216,  1598,  1598,  1792,  1216,  1695,  1695,  1216,  1216,  1216,
    1216,  1216,  1216,  1216,  1216,  -297,  -297,  -297,  3204,  -297,
     222,  2610,  1216,  1216,  1216,  1216,  1216,    47,  -297,  -297,
    3164,  3164,  3164,   106,  3217,  -297,  -297,  1216,  1216,   -59,
    -297,  -297,  -297,  2930,  2690,  -297,    51,  -297,  -297,   125,
    3385,  1410,  1410,  1216,  -297,  1216,  1216,  -297,  1216,  3008,
    -297,  -297,  1216,    52,  1216,     9,     9,     9,     9,  3398,
    3398,  -297,    -8,     9,     9,     9,     9,    -8,   -38,   -38,
    -297,  -297,    45,  1216,  1216,  1216,  1216,  1216,  1216,  1216,
    1216,  1216,  1216,  1216,  1216,  1216,  1216,  1216,  1216,  1216,
    1216,  2690,   137,  2690,  -297,    58,  -297,    59,    60,  1216,
    2690,  3230,  3372,  3086,  -297,  -297,    62,  -297,  -297,  -297,
    -297,  -297,  2043,  -297,  -297,  2690,  1216,  -297,  -297,   -18,
    3372,  3372,  3372,   -22,    28,    61,  -297,  3372,  -297,  3372,
       9,     9,     9,     9,  3398,  3398,  -297,    -8,     9,     9,
       9,     9,    -8,   -38,   -38,  -297,  -297,    45,  1963,  2770,
    2690,  -297,  -297,  -297,    93,  2130,  -297,  3086,   -59,    63,
    -297,  2210,  3372,  -297,  2850,    64,  2690,  -297,   120,  1216,
      66,    68,  -297,  -297,  3086,  2690,  -297,  -297,  2290,  1216,
    3286,  -297,  2690,    13,  -297,  2370,    75,  3299,  2690,  2690,
    -297,  3086,    77,  -297,  2690,  2450,  -297,  -297,  2530,    78,
      82,  -297,  -297
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    24,    31,    37,    61,    72,    36,     0,    38,    77,
     100,     0,    39,    41,     0,    52,    53,    56,    59,   100,
      60,    63,    70,     0,    73,    71,     0,    57,   100,    77,
      29,    58,    54,    51,    28,    30,    34,    67,    32,    33,
      42,    43,    45,    46,    75,    76,    25,    26,    27,    47,
      64,    35,    44,    48,    49,    50,    55,    69,    65,    66,
      40,    74,    68,    77,     0,    62,     5,     0,     2,     3,
       6,     7,     8,   100,     9,    81,    87,    88,    89,    82,
      83,    84,    85,    86,     0,    98,    53,    60,    63,    68,
      62,    79,     0,    78,   139,   140,   142,   141,    31,    37,
      61,    72,    38,    52,    54,    66,     0,     0,     0,   215,
     149,     0,     0,   168,   145,   146,   147,   148,   169,   170,
     175,   101,   176,     0,     0,    77,     0,     0,   101,     0,
       0,   149,   152,     0,     0,     0,     0,     0,     0,     0,
       0,    77,    99,     1,     4,     0,   101,    97,   118,     0,
     171,   172,   154,   153,   155,   156,   107,   108,   143,     0,
     166,   167,   105,     0,   142,   141,     0,   149,     0,   160,
     217,     0,   216,   215,    92,    93,    56,     0,     0,     0,
       0,    29,    58,    51,     0,    34,    67,     0,     0,     0,
       0,     0,     0,     0,     0,   173,   174,   194,   103,   195,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    91,
      41,    28,    30,     0,     0,   116,   117,     0,     0,    13,
      94,    95,   120,     0,   133,   119,     0,    90,    80,    56,
     111,     0,     0,     0,   165,     0,     0,   158,     0,     0,
     159,   157,     0,     0,     0,   186,   187,   184,   185,   188,
     189,   181,   191,   192,   193,   183,   182,   190,   177,   178,
     179,   180,   102,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   133,   121,   133,   150,     0,   151,     0,     0,     0,
     133,     0,   132,     0,    17,    18,    11,    14,    15,   130,
     137,   138,     0,   134,   135,   133,     0,   213,   214,     0,
     164,   162,   163,     0,     0,     0,   161,   218,   144,   110,
     205,   206,   203,   204,   207,   208,   200,   210,   211,   212,
     202,   201,   209,   196,   197,   198,   199,   104,     0,     0,
      10,   112,   113,   114,     0,     0,   115,    77,     0,     0,
     136,    20,   109,   106,     0,     0,   133,   123,     0,     0,
       0,     0,    16,   131,    21,   133,   125,   122,     0,     0,
       0,   127,   133,     0,    22,     0,     0,     0,   133,    12,
      19,     0,     0,   124,   133,     0,    23,   126,     0,     0,
       0,   128,   129
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -297,  -297,  -297,   112,  -297,  -297,  -297,  -297,  -297,  -167,
    -297,  -297,     0,   -28,  -297,     3,   -61,  -297,    -6,   161,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -266,  -297,
    -296,   -68,   -19,   -92,  -297,  -137,  -297,  -297,  -297,   -49,
     -94,   -53,   -99,    15,  -297,   218,    38,   -51,    22,  -182
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    67,    68,    69,    70,    71,   295,   296,   297,   298,
      72,   373,   110,    92,    93,   301,    75,    76,   111,   112,
      77,   217,    78,    79,    80,    81,    82,    83,   302,   303,
     304,   113,   114,   285,   133,   115,   116,   117,   168,   169,
     118,   119,   120,   152,   197,   170,   198,   122,   171,   172
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      73,   138,   132,    74,   142,   158,   350,   160,   161,    91,
      84,   204,   262,   127,   125,   338,   173,   340,   215,   126,
     238,   131,   136,   134,   345,   293,   135,   196,   216,    91,
     150,   150,   150,   150,   196,   140,   239,   143,   294,   351,
     240,   204,   350,   147,   350,   151,   151,   151,   151,   350,
     148,   309,   196,   195,   149,   350,   192,   193,   204,   173,
     195,   184,   196,    91,   141,    73,   174,   145,    73,   218,
     199,    74,   350,   286,   286,   286,    85,   199,   195,   350,
     242,   235,   353,   350,   190,   191,   192,   193,   195,   350,
     368,   237,   350,   129,   130,   199,   175,   207,   337,   375,
     189,   190,   191,   192,   193,   199,   379,   208,   358,   167,
     380,   381,   385,   226,   153,   154,   155,   209,   388,   287,
     288,   219,   220,   221,   222,    91,   225,   359,   227,   213,
     244,   236,   307,   308,   241,   177,   178,   179,   180,   202,
     203,    91,   204,   242,   283,   184,   205,   206,   305,   228,
     289,   306,   318,   339,   369,   341,   342,   343,   223,   348,
     363,   367,   299,   371,   238,   372,   177,   178,   179,   180,
     202,   203,   383,   204,   387,   391,   184,   205,   206,   392,
     144,   362,   187,   188,   189,   190,   191,   192,   193,   137,
     316,   284,   284,   284,   230,   243,     0,     0,     0,     0,
       0,    73,     0,     0,   282,     0,     0,     0,     0,     0,
     131,   131,   131,   187,   188,   189,   190,   191,   192,   193,
       0,   224,     0,    73,    73,     0,     0,     0,   121,   124,
       0,     0,     0,     0,     0,     0,     0,   128,   201,   315,
       0,     0,     0,     0,     0,     0,   121,   177,   178,   179,
     180,   202,   203,     0,   204,     0,     0,   184,   205,   206,
       0,     0,     0,     0,   177,   178,   179,   180,   202,   203,
       0,   204,     0,     0,   184,   205,   206,     0,     0,     0,
     139,    73,     0,    73,     0,     0,     0,     0,     0,     0,
      73,   146,     0,   347,   187,   188,   189,   190,   191,   192,
     193,     0,    73,     0,     0,    73,     0,     0,     0,     0,
       0,   187,   188,   189,   190,   191,   192,   193,     0,   361,
     156,   157,   234,   159,     0,     0,   163,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    73,
      73,   200,   357,     0,     0,    73,     0,    91,   214,     0,
       0,    73,     0,     0,    73,     0,    73,   366,     0,     0,
       0,     0,     0,     0,   374,    73,     0,     0,    73,     0,
       0,     0,    73,     0,     0,    73,     0,     0,    73,    73,
       0,   386,     0,     0,    73,    73,     0,     0,    73,     0,
       0,     0,     0,     0,     0,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,     0,     0,     0,     0,     0,     0,     0,     0,
     249,   250,   251,   253,   254,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   291,   292,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   310,   311,     0,   312,     0,     0,     0,
     317,     0,   319,   177,   178,   179,   180,   202,   203,     0,
     204,     0,     0,   184,   205,   206,     0,     0,     0,     0,
       0,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   344,     0,     0,
     187,   188,   189,   190,   191,   192,   193,     0,     0,   233,
       0,   234,     0,     0,   352,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      94,    95,     1,   164,   165,     0,    98,    99,   100,   101,
       6,     0,     0,     0,     8,     0,     0,     0,    12,    13,
       0,    15,    86,    17,    18,     0,    87,    88,     0,    22,
       0,    24,    25,     0,    27,     0,     0,   370,     0,     0,
       0,    30,    31,   104,    33,    34,    35,   377,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,    56,
       0,    57,    58,   105,    60,    61,    89,     0,     0,     0,
       0,     0,     0,    90,     0,     0,     0,   106,   107,     0,
       0,     0,     0,     0,   123,     0,   109,     0,   166,    94,
      95,     1,    96,    97,     0,    98,    99,   100,   101,     6,
       0,     0,     0,     8,     0,     0,     0,    12,    13,     0,
      15,    86,   176,    18,     0,    87,    88,     0,    22,     0,
      24,    25,     0,    27,     0,     0,   177,   178,   179,   180,
     181,   182,   104,   183,    34,    35,   184,   185,   186,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,     0,
      57,    58,   105,    60,    61,    89,     0,     0,     0,     0,
       0,     0,    90,   187,   188,   189,   190,   191,   192,   193,
       0,     0,   194,   123,     0,   109,    94,    95,     1,    96,
      97,     0,    98,    99,   100,   101,     6,     0,     0,     0,
       8,     0,     0,     0,    12,   210,     0,    15,    86,    17,
      18,     0,    87,    88,     0,    22,     0,    24,    25,     0,
      27,     0,     0,   177,   178,   179,   180,   181,   182,   104,
     183,   211,   212,   184,   185,   186,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,    54,    55,    56,     0,    57,    58,   105,
      60,    61,    89,     0,     0,     0,     0,     0,     0,    90,
     187,   188,   189,   190,   191,   192,   193,     0,     0,   194,
     123,     0,   109,    94,    95,     1,    96,    97,     0,    98,
      99,   100,   101,     6,     0,     0,     0,     8,     0,     0,
       0,    12,    13,     0,    15,    86,    17,    18,     0,    87,
      88,     0,    22,     0,    24,    25,     0,    27,     0,     0,
     177,   178,   179,   180,   181,   182,   104,   183,    34,    35,
     184,   185,   186,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,    56,     0,    57,    58,   105,    60,    61,    89,
       0,     0,     0,     0,     0,     0,    90,   187,   188,   189,
     190,   191,   192,   193,     0,     0,   194,   123,     0,   109,
      94,    95,     1,    96,    97,     0,    98,    99,   100,   101,
       6,     0,     0,     0,     8,     0,     0,     0,    12,    13,
       0,    15,    86,   229,    18,     0,    87,    88,     0,    22,
       0,    24,    25,     0,    27,     0,     0,   177,   178,   179,
     180,   181,   182,   104,   183,    34,    35,   184,   185,   186,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,    56,
       0,    57,    58,   105,    60,    61,    89,     0,     0,     0,
       0,     0,     0,    90,   187,   188,   189,   190,   191,   192,
     193,     0,     0,     0,   123,     0,   109,    94,    95,     1,
      96,    97,     0,    98,    99,   100,   101,     6,     0,     0,
       0,     8,     0,     0,     0,    12,    13,     0,    15,    86,
      17,    18,     0,    87,    88,     0,    22,     0,    24,    25,
       0,    27,     0,     0,     0,     0,     0,     0,    30,    31,
     104,    33,    34,    35,     0,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,    56,     0,    57,    58,
     105,    60,    61,    89,     0,     0,     0,     0,     0,     0,
      90,     0,     0,     0,   106,   107,     0,     0,     0,     0,
       0,   123,   162,   109,    94,    95,     1,    96,    97,     0,
      98,    99,   100,   101,     6,     0,     0,     0,   102,     0,
       0,     0,    12,    13,     0,   103,    86,    17,    18,     0,
      87,    88,     0,    22,     0,    24,    25,     0,    27,     0,
       0,     0,     0,     0,     0,    30,    31,   104,    33,    34,
      35,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,    54,    55,    56,     0,    57,    58,   105,    60,    61,
      89,     0,     0,     0,     0,     0,     0,    90,     0,     0,
       0,   106,   107,     0,     0,     0,     0,     0,   108,     0,
     109,    94,    95,     1,    96,    97,     0,    98,    99,   100,
     101,     6,     0,     0,     0,     8,     0,     0,     0,    12,
      13,     0,    15,    86,    17,    18,     0,    87,    88,     0,
      22,     0,    24,    25,     0,    27,     0,     0,     0,     0,
       0,     0,    30,    31,   104,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,     0,    57,    58,   105,    60,    61,    89,     0,     0,
       0,     0,     0,     0,    90,     0,     0,     0,   106,   107,
       0,     0,     0,     0,     0,   123,     0,   109,    94,    95,
       1,    96,    97,     0,    98,    99,   100,   101,     6,     0,
       0,     0,     8,     0,     0,     0,    12,    13,     0,    15,
      86,    17,    18,     0,    87,    88,     0,    22,     0,    24,
      25,     0,    27,     0,     0,     0,     0,     0,     0,    30,
      31,   104,    33,    34,    35,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,     0,    57,
      58,   105,    60,    61,    89,     0,     0,     0,     0,     0,
       0,    90,     0,     0,     0,   106,   107,     0,     0,     0,
       0,     0,   108,     0,   109,    94,    95,     1,    96,    97,
       0,    98,    99,   100,   101,     6,     0,     0,     0,     8,
       0,     0,     0,    12,    13,     0,    15,    86,    17,    18,
       0,    87,    88,     0,    22,     0,    24,    25,     0,    27,
       0,     0,     0,     0,     0,     0,    30,    31,   104,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    89,     0,     0,     0,     0,     0,     0,    90,     0,
       0,     0,   106,   107,     0,     0,     0,     0,     0,   123,
       0,   109,    94,    95,     1,    96,    97,     0,    98,    99,
     100,   101,     6,     0,     0,     0,     8,     0,     0,     0,
      12,    13,     0,    15,    86,    17,    18,     0,    87,    88,
       0,    22,     0,    24,    25,     0,    27,     0,     0,     0,
       0,     0,     0,    30,    31,   104,    33,    34,    35,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,    60,    61,    89,     0,
       0,     0,     0,     0,     0,    90,     0,     0,     0,   106,
     107,     0,     0,    94,    95,     1,    96,    97,   109,    98,
      99,   100,   101,     6,     0,     0,     0,     8,     0,     0,
       0,    12,    13,     0,    15,    86,    17,    18,     0,    87,
      88,     0,    22,     0,    24,    25,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,   104,    33,    34,    35,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,    56,     0,    57,    58,   105,    60,    61,    89,
       0,     0,     0,     0,     0,     0,    90,     0,     0,     0,
     106,   107,     0,     0,     0,     0,     0,   123,     0,   109,
      94,    95,     1,    96,    97,     0,    98,    99,   100,   101,
       6,     0,     0,     0,     8,     0,     0,     0,    12,    13,
       0,    15,    86,    17,    18,     0,    87,    88,     0,    22,
       0,    24,    25,     0,    27,     0,     0,     0,     0,     0,
       0,     0,     0,   104,    33,    34,    35,     0,     0,     0,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,    56,
       0,    57,    58,   105,    60,    61,    89,     0,     0,     0,
       0,     0,     0,    90,     0,     0,     0,   106,   107,     0,
       0,     0,     0,     0,   123,     0,   109,    94,    95,     1,
      96,    97,     0,    98,    99,   100,   101,     6,     0,     0,
       0,     8,     0,     0,     0,    12,    13,     0,    15,    86,
      17,    18,     0,    87,    88,     0,    22,     0,    24,    25,
       0,    27,     0,     0,     0,     0,     0,     0,     0,     0,
     104,     0,    34,    35,     0,     0,     0,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,    56,     0,    57,    58,
     105,    60,    61,    89,     0,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       1,   123,     0,   109,     2,     3,     4,     5,     6,     0,
       0,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,     0,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    30,
      31,    32,    33,    34,    35,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,    64,     0,     0,     0,
       1,    65,     0,     0,     2,     3,     4,     5,     6,   354,
      66,     7,     8,     9,    10,    11,    12,    13,     0,    15,
      16,    17,    18,    19,    20,    21,     0,    22,    23,    24,
      25,     0,    27,    28,    29,     0,     0,     0,     0,    30,
      31,    32,    33,    34,    35,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,     0,     0,   355,     0,
       1,    65,     0,     0,     2,     3,     4,     5,     6,     0,
     300,     7,     8,     9,    10,    11,    12,    13,     0,    15,
      16,    17,    18,    19,    20,    21,     0,    22,    23,    24,
      25,     0,    27,    28,    29,     0,     0,     0,     0,    30,
      31,    32,    33,    34,    35,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,     0,     0,     0,     0,
     349,    65,     0,     0,     0,     0,     0,     1,     0,     0,
     300,     2,     3,     4,     5,     6,     0,     0,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,   360,     1,    65,     0,
     364,     2,     3,     4,     5,     6,     0,   300,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   300,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,   376,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   300,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,   382,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   300,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,   389,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   300,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,   390,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   300,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   281,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   300,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   356,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   365,     7,     8,
       9,    10,     0,    12,    13,     0,    15,    16,    17,    18,
      19,    87,    21,     0,    22,     0,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    89,    63,     0,     0,     1,   313,   314,    65,     2,
       3,     4,     5,     6,     0,     0,     0,     8,     0,     0,
       0,    12,    13,     0,    15,    86,    17,    18,     0,    87,
      88,     0,    22,     0,    24,    25,     0,    27,     0,     0,
       0,     0,     0,     0,    30,    31,    32,    33,    34,    35,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,    56,     0,    57,    58,    59,    60,    61,    89,
       0,     0,     0,     1,     0,     0,    90,     2,     3,     4,
       5,     6,     0,     0,     0,     8,     0,     0,     0,    12,
      13,     0,    15,    86,    17,    18,     0,    87,    88,     0,
      22,     0,    24,    25,     0,    27,     0,     0,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,    60,    61,    89,     0,     0,
       0,     1,     0,     0,    90,    98,    99,     4,     5,     6,
       0,     0,     0,     8,     0,     0,     0,    12,    13,     0,
      15,    86,    17,    18,     0,    87,    88,     0,    22,     0,
      24,    25,     0,    27,     0,     0,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,    60,    61,    89,   263,   264,   265,   266,
     267,   268,    90,   269,     0,     0,   270,   271,   272,   177,
     178,   179,   180,   202,   203,     0,   204,     0,     0,   184,
     205,   206,   177,   178,   179,   180,   202,   203,     0,   204,
       0,     0,   184,   205,   206,     0,     0,     0,     0,     0,
       0,     0,     0,   273,   274,   275,   276,   277,   278,   279,
       0,     0,   280,     0,     0,     0,   187,   188,   189,   190,
     191,   192,   193,     0,   290,     0,     0,     0,     0,   187,
     188,   189,   190,   191,   192,   193,     0,   346,   177,   178,
     179,   180,   202,   203,     0,   204,     0,     0,   184,   205,
     206,   177,   178,   179,   180,   202,   203,     0,   204,     0,
       0,   184,   205,   206,     0,     0,     0,     0,     0,     0,
     177,   178,   179,   180,   202,   203,     0,   204,     0,     0,
     184,   205,   206,     0,     0,   187,   188,   189,   190,   191,
     192,   193,     0,   378,     0,     0,     0,     0,   187,   188,
     189,   190,   191,   192,   193,     0,   384,   231,   232,     0,
       0,     0,     0,     0,     0,     0,     0,   187,   188,   189,
     190,   191,   192,   193,   177,   178,   179,   180,   202,   203,
       0,   204,     0,     0,   184,   205,   206,   263,   264,   265,
     266,   267,   268,     0,   269,     0,     0,   270,   271,   272,
     177,   178,   179,   180,     0,     0,     0,   204,     0,     0,
     184,   205,   206,     0,     0,     0,     0,     0,     0,     0,
       0,   187,   188,   189,   190,   191,   192,   193,     0,     0,
       0,     0,     0,     0,   273,   274,   275,   276,   277,   278,
     279,     0,     0,     0,     0,     0,     0,   187,   188,   189,
     190,   191,   192,   193
};

static const yytype_int16 yycheck[] =
{
       0,    29,    21,     0,    65,   104,   302,   106,   107,     9,
      31,    49,   194,    19,    14,   281,    99,   283,    34,    31,
     103,    21,    28,    23,   290,    84,    26,   121,    44,    29,
      98,    99,   100,   101,   128,    63,    98,     0,    97,   305,
     102,    49,   338,    97,   340,    98,    99,   100,   101,   345,
      97,   233,   146,   121,    98,   351,    94,    95,    49,    99,
     128,    52,   156,    63,    64,    65,    97,    73,    68,    33,
     121,    68,   368,   210,   211,   212,    97,   128,   146,   375,
      98,   103,   100,   379,    92,    93,    94,    95,   156,   385,
     356,   102,   388,    36,    37,   146,    97,   125,   280,   365,
      91,    92,    93,    94,    95,   156,   372,    97,    15,   109,
      97,    98,   378,   141,    99,   100,   101,    97,   384,   211,
     212,    97,    97,    97,    97,   125,    97,    34,    97,   129,
      26,   103,   231,   232,   102,    42,    43,    44,    45,    46,
      47,   141,    49,    98,    97,    52,    53,    54,    97,   149,
      44,    26,   100,    16,    34,    97,    97,    97,    34,    97,
      97,    97,   223,    97,   103,    97,    42,    43,    44,    45,
      46,    47,    97,    49,    97,    97,    52,    53,    54,    97,
      68,   348,    89,    90,    91,    92,    93,    94,    95,    28,
     239,   210,   211,   212,   156,   173,    -1,    -1,    -1,    -1,
      -1,   201,    -1,    -1,   201,    -1,    -1,    -1,    -1,    -1,
     210,   211,   212,    89,    90,    91,    92,    93,    94,    95,
      -1,    97,    -1,   223,   224,    -1,    -1,    -1,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    33,   239,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      -1,    -1,    -1,    -1,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,
      62,   281,    -1,   283,    -1,    -1,    -1,    -1,    -1,    -1,
     290,    73,    -1,   293,    89,    90,    91,    92,    93,    94,
      95,    -1,   302,    -1,    -1,   305,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    -1,   347,
     102,   103,   100,   105,    -1,    -1,   108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   338,   339,
     340,   123,   339,    -1,    -1,   345,    -1,   347,   130,    -1,
      -1,   351,    -1,    -1,   354,    -1,   356,   354,    -1,    -1,
      -1,    -1,    -1,    -1,   364,   365,    -1,    -1,   368,    -1,
      -1,    -1,   372,    -1,    -1,   375,    -1,    -1,   378,   379,
      -1,   381,    -1,    -1,   384,   385,    -1,    -1,   388,    -1,
      -1,    -1,    -1,    -1,    -1,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     202,   203,   204,   205,   206,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   217,   218,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   235,   236,    -1,   238,    -1,    -1,    -1,
     242,    -1,   244,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,
      -1,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   289,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    98,
      -1,   100,    -1,    -1,   306,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       5,     6,     7,     8,     9,    -1,    11,    12,    13,    14,
      15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,
      -1,    26,    27,    28,    29,    -1,    31,    32,    -1,    34,
      -1,    36,    37,    -1,    39,    -1,    -1,   359,    -1,    -1,
      -1,    46,    47,    48,    49,    50,    51,   369,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      -1,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    -1,    -1,    99,    -1,   101,    -1,   103,     5,
       6,     7,     8,     9,    -1,    11,    12,    13,    14,    15,
      -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,
      26,    27,    28,    29,    -1,    31,    32,    -1,    34,    -1,
      36,    37,    -1,    39,    -1,    -1,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    -1,
      76,    77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      -1,    -1,    98,    99,    -1,   101,     5,     6,     7,     8,
       9,    -1,    11,    12,    13,    14,    15,    -1,    -1,    -1,
      19,    -1,    -1,    -1,    23,    24,    -1,    26,    27,    28,
      29,    -1,    31,    32,    -1,    34,    -1,    36,    37,    -1,
      39,    -1,    -1,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    73,    74,    -1,    76,    77,    78,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    98,
      99,    -1,   101,     5,     6,     7,     8,     9,    -1,    11,
      12,    13,    14,    15,    -1,    -1,    -1,    19,    -1,    -1,
      -1,    23,    24,    -1,    26,    27,    28,    29,    -1,    31,
      32,    -1,    34,    -1,    36,    37,    -1,    39,    -1,    -1,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    74,    -1,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    -1,    98,    99,    -1,   101,
       5,     6,     7,     8,     9,    -1,    11,    12,    13,    14,
      15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,
      -1,    26,    27,    28,    29,    -1,    31,    32,    -1,    34,
      -1,    36,    37,    -1,    39,    -1,    -1,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      -1,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    -1,    -1,    99,    -1,   101,     5,     6,     7,
       8,     9,    -1,    11,    12,    13,    14,    15,    -1,    -1,
      -1,    19,    -1,    -1,    -1,    23,    24,    -1,    26,    27,
      28,    29,    -1,    31,    32,    -1,    34,    -1,    36,    37,
      -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    73,    74,    -1,    76,    77,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,
      -1,    99,   100,   101,     5,     6,     7,     8,     9,    -1,
      11,    12,    13,    14,    15,    -1,    -1,    -1,    19,    -1,
      -1,    -1,    23,    24,    -1,    26,    27,    28,    29,    -1,
      31,    32,    -1,    34,    -1,    36,    37,    -1,    39,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    74,    -1,    76,    77,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,
      -1,    92,    93,    -1,    -1,    -1,    -1,    -1,    99,    -1,
     101,     5,     6,     7,     8,     9,    -1,    11,    12,    13,
      14,    15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,
      24,    -1,    26,    27,    28,    29,    -1,    31,    32,    -1,
      34,    -1,    36,    37,    -1,    39,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    -1,    76,    77,    78,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,
      -1,    -1,    -1,    -1,    -1,    99,    -1,   101,     5,     6,
       7,     8,     9,    -1,    11,    12,    13,    14,    15,    -1,
      -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,    26,
      27,    28,    29,    -1,    31,    32,    -1,    34,    -1,    36,
      37,    -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    -1,    76,
      77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,
      -1,    -1,    99,    -1,   101,     5,     6,     7,     8,     9,
      -1,    11,    12,    13,    14,    15,    -1,    -1,    -1,    19,
      -1,    -1,    -1,    23,    24,    -1,    26,    27,    28,    29,
      -1,    31,    32,    -1,    34,    -1,    36,    37,    -1,    39,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,
      -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    -1,    99,
      -1,   101,     5,     6,     7,     8,     9,    -1,    11,    12,
      13,    14,    15,    -1,    -1,    -1,    19,    -1,    -1,    -1,
      23,    24,    -1,    26,    27,    28,    29,    -1,    31,    32,
      -1,    34,    -1,    36,    37,    -1,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      73,    74,    -1,    76,    77,    78,    79,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    92,
      93,    -1,    -1,     5,     6,     7,     8,     9,   101,    11,
      12,    13,    14,    15,    -1,    -1,    -1,    19,    -1,    -1,
      -1,    23,    24,    -1,    26,    27,    28,    29,    -1,    31,
      32,    -1,    34,    -1,    36,    37,    -1,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    74,    -1,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,
      92,    93,    -1,    -1,    -1,    -1,    -1,    99,    -1,   101,
       5,     6,     7,     8,     9,    -1,    11,    12,    13,    14,
      15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,
      -1,    26,    27,    28,    29,    -1,    31,    32,    -1,    34,
      -1,    36,    37,    -1,    39,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    49,    50,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      -1,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    -1,    -1,    99,    -1,   101,     5,     6,     7,
       8,     9,    -1,    11,    12,    13,    14,    15,    -1,    -1,
      -1,    19,    -1,    -1,    -1,    23,    24,    -1,    26,    27,
      28,    29,    -1,    31,    32,    -1,    34,    -1,    36,    37,
      -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    73,    74,    -1,    76,    77,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       7,    99,    -1,   101,    11,    12,    13,    14,    15,    -1,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    -1,    76,
      77,    78,    79,    80,    81,    82,    83,    -1,    -1,    -1,
       7,    88,    -1,    -1,    11,    12,    13,    14,    15,    16,
      97,    18,    19,    20,    21,    22,    23,    24,    -1,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    36,
      37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    -1,    76,
      77,    78,    79,    80,    81,    82,    -1,    -1,    85,    -1,
       7,    88,    -1,    -1,    11,    12,    13,    14,    15,    -1,
      97,    18,    19,    20,    21,    22,    23,    24,    -1,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    36,
      37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    -1,    76,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      87,    88,    -1,    -1,    -1,    -1,    -1,     7,    -1,    -1,
      97,    11,    12,    13,    14,    15,    -1,    -1,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    86,     7,    88,    -1,
      10,    11,    12,    13,    14,    15,    -1,    97,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    -1,     7,    88,    -1,
      -1,    11,    12,    13,    14,    15,    -1,    97,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    85,    -1,     7,    88,    -1,
      -1,    11,    12,    13,    14,    15,    -1,    97,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    85,    -1,     7,    88,    -1,
      -1,    11,    12,    13,    14,    15,    -1,    97,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    86,     7,    88,    -1,
      -1,    11,    12,    13,    14,    15,    -1,    97,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    86,     7,    88,    -1,
      -1,    11,    12,    13,    14,    15,    -1,    97,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    -1,     7,    88,    -1,
      -1,    11,    12,    13,    14,    15,    -1,    97,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    -1,     7,    88,    -1,
      -1,    11,    12,    13,    14,    15,    -1,    97,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    -1,     7,    88,    -1,
      -1,    11,    12,    13,    14,    15,    -1,    97,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    -1,     7,    88,    -1,
      -1,    11,    12,    13,    14,    15,    -1,    97,    18,    19,
      20,    21,    -1,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    -1,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,     7,     8,     9,    88,    11,
      12,    13,    14,    15,    -1,    -1,    -1,    19,    -1,    -1,
      -1,    23,    24,    -1,    26,    27,    28,    29,    -1,    31,
      32,    -1,    34,    -1,    36,    37,    -1,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    74,    -1,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,     7,    -1,    -1,    88,    11,    12,    13,
      14,    15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,
      24,    -1,    26,    27,    28,    29,    -1,    31,    32,    -1,
      34,    -1,    36,    37,    -1,    39,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    -1,    76,    77,    78,    79,    80,    81,    -1,    -1,
      -1,     7,    -1,    -1,    88,    11,    12,    13,    14,    15,
      -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,
      26,    27,    28,    29,    -1,    31,    32,    -1,    34,    -1,
      36,    37,    -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    -1,
      76,    77,    78,    79,    80,    81,    42,    43,    44,    45,
      46,    47,    88,    49,    -1,    -1,    52,    53,    54,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      -1,    -1,    98,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    -1,    97,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    79,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      42,    43,    44,    45,    -1,    -1,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     7,    11,    12,    13,    14,    15,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    34,    35,    36,    37,    38,    39,    40,    41,
      46,    47,    48,    49,    50,    51,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    69,    70,    71,    72,    73,    74,    76,    77,    78,
      79,    80,    81,    82,    83,    88,    97,   105,   106,   107,
     108,   109,   114,   116,   119,   120,   121,   124,   126,   127,
     128,   129,   130,   131,    31,    97,    27,    31,    32,    81,
      88,   116,   117,   118,     5,     6,     8,     9,    11,    12,
      13,    14,    19,    26,    48,    78,    92,    93,    99,   101,
     116,   122,   123,   135,   136,   139,   140,   141,   144,   145,
     146,   149,   151,    99,   149,   116,    31,   122,   149,    36,
      37,   116,   136,   138,   116,   116,   122,   123,   117,   149,
     117,   116,   120,     0,   107,   122,   149,    97,    97,    98,
     135,   145,   147,   147,   147,   147,   149,   149,   146,   149,
     146,   146,   100,   149,     8,     9,   103,   116,   142,   143,
     149,   152,   153,    99,    97,    97,    28,    42,    43,    44,
      45,    46,    47,    49,    52,    53,    54,    89,    90,    91,
      92,    93,    94,    95,    98,   135,   144,   148,   150,   151,
     149,    33,    46,    47,    49,    53,    54,   117,    97,    97,
      24,    50,    51,   116,   149,    34,    44,   125,    33,    97,
      97,    97,    97,    34,    97,    97,   117,    97,   116,    28,
     150,    79,    80,    98,   100,   103,   103,   102,   103,    98,
     102,   102,    98,   152,    26,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   153,    42,    43,    44,    45,    46,    47,    49,
      52,    53,    54,    89,    90,    91,    92,    93,    94,    95,
      98,    97,   119,    97,   136,   137,   139,   137,   137,    44,
      97,   149,   149,    84,    97,   110,   111,   112,   113,   120,
      97,   119,   132,   133,   134,    97,    26,   146,   146,   153,
     149,   149,   149,     8,     9,   116,   143,   149,   100,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   153,   132,    16,
     132,    97,    97,    97,   149,   132,    97,   116,    97,    87,
     134,   132,   149,   100,    16,    85,    97,   119,    15,    34,
      86,   117,   113,    97,    10,    97,   119,    97,   132,    34,
     149,    97,    97,   115,   116,   132,    85,   149,    97,   132,
      97,    98,    85,    97,    97,   132,   116,    97,   132,    86,
      86,    97,    97
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   104,   105,   106,   106,   107,   107,   107,   107,   107,
     108,   109,   110,   111,   111,   112,   112,   113,   113,   114,
     114,   115,   115,   115,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   117,   117,   118,
     118,   119,   119,   119,   119,   119,   119,   120,   120,   120,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     122,   122,   122,   122,   122,   122,   122,   123,   123,   123,
     123,   123,   124,   124,   124,   124,   125,   125,   126,   126,
     126,   127,   127,   128,   128,   128,   128,   129,   129,   129,
     130,   130,   131,   132,   132,   133,   133,   134,   134,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   136,
     137,   137,   138,   139,   139,   140,   140,   141,   141,   141,
     142,   142,   143,   143,   143,   144,   145,   145,   146,   146,
     146,   147,   147,   148,   148,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   151,   151,   152,   152,   153,   153
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       5,     4,     5,     0,     1,     1,     3,     1,     1,     8,
       5,     0,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       0,     1,     3,     2,     4,     2,     5,     2,     2,     5,
       4,     3,     5,     5,     5,     5,     1,     1,     3,     3,
       3,     4,     7,     6,     9,     7,    10,     7,    11,    12,
       4,     6,     4,     0,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     2,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     3,     3,     3,
       1,     3,     3,     3,     3,     3,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     4,     4,     0,     1,     1,     3
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
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
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
#line 191 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1989 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 191 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1995 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 191 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2001 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 191 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2007 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 191 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2013 "engines/director/lingo/lingo-gr.cpp"
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
  case 2: /* script: scriptpartlist  */
#line 197 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->_compiler->_assemblyAST = new ScriptNode((yyvsp[0].nodelist)); }
#line 2281 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 199 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2292 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 205 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2302 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 212 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2308 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 245 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2314 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* factory: tFACTORY ID '\n' methodlist  */
#line 249 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2320 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 251 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2326 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* methodlist: %empty  */
#line 253 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2332 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* nonemptymethodlist: methodlistline  */
#line 257 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlist '\n' methodlistline  */
#line 263 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 2353 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlistline: '\n'  */
#line 270 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2359 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 276 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2368 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* handler: tON ID idlist '\n' stmtlist  */
#line 280 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2375 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* endargdef: ID  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2381 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* endargdef: endargdef ',' ID  */
#line 286 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2387 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* ID: tABBREVIATED  */
#line 292 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2393 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* ID: tABBREV  */
#line 293 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2399 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* ID: tABBR  */
#line 294 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2405 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* ID: tAFTER  */
#line 295 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2411 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* ID: tAND  */
#line 296 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 2417 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* ID: tBEFORE  */
#line 297 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2423 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* ID: tCAST  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2429 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* ID: tCHAR  */
#line 299 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2435 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* ID: tCHARS  */
#line 300 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* ID: tCONTAINS  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("contains"); }
#line 2447 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* ID: tDATE  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2453 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* ID: tDOWN  */
#line 303 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2459 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* ID: tFIELD  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2465 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* ID: tFRAME  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2471 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* ID: tIN  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2477 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* ID: tINTERSECTS  */
#line 315 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2483 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* ID: tINTO  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2489 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* ID: tITEM  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* ID: tITEMS  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2501 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* ID: tLAST  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2507 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* ID: tLINE  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2513 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* ID: tLINES  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2519 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* ID: tLONG  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* ID: tMENU  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2531 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* ID: tMENUITEM  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2537 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* ID: tMENUITEMS  */
#line 326 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2543 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* ID: tMOD  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2549 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* ID: tMOVIE  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2555 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* ID: tNEXT  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2561 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* ID: tNOT  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2567 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* ID: tNUMBER  */
#line 332 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2573 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* ID: tOF  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2579 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* ID: tOPEN  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 2585 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* ID: tOR  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2591 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* ID: tPREVIOUS  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2597 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* ID: tREPEAT  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2603 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* ID: tSCRIPT  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2609 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* ID: tASSERTERROR  */
#line 343 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2615 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* ID: tSET  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 2621 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* ID: tSHORT  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2627 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* ID: tSOUND  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2633 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* ID: tSPRITE  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2639 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* ID: tSTARTS  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2645 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* ID: tTELL  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 2651 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* ID: tTIME  */
#line 352 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 2657 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* ID: tTO  */
#line 353 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2663 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* ID: tWHILE  */
#line 355 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 2669 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* ID: tWINDOW  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2675 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* ID: tWITH  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 2681 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* ID: tWITHIN  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 2687 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* ID: tWORD  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 2693 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* ID: tWORDS  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 2699 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* idlist: %empty  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 2705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* nonemptyidlist: ID  */
#line 367 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 2714 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 371 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 2722 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* proc: ID cmdargs '\n'  */
#line 394 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist)); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* proc: tPUT cmdargs '\n'  */
#line 395 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist)); }
#line 2734 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* proc: tGO cmdargs '\n'  */
#line 396 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist)); }
#line 2740 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* proc: tGO frameargs '\n'  */
#line 397 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist)); }
#line 2746 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* proc: tPLAY cmdargs '\n'  */
#line 398 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 2752 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* proc: tPLAY frameargs '\n'  */
#line 399 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 2758 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* proc: tNEXT tREPEAT '\n'  */
#line 400 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 2764 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* proc: tEXIT tREPEAT '\n'  */
#line 401 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 2770 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* proc: tEXIT '\n'  */
#line 402 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 2776 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* proc: tASSERTERROR stmtoneliner  */
#line 403 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 2782 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* cmdargs: %empty  */
#line 406 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 2790 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* cmdargs: expr  */
#line 409 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2800 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102: /* cmdargs: expr ',' nonemptyexprlist  */
#line 414 "engines/director/lingo/lingo-gr.y"
                                                {
		// This matches `cmd args, ...)
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 2809 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* cmdargs: expr expr_nounarymath  */
#line 418 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2820 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist  */
#line 424 "engines/director/lingo/lingo-gr.y"
                                                           {
		// This matches `cmd arg arg, ...`
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 2830 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105: /* cmdargs: '(' ')'  */
#line 429 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 2838 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* cmdargs: '(' expr ',' nonemptyexprlist ')'  */
#line 432 "engines/director/lingo/lingo-gr.y"
                                                  {
		// This matches `cmd(args, ...)`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2847 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107: /* frameargs: tFRAME expr  */
#line 442 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* frameargs: tMOVIE expr  */
#line 447 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 453 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2879 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* frameargs: expr tOF tMOVIE expr  */
#line 459 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2890 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* frameargs: tFRAME expr expr_nounarymath  */
#line 465 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2901 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 473 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2907 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 474 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2913 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 475 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2919 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* asgn: tSET varorthe to expr '\n'  */
#line 476 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2925 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* definevars: tGLOBAL idlist '\n'  */
#line 481 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 2931 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* definevars: tPROPERTY idlist '\n'  */
#line 482 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 2937 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* definevars: tINSTANCE idlist '\n'  */
#line 483 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 2943 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* ifstmt: tIF expr tTHEN stmt  */
#line 486 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 2952 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* ifstmt: tIF expr tTHEN '\n' stmtlist tENDIF '\n'  */
#line 490 "engines/director/lingo/lingo-gr.y"
                                                   {
		(yyval.node) = new IfStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist)); }
#line 2959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 494 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 2970 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist tENDIF '\n'  */
#line 500 "engines/director/lingo/lingo-gr.y"
                                                                                {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-5].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), stmtlist1, (yyvsp[-2].nodelist)); }
#line 2979 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE stmt  */
#line 504 "engines/director/lingo/lingo-gr.y"
                                                                    {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 2988 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE '\n' stmtlist tENDIF '\n'  */
#line 508 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new IfElseStmtNode((yyvsp[-8].node), (yyvsp[-5].nodelist), (yyvsp[-2].nodelist)); }
#line 2995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 512 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3002 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 514 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3009 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 516 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3016 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* tell: tTELL expr tTO stmtoneliner  */
#line 520 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 524 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3032 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* when: tWHEN ID tTHEN expr  */
#line 528 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new WhenNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3038 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* stmtlist: %empty  */
#line 530 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3044 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* nonemptystmtlist: stmtlistline  */
#line 535 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* nonemptystmtlist: stmtlist stmtlistline  */
#line 541 "engines/director/lingo/lingo-gr.y"
                                                {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3065 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* stmtlistline: '\n'  */
#line 548 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3071 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* simpleexpr_noparens_nounarymath: tINT  */
#line 555 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3077 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* simpleexpr_noparens_nounarymath: tFLOAT  */
#line 556 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3083 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* simpleexpr_noparens_nounarymath: tSYMBOL  */
#line 557 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3089 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* simpleexpr_noparens_nounarymath: tSTRING  */
#line 558 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3095 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* simpleexpr_noparens_nounarymath: tNOT simpleexpr  */
#line 559 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3101 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* simpleexpr_noparens_nounarymath: ID '(' exprlist ')'  */
#line 560 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new FuncNode((yyvsp[-3].s), (yyvsp[-1].nodelist)); }
#line 3107 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* var: ID  */
#line 567 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3113 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* chunk: tFIELD simpleexpr_noparens  */
#line 577 "engines/director/lingo/lingo-gr.y"
                                        {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("field"), args); }
#line 3122 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* chunk: tCAST simpleexpr_noparens  */
#line 581 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("cast"), args); }
#line 3131 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* object: tSCRIPT simpleexpr_noparens  */
#line 587 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("script"), args); }
#line 3140 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* object: tWINDOW simpleexpr_noparens  */
#line 591 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("window"), args); }
#line 3149 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* list: '[' exprlist ']'  */
#line 597 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 3155 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* list: '[' ':' ']'  */
#line 598 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 3161 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* list: '[' proplist ']'  */
#line 599 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 3167 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* proplist: proppair  */
#line 602 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 3176 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* proplist: proplist ',' proppair  */
#line 606 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3184 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* proppair: tSYMBOL ':' expr  */
#line 611 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* proppair: ID ':' expr  */
#line 612 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3196 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* proppair: tSTRING ':' expr  */
#line 613 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3202 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* parens: '(' expr ')'  */
#line 616 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[-1].node); }
#line 3208 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* unarymath: '+' simpleexpr  */
#line 618 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 3214 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* unarymath: '-' simpleexpr  */
#line 619 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 3220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* expr: expr '+' expr  */
#line 638 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3226 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* expr: expr '-' expr  */
#line 639 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3232 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* expr: expr '*' expr  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3238 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* expr: expr '/' expr  */
#line 641 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3244 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* expr: expr tMOD expr  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3250 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* expr: expr '>' expr  */
#line 643 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3256 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* expr: expr '<' expr  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3262 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* expr: expr tEQ expr  */
#line 645 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3268 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* expr: expr tNEQ expr  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3274 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* expr: expr tGE expr  */
#line 647 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3280 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* expr: expr tLE expr  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3286 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* expr: expr tAND expr  */
#line 649 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3292 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189: /* expr: expr tOR expr  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3298 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* expr: expr '&' expr  */
#line 651 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 191: /* expr: expr tCONCAT expr  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3310 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* expr: expr tCONTAINS expr  */
#line 653 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3316 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* expr: expr tSTARTS expr  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 663 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3328 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 664 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 665 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3340 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 666 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3346 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 667 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3352 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 668 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3364 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 670 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 671 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3376 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 672 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3382 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 673 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3394 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 675 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3400 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 676 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3406 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 677 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3412 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 678 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 679 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3424 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 682 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3430 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 683 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3436 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* exprlist: %empty  */
#line 686 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 3442 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* nonemptyexprlist: expr  */
#line 690 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 3451 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 694 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3459 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3463 "engines/director/lingo/lingo-gr.cpp"

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

#line 699 "engines/director/lingo/lingo-gr.y"


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
