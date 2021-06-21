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
  YYSYMBOL_stmt_insideif = 121,            /* stmt_insideif  */
  YYSYMBOL_stmtoneliner = 122,             /* stmtoneliner  */
  YYSYMBOL_proc = 123,                     /* proc  */
  YYSYMBOL_cmdargs = 124,                  /* cmdargs  */
  YYSYMBOL_frameargs = 125,                /* frameargs  */
  YYSYMBOL_asgn = 126,                     /* asgn  */
  YYSYMBOL_to = 127,                       /* to  */
  YYSYMBOL_definevars = 128,               /* definevars  */
  YYSYMBOL_ifstmt = 129,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 130,               /* ifelsestmt  */
  YYSYMBOL_loop = 131,                     /* loop  */
  YYSYMBOL_tell = 132,                     /* tell  */
  YYSYMBOL_when = 133,                     /* when  */
  YYSYMBOL_stmtlist = 134,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 135,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 136,             /* stmtlistline  */
  YYSYMBOL_stmtlist_insideif = 137,        /* stmtlist_insideif  */
  YYSYMBOL_nonemptystmtlist_insideif = 138, /* nonemptystmtlist_insideif  */
  YYSYMBOL_stmtlistline_insideif = 139,    /* stmtlistline_insideif  */
  YYSYMBOL_simpleexpr_nounarymath = 140,   /* simpleexpr_nounarymath  */
  YYSYMBOL_var = 141,                      /* var  */
  YYSYMBOL_varorchunk = 142,               /* varorchunk  */
  YYSYMBOL_varorthe = 143,                 /* varorthe  */
  YYSYMBOL_chunk = 144,                    /* chunk  */
  YYSYMBOL_object = 145,                   /* object  */
  YYSYMBOL_the = 146,                      /* the  */
  YYSYMBOL_theobj = 147,                   /* theobj  */
  YYSYMBOL_menu = 148,                     /* menu  */
  YYSYMBOL_writablethe = 149,              /* writablethe  */
  YYSYMBOL_thedatetime = 150,              /* thedatetime  */
  YYSYMBOL_thenumberof = 151,              /* thenumberof  */
  YYSYMBOL_chunktype = 152,                /* chunktype  */
  YYSYMBOL_inof = 153,                     /* inof  */
  YYSYMBOL_list = 154,                     /* list  */
  YYSYMBOL_proplist = 155,                 /* proplist  */
  YYSYMBOL_proppair = 156,                 /* proppair  */
  YYSYMBOL_unarymath = 157,                /* unarymath  */
  YYSYMBOL_simpleexpr = 158,               /* simpleexpr  */
  YYSYMBOL_expr = 159,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 160,         /* expr_nounarymath  */
  YYSYMBOL_sprite = 161,                   /* sprite  */
  YYSYMBOL_exprlist = 162,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 163          /* nonemptyexprlist  */
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
#define YYFINAL  177
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3483

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  60
/* YYNRULES -- Number of rules.  */
#define YYNRULES  286
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  527

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
     385,   388,   389,   390,   391,   392,   393,   396,   397,   398,
     401,   402,   403,   404,   405,   406,   407,   408,   413,   414,
     415,   416,   419,   422,   427,   431,   437,   442,   445,   455,
     460,   466,   472,   478,   486,   487,   488,   489,   492,   492,
     494,   495,   496,   499,   503,   507,   513,   517,   521,   525,
     527,   529,   531,   535,   539,   543,   545,   546,   550,   556,
     563,   564,   567,   568,   572,   578,   585,   586,   592,   593,
     594,   595,   596,   597,   598,   599,   600,   601,   602,   603,
     606,   608,   609,   612,   613,   616,   620,   624,   626,   628,
     630,   632,   634,   636,   638,   642,   646,   652,   653,   654,
     655,   658,   659,   660,   661,   662,   665,   667,   668,   669,
     672,   673,   674,   675,   676,   677,   678,   679,   680,   681,
     685,   686,   687,   688,   689,   692,   693,   694,   695,   698,
     698,   700,   701,   702,   705,   709,   714,   715,   716,   719,
     720,   723,   724,   728,   729,   730,   731,   732,   733,   734,
     735,   736,   737,   738,   739,   740,   741,   742,   743,   744,
     745,   746,   753,   754,   755,   756,   757,   758,   759,   760,
     761,   762,   763,   764,   765,   766,   767,   768,   769,   770,
     771,   774,   775,   778,   779,   782,   786
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
  "ID", "idlist", "nonemptyidlist", "stmt", "stmt_insideif",
  "stmtoneliner", "proc", "cmdargs", "frameargs", "asgn", "to",
  "definevars", "ifstmt", "ifelsestmt", "loop", "tell", "when", "stmtlist",
  "nonemptystmtlist", "stmtlistline", "stmtlist_insideif",
  "nonemptystmtlist_insideif", "stmtlistline_insideif",
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

