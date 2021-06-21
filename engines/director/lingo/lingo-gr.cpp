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
  YYSYMBOL_simpleexpr_nounarymath = 135,   /* simpleexpr_nounarymath  */
  YYSYMBOL_var = 136,                      /* var  */
  YYSYMBOL_varorchunk = 137,               /* varorchunk  */
  YYSYMBOL_varorthe = 138,                 /* varorthe  */
  YYSYMBOL_chunk = 139,                    /* chunk  */
  YYSYMBOL_object = 140,                   /* object  */
  YYSYMBOL_the = 141,                      /* the  */
  YYSYMBOL_theobj = 142,                   /* theobj  */
  YYSYMBOL_menu = 143,                     /* menu  */
  YYSYMBOL_writablethe = 144,              /* writablethe  */
  YYSYMBOL_thedatetime = 145,              /* thedatetime  */
  YYSYMBOL_thenumberof = 146,              /* thenumberof  */
  YYSYMBOL_chunktype = 147,                /* chunktype  */
  YYSYMBOL_inof = 148,                     /* inof  */
  YYSYMBOL_list = 149,                     /* list  */
  YYSYMBOL_proplist = 150,                 /* proplist  */
  YYSYMBOL_proppair = 151,                 /* proppair  */
  YYSYMBOL_unarymath = 152,                /* unarymath  */
  YYSYMBOL_simpleexpr = 153,               /* simpleexpr  */
  YYSYMBOL_expr = 154,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 155,         /* expr_nounarymath  */
  YYSYMBOL_sprite = 156,                   /* sprite  */
  YYSYMBOL_exprlist = 157,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 158          /* nonemptyexprlist  */
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
#define YYFINAL  155
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4062

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  55
/* YYNRULES -- Number of rules.  */
#define YYNRULES  258
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  495

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
       0,   195,   195,   197,   203,   210,   211,   212,   213,   214,
     243,   247,   249,   251,   252,   255,   261,   268,   269,   274,
     278,   282,   283,   284,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   306,   307,   311,
     313,   314,   315,   316,   317,   318,   319,   320,   322,   323,
     324,   326,   327,   328,   329,   330,   331,   334,   336,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   350,   351,
     353,   354,   355,   356,   357,   358,   361,   362,   365,   369,
     379,   380,   381,   382,   383,   384,   387,   388,   389,   392,
     393,   394,   395,   396,   397,   398,   399,   404,   405,   406,
     407,   410,   413,   418,   422,   428,   433,   436,   446,   451,
     457,   463,   469,   477,   478,   479,   480,   483,   483,   485,
     486,   487,   490,   494,   498,   504,   508,   512,   516,   518,
     520,   524,   528,   532,   534,   535,   539,   545,   552,   553,
     559,   560,   561,   562,   563,   564,   565,   566,   567,   568,
     569,   570,   573,   575,   576,   579,   580,   583,   587,   591,
     593,   595,   597,   599,   601,   603,   605,   609,   613,   619,
     620,   621,   622,   625,   626,   627,   628,   629,   632,   634,
     635,   636,   639,   640,   641,   642,   643,   644,   645,   646,
     647,   648,   652,   653,   654,   655,   656,   659,   660,   661,
     662,   665,   665,   667,   668,   669,   672,   676,   681,   682,
     683,   686,   687,   690,   691,   695,   696,   697,   698,   699,
     700,   701,   702,   703,   704,   705,   706,   707,   708,   709,
     710,   711,   712,   713,   720,   721,   722,   723,   724,   725,
     726,   727,   728,   729,   730,   731,   732,   733,   734,   735,
     736,   737,   738,   741,   742,   745,   746,   749,   753
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
  "simpleexpr_nounarymath", "var", "varorchunk", "varorthe", "chunk",
  "object", "the", "theobj", "menu", "writablethe", "thedatetime",
  "thenumberof", "chunktype", "inof", "list", "proplist", "proppair",
  "unarymath", "simpleexpr", "expr", "expr_nounarymath", "sprite",
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,    60,
      62,    38,    43,    45,    42,    47,    37,    10,    44,    40,
      41,    91,    93,    58
};
#endif

