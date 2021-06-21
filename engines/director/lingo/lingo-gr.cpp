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
  YYSYMBOL_CMDID = 116,                    /* CMDID  */
  YYSYMBOL_ID = 117,                       /* ID  */
  YYSYMBOL_idlist = 118,                   /* idlist  */
  YYSYMBOL_nonemptyidlist = 119,           /* nonemptyidlist  */
  YYSYMBOL_stmt = 120,                     /* stmt  */
  YYSYMBOL_stmtoneliner = 121,             /* stmtoneliner  */
  YYSYMBOL_proc = 122,                     /* proc  */
  YYSYMBOL_cmdargs = 123,                  /* cmdargs  */
  YYSYMBOL_frameargs = 124,                /* frameargs  */
  YYSYMBOL_asgn = 125,                     /* asgn  */
  YYSYMBOL_to = 126,                       /* to  */
  YYSYMBOL_definevars = 127,               /* definevars  */
  YYSYMBOL_ifstmt = 128,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 129,               /* ifelsestmt  */
  YYSYMBOL_loop = 130,                     /* loop  */
  YYSYMBOL_tell = 131,                     /* tell  */
  YYSYMBOL_when = 132,                     /* when  */
  YYSYMBOL_stmtlist = 133,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 134,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 135,             /* stmtlistline  */
  YYSYMBOL_simpleexpr_nounarymath = 136,   /* simpleexpr_nounarymath  */
  YYSYMBOL_var = 137,                      /* var  */
  YYSYMBOL_varorchunk = 138,               /* varorchunk  */
  YYSYMBOL_varorthe = 139,                 /* varorthe  */
  YYSYMBOL_chunk = 140,                    /* chunk  */
  YYSYMBOL_object = 141,                   /* object  */
  YYSYMBOL_the = 142,                      /* the  */
  YYSYMBOL_theobj = 143,                   /* theobj  */
  YYSYMBOL_menu = 144,                     /* menu  */
  YYSYMBOL_writablethe = 145,              /* writablethe  */
  YYSYMBOL_thedatetime = 146,              /* thedatetime  */
  YYSYMBOL_thenumberof = 147,              /* thenumberof  */
  YYSYMBOL_chunktype = 148,                /* chunktype  */
  YYSYMBOL_inof = 149,                     /* inof  */
  YYSYMBOL_list = 150,                     /* list  */
  YYSYMBOL_proplist = 151,                 /* proplist  */
  YYSYMBOL_proppair = 152,                 /* proppair  */
  YYSYMBOL_unarymath = 153,                /* unarymath  */
  YYSYMBOL_simpleexpr = 154,               /* simpleexpr  */
  YYSYMBOL_expr = 155,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 156,         /* expr_nounarymath  */
  YYSYMBOL_sprite = 157,                   /* sprite  */
  YYSYMBOL_exprlist = 158,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 159          /* nonemptyexprlist  */
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
#define YYFINAL  174
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4223

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  56
/* YYNRULES -- Number of rules.  */
#define YYNRULES  278
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  520

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
     278,   282,   283,   284,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   366,   367,   370,   374,   384,
     385,   386,   387,   388,   389,   392,   393,   394,   397,   398,
     399,   400,   401,   402,   403,   404,   409,   410,   411,   412,
     415,   418,   423,   427,   433,   438,   441,   451,   456,   462,
     468,   474,   482,   483,   484,   485,   488,   488,   490,   491,
     492,   495,   499,   503,   509,   513,   517,   521,   523,   525,
     527,   531,   535,   539,   541,   542,   546,   552,   559,   560,
     566,   567,   568,   569,   570,   571,   572,   573,   574,   575,
     576,   577,   580,   582,   583,   586,   587,   590,   594,   598,
     600,   602,   604,   606,   608,   610,   612,   616,   620,   626,
     627,   628,   629,   632,   633,   634,   635,   636,   639,   641,
     642,   643,   646,   647,   648,   649,   650,   651,   652,   653,
     654,   655,   659,   660,   661,   662,   663,   666,   667,   668,
     669,   672,   672,   674,   675,   676,   679,   683,   688,   689,
     690,   693,   694,   697,   698,   702,   703,   704,   705,   706,
     707,   708,   709,   710,   711,   712,   713,   714,   715,   716,
     717,   718,   719,   720,   727,   728,   729,   730,   731,   732,
     733,   734,   735,   736,   737,   738,   739,   740,   741,   742,
     743,   744,   745,   748,   749,   752,   753,   756,   760
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
  "nonemptymethodlist", "methodlistline", "handler", "endargdef", "CMDID",
  "ID", "idlist", "nonemptyidlist", "stmt", "stmtoneliner", "proc",
  "cmdargs", "frameargs", "asgn", "to", "definevars", "ifstmt",
  "ifelsestmt", "loop", "tell", "when", "stmtlist", "nonemptystmtlist",
  "stmtlistline", "simpleexpr_nounarymath", "var", "varorchunk",
  "varorthe", "chunk", "object", "the", "theobj", "menu", "writablethe",
  "thedatetime", "thenumberof", "chunktype", "inof", "list", "proplist",
  "proppair", "unarymath", "simpleexpr", "expr", "expr_nounarymath",
  "sprite", "exprlist", "nonemptyexprlist", YY_NULLPTR
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