#define YYPACT_NINF (-462)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2364,  -462,  -462,  -462,  -462,  -462,  -462,   -22,  -462,  2934,
    1497,  1594,  -462,  -462,  2934,  -462,    28,  -462,  -462,  1691,
      -1,  3013,  -462,  2934,  -462,  -462,  2934,  1691,  1497,  2934,
    -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,
    -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,
    -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,
    -462,  -462,  -462,  1594,  2934,  2934,   -32,  3328,  -462,    68,
    2364,  -462,  -462,  -462,  -462,  1691,  -462,  -462,  -462,  -462,
    -462,  -462,  -462,  -462,  -462,  -462,  -462,     9,  -462,  -462,
    -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,
    -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,
    -462,  -462,  -462,    27,    29,  -462,  -462,  -462,  -462,  1788,
    1788,  1788,  1788,  1594,  1594,  1788,  1788,  1788,  1788,  1788,
    3092,  1594,  1788,  1788,  1400,   816,    -5,    36,    40,  -462,
    -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,
     915,  -462,  1594,   245,  2934,    41,    43,  1012,  2934,  1594,
    3171,  -462,  -462,   -10,  -462,    93,    44,    53,  1109,    58,
      59,    60,    -4,    61,  2934,  -462,  -462,  -462,  -462,    62,
    1206,  -462,  -462,  2934,  -462,  -462,  -462,  -462,  1303,   717,
    -462,    69,    77,    86,    87,   -25,    -3,     8,    31,    33,
     109,   124,   135,  3375,  -462,  -462,  -462,   436,    70,    71,
      63,    26,    34,  -462,   717,    73,    74,  1594,  -462,  -462,
     145,  1594,  1594,  1594,  1594,  2079,  2079,  2273,  1594,  2176,
    2176,  1594,  1594,  1594,  1594,  1594,  1594,  1594,  1594,  -462,
     523,  -462,   496,  2444,  1594,  1594,  1594,  1594,  1594,    89,
    -462,  -462,  3250,  3250,  3250,   -15,   320,   154,  -462,  -462,
    1594,  1594,   -69,  -462,  1594,  -462,  -462,  -462,  3328,  2524,
    -462,    90,  -462,  -462,   158,  3388,  1788,  1788,  1788,  1788,
    1788,  1788,  1788,  1788,  -462,  -462,  -462,  -462,  -462,  -462,
    -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,    55,  1885,
    1982,  1788,  1788,  1594,  -462,  1594,  1594,  -462,  1594,  2855,
    -462,  -462,  1594,    91,  1594,   -31,   -31,   -31,   -31,   136,
     136,  -462,   -39,   -31,   -31,   -31,   -31,   -39,   -23,   -23,
    -462,  -462,    74,  1594,  1594,  1594,  1594,  1594,  1594,  1594,
    1594,  1594,  1594,  1594,  1594,  1594,  1594,  1594,  1594,  1594,
    1594,  2764,   176,  2524,  -462,    96,  -462,    97,    98,  1594,
    1594,  2524,  1982,   589,   717,  2934,  -462,  -462,  -462,   -69,
    -462,   602,  -462,  -462,  -462,   110,  2524,  -462,  2524,  1594,
    -462,   170,  -462,   171,  -462,   172,  -462,   173,  -462,  -462,
    1788,    55,    55,    55,    55,  1788,  1788,    55,  1788,  1788,
    -462,  -462,  -462,  -462,  -462,  -462,   -73,   717,   717,   717,
      70,    71,   100,  -462,   717,  -462,   717,   -31,   -31,   -31,
     -31,   136,   136,  -462,   -39,   -31,   -31,   -31,   -31,   -39,
     -23,   -23,  -462,  -462,    74,  -462,  -462,   -11,  2764,  -462,
    2604,  -462,  -462,  -462,  -462,   615,   168,   119,  -462,  2934,
    -462,  -462,    99,  -462,   196,   717,  1788,  1788,  1788,  1788,
    -462,  1788,  1788,  1788,  1788,  -462,   179,   147,  -462,  -462,
    -462,  2684,   126,  -462,  2764,  -462,  2524,   190,  1594,   138,
     139,  -462,  2934,  -462,  -462,  -462,  -462,  -462,  -462,  -462,
    -462,   167,  1788,  -462,  2764,  -462,  -462,   155,   153,  1594,
     686,  -462,  2524,   -51,  -462,  1788,   156,   151,   157,   704,
    2524,  -462,  -462,  2934,  -462,   159,  -462,  -462,  2524,   164,
    -462,  -462,   165,   169,   174,  -462,  -462
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    24,    31,    37,    60,    72,    36,     0,    38,    95,
     122,     0,    39,    41,     0,    52,    53,    56,    58,   122,
      59,    62,    70,     0,    73,    71,     0,   122,   122,    95,
      29,    57,    54,    51,    28,    30,    34,    66,    32,    33,
      42,    43,    45,    46,    75,    76,    25,    26,    27,    47,
      63,    35,    44,    48,    49,    50,    55,    68,    69,    64,
      65,    40,    74,    67,    95,     0,     0,    61,     5,     0,
       2,     3,     6,     7,     8,   122,     9,    99,   101,   107,
     108,   109,   102,   103,   104,   105,   106,     0,   120,    79,
      78,    80,    82,    83,    84,    86,    53,    92,    59,    62,
      93,    94,    81,    89,    90,    85,    67,    91,    88,    87,
      61,    77,    97,     0,    96,   168,   169,   171,   170,    31,
      37,    60,    72,    38,    52,    54,    32,    42,    45,    75,
      68,    65,     0,     0,     0,   283,   180,     0,     0,   241,
     175,   176,   177,   178,   197,   198,   199,   179,   242,   243,
     123,   244,     0,     0,    95,     0,     0,   123,     0,     0,
      68,   180,   183,     0,   184,     0,     0,     0,   123,     0,
       0,     0,     0,     0,    95,   100,   121,     1,     4,     0,
     123,   119,   140,     0,   186,   185,   195,   196,   129,   130,
     172,     0,     0,     0,     0,    25,    26,    27,    47,    63,
      44,    55,   207,     0,   239,   240,   127,     0,   171,   170,
       0,   180,     0,   234,   285,     0,   284,   283,   112,   113,
      56,     0,     0,     0,     0,    29,    57,    51,     0,    34,
      66,     0,     0,     0,     0,     0,     0,     0,     0,   262,
     125,   263,     0,     0,     0,     0,     0,     0,     0,     0,
     118,   111,    41,    28,    30,     0,     0,    55,   138,   139,
       0,     0,    13,   116,    73,   114,   115,   142,     0,   156,
     141,     0,   110,    98,    56,   133,     0,     0,     0,     0,
       0,     0,     0,     0,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   225,   227,   228,   226,     0,     0,
       0,     0,     0,     0,   174,     0,     0,   232,     0,     0,
     233,   231,     0,     0,     0,   254,   255,   252,   253,   256,
     257,   249,   259,   260,   261,   251,   250,   258,   245,   246,
     247,   248,   124,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   162,   143,   156,   181,     0,   182,     0,     0,     0,
       0,   156,     0,     0,   155,     0,    17,    18,    11,    14,
      15,     0,   153,   160,   161,     0,   157,   158,   156,     0,
     187,     0,   191,     0,   193,     0,   189,     0,   229,   230,
       0,    33,    43,    46,    76,    48,    49,    50,    64,    65,
     209,   202,   201,   208,   281,   282,     0,   238,   236,   237,
       0,     0,     0,   235,   286,   173,   132,   273,   274,   271,
     272,   275,   276,   268,   278,   279,   280,   270,   269,   277,
     264,   265,   266,   267,   126,   166,   167,     0,   163,   164,
       0,    10,   134,   135,   136,     0,     0,     0,   137,    95,
      16,   117,     0,   159,    20,   131,     0,     0,     0,     0,
     200,     0,     0,     0,     0,   206,     0,     0,   204,   205,
     128,     0,     0,   165,   162,   145,   156,     0,     0,     0,
       0,   154,    21,   188,   192,   194,   190,   220,   222,   223,
     221,     0,     0,   224,   162,   147,   144,     0,     0,     0,
       0,   149,   156,     0,    22,     0,     0,     0,     0,     0,
     156,    12,    19,     0,   203,     0,   146,   152,   156,     0,
      23,   148,     0,     0,     0,   150,   151
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -462,  -462,  -462,   182,  -462,  -462,  -462,  -462,  -462,  -127,
    -462,  -462,     0,    -7,   -12,  -462,     6,  -340,   -64,  -462,
       4,   225,  -462,  -462,  -462,  -462,  -462,  -462,  -462,  -462,
    -341,  -462,  -121,  -461,  -462,  -174,   -58,   -20,  -136,  -462,
    -172,  -462,  -462,   -35,  -200,   249,  -462,  -462,  -462,  -248,
    -462,  -462,   -37,  -462,   328,    85,    88,   -49,    56,  -234
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    69,    70,    71,    72,    73,   367,   368,   369,   370,
      74,   503,   111,   136,   113,   114,   374,    77,    78,    79,
     137,   138,    80,   260,    81,    82,    83,    84,    85,    86,
     375,   376,   377,   437,   438,   439,   139,   140,   355,   163,
     141,   142,   143,   400,   401,   144,   145,   146,   298,   390,
     147,   212,   213,   148,   149,   214,   240,   151,   215,   216
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      75,   162,   112,   176,   332,   471,    76,   154,   359,    87,
     246,   436,   441,   497,   161,   365,   165,   171,   246,   166,
     447,   228,   112,   156,   258,   312,   246,   470,   366,   360,
     268,   167,   169,   506,   259,   158,   159,   454,   221,   222,
     223,   224,   244,   245,   284,   246,   512,   513,   228,   247,
     248,   285,   173,   234,   235,   236,   237,   112,   174,   155,
     233,   234,   235,   236,   237,   175,   286,    75,   177,   406,
      75,   236,   237,   287,   472,    88,    76,   288,   388,   179,
     356,   356,   356,   389,   289,   231,   232,   233,   234,   235,
     236,   237,   239,   269,   217,   150,   153,   276,   436,   239,
     290,   241,   292,   277,   157,   278,   181,   291,   241,   293,
     239,   279,   168,   150,   280,   282,   434,   357,   358,   241,
     281,   283,   239,   202,   182,   217,   261,   183,   211,   308,
     239,   241,   309,   218,   436,   498,   310,   219,   250,   241,
     251,   262,   249,   461,   462,   463,   464,   112,   172,   467,
     263,   255,   299,   202,   436,   265,   266,   267,   270,   272,
     180,   511,   271,   300,   294,   307,   295,   112,   296,   519,
     297,   314,   312,   305,   306,   311,   273,   522,   221,   222,
     223,   224,   362,   477,   379,   246,   353,   378,   228,   247,
     248,   415,   440,   442,   443,   444,   481,   452,   456,   457,
     458,   459,   478,   308,   372,   479,   482,   491,   188,   189,
     221,   222,   223,   224,   244,   245,   203,   246,   492,   207,
     228,   247,   248,   496,   499,   231,   232,   233,   234,   235,
     236,   237,   354,   354,   354,   501,   502,   242,   505,   508,
     507,   515,   450,    75,   256,   161,   161,   161,   516,   352,
     523,   524,   178,   170,   517,   453,   521,   231,   232,   233,
     234,   235,   236,   237,   473,   403,   525,   493,    75,    75,
     164,   526,   413,   313,     0,     0,   275,     0,   243,     0,
       0,     0,     0,     0,     0,     0,     0,   221,   222,   223,
     224,   244,   245,     0,   246,     0,     0,   228,   247,   248,
       0,     0,   412,     0,     0,     0,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,     0,     0,     0,     0,     0,     0,   319,
     320,   321,   323,   324,   231,   232,   233,   234,   235,   236,
     237,     0,     0,     0,     0,   363,   364,     0,     0,   371,
       0,    75,     0,    75,     0,     0,     0,     0,   449,     0,
       0,    75,   221,   222,   223,   224,   244,   245,     0,   246,
       0,     0,   228,   247,   248,     0,    75,     0,    75,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     407,   408,     0,   409,     0,     0,     0,   414,     0,   416,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   231,
     232,   233,   234,   235,   236,   237,     0,   361,   417,   418,
     419,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,     0,     0,   480,    75,     0,
      75,     0,   112,     0,   445,   446,   475,   184,   185,   186,
     187,     0,     0,   190,   191,   192,   193,   194,     0,     0,
     204,   205,     0,     0,   455,     0,     0,     0,     0,     0,
       0,    75,     0,     0,    75,   504,    75,   495,   221,   222,
     223,   224,   244,   245,     0,   246,     0,     0,   228,   247,
     248,     0,     0,     0,    75,     0,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,   520,     0,     0,     0,
      75,     0,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,     0,     0,   231,   232,   233,   234,   235,
     236,   237,     0,     0,   303,     0,   304,     0,   221,   222,
     223,   224,   244,   245,     0,   246,     0,     0,   228,   247,
     248,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   500,     0,   333,   334,   335,   336,   337,
     338,     0,   339,     0,     0,   340,   341,   342,     0,     0,
       0,     0,     0,     0,   509,   231,   232,   233,   234,   235,
     236,   237,     0,     0,     0,     0,   304,     0,     0,     0,
       0,     0,     0,     0,   380,   381,   382,   383,   384,   385,
     386,   387,   343,   344,   345,   346,   347,   348,   349,     0,
       0,   350,     0,     0,     0,     0,     0,   402,   402,   404,
     405,   221,   222,   223,   224,   244,   245,     0,   246,     0,
       0,   228,   247,   248,   221,   222,   223,   224,   244,   245,
       0,   246,     0,     0,   228,   247,   248,   221,   222,   223,
     224,   244,   245,     0,   246,     0,     0,   228,   247,   248,
       0,     0,     0,     0,     0,     0,     0,     0,   231,   232,
     233,   234,   235,   236,   237,     0,   448,     0,     0,     0,
     402,   231,   232,   233,   234,   235,   236,   237,     0,   451,
       0,     0,     0,     0,   231,   232,   233,   234,   235,   236,
     237,     0,   476,     0,     0,     0,     0,     0,   460,     0,
       0,     0,     0,   465,   466,     0,   468,   469,   221,   222,
     223,   224,   244,   245,     0,   246,     0,     0,   228,   247,
     248,     0,     0,     0,     0,     0,   221,   222,   223,   224,
     244,   245,     0,   246,     0,     0,   228,   247,   248,   221,
     222,   223,   224,   244,   245,     0,   246,     0,     0,   228,
     247,   248,     0,     0,     0,   231,   232,   233,   234,   235,
     236,   237,     0,   510,   483,   484,   485,   486,     0,   487,
     488,   489,   490,   231,   232,   233,   234,   235,   236,   237,
       0,   518,     0,     0,     0,     0,   231,   232,   233,   234,
     235,   236,   237,     0,     0,     0,     0,     0,     0,     0,
     465,   115,   116,     1,   208,   209,    89,   119,   120,   121,
     122,     6,    90,   514,    91,     8,    92,    93,    94,    12,
      13,    95,    15,    96,    17,    18,    97,    98,    99,   100,
      22,   101,    24,    25,   102,   103,   104,   105,     0,     0,
       0,     0,    30,    31,   125,    33,    34,    35,     0,    36,
      37,   126,    39,   127,    41,   128,    43,   129,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,   130,    58,    59,   131,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   132,   133,
       0,     0,     0,     0,     0,   152,     0,   135,     0,   210,
     115,   116,     1,   117,   118,    89,   119,   120,   121,   122,
       6,    90,     0,    91,     8,    92,    93,    94,    12,    13,
      95,    15,    96,   220,    18,    97,    98,    99,   100,    22,
     101,    24,    25,   102,   103,   104,   105,   221,   222,   223,
     224,   225,   226,   125,   227,    34,    35,   228,   229,   230,
     126,    39,   127,    41,   128,    43,   129,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,    56,
     130,    58,    59,   131,    61,    62,   106,   107,   108,   109,
       0,     0,     0,   110,   231,   232,   233,   234,   235,   236,
     237,     0,     0,   238,   152,     0,   135,   115,   116,     1,
     117,   118,    89,   119,   120,   121,   122,     6,    90,     0,
      91,     8,    92,    93,    94,    12,   252,    95,    15,    96,
      17,    18,    97,    98,    99,   100,    22,   101,    24,    25,
     102,   103,   104,   105,   221,   222,   223,   224,   225,   226,
     125,   227,   253,   254,   228,   229,   230,   126,    39,   127,
      41,   128,    43,   129,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,    56,   130,    58,    59,
     131,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,   231,   232,   233,   234,   235,   236,   237,     0,     0,
     238,   152,     0,   135,   115,   116,     1,   117,   118,    89,
     119,   120,   121,   122,     6,    90,     0,    91,     8,    92,
      93,    94,    12,    13,    95,    15,    96,    17,    18,    97,
      98,    99,   100,    22,   101,   264,    25,   102,   103,   104,
     105,   221,   222,   223,   224,   225,   226,   125,   227,    34,
      35,   228,   229,   230,   126,    39,   127,    41,   128,    43,
     129,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,    54,    55,    56,   130,    58,    59,   131,    61,    62,
     106,   107,   108,   109,     0,     0,     0,   110,   231,   232,
     233,   234,   235,   236,   237,     0,     0,   238,   152,     0,
     135,   115,   116,     1,   117,   118,    89,   119,   120,   121,
     122,     6,    90,     0,    91,     8,    92,    93,    94,    12,
      13,    95,    15,    96,    17,    18,    97,    98,    99,   100,
      22,   101,    24,    25,   102,   103,   104,   105,   221,   222,
     223,   224,   225,   226,   125,   227,    34,    35,   228,   229,
     230,   126,    39,   127,    41,   128,    43,   129,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,   130,    58,    59,   131,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,   231,   232,   233,   234,   235,
     236,   237,     0,     0,   238,   152,     0,   135,   115,   116,
       1,   117,   118,    89,   119,   120,   121,   122,     6,    90,
       0,    91,     8,    92,    93,    94,    12,    13,    95,    15,
      96,   274,    18,    97,    98,    99,   100,    22,   101,    24,
      25,   102,   103,   104,   105,   221,   222,   223,   224,   225,
     226,   125,   227,    34,    35,   228,   229,   230,   126,    39,
     127,    41,   128,    43,   129,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,   130,    58,
      59,   131,    61,    62,   106,   107,   108,   109,     0,     0,
       0,   110,   231,   232,   233,   234,   235,   236,   237,     0,
       0,     0,   152,     0,   135,   115,   116,     1,   117,   118,
      89,   119,   120,   121,   122,     6,    90,     0,    91,     8,
      92,    93,    94,    12,    13,    95,    15,    96,    17,    18,
      97,    98,    99,   100,    22,   101,    24,    25,   102,   103,
     104,   105,     0,     0,     0,     0,    30,    31,   125,    33,
      34,    35,     0,    36,    37,   126,    39,   127,    41,   128,
      43,   129,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,   130,    58,    59,   131,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,   132,   133,     0,     0,     0,     0,     0,   152,
     206,   135,   115,   116,     1,   117,   118,    89,   119,   120,
     121,   122,     6,    90,     0,    91,   123,    92,    93,    94,
      12,    13,    95,   124,    96,    17,    18,    97,    98,    99,
     100,    22,   101,    24,    25,   102,   103,   104,   105,     0,
       0,     0,     0,    30,    31,   125,    33,    34,    35,     0,
      36,    37,   126,    39,   127,    41,   128,    43,   129,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,    54,
      55,    56,   130,    58,    59,   131,    61,    62,   106,   107,
     108,   109,     0,     0,     0,   110,     0,     0,     0,   132,
     133,     0,     0,     0,     0,     0,   134,     0,   135,   115,
     116,     1,   117,   118,    89,   119,   120,   121,   122,     6,
      90,     0,    91,     8,    92,    93,    94,    12,    13,    95,
      15,    96,    17,    18,    97,    98,    99,   100,    22,   101,
      24,    25,   102,   103,   104,   105,     0,     0,     0,     0,
      30,    31,   125,    33,    34,    35,     0,    36,    37,   126,
      39,   127,    41,   128,    43,   129,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,   130,
      58,    59,   131,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,     0,     0,     0,   132,   133,     0,     0,
       0,     0,     0,   152,     0,   135,   115,   116,     1,   117,
     118,    89,   119,   120,   121,   122,     6,    90,     0,    91,
       8,    92,    93,    94,    12,    13,    95,    15,    96,    17,
      18,    97,    98,    99,   100,    22,   101,    24,    25,   102,
     103,   104,   105,     0,     0,     0,     0,    30,    31,   125,
      33,    34,    35,     0,    36,    37,   126,    39,   127,    41,
     128,    43,   129,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,    54,    55,    56,   130,    58,    59,   131,
      61,    62,   106,   107,   108,   109,     0,     0,     0,   110,
       0,     0,     0,   132,   133,     0,     0,     0,     0,     0,
     134,     0,   135,   115,   116,     1,   117,   118,    89,   119,
     120,   121,   122,     6,    90,     0,    91,     8,    92,    93,
      94,    12,    13,    95,    15,    96,    17,    18,    97,    98,
      99,   100,    22,   101,    24,    25,   102,   103,   104,   105,
       0,     0,     0,     0,    30,    31,   125,    33,    34,    35,
       0,    36,    37,   126,    39,   127,    41,   128,    43,   129,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,    56,   130,    58,    59,    60,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,     0,     0,     0,
     132,   133,     0,     0,     0,     0,     0,   152,     0,   135,
     115,   116,     1,   117,   118,    89,   119,   120,   121,   122,
       6,    90,     0,    91,     8,    92,    93,    94,    12,    13,
      95,    15,    96,    17,    18,    97,    98,    99,   100,    22,
     101,    24,    25,   102,   103,   104,   105,     0,     0,     0,
       0,    30,    31,   125,    33,    34,    35,     0,    36,    37,
     126,   391,   127,   392,   128,   393,   129,   394,    46,    47,
      48,    49,    50,     0,    51,    52,   395,   396,   397,    56,
     130,    58,   398,   399,    61,    62,   106,   107,   108,   109,
       0,     0,     0,   110,     0,     0,     0,   132,   133,     0,
       0,     0,     0,     0,   152,     0,   135,   115,   116,     1,
     117,   118,    89,   119,   120,   121,   122,     6,    90,     0,
      91,     8,    92,    93,    94,    12,    13,    95,    15,    96,
      17,    18,    97,    98,    99,   100,    22,   101,    24,    25,
     102,   103,   104,   105,     0,     0,     0,     0,    30,    31,
     125,    33,    34,    35,     0,    36,    37,   126,    39,   127,
      41,   128,    43,   129,    45,    46,    47,    48,    49,    50,
       0,    51,    52,   395,   396,    55,    56,   130,    58,   398,
     399,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,     0,     0,     0,   132,   133,     0,     0,     0,     0,
       0,   152,     0,   135,   115,   116,     1,   117,   118,    89,
     119,   120,   121,   122,     6,    90,     0,    91,     8,    92,
      93,    94,    12,    13,    95,    15,    96,    17,    18,    97,
      98,    99,   100,    22,   101,    24,    25,   102,   103,   104,
     105,     0,     0,     0,     0,     0,     0,   125,    33,    34,
      35,     0,    36,    37,   126,    39,   127,    41,   128,    43,
     129,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,    54,    55,    56,   130,    58,    59,   131,    61,    62,
     106,   107,   108,   109,     0,     0,     0,   110,     0,     0,
       0,   132,   133,     0,     0,     0,     0,     0,   152,     0,
     135,   115,   116,     1,   117,   118,    89,   119,   120,   121,
     122,     6,    90,     0,    91,     8,    92,    93,    94,    12,
      13,    95,    15,    96,    17,    18,    97,    98,    99,   100,
      22,   101,    24,    25,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   125,    33,    34,    35,     0,     0,
       0,   126,    39,   127,    41,   128,    43,   129,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
      56,   130,    58,    59,   131,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   132,   133,
       0,     0,     0,     0,     0,   152,     0,   135,   115,   116,
       1,   117,   118,    89,   119,   120,   121,   122,     6,    90,
       0,    91,     8,    92,    93,    94,    12,    13,    95,    15,
      96,    17,    18,    97,    98,    99,   100,    22,   101,    24,
      25,   102,   103,   104,   105,     0,     0,     0,     0,     0,
       0,   125,     0,    34,    35,     0,     0,     0,   126,    39,
     127,    41,   128,    43,   129,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,   130,    58,
      59,   131,    61,    62,   106,   107,   108,   109,     0,     0,
       0,   110,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,   152,     0,   135,     2,     3,     4,     5,     6,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,     0,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,     0,    66,
       0,     1,    67,     0,     0,     2,     3,     4,     5,     6,
       0,    68,     7,     8,     9,    10,    11,    12,    13,     0,
      15,    16,    17,    18,    19,    20,    21,     0,    22,    23,
      24,    25,     0,    27,    28,    29,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,     0,     0,    66,
       0,     1,    67,     0,     0,     2,     3,     4,     5,     6,
       0,   351,     7,     8,     9,    10,    11,    12,    13,     0,
      15,    16,    17,    18,    19,    20,    21,     0,    22,    23,
      24,    25,     0,    27,    28,    29,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,     0,     0,    66,
       0,     1,    67,     0,     0,     2,     3,     4,     5,     6,
       0,   373,     7,     8,     9,    10,    11,    12,    13,     0,
      15,    16,    17,    18,    19,    20,    21,     0,    22,    23,
      24,    25,     0,    27,    28,    29,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,     0,     0,    66,
       0,     1,    67,     0,     0,     2,     3,     4,     5,     6,
       0,   474,     7,     8,     9,    10,    11,    12,    13,     0,
      15,    16,    17,    18,    19,    20,    21,     0,    22,    23,
      24,    25,     0,    27,    28,    29,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,     0,     0,    66,
       0,     1,    67,     0,     0,     2,     3,     4,     5,     6,
       0,   494,     7,     8,     9,    10,    11,    12,    13,     0,
      15,    16,    17,    18,    19,    20,    21,     0,    22,    23,
      24,    25,     0,    27,    28,    29,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,     0,     0,     0,
       0,     0,    67,     0,     0,     0,     0,     0,     0,     0,
       0,   435,     1,   410,   411,    89,     2,     3,     4,     5,
       6,    90,     0,    91,     8,    92,    93,    94,    12,    13,
      95,    15,    96,    17,    18,    97,    98,    99,   100,    22,
     101,    24,    25,   102,   103,   104,   105,     0,     0,     0,
       0,    30,    31,    32,    33,    34,    35,     0,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,   106,   107,   108,   109,
       0,     1,     0,   110,    89,     2,     3,     4,     5,     6,
      90,     0,    91,     8,    92,    93,    94,    12,    13,    95,
      15,    96,    17,    18,    97,    98,    99,   100,    22,   101,
      24,    25,   102,   103,   104,   105,     0,     0,     0,     0,
      30,    31,    32,    33,    34,    35,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,   106,   107,   108,   109,     0,
       1,     0,   110,    89,     2,     3,     4,     5,     6,    90,
       0,    91,     8,    92,    93,    94,    12,    13,    95,    15,
      96,    17,    18,    97,    98,    99,   100,    22,   101,    24,
      25,   102,   103,   104,   105,     0,     0,     0,     0,    30,
      31,    32,    33,    34,    35,     0,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,    54,    55,    56,   160,    58,
      59,    60,    61,    62,   106,   107,   108,   109,     0,     1,
       0,   110,    89,     2,     3,     4,     5,     6,    90,     0,
      91,     8,    92,    93,    94,    12,    13,    95,    15,    96,
      17,    18,    97,    98,    99,   100,    22,   101,    24,    25,
     102,   103,   104,   105,     0,     0,     0,     0,    30,    31,
      32,    33,    34,    35,     0,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,   195,   196,   197,   198,   199,
       0,    51,   200,    53,    54,    55,   201,    57,    58,    59,
      60,    61,    62,   106,   107,   108,   109,     0,     1,     0,
     110,    89,     2,     3,     4,     5,     6,    90,     0,    91,
       8,    92,    93,    94,    12,    13,    95,    15,    96,    17,
      18,    97,    98,    99,   100,    22,   101,    24,    25,   102,
     103,   104,   105,     0,     0,     0,     0,    30,    31,    32,
      33,    34,    35,     0,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,    54,    55,   257,    57,    58,    59,    60,
      61,    62,   106,   107,   108,   109,     0,     1,     0,   110,
      89,   119,   120,     4,     5,     6,    90,     0,    91,     8,
      92,    93,    94,    12,    13,    95,    15,    96,    17,    18,
      97,    98,    99,   100,    22,   101,    24,    25,   102,   103,
     104,   105,     0,     0,     0,     0,    30,    31,    32,    33,
      34,    35,     0,    36,    37,   126,    39,   127,    41,   128,
      43,   129,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,   106,   107,   108,   109,     1,     0,     0,   110,     2,
       3,     4,     5,     6,     0,     0,     7,     8,     9,    10,
       0,    12,    13,     0,    15,    16,    17,    18,    19,    98,
      21,     0,    22,     0,    24,    25,     0,    27,    28,    29,
       0,     0,     0,     0,    30,    31,    32,    33,    34,    35,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,   106,
      64,     0,     0,     0,     0,     0,    67,   221,   222,   223,
     224,   244,   245,     0,   246,     0,     0,   228,   247,   248,
     333,   334,   335,   336,   337,   338,     0,   339,     0,     0,
     340,   341,   342,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   301,   302,     0,     0,     0,     0,
       0,     0,     0,     0,   231,   232,   233,   234,   235,   236,
     237,     0,     0,     0,     0,     0,     0,   343,   344,   345,
     346,   347,   348,   349
};

