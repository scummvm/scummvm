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
  YYSYMBOL_tGOLOOP = 30,                   /* tGOLOOP  */
  YYSYMBOL_tIF = 31,                       /* tIF  */
  YYSYMBOL_tIN = 32,                       /* tIN  */
  YYSYMBOL_tINTO = 33,                     /* tINTO  */
  YYSYMBOL_tMACRO = 34,                    /* tMACRO  */
  YYSYMBOL_tMOVIE = 35,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 36,                     /* tNEXT  */
  YYSYMBOL_tOF = 37,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 38,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 39,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 40,                   /* tREPEAT  */
  YYSYMBOL_tSET = 41,                      /* tSET  */
  YYSYMBOL_tTHEN = 42,                     /* tTHEN  */
  YYSYMBOL_tTO = 43,                       /* tTO  */
  YYSYMBOL_tWHEN = 44,                     /* tWHEN  */
  YYSYMBOL_tWITH = 45,                     /* tWITH  */
  YYSYMBOL_tWHILE = 46,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 47,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 48,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 49,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 50,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 51,                       /* tGE  */
  YYSYMBOL_tLE = 52,                       /* tLE  */
  YYSYMBOL_tEQ = 53,                       /* tEQ  */
  YYSYMBOL_tNEQ = 54,                      /* tNEQ  */
  YYSYMBOL_tAND = 55,                      /* tAND  */
  YYSYMBOL_tOR = 56,                       /* tOR  */
  YYSYMBOL_tNOT = 57,                      /* tNOT  */
  YYSYMBOL_tMOD = 58,                      /* tMOD  */
  YYSYMBOL_tAFTER = 59,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 60,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 61,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 62,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 63,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 64,                     /* tCHAR  */
  YYSYMBOL_tITEM = 65,                     /* tITEM  */
  YYSYMBOL_tLINE = 66,                     /* tLINE  */
  YYSYMBOL_tWORD = 67,                     /* tWORD  */
  YYSYMBOL_tSPRITE = 68,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 69,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 70,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 71,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 72,                 /* tPROPERTY  */
  YYSYMBOL_tON = 73,                       /* tON  */
  YYSYMBOL_tMETHOD = 74,                   /* tMETHOD  */
  YYSYMBOL_tENDIF = 75,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 76,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 77,                  /* tENDTELL  */
  YYSYMBOL_tASSERTERROR = 78,              /* tASSERTERROR  */
  YYSYMBOL_79_ = 79,                       /* '<'  */
  YYSYMBOL_80_ = 80,                       /* '>'  */
  YYSYMBOL_81_ = 81,                       /* '&'  */
  YYSYMBOL_82_ = 82,                       /* '+'  */
  YYSYMBOL_83_ = 83,                       /* '-'  */
  YYSYMBOL_84_ = 84,                       /* '*'  */
  YYSYMBOL_85_ = 85,                       /* '/'  */
  YYSYMBOL_86_ = 86,                       /* '%'  */
  YYSYMBOL_87_n_ = 87,                     /* '\n'  */
  YYSYMBOL_88_ = 88,                       /* ','  */
  YYSYMBOL_89_ = 89,                       /* '('  */
  YYSYMBOL_90_ = 90,                       /* ')'  */
  YYSYMBOL_91_ = 91,                       /* '['  */
  YYSYMBOL_92_ = 92,                       /* ']'  */
  YYSYMBOL_93_ = 93,                       /* ':'  */
  YYSYMBOL_YYACCEPT = 94,                  /* $accept  */
  YYSYMBOL_script = 95,                    /* script  */
  YYSYMBOL_scriptpartlist = 96,            /* scriptpartlist  */
  YYSYMBOL_scriptpart = 97,                /* scriptpart  */
  YYSYMBOL_macro = 98,                     /* macro  */
  YYSYMBOL_factory = 99,                   /* factory  */
  YYSYMBOL_method = 100,                   /* method  */
  YYSYMBOL_methodlist = 101,               /* methodlist  */
  YYSYMBOL_nonemptymethodlist = 102,       /* nonemptymethodlist  */
  YYSYMBOL_methodlistline = 103,           /* methodlistline  */
  YYSYMBOL_handler = 104,                  /* handler  */
  YYSYMBOL_endargdef = 105,                /* endargdef  */
  YYSYMBOL_ID = 106,                       /* ID  */
  YYSYMBOL_idlist = 107,                   /* idlist  */
  YYSYMBOL_nonemptyidlist = 108,           /* nonemptyidlist  */
  YYSYMBOL_stmt = 109,                     /* stmt  */
  YYSYMBOL_stmtoneliner = 110,             /* stmtoneliner  */
  YYSYMBOL_proc = 111,                     /* proc  */
  YYSYMBOL_cmdargs = 112,                  /* cmdargs  */
  YYSYMBOL_frameargs = 113,                /* frameargs  */
  YYSYMBOL_asgn = 114,                     /* asgn  */
  YYSYMBOL_to = 115,                       /* to  */
  YYSYMBOL_definevars = 116,               /* definevars  */
  YYSYMBOL_ifstmt = 117,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 118,               /* ifelsestmt  */
  YYSYMBOL_loop = 119,                     /* loop  */
  YYSYMBOL_stmtlist = 120,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 121,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 122,             /* stmtlistline  */
  YYSYMBOL_simpleexpr_noparens_nounarymath = 123, /* simpleexpr_noparens_nounarymath  */
  YYSYMBOL_var = 124,                      /* var  */
  YYSYMBOL_varorchunk = 125,               /* varorchunk  */
  YYSYMBOL_varorthe = 126,                 /* varorthe  */
  YYSYMBOL_list = 127,                     /* list  */
  YYSYMBOL_proplist = 128,                 /* proplist  */
  YYSYMBOL_proppair = 129,                 /* proppair  */
  YYSYMBOL_parens = 130,                   /* parens  */
  YYSYMBOL_unarymath = 131,                /* unarymath  */
  YYSYMBOL_simpleexpr = 132,               /* simpleexpr  */
  YYSYMBOL_simpleexpr_nounarymath = 133,   /* simpleexpr_nounarymath  */
  YYSYMBOL_expr = 134,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 135,         /* expr_nounarymath  */
  YYSYMBOL_exprlist = 136,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 137          /* nonemptyexprlist  */
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
#define YYFINAL  107
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2463

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  94
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  44
/* YYNRULES -- Number of rules.  */
#define YYNRULES  180
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  333

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   333


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
      87,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    86,    81,     2,
      89,    90,    84,    82,    88,    83,     2,    85,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    93,     2,
      79,     2,    80,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    91,     2,    92,     2,     2,     2,     2,     2,     2,
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
      75,    76,    77,    78
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
     329,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     342,   343,   346,   350,   360,   361,   362,   363,   366,   367,
     368,   371,   372,   373,   374,   375,   376,   377,   380,   383,
     388,   392,   398,   403,   406,   416,   421,   427,   433,   439,
     447,   448,   449,   450,   453,   453,   455,   456,   457,   460,
     464,   468,   474,   478,   482,   486,   488,   490,   494,   495,
     499,   505,   512,   513,   519,   520,   521,   522,   523,   524,
     525,   526,   529,   531,   535,   539,   540,   541,   544,   548,
     553,   554,   555,   558,   560,   561,   564,   565,   566,   573,
     574,   578,   579,   580,   581,   582,   583,   584,   585,   586,
     587,   588,   589,   590,   591,   592,   593,   594,   595,   602,
     603,   604,   605,   606,   607,   608,   609,   610,   611,   612,
     613,   614,   615,   616,   617,   618,   619,   622,   623,   626,
     630
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
  "tEXIT", "tFRAME", "tGLOBAL", "tGO", "tGOLOOP", "tIF", "tIN", "tINTO",
  "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT",
  "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tFACTORY", "tOPEN",
  "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT",
  "tMOD", "tAFTER", "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR",
  "tITEM", "tLINE", "tWORD", "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL",
  "tPROPERTY", "tON", "tMETHOD", "tENDIF", "tENDREPEAT", "tENDTELL",
  "tASSERTERROR", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'", "'/'", "'%'",
  "'\\n'", "','", "'('", "')'", "'['", "']'", "':'", "$accept", "script",
  "scriptpartlist", "scriptpart", "macro", "factory", "method",
  "methodlist", "nonemptymethodlist", "methodlistline", "handler",
  "endargdef", "ID", "idlist", "nonemptyidlist", "stmt", "stmtoneliner",
  "proc", "cmdargs", "frameargs", "asgn", "to", "definevars", "ifstmt",
  "ifelsestmt", "loop", "stmtlist", "nonemptystmtlist", "stmtlistline",
  "simpleexpr_noparens_nounarymath", "var", "varorchunk", "varorthe",
  "list", "proplist", "proppair", "parens", "unarymath", "simpleexpr",
  "simpleexpr_nounarymath", "expr", "expr_nounarymath", "exprlist",
  "nonemptyexprlist", YY_NULLPTR
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,    60,
      62,    38,    43,    45,    42,    47,    37,    10,    44,    40,
      41,    91,    93,    58
};
#endif

