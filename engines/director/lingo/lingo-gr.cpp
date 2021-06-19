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
  YYSYMBOL_tTHEENTITY = 6,                 /* tTHEENTITY  */
  YYSYMBOL_tTHEENTITYWITHID = 7,           /* tTHEENTITYWITHID  */
  YYSYMBOL_tTHEMENUITEMENTITY = 8,         /* tTHEMENUITEMENTITY  */
  YYSYMBOL_tTHEMENUITEMSENTITY = 9,        /* tTHEMENUITEMSENTITY  */
  YYSYMBOL_tFLOAT = 10,                    /* tFLOAT  */
  YYSYMBOL_tTHEFUNC = 11,                  /* tTHEFUNC  */
  YYSYMBOL_tTHEFUNCINOF = 12,              /* tTHEFUNCINOF  */
  YYSYMBOL_tVARID = 13,                    /* tVARID  */
  YYSYMBOL_tSTRING = 14,                   /* tSTRING  */
  YYSYMBOL_tSYMBOL = 15,                   /* tSYMBOL  */
  YYSYMBOL_tENDCLAUSE = 16,                /* tENDCLAUSE  */
  YYSYMBOL_tPLAYACCEL = 17,                /* tPLAYACCEL  */
  YYSYMBOL_tTHEOBJECTPROP = 18,            /* tTHEOBJECTPROP  */
  YYSYMBOL_tCAST = 19,                     /* tCAST  */
  YYSYMBOL_tFIELD = 20,                    /* tFIELD  */
  YYSYMBOL_tSCRIPT = 21,                   /* tSCRIPT  */
  YYSYMBOL_tWINDOW = 22,                   /* tWINDOW  */
  YYSYMBOL_tDOWN = 23,                     /* tDOWN  */
  YYSYMBOL_tELSE = 24,                     /* tELSE  */
  YYSYMBOL_tELSIF = 25,                    /* tELSIF  */
  YYSYMBOL_tEXIT = 26,                     /* tEXIT  */
  YYSYMBOL_tFRAME = 27,                    /* tFRAME  */
  YYSYMBOL_tGLOBAL = 28,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 29,                       /* tGO  */
  YYSYMBOL_tIF = 30,                       /* tIF  */
  YYSYMBOL_tIN = 31,                       /* tIN  */
  YYSYMBOL_tINTO = 32,                     /* tINTO  */
  YYSYMBOL_tMACRO = 33,                    /* tMACRO  */
  YYSYMBOL_tMOVIE = 34,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 35,                     /* tNEXT  */
  YYSYMBOL_tOF = 36,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 37,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 38,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 39,                   /* tREPEAT  */
  YYSYMBOL_tSET = 40,                      /* tSET  */
  YYSYMBOL_tTHEN = 41,                     /* tTHEN  */
  YYSYMBOL_tTO = 42,                       /* tTO  */
  YYSYMBOL_tWHEN = 43,                     /* tWHEN  */
  YYSYMBOL_tWITH = 44,                     /* tWITH  */
  YYSYMBOL_tWHILE = 45,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 46,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 47,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 48,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 49,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 50,                       /* tGE  */
  YYSYMBOL_tLE = 51,                       /* tLE  */
  YYSYMBOL_tEQ = 52,                       /* tEQ  */
  YYSYMBOL_tNEQ = 53,                      /* tNEQ  */
  YYSYMBOL_tAND = 54,                      /* tAND  */
  YYSYMBOL_tOR = 55,                       /* tOR  */
  YYSYMBOL_tNOT = 56,                      /* tNOT  */
  YYSYMBOL_tMOD = 57,                      /* tMOD  */
  YYSYMBOL_tAFTER = 58,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 59,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 60,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 61,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 62,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 63,                     /* tCHAR  */
  YYSYMBOL_tITEM = 64,                     /* tITEM  */
  YYSYMBOL_tLINE = 65,                     /* tLINE  */
  YYSYMBOL_tWORD = 66,                     /* tWORD  */
  YYSYMBOL_tSPRITE = 67,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 68,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 69,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 70,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 71,                 /* tPROPERTY  */
  YYSYMBOL_tON = 72,                       /* tON  */
  YYSYMBOL_tMETHOD = 73,                   /* tMETHOD  */
  YYSYMBOL_tENDIF = 74,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 75,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 76,                  /* tENDTELL  */
  YYSYMBOL_tASSERTERROR = 77,              /* tASSERTERROR  */
  YYSYMBOL_78_ = 78,                       /* '<'  */
  YYSYMBOL_79_ = 79,                       /* '>'  */
  YYSYMBOL_80_ = 80,                       /* '&'  */
  YYSYMBOL_81_ = 81,                       /* '+'  */
  YYSYMBOL_82_ = 82,                       /* '-'  */
  YYSYMBOL_83_ = 83,                       /* '*'  */
  YYSYMBOL_84_ = 84,                       /* '/'  */
  YYSYMBOL_85_ = 85,                       /* '%'  */
  YYSYMBOL_86_n_ = 86,                     /* '\n'  */
  YYSYMBOL_87_ = 87,                       /* ','  */
  YYSYMBOL_88_ = 88,                       /* '('  */
  YYSYMBOL_89_ = 89,                       /* ')'  */
  YYSYMBOL_90_ = 90,                       /* '['  */
  YYSYMBOL_91_ = 91,                       /* ']'  */
  YYSYMBOL_92_ = 92,                       /* ':'  */
  YYSYMBOL_YYACCEPT = 93,                  /* $accept  */
  YYSYMBOL_script = 94,                    /* script  */
  YYSYMBOL_scriptpartlist = 95,            /* scriptpartlist  */
  YYSYMBOL_scriptpart = 96,                /* scriptpart  */
  YYSYMBOL_macro = 97,                     /* macro  */
  YYSYMBOL_factory = 98,                   /* factory  */
  YYSYMBOL_method = 99,                    /* method  */
  YYSYMBOL_methodlist = 100,               /* methodlist  */
  YYSYMBOL_nonemptymethodlist = 101,       /* nonemptymethodlist  */
  YYSYMBOL_methodlistline = 102,           /* methodlistline  */
  YYSYMBOL_handler = 103,                  /* handler  */
  YYSYMBOL_endargdef = 104,                /* endargdef  */
  YYSYMBOL_ID = 105,                       /* ID  */
  YYSYMBOL_idlist = 106,                   /* idlist  */
  YYSYMBOL_nonemptyidlist = 107,           /* nonemptyidlist  */
  YYSYMBOL_stmt = 108,                     /* stmt  */
  YYSYMBOL_stmtoneliner = 109,             /* stmtoneliner  */
  YYSYMBOL_proc = 110,                     /* proc  */
  YYSYMBOL_cmdargs = 111,                  /* cmdargs  */
  YYSYMBOL_frameargs = 112,                /* frameargs  */
  YYSYMBOL_asgn = 113,                     /* asgn  */
  YYSYMBOL_to = 114,                       /* to  */
  YYSYMBOL_definevars = 115,               /* definevars  */
  YYSYMBOL_ifstmt = 116,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 117,               /* ifelsestmt  */
  YYSYMBOL_loop = 118,                     /* loop  */
  YYSYMBOL_tell = 119,                     /* tell  */
  YYSYMBOL_when = 120,                     /* when  */
  YYSYMBOL_stmtlist = 121,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 122,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 123,             /* stmtlistline  */
  YYSYMBOL_simpleexpr_noparens_nounarymath = 124, /* simpleexpr_noparens_nounarymath  */
  YYSYMBOL_var = 125,                      /* var  */
  YYSYMBOL_varorchunk = 126,               /* varorchunk  */
  YYSYMBOL_varorthe = 127,                 /* varorthe  */
  YYSYMBOL_list = 128,                     /* list  */
  YYSYMBOL_proplist = 129,                 /* proplist  */
  YYSYMBOL_proppair = 130,                 /* proppair  */
  YYSYMBOL_parens = 131,                   /* parens  */
  YYSYMBOL_unarymath = 132,                /* unarymath  */
  YYSYMBOL_simpleexpr = 133,               /* simpleexpr  */
  YYSYMBOL_simpleexpr_nounarymath = 134,   /* simpleexpr_nounarymath  */
  YYSYMBOL_expr = 135,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 136,         /* expr_nounarymath  */
  YYSYMBOL_exprlist = 137,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 138          /* nonemptyexprlist  */
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
#define YYFINAL  117
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2625

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  93
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  187
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  353

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   332


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
      86,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    85,    80,     2,
      88,    89,    83,    81,    87,    82,     2,    84,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    92,     2,
      78,     2,    79,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    90,     2,    91,     2,     2,     2,     2,     2,     2,
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
      75,    76,    77
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   196,   196,   198,   204,   211,   212,   213,   214,   215,
     244,   248,   250,   252,   253,   256,   262,   269,   270,   275,
     279,   283,   284,   285,   290,   291,   292,   293,   294,   295,
     296,   301,   302,   305,   307,   308,   309,   310,   313,   314,
     315,   316,   317,   319,   320,   322,   325,   326,   327,   328,
     329,   331,   332,   333,   335,   336,   337,   338,   339,   342,
     343,   346,   350,   360,   361,   362,   363,   364,   365,   368,
     369,   370,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   385,   388,   393,   397,   403,   408,   411,   421,
     426,   432,   438,   444,   452,   453,   454,   455,   458,   458,
     460,   461,   462,   465,   469,   473,   479,   483,   487,   491,
     493,   495,   499,   503,   507,   509,   510,   514,   520,   527,
     528,   534,   535,   536,   537,   538,   539,   540,   541,   544,
     546,   550,   554,   555,   556,   559,   563,   568,   569,   570,
     573,   575,   576,   579,   580,   581,   588,   589,   593,   594,
     595,   596,   597,   598,   599,   600,   601,   602,   603,   604,
     605,   606,   607,   608,   609,   610,   617,   618,   619,   620,
     621,   622,   623,   624,   625,   626,   627,   628,   629,   630,
     631,   632,   633,   634,   637,   638,   641,   645
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
  "tTHEENTITY", "tTHEENTITYWITHID", "tTHEMENUITEMENTITY",
  "tTHEMENUITEMSENTITY", "tFLOAT", "tTHEFUNC", "tTHEFUNCINOF", "tVARID",
  "tSTRING", "tSYMBOL", "tENDCLAUSE", "tPLAYACCEL", "tTHEOBJECTPROP",
  "tCAST", "tFIELD", "tSCRIPT", "tWINDOW", "tDOWN", "tELSE", "tELSIF",
  "tEXIT", "tFRAME", "tGLOBAL", "tGO", "tIF", "tIN", "tINTO", "tMACRO",
  "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET",
  "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tFACTORY", "tOPEN", "tPLAY",
  "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT", "tMOD",
  "tAFTER", "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR", "tITEM",
  "tLINE", "tWORD", "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL",
  "tPROPERTY", "tON", "tMETHOD", "tENDIF", "tENDREPEAT", "tENDTELL",
  "tASSERTERROR", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'", "'/'", "'%'",
  "'\\n'", "','", "'('", "')'", "'['", "']'", "':'", "$accept", "script",
  "scriptpartlist", "scriptpart", "macro", "factory", "method",
  "methodlist", "nonemptymethodlist", "methodlistline", "handler",
  "endargdef", "ID", "idlist", "nonemptyidlist", "stmt", "stmtoneliner",
  "proc", "cmdargs", "frameargs", "asgn", "to", "definevars", "ifstmt",
  "ifelsestmt", "loop", "tell", "when", "stmtlist", "nonemptystmtlist",
  "stmtlistline", "simpleexpr_noparens_nounarymath", "var", "varorchunk",
  "varorthe", "list", "proplist", "proppair", "parens", "unarymath",
  "simpleexpr", "simpleexpr_nounarymath", "expr", "expr_nounarymath",
  "exprlist", "nonemptyexprlist", YY_NULLPTR
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
     325,   326,   327,   328,   329,   330,   331,   332,    60,    62,
      38,    43,    45,    42,    47,    37,    10,    44,    40,    41,
      91,    93,    58
};
#endif