static const yytype_int16 yycheck[] =
{
       0,    21,     9,    67,   238,    16,     0,    14,    23,    31,
      49,   351,   353,   474,    21,    84,    23,    29,    49,    26,
     361,    52,    29,    19,    34,    98,    49,   100,    97,    44,
      34,    27,    28,   494,    44,    36,    37,   378,    42,    43,
      44,    45,    46,    47,    69,    49,    97,    98,    52,    53,
      54,    76,    64,    92,    93,    94,    95,    64,    65,    31,
      91,    92,    93,    94,    95,    97,    69,    67,     0,   303,
      70,    94,    95,    76,    85,    97,    70,    69,    23,    75,
     252,   253,   254,    28,    76,    89,    90,    91,    92,    93,
      94,    95,   150,    97,    99,    10,    11,    28,   438,   157,
      69,   150,    69,    34,    19,    28,    97,    76,   157,    76,
     168,    34,    27,    28,    28,    28,   350,   253,   254,   168,
      34,    34,   180,   130,    97,    99,    33,    98,   135,   103,
     188,   180,    98,    97,   474,   476,   102,    97,    97,   188,
      97,    97,   154,   391,   392,   393,   394,   154,    63,   397,
      97,   158,    28,   160,   494,    97,    97,    97,    97,    97,
      75,   502,   174,    28,    55,   102,    57,   174,    59,   510,
      61,    26,    98,   103,   103,   102,   183,   518,    42,    43,
      44,    45,    28,    15,    26,    49,    97,    97,    52,    53,
      54,   100,    16,    97,    97,    97,    97,    87,    28,    28,
      28,    28,    34,   103,   268,    86,    10,    28,   123,   124,
      42,    43,    44,    45,    46,    47,   131,    49,    71,   134,
      52,    53,    54,    97,    34,    89,    90,    91,    92,    93,
      94,    95,   252,   253,   254,    97,    97,   152,    71,    86,
      85,    85,   369,   243,   159,   252,   253,   254,    97,   243,
      86,    86,    70,    28,    97,   376,    97,    89,    90,    91,
      92,    93,    94,    95,   438,   300,    97,   467,   268,   269,
      21,    97,   309,   217,    -1,    -1,   188,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      -1,    -1,   309,    -1,    -1,    -1,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,    -1,    -1,    -1,    -1,    -1,    -1,   244,
     245,   246,   247,   248,    89,    90,    91,    92,    93,    94,
      95,    -1,    -1,    -1,    -1,   260,   261,    -1,    -1,   264,
      -1,   351,    -1,   353,    -1,    -1,    -1,    -1,   365,    -1,
      -1,   361,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    -1,   376,    -1,   378,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     305,   306,    -1,   308,    -1,    -1,    -1,   312,    -1,   314,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    -1,    97,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,    -1,    -1,   449,   438,    -1,
     440,    -1,   449,    -1,   359,   360,   440,   119,   120,   121,
     122,    -1,    -1,   125,   126,   127,   128,   129,    -1,    -1,
     132,   133,    -1,    -1,   379,    -1,    -1,    -1,    -1,    -1,
      -1,   471,    -1,    -1,   474,   482,   476,   471,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    -1,    -1,    -1,   494,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   502,    -1,    -1,    -1,   513,    -1,    -1,    -1,
     510,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   518,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    -1,    -1,    98,    -1,   100,    -1,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   478,    -1,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    -1,   499,    89,    90,    91,    92,    93,
      94,    95,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   276,   277,   278,   279,   280,   281,
     282,   283,    89,    90,    91,    92,    93,    94,    95,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,   299,   300,   301,
     302,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    95,    -1,    97,    -1,    -1,    -1,
     362,    89,    90,    91,    92,    93,    94,    95,    -1,    97,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    -1,    97,    -1,    -1,    -1,    -1,    -1,   390,    -1,
      -1,    -1,    -1,   395,   396,    -1,   398,   399,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    52,    53,    54,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    -1,    97,   456,   457,   458,   459,    -1,   461,
     462,   463,   464,    89,    90,    91,    92,    93,    94,    95,
      -1,    97,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     492,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,   505,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,
      -1,    -1,    -1,    -1,    -1,    99,    -1,   101,    -1,   103,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    -1,    -1,    98,    99,    -1,   101,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    -1,    -1,
      98,    99,    -1,   101,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    -1,    -1,    98,    99,    -1,
     101,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    -1,    -1,    98,    99,    -1,   101,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    95,    -1,
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
     100,   101,     5,     6,     7,     8,     9,    10,    11,    12,
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
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    88,    -1,    -1,    -1,    92,    93,    -1,    -1,
      -1,    -1,    -1,    99,    -1,   101,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    88,
      -1,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    -1,
      99,    -1,   101,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    88,    -1,    -1,    -1,
      92,    93,    -1,    -1,    -1,    -1,    -1,    99,    -1,   101,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    -1,    -1,    99,    -1,   101,     5,     6,     7,
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
      41,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    88,    -1,    -1,
      -1,    92,    93,    -1,    -1,    -1,    -1,    -1,    99,    -1,
     101,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    50,    51,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    88,    -1,    -1,    -1,    92,    93,
      -1,    -1,    -1,    -1,    -1,    99,    -1,   101,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     7,    99,    -1,   101,    11,    12,    13,    14,    15,
      -1,    -1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    -1,    85,
      -1,     7,    88,    -1,    -1,    11,    12,    13,    14,    15,
      -1,    97,    18,    19,    20,    21,    22,    23,    24,    -1,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      36,    37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    -1,    -1,    85,
      -1,     7,    88,    -1,    -1,    11,    12,    13,    14,    15,
      -1,    97,    18,    19,    20,    21,    22,    23,    24,    -1,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      36,    37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    -1,    -1,    85,
      -1,     7,    88,    -1,    -1,    11,    12,    13,    14,    15,
      -1,    97,    18,    19,    20,    21,    22,    23,    24,    -1,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      36,    37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    -1,    -1,    85,
      -1,     7,    88,    -1,    -1,    11,    12,    13,    14,    15,
      -1,    97,    18,    19,    20,    21,    22,    23,    24,    -1,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      36,    37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    -1,    -1,    85,
      -1,     7,    88,    -1,    -1,    11,    12,    13,    14,    15,
      -1,    97,    18,    19,    20,    21,    22,    23,    24,    -1,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      36,    37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    97,     7,     8,     9,    10,    11,    12,    13,    14,
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
      78,    79,    80,    81,    82,    83,    84,    -1,     7,    -1,
      88,    10,    11,    12,    13,    14,    15,    16,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    -1,     7,    -1,    88,
      10,    11,    12,    13,    14,    15,    16,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,     7,    -1,    -1,    88,    11,
      12,    13,    14,    15,    -1,    -1,    18,    19,    20,    21,
      -1,    23,    24,    -1,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,    -1,    36,    37,    -1,    39,    40,    41,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    88,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    79,    80,    -1,    -1,    -1,    -1,
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
      78,    79,    80,    81,    82,    83,    85,    88,    97,   105,
     106,   107,   108,   109,   114,   116,   120,   121,   122,   123,
     126,   128,   129,   130,   131,   132,   133,    31,    97,    10,
      16,    18,    20,    21,    22,    25,    27,    30,    31,    32,
      33,    35,    38,    39,    40,    41,    81,    82,    83,    84,
      88,   116,   117,   118,   119,     5,     6,     8,     9,    11,
      12,    13,    14,    19,    26,    48,    55,    57,    59,    61,
      75,    78,    92,    93,    99,   101,   117,   124,   125,   140,
     141,   144,   145,   146,   149,   150,   151,   154,   157,   158,
     159,   161,    99,   159,   117,    31,   124,   159,    36,    37,
      75,   117,   141,   143,   149,   117,   117,   124,   159,   124,
     125,   118,   159,   118,   117,    97,   122,     0,   107,   124,
     159,    97,    97,    98,   158,   158,   158,   158,   159,   159,
     158,   158,   158,   158,   158,    63,    64,    65,    66,    67,
      70,    74,   117,   159,   158,   158,   100,   159,     8,     9,
     103,   117,   155,   156,   159,   162,   163,    99,    97,    97,
      28,    42,    43,    44,    45,    46,    47,    49,    52,    53,
      54,    89,    90,    91,    92,    93,    94,    95,    98,   140,
     160,   161,   159,    33,    46,    47,    49,    53,    54,   118,
      97,    97,    24,    50,    51,   117,   159,    74,    34,    44,
     127,    33,    97,    97,    36,    97,    97,    97,    34,    97,
      97,   118,    97,   117,    28,   160,    28,    34,    28,    34,
      28,    34,    28,    34,    69,    76,    69,    76,    69,    76,
      69,    76,    69,    76,    55,    57,    59,    61,   152,    28,
      28,    79,    80,    98,   100,   103,   103,   102,   103,    98,
     102,   102,    98,   162,    26,   159,   159,   159,   159,   159,
     159,   159,   159,   159,   159,   159,   159,   159,   159,   159,
     159,   159,   163,    42,    43,    44,    45,    46,    47,    49,
      52,    53,    54,    89,    90,    91,    92,    93,    94,    95,
      98,    97,   120,    97,   141,   142,   144,   142,   142,    23,
      44,    97,    28,   159,   159,    84,    97,   110,   111,   112,
     113,   159,   122,    97,   120,   134,   135,   136,    97,    26,
     158,   158,   158,   158,   158,   158,   158,   158,    23,    28,
     153,    56,    58,    60,    62,    71,    72,    73,    77,    78,
     147,   148,   158,   147,   158,   158,   163,   159,   159,   159,
       8,     9,   117,   156,   159,   100,   159,   159,   159,   159,
     159,   159,   159,   159,   159,   159,   159,   159,   159,   159,
     159,   159,   159,   159,   163,    97,   121,   137,   138,   139,
      16,   134,    97,    97,    97,   159,   159,   134,    97,   117,
     113,    97,    87,   136,   134,   159,    28,    28,    28,    28,
     158,   153,   153,   153,   153,   158,   158,   153,   158,   158,
     100,    16,    85,   139,    97,   120,    97,    15,    34,    86,
     118,    97,    10,   158,   158,   158,   158,   158,   158,   158,
     158,    28,    71,   148,    97,   120,    97,   137,   134,    34,
     159,    97,    97,   115,   117,    71,   137,    85,    86,   159,
      97,   134,    97,    98,   158,    85,    97,    97,    97,   134,
     117,    97,   134,    86,    86,    97,    97
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
     120,   121,   121,   121,   121,   121,   121,   122,   122,   122,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   124,   124,   124,   124,   124,   124,   124,   125,
     125,   125,   125,   125,   126,   126,   126,   126,   127,   127,
     128,   128,   128,   129,   129,   130,   130,   130,   130,   131,
     131,   131,   131,   132,   132,   133,   134,   134,   135,   135,
     136,   136,   137,   137,   138,   138,   139,   139,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     141,   142,   142,   143,   143,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   145,   145,   146,   146,   146,
     146,   147,   147,   147,   147,   147,   148,   149,   149,   149,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     151,   151,   151,   151,   151,   152,   152,   152,   152,   153,
     153,   154,   154,   154,   155,   155,   156,   156,   156,   157,
     157,   158,   158,   159,   159,   159,   159,   159,   159,   159,
     159,   159,   159,   159,   159,   159,   159,   159,   159,   159,
     159,   159,   160,   160,   160,   160,   160,   160,   160,   160,
     160,   160,   160,   160,   160,   160,   160,   160,   160,   160,
     160,   161,   161,   162,   162,   163,   163
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       5,     4,     5,     0,     1,     1,     2,     1,     1,     8,
       5,     0,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     3,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     5,     3,     3,
       2,     2,     0,     1,     3,     2,     4,     2,     5,     2,
       2,     5,     4,     3,     5,     5,     5,     5,     1,     1,
       3,     3,     3,     4,     7,     6,     9,     7,    10,     7,
      11,    12,     9,     4,     6,     4,     0,     1,     1,     2,
       1,     1,     0,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     2,     4,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     4,     6,     4,
       6,     4,     6,     4,     6,     2,     2,     1,     1,     1,
       5,     1,     1,     5,     2,     2,     2,     2,     4,     4,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       6,     6,     6,     6,     6,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     1,     3,     3,     3,     3,     2,
       2,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     4,     4,     0,     1,     1,     3
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
#line 2063 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2069 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2075 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2081 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2087 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 189 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2093 "engines/director/lingo/lingo-gr.cpp"
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
#line 2361 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 197 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 203 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2382 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 210 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 243 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2394 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* factory: tFACTORY ID '\n' methodlist  */
#line 247 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2400 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 249 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2406 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* methodlist: %empty  */
#line 251 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2412 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* nonemptymethodlist: methodlistline  */
#line 255 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2423 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 261 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2433 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlistline: '\n'  */
#line 268 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2439 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 274 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2448 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* handler: tON ID idlist '\n' stmtlist  */
#line 278 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2455 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* endargdef: ID  */
#line 283 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2461 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* endargdef: endargdef ',' ID  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2467 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* CMDID: tABBREVIATED  */
#line 292 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* CMDID: tABBREV  */
#line 293 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2479 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* CMDID: tABBR  */
#line 294 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2485 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* CMDID: tAFTER  */
#line 295 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2491 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* CMDID: tAND  */
#line 296 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 2497 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* CMDID: tBEFORE  */
#line 297 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2503 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* CMDID: tCAST  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2509 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tCHAR  */
#line 299 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2515 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tCHARS  */
#line 300 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2521 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tCONTAINS  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("contains"); }
#line 2527 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tDATE  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2533 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tDOWN  */
#line 303 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2539 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tFIELD  */
#line 304 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2545 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tFRAME  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2551 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tIN  */
#line 306 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2557 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tINTERSECTS  */
#line 307 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2563 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tINTO  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2569 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tITEM  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2575 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tITEMS  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2581 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tLAST  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2587 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tLINE  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2593 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tLINES  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2599 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tLONG  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tMENU  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tMENUITEM  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2617 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tMENUITEMS  */
#line 317 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2623 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tMOD  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2629 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tMOVIE  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2635 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tNEXT  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2641 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tNOT  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2647 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tNUMBER  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2653 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tOF  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2659 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tOR  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2665 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tPREVIOUS  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2671 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tREPEAT  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2677 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tSCRIPT  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2683 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tASSERTERROR  */
#line 328 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2689 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tSET  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 2695 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tSHORT  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2701 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tSOUND  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2707 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tSPRITE  */
#line 332 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2713 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tSTARTS  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2719 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tTELL  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 2725 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tTHE  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 2731 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tTIME  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 2737 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tTO  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2743 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tWHILE  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 2749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tWINDOW  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2755 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* CMDID: tWITH  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 2761 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* CMDID: tWITHIN  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 2767 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* CMDID: tWORD  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 2773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* CMDID: tWORDS  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 2779 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* ID: tELSE  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 2785 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* ID: tENDCLAUSE  */
#line 348 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); }
#line 2791 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* ID: tEXIT  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 2797 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tFACTORY  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 2803 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tGLOBAL  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 2809 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tGO  */
#line 352 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 2815 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tIF  */
#line 353 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 2821 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tINSTANCE  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 2827 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tMACRO  */
#line 355 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 2833 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tMETHOD  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 2839 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tON  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 2845 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tOPEN  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 2851 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tPLAY  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 2857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tPROPERTY  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 2863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tPUT  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 2869 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ID: tTHEN  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 2875 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* ID: tWHEN  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("when"); }
#line 2881 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* idlist: %empty  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 2887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* nonemptyidlist: ID  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 2896 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 2904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* stmt: tENDIF '\n'  */
#line 385 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2910 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* proc: CMDID cmdargs '\n'  */
#line 401 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist)); }
#line 2916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* proc: tPUT cmdargs '\n'  */
#line 402 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist)); }
#line 2922 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* proc: tGO cmdargs '\n'  */
#line 403 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist)); }
#line 2928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* proc: tGO frameargs '\n'  */
#line 404 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist)); }
#line 2934 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* proc: tPLAY cmdargs '\n'  */
#line 405 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 2940 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* proc: tPLAY frameargs '\n'  */
#line 406 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist)); }
#line 2946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: tOPEN cmdargs '\n'  */
#line 407 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist)); }
#line 2952 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: tOPEN expr tWITH expr '\n'  */
#line 408 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args); }
#line 2962 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tNEXT tREPEAT '\n'  */
#line 413 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 2968 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tEXIT tREPEAT '\n'  */
#line 414 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 2974 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tEXIT '\n'  */
#line 415 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 2980 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tASSERTERROR stmtoneliner  */
#line 416 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 2986 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* cmdargs: %empty  */
#line 419 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 2994 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* cmdargs: expr  */
#line 422 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3004 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* cmdargs: expr ',' nonemptyexprlist  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                                {
		// This matches `cmd args, ...)
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 3013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* cmdargs: expr expr_nounarymath  */
#line 431 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist  */
#line 437 "engines/director/lingo/lingo-gr.y"
                                                           {
		// This matches `cmd arg arg, ...`
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-2].node));
		(yyvsp[0].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[0].nodelist); }