#define YYPACT_NINF (-225)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1322,  -225,  -225,  -225,  -225,  -225,  -225,   -33,  -225,  2135,
     929,  -225,  -225,  2135,  -225,   -30,  -225,  -225,  1009,    20,
    2135,  -225,  -225,  -225,  -225,  2135,  -225,  1089,  2135,  -225,
    -225,  -225,  -225,  -225,  -225,  -225,  -225,  -225,  -225,  -225,
    -225,  -225,  -225,  -225,  2135,  2135,  -225,  -225,    34,  1322,
    -225,  -225,  -225,  -225,  1009,  -225,  -225,  -225,  -225,  -225,
    -225,  -225,  -225,   -51,  -225,  -225,  -225,  -225,  -225,   -41,
     -38,  -225,  -225,  -225,  -225,   929,   929,   929,   929,   437,
     -29,  -225,  -225,  -225,  -225,  -225,  -225,  2179,  2135,   -10,
     847,    -9,   519,  2135,   929,  -225,  -225,   -35,    -5,   929,
     929,    -2,    -1,   601,     3,     7,  2135,  -225,  -225,     9,
     683,  -225,  -225,  2135,  -225,  -225,  -225,  2214,   -31,   -17,
     -12,   -78,   -66,  -225,  2365,     5,    10,   929,  1828,   929,
     929,   929,   929,   929,   929,   929,   929,   929,   929,   929,
     929,   929,   929,   929,   929,   929,    12,  -225,  -225,   310,
    -225,  2135,  1169,  1169,  1249,  2135,  2135,   155,   929,  -225,
    -225,  -225,  2227,    47,  2265,  -225,  -225,   929,   -23,   765,
    2365,  -225,  -225,    52,  -225,  -225,    16,  -225,  -225,  -225,
     929,   929,  -225,   929,  2075,  -225,  -225,   929,    15,  1890,
      83,   -26,   -26,   -26,   -26,   189,   189,  -225,   -44,   -26,
     -26,   -26,   -26,   -44,   -37,   -37,  -225,  -225,  1890,   929,
    -225,    21,    23,    24,    10,   929,   929,   929,   929,   929,
     929,   929,   929,   929,   929,   929,   929,   929,   929,   929,
     929,   929,   929,   929,  1890,  2278,  2135,  -225,  -225,    25,
    -225,  -225,    79,  2378,   929,  1890,  2365,  2365,  2365,   -31,
     -17,    26,  -225,  2365,  -225,  -225,  -225,  1384,  -225,  -225,
    1952,  1890,   -61,  -225,  -225,  -225,   -26,   -26,   -26,   -26,
     189,   189,  -225,   -44,   -26,   -26,   -26,   -26,   -44,   -37,
     -37,  -225,  -225,    10,  2164,  1447,  -225,  2135,   -23,   929,
    2365,  1516,  2014,    29,  -225,  1890,  -225,  -225,    74,   929,
      31,    33,  -225,  2365,  2135,  1890,  -225,  -225,  1578,   929,
    2315,  -225,  1890,   -16,  -225,  1641,    35,  2328,  1890,  1890,
    -225,  2135,    36,  -225,  1890,  1704,  -225,  -225,  1766,    37,
      38,  -225,  -225
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    24,    28,    31,    47,    56,    30,     0,    32,    60,
       0,    33,    35,     0,    39,    40,    42,    45,    78,    46,
      48,    51,    54,    57,    55,     0,    43,    78,    60,    26,
      44,    41,    38,    25,    27,    49,    29,    36,    37,    59,
      53,    34,    58,    50,    60,     0,    52,     5,     0,     2,
       3,     6,     7,     8,    78,     9,    64,    68,    69,    70,
      65,    66,    67,     0,    77,    40,    46,    48,    62,     0,
      61,   114,   115,   117,   116,    41,     0,     0,     0,   177,
     122,   136,   120,   121,   137,   138,   141,     0,    60,     0,
       0,     0,    79,     0,     0,   122,   124,     0,     0,    32,
      39,     0,     0,    79,     0,     0,    60,     1,     4,     0,
      79,    76,    96,     0,   118,   134,   135,     0,   117,   116,
       0,   122,     0,   128,   179,     0,   178,   177,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,    83,     0,
      72,    35,    26,    44,    38,    25,    27,    49,     0,   139,
     140,   159,    81,     0,     0,    94,    95,     0,    13,    85,
      86,    73,    74,    42,    98,    97,     0,    71,    63,   133,
       0,     0,   126,     0,     0,   127,   125,     0,     0,   108,
      99,   151,   152,   149,   150,   153,   154,   146,   156,   157,
     158,   148,   147,   155,   142,   143,   144,   145,   108,     0,
     123,     0,     0,     0,    80,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   108,     0,     0,    17,    18,    11,
      14,    15,    42,    89,     0,   108,   132,   130,   131,     0,
       0,     0,   129,   180,   119,   112,   113,     0,   109,   110,
       0,    10,     0,    90,    91,    92,   169,   170,   167,   168,
     171,   172,   164,   174,   175,   176,   166,   165,   173,   160,
     161,   162,   163,    82,     0,     0,    93,    60,     0,     0,
      88,    20,     0,     0,   111,   108,   101,    84,     0,     0,
       0,     0,    16,    87,    21,   108,   103,   100,     0,     0,
       0,   105,   108,     0,    22,     0,     0,     0,   108,    12,
      19,     0,     0,   102,   108,     0,    23,   104,     0,     0,
       0,   106,   107
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -225,  -225,  -225,    77,  -225,  -225,  -225,  -225,  -225,  -161,
    -225,  -225,     0,   -25,  -225,     4,  -225,  -225,   -11,  -225,
    -225,  -225,  -225,  -225,  -225,  -225,  -203,  -225,  -224,   -86,
     -18,   -82,  -225,  -225,  -225,   -55,   -80,  -225,    -7,  -225,
     200,   -39,     8,  -157
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    48,    49,    50,    51,    52,   238,   239,   240,   241,
      53,   313,    80,    69,    70,   256,    56,    57,    91,   102,
      58,   167,    59,    60,    61,    62,   257,   258,   259,    81,
      82,   211,    97,    83,   122,   123,    84,    85,    86,   161,
     124,   162,   125,   126
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      54,   214,    96,   104,    55,   261,   159,    63,   165,    68,
      89,   127,   160,    88,   135,   183,   101,   159,   166,   105,
      95,   135,   184,   160,   159,    98,   185,   187,    68,   297,
     160,   285,   135,   294,   107,   136,   111,   294,   142,   143,
     144,   145,   291,   109,    68,   106,   112,   144,   145,    54,
     113,   236,   262,    55,    64,   141,   142,   143,   144,   145,
     127,   294,   180,   146,   237,    93,    94,   294,   114,   115,
     116,   320,   321,   212,   213,   283,   181,   147,   150,   121,
     182,   176,   168,   159,   294,   171,   172,   244,    68,   160,
     174,   294,   308,   163,   175,   294,   177,   186,   187,   208,
     233,   294,   315,   245,   294,   254,    68,   260,   263,   319,
     264,   265,   288,   178,   289,   325,   307,   309,   311,   183,
     312,   328,   323,   327,   331,   332,   108,   302,    54,   252,
     243,     0,   190,   210,     0,   188,     0,   210,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    95,     0,     0,     0,    95,    95,     0,     0,     0,
      71,     0,     0,     0,     0,    72,     0,     0,     1,    73,
      74,     0,     0,     0,     2,     3,     4,     5,     6,     0,
       0,     0,     8,     0,   251,     0,     0,    11,    12,    54,
      14,    65,    16,    17,     0,    66,    67,     0,    21,    22,
      23,    24,     0,    26,     0,     0,     0,     0,    54,     0,
      87,     0,    75,    32,    33,    34,     0,     0,    92,    36,
      37,    38,    39,    40,    41,    42,    43,   103,     0,     0,
       0,     0,     0,    46,    54,     0,   287,    76,    77,     0,
     129,   130,   131,   132,    78,    54,    79,   135,     0,     0,
     136,   137,   138,     0,   110,     0,     0,    54,     0,     0,
      54,    54,   301,     0,   296,     0,     0,     0,   139,   140,
     141,   142,   143,   144,   145,     0,     0,     0,   117,     0,
       0,     0,     0,     0,     0,    54,     0,    68,     0,     0,
     149,    54,    54,     0,   164,    54,   306,     0,     0,   169,
     170,     0,     0,     0,   314,    54,     0,     0,    54,     0,
       0,     0,    54,     0,     0,    54,     0,     0,    54,    54,
       0,   326,     0,     0,    54,    54,     0,     0,    54,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,     0,     0,     0,     0,
       0,     0,   195,   196,   197,     0,     0,   200,     0,     0,
       0,   129,   130,   131,   132,   133,   134,   235,   135,     0,
       0,   136,   137,   138,     0,     0,     0,     0,     0,     0,
     246,   247,     0,   248,     0,     0,     0,   253,     0,   139,
     140,   141,   142,   143,   144,   145,     0,     0,   209,     0,
     179,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,     0,   284,     0,     0,     0,     0,     0,     0,
       0,     0,    71,     0,   290,     0,     0,    72,     0,     0,
       1,   118,   119,     0,     0,     0,     2,     3,     4,     5,
       6,     0,     0,     0,     8,     0,     0,     0,     0,    11,
      12,     0,    14,    65,    16,    17,     0,    66,    67,     0,
      21,    22,    23,    24,     0,    26,     0,     0,     0,   303,
       0,     0,    29,    30,    75,    32,    33,    34,     0,   310,
      35,    36,    37,    38,    39,    40,    41,    42,    43,   317,
       0,     0,     0,     0,     0,    46,     0,     0,     0,    76,
      77,     0,     0,     0,    71,     0,    78,     0,    79,    72,
     120,     0,     1,    73,    74,     0,     0,     0,     2,     3,
       4,     5,     6,     0,     0,     0,     8,     0,     0,     0,
       0,    11,   151,     0,    14,    65,    16,    17,     0,    66,
      67,     0,    21,    22,    23,    24,     0,    26,     0,     0,
     129,   130,   131,   132,   152,   153,    75,   154,   155,   156,
     136,   137,   157,    36,    37,    38,    39,    40,    41,    42,
      43,     0,     0,     0,     0,     0,     0,    46,   139,   140,
     141,   142,   143,   144,   145,     0,    71,   158,    78,     0,
      79,    72,     0,     0,     1,    73,    74,     0,     0,     0,
       2,     3,     4,     5,     6,     0,     0,     0,     8,     0,
       0,     0,     0,    11,    12,     0,    14,    65,   173,    17,
       0,    66,    67,     0,    21,    22,    23,    24,     0,    26,
       0,     0,   129,   130,   131,   132,   152,   153,    75,   154,
      33,    34,   136,   137,   157,    36,    37,    38,    39,    40,
      41,    42,    43,     0,     0,     0,     0,     0,     0,    46,
     139,   140,   141,   142,   143,   144,   145,     0,    71,   158,
      78,     0,    79,    72,     0,     0,     1,    73,    74,     0,
       0,     0,     2,     3,     4,     5,     6,     0,     0,     0,
       8,     0,     0,     0,     0,    11,    12,     0,    14,    65,
      16,    17,     0,    66,    67,     0,    21,    22,    23,    24,
       0,    26,     0,     0,   129,   130,   131,   132,   152,   153,
      75,   154,    33,    34,   136,   137,   157,    36,    37,    38,
      39,    40,    41,    42,    43,     0,     0,     0,     0,     0,
       0,    46,   139,   140,   141,   142,   143,   144,   145,     0,
      71,   158,    78,     0,    79,    72,     0,     0,     1,    73,
      74,     0,     0,     0,     2,     3,     4,     5,     6,     0,
       0,     0,     8,     0,     0,     0,     0,    11,    12,     0,
      14,    65,   242,    17,     0,    66,    67,     0,    21,    22,
      23,    24,     0,    26,     0,     0,   129,   130,   131,   132,
     152,   153,    75,   154,    33,    34,   136,   137,   157,    36,
      37,    38,    39,    40,    41,    42,    43,     0,     0,     0,
       0,     0,     0,    46,   139,   140,   141,   142,   143,   144,
     145,     0,    71,     0,    78,     0,    79,    72,     0,     0,
       1,    73,    74,     0,     0,     0,     2,     3,     4,     5,
       6,     0,     0,     0,     8,     0,     0,     0,     0,    11,
      12,     0,    14,    65,    16,    17,     0,    66,    67,     0,
      21,    22,    23,    24,     0,    26,     0,     0,     0,     0,
       0,     0,    29,    30,    75,    32,    33,    34,     0,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,     0,
       0,     0,     0,     0,     0,    46,     0,     0,     0,    76,
      77,     0,     0,     0,    71,     0,    78,   148,    79,    72,
       0,     0,     1,    73,    74,     0,     0,     0,     2,     3,
       4,     5,     6,     0,     0,     0,     8,     0,     0,     0,
       0,    11,    12,     0,    14,    65,    16,    17,     0,    66,
      67,     0,    21,    22,    23,    24,     0,    26,     0,     0,
       0,     0,     0,     0,    29,    30,    75,    32,    33,    34,
       0,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,     0,     0,     0,     0,     0,     0,    46,     0,     0,
       0,    76,    77,     0,    71,     0,     0,     0,    78,    72,
      79,     0,     1,    73,    74,     0,     0,     0,     2,     3,
       4,     5,     6,     0,     0,     0,     8,     0,     0,     0,
       0,    11,    12,     0,    14,    65,    16,    17,     0,    66,
      67,     0,    21,    22,    23,    24,     0,    26,     0,     0,
       0,     0,     0,     0,    29,    30,    75,    32,    33,    34,
       0,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,     0,     0,     0,     0,     0,     0,    46,     0,     0,
       0,    76,    77,     0,    71,     0,     0,     0,    90,    72,
      79,     0,     1,    73,    74,     0,     0,     0,     2,     3,
       4,     5,     6,     0,     0,     0,    99,     0,     0,     0,
       0,    11,    12,     0,   100,    65,    16,    17,     0,    66,
      67,     0,    21,    22,    23,    24,     0,    26,     0,     0,
       0,     0,     0,     0,    29,    30,    75,    32,    33,    34,
       0,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,     0,     0,     0,     0,     0,     0,    46,     0,     0,
       0,    76,    77,     0,    71,     0,     0,     0,    90,    72,
      79,     0,     1,    73,    74,     0,     0,     0,     2,     3,
       4,     5,     6,     0,     0,     0,     8,     0,     0,     0,
       0,    11,    12,     0,    14,    65,    16,    17,     0,    66,
      67,     0,    21,    22,    23,    24,     0,    26,     0,     0,
       0,     0,     0,     0,     0,     0,    75,    32,    33,    34,
       0,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,     0,     0,     0,     0,     0,     0,    46,     0,     0,
       0,    76,    77,     0,    71,     0,     0,     0,    78,    72,
      79,     0,     1,    73,    74,     0,     0,     0,     2,     3,
       4,     5,     6,     0,     0,     0,     8,     0,     0,     0,
       0,    11,    12,     0,    14,    65,    16,    17,     0,    66,
      67,     0,    21,    22,    23,    24,     0,    26,     0,     0,
       0,     0,     0,     0,     0,     0,    75,     0,    33,    34,
       0,     0,     0,    36,    37,    38,    39,    40,    41,    42,
      43,     0,     0,     0,     0,     0,     0,    46,     0,     0,
       0,     0,     0,     0,     0,     1,     0,     0,    78,     0,
      79,     2,     3,     4,     5,     6,     0,     0,     7,     8,
       9,     0,     0,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,     0,    21,    22,    23,    24,    25,
      26,    27,    28,     0,     0,     0,     0,    29,    30,    31,
      32,    33,    34,     0,     0,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,     0,     1,     0,     0,
      46,     0,     0,     2,     3,     4,     5,     6,   292,    47,
       7,     8,     9,     0,     0,    10,    11,    12,     0,    14,
      15,    16,    17,    18,    19,    20,     0,    21,    22,    23,
      24,     0,    26,    27,    28,     0,     0,     0,     0,    29,
      30,    31,    32,    33,    34,     0,     0,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,     0,     0,   293,
       1,     0,    46,     0,     0,     0,     2,     3,     4,     5,
       6,   255,     0,     7,     8,     9,     0,     0,    10,    11,
      12,     0,    14,    15,    16,    17,    18,    19,    20,     0,
      21,    22,    23,    24,     0,    26,    27,    28,     0,     0,
       0,     0,    29,    30,    31,    32,    33,    34,     0,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
       0,     0,     0,   300,     0,    46,     0,     0,     0,     1,
       0,     0,   304,     0,   255,     2,     3,     4,     5,     6,
       0,     0,     7,     8,     9,     0,     0,    10,    11,    12,
       0,    14,    15,    16,    17,    18,    19,    20,     0,    21,
      22,    23,    24,     0,    26,    27,    28,     0,     0,     0,
       0,    29,    30,    31,    32,    33,    34,     0,     0,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,     0,
       0,     1,     0,     0,    46,     0,     0,     2,     3,     4,
       5,     6,     0,   255,     7,     8,     9,     0,     0,    10,
      11,    12,     0,    14,    15,    16,    17,    18,    19,    20,
       0,    21,    22,    23,    24,     0,    26,    27,    28,     0,
       0,     0,     0,    29,    30,    31,    32,    33,    34,     0,
       0,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,     0,     0,   316,     1,     0,    46,     0,     0,     0,
       2,     3,     4,     5,     6,   255,     0,     7,     8,     9,
       0,     0,    10,    11,    12,     0,    14,    15,    16,    17,
      18,    19,    20,     0,    21,    22,    23,    24,     0,    26,
      27,    28,     0,     0,     0,     0,    29,    30,    31,    32,
      33,    34,     0,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,     0,     0,   322,     1,     0,    46,
       0,     0,     0,     2,     3,     4,     5,     6,   255,     0,
       7,     8,     9,     0,     0,    10,    11,    12,     0,    14,
      15,    16,    17,    18,    19,    20,     0,    21,    22,    23,
      24,     0,    26,    27,    28,     0,     0,     0,     0,    29,
      30,    31,    32,    33,    34,     0,     0,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,     0,     0,     1,
     329,     0,    46,     0,     0,     2,     3,     4,     5,     6,
       0,   255,     7,     8,     9,     0,     0,    10,    11,    12,
       0,    14,    15,    16,    17,    18,    19,    20,     0,    21,
      22,    23,    24,     0,    26,    27,    28,     0,     0,     0,
       0,    29,    30,    31,    32,    33,    34,     0,     0,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,     0,
       0,     1,   330,     0,    46,     0,     0,     2,     3,     4,
       5,     6,     0,   255,     7,     8,     9,     0,     0,    10,
      11,    12,     0,    14,    15,    16,    17,    18,    19,    20,
       0,    21,    22,    23,    24,     0,    26,    27,    28,     0,
       0,     0,     0,    29,    30,    31,    32,    33,    34,     0,
       0,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,     0,     0,     1,     0,     0,    46,     0,     0,     2,
       3,     4,     5,     6,     0,   189,     7,     8,     9,     0,
       0,    10,    11,    12,     0,    14,    15,    16,    17,    18,
      19,    20,     0,    21,    22,    23,    24,     0,    26,    27,
      28,     0,     0,     0,     0,    29,    30,    31,    32,    33,
      34,     0,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,     0,     0,     1,     0,     0,    46,     0,
       0,     2,     3,     4,     5,     6,     0,   255,     7,     8,
       9,     0,     0,    10,    11,    12,     0,    14,    15,    16,
      17,    18,    19,    20,     0,    21,    22,    23,    24,     0,
      26,    27,    28,     0,     0,     0,     0,    29,    30,    31,
      32,    33,    34,     0,     0,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,     0,     0,     1,     0,     0,
      46,     0,     0,     2,     3,     4,     5,     6,     0,   295,
       7,     8,     9,     0,     0,    10,    11,    12,     0,    14,
      15,    16,    17,    18,    19,    20,     0,    21,    22,    23,
      24,     0,    26,    27,    28,     0,     0,     0,     0,    29,
      30,    31,    32,    33,    34,     0,     0,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,     0,     1,   249,
     250,     0,    46,     0,     2,     3,     4,     5,     6,     0,
       0,   305,     8,     0,     0,     0,     0,    11,    12,     0,
      14,    65,    16,    17,     0,    66,    67,     0,    21,    22,
      23,    24,     0,    26,     0,     0,     0,     0,     0,     0,
      29,    30,    31,    32,    33,    34,     0,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,     0,     1,     0,
       0,     0,     0,    46,     2,     3,     4,     5,     6,     0,
       0,     0,     8,     0,     0,     0,     0,    11,    12,     0,
      14,    65,    16,    17,     0,    66,    67,     0,    21,    22,
      23,    24,     0,    26,     0,     0,     0,   298,     0,     0,
      29,    30,    31,    32,    33,    34,     0,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,   299,     0,     0,
       0,     0,     0,    46,     0,   129,   130,   131,   132,   133,
     134,   128,   135,     0,     0,   136,   137,   138,     0,     0,
     129,   130,   131,   132,   133,   134,     0,   135,     0,     0,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
       0,     0,     0,     0,     0,     0,     0,     0,   139,   140,
     141,   142,   143,   144,   145,   129,   130,   131,   132,   133,
     134,     0,   135,     0,     0,   136,   137,   138,   215,   216,
     217,   218,   219,   220,     0,   221,     0,     0,   222,   223,
     224,     0,     0,   139,   140,   141,   142,   143,   144,   145,
       0,     0,     0,     0,   179,     0,   225,   226,   227,   228,
     229,   230,   231,     0,     0,   232,   129,   130,   131,   132,
     133,   134,     0,   135,     0,     0,   136,   137,   138,   129,
     130,   131,   132,   133,   134,     0,   135,     0,     0,   136,
     137,   138,     0,     0,   139,   140,   141,   142,   143,   144,
     145,     0,   234,     0,     0,     0,     0,   139,   140,   141,
     142,   143,   144,   145,     0,   286,   129,   130,   131,   132,
     133,   134,     0,   135,     0,     0,   136,   137,   138,   129,
     130,   131,   132,   133,   134,     0,   135,     0,     0,   136,
     137,   138,     0,     0,   139,   140,   141,   142,   143,   144,
     145,     0,   318,     0,     0,     0,     0,   139,   140,   141,
     142,   143,   144,   145,     0,   324,   129,   130,   131,   132,
     133,   134,     0,   135,     0,     0,   136,   137,   138,   215,
     216,   217,   218,   219,   220,     0,   221,     0,     0,   222,
     223,   224,     0,     0,   139,   140,   141,   142,   143,   144,
     145,     0,     0,     0,     0,     0,     0,   225,   226,   227,
     228,   229,   230,   231
};

