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
  YYSYMBOL_tGLOBAL = 27,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 28,                       /* tGO  */
  YYSYMBOL_tGOLOOP = 29,                   /* tGOLOOP  */
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
  YYSYMBOL_asgn = 111,                     /* asgn  */
  YYSYMBOL_to = 112,                       /* to  */
  YYSYMBOL_definevars = 113,               /* definevars  */
  YYSYMBOL_ifstmt = 114,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 115,               /* ifelsestmt  */
  YYSYMBOL_loop = 116,                     /* loop  */
  YYSYMBOL_stmtlist = 117,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 118,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 119,             /* stmtlistline  */
  YYSYMBOL_simpleexprnoparens = 120,       /* simpleexprnoparens  */
  YYSYMBOL_var = 121,                      /* var  */
  YYSYMBOL_varorchunk = 122,               /* varorchunk  */
  YYSYMBOL_varorthe = 123,                 /* varorthe  */
  YYSYMBOL_simpleexpr = 124,               /* simpleexpr  */
  YYSYMBOL_list = 125,                     /* list  */
  YYSYMBOL_proplist = 126,                 /* proplist  */
  YYSYMBOL_proppair = 127,                 /* proppair  */
  YYSYMBOL_expr = 128,                     /* expr  */
  YYSYMBOL_exprlist = 129,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 130          /* nonemptyexprlist  */
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
#define YYFINAL  101
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1688

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  93
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  38
/* YYNRULES -- Number of rules.  */
#define YYNRULES  147
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  271

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
       0,   194,   194,   196,   202,   209,   210,   211,   212,   213,
     242,   246,   248,   250,   251,   254,   260,   267,   268,   273,
     277,   281,   282,   283,   288,   289,   290,   291,   292,   293,
     294,   297,   299,   302,   304,   305,   306,   307,   310,   311,
     312,   313,   314,   316,   317,   318,   319,   320,   323,   324,
     325,   326,   327,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   340,   341,   344,   348,   358,   359,   360,   361,
     364,   365,   366,   369,   370,   371,   372,   373,   374,   377,
     378,   379,   380,   383,   383,   385,   386,   387,   390,   394,
     398,   404,   408,   412,   416,   418,   420,   424,   425,   429,
     435,   442,   443,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   460,   462,   466,   470,   471,   474,   475,
     476,   479,   483,   488,   489,   490,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
     507,   508,   509,   510,   513,   514,   517,   521
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
  "tEXIT", "tGLOBAL", "tGO", "tGOLOOP", "tIF", "tIN", "tINTO", "tMACRO",
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
  "proc", "asgn", "to", "definevars", "ifstmt", "ifelsestmt", "loop",
  "stmtlist", "nonemptystmtlist", "stmtlistline", "simpleexprnoparens",
  "var", "varorchunk", "varorthe", "simpleexpr", "list", "proplist",
  "proppair", "expr", "exprlist", "nonemptyexprlist", YY_NULLPTR
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