#line 3034 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* cmdargs: '(' ')'  */
#line 442 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3042 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* cmdargs: '(' expr ',' nonemptyexprlist ')'  */
#line 445 "engines/director/lingo/lingo-gr.y"
                                                  {
		// This matches `cmd(args, ...)`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3051 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* frameargs: tFRAME expr  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* frameargs: tMOVIE expr  */
#line 460 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3072 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 466 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3083 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* frameargs: expr tOF tMOVIE expr  */
#line 472 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* frameargs: tFRAME expr expr_nounarymath  */
#line 478 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3105 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 486 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3111 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 487 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3117 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 488 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3123 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* asgn: tSET varorthe to expr '\n'  */
#line 489 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3129 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* definevars: tGLOBAL idlist '\n'  */
#line 494 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3135 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* definevars: tPROPERTY idlist '\n'  */
#line 495 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* definevars: tINSTANCE idlist '\n'  */
#line 496 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3147 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* ifstmt: tIF expr tTHEN stmt  */
#line 499 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3156 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif tENDIF '\n'  */
#line 503 "engines/director/lingo/lingo-gr.y"
                                                            {
		(yyval.node) = new IfStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist)); }
#line 3163 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 507 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3174 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif tENDIF '\n'  */
#line 513 "engines/director/lingo/lingo-gr.y"
                                                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-5].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), stmtlist1, (yyvsp[-2].nodelist)); }