#define YYPACT_NINF (-347)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2289,  -347,  -347,  -347,  -347,  -347,  -347,   -21,  -347,  3506,
    1422,  1519,  -347,  -347,  3506,  -347,    -4,  -347,  -347,  1616,
      72,  3585,  -347,  3506,  -347,  -347,  3506,  1616,  1422,  3506,
    -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,
    -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,
    -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,
    -347,  -347,  -347,  1519,  3506,  3506,  3900,  -347,    30,  2289,
    -347,  -347,  -347,  -347,  1616,  -347,  -347,  -347,  -347,  -347,
    -347,  -347,  -347,  -347,  -347,   -50,  -347,  -347,  -347,  -347,
    -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,
    -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,
    -347,   -14,     5,  -347,  -347,  -347,  -347,  1713,  1713,  1713,
    1713,  1519,  1519,  1713,  1713,  1713,  1713,  1713,  3664,  1519,
    1713,  1713,  1325,   741,     7,    14,    16,  -347,  -347,  -347,
    -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,   840,  -347,
    1519,   518,  3506,    24,    26,   937,  3506,  1519,  3743,  -347,
    -347,   -15,  -347,    93,    34,    36,  1034,    38,    41,    42,
     429,    43,  3506,  -347,  -347,  -347,    44,  1131,  -347,  -347,
    3506,  -347,  -347,  -347,  -347,  1228,  4115,  -347,    40,    51,
      54,    67,   -38,   -36,   -35,     8,    11,    37,   106,   109,
    4061,  -347,  -347,  -347,   531,    39,    45,    47,     1,    12,
    -347,  4115,    52,    49,  1519,  -347,  -347,   117,  1519,  1519,
    1519,  1519,  2004,  2004,  2198,  1519,  2101,  2101,  1519,  1519,
    1519,  1519,  1519,  1519,  1519,  1519,  -347,   629,  -347,   544,
    3096,  1519,  1519,  1519,  1519,  1519,    48,  -347,  -347,  3822,
    3822,  3822,   -12,   642,   125,  -347,  -347,  1519,  1519,   -71,
    -347,  1519,  -347,  -347,  -347,  3900,  3176,  -347,    60,  -347,
    -347,   132,  4128,  1713,  1713,  1713,  1713,  1713,  1713,  1713,
    1713,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,
    -347,  -347,  -347,  -347,  -347,    79,  1810,  1907,  1713,  1713,
    1519,  -347,  1519,  1519,  -347,  1519,  3427,  -347,  -347,  1519,
      61,  1519,   -31,   -31,   -31,   -31,   184,   184,  -347,   -42,
     -31,   -31,   -31,   -31,   -42,   -40,   -40,  -347,  -347,    49,
    1519,  1519,  1519,  1519,  1519,  1519,  1519,  1519,  1519,  1519,
    1519,  1519,  1519,  1519,  1519,  1519,  1519,  1519,  3176,   144,
    3176,  -347,    65,  -347,    66,    68,  1519,  1519,  3176,  1907,
    3947,  4115,  3506,  -347,  -347,    71,  -347,  -347,  3960,  -347,
    -347,  -347,  2449,  -347,  -347,  3176,  1519,  -347,   142,  -347,
     143,  -347,   145,  -347,   148,  -347,  -347,  1713,    79,    79,
      79,    79,  1713,  1713,    79,  1713,  1713,  -347,  -347,  -347,
    -347,  -347,  -347,    -7,  4115,  4115,  4115,    39,    45,    69,
    -347,  4115,  -347,  4115,   -31,   -31,   -31,   -31,   184,   184,
    -347,   -42,   -31,   -31,   -31,   -31,   -42,   -40,   -40,  -347,
    -347,    49,  2369,  3256,  3176,  -347,  -347,  -347,  3973,   164,
    2536,  -347,  3506,   -71,  -347,    81,  -347,  2616,  4115,  1713,
    1713,  1713,  1713,  -347,  1713,  1713,  1713,  1713,  -347,   153,
     111,  -347,  -347,  -347,  3336,    88,  3176,  -347,  3176,   152,
    1519,    90,    91,  -347,  -347,  3506,  -347,  -347,  -347,  -347,
    -347,  -347,  -347,  -347,   118,  1713,  -347,  3176,  -347,  -347,
    2696,  2776,  1519,  4029,  -347,  3176,    18,  -347,  1713,  2856,
      95,    96,  4042,  3176,  3176,  -347,  3506,  -347,    98,  -347,
    -347,  3176,  2936,  -347,  -347,  3016,    99,   100,  -347,  -347
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    24,    31,    37,    60,    72,    36,     0,    38,    95,
     120,     0,    39,    41,     0,    52,    53,    56,    58,   120,
      59,    62,    70,     0,    73,    71,     0,   120,   120,    95,
      29,    57,    54,    51,    28,    30,    34,    66,    32,    33,
      42,    43,    45,    46,    75,    76,    25,    26,    27,    47,
      63,    35,    44,    48,    49,    50,    55,    68,    69,    64,
      65,    40,    74,    67,    95,     0,    61,     5,     0,     2,
       3,     6,     7,     8,   120,     9,    99,   105,   106,   107,
     100,   101,   102,   103,   104,     0,   118,    79,    78,    80,
      82,    83,    84,    86,    53,    92,    59,    62,    93,    94,
      81,    89,    90,    85,    67,    91,    88,    87,    61,    77,
      97,     0,    96,   160,   161,   163,   162,    31,    37,    60,
      72,    38,    52,    54,    32,    42,    45,    75,    68,    65,
       0,     0,     0,   275,   172,     0,     0,   233,   167,   168,
     169,   170,   189,   190,   191,   171,   234,   235,   121,   236,
       0,     0,    95,     0,     0,   121,     0,     0,    68,   172,
     175,     0,   176,     0,     0,     0,   121,     0,     0,     0,
       0,     0,    95,   119,     1,     4,     0,   121,   117,   138,
       0,   178,   177,   187,   188,   127,   128,   164,     0,     0,
       0,     0,    25,    26,    27,    47,    63,    44,    55,   199,
       0,   231,   232,   125,     0,   163,   162,     0,   172,     0,
     226,   277,     0,   276,   275,   110,   111,    56,     0,     0,
       0,     0,    29,    57,    51,     0,    34,    66,     0,     0,
       0,     0,     0,     0,     0,     0,   254,   123,   255,     0,
       0,     0,     0,     0,     0,     0,     0,   116,   109,    41,
      28,    30,     0,     0,    55,   136,   137,     0,     0,    13,
     114,    73,   112,   113,   140,     0,   154,   139,     0,   108,
      98,    56,   131,     0,     0,     0,     0,     0,     0,     0,
       0,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   217,   219,   220,   218,     0,     0,     0,     0,     0,
       0,   166,     0,     0,   224,     0,     0,   225,   223,     0,
       0,     0,   246,   247,   244,   245,   248,   249,   241,   251,
     252,   253,   243,   242,   250,   237,   238,   239,   240,   122,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   154,   141,
     154,   173,     0,   174,     0,     0,     0,     0,   154,     0,
       0,   153,     0,    17,    18,    11,    14,    15,     0,   151,
     158,   159,     0,   155,   156,   154,     0,   179,     0,   183,
       0,   185,     0,   181,     0,   221,   222,     0,    33,    43,
      46,    76,    48,    49,    50,    64,    65,   201,   194,   193,
     200,   273,   274,     0,   230,   228,   229,     0,     0,     0,
     227,   278,   165,   130,   265,   266,   263,   264,   267,   268,
     260,   270,   271,   272,   262,   261,   269,   256,   257,   258,
     259,   124,     0,     0,    10,   132,   133,   134,     0,     0,
       0,   135,    95,     0,   115,     0,   157,    20,   129,     0,
       0,     0,     0,   192,     0,     0,     0,     0,   198,     0,
       0,   196,   197,   126,     0,     0,   154,   143,   154,     0,
       0,     0,     0,    16,   152,    21,   180,   184,   186,   182,
     212,   214,   215,   213,     0,     0,   216,   154,   145,   142,
       0,     0,     0,     0,   147,   154,     0,    22,     0,     0,
       0,     0,     0,   154,    12,    19,     0,   195,     0,   144,
     150,   154,     0,    23,   146,     0,     0,     0,   148,   149
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -347,  -347,  -347,   121,  -347,  -347,  -347,  -347,  -347,  -244,
    -347,  -347,     0,    -6,   -28,  -347,     6,   -62,  -347,    -3,
     172,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -347,  -336,
    -347,  -335,  -120,   -19,  -122,  -347,  -131,  -347,  -347,   -96,
    -258,   183,  -347,  -347,  -347,  -346,  -347,  -347,  -101,  -347,
     260,    62,    29,   -99,     9,  -230
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    68,    69,    70,    71,    72,   364,   365,   366,   367,
      73,   496,   109,   134,   111,   112,   371,    76,    77,   135,
     136,    78,   257,    79,    80,    81,    82,    83,    84,   372,
     373,   374,   137,   138,   352,   161,   139,   140,   141,   397,
     398,   142,   143,   144,   295,   387,   145,   209,   210,   146,
     147,   211,   237,   149,   212,   213
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      74,   169,   160,   110,   173,   329,    75,   243,   152,   243,
      85,   356,   432,   362,   434,   159,   154,   163,   243,   255,
     164,   225,   440,   110,   165,   167,   363,   153,   236,   256,
     174,   281,   357,   283,   285,   236,   171,   446,   282,   447,
     284,   286,   454,   455,   456,   457,   236,   178,   460,   238,
     231,   232,   233,   234,   233,   234,   238,   236,   110,   172,
     230,   231,   232,   233,   234,   236,    74,   238,   273,    74,
     403,   176,   148,   151,   274,    75,    86,   287,   238,   275,
     289,   155,   277,   179,   288,   276,   238,   290,   278,   166,
     148,   309,   291,   463,   292,   279,   293,   446,   294,   446,
     214,   280,   385,   180,   305,   446,   214,   386,   156,   157,
     306,   215,   446,   216,   307,   505,   506,   431,   353,   353,
     353,   247,   199,   248,   246,   170,   258,   208,   354,   355,
     490,   259,   491,   260,   296,   262,   177,   297,   263,   264,
     267,   269,   302,   311,   268,   350,   110,   309,   303,   304,
     252,   499,   199,   359,   308,   446,   446,   375,   376,   504,
     433,   412,   435,   436,   446,   437,   110,   512,   443,   446,
     449,   450,   305,   451,   270,   515,   452,   446,   474,   469,
     446,   484,   485,   185,   186,   489,   492,   494,   495,   498,
     175,   200,   509,   510,   204,   514,   518,   519,   470,   473,
     168,   400,   486,   369,   162,   410,   218,   219,   220,   221,
     241,   242,   239,   243,   272,     0,   225,   244,   245,   253,
       0,     0,     0,   310,     0,     0,   218,   219,   220,   221,
     351,   351,   351,   243,     0,     0,   225,   244,   245,     0,
      74,     0,     0,   159,   159,   159,   349,     0,     0,     0,
       0,     0,     0,   228,   229,   230,   231,   232,   233,   234,
       0,     0,     0,     0,     0,    74,    74,     0,     0,     0,
       0,     0,     0,   228,   229,   230,   231,   232,   233,   234,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,     0,     0,     0,
     409,     0,     0,   316,   317,   318,   320,   321,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   360,
     361,     0,     0,   368,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,     0,
      74,     0,     0,     0,     0,     0,   442,     0,    74,     0,
       0,     0,     0,     0,   404,   405,     0,   406,     0,     0,
       0,   411,    74,   413,     0,    74,     0,   181,   182,   183,
     184,     0,     0,   187,   188,   189,   190,   191,     0,     0,
     201,   202,   414,   415,   416,   417,   418,   419,   420,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,     0,
       0,     0,     0,     0,   472,     0,     0,     0,   438,   439,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    74,    74,     0,   110,     0,   448,   467,
      74,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   265,    74,     0,    74,     0,    74,   497,
     488,   218,   219,   220,   221,   241,   242,     0,   243,     0,
       0,   225,   244,   245,     0,     0,     0,    74,     0,     0,
      74,    74,     0,     0,     0,    74,     0,     0,     0,    74,
     513,     0,     0,    74,    74,     0,     0,     0,     0,     0,
       0,    74,    74,     0,     0,    74,     0,     0,   228,   229,
     230,   231,   232,   233,   234,     0,   266,     0,     0,     0,
       0,     0,   493,   377,   378,   379,   380,   381,   382,   383,
     384,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   240,     0,     0,   502,     0,   399,   399,   401,   402,
     218,   219,   220,   221,   241,   242,     0,   243,     0,     0,
     225,   244,   245,   218,   219,   220,   221,   241,   242,     0,
     243,     0,     0,   225,   244,   245,   218,   219,   220,   221,
     241,   242,     0,   243,     0,     0,   225,   244,   245,     0,
       0,     0,     0,     0,     0,     0,     0,   228,   229,   230,
     231,   232,   233,   234,     0,     0,     0,     0,     0,   399,
     228,   229,   230,   231,   232,   233,   234,     0,     0,   300,
       0,   301,     0,   228,   229,   230,   231,   232,   233,   234,
       0,     0,     0,     0,   301,     0,     0,   453,     0,     0,
       0,     0,   458,   459,     0,   461,   462,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   330,   331,   332,   333,   334,   335,     0,   336,     0,
       0,   337,   338,   339,   218,   219,   220,   221,   241,   242,
       0,   243,     0,     0,   225,   244,   245,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   476,
     477,   478,   479,     0,   480,   481,   482,   483,   340,   341,
     342,   343,   344,   345,   346,     0,     0,   347,     0,     0,
       0,   228,   229,   230,   231,   232,   233,   234,     0,   358,
       0,     0,     0,     0,     0,   458,   113,   114,     1,   205,
     206,    87,   117,   118,   119,   120,     6,    88,   507,    89,
       8,    90,    91,    92,    12,    13,    93,    15,    94,    17,
      18,    95,    96,    97,    98,    22,    99,    24,    25,   100,
     101,   102,   103,     0,     0,     0,     0,    30,    31,   123,
      33,    34,    35,     0,    36,    37,   124,    39,   125,    41,
     126,    43,   127,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,    54,    55,    56,   128,    58,    59,   129,
      61,    62,   104,   105,   106,   107,     0,     0,     0,   108,
       0,     0,     0,   130,   131,     0,     0,     0,     0,     0,
     150,     0,   133,     0,   207,   113,   114,     1,   115,   116,
      87,   117,   118,   119,   120,     6,    88,     0,    89,     8,
      90,    91,    92,    12,    13,    93,    15,    94,   217,    18,
      95,    96,    97,    98,    22,    99,    24,    25,   100,   101,
     102,   103,   218,   219,   220,   221,   222,   223,   123,   224,
      34,    35,   225,   226,   227,   124,    39,   125,    41,   126,
      43,   127,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,   128,    58,    59,   129,    61,
      62,   104,   105,   106,   107,     0,     0,     0,   108,   228,
     229,   230,   231,   232,   233,   234,     0,     0,   235,   150,
       0,   133,   113,   114,     1,   115,   116,    87,   117,   118,
     119,   120,     6,    88,     0,    89,     8,    90,    91,    92,
      12,   249,    93,    15,    94,    17,    18,    95,    96,    97,
      98,    22,    99,    24,    25,   100,   101,   102,   103,   218,
     219,   220,   221,   222,   223,   123,   224,   250,   251,   225,
     226,   227,   124,    39,   125,    41,   126,    43,   127,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,    54,
      55,    56,   128,    58,    59,   129,    61,    62,   104,   105,
     106,   107,     0,     0,     0,   108,   228,   229,   230,   231,
     232,   233,   234,     0,     0,   235,   150,     0,   133,   113,
     114,     1,   115,   116,    87,   117,   118,   119,   120,     6,
      88,     0,    89,     8,    90,    91,    92,    12,    13,    93,
      15,    94,    17,    18,    95,    96,    97,    98,    22,    99,
     261,    25,   100,   101,   102,   103,   218,   219,   220,   221,
     222,   223,   123,   224,    34,    35,   225,   226,   227,   124,
      39,   125,    41,   126,    43,   127,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,   128,
      58,    59,   129,    61,    62,   104,   105,   106,   107,     0,
       0,     0,   108,   228,   229,   230,   231,   232,   233,   234,
       0,     0,   235,   150,     0,   133,   113,   114,     1,   115,
     116,    87,   117,   118,   119,   120,     6,    88,     0,    89,
       8,    90,    91,    92,    12,    13,    93,    15,    94,    17,
      18,    95,    96,    97,    98,    22,    99,    24,    25,   100,
     101,   102,   103,   218,   219,   220,   221,   222,   223,   123,
     224,    34,    35,   225,   226,   227,   124,    39,   125,    41,
     126,    43,   127,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,    54,    55,    56,   128,    58,    59,   129,
      61,    62,   104,   105,   106,   107,     0,     0,     0,   108,
     228,   229,   230,   231,   232,   233,   234,     0,     0,   235,
     150,     0,   133,   113,   114,     1,   115,   116,    87,   117,
     118,   119,   120,     6,    88,     0,    89,     8,    90,    91,
      92,    12,    13,    93,    15,    94,   271,    18,    95,    96,
      97,    98,    22,    99,    24,    25,   100,   101,   102,   103,
     218,   219,   220,   221,   222,   223,   123,   224,    34,    35,
     225,   226,   227,   124,    39,   125,    41,   126,    43,   127,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,    56,   128,    58,    59,   129,    61,    62,   104,
     105,   106,   107,     0,     0,     0,   108,   228,   229,   230,
     231,   232,   233,   234,     0,     0,     0,   150,     0,   133,
     113,   114,     1,   115,   116,    87,   117,   118,   119,   120,
       6,    88,     0,    89,     8,    90,    91,    92,    12,    13,
      93,    15,    94,    17,    18,    95,    96,    97,    98,    22,
      99,    24,    25,   100,   101,   102,   103,     0,     0,     0,
       0,    30,    31,   123,    33,    34,    35,     0,    36,    37,
     124,    39,   125,    41,   126,    43,   127,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,    56,
     128,    58,    59,   129,    61,    62,   104,   105,   106,   107,
       0,     0,     0,   108,     0,     0,     0,   130,   131,     0,
       0,     0,     0,     0,   150,   203,   133,   113,   114,     1,
     115,   116,    87,   117,   118,   119,   120,     6,    88,     0,
      89,   121,    90,    91,    92,    12,    13,    93,   122,    94,
      17,    18,    95,    96,    97,    98,    22,    99,    24,    25,
     100,   101,   102,   103,     0,     0,     0,     0,    30,    31,
     123,    33,    34,    35,     0,    36,    37,   124,    39,   125,
      41,   126,    43,   127,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,    56,   128,    58,    59,
     129,    61,    62,   104,   105,   106,   107,     0,     0,     0,
     108,     0,     0,     0,   130,   131,     0,     0,     0,     0,
       0,   132,     0,   133,   113,   114,     1,   115,   116,    87,
     117,   118,   119,   120,     6,    88,     0,    89,     8,    90,
      91,    92,    12,    13,    93,    15,    94,    17,    18,    95,
      96,    97,    98,    22,    99,    24,    25,   100,   101,   102,
     103,     0,     0,     0,     0,    30,    31,   123,    33,    34,
      35,     0,    36,    37,   124,    39,   125,    41,   126,    43,
     127,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,    54,    55,    56,   128,    58,    59,   129,    61,    62,
     104,   105,   106,   107,     0,     0,     0,   108,     0,     0,
       0,   130,   131,     0,     0,     0,     0,     0,   150,     0,
     133,   113,   114,     1,   115,   116,    87,   117,   118,   119,
     120,     6,    88,     0,    89,     8,    90,    91,    92,    12,
      13,    93,    15,    94,    17,    18,    95,    96,    97,    98,
      22,    99,    24,    25,   100,   101,   102,   103,     0,     0,
       0,     0,    30,    31,   123,    33,    34,    35,     0,    36,
      37,   124,    39,   125,    41,   126,    43,   127,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,   128,    58,    59,   129,    61,    62,   104,   105,   106,
     107,     0,     0,     0,   108,     0,     0,     0,   130,   131,
       0,     0,     0,     0,     0,   132,     0,   133,   113,   114,
       1,   115,   116,    87,   117,   118,   119,   120,     6,    88,
       0,    89,     8,    90,    91,    92,    12,    13,    93,    15,
      94,    17,    18,    95,    96,    97,    98,    22,    99,    24,
      25,   100,   101,   102,   103,     0,     0,     0,     0,    30,
      31,   123,    33,    34,    35,     0,    36,    37,   124,    39,
     125,    41,   126,    43,   127,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,   128,    58,
      59,    60,    61,    62,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   130,   131,     0,     0,     0,
       0,     0,   150,     0,   133,   113,   114,     1,   115,   116,
      87,   117,   118,   119,   120,     6,    88,     0,    89,     8,
      90,    91,    92,    12,    13,    93,    15,    94,    17,    18,
      95,    96,    97,    98,    22,    99,    24,    25,   100,   101,
     102,   103,     0,     0,     0,     0,    30,    31,   123,    33,
      34,    35,     0,    36,    37,   124,   388,   125,   389,   126,
     390,   127,   391,    46,    47,    48,    49,    50,     0,    51,
      52,   392,   393,   394,    56,   128,    58,   395,   396,    61,
      62,   104,   105,   106,   107,     0,     0,     0,   108,     0,
       0,     0,   130,   131,     0,     0,     0,     0,     0,   150,
       0,   133,   113,   114,     1,   115,   116,    87,   117,   118,
     119,   120,     6,    88,     0,    89,     8,    90,    91,    92,
      12,    13,    93,    15,    94,    17,    18,    95,    96,    97,
      98,    22,    99,    24,    25,   100,   101,   102,   103,     0,
       0,     0,     0,    30,    31,   123,    33,    34,    35,     0,
      36,    37,   124,    39,   125,    41,   126,    43,   127,    45,
      46,    47,    48,    49,    50,     0,    51,    52,   392,   393,
      55,    56,   128,    58,   395,   396,    61,    62,   104,   105,
     106,   107,     0,     0,     0,   108,     0,     0,     0,   130,
     131,     0,     0,     0,     0,     0,   150,     0,   133,   113,
     114,     1,   115,   116,    87,   117,   118,   119,   120,     6,
      88,     0,    89,     8,    90,    91,    92,    12,    13,    93,
      15,    94,    17,    18,    95,    96,    97,    98,    22,    99,
      24,    25,   100,   101,   102,   103,     0,     0,     0,     0,
       0,     0,   123,    33,    34,    35,     0,    36,    37,   124,
      39,   125,    41,   126,    43,   127,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,   128,
      58,    59,   129,    61,    62,   104,   105,   106,   107,     0,
       0,     0,   108,     0,     0,     0,   130,   131,     0,     0,
       0,     0,     0,   150,     0,   133,   113,   114,     1,   115,
     116,    87,   117,   118,   119,   120,     6,    88,     0,    89,
       8,    90,    91,    92,    12,    13,    93,    15,    94,    17,
      18,    95,    96,    97,    98,    22,    99,    24,    25,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   123,
      33,    34,    35,     0,     0,     0,   124,    39,   125,    41,
     126,    43,   127,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,    54,    55,    56,   128,    58,    59,   129,
      61,    62,   104,   105,   106,   107,     0,     0,     0,   108,
       0,     0,     0,   130,   131,     0,     0,     0,     0,     0,
     150,     0,   133,   113,   114,     1,   115,   116,    87,   117,
     118,   119,   120,     6,    88,     0,    89,     8,    90,    91,
      92,    12,    13,    93,    15,    94,    17,    18,    95,    96,
      97,    98,    22,    99,    24,    25,   100,   101,   102,   103,
       0,     0,     0,     0,     0,     0,   123,     0,    34,    35,
       0,     0,     0,   124,    39,   125,    41,   126,    43,   127,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,    56,   128,    58,    59,   129,    61,    62,   104,
     105,   106,   107,     0,     0,     0,   108,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     1,   150,     0,   133,
       2,     3,     4,     5,     6,     0,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,     0,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    30,    31,    32,    33,    34,
      35,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,     0,     0,     0,     1,    66,     0,     0,
       2,     3,     4,     5,     6,   464,    67,     7,     8,     9,
      10,    11,    12,    13,     0,    15,    16,    17,    18,    19,
      20,    21,     0,    22,    23,    24,    25,     0,    27,    28,
      29,     0,     0,     0,     0,    30,    31,    32,    33,    34,
      35,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,     0,     0,   465,     0,     1,    66,     0,     0,
       2,     3,     4,     5,     6,     0,   370,     7,     8,     9,
      10,    11,    12,    13,     0,    15,    16,    17,    18,    19,
      20,    21,     0,    22,    23,    24,    25,     0,    27,    28,
      29,     0,     0,     0,     0,    30,    31,    32,    33,    34,
      35,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,     0,     0,     0,     0,   445,    66,     0,     0,
       0,     0,     0,     1,     0,     0,   370,     2,     3,     4,
       5,     6,     0,     0,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,     0,   471,     1,    66,     0,   475,     2,     3,     4,
       5,     6,     0,   370,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,     0,     0,     1,    66,     0,     0,     2,     3,     4,
       5,     6,     0,   370,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,   500,     0,     1,    66,     0,     0,     2,     3,     4,
       5,     6,     0,   370,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,     0,   501,     1,    66,     0,     0,     2,     3,     4,
       5,     6,     0,   370,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,   508,     0,     1,    66,     0,     0,     2,     3,     4,
       5,     6,     0,   370,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,     0,   516,     1,    66,     0,     0,     2,     3,     4,
       5,     6,     0,   370,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,     0,   517,     1,    66,     0,     0,     2,     3,     4,
       5,     6,     0,   370,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,     0,     0,     1,    66,     0,     0,     2,     3,     4,
       5,     6,     0,   348,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,     0,     0,     1,    66,     0,     0,     2,     3,     4,
       5,     6,     0,   370,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,     0,     0,     1,    66,     0,     0,     2,     3,     4,
       5,     6,     0,   466,     7,     8,     9,    10,    11,    12,
      13,     0,    15,    16,    17,    18,    19,    20,    21,     0,
      22,    23,    24,    25,     0,    27,    28,    29,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
       0,     0,     0,     0,    66,     0,     0,     0,     0,     0,
       0,     0,     0,   487,     1,   407,   408,    87,     2,     3,
       4,     5,     6,    88,     0,    89,     8,    90,    91,    92,
      12,    13,    93,    15,    94,    17,    18,    95,    96,    97,
      98,    22,    99,    24,    25,   100,   101,   102,   103,     0,
       0,     0,     0,    30,    31,    32,    33,    34,    35,     0,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,   104,   105,
     106,   107,     0,     1,     0,   108,    87,     2,     3,     4,
       5,     6,    88,     0,    89,     8,    90,    91,    92,    12,
      13,    93,    15,    94,    17,    18,    95,    96,    97,    98,
      22,    99,    24,    25,   100,   101,   102,   103,     0,     0,
       0,     0,    30,    31,    32,    33,    34,    35,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,   104,   105,   106,
     107,     0,     1,     0,   108,    87,     2,     3,     4,     5,
       6,    88,     0,    89,     8,    90,    91,    92,    12,    13,
      93,    15,    94,    17,    18,    95,    96,    97,    98,    22,
      99,    24,    25,   100,   101,   102,   103,     0,     0,     0,
       0,    30,    31,    32,    33,    34,    35,     0,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,    56,
     158,    58,    59,    60,    61,    62,   104,   105,   106,   107,
       0,     1,     0,   108,    87,     2,     3,     4,     5,     6,
      88,     0,    89,     8,    90,    91,    92,    12,    13,    93,
      15,    94,    17,    18,    95,    96,    97,    98,    22,    99,
      24,    25,   100,   101,   102,   103,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,   192,   193,   194,
     195,   196,     0,    51,   197,    53,    54,    55,   198,    57,
      58,    59,    60,    61,    62,   104,   105,   106,   107,     0,
       1,     0,   108,    87,     2,     3,     4,     5,     6,    88,
       0,    89,     8,    90,    91,    92,    12,    13,    93,    15,
      94,    17,    18,    95,    96,    97,    98,    22,    99,    24,
      25,   100,   101,   102,   103,     0,     0,     0,     0,    30,
      31,    32,    33,    34,    35,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,   254,    57,    58,
      59,    60,    61,    62,   104,   105,   106,   107,     0,     1,
       0,   108,    87,   117,   118,     4,     5,     6,    88,     0,
      89,     8,    90,    91,    92,    12,    13,    93,    15,    94,
      17,    18,    95,    96,    97,    98,    22,    99,    24,    25,
     100,   101,   102,   103,     0,     0,     0,     0,    30,    31,
      32,    33,    34,    35,     0,    36,    37,   124,    39,   125,
      41,   126,    43,   127,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,   104,   105,   106,   107,     1,     0,     0,
     108,     2,     3,     4,     5,     6,     0,     0,     7,     8,
       9,    10,     0,    12,    13,     0,    15,    16,    17,    18,
      19,    96,    21,     0,    22,     0,    24,    25,     0,    27,
      28,    29,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,   104,    64,     0,     0,     0,     0,     0,    66,   218,
     219,   220,   221,   241,   242,     0,   243,     0,     0,   225,
     244,   245,   218,   219,   220,   221,   241,   242,     0,   243,
       0,     0,   225,   244,   245,   218,   219,   220,   221,   241,
     242,     0,   243,     0,     0,   225,   244,   245,     0,     0,
       0,     0,     0,     0,     0,     0,   228,   229,   230,   231,
     232,   233,   234,     0,   441,     0,     0,     0,     0,   228,
     229,   230,   231,   232,   233,   234,     0,   444,     0,     0,
       0,     0,   228,   229,   230,   231,   232,   233,   234,     0,
     468,   218,   219,   220,   221,   241,   242,     0,   243,     0,
       0,   225,   244,   245,   218,   219,   220,   221,   241,   242,
       0,   243,     0,     0,   225,   244,   245,     0,     0,     0,
       0,     0,     0,   218,   219,   220,   221,   241,   242,     0,
     243,     0,     0,   225,   244,   245,     0,     0,   228,   229,
     230,   231,   232,   233,   234,     0,   503,     0,     0,     0,
       0,   228,   229,   230,   231,   232,   233,   234,     0,   511,
     298,   299,     0,     0,     0,     0,     0,     0,     0,     0,
     228,   229,   230,   231,   232,   233,   234,   218,   219,   220,
     221,   241,   242,     0,   243,     0,     0,   225,   244,   245,
     330,   331,   332,   333,   334,   335,     0,   336,     0,     0,
     337,   338,   339,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   228,   229,   230,   231,   232,   233,
     234,     0,     0,     0,     0,     0,     0,   340,   341,   342,
     343,   344,   345,   346
};