#define YYPACT_NINF (-331)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2243,  -331,  -331,  -331,  -331,  -331,  -331,   -24,  -331,  3446,
    1376,  1473,  -331,  -331,  3446,  -331,    25,  -331,  -331,  1570,
      75,  3368,  -331,  3446,  -331,  -331,  3446,  1570,  1376,  3446,
    -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,  1473,  3446,  3446,  3290,  -331,    31,  2243,  -331,
    -331,  -331,  -331,  1570,  -331,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,  -331,  -331,   -49,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,   -12,   -31,  -331,  -331,  -331,  -331,  1667,  1667,
    1667,  1667,  1473,  1473,  1667,  1667,  1667,  1667,  1667,  3524,
    1473,  1667,  1667,  1279,   695,   -11,    10,    21,  -331,  -331,
    -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,   794,
    -331,  1473,   405,  3446,    23,    37,   891,  3446,  1473,  3602,
    -331,  -331,   -17,  -331,   103,    44,    47,   988,    48,    50,
      51,     8,    54,  3446,  -331,  -331,  -331,    55,  1085,  -331,
    -331,  3446,  -331,  -331,  -331,  -331,  1182,  3897,  -331,   -18,
      38,    41,    76,   -41,   -33,   -32,   -30,     5,    64,   126,
     127,  3884,  -331,  -331,  -331,   459,    56,    61,    63,   -16,
     -22,  -331,  3897,    65,    60,  1473,  -331,  -331,   140,  1473,
    1473,  1473,  1473,  1958,  1958,  2152,  1473,  2055,  2055,  1473,
    1473,  1473,  1473,  1473,  1473,  1473,  1473,  -331,  3720,  -331,
     207,  2970,  1473,  1473,  1473,  1473,  1473,    71,  -331,  -331,
    3680,  3680,  3680,   128,  3733,   145,  -331,  -331,  1473,  1473,
     -73,  -331,  1473,  -331,  -331,  -331,  3290,  3050,  -331,    78,
    -331,  -331,   150,  3913,  1667,  1667,  1667,  1667,  1667,  1667,
    1667,  1667,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,  -331,  -331,  -331,  -331,    17,  1764,  1861,  1667,
    1667,  1473,  -331,  1473,  1473,  -331,  1473,   596,  -331,  -331,
    1473,    79,  1473,   122,   122,   122,   122,  3967,  3967,  -331,
      -2,   122,   122,   122,   122,    -2,   -36,   -36,  -331,  -331,
      60,  1473,  1473,  1473,  1473,  1473,  1473,  1473,  1473,  1473,
    1473,  1473,  1473,  1473,  1473,  1473,  1473,  1473,  1473,  3050,
     162,  3050,  -331,    85,  -331,    87,    88,  1473,  3050,  1861,
    3746,  3897,  3446,  -331,  -331,    89,  -331,  -331,  3802,  -331,
    -331,  -331,  2403,  -331,  -331,  3050,  1473,  -331,   159,  -331,
     160,  -331,   167,  -331,   169,  -331,  -331,  1667,    17,    17,
      17,    17,  1667,  1667,    17,  1667,  1667,  -331,  -331,  -331,
    -331,  -331,  -331,   -80,  3897,  3897,  3897,    56,    61,    99,
    -331,  3897,  -331,  3897,   122,   122,   122,   122,  3967,  3967,
    -331,    -2,   122,   122,   122,   122,    -2,   -36,   -36,  -331,
    -331,    60,  2323,  3130,  3050,  -331,  -331,  -331,   147,  2490,
    -331,  3446,   -73,  -331,   106,  -331,  2570,  3897,  1667,  1667,
    1667,  1667,  -331,  1667,  1667,  1667,  1667,  -331,   176,   135,
    -331,  -331,  -331,  3210,   110,  3050,  -331,   174,  1473,   112,
     121,  -331,  -331,  3446,  -331,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,   148,  1667,  -331,  3050,  -331,  -331,  2650,  1473,
    3815,  -331,  3050,    18,  -331,  1667,  2730,   123,  3828,  3050,
    3050,  -331,  3446,  -331,   125,  -331,  3050,  2810,  -331,  -331,
    2890,   129,   130,  -331,  -331
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    24,    31,    37,    60,    71,    36,     0,    38,    76,
     101,     0,    39,    41,     0,    52,    53,    56,    58,   101,
      59,    62,    69,     0,    72,    70,     0,   101,   101,    76,
      29,    57,    54,    51,    28,    30,    34,    66,    32,    33,
      42,    43,    45,    46,    74,    75,    25,    26,    27,    47,
      63,    35,    44,    48,    49,    50,    55,    68,    64,    65,
      40,    73,    67,    76,     0,    61,     5,     0,     2,     3,
       6,     7,     8,   101,     9,    80,    86,    87,    88,    81,
      82,    83,    84,    85,     0,    99,    53,    59,    62,    67,
      61,    78,     0,    77,   140,   141,   143,   142,    31,    37,
      60,    71,    38,    52,    54,    32,    42,    45,    74,     0,
      65,     0,     0,     0,   255,   152,     0,     0,   213,   147,
     148,   149,   150,   169,   170,   171,   151,   214,   215,   102,
     216,     0,     0,    76,     0,     0,   102,     0,     0,     0,
     152,   155,     0,   156,     0,     0,     0,   102,     0,     0,
       0,     0,     0,    76,   100,     1,     4,     0,   102,    98,
     119,     0,   158,   157,   167,   168,   108,   109,   144,     0,
       0,     0,     0,    25,    26,    27,    47,    63,    44,    55,
     179,     0,   211,   212,   106,     0,   143,   142,     0,   152,
       0,   206,   257,     0,   256,   255,    91,    92,    56,     0,
       0,     0,     0,    29,    57,    51,     0,    34,    66,     0,
       0,     0,     0,     0,     0,     0,     0,   234,   104,   235,
       0,     0,     0,     0,     0,     0,     0,     0,    97,    90,
      41,    28,    30,     0,     0,    55,   117,   118,     0,     0,
      13,    95,    72,    93,    94,   121,     0,   134,   120,     0,
      89,    79,    56,   112,     0,     0,     0,     0,     0,     0,
       0,     0,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   197,   199,   200,   198,     0,     0,     0,     0,
       0,     0,   146,     0,     0,   204,     0,     0,   205,   203,
       0,     0,     0,   226,   227,   224,   225,   228,   229,   221,
     231,   232,   233,   223,   222,   230,   217,   218,   219,   220,
     103,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   134,
     122,   134,   153,     0,   154,     0,     0,     0,   134,     0,
       0,   133,     0,    17,    18,    11,    14,    15,     0,   131,
     138,   139,     0,   135,   136,   134,     0,   159,     0,   163,
       0,   165,     0,   161,     0,   201,   202,     0,    33,    43,
      46,    75,    48,    49,    50,    64,    65,   181,   174,   173,
     180,   253,   254,     0,   210,   208,   209,     0,     0,     0,
     207,   258,   145,   111,   245,   246,   243,   244,   247,   248,
     240,   250,   251,   252,   242,   241,   249,   236,   237,   238,
     239,   105,     0,     0,    10,   113,   114,   115,     0,     0,
     116,    76,     0,    96,     0,   137,    20,   110,     0,     0,
       0,     0,   172,     0,     0,     0,     0,   178,     0,     0,
     176,   177,   107,     0,     0,   134,   124,     0,     0,     0,
       0,    16,   132,    21,   160,   164,   166,   162,   192,   194,
     195,   193,     0,     0,   196,   134,   126,   123,     0,     0,
       0,   128,   134,     0,    22,     0,     0,     0,     0,   134,
      12,    19,     0,   175,     0,   125,   134,     0,    23,   127,
       0,     0,     0,   129,   130
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -331,  -331,  -331,   155,  -331,  -331,  -331,  -331,  -331,  -197,
    -331,  -331,     0,   -25,  -331,     3,   -63,  -331,     6,   200,
    -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -323,  -331,
    -330,  -117,   -20,  -105,  -331,  -100,  -331,  -331,   -45,  -210,
     213,  -331,  -331,  -331,   -58,  -331,  -331,   -52,  -331,   262,
      67,    77,   -23,    49,  -211
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    67,    68,    69,    70,    71,   344,   345,   346,   347,
      72,   473,   115,    92,    93,   351,    75,    76,   116,   117,
      77,   238,    78,    79,    80,    81,    82,    83,   352,   353,
     354,   118,   119,   333,   142,   120,   121,   122,   377,   378,
     123,   124,   125,   276,   367,   126,   190,   191,   127,   128,
     192,   218,   130,   193,   194
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      73,   141,   154,    74,   150,   310,   412,    84,   414,    91,
     254,   342,   217,   224,   133,   419,   255,   236,   290,   217,
     442,   140,   425,   144,   343,   135,   145,   237,   262,    91,
     217,   155,   426,   146,   148,   263,   264,   266,   152,   268,
     365,   217,   246,   265,   267,   366,   269,   224,   159,   217,
     199,   200,   201,   202,   222,   223,   134,   224,   214,   215,
     206,   225,   226,    91,   153,    73,   256,   161,    73,   258,
     383,    74,   257,    85,   270,   259,   287,   129,   132,   157,
     288,   271,   425,   195,   425,   160,   136,   286,   195,   425,
     212,   213,   214,   215,   147,   129,   425,   209,   210,   211,
     212,   213,   214,   215,   260,   247,   219,   196,   227,   180,
     261,   137,   138,   219,   189,   481,   482,   411,   197,   272,
     228,   273,   468,   274,   219,   275,   335,   336,   249,   151,
     334,   334,   334,    91,   229,   219,   239,   233,   425,   180,
     158,   240,   476,   219,   241,   243,   425,   244,   245,   480,
     425,   248,   250,    91,   277,   278,   487,   425,   290,   283,
     425,   251,   447,   490,   284,   285,   292,   289,   331,   166,
     167,   224,   337,   339,   206,   355,   356,   181,   413,   392,
     185,   448,   415,   349,   416,   417,   422,   428,   429,   199,
     200,   201,   202,   222,   223,   430,   224,   431,   220,   206,
     225,   226,   286,   452,   462,   234,   463,   467,   469,   471,
     332,   332,   332,   211,   212,   213,   214,   215,   472,   475,
     485,    73,   489,   156,   330,   451,   493,   494,   149,   464,
     140,   140,   140,   380,   143,   390,   209,   210,   211,   212,
     213,   214,   215,   253,   291,     0,    73,    73,     0,   199,
     200,   201,   202,   222,   223,     0,   224,     0,     0,   206,
     225,   226,     0,     0,     0,     0,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,     0,     0,     0,     0,   389,     0,   297,
     298,   299,   301,   302,     0,     0,   209,   210,   211,   212,
     213,   214,   215,     0,     0,   340,   341,   282,     0,   348,
     433,   434,   435,   436,     0,     0,   439,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    73,
       0,    73,     0,     0,     0,     0,     0,     0,    73,     0,
       0,     0,   421,     0,     0,     0,     0,     0,     0,     0,
     384,   385,    73,   386,     0,    73,     0,   391,     0,   393,
     162,   163,   164,   165,     0,     0,   168,   169,   170,   171,
     172,     0,     0,   182,   183,     0,     0,     0,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,   410,     0,   450,     0,     0,     0,
       0,     0,     0,     0,   418,     0,     0,     0,     0,     0,
       0,     0,    73,    73,    73,     0,   446,     0,     0,    73,
       0,    91,     0,   427,     0,     0,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   221,     0,
       0,     0,     0,    73,     0,    73,   466,   199,   200,   201,
     202,   222,   223,   474,   224,     0,     0,   206,   225,   226,
       0,     0,     0,     0,     0,    73,     0,     0,    73,     0,
       0,     0,    73,     0,     0,     0,    73,     0,     0,    73,
      73,     0,   488,     0,     0,     0,    73,    73,     0,     0,
      73,     0,     0,     0,   209,   210,   211,   212,   213,   214,
     215,   199,   200,   201,   202,   222,   223,     0,   224,     0,
       0,   206,   225,   226,     0,   470,   357,   358,   359,   360,
     361,   362,   363,   364,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   478,     0,     0,   379,
     379,   381,   382,     0,     0,     0,     0,     0,   209,   210,
     211,   212,   213,   214,   215,     0,     0,   281,     0,   282,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   379,     0,     1,   387,   388,     0,     2,     3,     4,
       5,     6,     0,     0,     0,     8,     0,     0,     0,    12,
      13,     0,    15,    86,    17,    18,     0,    87,    88,   432,
      22,     0,    24,    25,   437,   438,     0,   440,   441,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,    60,    61,    89,     0,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,     0,
     454,   455,   456,   457,     0,   458,   459,   460,   461,     0,
      94,    95,     1,   186,   187,     0,    98,    99,   100,   101,
       6,     0,     0,     0,     8,     0,     0,     0,    12,    13,
       0,    15,    86,    17,    18,   437,    87,    88,     0,    22,
       0,    24,    25,     0,     0,     0,     0,   483,     0,     0,
       0,    30,    31,   104,    33,    34,    35,     0,    36,    37,
     105,    39,   106,    41,   107,    43,   108,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,    56,
     109,    57,    58,   110,    60,    61,    89,     0,     0,     0,
       0,     0,     0,    90,     0,     0,     0,   111,   112,     0,
       0,     0,     0,     0,   131,     0,   114,     0,   188,    94,
      95,     1,    96,    97,     0,    98,    99,   100,   101,     6,
       0,     0,     0,     8,     0,     0,     0,    12,    13,     0,
      15,    86,   198,    18,     0,    87,    88,     0,    22,     0,
      24,    25,     0,     0,     0,     0,   199,   200,   201,   202,
     203,   204,   104,   205,    34,    35,   206,   207,   208,   105,
      39,   106,    41,   107,    43,   108,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,   109,
      57,    58,   110,    60,    61,    89,     0,     0,     0,     0,
       0,     0,    90,   209,   210,   211,   212,   213,   214,   215,
       0,     0,   216,   131,     0,   114,    94,    95,     1,    96,
      97,     0,    98,    99,   100,   101,     6,     0,     0,     0,
       8,     0,     0,     0,    12,   230,     0,    15,    86,    17,
      18,     0,    87,    88,     0,    22,     0,    24,    25,     0,
       0,     0,     0,   199,   200,   201,   202,   203,   204,   104,
     205,   231,   232,   206,   207,   208,   105,    39,   106,    41,
     107,    43,   108,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,    54,    55,    56,   109,    57,    58,   110,
      60,    61,    89,     0,     0,     0,     0,     0,     0,    90,
     209,   210,   211,   212,   213,   214,   215,     0,     0,   216,
     131,     0,   114,    94,    95,     1,    96,    97,     0,    98,
      99,   100,   101,     6,     0,     0,     0,     8,     0,     0,
       0,    12,    13,     0,    15,    86,    17,    18,     0,    87,
      88,     0,    22,     0,   242,    25,     0,     0,     0,     0,
     199,   200,   201,   202,   203,   204,   104,   205,    34,    35,
     206,   207,   208,   105,    39,   106,    41,   107,    43,   108,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,    56,   109,    57,    58,   110,    60,    61,    89,
       0,     0,     0,     0,     0,     0,    90,   209,   210,   211,
     212,   213,   214,   215,     0,     0,   216,   131,     0,   114,
      94,    95,     1,    96,    97,     0,    98,    99,   100,   101,
       6,     0,     0,     0,     8,     0,     0,     0,    12,    13,
       0,    15,    86,    17,    18,     0,    87,    88,     0,    22,
       0,    24,    25,     0,     0,     0,     0,   199,   200,   201,
     202,   203,   204,   104,   205,    34,    35,   206,   207,   208,
     105,    39,   106,    41,   107,    43,   108,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,    56,
     109,    57,    58,   110,    60,    61,    89,     0,     0,     0,
       0,     0,     0,    90,   209,   210,   211,   212,   213,   214,
     215,     0,     0,   216,   131,     0,   114,    94,    95,     1,
      96,    97,     0,    98,    99,   100,   101,     6,     0,     0,
       0,     8,     0,     0,     0,    12,    13,     0,    15,    86,
     252,    18,     0,    87,    88,     0,    22,     0,    24,    25,
       0,     0,     0,     0,   199,   200,   201,   202,   203,   204,
     104,   205,    34,    35,   206,   207,   208,   105,    39,   106,
      41,   107,    43,   108,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,    56,   109,    57,    58,
     110,    60,    61,    89,     0,     0,     0,     0,     0,     0,
      90,   209,   210,   211,   212,   213,   214,   215,     0,     0,
       0,   131,     0,   114,    94,    95,     1,    96,    97,     0,
      98,    99,   100,   101,     6,     0,     0,     0,     8,     0,
       0,     0,    12,    13,     0,    15,    86,    17,    18,     0,
      87,    88,     0,    22,     0,    24,    25,     0,     0,     0,
       0,     0,     0,     0,     0,    30,    31,   104,    33,    34,
      35,     0,    36,    37,   105,    39,   106,    41,   107,    43,
     108,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,    54,    55,    56,   109,    57,    58,   110,    60,    61,
      89,     0,     0,     0,     0,     0,     0,    90,     0,     0,
       0,   111,   112,     0,     0,     0,     0,     0,   131,   184,
     114,    94,    95,     1,    96,    97,     0,    98,    99,   100,
     101,     6,     0,     0,     0,   102,     0,     0,     0,    12,
      13,     0,   103,    86,    17,    18,     0,    87,    88,     0,
      22,     0,    24,    25,     0,     0,     0,     0,     0,     0,
       0,     0,    30,    31,   104,    33,    34,    35,     0,    36,
      37,   105,    39,   106,    41,   107,    43,   108,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,   109,    57,    58,   110,    60,    61,    89,     0,     0,
       0,     0,     0,     0,    90,     0,     0,     0,   111,   112,
       0,     0,     0,     0,     0,   113,     0,   114,    94,    95,
       1,    96,    97,     0,    98,    99,   100,   101,     6,     0,
       0,     0,     8,     0,     0,     0,    12,    13,     0,    15,
      86,    17,    18,     0,    87,    88,     0,    22,     0,    24,
      25,     0,     0,     0,     0,     0,     0,     0,     0,    30,
      31,   104,    33,    34,    35,     0,    36,    37,   105,    39,
     106,    41,   107,    43,   108,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,   109,    57,
      58,   110,    60,    61,    89,     0,     0,     0,     0,     0,
       0,    90,     0,     0,     0,   111,   112,     0,     0,     0,
       0,     0,   131,     0,   114,    94,    95,     1,    96,    97,
       0,    98,    99,   100,   101,     6,     0,     0,     0,     8,
       0,     0,     0,    12,    13,     0,    15,    86,    17,    18,
       0,    87,    88,     0,    22,     0,    24,    25,     0,     0,
       0,     0,     0,     0,     0,     0,    30,    31,   104,    33,
      34,    35,     0,    36,    37,   105,    39,   106,    41,   107,
      43,   108,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,   109,    57,    58,   110,    60,
      61,    89,     0,     0,     0,     0,     0,     0,    90,     0,
       0,     0,   111,   112,     0,     0,     0,     0,     0,   113,
       0,   114,    94,    95,     1,    96,    97,     0,    98,    99,
     100,   101,     6,     0,     0,     0,     8,     0,     0,     0,
      12,    13,     0,    15,    86,    17,    18,     0,    87,    88,
       0,    22,     0,    24,    25,     0,     0,     0,     0,     0,
       0,     0,     0,    30,    31,   104,    33,    34,    35,     0,
      36,    37,   105,    39,   106,    41,   107,    43,   108,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,    54,
      55,    56,   109,    57,    58,    59,    60,    61,    89,     0,
       0,     0,     0,     0,     0,    90,     0,     0,     0,   111,
     112,     0,     0,     0,     0,     0,   131,     0,   114,    94,
      95,     1,    96,    97,     0,    98,    99,   100,   101,     6,
       0,     0,     0,     8,     0,     0,     0,    12,    13,     0,
      15,    86,    17,    18,     0,    87,    88,     0,    22,     0,
      24,    25,     0,     0,     0,     0,     0,     0,     0,     0,
      30,    31,   104,    33,    34,    35,     0,    36,    37,   105,
     368,   106,   369,   107,   370,   108,   371,    46,    47,    48,
      49,    50,     0,    51,    52,   372,   373,   374,    56,   109,
      57,   375,   376,    60,    61,    89,     0,     0,     0,     0,
       0,     0,    90,     0,     0,     0,   111,   112,     0,     0,
       0,     0,     0,   131,     0,   114,    94,    95,     1,    96,
      97,     0,    98,    99,   100,   101,     6,     0,     0,     0,
       8,     0,     0,     0,    12,    13,     0,    15,    86,    17,
      18,     0,    87,    88,     0,    22,     0,    24,    25,     0,
       0,     0,     0,     0,     0,     0,     0,    30,    31,   104,
      33,    34,    35,     0,    36,    37,   105,    39,   106,    41,
     107,    43,   108,    45,    46,    47,    48,    49,    50,     0,
      51,    52,   372,   373,    55,    56,   109,    57,   375,   376,
      60,    61,    89,     0,     0,     0,     0,     0,     0,    90,
       0,     0,     0,   111,   112,     0,     0,     0,     0,     0,
     131,     0,   114,    94,    95,     1,    96,    97,     0,    98,
      99,   100,   101,     6,     0,     0,     0,     8,     0,     0,
       0,    12,    13,     0,    15,    86,    17,    18,     0,    87,
      88,     0,    22,     0,    24,    25,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   104,    33,    34,    35,
       0,    36,    37,   105,    39,   106,    41,   107,    43,   108,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,    56,   109,    57,    58,   110,    60,    61,    89,
       0,     0,     0,     0,     0,     0,    90,     0,     0,     0,
     111,   112,     0,     0,     0,     0,     0,   131,     0,   114,
      94,    95,     1,    96,    97,     0,    98,    99,   100,   101,
       6,     0,     0,     0,     8,     0,     0,     0,    12,    13,
       0,    15,    86,    17,    18,     0,    87,    88,     0,    22,
       0,    24,    25,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   104,    33,    34,    35,     0,     0,     0,
     105,    39,   106,    41,   107,    43,   108,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,    56,
     109,    57,    58,   110,    60,    61,    89,     0,     0,     0,
       0,     0,     0,    90,     0,     0,     0,   111,   112,     0,
       0,     0,     0,     0,   131,     0,   114,    94,    95,     1,
      96,    97,     0,    98,    99,   100,   101,     6,     0,     0,
       0,     8,     0,     0,     0,    12,    13,     0,    15,    86,
      17,    18,     0,    87,    88,     0,    22,     0,    24,    25,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     104,     0,    34,    35,     0,     0,     0,   105,    39,   106,
      41,   107,    43,   108,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,    56,   109,    57,    58,
     110,    60,    61,    89,     0,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       1,   131,     0,   114,     2,     3,     4,     5,     6,     0,
       0,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,     0,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    30,
      31,    32,    33,    34,    35,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,    64,     0,     0,     0,
       1,    65,     0,     0,     2,     3,     4,     5,     6,   443,
      66,     7,     8,     9,    10,    11,    12,    13,     0,    15,
      16,    17,    18,    19,    20,    21,     0,    22,    23,    24,
      25,     0,    27,    28,    29,     0,     0,     0,     0,    30,
      31,    32,    33,    34,    35,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,     0,     0,   444,     0,
       1,    65,     0,     0,     2,     3,     4,     5,     6,     0,
     350,     7,     8,     9,    10,    11,    12,    13,     0,    15,
      16,    17,    18,    19,    20,    21,     0,    22,    23,    24,
      25,     0,    27,    28,    29,     0,     0,     0,     0,    30,
      31,    32,    33,    34,    35,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,     0,     0,     0,     0,
     424,    65,     0,     0,     0,     0,     0,     1,     0,     0,
     350,     2,     3,     4,     5,     6,     0,     0,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,   449,     1,    65,     0,
     453,     2,     3,     4,     5,     6,     0,   350,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   350,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,   477,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   350,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,   484,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   350,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,   491,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   350,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,   492,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   350,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   329,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   350,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   445,     7,     8,
       9,    10,    11,    12,    13,     0,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,     0,     1,    65,     0,
       0,     2,     3,     4,     5,     6,     0,   465,     7,     8,
       9,    10,     0,    12,    13,     0,    15,    16,    17,    18,
      19,    87,    21,     0,    22,     0,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    89,    63,     0,     0,     1,     0,     0,    65,     2,
       3,     4,     5,     6,     0,     0,     0,     8,     0,     0,
       0,    12,    13,     0,    15,    86,    17,    18,     0,    87,
      88,     0,    22,     0,    24,    25,     0,     0,     0,     0,
       0,     0,     0,     0,    30,    31,    32,    33,    34,    35,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,    56,   139,    57,    58,    59,    60,    61,    89,
       0,     0,     0,     1,     0,     0,    90,     2,     3,     4,
       5,     6,     0,     0,     0,     8,     0,     0,     0,    12,
      13,     0,    15,    86,    17,    18,     0,    87,    88,     0,
      22,     0,    24,    25,     0,     0,     0,     0,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,    60,    61,    89,     0,     0,
       0,     1,     0,     0,    90,     2,     3,     4,     5,     6,
       0,     0,     0,     8,     0,     0,     0,    12,    13,     0,
      15,    86,    17,    18,     0,    87,    88,     0,    22,     0,
      24,    25,     0,     0,     0,     0,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,   173,   174,   175,
     176,   177,     0,    51,   178,    53,    54,    55,   179,     0,
      57,    58,    59,    60,    61,    89,     0,     0,     0,     1,
       0,     0,    90,     2,     3,     4,     5,     6,     0,     0,
       0,     8,     0,     0,     0,    12,    13,     0,    15,    86,
      17,    18,     0,    87,    88,     0,    22,     0,    24,    25,
       0,     0,     0,     0,     0,     0,     0,     0,    30,    31,
      32,    33,    34,    35,     0,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,   235,     0,    57,    58,
      59,    60,    61,    89,     0,     0,     0,     1,     0,     0,
      90,    98,    99,     4,     5,     6,     0,     0,     0,     8,
       0,     0,     0,    12,    13,     0,    15,    86,    17,    18,
       0,    87,    88,     0,    22,     0,    24,    25,     0,     0,
       0,     0,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,   105,    39,   106,    41,   107,
      43,   108,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    89,   311,   312,   313,   314,   315,   316,    90,   317,
       0,     0,   318,   319,   320,   199,   200,   201,   202,   222,
     223,     0,   224,     0,     0,   206,   225,   226,   199,   200,
     201,   202,   222,   223,     0,   224,     0,     0,   206,   225,
     226,     0,     0,     0,     0,     0,     0,     0,     0,   321,
     322,   323,   324,   325,   326,   327,     0,     0,   328,     0,
       0,     0,   209,   210,   211,   212,   213,   214,   215,     0,
     338,     0,     0,     0,     0,   209,   210,   211,   212,   213,
     214,   215,     0,   420,   199,   200,   201,   202,   222,   223,
       0,   224,     0,     0,   206,   225,   226,   199,   200,   201,
     202,   222,   223,     0,   224,     0,     0,   206,   225,   226,
     199,   200,   201,   202,   222,   223,     0,   224,     0,     0,
     206,   225,   226,     0,     0,     0,     0,     0,     0,     0,
       0,   209,   210,   211,   212,   213,   214,   215,     0,   423,
       0,     0,     0,     0,   209,   210,   211,   212,   213,   214,
     215,     0,   479,     0,     0,     0,     0,   209,   210,   211,
     212,   213,   214,   215,     0,   486,   199,   200,   201,   202,
     222,   223,     0,   224,     0,     0,   206,   225,   226,   199,
     200,   201,   202,   222,   223,     0,   224,     0,     0,   206,
     225,   226,     0,     0,     0,   311,   312,   313,   314,   315,
     316,     0,   317,   279,   280,   318,   319,   320,     0,     0,
       0,     0,     0,   209,   210,   211,   212,   213,   214,   215,
       0,     0,     0,     0,     0,     0,   209,   210,   211,   212,
     213,   214,   215,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   321,   322,   323,   324,   325,   326,   327,   199,
     200,   201,   202,     0,     0,     0,   224,     0,     0,   206,
     225,   226,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   209,   210,   211,   212,
     213,   214,   215
};