#line 3183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 517 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3192 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif tENDIF '\n'  */
#line 521 "engines/director/lingo/lingo-gr.y"
                                                                                                               {
		(yyval.node) = new IfElseStmtNode((yyvsp[-8].node), (yyvsp[-5].nodelist), (yyvsp[-2].nodelist)); }
#line 3199 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 525 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3206 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 527 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 529 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 531 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3227 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* tell: tTELL expr tTO stmtoneliner  */
#line 535 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3236 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3243 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* when: tWHEN ID tTHEN expr  */
#line 543 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new WhenNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3249 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* stmtlist: %empty  */
#line 545 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3255 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* nonemptystmtlist: stmtlistline  */
#line 550 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3266 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 556 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* stmtlistline: '\n'  */
#line 563 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3282 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* stmtlist_insideif: %empty  */
#line 567 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 3288 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 572 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3299 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 578 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3309 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* stmtlistline_insideif: '\n'  */
#line 585 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3315 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* simpleexpr_nounarymath: tINT  */
#line 592 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3321 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* simpleexpr_nounarymath: tFLOAT  */
#line 593 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3327 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* simpleexpr_nounarymath: tSYMBOL  */
#line 594 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* simpleexpr_nounarymath: tSTRING  */
#line 595 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 596 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* simpleexpr_nounarymath: ID '(' exprlist ')'  */
#line 597 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new FuncNode((yyvsp[-3].s), (yyvsp[-1].nodelist)); }
#line 3351 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 598 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3357 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* var: ID  */
#line 606 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3363 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* chunk: tFIELD simpleexpr  */
#line 616 "engines/director/lingo/lingo-gr.y"
                                {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("field"), args); }