static const yytype_int16 yycheck[] =
{
       0,    29,    21,     9,    66,   235,     0,    49,    14,    49,
      31,    23,   348,    84,   350,    21,    19,    23,    49,    34,
      26,    52,   358,    29,    27,    28,    97,    31,   148,    44,
       0,    69,    44,    69,    69,   155,    64,   372,    76,   375,
      76,    76,   388,   389,   390,   391,   166,    97,   394,   148,
      92,    93,    94,    95,    94,    95,   155,   177,    64,    65,
      91,    92,    93,    94,    95,   185,    66,   166,    28,    69,
     300,    74,    10,    11,    34,    69,    97,    69,   177,    28,
      69,    19,    28,    97,    76,    34,   185,    76,    34,    27,
      28,    98,    55,   100,    57,    28,    59,   432,    61,   434,
      99,    34,    23,    98,   103,   440,    99,    28,    36,    37,
      98,    97,   447,    97,   102,    97,    98,   347,   249,   250,
     251,    97,   128,    97,   152,    63,    33,   133,   250,   251,
     466,    97,   468,    97,    28,    97,    74,    28,    97,    97,
      97,    97,   103,    26,   172,    97,   152,    98,   103,   102,
     156,   487,   158,    28,   102,   490,   491,    97,    26,   495,
      16,   100,    97,    97,   499,    97,   172,   503,    97,   504,
      28,    28,   103,    28,   180,   511,    28,   512,    97,    15,
     515,    28,    71,   121,   122,    97,    34,    97,    97,    71,
      69,   129,    97,    97,   132,    97,    97,    97,    34,   443,
      28,   297,   460,   265,    21,   306,    42,    43,    44,    45,
      46,    47,   150,    49,   185,    -1,    52,    53,    54,   157,
      -1,    -1,    -1,   214,    -1,    -1,    42,    43,    44,    45,
     249,   250,   251,    49,    -1,    -1,    52,    53,    54,    -1,
     240,    -1,    -1,   249,   250,   251,   240,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      -1,    -1,    -1,    -1,    -1,   265,   266,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,    -1,    -1,    -1,
     306,    -1,    -1,   241,   242,   243,   244,   245,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   257,
     258,    -1,    -1,   261,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   348,    -1,
     350,    -1,    -1,    -1,    -1,    -1,   362,    -1,   358,    -1,
      -1,    -1,    -1,    -1,   302,   303,    -1,   305,    -1,    -1,
      -1,   309,   372,   311,    -1,   375,    -1,   117,   118,   119,
     120,    -1,    -1,   123,   124,   125,   126,   127,    -1,    -1,
     130,   131,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,    -1,
      -1,    -1,    -1,    -1,   442,    -1,    -1,    -1,   356,   357,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   432,   433,   434,    -1,   442,    -1,   376,   433,
     440,    -1,    -1,    -1,    -1,    -1,    -1,   447,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    34,   464,    -1,   466,    -1,   468,   475,
     464,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    -1,    -1,    -1,   487,    -1,    -1,
     490,   491,    -1,    -1,    -1,   495,    -1,    -1,    -1,   499,
     506,    -1,    -1,   503,   504,    -1,    -1,    -1,    -1,    -1,
      -1,   511,   512,    -1,    -1,   515,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    -1,    -1,    -1,
      -1,    -1,   470,   273,   274,   275,   276,   277,   278,   279,
     280,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    33,    -1,    -1,   492,    -1,   296,   297,   298,   299,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    52,    53,    54,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,   359,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    98,
      -1,   100,    -1,    89,    90,    91,    92,    93,    94,    95,
      -1,    -1,    -1,    -1,   100,    -1,    -1,   387,    -1,    -1,
      -1,    -1,   392,   393,    -1,   395,   396,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   449,
     450,   451,   452,    -1,   454,   455,   456,   457,    89,    90,
      91,    92,    93,    94,    95,    -1,    -1,    98,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      -1,    -1,    -1,    -1,    -1,   485,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,   498,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    88,
      -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    -1,
      99,    -1,   101,    -1,   103,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    -1,    -1,    98,    99,
      -1,   101,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    95,    -1,    -1,    98,    99,    -1,   101,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    -1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      -1,    -1,    98,    99,    -1,   101,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    98,
      99,    -1,   101,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    -1,    -1,    -1,    99,    -1,   101,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    -1,    -1,    99,   100,   101,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      88,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,
      -1,    99,    -1,   101,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    88,    -1,    -1,
      -1,    92,    93,    -1,    -1,    -1,    -1,    -1,    99,    -1,
     101,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,
      -1,    -1,    -1,    -1,    -1,    99,    -1,   101,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    88,    -1,    -1,    -1,    92,    93,    -1,    -1,    -1,
      -1,    -1,    99,    -1,   101,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    88,    -1,
      -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    -1,    99,
      -1,   101,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    88,    -1,    -1,    -1,    92,
      93,    -1,    -1,    -1,    -1,    -1,    99,    -1,   101,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    -1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    88,    -1,    -1,    -1,    92,    93,    -1,    -1,
      -1,    -1,    -1,    99,    -1,   101,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    88,
      -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    -1,
      99,    -1,   101,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    88,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     7,    99,    -1,   101,
      11,    12,    13,    14,    15,    -1,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    -1,    -1,    -1,     7,    88,    -1,    -1,
      11,    12,    13,    14,    15,    16,    97,    18,    19,    20,
      21,    22,    23,    24,    -1,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    36,    37,    -1,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    -1,    -1,    85,    -1,     7,    88,    -1,    -1,
      11,    12,    13,    14,    15,    -1,    97,    18,    19,    20,
      21,    22,    23,    24,    -1,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    36,    37,    -1,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    -1,    -1,    -1,    -1,    87,    88,    -1,    -1,
      -1,    -1,    -1,     7,    -1,    -1,    97,    11,    12,    13,
      14,    15,    -1,    -1,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    86,     7,    88,    -1,    10,    11,    12,    13,
      14,    15,    -1,    97,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    -1,     7,    88,    -1,    -1,    11,    12,    13,
      14,    15,    -1,    97,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    85,    -1,     7,    88,    -1,    -1,    11,    12,    13,
      14,    15,    -1,    97,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    86,     7,    88,    -1,    -1,    11,    12,    13,
      14,    15,    -1,    97,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    85,    -1,     7,    88,    -1,    -1,    11,    12,    13,
      14,    15,    -1,    97,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    86,     7,    88,    -1,    -1,    11,    12,    13,
      14,    15,    -1,    97,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    86,     7,    88,    -1,    -1,    11,    12,    13,
      14,    15,    -1,    97,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    -1,     7,    88,    -1,    -1,    11,    12,    13,
      14,    15,    -1,    97,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    -1,     7,    88,    -1,    -1,    11,    12,    13,
      14,    15,    -1,    97,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    -1,     7,    88,    -1,    -1,    11,    12,    13,
      14,    15,    -1,    97,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    97,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    -1,     7,    -1,    88,    10,    11,    12,    13,
      14,    15,    16,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    -1,     7,    -1,    88,    10,    11,    12,    13,    14,
      15,    16,    -1,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      -1,     7,    -1,    88,    10,    11,    12,    13,    14,    15,
      16,    -1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    -1,
       7,    -1,    88,    10,    11,    12,    13,    14,    15,    16,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    -1,     7,
      -1,    88,    10,    11,    12,    13,    14,    15,    16,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,     7,    -1,    -1,
      88,    11,    12,    13,    14,    15,    -1,    -1,    18,    19,
      20,    21,    -1,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    -1,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    -1,    -1,    88,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    -1,    97,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    -1,
      97,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    52,    53,    54,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      67,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    88,    97,   105,   106,
     107,   108,   109,   114,   116,   120,   121,   122,   125,   127,
     128,   129,   130,   131,   132,    31,    97,    10,    16,    18,
      20,    21,    22,    25,    27,    30,    31,    32,    33,    35,
      38,    39,    40,    41,    81,    82,    83,    84,    88,   116,
     117,   118,   119,     5,     6,     8,     9,    11,    12,    13,
      14,    19,    26,    48,    55,    57,    59,    61,    75,    78,
      92,    93,    99,   101,   117,   123,   124,   136,   137,   140,
     141,   142,   145,   146,   147,   150,   153,   154,   155,   157,
      99,   155,   117,    31,   123,   155,    36,    37,    75,   117,
     137,   139,   145,   117,   117,   123,   155,   123,   124,   118,
     155,   118,   117,   121,     0,   107,   123,   155,    97,    97,
      98,   154,   154,   154,   154,   155,   155,   154,   154,   154,
     154,   154,    63,    64,    65,    66,    67,    70,    74,   117,
     155,   154,   154,   100,   155,     8,     9,   103,   117,   151,
     152,   155,   158,   159,    99,    97,    97,    28,    42,    43,
      44,    45,    46,    47,    49,    52,    53,    54,    89,    90,
      91,    92,    93,    94,    95,    98,   136,   156,   157,   155,
      33,    46,    47,    49,    53,    54,   118,    97,    97,    24,
      50,    51,   117,   155,    74,    34,    44,   126,    33,    97,
      97,    36,    97,    97,    97,    34,    97,    97,   118,    97,
     117,    28,   156,    28,    34,    28,    34,    28,    34,    28,
      34,    69,    76,    69,    76,    69,    76,    69,    76,    69,
      76,    55,    57,    59,    61,   148,    28,    28,    79,    80,
      98,   100,   103,   103,   102,   103,    98,   102,   102,    98,
     158,    26,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   159,
      42,    43,    44,    45,    46,    47,    49,    52,    53,    54,
      89,    90,    91,    92,    93,    94,    95,    98,    97,   120,
      97,   137,   138,   140,   138,   138,    23,    44,    97,    28,
     155,   155,    84,    97,   110,   111,   112,   113,   155,   121,
      97,   120,   133,   134,   135,    97,    26,   154,   154,   154,
     154,   154,   154,   154,   154,    23,    28,   149,    56,    58,
      60,    62,    71,    72,    73,    77,    78,   143,   144,   154,
     143,   154,   154,   159,   155,   155,   155,     8,     9,   117,
     152,   155,   100,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   159,   133,    16,   133,    97,    97,    97,   155,   155,
     133,    97,   117,    97,    97,    87,   135,   133,   155,    28,
      28,    28,    28,   154,   149,   149,   149,   149,   154,   154,
     149,   154,   154,   100,    16,    85,    97,   120,    97,    15,
      34,    86,   118,   113,    97,    10,   154,   154,   154,   154,
     154,   154,   154,   154,    28,    71,   144,    97,   120,    97,
     133,   133,    34,   155,    97,    97,   115,   117,    71,   133,
      85,    86,   155,    97,   133,    97,    98,   154,    85,    97,
      97,    97,   133,   117,    97,   133,    86,    86,    97,    97
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
     116,   116,   116,   116,   116,   116,   116,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   118,   118,   119,   119,   120,
     120,   120,   120,   120,   120,   121,   121,   121,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     123,   123,   123,   123,   123,   123,   123,   124,   124,   124,
     124,   124,   125,   125,   125,   125,   126,   126,   127,   127,
     127,   128,   128,   129,   129,   129,   129,   130,   130,   130,
     130,   131,   131,   132,   133,   133,   134,   134,   135,   135,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   137,   138,   138,   139,   139,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   141,   141,   142,
     142,   142,   142,   143,   143,   143,   143,   143,   144,   145,
     145,   145,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   147,   147,   147,   147,   147,   148,   148,   148,
     148,   149,   149,   150,   150,   150,   151,   151,   152,   152,
     152,   153,   153,   154,   154,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   157,   157,   158,   158,   159,   159
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     5,     3,     3,     2,     2,
       0,     1,     3,     2,     4,     2,     5,     2,     2,     5,
       4,     3,     5,     5,     5,     5,     1,     1,     3,     3,
       3,     4,     7,     6,     9,     7,    10,     7,    11,    12,
       9,     4,     6,     4,     0,     1,     1,     2,     1,     1,
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
#line 2199 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2205 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2211 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2217 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2223 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2229 "engines/director/lingo/lingo-gr.cpp"
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
#line 2497 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 197 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2508 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 203 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2518 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 210 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2524 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 243 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2530 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* factory: tFACTORY ID '\n' methodlist  */
#line 247 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2536 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 249 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2542 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* methodlist: %empty  */
#line 251 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2548 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* nonemptymethodlist: methodlistline  */
#line 255 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2559 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlist '\n' methodlistline  */
#line 261 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 2569 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlistline: '\n'  */
#line 268 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2575 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 274 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2584 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* handler: tON ID idlist '\n' stmtlist  */
#line 278 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2591 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* endargdef: ID  */
#line 283 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2597 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* endargdef: endargdef ',' ID  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2603 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* CMDID: tABBREVIATED  */
#line 292 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2609 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* CMDID: tABBREV  */
#line 293 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2615 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* CMDID: tABBR  */
#line 294 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2621 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* CMDID: tAFTER  */
#line 295 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2627 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* CMDID: tAND  */
#line 296 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 2633 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* CMDID: tBEFORE  */
#line 297 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2639 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* CMDID: tCAST  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2645 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tCHAR  */
#line 299 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2651 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tCHARS  */
#line 300 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2657 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tCONTAINS  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("contains"); }
#line 2663 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tDATE  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2669 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tDOWN  */
#line 303 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2675 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tFIELD  */
#line 304 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2681 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tFRAME  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2687 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tIN  */
#line 306 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2693 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tINTERSECTS  */
#line 307 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2699 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tINTO  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tITEM  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tITEMS  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tLAST  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2723 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tLINE  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tLINES  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tLONG  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tMENU  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2747 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tMENUITEM  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tMENUITEMS  */
#line 317 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tMOD  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2765 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tMOVIE  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2771 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tNEXT  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2777 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tNOT  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tNUMBER  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tOF  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tOR  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2801 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tPREVIOUS  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2807 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tREPEAT  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tSCRIPT  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2819 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tASSERTERROR  */
#line 328 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2825 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tSET  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 2831 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tSHORT  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tSOUND  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2843 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tSPRITE  */
#line 332 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2849 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tSTARTS  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2855 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tTELL  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 2861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tTHE  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 2867 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tTIME  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 2873 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tTO  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2879 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tWHILE  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 2885 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tWINDOW  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2891 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* CMDID: tWITH  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 2897 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* CMDID: tWITHIN  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 2903 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* CMDID: tWORD  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 2909 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* CMDID: tWORDS  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 2915 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* ID: tELSE  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 2921 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* ID: tENDCLAUSE  */
#line 348 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); }
#line 2927 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* ID: tEXIT  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 2933 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tFACTORY  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 2939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tGLOBAL  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 2945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tGO  */
#line 352 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 2951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tIF  */
#line 353 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 2957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tINSTANCE  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 2963 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tMACRO  */
#line 355 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 2969 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tMETHOD  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 2975 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tON  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 2981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tOPEN  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 2987 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tPLAY  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 2993 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tPROPERTY  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 2999 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tPUT  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3005 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ID: tTHEN  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3011 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* ID: tWHEN  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("when"); }
#line 3017 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* idlist: %empty  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3023 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* nonemptyidlist: ID  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3032 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3040 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* proc: CMDID cmdargs '\n'  */
#line 397 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist)); }
#line 3046 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* proc: tPUT cmdargs '\n'  */
#line 398 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist)); }
#line 3052 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* proc: tGO cmdargs '\n'  */
#line 399 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist)); }
#line 3058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* proc: tGO frameargs '\n'  */
#line 400 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist)); }
#line 3064 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* proc: tPLAY cmdargs '\n'  */
#line 401 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 3070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* proc: tPLAY frameargs '\n'  */
#line 402 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 3076 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* proc: tOPEN cmdargs '\n'  */
#line 403 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist)); }
#line 3082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* proc: tOPEN expr tWITH expr '\n'  */
#line 404 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args); }
#line 3092 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: tNEXT tREPEAT '\n'  */
#line 409 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3098 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: tEXIT tREPEAT '\n'  */
#line 410 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3104 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tEXIT '\n'  */
#line 411 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3110 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tASSERTERROR stmtoneliner  */
#line 412 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3116 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* cmdargs: %empty  */
#line 415 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* cmdargs: expr  */
#line 418 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3134 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* cmdargs: expr ',' nonemptyexprlist  */
#line 423 "engines/director/lingo/lingo-gr.y"
                                                {
		// This matches `cmd args, ...)
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 3143 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* cmdargs: expr expr_nounarymath  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3154 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist  */
#line 433 "engines/director/lingo/lingo-gr.y"
                                                           {
		// This matches `cmd arg arg, ...`
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 3164 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* cmdargs: '(' ')'  */
#line 438 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* cmdargs: '(' expr ',' nonemptyexprlist ')'  */
#line 441 "engines/director/lingo/lingo-gr.y"
                                                  {
		// This matches `cmd(args, ...)`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3181 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* frameargs: tFRAME expr  */
#line 451 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3191 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* frameargs: tMOVIE expr  */
#line 456 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3202 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 462 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* frameargs: expr tOF tMOVIE expr  */
#line 468 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3224 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* frameargs: tFRAME expr expr_nounarymath  */
#line 474 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3235 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 482 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3241 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 483 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 484 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3253 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* asgn: tSET varorthe to expr '\n'  */
#line 485 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* definevars: tGLOBAL idlist '\n'  */
#line 490 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* definevars: tPROPERTY idlist '\n'  */
#line 491 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3271 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* definevars: tINSTANCE idlist '\n'  */
#line 492 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* ifstmt: tIF expr tTHEN stmt  */
#line 495 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3286 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* ifstmt: tIF expr tTHEN '\n' stmtlist tENDIF '\n'  */
#line 499 "engines/director/lingo/lingo-gr.y"
                                                   {
		(yyval.node) = new IfStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist)); }
#line 3293 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 503 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist tENDIF '\n'  */
#line 509 "engines/director/lingo/lingo-gr.y"
                                                                                {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-5].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), stmtlist1, (yyvsp[-2].nodelist)); }