#define YYPACT_NINF (-237)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1398,  -237,  -237,  -237,  -237,  -237,  -237,   -34,  -237,  2319,
     931,  1010,  -237,  -237,  2319,  -237,   -12,  -237,  -237,  1089,
     -26,  2319,  -237,  2319,  -237,  -237,  2319,  -237,   931,  2319,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  1010,  2319,  2319,  2201,  -237,    12,
    1398,  -237,  -237,  -237,  -237,  1089,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,   -56,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,   -38,   -47,  -237,  -237,  -237,  -237,
    1010,  1010,  1010,  1010,  1010,   850,   445,   -35,   -10,    -8,
    -237,  -237,  -237,  -237,  -237,  -237,   526,  1010,    81,  2319,
      -2,    -1,   607,  2319,  1010,  -237,  -237,   -27,    33,    11,
      19,    20,    23,   339,    24,  2319,  -237,  -237,  -237,    26,
     688,  -237,  -237,  2319,   769,  2506,  -237,  -237,  -237,  -237,
    2340,     6,    21,    25,   -64,   -32,  -237,  2506,    27,     3,
    1010,  -237,  -237,    80,  1010,  1010,  1010,  1010,  1168,  1168,
    1326,  1010,  1010,  1247,  1010,  1010,  1010,  1010,  1010,  1010,
    1010,  1010,  -237,  -237,  -237,  2393,  2353,  1957,  1010,  1010,
    1010,  1010,    43,  -237,  -237,  2319,  2319,  2319,    67,  2406,
    -237,  -237,  1010,  1010,   -66,  -237,  -237,  -237,  2201,  2018,
    -237,    51,  -237,  -237,    96,  2541,  1010,  -237,  1010,  1010,
    -237,  1010,  2260,  -237,  -237,  1010,    50,  1010,    44,    44,
      44,    44,   -18,   -18,  -237,    64,    44,    44,    44,    44,
      64,   -46,   -46,  -237,  -237,     3,  1010,  1010,  1010,  1010,
    1010,  1010,  1010,  1010,  1010,  1010,  1010,  1010,  1010,  1010,
    1010,  1010,  1010,  1010,  2018,   116,  2018,  -237,    63,    65,
      66,  1010,  2018,  2443,  2506,  2319,  -237,  -237,    68,  -237,
    -237,  -237,  -237,  -237,  1521,  -237,  -237,  2018,  1010,   -31,
    2506,  2506,  2506,     6,    21,    58,  -237,  2506,  -237,  2506,
      44,    44,    44,    44,   -18,   -18,  -237,    64,    44,    44,
      44,    44,    64,   -46,   -46,  -237,  -237,     3,  1459,  2079,
    2018,  -237,  -237,  -237,   130,  1582,  -237,  2319,   -66,    69,
    -237,  1650,  2506,  -237,  2140,    82,  2018,  -237,   124,  1010,
      83,    84,  -237,  -237,  2319,  2018,  -237,  -237,  1711,  1010,
    2456,  -237,  2018,    -5,  -237,  1773,    87,  2493,  2018,  2018,
    -237,  2319,    88,  -237,  2018,  1835,  -237,  -237,  1896,    92,
      93,  -237,  -237
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    24,    28,    31,    47,    55,    30,     0,    32,    59,
      82,     0,    33,    35,     0,    39,    40,    42,    45,    82,
      46,    48,    51,     0,    56,    54,     0,    43,    82,    59,
      26,    44,    41,    38,    25,    27,    49,    29,    36,    37,
      58,    53,    34,    57,    50,    59,     0,    52,     5,     0,
       2,     3,     6,     7,     8,    82,     9,    63,    69,    70,
      71,    64,    65,    66,    67,    68,     0,    80,    40,    46,
      48,    50,    52,    61,     0,    60,   121,   122,   124,   123,
      32,    39,    41,     0,     0,     0,   184,   129,     0,     0,
     143,   127,   128,   144,   145,   148,    83,     0,     0,    59,
       0,     0,    83,     0,     0,   129,   131,     0,     0,     0,
       0,     0,     0,     0,     0,    59,    81,     1,     4,     0,
      83,    79,   100,     0,    89,    90,   125,   141,   142,    87,
       0,   124,   123,     0,   129,     0,   135,   186,     0,   185,
     184,    74,    75,    42,     0,     0,     0,     0,    26,    44,
      38,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,     0,   146,   147,   166,    85,     0,     0,     0,     0,
       0,     0,     0,    78,    73,    35,    25,    27,     0,     0,
      98,    99,     0,     0,    13,    76,    77,   102,     0,   115,
     101,     0,    72,    62,    42,    93,     0,   140,     0,     0,
     133,     0,     0,   134,   132,     0,     0,     0,   158,   159,
     156,   157,   160,   161,   153,   163,   164,   165,   155,   154,
     162,   149,   150,   151,   152,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   115,   103,   115,   130,     0,     0,
       0,     0,   115,     0,   114,     0,    17,    18,    11,    14,
      15,   112,   119,   120,     0,   116,   117,   115,     0,     0,
     139,   137,   138,     0,     0,     0,   136,   187,   126,    92,
     176,   177,   174,   175,   178,   179,   171,   181,   182,   183,
     173,   172,   180,   167,   168,   169,   170,    86,     0,     0,
      10,    94,    95,    96,     0,     0,    97,    59,     0,     0,
     118,    20,    91,    88,     0,     0,   115,   105,     0,     0,
       0,     0,    16,   113,    21,   115,   107,   104,     0,     0,
       0,   109,   115,     0,    22,     0,     0,     0,   115,    12,
      19,     0,     0,   106,   115,     0,    23,   108,     0,     0,
       0,   110,   111
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -237,  -237,  -237,   136,  -237,  -237,  -237,  -237,  -237,  -115,
    -237,  -237,     0,   -28,  -237,     4,   -44,  -237,    -6,   166,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -236,  -237,
    -228,   -45,   -19,  -108,  -237,  -237,  -237,    -7,   -29,  -237,
      10,  -237,   205,    72,    57,  -155
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    49,    50,    51,    52,    53,   257,   258,   259,   260,
      54,   333,    87,    74,    75,   263,    57,    58,    88,    89,
      59,   182,    60,    61,    62,    63,    64,    65,   264,   265,
     266,    90,    91,   248,   107,    92,   135,   136,    93,    94,
      95,   164,   137,   165,   138,   139
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      55,   112,   106,   116,    56,    66,   225,   255,   298,    73,
     300,   170,   117,   101,    99,   180,   305,   114,   103,   104,
     256,   105,   110,   108,   140,   181,   109,   100,   201,    73,
     121,   311,   144,   145,   146,   147,   310,   159,   160,   170,
     123,   269,   151,   152,   171,    73,   115,    55,   122,   119,
      55,   162,    67,   140,    56,   202,   205,   162,   313,   203,
     154,   155,   156,   157,   158,   159,   160,   163,   249,   250,
     310,   172,   310,   163,   183,   162,   141,   310,   142,   162,
     328,   340,   341,   310,   173,   174,   134,   191,   297,   335,
     205,   163,   126,   127,   128,   163,   339,   184,   198,    73,
     310,   170,   345,   178,   151,   185,   186,   310,   348,   187,
     190,   310,   192,   199,   207,    73,   200,   310,   204,   251,
     310,   170,   167,   193,   156,   157,   158,   159,   160,   246,
     268,   144,   145,   146,   147,   168,   169,   267,   170,   278,
     299,   151,   152,   171,   261,   157,   158,   159,   160,   301,
     201,   302,   303,   318,   308,   323,   247,   247,   247,   154,
     155,   156,   157,   158,   159,   160,   329,    55,   327,   331,
     332,   245,   319,   343,   347,   105,   105,   105,   351,   352,
     144,   145,   146,   147,   168,   169,   118,   170,    55,    55,
     151,   152,   171,   322,   111,   276,   195,   206,     0,     0,
       0,     0,   275,     0,     0,     0,     0,     0,   154,   155,
     156,   157,   158,   159,   160,    96,    98,     0,     0,     0,
       0,     0,     0,     0,   102,     0,     0,     0,     0,     0,
       0,     0,     0,    96,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    55,     0,    55,     0,     0,   113,
       0,     0,    55,     0,     0,   307,     0,     0,     0,     0,
     120,     0,     0,     0,    55,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   321,
       0,     0,     0,     0,     0,   124,   125,     0,     0,     0,
     130,     0,     0,     0,     0,     0,     0,     0,    55,    55,
      55,     0,   166,   317,     0,    55,     0,    73,     0,   179,
       0,    55,     0,     0,    55,     0,    55,     0,   326,     0,
       0,     0,     0,     0,   334,    55,     0,     0,    55,     0,
       0,     0,    55,     0,     0,    55,     0,     0,    55,    55,
       0,   346,     0,     0,    55,    55,     0,     0,    55,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,     0,     0,     0,     0,
       0,     0,     0,   212,   213,   214,   217,     0,     0,     0,
       0,   188,     0,     0,     0,     0,     0,   253,   254,   144,
     145,   146,   147,   168,   169,     0,   170,     0,     0,   151,
     152,   171,     0,   270,   271,     0,   272,     0,     0,     0,
     277,     0,   279,     0,     0,     0,     0,   154,   155,   156,
     157,   158,   159,   160,     0,   189,     0,     0,     0,     0,
       0,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,     0,     0,
      76,     0,     0,     0,     0,    77,   304,     0,     1,   131,
     132,     0,     0,     0,     2,     3,     4,     5,     6,     0,
       0,     0,     8,   312,     0,     0,    12,    13,     0,    15,
      68,    17,    18,     0,    69,    70,     0,    22,     0,    24,
      25,     0,    27,     0,     0,     0,     0,     0,     0,    30,
      31,    82,    33,    34,    35,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    71,     0,     0,     0,     0,
       0,     0,    72,     0,   330,     0,    83,    84,     0,     0,
       0,    76,     0,    97,   337,    86,    77,   133,     0,     1,
      78,    79,     0,     0,     0,     2,     3,     4,     5,     6,
       0,     0,     0,     8,     0,     0,     0,    12,    13,     0,
      15,    68,   143,    18,     0,    69,    70,     0,    22,     0,
      24,    25,     0,    27,     0,     0,   144,   145,   146,   147,
     148,   149,    82,   150,    34,    35,   151,   152,   153,    37,
      38,    39,    40,    41,    42,    43,    71,     0,     0,     0,
       0,     0,     0,    72,   154,   155,   156,   157,   158,   159,
     160,     0,    76,   161,    97,     0,    86,    77,     0,     0,
       1,    78,    79,     0,     0,     0,     2,     3,     4,     5,
       6,     0,     0,     0,     8,     0,     0,     0,    12,   175,
       0,    15,    68,    17,    18,     0,    69,    70,     0,    22,
       0,    24,    25,     0,    27,     0,     0,   144,   145,   146,
     147,   148,   149,    82,   150,   176,   177,   151,   152,   153,
      37,    38,    39,    40,    41,    42,    43,    71,     0,     0,
       0,     0,     0,     0,    72,   154,   155,   156,   157,   158,
     159,   160,     0,    76,   161,    97,     0,    86,    77,     0,
       0,     1,    78,    79,     0,     0,     0,     2,     3,     4,
       5,     6,     0,     0,     0,     8,     0,     0,     0,    12,
      13,     0,    15,    68,    17,    18,     0,    69,    70,     0,
      22,     0,    24,    25,     0,    27,     0,     0,   144,   145,
     146,   147,   148,   149,    82,   150,    34,    35,   151,   152,
     153,    37,    38,    39,    40,    41,    42,    43,    71,     0,
       0,     0,     0,     0,     0,    72,   154,   155,   156,   157,
     158,   159,   160,     0,    76,   161,    97,     0,    86,    77,
       0,     0,     1,    78,    79,     0,     0,     0,     2,     3,
       4,     5,     6,     0,     0,     0,     8,     0,     0,     0,
      12,    13,     0,    15,    68,   194,    18,     0,    69,    70,
       0,    22,     0,    24,    25,     0,    27,     0,     0,   144,
     145,   146,   147,   148,   149,    82,   150,    34,    35,   151,
     152,   153,    37,    38,    39,    40,    41,    42,    43,    71,
       0,     0,     0,     0,     0,     0,    72,   154,   155,   156,
     157,   158,   159,   160,     0,    76,     0,    97,     0,    86,
      77,     0,     0,     1,    78,    79,     0,     0,     0,     2,
       3,     4,     5,     6,     0,     0,     0,     8,     0,     0,
       0,    12,    13,     0,    15,    68,    17,    18,     0,    69,
      70,     0,    22,     0,    24,    25,     0,    27,     0,     0,
       0,     0,     0,     0,    30,    31,    82,    33,    34,    35,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      71,     0,     0,     0,     0,     0,     0,    72,     0,     0,
       0,    83,    84,     0,     0,     0,    76,     0,    97,   129,
      86,    77,     0,     0,     1,    78,    79,     0,     0,     0,
       2,     3,     4,     5,     6,     0,     0,     0,    80,     0,
       0,     0,    12,    13,     0,    81,    68,    17,    18,     0,
      69,    70,     0,    22,     0,    24,    25,     0,    27,     0,
       0,     0,     0,     0,     0,    30,    31,    82,    33,    34,
      35,     0,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    71,     0,     0,     0,     0,     0,     0,    72,     0,
       0,     0,    83,    84,     0,    76,     0,     0,     0,    85,
      77,    86,     0,     1,    78,    79,     0,     0,     0,     2,
       3,     4,     5,     6,     0,     0,     0,     8,     0,     0,
       0,    12,    13,     0,    15,    68,    17,    18,     0,    69,
      70,     0,    22,     0,    24,    25,     0,    27,     0,     0,
       0,     0,     0,     0,    30,    31,    82,    33,    34,    35,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      71,     0,     0,     0,     0,     0,     0,    72,     0,     0,
       0,    83,    84,     0,    76,     0,     0,     0,    97,    77,
      86,     0,     1,    78,    79,     0,     0,     0,     2,     3,
       4,     5,     6,     0,     0,     0,     8,     0,     0,     0,
      12,    13,     0,    15,    68,    17,    18,     0,    69,    70,
       0,    22,     0,    24,    25,     0,    27,     0,     0,     0,
       0,     0,     0,    30,    31,    82,    33,    34,    35,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    71,
       0,     0,     0,     0,     0,     0,    72,     0,     0,     0,
      83,    84,     0,    76,     0,     0,     0,    85,    77,    86,
       0,     1,    78,    79,     0,     0,     0,     2,     3,     4,
       5,     6,     0,     0,     0,     8,     0,     0,     0,    12,
      13,     0,    15,    68,    17,    18,     0,    69,    70,     0,
      22,     0,    24,    25,     0,    27,     0,     0,     0,     0,
       0,     0,     0,     0,    82,    33,    34,    35,     0,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    71,     0,
       0,     0,     0,     0,     0,    72,     0,     0,     0,    83,
      84,     0,    76,     0,     0,     0,    97,    77,    86,     0,
       1,    78,    79,     0,     0,     0,     2,     3,     4,     5,
       6,     0,     0,     0,     8,     0,     0,     0,    12,    13,
       0,    15,    68,    17,    18,     0,    69,    70,     0,    22,
       0,    24,    25,     0,    27,     0,     0,     0,     0,     0,
       0,     0,     0,    82,    33,    34,    35,     0,     0,     0,
      37,    38,    39,    40,    41,    42,    43,    71,     0,     0,
       0,     0,     0,     0,    72,     0,     0,     0,    83,    84,
       0,    76,     0,     0,     0,    97,    77,    86,     0,     1,
      78,    79,     0,     0,     0,     2,     3,     4,     5,     6,
       0,     0,     0,     8,     0,     0,     0,    12,    13,     0,
      15,    68,    17,    18,     0,    69,    70,     0,    22,     0,
      24,    25,     0,    27,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,    34,    35,     0,     0,     0,    37,
      38,    39,    40,    41,    42,    43,    71,     0,     0,     0,
       0,     0,     0,    72,     0,     0,     0,     0,     0,     0,
       0,     1,     0,     0,    97,     0,    86,     2,     3,     4,
       5,     6,     0,     0,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,     0,     1,     0,     0,    47,     0,     0,     2,     3,
       4,     5,     6,   314,    48,     7,     8,     9,    10,    11,
      12,    13,     0,    15,    16,    17,    18,    19,    20,    21,
       0,    22,    23,    24,    25,     0,    27,    28,    29,     0,
       0,     0,     0,    30,    31,    32,    33,    34,    35,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,     0,     0,   315,     1,     0,    47,     0,     0,     0,
       2,     3,     4,     5,     6,   262,     0,     7,     8,     9,
      10,    11,    12,    13,     0,    15,    16,    17,    18,    19,
      20,    21,     0,    22,    23,    24,    25,     0,    27,    28,
      29,     0,     0,     0,     0,    30,    31,    32,    33,    34,
      35,     0,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,     0,     0,     1,     0,   309,    47,     0,
       0,     2,     3,     4,     5,     6,     0,   262,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,     0,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,     0,     0,     0,   320,     0,    47,
       0,     0,     0,     1,     0,     0,   324,     0,   262,     2,
       3,     4,     5,     6,     0,     0,     7,     8,     9,    10,
      11,    12,    13,     0,    15,    16,    17,    18,    19,    20,
      21,     0,    22,    23,    24,    25,     0,    27,    28,    29,
       0,     0,     0,     0,    30,    31,    32,    33,    34,    35,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,     0,     0,     1,     0,     0,    47,     0,     0,
       2,     3,     4,     5,     6,     0,   262,     7,     8,     9,
      10,    11,    12,    13,     0,    15,    16,    17,    18,    19,
      20,    21,     0,    22,    23,    24,    25,     0,    27,    28,
      29,     0,     0,     0,     0,    30,    31,    32,    33,    34,
      35,     0,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,     0,     0,   336,     1,     0,    47,     0,
       0,     0,     2,     3,     4,     5,     6,   262,     0,     7,
       8,     9,    10,    11,    12,    13,     0,    15,    16,    17,
      18,    19,    20,    21,     0,    22,    23,    24,    25,     0,
      27,    28,    29,     0,     0,     0,     0,    30,    31,    32,
      33,    34,    35,     0,     0,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,     0,     0,   342,     1,     0,
      47,     0,     0,     0,     2,     3,     4,     5,     6,   262,
       0,     7,     8,     9,    10,    11,    12,    13,     0,    15,
      16,    17,    18,    19,    20,    21,     0,    22,    23,    24,
      25,     0,    27,    28,    29,     0,     0,     0,     0,    30,
      31,    32,    33,    34,    35,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,     1,
     349,     0,    47,     0,     0,     2,     3,     4,     5,     6,
       0,   262,     7,     8,     9,    10,    11,    12,    13,     0,
      15,    16,    17,    18,    19,    20,    21,     0,    22,    23,
      24,    25,     0,    27,    28,    29,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,     0,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,     0,     0,
       1,   350,     0,    47,     0,     0,     2,     3,     4,     5,
       6,     0,   262,     7,     8,     9,    10,    11,    12,    13,
       0,    15,    16,    17,    18,    19,    20,    21,     0,    22,
      23,    24,    25,     0,    27,    28,    29,     0,     0,     0,
       0,    30,    31,    32,    33,    34,    35,     0,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,     0,
       0,     1,     0,     0,    47,     0,     0,     2,     3,     4,
       5,     6,     0,   244,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
       0,     0,     1,     0,     0,    47,     0,     0,     2,     3,
       4,     5,     6,     0,   262,     7,     8,     9,    10,    11,
      12,    13,     0,    15,    16,    17,    18,    19,    20,    21,
       0,    22,    23,    24,    25,     0,    27,    28,    29,     0,
       0,     0,     0,    30,    31,    32,    33,    34,    35,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,     0,     0,     1,     0,     0,    47,     0,     0,     2,
       3,     4,     5,     6,     0,   316,     7,     8,     9,    10,
      11,    12,    13,     0,    15,    16,    17,    18,    19,    20,
      21,     0,    22,    23,    24,    25,     0,    27,    28,    29,
       0,     0,     0,     0,    30,    31,    32,    33,    34,    35,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,     0,     0,     1,     0,     0,    47,     0,     0,
       2,     3,     4,     5,     6,     0,   325,     7,     8,     9,
      10,     0,    12,    13,     0,    15,    16,    17,    18,    19,
      69,    21,     0,    22,     0,    24,    25,     0,    27,    28,
      29,     0,     0,     0,     0,    30,    31,    32,    33,    34,
      35,     0,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    71,    45,     1,   273,   274,     0,     0,    47,     2,
       3,     4,     5,     6,     0,     0,     0,     8,     0,     0,
       0,    12,    13,     0,    15,    68,    17,    18,     0,    69,
      70,     0,    22,     0,    24,    25,     0,    27,     0,     0,
       0,     0,     0,     0,    30,    31,    32,    33,    34,    35,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      71,     0,     1,     0,     0,     0,     0,    72,     2,     3,
       4,     5,     6,     0,     0,     0,     8,     0,     0,     0,
      12,    13,     0,    15,    68,    17,    18,     0,    69,    70,
       0,    22,     0,    24,    25,     0,    27,     0,     0,     0,
       0,     0,     0,    30,    31,    32,    33,    34,    35,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    71,
     144,   145,   146,   147,   168,   169,    72,   170,     0,     0,
     151,   152,   171,   144,   145,   146,   147,   168,   169,     0,
     170,     0,     0,   151,   152,   171,     0,     0,   154,   155,
     156,   157,   158,   159,   160,     0,     0,   196,     0,   197,
       0,   154,   155,   156,   157,   158,   159,   160,     0,     0,
       0,     0,   197,   226,   227,   228,   229,   230,   231,     0,
     232,     0,     0,   233,   234,   235,   144,   145,   146,   147,
     168,   169,     0,   170,     0,     0,   151,   152,   171,     0,
       0,   236,   237,   238,   239,   240,   241,   242,     0,     0,
     243,     0,     0,     0,   154,   155,   156,   157,   158,   159,
     160,     0,   252,   144,   145,   146,   147,   168,   169,     0,
     170,     0,     0,   151,   152,   171,   144,   145,   146,   147,
     168,   169,     0,   170,     0,     0,   151,   152,   171,     0,
       0,   154,   155,   156,   157,   158,   159,   160,     0,   306,
       0,     0,     0,     0,   154,   155,   156,   157,   158,   159,
     160,     0,   338,   144,   145,   146,   147,   168,   169,     0,
     170,     0,     0,   151,   152,   171,   144,   145,   146,   147,
     168,   169,     0,   170,     0,     0,   151,   152,   171,     0,
       0,   154,   155,   156,   157,   158,   159,   160,     0,   344,
       0,     0,     0,     0,   154,   155,   156,   157,   158,   159,
     160,   226,   227,   228,   229,   230,   231,     0,   232,     0,
       0,   233,   234,   235,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   236,
     237,   238,   239,   240,   241,   242
};