static const yytype_int16 yycheck[] =
{
       0,    21,    65,     0,    29,   216,   329,    31,   331,     9,
      28,    84,   129,    49,    14,   338,    34,    34,    98,   136,
     100,    21,   352,    23,    97,    19,    26,    44,    69,    29,
     147,     0,   355,    27,    28,    76,    69,    69,    63,    69,
      23,   158,    34,    76,    76,    28,    76,    49,    97,   166,
      42,    43,    44,    45,    46,    47,    31,    49,    94,    95,
      52,    53,    54,    63,    64,    65,    28,    98,    68,    28,
     281,    68,    34,    97,    69,    34,    98,    10,    11,    73,
     102,    76,   412,    99,   414,    97,    19,   103,    99,   419,
      92,    93,    94,    95,    27,    28,   426,    89,    90,    91,
      92,    93,    94,    95,    28,    97,   129,    97,   133,   109,
      34,    36,    37,   136,   114,    97,    98,   328,    97,    55,
      97,    57,   445,    59,   147,    61,   231,   232,   153,    62,
     230,   231,   232,   133,    97,   158,    33,   137,   468,   139,
      73,    97,   465,   166,    97,    97,   476,    97,    97,   472,
     480,    97,    97,   153,    28,    28,   479,   487,    98,   103,
     490,   161,    15,   486,   103,   102,    26,   102,    97,   102,
     103,    49,    44,    28,    52,    97,    26,   110,    16,   100,
     113,    34,    97,   246,    97,    97,    97,    28,    28,    42,
      43,    44,    45,    46,    47,    28,    49,    28,   131,    52,
      53,    54,   103,    97,    28,   138,    71,    97,    34,    97,
     230,   231,   232,    91,    92,    93,    94,    95,    97,    71,
      97,   221,    97,    68,   221,   422,    97,    97,    28,   439,
     230,   231,   232,   278,    21,   287,    89,    90,    91,    92,
      93,    94,    95,   166,   195,    -1,   246,   247,    -1,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    -1,    -1,    -1,    -1,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,    -1,    -1,    -1,    -1,   287,    -1,   222,
     223,   224,   225,   226,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    -1,    -1,   238,   239,   100,    -1,   242,
     368,   369,   370,   371,    -1,    -1,   374,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   329,
      -1,   331,    -1,    -1,    -1,    -1,    -1,    -1,   338,    -1,
      -1,    -1,   342,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     283,   284,   352,   286,    -1,   355,    -1,   290,    -1,   292,
      98,    99,   100,   101,    -1,    -1,   104,   105,   106,   107,
     108,    -1,    -1,   111,   112,    -1,    -1,    -1,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,    -1,   421,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   337,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   412,   413,   414,    -1,   413,    -1,    -1,   419,
      -1,   421,    -1,   356,    -1,    -1,   426,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,   443,    -1,   445,   443,    42,    43,    44,
      45,    46,    47,   453,    49,    -1,    -1,    52,    53,    54,
      -1,    -1,    -1,    -1,    -1,   465,    -1,    -1,   468,    -1,
      -1,    -1,   472,    -1,    -1,    -1,   476,    -1,    -1,   479,
     480,    -1,   482,    -1,    -1,    -1,   486,   487,    -1,    -1,
     490,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    -1,   448,   254,   255,   256,   257,
     258,   259,   260,   261,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   469,    -1,    -1,   277,
     278,   279,   280,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   339,    -1,     7,     8,     9,    -1,    11,    12,    13,
      14,    15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,
      24,    -1,    26,    27,    28,    29,    -1,    31,    32,   367,
      34,    -1,    36,    37,   372,   373,    -1,   375,   376,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    -1,    76,    77,    78,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,
     428,   429,   430,   431,    -1,   433,   434,   435,   436,    -1,
       5,     6,     7,     8,     9,    -1,    11,    12,    13,    14,
      15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,
      -1,    26,    27,    28,    29,   463,    31,    32,    -1,    34,
      -1,    36,    37,    -1,    -1,    -1,    -1,   475,    -1,    -1,
      -1,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    -1,    -1,    99,    -1,   101,    -1,   103,     5,
       6,     7,     8,     9,    -1,    11,    12,    13,    14,    15,
      -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,
      26,    27,    28,    29,    -1,    31,    32,    -1,    34,    -1,
      36,    37,    -1,    -1,    -1,    -1,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      -1,    -1,    98,    99,    -1,   101,     5,     6,     7,     8,
       9,    -1,    11,    12,    13,    14,    15,    -1,    -1,    -1,
      19,    -1,    -1,    -1,    23,    24,    -1,    26,    27,    28,
      29,    -1,    31,    32,    -1,    34,    -1,    36,    37,    -1,
      -1,    -1,    -1,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    98,
      99,    -1,   101,     5,     6,     7,     8,     9,    -1,    11,
      12,    13,    14,    15,    -1,    -1,    -1,    19,    -1,    -1,
      -1,    23,    24,    -1,    26,    27,    28,    29,    -1,    31,
      32,    -1,    34,    -1,    36,    37,    -1,    -1,    -1,    -1,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    -1,    98,    99,    -1,   101,
       5,     6,     7,     8,     9,    -1,    11,    12,    13,    14,
      15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,
      -1,    26,    27,    28,    29,    -1,    31,    32,    -1,    34,
      -1,    36,    37,    -1,    -1,    -1,    -1,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    -1,    98,    99,    -1,   101,     5,     6,     7,
       8,     9,    -1,    11,    12,    13,    14,    15,    -1,    -1,
      -1,    19,    -1,    -1,    -1,    23,    24,    -1,    26,    27,
      28,    29,    -1,    31,    32,    -1,    34,    -1,    36,    37,
      -1,    -1,    -1,    -1,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    -1,
      -1,    99,    -1,   101,     5,     6,     7,     8,     9,    -1,
      11,    12,    13,    14,    15,    -1,    -1,    -1,    19,    -1,
      -1,    -1,    23,    24,    -1,    26,    27,    28,    29,    -1,
      31,    32,    -1,    34,    -1,    36,    37,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,
      -1,    92,    93,    -1,    -1,    -1,    -1,    -1,    99,   100,
     101,     5,     6,     7,     8,     9,    -1,    11,    12,    13,
      14,    15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,
      24,    -1,    26,    27,    28,    29,    -1,    31,    32,    -1,
      34,    -1,    36,    37,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,
      -1,    -1,    -1,    -1,    -1,    99,    -1,   101,     5,     6,
       7,     8,     9,    -1,    11,    12,    13,    14,    15,    -1,
      -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,    26,
      27,    28,    29,    -1,    31,    32,    -1,    34,    -1,    36,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,
      -1,    -1,    99,    -1,   101,     5,     6,     7,     8,     9,
      -1,    11,    12,    13,    14,    15,    -1,    -1,    -1,    19,
      -1,    -1,    -1,    23,    24,    -1,    26,    27,    28,    29,
      -1,    31,    32,    -1,    34,    -1,    36,    37,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,
      -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    -1,    99,
      -1,   101,     5,     6,     7,     8,     9,    -1,    11,    12,
      13,    14,    15,    -1,    -1,    -1,    19,    -1,    -1,    -1,
      23,    24,    -1,    26,    27,    28,    29,    -1,    31,    32,
      -1,    34,    -1,    36,    37,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    92,
      93,    -1,    -1,    -1,    -1,    -1,    99,    -1,   101,     5,
       6,     7,     8,     9,    -1,    11,    12,    13,    14,    15,
      -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,
      26,    27,    28,    29,    -1,    31,    32,    -1,    34,    -1,
      36,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    -1,    -1,    -1,    92,    93,    -1,    -1,
      -1,    -1,    -1,    99,    -1,   101,     5,     6,     7,     8,
       9,    -1,    11,    12,    13,    14,    15,    -1,    -1,    -1,
      19,    -1,    -1,    -1,    23,    24,    -1,    26,    27,    28,
      29,    -1,    31,    32,    -1,    34,    -1,    36,    37,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    -1,
      99,    -1,   101,     5,     6,     7,     8,     9,    -1,    11,
      12,    13,    14,    15,    -1,    -1,    -1,    19,    -1,    -1,
      -1,    23,    24,    -1,    26,    27,    28,    29,    -1,    31,
      32,    -1,    34,    -1,    36,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,
      92,    93,    -1,    -1,    -1,    -1,    -1,    99,    -1,   101,
       5,     6,     7,     8,     9,    -1,    11,    12,    13,    14,
      15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,
      -1,    26,    27,    28,    29,    -1,    31,    32,    -1,    34,
      -1,    36,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    49,    50,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    -1,    -1,    99,    -1,   101,     5,     6,     7,
       8,     9,    -1,    11,    12,    13,    14,    15,    -1,    -1,
      -1,    19,    -1,    -1,    -1,    23,    24,    -1,    26,    27,
      28,    29,    -1,    31,    32,    -1,    34,    -1,    36,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    73,    74,    75,    76,    77,
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
      80,    81,    82,    -1,    -1,     7,    -1,    -1,    88,    11,
      12,    13,    14,    15,    -1,    -1,    -1,    19,    -1,    -1,
      -1,    23,    24,    -1,    26,    27,    28,    29,    -1,    31,
      32,    -1,    34,    -1,    36,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      -1,    -1,    -1,     7,    -1,    -1,    88,    11,    12,    13,
      14,    15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,
      24,    -1,    26,    27,    28,    29,    -1,    31,    32,    -1,
      34,    -1,    36,    37,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    -1,    76,    77,    78,    79,    80,    81,    -1,    -1,
      -1,     7,    -1,    -1,    88,    11,    12,    13,    14,    15,
      -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,
      26,    27,    28,    29,    -1,    31,    32,    -1,    34,    -1,
      36,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    -1,
      76,    77,    78,    79,    80,    81,    -1,    -1,    -1,     7,
      -1,    -1,    88,    11,    12,    13,    14,    15,    -1,    -1,
      -1,    19,    -1,    -1,    -1,    23,    24,    -1,    26,    27,
      28,    29,    -1,    31,    32,    -1,    34,    -1,    36,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    73,    74,    -1,    76,    77,
      78,    79,    80,    81,    -1,    -1,    -1,     7,    -1,    -1,
      88,    11,    12,    13,    14,    15,    -1,    -1,    -1,    19,
      -1,    -1,    -1,    23,    24,    -1,    26,    27,    28,    29,
      -1,    31,    32,    -1,    34,    -1,    36,    37,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    76,    77,    78,    79,
      80,    81,    42,    43,    44,    45,    46,    47,    88,    49,
      -1,    -1,    52,    53,    54,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    -1,    -1,    98,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    -1,    97,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    -1,    97,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    52,    53,    54,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    -1,    -1,    -1,    42,    43,    44,    45,    46,
      47,    -1,    49,    79,    80,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    42,
      43,    44,    45,    -1,    -1,    -1,    49,    -1,    -1,    52,
      53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95
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
      13,    14,    19,    26,    48,    55,    57,    59,    61,    75,
      78,    92,    93,    99,   101,   116,   122,   123,   135,   136,
     139,   140,   141,   144,   145,   146,   149,   152,   153,   154,
     156,    99,   154,   116,    31,   122,   154,    36,    37,    75,
     116,   136,   138,   144,   116,   116,   122,   154,   122,   123,
     117,   154,   117,   116,   120,     0,   107,   122,   154,    97,
      97,    98,   153,   153,   153,   153,   154,   154,   153,   153,
     153,   153,   153,    63,    64,    65,    66,    67,    70,    74,
     116,   154,   153,   153,   100,   154,     8,     9,   103,   116,
     150,   151,   154,   157,   158,    99,    97,    97,    28,    42,
      43,    44,    45,    46,    47,    49,    52,    53,    54,    89,
      90,    91,    92,    93,    94,    95,    98,   135,   155,   156,
     154,    33,    46,    47,    49,    53,    54,   117,    97,    97,
      24,    50,    51,   116,   154,    74,    34,    44,   125,    33,
      97,    97,    36,    97,    97,    97,    34,    97,    97,   117,
      97,   116,    28,   155,    28,    34,    28,    34,    28,    34,
      28,    34,    69,    76,    69,    76,    69,    76,    69,    76,
      69,    76,    55,    57,    59,    61,   147,    28,    28,    79,
      80,    98,   100,   103,   103,   102,   103,    98,   102,   102,
      98,   157,    26,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     158,    42,    43,    44,    45,    46,    47,    49,    52,    53,
      54,    89,    90,    91,    92,    93,    94,    95,    98,    97,
     119,    97,   136,   137,   139,   137,   137,    44,    97,    28,
     154,   154,    84,    97,   110,   111,   112,   113,   154,   120,
      97,   119,   132,   133,   134,    97,    26,   153,   153,   153,
     153,   153,   153,   153,   153,    23,    28,   148,    56,    58,
      60,    62,    71,    72,    73,    77,    78,   142,   143,   153,
     142,   153,   153,   158,   154,   154,   154,     8,     9,   116,
     151,   154,   100,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   158,   132,    16,   132,    97,    97,    97,   154,   132,
      97,   116,    97,    97,    87,   134,   132,   154,    28,    28,
      28,    28,   153,   148,   148,   148,   148,   153,   153,   148,
     153,   153,   100,    16,    85,    97,   119,    15,    34,    86,
     117,   113,    97,    10,   153,   153,   153,   153,   153,   153,
     153,   153,    28,    71,   143,    97,   119,    97,   132,    34,
     154,    97,    97,   115,   116,    71,   132,    85,   154,    97,
     132,    97,    98,   153,    85,    97,    97,   132,   116,    97,
     132,    86,    86,    97,    97
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
     116,   116,   116,   116,   116,   116,   117,   117,   118,   118,
     119,   119,   119,   119,   119,   119,   120,   120,   120,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   122,   122,   122,   122,   122,   122,   122,   123,   123,
     123,   123,   123,   124,   124,   124,   124,   125,   125,   126,
     126,   126,   127,   127,   128,   128,   128,   128,   129,   129,
     129,   130,   130,   131,   132,   132,   133,   133,   134,   134,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   136,   137,   137,   138,   138,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   140,   140,   141,
     141,   141,   141,   142,   142,   142,   142,   142,   143,   144,
     144,   144,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   146,   146,   146,   146,   146,   147,   147,   147,
     147,   148,   148,   149,   149,   149,   150,   150,   151,   151,
     151,   152,   152,   153,   153,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   156,   156,   157,   157,   158,   158
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
       1,     1,     1,     1,     1,     1,     0,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     5,     3,     3,     2,
       2,     0,     1,     3,     2,     4,     2,     5,     2,     2,
       5,     4,     3,     5,     5,     5,     5,     1,     1,     3,
       3,     3,     4,     7,     6,     9,     7,    10,     7,    11,
      12,     4,     6,     4,     0,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     2,     4,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     4,
       6,     4,     6,     4,     6,     4,     6,     2,     2,     1,
       1,     1,     5,     1,     1,     5,     2,     2,     2,     2,
       4,     4,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     6,     6,     6,     6,     6,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     1,     3,     3,     3,
       3,     2,     2,     1,     1,     1,     1,     3,     3,     3,
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
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2154 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2160 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2166 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2172 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2178 "engines/director/lingo/lingo-gr.cpp"
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
#line 195 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->_compiler->_assemblyAST = new ScriptNode((yyvsp[0].nodelist)); }
#line 2446 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 197 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2457 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 203 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2467 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 210 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 243 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2479 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* factory: tFACTORY ID '\n' methodlist  */
#line 247 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2485 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 249 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2491 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* methodlist: %empty  */
#line 251 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2497 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* nonemptymethodlist: methodlistline  */
#line 255 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2508 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlist '\n' methodlistline  */
#line 261 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 2518 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlistline: '\n'  */
#line 268 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2524 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 274 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2533 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* handler: tON ID idlist '\n' stmtlist  */
#line 278 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2540 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* endargdef: ID  */
#line 283 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2546 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* endargdef: endargdef ',' ID  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2552 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* ID: tABBREVIATED  */
#line 290 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2558 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* ID: tABBREV  */
#line 291 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2564 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* ID: tABBR  */
#line 292 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2570 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* ID: tAFTER  */
#line 293 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* ID: tAND  */
#line 294 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 2582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* ID: tBEFORE  */
#line 295 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* ID: tCAST  */
#line 296 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2594 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* ID: tCHAR  */
#line 297 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2600 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* ID: tCHARS  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2606 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* ID: tCONTAINS  */
#line 299 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("contains"); }
#line 2612 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* ID: tDATE  */
#line 300 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* ID: tDOWN  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2624 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* ID: tFIELD  */
#line 306 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2630 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* ID: tFRAME  */
#line 307 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* ID: tIN  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2642 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* ID: tINTERSECTS  */
#line 313 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2648 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* ID: tINTO  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2654 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* ID: tITEM  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2660 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* ID: tITEMS  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2666 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* ID: tLAST  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2672 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* ID: tLINE  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2678 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* ID: tLINES  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2684 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* ID: tLONG  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2690 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* ID: tMENU  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2696 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* ID: tMENUITEM  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2702 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* ID: tMENUITEMS  */
#line 324 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2708 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* ID: tMOD  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2714 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* ID: tMOVIE  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2720 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* ID: tNEXT  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* ID: tNOT  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2732 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* ID: tNUMBER  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2738 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* ID: tOF  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2744 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* ID: tOR  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2750 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* ID: tPREVIOUS  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2756 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* ID: tREPEAT  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2762 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* ID: tSCRIPT  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2768 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* ID: tASSERTERROR  */
#line 341 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2774 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* ID: tSET  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 2780 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* ID: tSHORT  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2786 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* ID: tSOUND  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2792 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* ID: tSPRITE  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2798 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* ID: tSTARTS  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2804 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* ID: tTELL  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 2810 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* ID: tTIME  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 2816 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* ID: tTO  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2822 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* ID: tWHILE  */
#line 353 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 2828 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* ID: tWINDOW  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2834 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* ID: tWITH  */
#line 355 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 2840 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* ID: tWITHIN  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 2846 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* ID: tWORD  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 2852 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* ID: tWORDS  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 2858 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* idlist: %empty  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 2864 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* nonemptyidlist: ID  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 2873 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 2881 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* proc: ID cmdargs '\n'  */
#line 392 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist)); }
#line 2887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* proc: tPUT cmdargs '\n'  */
#line 393 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist)); }
#line 2893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* proc: tGO cmdargs '\n'  */
#line 394 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist)); }
#line 2899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* proc: tGO frameargs '\n'  */
#line 395 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist)); }
#line 2905 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* proc: tPLAY cmdargs '\n'  */
#line 396 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 2911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* proc: tPLAY frameargs '\n'  */
#line 397 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 2917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* proc: tOPEN cmdargs '\n'  */
#line 398 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist)); }
#line 2923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* proc: tOPEN expr tWITH expr '\n'  */
#line 399 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args); }
#line 2933 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* proc: tNEXT tREPEAT '\n'  */
#line 404 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 2939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* proc: tEXIT tREPEAT '\n'  */
#line 405 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 2945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* proc: tEXIT '\n'  */
#line 406 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 2951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* proc: tASSERTERROR stmtoneliner  */
#line 407 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 2957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* cmdargs: %empty  */
#line 410 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 2965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102: /* cmdargs: expr  */
#line 413 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2975 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* cmdargs: expr ',' nonemptyexprlist  */
#line 418 "engines/director/lingo/lingo-gr.y"
                                                {
		// This matches `cmd args, ...)
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 2984 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* cmdargs: expr expr_nounarymath  */
#line 422 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 2995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist  */
#line 428 "engines/director/lingo/lingo-gr.y"
                                                           {
		// This matches `cmd arg arg, ...`
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 3005 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* cmdargs: '(' ')'  */
#line 433 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107: /* cmdargs: '(' expr ',' nonemptyexprlist ')'  */
#line 436 "engines/director/lingo/lingo-gr.y"
                                                  {
		// This matches `cmd(args, ...)`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3022 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* frameargs: tFRAME expr  */
#line 446 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3032 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* frameargs: tMOVIE expr  */
#line 451 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 457 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3054 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* frameargs: expr tOF tMOVIE expr  */
#line 463 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3065 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* frameargs: tFRAME expr expr_nounarymath  */
#line 469 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3076 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 477 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 478 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3088 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 479 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* asgn: tSET varorthe to expr '\n'  */
#line 480 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* definevars: tGLOBAL idlist '\n'  */
#line 485 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3106 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* definevars: tPROPERTY idlist '\n'  */
#line 486 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3112 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* definevars: tINSTANCE idlist '\n'  */
#line 487 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3118 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* ifstmt: tIF expr tTHEN stmt  */
#line 490 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3127 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* ifstmt: tIF expr tTHEN '\n' stmtlist tENDIF '\n'  */
#line 494 "engines/director/lingo/lingo-gr.y"
                                                   {
		(yyval.node) = new IfStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist)); }
#line 3134 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 498 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3145 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist tENDIF '\n'  */
#line 504 "engines/director/lingo/lingo-gr.y"
                                                                                {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-5].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), stmtlist1, (yyvsp[-2].nodelist)); }