static const yytype_int16 yycheck[] =
{
       0,   158,    20,    28,     0,   208,    92,    40,    43,     9,
      40,    89,    92,    13,    58,    93,    27,   103,    53,    44,
      20,    58,    88,   103,   110,    25,    92,    88,    28,    90,
     110,   234,    58,   257,     0,    61,    87,   261,    82,    83,
      84,    85,   245,    54,    44,    45,    87,    84,    85,    49,
      88,    74,   209,    49,    87,    81,    82,    83,    84,    85,
      89,   285,    93,    88,    87,    45,    46,   291,    75,    76,
      77,    87,    88,   155,   156,   232,    93,    87,    87,    79,
      92,   106,    87,   169,   308,    87,    87,    35,    88,   169,
      87,   315,   295,    93,    87,   319,    87,    92,    88,    87,
      53,   325,   305,    87,   328,    90,   106,    24,    87,   312,
      87,    87,    87,   113,    35,   318,    87,    43,    87,    93,
      87,   324,    87,    87,    87,    87,    49,   288,   128,   184,
     169,    -1,   128,   151,    -1,   127,    -1,   155,   156,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,    -1,    -1,   155,   156,    -1,    -1,    -1,
       5,    -1,    -1,    -1,    -1,    10,    -1,    -1,    13,    14,
      15,    -1,    -1,    -1,    19,    20,    21,    22,    23,    -1,
      -1,    -1,    27,    -1,   184,    -1,    -1,    32,    33,   189,
      35,    36,    37,    38,    -1,    40,    41,    -1,    43,    44,
      45,    46,    -1,    48,    -1,    -1,    -1,    -1,   208,    -1,
      10,    -1,    57,    58,    59,    60,    -1,    -1,    18,    64,
      65,    66,    67,    68,    69,    70,    71,    27,    -1,    -1,
      -1,    -1,    -1,    78,   234,    -1,   236,    82,    83,    -1,
      51,    52,    53,    54,    89,   245,    91,    58,    -1,    -1,
      61,    62,    63,    -1,    54,    -1,    -1,   257,    -1,    -1,
     260,   261,   287,    -1,   260,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    -1,    -1,    -1,   285,    -1,   287,    -1,    -1,
      90,   291,   292,    -1,    94,   295,   292,    -1,    -1,    99,
     100,    -1,    -1,    -1,   304,   305,    -1,    -1,   308,    -1,
      -1,    -1,   312,    -1,    -1,   315,    -1,    -1,   318,   319,
      -1,   321,    -1,    -1,   324,   325,    -1,    -1,   328,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,    51,    52,    53,    54,    55,    56,   167,    58,    -1,
      -1,    61,    62,    63,    -1,    -1,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,    -1,    -1,    -1,   187,    -1,    79,
      80,    81,    82,    83,    84,    85,    -1,    -1,    88,    -1,
      90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,    -1,   233,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     5,    -1,   244,    -1,    -1,    10,    -1,    -1,
      13,    14,    15,    -1,    -1,    -1,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    32,
      33,    -1,    35,    36,    37,    38,    -1,    40,    41,    -1,
      43,    44,    45,    46,    -1,    48,    -1,    -1,    -1,   289,
      -1,    -1,    55,    56,    57,    58,    59,    60,    -1,   299,
      63,    64,    65,    66,    67,    68,    69,    70,    71,   309,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    82,
      83,    -1,    -1,    -1,     5,    -1,    89,    -1,    91,    10,
      93,    -1,    13,    14,    15,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    32,    33,    -1,    35,    36,    37,    38,    -1,    40,
      41,    -1,    43,    44,    45,    46,    -1,    48,    -1,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    85,    -1,     5,    88,    89,    -1,
      91,    10,    -1,    -1,    13,    14,    15,    -1,    -1,    -1,
      19,    20,    21,    22,    23,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    32,    33,    -1,    35,    36,    37,    38,
      -1,    40,    41,    -1,    43,    44,    45,    46,    -1,    48,
      -1,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    85,    -1,     5,    88,
      89,    -1,    91,    10,    -1,    -1,    13,    14,    15,    -1,
      -1,    -1,    19,    20,    21,    22,    23,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    32,    33,    -1,    35,    36,
      37,    38,    -1,    40,    41,    -1,    43,    44,    45,    46,
      -1,    48,    -1,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    85,    -1,
       5,    88,    89,    -1,    91,    10,    -1,    -1,    13,    14,
      15,    -1,    -1,    -1,    19,    20,    21,    22,    23,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    32,    33,    -1,
      35,    36,    37,    38,    -1,    40,    41,    -1,    43,    44,
      45,    46,    -1,    48,    -1,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      85,    -1,     5,    -1,    89,    -1,    91,    10,    -1,    -1,
      13,    14,    15,    -1,    -1,    -1,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    32,
      33,    -1,    35,    36,    37,    38,    -1,    40,    41,    -1,
      43,    44,    45,    46,    -1,    48,    -1,    -1,    -1,    -1,
      -1,    -1,    55,    56,    57,    58,    59,    60,    -1,    -1,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    82,
      83,    -1,    -1,    -1,     5,    -1,    89,    90,    91,    10,
      -1,    -1,    13,    14,    15,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    32,    33,    -1,    35,    36,    37,    38,    -1,    40,
      41,    -1,    43,    44,    45,    46,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      -1,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    82,    83,    -1,     5,    -1,    -1,    -1,    89,    10,
      91,    -1,    13,    14,    15,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    32,    33,    -1,    35,    36,    37,    38,    -1,    40,
      41,    -1,    43,    44,    45,    46,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      -1,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    82,    83,    -1,     5,    -1,    -1,    -1,    89,    10,
      91,    -1,    13,    14,    15,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    32,    33,    -1,    35,    36,    37,    38,    -1,    40,
      41,    -1,    43,    44,    45,    46,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      -1,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    82,    83,    -1,     5,    -1,    -1,    -1,    89,    10,
      91,    -1,    13,    14,    15,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    32,    33,    -1,    35,    36,    37,    38,    -1,    40,
      41,    -1,    43,    44,    45,    46,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    58,    59,    60,
      -1,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    82,    83,    -1,     5,    -1,    -1,    -1,    89,    10,
      91,    -1,    13,    14,    15,    -1,    -1,    -1,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    32,    33,    -1,    35,    36,    37,    38,    -1,    40,
      41,    -1,    43,    44,    45,    46,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    59,    60,
      -1,    -1,    -1,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    89,    -1,
      91,    19,    20,    21,    22,    23,    -1,    -1,    26,    27,
      28,    -1,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    43,    44,    45,    46,    47,
      48,    49,    50,    -1,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    -1,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    13,    -1,    -1,
      78,    -1,    -1,    19,    20,    21,    22,    23,    24,    87,
      26,    27,    28,    -1,    -1,    31,    32,    33,    -1,    35,
      36,    37,    38,    39,    40,    41,    -1,    43,    44,    45,
      46,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    -1,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    -1,    75,
      13,    -1,    78,    -1,    -1,    -1,    19,    20,    21,    22,
      23,    87,    -1,    26,    27,    28,    -1,    -1,    31,    32,
      33,    -1,    35,    36,    37,    38,    39,    40,    41,    -1,
      43,    44,    45,    46,    -1,    48,    49,    50,    -1,    -1,
      -1,    -1,    55,    56,    57,    58,    59,    60,    -1,    -1,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      -1,    -1,    -1,    76,    -1,    78,    -1,    -1,    -1,    13,
      -1,    -1,    16,    -1,    87,    19,    20,    21,    22,    23,
      -1,    -1,    26,    27,    28,    -1,    -1,    31,    32,    33,
      -1,    35,    36,    37,    38,    39,    40,    41,    -1,    43,
      44,    45,    46,    -1,    48,    49,    50,    -1,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    -1,
      -1,    13,    -1,    -1,    78,    -1,    -1,    19,    20,    21,
      22,    23,    -1,    87,    26,    27,    28,    -1,    -1,    31,
      32,    33,    -1,    35,    36,    37,    38,    39,    40,    41,
      -1,    43,    44,    45,    46,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    -1,
      -1,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    -1,    75,    13,    -1,    78,    -1,    -1,    -1,
      19,    20,    21,    22,    23,    87,    -1,    26,    27,    28,
      -1,    -1,    31,    32,    33,    -1,    35,    36,    37,    38,
      39,    40,    41,    -1,    43,    44,    45,    46,    -1,    48,
      49,    50,    -1,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    -1,    -1,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    -1,    -1,    75,    13,    -1,    78,
      -1,    -1,    -1,    19,    20,    21,    22,    23,    87,    -1,
      26,    27,    28,    -1,    -1,    31,    32,    33,    -1,    35,
      36,    37,    38,    39,    40,    41,    -1,    43,    44,    45,
      46,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    -1,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    -1,    13,
      76,    -1,    78,    -1,    -1,    19,    20,    21,    22,    23,
      -1,    87,    26,    27,    28,    -1,    -1,    31,    32,    33,
      -1,    35,    36,    37,    38,    39,    40,    41,    -1,    43,
      44,    45,    46,    -1,    48,    49,    50,    -1,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    -1,
      -1,    13,    76,    -1,    78,    -1,    -1,    19,    20,    21,
      22,    23,    -1,    87,    26,    27,    28,    -1,    -1,    31,
      32,    33,    -1,    35,    36,    37,    38,    39,    40,    41,
      -1,    43,    44,    45,    46,    -1,    48,    49,    50,    -1,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    -1,
      -1,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    -1,    13,    -1,    -1,    78,    -1,    -1,    19,
      20,    21,    22,    23,    -1,    87,    26,    27,    28,    -1,
      -1,    31,    32,    33,    -1,    35,    36,    37,    38,    39,
      40,    41,    -1,    43,    44,    45,    46,    -1,    48,    49,
      50,    -1,    -1,    -1,    -1,    55,    56,    57,    58,    59,
      60,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    -1,    -1,    13,    -1,    -1,    78,    -1,
      -1,    19,    20,    21,    22,    23,    -1,    87,    26,    27,
      28,    -1,    -1,    31,    32,    33,    -1,    35,    36,    37,
      38,    39,    40,    41,    -1,    43,    44,    45,    46,    -1,
      48,    49,    50,    -1,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    -1,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    -1,    -1,    13,    -1,    -1,
      78,    -1,    -1,    19,    20,    21,    22,    23,    -1,    87,
      26,    27,    28,    -1,    -1,    31,    32,    33,    -1,    35,
      36,    37,    38,    39,    40,    41,    -1,    43,    44,    45,
      46,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    -1,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    13,    14,
      15,    -1,    78,    -1,    19,    20,    21,    22,    23,    -1,
      -1,    87,    27,    -1,    -1,    -1,    -1,    32,    33,    -1,
      35,    36,    37,    38,    -1,    40,    41,    -1,    43,    44,
      45,    46,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    60,    -1,    -1,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    -1,    13,    -1,
      -1,    -1,    -1,    78,    19,    20,    21,    22,    23,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    32,    33,    -1,
      35,    36,    37,    38,    -1,    40,    41,    -1,    43,    44,
      45,    46,    -1,    48,    -1,    -1,    -1,    23,    -1,    -1,
      55,    56,    57,    58,    59,    60,    -1,    -1,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    43,    -1,    -1,
      -1,    -1,    -1,    78,    -1,    51,    52,    53,    54,    55,
      56,    42,    58,    -1,    -1,    61,    62,    63,    -1,    -1,
      51,    52,    53,    54,    55,    56,    -1,    58,    -1,    -1,
      61,    62,    63,    79,    80,    81,    82,    83,    84,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    51,    52,    53,    54,    55,
      56,    -1,    58,    -1,    -1,    61,    62,    63,    51,    52,
      53,    54,    55,    56,    -1,    58,    -1,    -1,    61,    62,
      63,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      -1,    -1,    -1,    -1,    90,    -1,    79,    80,    81,    82,
      83,    84,    85,    -1,    -1,    88,    51,    52,    53,    54,
      55,    56,    -1,    58,    -1,    -1,    61,    62,    63,    51,
      52,    53,    54,    55,    56,    -1,    58,    -1,    -1,    61,
      62,    63,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    -1,    87,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    -1,    87,    51,    52,    53,    54,
      55,    56,    -1,    58,    -1,    -1,    61,    62,    63,    51,
      52,    53,    54,    55,    56,    -1,    58,    -1,    -1,    61,
      62,    63,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    -1,    87,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    -1,    87,    51,    52,    53,    54,
      55,    56,    -1,    58,    -1,    -1,    61,    62,    63,    51,
      52,    53,    54,    55,    56,    -1,    58,    -1,    -1,    61,
      62,    63,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    13,    19,    20,    21,    22,    23,    26,    27,    28,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    43,    44,    45,    46,    47,    48,    49,    50,    55,
      56,    57,    58,    59,    60,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    78,    87,    95,    96,
      97,    98,    99,   104,   106,   109,   110,   111,   114,   116,
     117,   118,   119,    40,    87,    36,    40,    41,   106,   107,
     108,     5,    10,    14,    15,    57,    82,    83,    89,    91,
     106,   123,   124,   127,   130,   131,   132,   134,   106,    40,
      89,   112,   134,    45,    46,   106,   124,   126,   106,    27,
      35,   112,   113,   134,   107,   107,   106,     0,    97,   112,
     134,    87,    87,    88,   132,   132,   132,   134,    14,    15,
      93,   106,   128,   129,   134,   136,   137,    89,    42,    51,
      52,    53,    54,    55,    56,    58,    61,    62,    63,    79,
      80,    81,    82,    83,    84,    85,   107,    87,    90,   134,
      87,    33,    55,    56,    58,    59,    60,    63,    88,   123,
     130,   133,   135,   106,   134,    43,    53,   115,    87,   134,
     134,    87,    87,    37,    87,    87,   107,    87,   106,    90,
      93,    93,    92,    93,    88,    92,    92,    88,   136,    87,
     109,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,    87,    88,
     124,   125,   125,   125,   137,    51,    52,    53,    54,    55,
      56,    58,    61,    62,    63,    79,    80,    81,    82,    83,
      84,    85,    88,    53,    87,   134,    74,    87,   100,   101,
     102,   103,    37,   135,    35,    87,   134,   134,   134,    14,
      15,   106,   129,   134,    90,    87,   109,   120,   121,   122,
      24,   120,   137,    87,    87,    87,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   137,   134,   120,    87,   106,    87,    35,
     134,   120,    24,    75,   122,    87,   109,    90,    23,    43,
      76,   107,   103,   134,    16,    87,   109,    87,   120,    43,
     134,    87,    87,   105,   106,   120,    75,   134,    87,   120,
      87,    88,    75,    87,    87,   120,   106,    87,   120,    76,
      76,    87,    87
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    94,    95,    96,    96,    97,    97,    97,    97,    97,
      98,    99,   100,   101,   101,   102,   102,   103,   103,   104,
     104,   105,   105,   105,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     107,   107,   108,   108,   109,   109,   109,   109,   110,   110,
     110,   111,   111,   111,   111,   111,   111,   111,   112,   112,
     112,   112,   112,   112,   112,   113,   113,   113,   113,   113,
     114,   114,   114,   114,   115,   115,   116,   116,   116,   117,
     117,   118,   118,   118,   118,   119,   119,   119,   120,   120,
     121,   121,   122,   122,   123,   123,   123,   123,   123,   123,
     123,   123,   124,   125,   126,   127,   127,   127,   128,   128,
     129,   129,   129,   130,   131,   131,   132,   132,   132,   133,
     133,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   136,   136,   137,
     137
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
       0,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     2,     0,     1,
       3,     2,     4,     2,     5,     2,     2,     5,     4,     3,
       5,     5,     5,     5,     1,     1,     3,     3,     3,     4,
       7,     6,     9,     7,    10,     7,    11,    12,     0,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     2,     4,
       1,     1,     1,     1,     1,     3,     3,     3,     1,     3,
       3,     3,     3,     3,     2,     2,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     0,     1,     1,
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
#line 1737 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tTHEFUNCINOF: /* tTHEFUNCINOF  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1743 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tVARID: /* tVARID  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1749 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1755 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1761 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1767 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tPLAYACCEL: /* tPLAYACCEL  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1773 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1779 "engines/director/lingo/lingo-gr.cpp"
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
#line 2047 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 198 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 204 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2068 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 211 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2074 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 244 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2080 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* factory: tFACTORY ID '\n' methodlist  */
#line 248 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2086 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 250 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2092 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* methodlist: %empty  */
#line 252 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2098 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* nonemptymethodlist: methodlistline  */
#line 256 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2109 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlist '\n' methodlistline  */
#line 262 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 2119 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlistline: '\n'  */
#line 269 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2125 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 275 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2134 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* handler: tON ID idlist '\n' stmtlist  */
#line 279 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* endargdef: ID  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2147 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* endargdef: endargdef ',' ID  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2153 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* ID: tAFTER  */
#line 291 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2159 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* ID: tAND  */
#line 292 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 2165 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* ID: tBEFORE  */
#line 293 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2171 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* ID: tCAST  */
#line 294 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2177 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* ID: tCHAR  */
#line 295 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* ID: tDOWN  */
#line 296 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2189 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* ID: tFIELD  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2195 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* ID: tFRAME  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2201 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* ID: tIN  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2207 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* ID: tINTERSECTS  */
#line 307 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* ID: tINTO  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2219 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* ID: tITEM  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2225 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* ID: tLINE  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2231 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* ID: tMOD  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2237 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* ID: tMOVIE  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2243 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* ID: tNEXT  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2249 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* ID: tNOT  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2255 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* ID: tOF  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2261 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* ID: tOPEN  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 2267 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* ID: tOR  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2273 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* ID: tPREVIOUS  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2279 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* ID: tREPEAT  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2285 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* ID: tSCRIPT  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2291 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* ID: tSET  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 2297 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* ID: tSTARTS  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2303 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* ID: tTELL  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 2309 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* ID: tTO  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2315 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* ID: tASSERTERROR  */
#line 332 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2321 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* ID: tSPRITE  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2327 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* ID: tWHEN  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("when"); }
#line 2333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* ID: tWHILE  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 2339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* ID: tWINDOW  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* ID: tWITH  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 2351 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* ID: tWITHIN  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 2357 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* ID: tWORD  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 2363 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* idlist: %empty  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 2369 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* nonemptyidlist: ID  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 2378 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 2386 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* proc: ID cmdargs '\n'  */
#line 371 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist)); }
#line 2392 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* proc: tPUT cmdargs '\n'  */
#line 372 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist)); }
#line 2398 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* proc: tPLAY cmdargs '\n'  */
#line 373 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 2404 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* proc: tPLAY frameargs '\n'  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 2410 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* proc: tNEXT tREPEAT '\n'  */
#line 375 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 2416 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* proc: tEXIT tREPEAT '\n'  */
#line 376 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 2422 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* proc: tEXIT '\n'  */
#line 377 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 2428 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* cmdargs: %empty  */
#line 380 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 2436 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* cmdargs: expr  */
#line 383 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2446 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* cmdargs: expr ',' nonemptyexprlist  */
#line 388 "engines/director/lingo/lingo-gr.y"
                                                {
		// This matches `cmd args, ...)
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 2455 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* cmdargs: expr expr_nounarymath  */
#line 392 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2466 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist  */
#line 398 "engines/director/lingo/lingo-gr.y"
                                                           {
		// This matches `cmd arg arg, ...`
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 2476 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* cmdargs: '(' ')'  */
#line 403 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 2484 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* cmdargs: '(' expr ',' nonemptyexprlist ')'  */
#line 406 "engines/director/lingo/lingo-gr.y"
                                                  {
		// This matches `cmd(args, ...)`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2493 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* frameargs: tFRAME expr  */
#line 416 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2503 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* frameargs: tMOVIE expr  */
#line 421 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2514 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* frameargs: expr tOF tMOVIE expr  */
#line 433 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 2536 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* frameargs: tFRAME expr expr_nounarymath  */
#line 439 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2547 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 447 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2553 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 448 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2559 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 449 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2565 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* asgn: tSET varorthe to expr '\n'  */
#line 450 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2571 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* definevars: tGLOBAL idlist '\n'  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 2577 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* definevars: tPROPERTY idlist '\n'  */
#line 456 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 2583 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* definevars: tINSTANCE idlist '\n'  */
#line 457 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 2589 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* ifstmt: tIF expr tTHEN stmt  */
#line 460 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 2598 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* ifstmt: tIF expr tTHEN '\n' stmtlist tENDIF '\n'  */
#line 464 "engines/director/lingo/lingo-gr.y"
                                                   {
		(yyval.node) = new IfStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist)); }
#line 2605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 468 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 2616 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist tENDIF '\n'  */
#line 474 "engines/director/lingo/lingo-gr.y"
                                                                                {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-5].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), stmtlist1, (yyvsp[-2].nodelist)); }