static const yytype_int16 yycheck[] =
{
       0,    29,    21,    47,     0,    39,   161,    73,   244,     9,
     246,    57,     0,    19,    14,    42,   252,    45,    44,    45,
      86,    21,    28,    23,    88,    52,    26,    39,    92,    29,
      86,   267,    50,    51,    52,    53,   264,    83,    84,    57,
      87,   196,    60,    61,    62,    45,    46,    47,    86,    55,
      50,    96,    86,    88,    50,    87,    87,   102,    89,    91,
      78,    79,    80,    81,    82,    83,    84,    96,   176,   177,
     298,    99,   300,   102,    41,   120,    86,   305,    86,   124,
     316,    86,    87,   311,    86,    86,    86,   115,   243,   325,
      87,   120,    82,    83,    84,   124,   332,    86,    92,    99,
     328,    57,   338,   103,    60,    86,    86,   335,   344,    86,
      86,   339,    86,    92,    34,   115,    91,   345,    91,    52,
     348,    57,    41,   123,    80,    81,    82,    83,    84,    86,
      34,    50,    51,    52,    53,    54,    55,    86,    57,    89,
      24,    60,    61,    62,   188,    81,    82,    83,    84,    86,
      92,    86,    86,    23,    86,    86,   175,   176,   177,    78,
      79,    80,    81,    82,    83,    84,    42,   167,    86,    86,
      86,   167,    42,    86,    86,   175,   176,   177,    86,    86,
      50,    51,    52,    53,    54,    55,    50,    57,   188,   189,
      60,    61,    62,   308,    28,   202,   124,   140,    -1,    -1,
      -1,    -1,   202,    -1,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   244,    -1,   246,    -1,    -1,    44,
      -1,    -1,   252,    -1,    -1,   255,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,   264,    -1,    -1,   267,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   307,
      -1,    -1,    -1,    -1,    -1,    80,    81,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   298,   299,
     300,    -1,    97,   299,    -1,   305,    -1,   307,    -1,   104,
      -1,   311,    -1,    -1,   314,    -1,   316,    -1,   314,    -1,
      -1,    -1,    -1,    -1,   324,   325,    -1,    -1,   328,    -1,
      -1,    -1,   332,    -1,    -1,   335,    -1,    -1,   338,   339,
      -1,   341,    -1,    -1,   344,   345,    -1,    -1,   348,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   168,   169,   170,   171,    -1,    -1,    -1,
      -1,    42,    -1,    -1,    -1,    -1,    -1,   182,   183,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    60,
      61,    62,    -1,   198,   199,    -1,   201,    -1,    -1,    -1,
     205,    -1,   207,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    -1,    -1,    -1,
      -1,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,    -1,    -1,
       5,    -1,    -1,    -1,    -1,    10,   251,    -1,    13,    14,
      15,    -1,    -1,    -1,    19,    20,    21,    22,    23,    -1,
      -1,    -1,    27,   268,    -1,    -1,    31,    32,    -1,    34,
      35,    36,    37,    -1,    39,    40,    -1,    42,    -1,    44,
      45,    -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,    54,
      55,    56,    57,    58,    59,    -1,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,   319,    -1,    81,    82,    -1,    -1,
      -1,     5,    -1,    88,   329,    90,    10,    92,    -1,    13,
      14,    15,    -1,    -1,    -1,    19,    20,    21,    22,    23,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    -1,    42,    -1,
      44,    45,    -1,    47,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    78,    79,    80,    81,    82,    83,
      84,    -1,     5,    87,    88,    -1,    90,    10,    -1,    -1,
      13,    14,    15,    -1,    -1,    -1,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    -1,    -1,    -1,    31,    32,
      -1,    34,    35,    36,    37,    -1,    39,    40,    -1,    42,
      -1,    44,    45,    -1,    47,    -1,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,    82,
      83,    84,    -1,     5,    87,    88,    -1,    90,    10,    -1,
      -1,    13,    14,    15,    -1,    -1,    -1,    19,    20,    21,
      22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,    31,
      32,    -1,    34,    35,    36,    37,    -1,    39,    40,    -1,
      42,    -1,    44,    45,    -1,    47,    -1,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    83,    84,    -1,     5,    87,    88,    -1,    90,    10,
      -1,    -1,    13,    14,    15,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      31,    32,    -1,    34,    35,    36,    37,    -1,    39,    40,
      -1,    42,    -1,    44,    45,    -1,    47,    -1,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    78,    79,    80,
      81,    82,    83,    84,    -1,     5,    -1,    88,    -1,    90,
      10,    -1,    -1,    13,    14,    15,    -1,    -1,    -1,    19,
      20,    21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    -1,    42,    -1,    44,    45,    -1,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    81,    82,    -1,    -1,    -1,     5,    -1,    88,    89,
      90,    10,    -1,    -1,    13,    14,    15,    -1,    -1,    -1,
      19,    20,    21,    22,    23,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    31,    32,    -1,    34,    35,    36,    37,    -1,
      39,    40,    -1,    42,    -1,    44,    45,    -1,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,
      59,    -1,    -1,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    81,    82,    -1,     5,    -1,    -1,    -1,    88,
      10,    90,    -1,    13,    14,    15,    -1,    -1,    -1,    19,
      20,    21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    -1,    42,    -1,    44,    45,    -1,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    81,    82,    -1,     5,    -1,    -1,    -1,    88,    10,
      90,    -1,    13,    14,    15,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      31,    32,    -1,    34,    35,    36,    37,    -1,    39,    40,
      -1,    42,    -1,    44,    45,    -1,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    55,    56,    57,    58,    59,    -1,
      -1,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      81,    82,    -1,     5,    -1,    -1,    -1,    88,    10,    90,
      -1,    13,    14,    15,    -1,    -1,    -1,    19,    20,    21,
      22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,    31,
      32,    -1,    34,    35,    36,    37,    -1,    39,    40,    -1,
      42,    -1,    44,    45,    -1,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    81,
      82,    -1,     5,    -1,    -1,    -1,    88,    10,    90,    -1,
      13,    14,    15,    -1,    -1,    -1,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    -1,    -1,    -1,    31,    32,
      -1,    34,    35,    36,    37,    -1,    39,    40,    -1,    42,
      -1,    44,    45,    -1,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    57,    58,    59,    -1,    -1,    -1,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    81,    82,
      -1,     5,    -1,    -1,    -1,    88,    10,    90,    -1,    13,
      14,    15,    -1,    -1,    -1,    19,    20,    21,    22,    23,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    -1,    42,    -1,
      44,    45,    -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    -1,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    88,    -1,    90,    19,    20,    21,
      22,    23,    -1,    -1,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    -1,
      -1,    -1,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    13,    -1,    -1,    77,    -1,    -1,    19,    20,
      21,    22,    23,    24,    86,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    36,    37,    38,    39,    40,
      -1,    42,    43,    44,    45,    -1,    47,    48,    49,    -1,
      -1,    -1,    -1,    54,    55,    56,    57,    58,    59,    -1,
      -1,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    -1,    74,    13,    -1,    77,    -1,    -1,    -1,
      19,    20,    21,    22,    23,    86,    -1,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    36,    37,    38,
      39,    40,    -1,    42,    43,    44,    45,    -1,    47,    48,
      49,    -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,
      59,    -1,    -1,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    -1,    -1,    13,    -1,    76,    77,    -1,
      -1,    19,    20,    21,    22,    23,    -1,    86,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    36,    37,
      38,    39,    40,    -1,    42,    43,    44,    45,    -1,    47,
      48,    49,    -1,    -1,    -1,    -1,    54,    55,    56,    57,
      58,    59,    -1,    -1,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    -1,    -1,    -1,    75,    -1,    77,
      -1,    -1,    -1,    13,    -1,    -1,    16,    -1,    86,    19,
      20,    21,    22,    23,    -1,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    38,    39,
      40,    -1,    42,    43,    44,    45,    -1,    47,    48,    49,
      -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    -1,    13,    -1,    -1,    77,    -1,    -1,
      19,    20,    21,    22,    23,    -1,    86,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    36,    37,    38,
      39,    40,    -1,    42,    43,    44,    45,    -1,    47,    48,
      49,    -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,
      59,    -1,    -1,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    -1,    -1,    74,    13,    -1,    77,    -1,
      -1,    -1,    19,    20,    21,    22,    23,    86,    -1,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    36,
      37,    38,    39,    40,    -1,    42,    43,    44,    45,    -1,
      47,    48,    49,    -1,    -1,    -1,    -1,    54,    55,    56,
      57,    58,    59,    -1,    -1,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    -1,    -1,    74,    13,    -1,
      77,    -1,    -1,    -1,    19,    20,    21,    22,    23,    86,
      -1,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    36,    37,    38,    39,    40,    -1,    42,    43,    44,
      45,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,
      55,    56,    57,    58,    59,    -1,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    -1,    -1,    13,
      75,    -1,    77,    -1,    -1,    19,    20,    21,    22,    23,
      -1,    86,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    38,    39,    40,    -1,    42,    43,
      44,    45,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,
      54,    55,    56,    57,    58,    59,    -1,    -1,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    -1,    -1,
      13,    75,    -1,    77,    -1,    -1,    19,    20,    21,    22,
      23,    -1,    86,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    36,    37,    38,    39,    40,    -1,    42,
      43,    44,    45,    -1,    47,    48,    49,    -1,    -1,    -1,
      -1,    54,    55,    56,    57,    58,    59,    -1,    -1,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    -1,
      -1,    13,    -1,    -1,    77,    -1,    -1,    19,    20,    21,
      22,    23,    -1,    86,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,    35,    36,    37,    38,    39,    40,    -1,
      42,    43,    44,    45,    -1,    47,    48,    49,    -1,    -1,
      -1,    -1,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      -1,    -1,    13,    -1,    -1,    77,    -1,    -1,    19,    20,
      21,    22,    23,    -1,    86,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    36,    37,    38,    39,    40,
      -1,    42,    43,    44,    45,    -1,    47,    48,    49,    -1,
      -1,    -1,    -1,    54,    55,    56,    57,    58,    59,    -1,
      -1,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    -1,    13,    -1,    -1,    77,    -1,    -1,    19,
      20,    21,    22,    23,    -1,    86,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    38,    39,
      40,    -1,    42,    43,    44,    45,    -1,    47,    48,    49,
      -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    -1,    13,    -1,    -1,    77,    -1,    -1,
      19,    20,    21,    22,    23,    -1,    86,    26,    27,    28,
      29,    -1,    31,    32,    -1,    34,    35,    36,    37,    38,
      39,    40,    -1,    42,    -1,    44,    45,    -1,    47,    48,
      49,    -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,
      59,    -1,    -1,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    13,    14,    15,    -1,    -1,    77,    19,
      20,    21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    -1,    42,    -1,    44,    45,    -1,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    13,    -1,    -1,    -1,    -1,    77,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      31,    32,    -1,    34,    35,    36,    37,    -1,    39,    40,
      -1,    42,    -1,    44,    45,    -1,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    55,    56,    57,    58,    59,    -1,
      -1,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      50,    51,    52,    53,    54,    55,    77,    57,    -1,    -1,
      60,    61,    62,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    60,    61,    62,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    87,    -1,    89,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    89,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    60,    61,    62,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    60,    61,    62,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      87,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    60,    61,    62,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    60,    61,    62,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    60,    61,    62,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    60,    61,    62,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    13,    19,    20,    21,    22,    23,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    42,    43,    44,    45,    46,    47,    48,    49,
      54,    55,    56,    57,    58,    59,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    77,    86,    94,
      95,    96,    97,    98,   103,   105,   108,   109,   110,   113,
     115,   116,   117,   118,   119,   120,    39,    86,    35,    39,
      40,    70,    77,   105,   106,   107,     5,    10,    14,    15,
      27,    34,    56,    81,    82,    88,    90,   105,   111,   112,
     124,   125,   128,   131,   132,   133,   135,    88,   135,   105,
      39,   111,   135,    44,    45,   105,   125,   127,   105,   105,
     111,   112,   106,   135,   106,   105,   109,     0,    96,   111,
     135,    86,    86,    87,   135,   135,   133,   133,   133,    89,
     135,    14,    15,    92,   105,   129,   130,   135,   137,   138,
      88,    86,    86,    36,    50,    51,    52,    53,    54,    55,
      57,    60,    61,    62,    78,    79,    80,    81,    82,    83,
      84,    87,   124,   131,   134,   136,   135,    41,    54,    55,
      57,    62,   106,    86,    86,    32,    58,    59,   105,   135,
      42,    52,   114,    41,    86,    86,    86,    86,    42,    86,
      86,   106,    86,   105,    36,   136,    87,    89,    92,    92,
      91,    92,    87,    91,    91,    87,   137,    34,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   138,    50,    51,    52,    53,
      54,    55,    57,    60,    61,    62,    78,    79,    80,    81,
      82,    83,    84,    87,    86,   108,    86,   125,   126,   126,
     126,    52,    86,   135,   135,    73,    86,    99,   100,   101,
     102,   109,    86,   108,   121,   122,   123,    86,    34,   138,
     135,   135,   135,    14,    15,   105,   130,   135,    89,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   138,   121,    24,
     121,    86,    86,    86,   135,   121,    86,   105,    86,    76,
     123,   121,   135,    89,    24,    74,    86,   108,    23,    42,
      75,   106,   102,    86,    16,    86,   108,    86,   121,    42,
     135,    86,    86,   104,   105,   121,    74,   135,    86,   121,
      86,    87,    74,    86,    86,   121,   105,    86,   121,    75,
      75,    86,    86
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    93,    94,    95,    95,    96,    96,    96,    96,    96,
      97,    98,    99,   100,   100,   101,   101,   102,   102,   103,
     103,   104,   104,   104,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   106,
     106,   107,   107,   108,   108,   108,   108,   108,   108,   109,
     109,   109,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   111,   111,   111,   111,   111,   111,   111,   112,
     112,   112,   112,   112,   113,   113,   113,   113,   114,   114,
     115,   115,   115,   116,   116,   117,   117,   117,   117,   118,
     118,   118,   119,   119,   120,   121,   121,   122,   122,   123,
     123,   124,   124,   124,   124,   124,   124,   124,   124,   125,
     126,   127,   128,   128,   128,   129,   129,   130,   130,   130,
     131,   132,   132,   133,   133,   133,   134,   134,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   137,   137,   138,   138
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       5,     4,     5,     0,     1,     1,     3,     1,     1,     8,
       5,     0,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     0,     1,     3,     2,     4,     2,     5,     2,
       2,     5,     4,     3,     5,     5,     5,     5,     1,     1,
       3,     3,     3,     4,     7,     6,     9,     7,    10,     7,
      11,    12,     4,     6,     4,     0,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     2,     4,     1,     1,     1,
       1,     1,     3,     3,     3,     1,     3,     3,     3,     3,
       3,     2,     2,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     0,     1,     1,     3
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
    case YYSYMBOL_tTHEFUNC: /* tTHEFUNC  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1776 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tTHEFUNCINOF: /* tTHEFUNCINOF  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1782 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tVARID: /* tVARID  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1788 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1794 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1800 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1806 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tPLAYACCEL: /* tPLAYACCEL  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1812 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1818 "engines/director/lingo/lingo-gr.cpp"
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
#line 196 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->_compiler->_assemblyAST = new ScriptNode((yyvsp[0].nodelist)); }
#line 2086 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 198 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2097 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 204 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2107 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 211 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2113 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 244 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2119 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* factory: tFACTORY ID '\n' methodlist  */
#line 248 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2125 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 250 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2131 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* methodlist: %empty  */
#line 252 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2137 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* nonemptymethodlist: methodlistline  */
#line 256 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlist '\n' methodlistline  */
#line 262 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 2158 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlistline: '\n'  */
#line 269 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2164 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 275 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2173 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* handler: tON ID idlist '\n' stmtlist  */
#line 279 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2180 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* endargdef: ID  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2186 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* endargdef: endargdef ',' ID  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2192 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* ID: tAFTER  */
#line 291 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2198 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* ID: tAND  */
#line 292 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 2204 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* ID: tBEFORE  */
#line 293 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2210 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* ID: tCAST  */
#line 294 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2216 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* ID: tCHAR  */
#line 295 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2222 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* ID: tDOWN  */
#line 296 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* ID: tFIELD  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2234 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* ID: tFRAME  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2240 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* ID: tIN  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2246 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* ID: tINTERSECTS  */
#line 307 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* ID: tINTO  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2258 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* ID: tITEM  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2264 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* ID: tLINE  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* ID: tMOD  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* ID: tMOVIE  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2282 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* ID: tNEXT  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2288 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* ID: tNOT  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2294 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* ID: tOF  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2300 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* ID: tOPEN  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 2306 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* ID: tOR  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2312 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* ID: tPREVIOUS  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2318 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* ID: tREPEAT  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2324 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* ID: tSCRIPT  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2330 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* ID: tSET  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 2336 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* ID: tSTARTS  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2342 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* ID: tTELL  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 2348 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* ID: tTO  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2354 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* ID: tASSERTERROR  */
#line 332 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2360 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* ID: tSPRITE  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* ID: tWHILE  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 2372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* ID: tWINDOW  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2378 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* ID: tWITH  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 2384 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* ID: tWITHIN  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 2390 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* ID: tWORD  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 2396 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* idlist: %empty  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 2402 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* nonemptyidlist: ID  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 2411 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 2419 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* proc: ID cmdargs '\n'  */
#line 373 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist)); }
#line 2425 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* proc: tPUT cmdargs '\n'  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist)); }
#line 2431 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* proc: tGO cmdargs '\n'  */
#line 375 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist)); }
#line 2437 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* proc: tGO frameargs '\n'  */
#line 376 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist)); }
#line 2443 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* proc: tPLAY cmdargs '\n'  */
#line 377 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 2449 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* proc: tPLAY frameargs '\n'  */
#line 378 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 2455 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* proc: tNEXT tREPEAT '\n'  */
#line 379 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 2461 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* proc: tEXIT tREPEAT '\n'  */
#line 380 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 2467 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* proc: tEXIT '\n'  */
#line 381 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 2473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* proc: tASSERTERROR stmtoneliner  */
#line 382 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 2479 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* cmdargs: %empty  */
#line 385 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 2487 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* cmdargs: expr  */
#line 388 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2497 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* cmdargs: expr ',' nonemptyexprlist  */
#line 393 "engines/director/lingo/lingo-gr.y"
                                                {
		// This matches `cmd args, ...)
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 2506 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* cmdargs: expr expr_nounarymath  */
#line 397 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2517 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist  */
#line 403 "engines/director/lingo/lingo-gr.y"
                                                           {
		// This matches `cmd arg arg, ...`
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 2527 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* cmdargs: '(' ')'  */
#line 408 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 2535 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* cmdargs: '(' expr ',' nonemptyexprlist ')'  */
#line 411 "engines/director/lingo/lingo-gr.y"
                                                  {
		// This matches `cmd(args, ...)`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2544 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* frameargs: tFRAME expr  */
#line 421 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2554 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* frameargs: tMOVIE expr  */
#line 426 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2565 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 432 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* frameargs: expr tOF tMOVIE expr  */
#line 438 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2587 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* frameargs: tFRAME expr expr_nounarymath  */
#line 444 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2598 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 452 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2604 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 453 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2610 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 454 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2616 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* asgn: tSET varorthe to expr '\n'  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2622 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* definevars: tGLOBAL idlist '\n'  */
#line 460 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 2628 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* definevars: tPROPERTY idlist '\n'  */
#line 461 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 2634 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102: /* definevars: tINSTANCE idlist '\n'  */
#line 462 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 2640 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* ifstmt: tIF expr tTHEN stmt  */
#line 465 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 2649 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* ifstmt: tIF expr tTHEN '\n' stmtlist tENDIF '\n'  */
#line 469 "engines/director/lingo/lingo-gr.y"
                                                   {
		(yyval.node) = new IfStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist)); }
#line 2656 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 473 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 2667 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist tENDIF '\n'  */
#line 479 "engines/director/lingo/lingo-gr.y"
                                                                                {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-5].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), stmtlist1, (yyvsp[-2].nodelist)); }