#define YYPACT_NINF (-183)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     618,  -183,  -183,  -183,  -183,  -183,  -183,  -183,   -21,  1417,
     388,  -183,  -183,  1417,  -183,   -15,  -183,  -183,   467,     3,
    1417,  -183,  -183,  -183,  -183,  1417,  -183,  -183,  1417,  -183,
    -183,  -183,  -183,  -183,  -183,  -183,  -183,  -183,  -183,  -183,
    -183,  -183,  -183,  -183,  1417,  1417,  -183,  -183,     4,   618,
    -183,  -183,  -183,  -183,   546,  -183,  -183,  -183,  -183,  -183,
    -183,  -183,  -183,   -74,  -183,  -183,  -183,  -183,  -183,   -59,
     -58,  -183,  -183,  -183,  -183,   388,   388,   388,   388,   307,
     -53,  -183,  -183,  -183,  -183,    96,  1417,   -48,   388,  1438,
     -43,   -32,  1417,   388,  -183,  -183,   -26,   -34,   -22,   -20,
    1417,  -183,  -183,   388,  1591,   -19,  -183,  -183,  1417,  -183,
    -183,  -183,  1451,   -23,   -14,   -10,   -77,   -68,  -183,    -9,
     388,  1116,   388,   388,   388,   388,   388,   388,   388,   388,
     388,   388,   388,   388,   388,   388,   388,   388,   388,   -16,
    -183,  1451,   -24,  1417,  1417,  1417,  -183,   388,    20,  1491,
    -183,  -183,   388,   -65,  -183,  -183,    -2,   -12,  -183,  -183,
    -183,   388,   388,  -183,   388,  1358,  -183,  -183,    -4,  1177,
      64,    58,    58,    58,    58,  1604,  1604,  -183,   -50,    58,
      58,    58,    58,   -50,   -47,   -47,  -183,  -183,  1177,     5,
    -183,     9,    11,    12,  1591,   388,  1177,  1504,  1417,  -183,
    -183,    13,  -183,  -183,  1177,    15,  1591,  1591,  1591,   -23,
     -14,    -3,  -183,  -183,  -183,  -183,   679,  -183,  -183,  1238,
    1177,  -183,  -183,  -183,  -183,    52,   741,  -183,  1417,   -65,
     809,  -183,  1299,    24,  -183,  1177,  -183,    48,   388,    25,
      30,  -183,  1417,  1177,  -183,  -183,   870,   388,  1541,  -183,
    1177,   -29,  -183,   932,    31,  1554,  1177,  1177,  -183,  1417,
      33,  -183,  1177,   994,  -183,  -183,  1055,    34,    36,  -183,
    -183
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    24,    46,    28,    32,    49,    58,    30,    31,    62,
       0,    33,    35,     0,    39,    40,    42,    47,   144,    48,
      50,    53,    56,    59,    57,     0,    43,    45,    62,    26,
      44,    41,    38,    25,    27,    51,    29,    36,    37,    61,
      55,    34,    60,    52,    62,     0,    54,     5,     0,     2,
       3,     6,     7,     8,   144,     9,    66,    70,    71,    72,
      67,    68,    69,     0,    31,    40,    48,    50,    64,     0,
      63,   103,   104,   106,   105,    41,     0,     0,     0,   144,
     113,   116,   111,   126,   112,     0,    62,     0,   144,   146,
       0,   145,     0,     0,   113,   115,     0,     0,     0,     0,
      62,     1,     4,   144,   146,     0,    78,    85,     0,   109,
     107,   108,     0,   106,   105,     0,   113,     0,   121,     0,
     144,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      77,   146,     0,     0,     0,     0,    76,     0,     0,     0,
      83,    84,     0,    13,    87,    86,     0,     0,    74,    65,
     117,     0,     0,   119,     0,     0,   120,   118,     0,    97,
      88,   136,   137,   134,   135,   138,   139,   131,   141,   142,
     143,   133,   132,   140,   127,   128,   129,   130,    97,     0,
     114,     0,     0,     0,   147,     0,    97,     0,     0,    17,
      18,    11,    14,    15,    97,     0,   125,   123,   124,     0,
       0,     0,   122,   110,   101,   102,     0,    98,    99,     0,
      10,    75,    79,    80,    81,     0,     0,    82,    62,     0,
      20,    73,     0,     0,   100,    97,    90,     0,     0,     0,
       0,    16,    21,    97,    92,    89,     0,     0,     0,    94,
      97,     0,    22,     0,     0,     0,    97,    12,    19,     0,
       0,    91,    97,     0,    23,    93,     0,     0,     0,    95,
      96
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -183,  -183,  -183,    75,  -183,  -183,  -183,  -183,  -183,  -104,
    -183,  -183,     0,   -27,  -183,     2,  -183,  -183,  -183,  -183,
    -183,  -183,  -183,  -183,  -182,  -183,  -170,  -183,   -17,   -82,
    -183,   -35,  -183,  -183,   -36,   145,   -49,  -183
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    48,    49,    50,    51,    52,   200,   201,   202,   203,
      53,   251,    80,    69,    70,   215,    56,    57,    58,   152,
      59,    60,    61,    62,   216,   217,   218,    81,    82,   191,
      96,    83,    84,   117,   118,   104,    90,    91
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      54,    98,    55,    95,   101,   105,   220,   128,   198,    68,
     128,   120,   106,    86,   226,   164,   150,    99,    63,   165,
      94,   199,   230,   166,    87,    97,   151,   107,    68,   108,
     119,   135,   136,   137,   138,   120,   137,   138,   140,   142,
     109,   110,   111,   146,    68,   100,   234,    92,    93,    54,
     234,    55,   153,   246,   157,   147,   234,   258,   259,   139,
     234,   253,   192,   193,   154,   189,   155,   158,   257,   161,
     188,   168,   195,   156,   263,   237,   234,   205,   162,   116,
     266,   163,   167,   234,   204,   213,    68,   234,   219,   164,
     247,   221,   148,   234,   238,   222,   234,   223,   224,   229,
      68,   231,   122,   123,   124,   125,   126,   127,   159,   128,
     245,   249,   129,   130,   131,   128,   250,   261,   129,   265,
     269,    54,   270,   170,   102,   241,   190,   190,   190,   212,
     132,   133,   134,   135,   136,   137,   138,   121,   134,   135,
     136,   137,   138,    94,    94,    94,   122,   123,   124,   125,
     126,   127,     0,   128,     0,    85,   129,   130,   131,     0,
       0,     0,     0,    89,     0,   211,     0,     0,     0,    54,
       0,     0,     0,     0,   132,   133,   134,   135,   136,   137,
     138,     0,     0,     0,     0,     0,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,    54,     0,   228,     0,
       0,   240,     0,     0,    54,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    54,     0,     0,    54,
      54,   236,     0,   112,     0,     0,    54,     0,    68,     0,
      54,     0,    54,   141,   244,    54,     0,     0,   149,     0,
       0,     0,   252,    54,     0,     0,    54,     0,   141,     0,
      54,     0,     0,    54,     0,     0,    54,    54,     0,   264,
       0,     0,    54,    54,     0,     0,    54,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,     0,     0,     0,     0,     0,     0,
       0,     0,   194,     0,     0,     0,     0,   197,     0,     0,
       0,     0,     0,     0,     0,     0,   206,   207,     0,   208,
       0,     0,    71,     0,     0,     0,     0,    72,     0,     0,
       1,   113,   114,     0,     2,     0,     3,     4,     5,     6,
       7,     0,     0,    64,     0,     0,     0,     0,    11,    12,
     225,    14,    65,    16,    17,     0,    66,    67,     0,    21,
      22,    23,    24,     0,    26,    27,     0,     0,     0,     0,
       0,    29,    30,    75,    32,    33,    34,     0,     0,    35,
      36,    37,    38,    39,    40,    41,    42,    43,     0,     0,
       0,     0,     0,   248,    46,     0,     0,     0,    76,    77,
       0,     0,   255,    71,     0,    78,     0,    79,    72,   115,
       0,     1,    73,    74,     0,     2,     0,     3,     4,     5,
       6,     7,     0,     0,    64,     0,     0,     0,     0,    11,
      12,     0,    14,    65,    16,    17,     0,    66,    67,     0,
      21,    22,    23,    24,     0,    26,    27,     0,     0,     0,
       0,     0,    29,    30,    75,    32,    33,    34,     0,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,     0,
       0,     0,     0,     0,     0,    46,     0,     0,     0,    76,
      77,     0,    71,     0,     0,     0,    78,    72,    79,     0,
       1,    73,    74,     0,     2,     0,     3,     4,     5,     6,
       7,     0,     0,    64,     0,     0,     0,     0,    11,    12,
       0,    14,    65,    16,    17,     0,    66,    67,     0,    21,
      22,    23,    24,     0,    26,    27,     0,     0,     0,     0,
       0,    29,    30,    75,    32,    33,    34,     0,     0,    35,
      36,    37,    38,    39,    40,    41,    42,    43,     0,     0,
       0,     0,     0,     0,    46,     0,     0,     0,    76,    77,
       0,    71,     0,     0,     0,    88,    72,    79,     0,     1,
      73,    74,     0,     2,     0,     3,     4,     5,     6,     7,
       0,     0,    64,     0,     0,     0,     0,    11,    12,     0,
      14,    65,    16,    17,     0,    66,    67,     0,    21,    22,
      23,    24,     0,    26,    27,     0,     0,     0,     0,     0,
      29,    30,    75,    32,    33,    34,     0,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,     0,     0,     0,
       0,     0,     0,    46,     0,     0,     0,    76,    77,     0,
       0,     1,     0,     0,   103,     2,    79,     3,     4,     5,
       6,     7,     0,     0,     8,     9,     0,     0,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,     0,
      21,    22,    23,    24,    25,    26,    27,    28,     0,     0,
       0,     0,    29,    30,    31,    32,    33,    34,     0,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,     0,     1,     0,     0,    46,     2,     0,     3,     4,
       5,     6,     7,   232,    47,     8,     9,     0,     0,    10,
      11,    12,     0,    14,    15,    16,    17,    18,    19,    20,
       0,    21,    22,    23,    24,     0,    26,    27,    28,     0,
       0,     0,     0,    29,    30,    31,    32,    33,    34,     0,
       0,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,     0,     0,   233,     1,     0,    46,     0,     2,     0,
       3,     4,     5,     6,     7,   214,     0,     8,     9,     0,
       0,    10,    11,    12,     0,    14,    15,    16,    17,    18,
      19,    20,     0,    21,    22,    23,    24,     0,    26,    27,
      28,     0,     0,     0,     0,    29,    30,    31,    32,    33,
      34,     0,     0,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,     0,     0,     0,   239,     0,    46,     0,
       0,     0,     1,     0,     0,   242,     2,   214,     3,     4,
       5,     6,     7,     0,     0,     8,     9,     0,     0,    10,
      11,    12,     0,    14,    15,    16,    17,    18,    19,    20,
       0,    21,    22,    23,    24,     0,    26,    27,    28,     0,
       0,     0,     0,    29,    30,    31,    32,    33,    34,     0,
       0,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,     0,     0,     1,     0,     0,    46,     2,     0,     3,
       4,     5,     6,     7,     0,   214,     8,     9,     0,     0,
      10,    11,    12,     0,    14,    15,    16,    17,    18,    19,
      20,     0,    21,    22,    23,    24,     0,    26,    27,    28,
       0,     0,     0,     0,    29,    30,    31,    32,    33,    34,
       0,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,     0,     0,   254,     1,     0,    46,     0,     2,
       0,     3,     4,     5,     6,     7,   214,     0,     8,     9,
       0,     0,    10,    11,    12,     0,    14,    15,    16,    17,
      18,    19,    20,     0,    21,    22,    23,    24,     0,    26,
      27,    28,     0,     0,     0,     0,    29,    30,    31,    32,
      33,    34,     0,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,     0,     0,   260,     1,     0,    46,
       0,     2,     0,     3,     4,     5,     6,     7,   214,     0,
       8,     9,     0,     0,    10,    11,    12,     0,    14,    15,
      16,    17,    18,    19,    20,     0,    21,    22,    23,    24,
       0,    26,    27,    28,     0,     0,     0,     0,    29,    30,
      31,    32,    33,    34,     0,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,     0,     0,     1,   267,
       0,    46,     2,     0,     3,     4,     5,     6,     7,     0,
     214,     8,     9,     0,     0,    10,    11,    12,     0,    14,
      15,    16,    17,    18,    19,    20,     0,    21,    22,    23,
      24,     0,    26,    27,    28,     0,     0,     0,     0,    29,
      30,    31,    32,    33,    34,     0,     0,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,     0,     0,     1,
     268,     0,    46,     2,     0,     3,     4,     5,     6,     7,
       0,   214,     8,     9,     0,     0,    10,    11,    12,     0,
      14,    15,    16,    17,    18,    19,    20,     0,    21,    22,
      23,    24,     0,    26,    27,    28,     0,     0,     0,     0,
      29,    30,    31,    32,    33,    34,     0,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,     0,     0,
       1,     0,     0,    46,     2,     0,     3,     4,     5,     6,
       7,     0,   169,     8,     9,     0,     0,    10,    11,    12,
       0,    14,    15,    16,    17,    18,    19,    20,     0,    21,
      22,    23,    24,     0,    26,    27,    28,     0,     0,     0,
       0,    29,    30,    31,    32,    33,    34,     0,     0,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,     0,
       0,     1,     0,     0,    46,     2,     0,     3,     4,     5,
       6,     7,     0,   214,     8,     9,     0,     0,    10,    11,
      12,     0,    14,    15,    16,    17,    18,    19,    20,     0,
      21,    22,    23,    24,     0,    26,    27,    28,     0,     0,
       0,     0,    29,    30,    31,    32,    33,    34,     0,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
       0,     0,     1,     0,     0,    46,     2,     0,     3,     4,
       5,     6,     7,     0,   235,     8,     9,     0,     0,    10,
      11,    12,     0,    14,    15,    16,    17,    18,    19,    20,
       0,    21,    22,    23,    24,     0,    26,    27,    28,     0,
       0,     0,     0,    29,    30,    31,    32,    33,    34,     0,
       0,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,     1,   209,   210,     0,     2,    46,     3,     4,     5,
       6,     7,     0,     0,    64,   243,     0,     0,     0,    11,
      12,     0,    14,    65,    16,    17,     0,    66,    67,     0,
      21,    22,    23,    24,     0,    26,    27,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,    34,     0,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,     0,
       1,     0,     0,     0,     2,    46,     3,     4,     5,     6,
       7,     0,     0,    64,     0,     0,     0,     0,    11,    12,
       0,    14,    65,    16,    17,     0,    66,    67,     0,    21,
      22,    23,    24,     0,    26,    27,     0,     0,     0,     0,
     143,    29,    30,    31,    32,    33,    34,     0,     0,    35,
      36,    37,    38,    39,    40,    41,    42,    43,   122,   123,
     124,   125,   126,   127,    46,   128,   144,   145,   129,   130,
     131,   122,   123,   124,   125,   126,   127,     0,   128,     0,
       0,   129,   130,   131,     0,     0,   132,   133,   134,   135,
     136,   137,   138,     0,     0,     0,     0,     0,     0,   132,
     133,   134,   135,   136,   137,   138,     0,     0,     0,     0,
     160,   122,   123,   124,   125,   126,   127,     0,   128,     0,
       0,   129,   130,   131,   122,   123,   124,   125,   126,   127,
       0,   128,     0,     0,   129,   130,   131,     0,     0,   132,
     133,   134,   135,   136,   137,   138,     0,   196,     0,     0,
       0,     0,   132,   133,   134,   135,   136,   137,   138,     0,
     227,   122,   123,   124,   125,   126,   127,     0,   128,     0,
       0,   129,   130,   131,   122,   123,   124,   125,   126,   127,
       0,   128,     0,     0,   129,   130,   131,     0,     0,   132,
     133,   134,   135,   136,   137,   138,     0,   256,     0,     0,
       0,     0,   132,   133,   134,   135,   136,   137,   138,     0,
     262,   122,   123,   124,   125,   126,   127,     0,   128,     0,
       0,   129,   130,   131,   122,   123,   124,   125,     0,     0,
       0,   128,     0,     0,   129,   130,   131,     0,     0,   132,
     133,   134,   135,   136,   137,   138,     0,     0,     0,     0,
       0,     0,   132,   133,   134,   135,   136,   137,   138
};