#line 2625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE stmt  */
#line 478 "engines/director/lingo/lingo-gr.y"
                                                                    {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 2634 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE '\n' stmtlist tENDIF '\n'  */
#line 482 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new IfElseStmtNode((yyvsp[-8].node), (yyvsp[-5].nodelist), (yyvsp[-2].nodelist)); }
#line 2641 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 486 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 2648 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 488 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 2655 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 490 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 2662 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* stmtlist: %empty  */
#line 494 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2668 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* nonemptystmtlist: stmtlistline  */
#line 499 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2679 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* nonemptystmtlist: stmtlist stmtlistline  */
#line 505 "engines/director/lingo/lingo-gr.y"
                                                {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2689 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* stmtlistline: '\n'  */
#line 512 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 2695 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* simpleexpr_noparens_nounarymath: tINT  */
#line 519 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 2701 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* simpleexpr_noparens_nounarymath: tFLOAT  */
#line 520 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 2707 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* simpleexpr_noparens_nounarymath: tSYMBOL  */
#line 521 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 2713 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* simpleexpr_noparens_nounarymath: tSTRING  */
#line 522 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 2719 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* simpleexpr_noparens_nounarymath: tNOT simpleexpr  */
#line 523 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 2725 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* simpleexpr_noparens_nounarymath: ID '(' exprlist ')'  */
#line 524 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new FuncNode((yyvsp[-3].s), (yyvsp[-1].nodelist)); }
#line 2731 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* var: ID  */
#line 529 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 2737 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* list: '[' exprlist ']'  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 2743 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* list: '[' ':' ']'  */
#line 540 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 2749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* list: '[' proplist ']'  */
#line 541 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 2755 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* proplist: proppair  */
#line 544 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 2764 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* proplist: proplist ',' proppair  */
#line 548 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 2772 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* proppair: tSYMBOL ':' expr  */
#line 553 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 2778 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* proppair: ID ':' expr  */
#line 554 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 2784 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* proppair: tSTRING ':' expr  */
#line 555 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 2790 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* parens: '(' expr ')'  */
#line 558 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[-1].node); }
#line 2796 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* unarymath: '+' simpleexpr  */
#line 560 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 2802 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* unarymath: '-' simpleexpr  */
#line 561 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 2808 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* expr: expr '+' expr  */
#line 579 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2814 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* expr: expr '-' expr  */
#line 580 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2820 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* expr: expr '*' expr  */
#line 581 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2826 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* expr: expr '/' expr  */
#line 582 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2832 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* expr: expr tMOD expr  */
#line 583 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2838 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* expr: expr '>' expr  */
#line 584 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2844 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* expr: expr '<' expr  */
#line 585 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2850 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* expr: expr tEQ expr  */
#line 586 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2856 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* expr: expr tNEQ expr  */
#line 587 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2862 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* expr: expr tGE expr  */
#line 588 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* expr: expr tLE expr  */
#line 589 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* expr: expr tAND expr  */
#line 590 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2880 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* expr: expr tOR expr  */
#line 591 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2886 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* expr: expr '&' expr  */
#line 592 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2892 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* expr: expr tCONCAT expr  */
#line 593 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2898 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* expr: expr tCONTAINS expr  */
#line 594 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* expr: expr tSTARTS expr  */
#line 595 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2910 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 603 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 604 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2922 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 605 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 606 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2934 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 607 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2940 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 608 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 609 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2952 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 610 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2958 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 611 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2964 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 612 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2970 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 613 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2976 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 614 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2982 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 615 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2988 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 616 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2994 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 617 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3000 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 618 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3006 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 619 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3012 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* exprlist: %empty  */
#line 622 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 3018 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* nonemptyexprlist: expr  */
#line 626 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 3027 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 630 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3035 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3039 "engines/director/lingo/lingo-gr.cpp"

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

#line 635 "engines/director/lingo/lingo-gr.y"


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