#line 3313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE stmt  */
#line 513 "engines/director/lingo/lingo-gr.y"
                                                                    {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist tELSE '\n' stmtlist tENDIF '\n'  */
#line 517 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new IfElseStmtNode((yyvsp[-8].node), (yyvsp[-5].nodelist), (yyvsp[-2].nodelist)); }
#line 3329 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 521 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3336 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 523 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 525 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3350 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 527 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3357 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* tell: tTELL expr tTO stmtoneliner  */
#line 531 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 535 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3373 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* when: tWHEN ID tTHEN expr  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new WhenNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* stmtlist: %empty  */
#line 541 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3385 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* nonemptystmtlist: stmtlistline  */
#line 546 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3396 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* nonemptystmtlist: stmtlist stmtlistline  */
#line 552 "engines/director/lingo/lingo-gr.y"
                                                {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3406 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* stmtlistline: '\n'  */
#line 559 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3412 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* simpleexpr_nounarymath: tINT  */
#line 566 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* simpleexpr_nounarymath: tFLOAT  */
#line 567 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3424 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* simpleexpr_nounarymath: tSYMBOL  */
#line 568 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3430 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* simpleexpr_nounarymath: tSTRING  */
#line 569 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3436 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 570 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3442 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* simpleexpr_nounarymath: ID '(' exprlist ')'  */
#line 571 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new FuncNode((yyvsp[-3].s), (yyvsp[-1].nodelist)); }
#line 3448 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 572 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3454 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* var: ID  */
#line 580 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3460 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* chunk: tFIELD simpleexpr  */
#line 590 "engines/director/lingo/lingo-gr.y"
                                {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("field"), args); }
#line 3469 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* chunk: tCAST simpleexpr  */
#line 594 "engines/director/lingo/lingo-gr.y"
                                        {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("cast"), args); }