#line 3154 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE stmt  */
#line 508 "engines/director/lingo/lingo-gr.y"
                                                                    {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3163 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE '\n' stmtlist tENDIF '\n'  */
#line 512 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new IfElseStmtNode((yyvsp[-8].node), (yyvsp[-5].nodelist), (yyvsp[-2].nodelist)); }
#line 3170 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 516 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3177 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 518 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3184 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 520 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3191 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* tell: tTELL expr tTO stmtoneliner  */
#line 524 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3200 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 528 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3207 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* when: tWHEN ID tTHEN expr  */
#line 532 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new WhenNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* stmtlist: %empty  */
#line 534 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3219 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* nonemptystmtlist: stmtlistline  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3230 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* nonemptystmtlist: stmtlist stmtlistline  */
#line 545 "engines/director/lingo/lingo-gr.y"
                                                {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3240 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* stmtlistline: '\n'  */
#line 552 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3246 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* simpleexpr_nounarymath: tINT  */
#line 559 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* simpleexpr_nounarymath: tFLOAT  */
#line 560 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3258 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* simpleexpr_nounarymath: tSYMBOL  */
#line 561 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3264 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* simpleexpr_nounarymath: tSTRING  */
#line 562 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 563 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* simpleexpr_nounarymath: ID '(' exprlist ')'  */
#line 564 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new FuncNode((yyvsp[-3].s), (yyvsp[-1].nodelist)); }
#line 3282 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 565 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3288 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* var: ID  */
#line 573 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3294 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* chunk: tFIELD simpleexpr  */
#line 583 "engines/director/lingo/lingo-gr.y"
                                {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("field"), args); }
#line 3303 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* chunk: tCAST simpleexpr  */
#line 587 "engines/director/lingo/lingo-gr.y"
                                        {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("cast"), args); }