static const yytype_int16 yycheck[] =
{
       0,    28,     0,    20,     0,    54,   188,    57,    73,     9,
      57,    88,    86,    13,   196,    92,    42,    44,    39,    87,
      20,    86,   204,    91,    39,    25,    52,    86,    28,    87,
      79,    81,    82,    83,    84,    88,    83,    84,    86,    88,
      75,    76,    77,    86,    44,    45,   216,    44,    45,    49,
     220,    49,    86,   235,   103,    87,   226,    86,    87,    86,
     230,   243,   144,   145,    86,    89,    86,    86,   250,    92,
      86,   120,    52,   100,   256,    23,   246,    89,    92,    79,
     262,    91,    91,   253,    86,    89,    86,   257,    24,    92,
      42,    86,    92,   263,    42,    86,   266,    86,    86,    86,
     100,    86,    50,    51,    52,    53,    54,    55,   108,    57,
      86,    86,    60,    61,    62,    57,    86,    86,    60,    86,
      86,   121,    86,   121,    49,   229,   143,   144,   145,   165,
      78,    79,    80,    81,    82,    83,    84,    41,    80,    81,
      82,    83,    84,   143,   144,   145,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    10,    60,    61,    62,    -1,
      -1,    -1,    -1,    18,    -1,   165,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,   198,    -1,
      -1,   228,    -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   216,    -1,    -1,   219,
     220,   219,    -1,    78,    -1,    -1,   226,    -1,   228,    -1,
     230,    -1,   232,    88,   232,   235,    -1,    -1,    93,    -1,
      -1,    -1,   242,   243,    -1,    -1,   246,    -1,   103,    -1,
     250,    -1,    -1,   253,    -1,    -1,   256,   257,    -1,   259,
      -1,    -1,   262,   263,    -1,    -1,   266,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,   162,    -1,   164,
      -1,    -1,     5,    -1,    -1,    -1,    -1,    10,    -1,    -1,
      13,    14,    15,    -1,    17,    -1,    19,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    32,
     195,    34,    35,    36,    37,    -1,    39,    40,    -1,    42,
      43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,    -1,
      -1,    54,    55,    56,    57,    58,    59,    -1,    -1,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    -1,    -1,   238,    77,    -1,    -1,    -1,    81,    82,
      -1,    -1,   247,     5,    -1,    88,    -1,    90,    10,    92,
      -1,    13,    14,    15,    -1,    17,    -1,    19,    20,    21,
      22,    23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,
      32,    -1,    34,    35,    36,    37,    -1,    39,    40,    -1,
      42,    43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,
      -1,    -1,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    81,
      82,    -1,     5,    -1,    -1,    -1,    88,    10,    90,    -1,
      13,    14,    15,    -1,    17,    -1,    19,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    32,
      -1,    34,    35,    36,    37,    -1,    39,    40,    -1,    42,
      43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,    -1,
      -1,    54,    55,    56,    57,    58,    59,    -1,    -1,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    81,    82,
      -1,     5,    -1,    -1,    -1,    88,    10,    90,    -1,    13,
      14,    15,    -1,    17,    -1,    19,    20,    21,    22,    23,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    -1,    42,    43,
      44,    45,    -1,    47,    48,    -1,    -1,    -1,    -1,    -1,
      54,    55,    56,    57,    58,    59,    -1,    -1,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    81,    82,    -1,
      -1,    13,    -1,    -1,    88,    17,    90,    19,    20,    21,
      22,    23,    -1,    -1,    26,    27,    -1,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    -1,
      -1,    -1,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    13,    -1,    -1,    77,    17,    -1,    19,    20,
      21,    22,    23,    24,    86,    26,    27,    -1,    -1,    30,
      31,    32,    -1,    34,    35,    36,    37,    38,    39,    40,
      -1,    42,    43,    44,    45,    -1,    47,    48,    49,    -1,
      -1,    -1,    -1,    54,    55,    56,    57,    58,    59,    -1,
      -1,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    -1,    74,    13,    -1,    77,    -1,    17,    -1,
      19,    20,    21,    22,    23,    86,    -1,    26,    27,    -1,
      -1,    30,    31,    32,    -1,    34,    35,    36,    37,    38,
      39,    40,    -1,    42,    43,    44,    45,    -1,    47,    48,
      49,    -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,
      59,    -1,    -1,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    -1,    -1,    -1,    75,    -1,    77,    -1,
      -1,    -1,    13,    -1,    -1,    16,    17,    86,    19,    20,
      21,    22,    23,    -1,    -1,    26,    27,    -1,    -1,    30,
      31,    32,    -1,    34,    35,    36,    37,    38,    39,    40,
      -1,    42,    43,    44,    45,    -1,    47,    48,    49,    -1,
      -1,    -1,    -1,    54,    55,    56,    57,    58,    59,    -1,
      -1,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    -1,    13,    -1,    -1,    77,    17,    -1,    19,
      20,    21,    22,    23,    -1,    86,    26,    27,    -1,    -1,
      30,    31,    32,    -1,    34,    35,    36,    37,    38,    39,
      40,    -1,    42,    43,    44,    45,    -1,    47,    48,    49,
      -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    -1,    74,    13,    -1,    77,    -1,    17,
      -1,    19,    20,    21,    22,    23,    86,    -1,    26,    27,
      -1,    -1,    30,    31,    32,    -1,    34,    35,    36,    37,
      38,    39,    40,    -1,    42,    43,    44,    45,    -1,    47,
      48,    49,    -1,    -1,    -1,    -1,    54,    55,    56,    57,
      58,    59,    -1,    -1,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    -1,    -1,    74,    13,    -1,    77,
      -1,    17,    -1,    19,    20,    21,    22,    23,    86,    -1,
      26,    27,    -1,    -1,    30,    31,    32,    -1,    34,    35,
      36,    37,    38,    39,    40,    -1,    42,    43,    44,    45,
      -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,    55,
      56,    57,    58,    59,    -1,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    -1,    -1,    13,    75,
      -1,    77,    17,    -1,    19,    20,    21,    22,    23,    -1,
      86,    26,    27,    -1,    -1,    30,    31,    32,    -1,    34,
      35,    36,    37,    38,    39,    40,    -1,    42,    43,    44,
      45,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,
      55,    56,    57,    58,    59,    -1,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    -1,    -1,    13,
      75,    -1,    77,    17,    -1,    19,    20,    21,    22,    23,
      -1,    86,    26,    27,    -1,    -1,    30,    31,    32,    -1,
      34,    35,    36,    37,    38,    39,    40,    -1,    42,    43,
      44,    45,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,
      54,    55,    56,    57,    58,    59,    -1,    -1,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    -1,    -1,
      13,    -1,    -1,    77,    17,    -1,    19,    20,    21,    22,
      23,    -1,    86,    26,    27,    -1,    -1,    30,    31,    32,
      -1,    34,    35,    36,    37,    38,    39,    40,    -1,    42,
      43,    44,    45,    -1,    47,    48,    49,    -1,    -1,    -1,
      -1,    54,    55,    56,    57,    58,    59,    -1,    -1,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    -1,
      -1,    13,    -1,    -1,    77,    17,    -1,    19,    20,    21,
      22,    23,    -1,    86,    26,    27,    -1,    -1,    30,    31,
      32,    -1,    34,    35,    36,    37,    38,    39,    40,    -1,
      42,    43,    44,    45,    -1,    47,    48,    49,    -1,    -1,
      -1,    -1,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      -1,    -1,    13,    -1,    -1,    77,    17,    -1,    19,    20,
      21,    22,    23,    -1,    86,    26,    27,    -1,    -1,    30,
      31,    32,    -1,    34,    35,    36,    37,    38,    39,    40,
      -1,    42,    43,    44,    45,    -1,    47,    48,    49,    -1,
      -1,    -1,    -1,    54,    55,    56,    57,    58,    59,    -1,
      -1,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    13,    14,    15,    -1,    17,    77,    19,    20,    21,
      22,    23,    -1,    -1,    26,    86,    -1,    -1,    -1,    31,
      32,    -1,    34,    35,    36,    37,    -1,    39,    40,    -1,
      42,    43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,
      -1,    -1,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      13,    -1,    -1,    -1,    17,    77,    19,    20,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    -1,    -1,    31,    32,
      -1,    34,    35,    36,    37,    -1,    39,    40,    -1,    42,
      43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,    -1,
      32,    54,    55,    56,    57,    58,    59,    -1,    -1,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    50,    51,
      52,    53,    54,    55,    77,    57,    58,    59,    60,    61,
      62,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    60,    61,    62,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      89,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    60,    61,    62,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    60,    61,    62,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    60,    61,    62,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    60,    61,    62,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    60,    61,    62,    50,    51,    52,    53,    -1,    -1,
      -1,    57,    -1,    -1,    60,    61,    62,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    13,    17,    19,    20,    21,    22,    23,    26,    27,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    42,    43,    44,    45,    46,    47,    48,    49,    54,
      55,    56,    57,    58,    59,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    77,    86,    94,    95,
      96,    97,    98,   103,   105,   108,   109,   110,   111,   113,
     114,   115,   116,    39,    26,    35,    39,    40,   105,   106,
     107,     5,    10,    14,    15,    56,    81,    82,    88,    90,
     105,   120,   121,   124,   125,   128,   105,    39,    88,   128,
     129,   130,    44,    45,   105,   121,   123,   105,   106,   106,
     105,     0,    96,    88,   128,   129,    86,    86,    87,   124,
     124,   124,   128,    14,    15,    92,   105,   126,   127,   129,
      88,    41,    50,    51,    52,    53,    54,    55,    57,    60,
      61,    62,    78,    79,    80,    81,    82,    83,    84,   106,
      86,   128,   129,    32,    58,    59,    86,    87,   105,   128,
      42,    52,   112,    86,    86,    86,   106,   129,    86,   105,
      89,    92,    92,    91,    92,    87,    91,    91,   129,    86,
     108,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,    86,    89,
     121,   122,   122,   122,   128,    52,    86,   128,    73,    86,
      99,   100,   101,   102,    86,    89,   128,   128,   128,    14,
      15,   105,   127,    89,    86,   108,   117,   118,   119,    24,
     117,    86,    86,    86,    86,   128,   117,    86,   105,    86,
     117,    86,    24,    74,   119,    86,   108,    23,    42,    75,
     106,   102,    16,    86,   108,    86,   117,    42,   128,    86,
      86,   104,   105,   117,    74,   128,    86,   117,    86,    87,
      74,    86,    86,   117,   105,    86,   117,    75,    75,    86,
      86
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    93,    94,    95,    95,    96,    96,    96,    96,    96,
      97,    98,    99,   100,   100,   101,   101,   102,   102,   103,
     103,   104,   104,   104,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   106,   106,   107,   107,   108,   108,   108,   108,
     109,   109,   109,   110,   110,   110,   110,   110,   110,   111,
     111,   111,   111,   112,   112,   113,   113,   113,   114,   114,
     115,   115,   115,   115,   116,   116,   116,   117,   117,   118,
     118,   119,   119,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   121,   122,   123,   124,   124,   125,   125,
     125,   126,   126,   127,   127,   127,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   129,   129,   130,   130
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
       1,     1,     0,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     5,     3,     5,     3,     3,     3,     5,
       5,     5,     5,     1,     1,     3,     3,     3,     4,     7,
       6,     9,     7,    10,     7,    11,    12,     0,     1,     1,
       2,     1,     1,     1,     1,     1,     1,     2,     2,     2,
       4,     1,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     1,     3,     3,     3,     3,     1,     3,     3,     3,
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
#line 188 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1540 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tTHEFUNCINOF: /* tTHEFUNCINOF  */
#line 188 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1546 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tVARID: /* tVARID  */
#line 188 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1552 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 188 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1558 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 188 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1564 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 188 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1570 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tPLAYACCEL: /* tPLAYACCEL  */
#line 188 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1576 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 188 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1582 "engines/director/lingo/lingo-gr.cpp"
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
#line 194 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->_compiler->_assemblyAST = new ScriptNode((yyvsp[0].nodelist)); }
#line 1850 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 196 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 1861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 202 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 1871 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 209 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 1877 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 242 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 1883 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* factory: tFACTORY ID '\n' methodlist  */
#line 246 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 1889 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 248 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 1895 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* methodlist: %empty  */
#line 250 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 1901 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* nonemptymethodlist: methodlistline  */
#line 254 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 1912 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlist '\n' methodlistline  */
#line 260 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 1922 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlistline: '\n'  */
#line 267 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 1928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 273 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 1937 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* handler: tON ID idlist '\n' stmtlist  */
#line 277 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 1944 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* endargdef: ID  */
#line 282 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 1950 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* endargdef: endargdef ',' ID  */
#line 283 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 1956 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* ID: tAFTER  */
#line 289 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 1962 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* ID: tAND  */
#line 290 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 1968 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* ID: tBEFORE  */
#line 291 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 1974 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* ID: tCAST  */
#line 292 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 1980 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* ID: tCHAR  */
#line 293 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 1986 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* ID: tDOWN  */
#line 294 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 1992 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* ID: tEXIT  */
#line 297 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 1998 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* ID: tFIELD  */
#line 299 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2004 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* ID: tIN  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2010 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* ID: tINTERSECTS  */
#line 304 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2016 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* ID: tINTO  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2022 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* ID: tITEM  */
#line 306 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2028 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* ID: tLINE  */
#line 307 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2034 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* ID: tMOD  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2040 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* ID: tMOVIE  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2046 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* ID: tNEXT  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2052 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* ID: tNOT  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* ID: tOF  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2064 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* ID: tOPEN  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 2070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* ID: tOR  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2076 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* ID: tPLAY  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 2082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* ID: tPLAYACCEL  */
#line 319 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("playAccel"); }
#line 2088 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* ID: tPREVIOUS  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* ID: tREPEAT  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* ID: tSCRIPT  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2106 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* ID: tSET  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 2112 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* ID: tSTARTS  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2118 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* ID: tTELL  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 2124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* ID: tTO  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2130 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* ID: tASSERTERROR  */
#line 330 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2136 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* ID: tSPRITE  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* ID: tWHEN  */
#line 332 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("when"); }
#line 2148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* ID: tWHILE  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 2154 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* ID: tWINDOW  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2160 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* ID: tWITH  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 2166 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* ID: tWITHIN  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 2172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* ID: tWORD  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 2178 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* idlist: %empty  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 2184 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* nonemptyidlist: ID  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 2193 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 2201 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* proc: ID '(' exprlist ')' '\n'  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new CmdNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 2207 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* proc: ID exprlist '\n'  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist)); }
#line 2213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* proc: tPUT '(' exprlist ')' '\n'  */
#line 371 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-2].nodelist)); }
#line 2219 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* proc: tPUT exprlist '\n'  */
#line 372 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist)); }
#line 2225 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* proc: tNEXT tREPEAT '\n'  */
#line 373 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 2231 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* proc: tEXIT tREPEAT '\n'  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 2237 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 377 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2243 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 378 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2249 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 379 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2255 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* asgn: tSET varorthe to expr '\n'  */
#line 380 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2261 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* definevars: tGLOBAL idlist '\n'  */
#line 385 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 2267 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* definevars: tPROPERTY idlist '\n'  */
#line 386 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 2273 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* definevars: tINSTANCE idlist '\n'  */
#line 387 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 2279 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ifstmt: tIF expr tTHEN stmt  */
#line 390 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 2288 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ifstmt: tIF expr tTHEN '\n' stmtlist tENDIF '\n'  */
#line 394 "engines/director/lingo/lingo-gr.y"
                                                   {
		(yyval.node) = new IfStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist)); }