#line 3478 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* chunk: tCHAR simpleexpr tOF simpleexpr  */
#line 598 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3485 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* chunk: tCHAR simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 600 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* chunk: tWORD simpleexpr tOF simpleexpr  */
#line 602 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3499 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* chunk: tWORD simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 604 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3506 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* chunk: tITEM simpleexpr tOF simpleexpr  */
#line 606 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3513 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* chunk: tITEM simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 608 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3520 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* chunk: tLINE simpleexpr tOF simpleexpr  */
#line 610 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3527 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* chunk: tLINE simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 612 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3534 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* object: tSCRIPT simpleexpr  */
#line 616 "engines/director/lingo/lingo-gr.y"
                                        {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("script"), args); }
#line 3543 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* object: tWINDOW simpleexpr  */
#line 620 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("window"), args); }
#line 3552 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* the: tTHE tLAST chunktype inof simpleexpr  */
#line 629 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 3558 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 634 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3564 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* theobj: tSOUND simpleexpr  */
#line 635 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3570 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* theobj: tSPRITE simpleexpr  */
#line 636 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* menu: tMENU simpleexpr  */
#line 639 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* writablethe: tTHE ID  */
#line 641 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* writablethe: tTHE ID tOF theobj  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3594 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* writablethe: tTHE tNUMBER tOF theobj  */
#line 643 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 3600 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3606 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 647 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3612 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* thedatetime: tTHE tABBREV tDATE  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* thedatetime: tTHE tABBREV tTIME  */
#line 649 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3624 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* thedatetime: tTHE tABBR tDATE  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3630 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* thedatetime: tTHE tABBR tTIME  */
#line 651 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* thedatetime: tTHE tLONG tDATE  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 3642 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* thedatetime: tTHE tLONG tTIME  */
#line 653 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 3648 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* thedatetime: tTHE tSHORT tDATE  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 3654 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* thedatetime: tTHE tSHORT tTIME  */
#line 655 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 3660 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 659 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 3666 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 660 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 3672 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 661 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 3678 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 662 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 3684 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 663 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 3690 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* chunktype: tCHAR  */
#line 666 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 3696 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* chunktype: tWORD  */
#line 667 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 3702 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* chunktype: tITEM  */
#line 668 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 3708 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* chunktype: tLINE  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 3714 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 223: /* list: '[' exprlist ']'  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 3720 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 224: /* list: '[' ':' ']'  */
#line 675 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 3726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 225: /* list: '[' proplist ']'  */
#line 676 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 3732 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 226: /* proplist: proppair  */
#line 679 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 3741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* proplist: proplist ',' proppair  */
#line 683 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* proppair: tSYMBOL ':' expr  */
#line 688 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3755 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 229: /* proppair: ID ':' expr  */
#line 689 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3761 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 230: /* proppair: tSTRING ':' expr  */
#line 690 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3767 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* unarymath: '+' simpleexpr  */
#line 693 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 3773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* unarymath: '-' simpleexpr  */
#line 694 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 3779 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* expr: expr '+' expr  */
#line 704 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3785 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* expr: expr '-' expr  */
#line 705 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3791 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* expr: expr '*' expr  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3797 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* expr: expr '/' expr  */
#line 707 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3803 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* expr: expr tMOD expr  */
#line 708 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3809 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* expr: expr '>' expr  */
#line 709 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3815 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* expr: expr '<' expr  */
#line 710 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3821 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* expr: expr tEQ expr  */
#line 711 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3827 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* expr: expr tNEQ expr  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3833 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* expr: expr tGE expr  */
#line 713 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3839 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* expr: expr tLE expr  */
#line 714 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3845 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* expr: expr tAND expr  */
#line 715 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3851 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* expr: expr tOR expr  */
#line 716 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* expr: expr '&' expr  */
#line 717 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* expr: expr tCONCAT expr  */
#line 718 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3869 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* expr: expr tCONTAINS expr  */
#line 719 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3875 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* expr: expr tSTARTS expr  */
#line 720 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3881 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 729 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 730 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 731 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 732 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3905 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 733 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 261: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 262: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 735 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 263: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 736 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3929 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 264: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 737 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 265: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 738 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3941 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 739 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 740 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3953 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 270: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 743 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3971 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 271: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 744 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 745 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 748 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 749 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* exprlist: %empty  */
#line 752 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 4001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* nonemptyexprlist: expr  */
#line 756 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4010 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 760 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4018 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 4022 "engines/director/lingo/lingo-gr.cpp"

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

#line 765 "engines/director/lingo/lingo-gr.y"


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