#line 3312 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* chunk: tCHAR simpleexpr tOF simpleexpr  */
#line 591 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3319 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* chunk: tCHAR simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 593 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3326 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* chunk: tWORD simpleexpr tOF simpleexpr  */
#line 595 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* chunk: tWORD simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 597 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3340 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* chunk: tITEM simpleexpr tOF simpleexpr  */
#line 599 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3347 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* chunk: tITEM simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 601 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3354 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* chunk: tLINE simpleexpr tOF simpleexpr  */
#line 603 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3361 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* chunk: tLINE simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 605 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3368 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* object: tSCRIPT simpleexpr  */
#line 609 "engines/director/lingo/lingo-gr.y"
                                        {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("script"), args); }
#line 3377 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* object: tWINDOW simpleexpr  */
#line 613 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("window"), args); }
#line 3386 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* the: tTHE tLAST chunktype inof simpleexpr  */
#line 622 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 3392 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 627 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3398 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* theobj: tSOUND simpleexpr  */
#line 628 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3404 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* theobj: tSPRITE simpleexpr  */
#line 629 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3410 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* menu: tMENU simpleexpr  */
#line 632 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3416 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* writablethe: tTHE ID  */
#line 634 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3422 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* writablethe: tTHE ID tOF theobj  */
#line 635 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3428 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* writablethe: tTHE tNUMBER tOF theobj  */
#line 636 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 3434 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 639 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3440 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3446 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* thedatetime: tTHE tABBREV tDATE  */
#line 641 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3452 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* thedatetime: tTHE tABBREV tTIME  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3458 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* thedatetime: tTHE tABBR tDATE  */
#line 643 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3464 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* thedatetime: tTHE tABBR tTIME  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3470 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* thedatetime: tTHE tLONG tDATE  */
#line 645 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 3476 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189: /* thedatetime: tTHE tLONG tTIME  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 3482 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* thedatetime: tTHE tSHORT tDATE  */
#line 647 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 3488 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 191: /* thedatetime: tTHE tSHORT tTIME  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 3494 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 3500 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 653 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 3506 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 194: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 3512 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 655 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 3518 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 656 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 3524 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* chunktype: tCHAR  */
#line 659 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 3530 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* chunktype: tWORD  */
#line 660 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 3536 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* chunktype: tITEM  */
#line 661 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 3542 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* chunktype: tLINE  */
#line 662 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 3548 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* list: '[' exprlist ']'  */
#line 667 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 3554 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* list: '[' ':' ']'  */
#line 668 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 3560 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* list: '[' proplist ']'  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 3566 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* proplist: proppair  */
#line 672 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 3575 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* proplist: proplist ',' proppair  */
#line 676 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3583 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* proppair: tSYMBOL ':' expr  */
#line 681 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3589 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* proppair: ID ':' expr  */
#line 682 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3595 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* proppair: tSTRING ':' expr  */
#line 683 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3601 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* unarymath: '+' simpleexpr  */
#line 686 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 3607 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* unarymath: '-' simpleexpr  */
#line 687 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 3613 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* expr: expr '+' expr  */
#line 697 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3619 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* expr: expr '-' expr  */
#line 698 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* expr: expr '*' expr  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* expr: expr '/' expr  */
#line 700 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 221: /* expr: expr tMOD expr  */
#line 701 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3643 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 222: /* expr: expr '>' expr  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3649 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 223: /* expr: expr '<' expr  */
#line 703 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3655 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 224: /* expr: expr tEQ expr  */
#line 704 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3661 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 225: /* expr: expr tNEQ expr  */
#line 705 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3667 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 226: /* expr: expr tGE expr  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3673 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* expr: expr tLE expr  */
#line 707 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3679 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* expr: expr tAND expr  */
#line 708 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3685 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 229: /* expr: expr tOR expr  */
#line 709 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 230: /* expr: expr '&' expr  */
#line 710 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3697 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* expr: expr tCONCAT expr  */
#line 711 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3703 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* expr: expr tCONTAINS expr  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* expr: expr tSTARTS expr  */
#line 713 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3715 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 722 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3721 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 723 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3727 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 724 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3733 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 725 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3739 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 726 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3745 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 727 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3751 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 728 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3757 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 729 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3763 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 730 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3769 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 731 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3775 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 732 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3781 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 733 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3787 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3793 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 735 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3799 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 736 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3805 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 737 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3811 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 738 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3817 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3823 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3829 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* exprlist: %empty  */
#line 745 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 3835 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* nonemptyexprlist: expr  */
#line 749 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 3844 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 753 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3852 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3856 "engines/director/lingo/lingo-gr.cpp"

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

#line 758 "engines/director/lingo/lingo-gr.y"


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