#line 2676 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE stmt  */
#line 483 "engines/director/lingo/lingo-gr.y"
                                                                    {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 2685 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE '\n' stmtlist tENDIF '\n'  */
#line 487 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new IfElseStmtNode((yyvsp[-8].node), (yyvsp[-5].nodelist), (yyvsp[-2].nodelist)); }
#line 2692 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 491 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 2699 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 493 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 2706 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 495 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 2713 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* tell: tTELL expr tTO stmtoneliner  */
#line 499 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 2722 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 503 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 2729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* when: tWHEN ID tTHEN expr  */
#line 507 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new WhenNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 2735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* stmtlist: %empty  */
#line 509 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* nonemptystmtlist: stmtlistline  */
#line 514 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2752 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* nonemptystmtlist: stmtlist stmtlistline  */
#line 520 "engines/director/lingo/lingo-gr.y"
                                                {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2762 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* stmtlistline: '\n'  */
#line 527 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 2768 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* simpleexpr_noparens_nounarymath: tINT  */
#line 534 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 2774 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* simpleexpr_noparens_nounarymath: tFLOAT  */
#line 535 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 2780 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* simpleexpr_noparens_nounarymath: tSYMBOL  */
#line 536 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 2786 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* simpleexpr_noparens_nounarymath: tSTRING  */
#line 537 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 2792 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* simpleexpr_noparens_nounarymath: tNOT simpleexpr  */
#line 538 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 2798 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* simpleexpr_noparens_nounarymath: ID '(' exprlist ')'  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new FuncNode((yyvsp[-3].s), (yyvsp[-1].nodelist)); }
#line 2804 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* var: ID  */
#line 544 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 2810 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* list: '[' exprlist ']'  */
#line 554 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 2816 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* list: '[' ':' ']'  */
#line 555 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 2822 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* list: '[' proplist ']'  */
#line 556 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 2828 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* proplist: proppair  */
#line 559 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 2837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* proplist: proplist ',' proppair  */
#line 563 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 2845 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* proppair: tSYMBOL ':' expr  */
#line 568 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 2851 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* proppair: ID ':' expr  */
#line 569 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 2857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* proppair: tSTRING ':' expr  */
#line 570 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 2863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* parens: '(' expr ')'  */
#line 573 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[-1].node); }
#line 2869 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* unarymath: '+' simpleexpr  */
#line 575 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 2875 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* unarymath: '-' simpleexpr  */
#line 576 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 2881 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* expr: expr '+' expr  */
#line 594 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* expr: expr '-' expr  */
#line 595 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* expr: expr '*' expr  */
#line 596 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* expr: expr '/' expr  */
#line 597 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2905 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* expr: expr tMOD expr  */
#line 598 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* expr: expr '>' expr  */
#line 599 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* expr: expr '<' expr  */
#line 600 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* expr: expr tEQ expr  */
#line 601 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2929 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* expr: expr tNEQ expr  */
#line 602 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* expr: expr tGE expr  */
#line 603 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2941 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* expr: expr tLE expr  */
#line 604 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* expr: expr tAND expr  */
#line 605 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2953 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* expr: expr tOR expr  */
#line 606 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* expr: expr '&' expr  */
#line 607 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* expr: expr tCONCAT expr  */
#line 608 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2971 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* expr: expr tCONTAINS expr  */
#line 609 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* expr: expr tSTARTS expr  */
#line 610 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 618 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 619 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 620 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 621 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 622 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 623 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 624 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 625 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 626 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 627 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 628 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 629 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 630 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 631 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3067 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 632 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3073 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 633 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 634 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* exprlist: %empty  */
#line 637 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 3091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* nonemptyexprlist: expr  */
#line 641 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 3100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 645 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3108 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3112 "engines/director/lingo/lingo-gr.cpp"

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

#line 650 "engines/director/lingo/lingo-gr.y"


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