#line 3372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* chunk: tCAST simpleexpr  */
#line 620 "engines/director/lingo/lingo-gr.y"
                                        {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("cast"), args); }
#line 3381 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* chunk: tCHAR simpleexpr tOF simpleexpr  */
#line 624 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* chunk: tCHAR simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 626 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3395 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189: /* chunk: tWORD simpleexpr tOF simpleexpr  */
#line 628 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3402 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* chunk: tWORD simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 630 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3409 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 191: /* chunk: tITEM simpleexpr tOF simpleexpr  */
#line 632 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3416 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* chunk: tITEM simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 634 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3423 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* chunk: tLINE simpleexpr tOF simpleexpr  */
#line 636 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3430 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 194: /* chunk: tLINE simpleexpr tTO simpleexpr tOF simpleexpr  */
#line 638 "engines/director/lingo/lingo-gr.y"
                                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3437 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* object: tSCRIPT simpleexpr  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                        {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("script"), args); }
#line 3446 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* object: tWINDOW simpleexpr  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("window"), args); }
#line 3455 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* the: tTHE tLAST chunktype inof simpleexpr  */
#line 655 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 3461 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 660 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3467 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* theobj: tSOUND simpleexpr  */
#line 661 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* theobj: tSPRITE simpleexpr  */
#line 662 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3479 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* menu: tMENU simpleexpr  */
#line 665 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3485 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* writablethe: tTHE ID  */
#line 667 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3491 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* writablethe: tTHE ID tOF theobj  */
#line 668 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3497 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* writablethe: tTHE tNUMBER tOF theobj  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 3503 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 672 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3509 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 673 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3515 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* thedatetime: tTHE tABBREV tDATE  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3521 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* thedatetime: tTHE tABBREV tTIME  */
#line 675 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3527 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* thedatetime: tTHE tABBR tDATE  */
#line 676 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3533 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* thedatetime: tTHE tABBR tTIME  */
#line 677 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3539 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* thedatetime: tTHE tLONG tDATE  */
#line 678 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 3545 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* thedatetime: tTHE tLONG tTIME  */
#line 679 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 3551 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* thedatetime: tTHE tSHORT tDATE  */
#line 680 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 3557 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* thedatetime: tTHE tSHORT tTIME  */
#line 681 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 3563 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 685 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 3569 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 221: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 686 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 3575 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 222: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 687 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 3581 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 223: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 688 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 3587 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 224: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 689 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 3593 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 225: /* chunktype: tCHAR  */
#line 692 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 3599 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 226: /* chunktype: tWORD  */
#line 693 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 3605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* chunktype: tITEM  */
#line 694 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 3611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* chunktype: tLINE  */
#line 695 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 3617 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* list: '[' exprlist ']'  */
#line 700 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 3623 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* list: '[' ':' ']'  */
#line 701 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 3629 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* list: '[' proplist ']'  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 3635 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* proplist: proppair  */
#line 705 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 3644 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* proplist: proplist ',' proppair  */
#line 709 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3652 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* proppair: tSYMBOL ':' expr  */
#line 714 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3658 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* proppair: ID ':' expr  */
#line 715 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3664 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* proppair: tSTRING ':' expr  */
#line 716 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 3670 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* unarymath: '+' simpleexpr  */
#line 719 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 3676 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* unarymath: '-' simpleexpr  */
#line 720 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 3682 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* expr: expr '+' expr  */
#line 730 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3688 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* expr: expr '-' expr  */
#line 731 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3694 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* expr: expr '*' expr  */
#line 732 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3700 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* expr: expr '/' expr  */
#line 733 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3706 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* expr: expr tMOD expr  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3712 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* expr: expr '>' expr  */
#line 735 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3718 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* expr: expr '<' expr  */
#line 736 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3724 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* expr: expr tEQ expr  */
#line 737 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3730 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* expr: expr tNEQ expr  */
#line 738 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3736 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* expr: expr tGE expr  */
#line 739 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3742 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* expr: expr tLE expr  */
#line 740 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3748 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* expr: expr tAND expr  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3754 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* expr: expr tOR expr  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3760 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* expr: expr '&' expr  */
#line 743 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3766 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* expr: expr tCONCAT expr  */
#line 744 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3772 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* expr: expr tCONTAINS expr  */
#line 745 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3778 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 261: /* expr: expr tSTARTS expr  */
#line 746 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3784 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 264: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 755 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3790 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 265: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 756 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3796 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 757 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3802 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 758 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3808 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 759 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3814 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 760 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3820 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 270: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 761 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3826 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 271: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 762 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3832 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 763 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3838 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 764 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3844 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 765 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3850 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 766 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3856 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 767 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3862 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 768 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 769 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 770 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3880 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 771 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3886 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 774 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3892 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 775 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3898 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 283: /* exprlist: %empty  */
#line 778 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 3904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* nonemptyexprlist: expr  */
#line 782 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 3913 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 786 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3921 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3925 "engines/director/lingo/lingo-gr.cpp"

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

#line 791 "engines/director/lingo/lingo-gr.y"


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