#line 2295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 398 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 2306 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist tENDIF '\n'  */
#line 404 "engines/director/lingo/lingo-gr.y"
                                                                                {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-5].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), stmtlist1, (yyvsp[-2].nodelist)); }
#line 2315 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE stmt  */
#line 408 "engines/director/lingo/lingo-gr.y"
                                                                    {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 2324 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE '\n' stmtlist tENDIF '\n'  */
#line 412 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new IfElseStmtNode((yyvsp[-8].node), (yyvsp[-5].nodelist), (yyvsp[-2].nodelist)); }
#line 2331 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 416 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 2338 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 418 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 2345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 420 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 2352 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* stmtlist: %empty  */
#line 424 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* nonemptystmtlist: stmtlistline  */
#line 429 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2369 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* nonemptystmtlist: stmtlist stmtlistline  */
#line 435 "engines/director/lingo/lingo-gr.y"
                                                {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* stmtlistline: '\n'  */
#line 442 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 2385 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* simpleexprnoparens: tINT  */
#line 448 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 2391 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* simpleexprnoparens: tFLOAT  */
#line 449 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 2397 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105: /* simpleexprnoparens: tSYMBOL  */
#line 450 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 2403 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* simpleexprnoparens: tSTRING  */
#line 451 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 2409 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107: /* simpleexprnoparens: '+' simpleexpr  */
#line 452 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[0].node); }
#line 2415 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* simpleexprnoparens: '-' simpleexpr  */
#line 453 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 2421 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* simpleexprnoparens: tNOT simpleexpr  */
#line 454 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 2427 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* simpleexprnoparens: ID '(' exprlist ')'  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new FuncNode((yyvsp[-3].s), (yyvsp[-1].nodelist)); }
#line 2433 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* var: ID  */
#line 460 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 2439 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* simpleexpr: '(' expr ')'  */
#line 471 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[-1].node); }
#line 2445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* list: '[' exprlist ']'  */
#line 474 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 2451 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* list: '[' ':' ']'  */
#line 475 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 2457 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* list: '[' proplist ']'  */
#line 476 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 2463 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proplist: proppair  */
#line 479 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 2472 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* proplist: proplist ',' proppair  */
#line 483 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 2480 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proppair: tSYMBOL ':' expr  */
#line 488 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 2486 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proppair: ID ':' expr  */
#line 489 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 2492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proppair: tSTRING ':' expr  */
#line 490 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 2498 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* expr: simpleexpr  */
#line 493 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 2504 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* expr: expr '+' expr  */
#line 494 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2510 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* expr: expr '-' expr  */
#line 495 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* expr: expr '*' expr  */
#line 496 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2522 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* expr: expr '/' expr  */
#line 497 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2528 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* expr: expr tMOD expr  */
#line 498 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2534 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* expr: expr '>' expr  */
#line 499 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2540 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* expr: expr '<' expr  */
#line 500 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2546 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* expr: expr tEQ expr  */
#line 501 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2552 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* expr: expr tNEQ expr  */
#line 502 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2558 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* expr: expr tGE expr  */
#line 503 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2564 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* expr: expr tLE expr  */
#line 504 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2570 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* expr: expr tAND expr  */
#line 505 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* expr: expr tOR expr  */
#line 506 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* expr: expr '&' expr  */
#line 507 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* expr: expr tCONCAT expr  */
#line 508 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2594 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* expr: expr tCONTAINS expr  */
#line 509 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2600 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* expr: expr tSTARTS expr  */
#line 510 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2606 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* exprlist: %empty  */
#line 513 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 2612 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* nonemptyexprlist: expr  */
#line 517 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 2621 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 521 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 2629 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 2633 "engines/director/lingo/lingo-gr.cpp"

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

#line 526 "engines/director/lingo/lingo-gr.y"


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
