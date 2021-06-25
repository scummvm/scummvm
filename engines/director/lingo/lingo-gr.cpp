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
  YYSYMBOL_tDELETE = 15,                   /* tDELETE  */
  YYSYMBOL_tDOWN = 16,                     /* tDOWN  */
  YYSYMBOL_tELSE = 17,                     /* tELSE  */
  YYSYMBOL_tELSIF = 18,                    /* tELSIF  */
  YYSYMBOL_tEXIT = 19,                     /* tEXIT  */
  YYSYMBOL_tFRAME = 20,                    /* tFRAME  */
  YYSYMBOL_tGLOBAL = 21,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 22,                       /* tGO  */
  YYSYMBOL_tHILITE = 23,                   /* tHILITE  */
  YYSYMBOL_tIF = 24,                       /* tIF  */
  YYSYMBOL_tIN = 25,                       /* tIN  */
  YYSYMBOL_tINTO = 26,                     /* tINTO  */
  YYSYMBOL_tMACRO = 27,                    /* tMACRO  */
  YYSYMBOL_tMOVIE = 28,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 29,                     /* tNEXT  */
  YYSYMBOL_tOF = 30,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 31,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 32,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 33,                   /* tREPEAT  */
  YYSYMBOL_tSET = 34,                      /* tSET  */
  YYSYMBOL_tTHEN = 35,                     /* tTHEN  */
  YYSYMBOL_tTO = 36,                       /* tTO  */
  YYSYMBOL_tWHEN = 37,                     /* tWHEN  */
  YYSYMBOL_tWITH = 38,                     /* tWITH  */
  YYSYMBOL_tWHILE = 39,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 40,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 41,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 42,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 43,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 44,                       /* tGE  */
  YYSYMBOL_tLE = 45,                       /* tLE  */
  YYSYMBOL_tEQ = 46,                       /* tEQ  */
  YYSYMBOL_tNEQ = 47,                      /* tNEQ  */
  YYSYMBOL_tAND = 48,                      /* tAND  */
  YYSYMBOL_tOR = 49,                       /* tOR  */
  YYSYMBOL_tNOT = 50,                      /* tNOT  */
  YYSYMBOL_tMOD = 51,                      /* tMOD  */
  YYSYMBOL_tAFTER = 52,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 53,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 54,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 55,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 56,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 57,                     /* tCHAR  */
  YYSYMBOL_tCHARS = 58,                    /* tCHARS  */
  YYSYMBOL_tITEM = 59,                     /* tITEM  */
  YYSYMBOL_tITEMS = 60,                    /* tITEMS  */
  YYSYMBOL_tLINE = 61,                     /* tLINE  */
  YYSYMBOL_tLINES = 62,                    /* tLINES  */
  YYSYMBOL_tWORD = 63,                     /* tWORD  */
  YYSYMBOL_tWORDS = 64,                    /* tWORDS  */
  YYSYMBOL_tABBREVIATED = 65,              /* tABBREVIATED  */
  YYSYMBOL_tABBREV = 66,                   /* tABBREV  */
  YYSYMBOL_tABBR = 67,                     /* tABBR  */
  YYSYMBOL_tLONG = 68,                     /* tLONG  */
  YYSYMBOL_tSHORT = 69,                    /* tSHORT  */
  YYSYMBOL_tCASTMEMBERS = 70,              /* tCASTMEMBERS  */
  YYSYMBOL_tDATE = 71,                     /* tDATE  */
  YYSYMBOL_tLAST = 72,                     /* tLAST  */
  YYSYMBOL_tMENU = 73,                     /* tMENU  */
  YYSYMBOL_tMENUITEM = 74,                 /* tMENUITEM  */
  YYSYMBOL_tMENUITEMS = 75,                /* tMENUITEMS  */
  YYSYMBOL_tNUMBER = 76,                   /* tNUMBER  */
  YYSYMBOL_tTHE = 77,                      /* tTHE  */
  YYSYMBOL_tTIME = 78,                     /* tTIME  */
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
  YYSYMBOL_98_ = 98,                       /* '%'  */
  YYSYMBOL_99_n_ = 99,                     /* '\n'  */
  YYSYMBOL_100_ = 100,                     /* ','  */
  YYSYMBOL_101_ = 101,                     /* '('  */
  YYSYMBOL_102_ = 102,                     /* ')'  */
  YYSYMBOL_103_ = 103,                     /* '['  */
  YYSYMBOL_104_ = 104,                     /* ']'  */
  YYSYMBOL_105_ = 105,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 106,                 /* $accept  */
  YYSYMBOL_script = 107,                   /* script  */
  YYSYMBOL_scriptpartlist = 108,           /* scriptpartlist  */
  YYSYMBOL_scriptpart = 109,               /* scriptpart  */
  YYSYMBOL_macro = 110,                    /* macro  */
  YYSYMBOL_factory = 111,                  /* factory  */
  YYSYMBOL_method = 112,                   /* method  */
  YYSYMBOL_methodlist = 113,               /* methodlist  */
  YYSYMBOL_nonemptymethodlist = 114,       /* nonemptymethodlist  */
  YYSYMBOL_methodlistline = 115,           /* methodlistline  */
  YYSYMBOL_handler = 116,                  /* handler  */
  YYSYMBOL_endargdef = 117,                /* endargdef  */
  YYSYMBOL_CMDID = 118,                    /* CMDID  */
  YYSYMBOL_ID = 119,                       /* ID  */
  YYSYMBOL_idlist = 120,                   /* idlist  */
  YYSYMBOL_nonemptyidlist = 121,           /* nonemptyidlist  */
  YYSYMBOL_stmt = 122,                     /* stmt  */
  YYSYMBOL_stmt_insideif = 123,            /* stmt_insideif  */
  YYSYMBOL_stmtoneliner = 124,             /* stmtoneliner  */
  YYSYMBOL_proc = 125,                     /* proc  */
  YYSYMBOL_cmdargs = 126,                  /* cmdargs  */
  YYSYMBOL_trailingcomma = 127,            /* trailingcomma  */
  YYSYMBOL_frameargs = 128,                /* frameargs  */
  YYSYMBOL_asgn = 129,                     /* asgn  */
  YYSYMBOL_to = 130,                       /* to  */
  YYSYMBOL_definevars = 131,               /* definevars  */
  YYSYMBOL_ifstmt = 132,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 133,               /* ifelsestmt  */
  YYSYMBOL_endif = 134,                    /* endif  */
  YYSYMBOL_loop = 135,                     /* loop  */
  YYSYMBOL_tell = 136,                     /* tell  */
  YYSYMBOL_when = 137,                     /* when  */
  YYSYMBOL_stmtlist = 138,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 139,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 140,             /* stmtlistline  */
  YYSYMBOL_stmtlist_insideif = 141,        /* stmtlist_insideif  */
  YYSYMBOL_nonemptystmtlist_insideif = 142, /* nonemptystmtlist_insideif  */
  YYSYMBOL_stmtlistline_insideif = 143,    /* stmtlistline_insideif  */
  YYSYMBOL_simpleexpr_nounarymath = 144,   /* simpleexpr_nounarymath  */
  YYSYMBOL_var = 145,                      /* var  */
  YYSYMBOL_varorchunk = 146,               /* varorchunk  */
  YYSYMBOL_varorthe = 147,                 /* varorthe  */
  YYSYMBOL_chunk = 148,                    /* chunk  */
  YYSYMBOL_chunktype = 149,                /* chunktype  */
  YYSYMBOL_object = 150,                   /* object  */
  YYSYMBOL_refargs = 151,                  /* refargs  */
  YYSYMBOL_the = 152,                      /* the  */
  YYSYMBOL_theobj = 153,                   /* theobj  */
  YYSYMBOL_menu = 154,                     /* menu  */
  YYSYMBOL_thedatetime = 155,              /* thedatetime  */
  YYSYMBOL_thenumberof = 156,              /* thenumberof  */
  YYSYMBOL_inof = 157,                     /* inof  */
  YYSYMBOL_writablethe = 158,              /* writablethe  */
  YYSYMBOL_writabletheobj = 159,           /* writabletheobj  */
  YYSYMBOL_list = 160,                     /* list  */
  YYSYMBOL_proplist = 161,                 /* proplist  */
  YYSYMBOL_proppair = 162,                 /* proppair  */
  YYSYMBOL_unarymath = 163,                /* unarymath  */
  YYSYMBOL_simpleexpr = 164,               /* simpleexpr  */
  YYSYMBOL_expr = 165,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 166,         /* expr_nounarymath  */
  YYSYMBOL_expr_noeq = 167,                /* expr_noeq  */
  YYSYMBOL_sprite = 168,                   /* sprite  */
  YYSYMBOL_exprlist = 169,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 170          /* nonemptyexprlist  */
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
#define YYFINAL  189
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4539

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  325
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  607

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
      99,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    98,    93,     2,
     101,   102,    96,    94,   100,    95,     2,    97,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   105,     2,
      91,     2,    92,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   103,     2,   104,     2,     2,     2,     2,     2,     2,
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
       0,   196,   196,   198,   204,   211,   212,   213,   214,   215,
     244,   248,   250,   252,   253,   256,   262,   269,   270,   275,
     279,   283,   284,   285,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   369,   370,   371,
     374,   378,   388,   389,   392,   393,   394,   395,   396,   397,
     400,   401,   402,   405,   406,   407,   408,   409,   410,   411,
     412,   417,   418,   419,   420,   421,   422,   425,   428,   433,
     437,   443,   448,   451,   456,   462,   462,   468,   473,   479,
     485,   491,   499,   500,   501,   502,   505,   505,   507,   508,
     509,   512,   516,   520,   526,   530,   534,   538,   539,   541,
     543,   545,   547,   551,   555,   559,   561,   562,   566,   572,
     579,   580,   583,   584,   588,   594,   601,   602,   608,   609,
     610,   611,   612,   613,   614,   615,   616,   617,   618,   619,
     620,   623,   625,   626,   629,   630,   633,   634,   635,   637,
     639,   641,   643,   645,   647,   649,   651,   654,   655,   656,
     657,   660,   661,   664,   669,   672,   677,   683,   684,   685,
     686,   687,   690,   691,   692,   693,   694,   697,   699,   700,
     701,   702,   703,   704,   705,   706,   707,   708,   712,   713,
     714,   715,   716,   719,   719,   721,   722,   725,   726,   727,
     728,   729,   732,   733,   734,   737,   741,   746,   747,   748,
     751,   752,   755,   756,   760,   761,   762,   763,   764,   765,
     766,   767,   768,   769,   770,   771,   772,   773,   774,   775,
     776,   777,   778,   785,   786,   787,   788,   789,   790,   791,
     792,   793,   794,   795,   796,   797,   798,   799,   800,   801,
     802,   803,   806,   807,   808,   809,   810,   811,   812,   813,
     814,   815,   816,   817,   818,   819,   820,   821,   822,   823,
     826,   827,   830,   831,   834,   838
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
  "tFIELD", "tSCRIPT", "tWINDOW", "tDELETE", "tDOWN", "tELSE", "tELSIF",
  "tEXIT", "tFRAME", "tGLOBAL", "tGO", "tHILITE", "tIF", "tIN", "tINTO",
  "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT",
  "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tFACTORY", "tOPEN",
  "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT",
  "tMOD", "tAFTER", "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR",
  "tCHARS", "tITEM", "tITEMS", "tLINE", "tLINES", "tWORD", "tWORDS",
  "tABBREVIATED", "tABBREV", "tABBR", "tLONG", "tSHORT", "tCASTMEMBERS",
  "tDATE", "tLAST", "tMENU", "tMENUITEM", "tMENUITEMS", "tNUMBER", "tTHE",
  "tTIME", "tSOUND", "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL",
  "tPROPERTY", "tON", "tMETHOD", "tENDIF", "tENDREPEAT", "tENDTELL",
  "tASSERTERROR", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'", "'/'", "'%'",
  "'\\n'", "','", "'('", "')'", "'['", "']'", "':'", "$accept", "script",
  "scriptpartlist", "scriptpart", "macro", "factory", "method",
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
     345,    60,    62,    38,    43,    45,    42,    47,    37,    10,
      44,    40,    41,    91,    93,    58
};
#endif

#define YYPACT_NINF (-518)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2949,  -518,  -518,  -518,  -518,  -518,    26,  -518,   -17,  -518,
    3528,  1866,    26,  1965,  -518,  -518,  3528,  -518,     7,  -518,
    -518,  2064,    41,  3609,  -518,  3528,  -518,  -518,  3528,  2064,
    1866,  3528,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,
    -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,
    -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,
    -518,  -518,  -518,  -518,  -518,  1965,  3528,  3528,   -50,  3851,
    -518,    51,  2949,  -518,  -518,  -518,  -518,  2064,  -518,  -518,
    -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  2163,
    2163,  1965,  1965,  1965,  1965,    -7,   -37,   -31,  -518,  -518,
    -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,
    -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,
    -518,  -518,  -518,  -518,  -518,    23,    -1,  -518,  -518,  -518,
    -518,  2163,  2163,  2163,  2163,  1965,  1965,  2262,  1965,  1965,
    1965,  1965,  3690,  1965,  2262,  2262,  1371,   775,     3,    25,
      34,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,
    -518,   876,  -518,    52,  1965,  4078,  3528,    75,    77,   975,
    3528,  1965,  3528,  -518,  -518,    -4,  -518,    61,    78,    82,
    1074,    90,    92,   101,  4045,   108,  3528,  -518,  -518,  -518,
    -518,   109,  1173,  1470,  -518,  -518,  -518,   434,   555,   681,
    3904,   160,  -518,  -518,  -518,  3528,  -518,  -518,  1272,  4388,
    -518,   -32,   -11,    -5,    17,    20,   160,    95,   180,  4359,
    -518,  -518,  -518,   612,    85,   111,   118,    58,   -10,  -518,
    4388,   121,   126,  1569,  -518,  -518,   204,  1965,  1965,  1965,
    1965,  2658,  2658,  2856,  1965,  2757,  2757,  1965,  1965,  1965,
    1965,  1965,  1965,  1965,  1965,  -518,  -518,  4160,  -518,  -518,
    4132,  3030,  1965,  1965,  1965,  1965,  1965,   131,  -518,  -518,
    3771,  3771,  3771,     4,  4191,   206,  -518,  -518,  1965,  1965,
     -42,  -518,  1965,  -518,  -518,  -518,  3851,  3111,  -518,   138,
    -518,  -518,  4101,  2262,  1965,  2262,  1965,  2262,  1965,  2262,
    1965,  -518,  -518,  -518,  -518,    18,  -518,   214,  4413,  -518,
    -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  -518,  2361,
    2460,  2262,  2262,  1668,  -518,  1965,  1965,  -518,  1965,  3447,
    -518,  -518,  1965,  -518,   143,  1965,    72,    72,    72,    72,
      91,    91,  -518,    35,    72,    72,    72,    72,    35,   -33,
     -33,  -518,  -518,   143,  1965,  1965,  1965,  1965,  1965,  1965,
    1965,  1965,  1965,  1965,  1965,  1965,  1965,  1965,  1965,  1965,
    1965,  1965,  -518,  3354,   228,  3111,    -7,  -518,   147,  -518,
     148,   149,  1965,  1965,  3111,  2559,  4217,  4388,  3528,  -518,
    -518,  -518,   -42,  -518,  4247,  -518,  -518,  -518,   161,  3111,
    -518,  3111,  1767,  -518,  3917,  -518,  3937,  -518,  3991,  -518,
    4011,  -518,  -518,  2262,  1965,    18,    18,    18,    18,  2262,
    2262,    18,  2262,  2262,  -518,  -518,  -518,  -518,  -518,  -518,
    -518,   143,  4388,  4388,  4388,    85,   111,   150,  -518,  4388,
    1965,   152,  4388,  -518,    72,    72,    72,    72,    91,    91,
    -518,    35,    72,    72,    72,    72,    35,   -33,   -33,  -518,
    -518,   143,  -518,  -518,    -6,  3354,  -518,  3192,  -518,  -518,
    -518,  -518,  4273,   358,   169,  1965,  1965,  1965,  1965,  -518,
    -518,  -518,  3528,  -518,  -518,   159,  -518,   249,  -518,   143,
    2262,  2262,  2262,  2262,  -518,  4388,  2262,  2262,  2262,  2262,
    -518,   235,   197,  -518,  -518,   171,  -518,  -518,  3273,   172,
    -518,  -518,  3354,  -518,  3111,   238,  1965,   183,  -518,  4442,
    -518,  4024,  4442,  4442,   185,  -518,  3528,   181,  -518,  -518,
    -518,  -518,  -518,  -518,  -518,  -518,   212,  2262,  -518,  -518,
    3354,  -518,  -518,   203,   201,  1965,  4303,  -518,  1965,  1965,
    1965,  1965,  1965,  1965,  1965,  1965,  1965,  1965,  1965,  1965,
    1965,  1965,  1965,  1965,   220,  3111,     2,  -518,  -518,  2262,
     203,  -518,   195,  4329,  3111,    99,    99,    99,   184,   184,
    -518,    76,    99,    99,    99,    99,    76,   -26,   -26,  -518,
    -518,  1965,  -518,  -518,  3528,  -518,  -518,  -518,  3111,   207,
    4442,  -518,   208,   198,   199,  -518,  -518
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    24,    31,    38,    62,    72,    36,    37,     0,    39,
      97,   127,    40,     0,    41,    43,     0,    54,    55,    58,
      60,   127,    61,     0,    70,     0,    73,    71,     0,   127,
     127,    97,    29,    59,    56,    53,    28,    30,    34,    67,
      32,    33,    44,    45,    47,    48,    75,    76,    25,    26,
      27,    49,    64,    35,    46,    50,    51,    52,    57,    68,
      69,    65,    66,    42,    74,     0,    97,     0,     0,    63,
       5,     0,     2,     3,     6,     7,     8,   127,     9,   102,
     104,   110,   111,   112,   105,   106,   107,   108,   109,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   123,    79,
      36,    78,    80,    82,    83,    40,    84,    86,    55,    92,
      61,    93,    95,    96,    81,    89,    90,    85,    94,    91,
      88,    87,    63,    77,   100,     0,    98,   178,   179,   181,
     180,    31,    38,    62,    72,    39,    54,    56,    32,    44,
      47,    75,    68,    66,     0,     0,     0,   322,   191,     0,
       0,   262,   186,   187,   188,   189,   220,   221,   190,   263,
     264,   135,   265,     0,     0,     0,    97,     0,     0,   135,
       0,     0,    68,   191,   194,     0,   195,     0,     0,     0,
     135,     0,     0,     0,     0,     0,    97,   103,   126,     1,
       4,     0,   135,     0,   197,   213,   196,     0,     0,     0,
       0,     0,   124,   122,   148,    99,   211,   212,   137,   138,
     182,    25,    26,    27,    49,    64,    46,    57,   217,     0,
     260,   261,   132,     0,   181,   180,     0,   191,     0,   255,
     324,     0,   323,     0,   115,   116,    58,     0,     0,     0,
       0,    29,    59,    53,     0,    34,    67,     0,     0,     0,
       0,     0,     0,     0,   136,   128,   283,   135,   284,   125,
       0,     0,     0,     0,     0,     0,     0,     0,   121,   114,
      43,    28,    30,     0,     0,   245,   146,   147,     0,     0,
      13,   119,    73,   117,   118,   150,     0,   166,   149,     0,
     113,   214,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   207,   209,   210,   208,     0,   101,    58,   141,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,     0,
       0,     0,     0,     0,   185,     0,     0,   253,     0,     0,
     254,   252,     0,   183,   135,     0,   275,   276,   273,   274,
     277,   278,   270,   280,   281,   282,   272,   271,   279,   266,
     267,   268,   269,   135,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   136,   130,   172,   151,   166,    68,   192,     0,   193,
       0,     0,     0,     0,   166,     0,     0,   165,     0,    17,
      18,    11,    14,    15,     0,   163,   170,   171,     0,   167,
     168,   166,     0,   198,     0,   202,     0,   204,     0,   200,
       0,   243,   244,     0,     0,    33,    45,    48,    76,    50,
      51,    52,    65,    66,   219,   223,   222,   218,   320,   321,
     133,   135,   259,   257,   258,     0,     0,     0,   256,   325,
     136,     0,   140,   129,   294,   295,   292,   293,   296,   297,
     289,   299,   300,   301,   291,   290,   298,   285,   286,   287,
     288,   135,   176,   177,   157,   173,   174,     0,    10,   142,
     143,   144,     0,     0,     0,    50,    51,    65,    66,   246,
     247,   145,    97,    16,   120,     0,   169,    20,   215,   135,
       0,     0,     0,     0,   206,   139,     0,     0,     0,     0,
     227,     0,     0,   225,   226,     0,   184,   131,     0,     0,
     152,   175,   172,   153,   166,     0,     0,     0,   302,   248,
     303,     0,   250,   251,     0,   164,    21,     0,   199,   203,
     205,   201,   238,   240,   241,   239,     0,     0,   242,   134,
     172,   155,   158,   157,     0,     0,     0,   159,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   166,     0,    22,   216,     0,
     157,   154,     0,     0,   166,   312,   313,   311,   314,   315,
     308,   317,   318,   319,   310,   309,   316,   304,   305,   306,
     307,     0,    12,    19,     0,   224,   156,   162,   166,     0,
     249,    23,     0,     0,     0,   160,   161
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -518,  -518,  -518,   227,  -518,  -518,  -518,  -518,  -518,   -75,
    -518,  -518,     0,    -8,   -25,  -518,     5,  -360,   -62,  -518,
      -2,  -256,   288,  -518,  -518,  -518,  -518,  -518,  -517,  -518,
    -518,  -518,  -354,  -518,   -80,  -488,  -518,  -145,  -147,   -19,
    -128,  -518,    -3,  -518,  -518,   -78,  -518,    10,  -180,  -518,
    -518,  -203,  -518,  -518,  -518,  -518,     8,  -518,   202,    63,
     115,  -442,    80,  -518,  -223
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    71,    72,    73,    74,    75,   390,   391,   392,   393,
      76,   566,   123,   148,   125,   126,   397,    79,    80,    81,
     149,   255,   150,    82,   278,    83,    84,    85,   510,    86,
      87,    88,   398,   399,   400,   464,   465,   466,   151,   152,
     378,   175,   153,   305,   154,   194,   155,   424,   425,   156,
     157,   413,   176,   479,   158,   228,   229,   159,   160,   230,
     257,   519,   162,   231,   232
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      77,   372,   124,    96,   174,    78,   183,   188,   166,   163,
     334,   508,   196,   463,   256,   173,    97,   177,   264,   168,
     178,   468,   256,   124,   543,   553,   571,   179,   181,   382,
     474,   353,   276,   256,   521,   522,   523,    89,    90,   309,
     167,   185,   277,   411,   388,   256,   310,   487,   412,   187,
     383,   189,   570,   596,   196,   206,   207,   389,   124,   186,
     311,   256,   202,   252,   253,   201,   313,   312,   203,    77,
     562,   563,    77,   314,   161,   191,   165,    78,   441,   170,
     171,   509,    98,    91,   169,    92,   264,    93,   315,    94,
     329,   317,   180,   161,   330,   316,   279,   443,   318,   205,
     431,   593,   594,    95,   233,   463,   575,   576,   577,   578,
     579,   580,   581,   582,   583,   584,   585,   586,   587,   588,
     589,   590,   204,   264,   234,   319,   244,   553,   184,   250,
     251,   252,   253,   235,   218,   237,   238,   239,   240,   227,
     192,   267,   264,   380,   381,   244,   265,   266,   461,   600,
     553,   259,   463,   554,   197,   198,   199,   200,   124,   233,
     544,   289,   273,   328,   275,   249,   250,   251,   252,   253,
     560,   561,   562,   563,   268,   505,   269,   280,   124,   489,
     463,   281,   247,   248,   249,   250,   251,   252,   253,   283,
     325,   284,   559,   560,   561,   562,   563,   306,   208,   209,
     285,   197,   198,   199,   200,   507,   219,   288,   290,   223,
     320,   592,   496,   497,   498,   499,   326,   301,   502,   302,
     599,   303,   327,   304,   395,   331,   332,   260,   548,   549,
     375,   550,   335,   527,   274,   553,   385,   401,   554,   555,
     556,   258,   414,   440,   602,   467,   469,   470,   471,   258,
     485,   377,   377,   377,   506,   328,   292,   517,   525,   526,
     258,    77,   173,   173,   173,   536,   374,   379,   379,   379,
     537,   542,   258,   539,   545,   557,   558,   559,   560,   561,
     562,   563,   547,   568,   565,   569,    77,    77,   258,   572,
     509,   195,   195,   591,   597,   603,   604,   605,   606,   190,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   483,   182,   486,
     511,   437,   538,   308,     0,   340,   341,   342,   344,   345,
     427,     0,     0,   195,   195,   195,   195,   438,     0,   210,
       0,   386,   387,     0,     0,   394,   220,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   404,     0,   406,
       0,   408,     0,   410,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    77,   515,    77,     0,     0,     0,     0,
     482,     0,     0,     0,    77,     0,     0,     0,   432,   433,
       0,   434,     0,     0,   516,   439,     0,     0,   442,    77,
       0,    77,   237,   238,   239,   240,   262,   263,     0,   264,
       0,     0,   244,   265,   266,     0,     0,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   472,   473,     0,     0,   247,
     248,   249,   250,   251,   252,   253,     0,   524,     0,     0,
       0,     0,     0,     0,   293,    77,     0,    77,     0,     0,
     294,     0,   513,     0,   124,     0,     0,   495,   237,   238,
     239,   240,   262,   263,     0,   264,     0,     0,   244,   265,
     266,     0,     0,     0,     0,   403,     0,   405,     0,   407,
       0,   409,     0,   439,     0,     0,     0,     0,    77,     0,
       0,     0,    77,   541,    77,     0,     0,     0,   567,     0,
       0,   426,   426,   428,   429,   247,   248,   249,   250,   251,
     252,   253,     0,     0,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   520,   520,   520,   520,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   546,
       0,     0,     0,     0,     0,   295,   601,   480,     0,     0,
       0,   296,     0,     0,     0,     0,     0,     0,    77,   237,
     238,   239,   240,   262,   263,     0,   264,     0,   573,   244,
     265,   266,     0,     0,     0,   494,     0,     0,     0,     0,
       0,   500,   501,     0,   503,   504,     0,     0,   520,   520,
     520,   520,   520,   520,   520,   520,   520,   520,   520,   520,
     520,   520,   520,   520,     0,     0,   247,   248,   249,   250,
     251,   252,   253,     0,     0,     0,   237,   238,   239,   240,
     262,   263,     0,   264,     0,     0,   244,   265,   266,     0,
       0,   520,     0,     0,     0,     0,     0,   518,   518,   518,
     518,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   528,   529,   530,   531,     0,     0,   532,   533,
     534,   535,     0,   247,   248,   249,   250,   251,   252,   253,
       0,   297,   323,     0,   324,     0,     0,   298,     0,     0,
       0,     0,     0,     0,     0,   237,   238,   239,   240,   262,
     263,     0,   264,     0,     0,   244,   265,   266,     0,   500,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     518,   518,   518,   518,   518,   518,   518,   518,   518,   518,
     518,   518,   518,   518,   518,   518,     0,     0,     0,     0,
       0,   595,   247,   248,   249,   250,   251,   252,   253,     0,
     127,   128,     1,   224,   225,    99,   131,   132,   133,   134,
     100,     7,   101,   518,   102,     9,   103,   104,   105,   106,
      14,    15,   107,    17,   108,    19,    20,   109,   110,   111,
     112,    24,   113,    26,    27,   114,   115,   116,   117,     0,
       0,     0,     0,    32,    33,   137,    35,    36,    37,     0,
      38,    39,   138,    41,   139,    43,   140,    45,   141,    47,
      48,    49,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,   142,    60,    61,   143,    63,    64,   118,   119,
     120,   121,     0,     0,     0,   122,     0,     0,     0,   144,
     145,     0,     0,     0,     0,     0,   164,     0,   147,     0,
     226,   127,   128,     1,   129,   130,    99,   131,   132,   133,
     134,   100,     7,   101,     0,   102,     9,   103,   104,   105,
     106,    14,    15,   107,    17,   108,   236,    20,   109,   110,
     111,   112,    24,   113,    26,    27,   114,   115,   116,   117,
     237,   238,   239,   240,   241,   242,   137,   243,    36,    37,
     244,   245,   246,   138,    41,   139,    43,   140,    45,   141,
      47,    48,    49,    50,    51,    52,     0,    53,    54,    55,
      56,    57,    58,   142,    60,    61,   143,    63,    64,   118,
     119,   120,   121,     0,     0,     0,   122,   247,   248,   249,
     250,   251,   252,   253,     0,     0,   254,   164,     0,   147,
     127,   128,     1,   129,   130,    99,   131,   132,   133,   134,
     100,     7,   101,     0,   102,     9,   103,   104,   105,   106,
      14,   270,   107,    17,   108,    19,    20,   109,   110,   111,
     112,    24,   113,    26,    27,   114,   115,   116,   117,   237,
     238,   239,   240,   241,   242,   137,   243,   271,   272,   244,
     245,   246,   138,    41,   139,    43,   140,    45,   141,    47,
      48,    49,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,   142,    60,    61,   143,    63,    64,   118,   119,
     120,   121,     0,     0,     0,   122,   247,   248,   249,   250,
     251,   252,   253,     0,     0,   254,   164,     0,   147,   127,
     128,     1,   129,   130,    99,   131,   132,   133,   134,   100,
       7,   101,     0,   102,     9,   103,   104,   105,   106,    14,
      15,   107,    17,   108,    19,    20,   109,   110,   111,   112,
      24,   113,   282,    27,   114,   115,   116,   117,   237,   238,
     239,   240,   241,   242,   137,   243,    36,    37,   244,   245,
     246,   138,    41,   139,    43,   140,    45,   141,    47,    48,
      49,    50,    51,    52,     0,    53,    54,    55,    56,    57,
      58,   142,    60,    61,   143,    63,    64,   118,   119,   120,
     121,     0,     0,     0,   122,   247,   248,   249,   250,   251,
     252,   253,     0,     0,   254,   164,     0,   147,   127,   128,
       1,   129,   130,    99,   131,   132,   133,   134,   100,     7,
     101,     0,   102,     9,   103,   104,   105,   106,    14,    15,
     107,    17,   108,    19,    20,   109,   110,   111,   112,    24,
     113,    26,    27,   114,   115,   116,   117,   237,   238,   239,
     240,   241,   242,   137,   243,    36,    37,   244,   245,   246,
     138,    41,   139,    43,   140,    45,   141,    47,    48,    49,
      50,    51,    52,     0,    53,    54,    55,    56,    57,    58,
     142,    60,    61,   143,    63,    64,   118,   119,   120,   121,
       0,     0,     0,   122,   247,   248,   249,   250,   251,   252,
     253,     0,     0,   254,   164,     0,   147,   127,   128,     1,
     129,   130,    99,   131,   132,   133,   134,   100,     7,   101,
       0,   102,     9,   103,   104,   105,   106,    14,    15,   107,
      17,   108,   307,    20,   109,   110,   111,   112,    24,   113,
      26,    27,   114,   115,   116,   117,   237,   238,   239,   240,
     241,   242,   137,   243,    36,    37,   244,   245,   246,   138,
      41,   139,    43,   140,    45,   141,    47,    48,    49,    50,
      51,    52,     0,    53,    54,    55,    56,    57,    58,   142,
      60,    61,   143,    63,    64,   118,   119,   120,   121,     0,
       0,     0,   122,   247,   248,   249,   250,   251,   252,   253,
       0,     0,     0,   164,     0,   147,   127,   128,     1,   129,
     130,    99,   131,   132,   133,   134,   100,     7,   101,     0,
     102,     9,   103,   104,   105,   106,    14,    15,   107,    17,
     108,    19,    20,   109,   110,   111,   112,    24,   113,    26,
      27,   114,   115,   116,   117,     0,     0,     0,     0,    32,
      33,   137,    35,    36,    37,     0,    38,    39,   138,    41,
     139,    43,   140,    45,   141,    47,    48,    49,    50,    51,
      52,     0,    53,    54,    55,    56,    57,    58,   142,    60,
      61,   143,    63,    64,   118,   119,   120,   121,     0,     0,
       0,   122,     0,     0,     0,   144,   145,     0,     0,     0,
       0,     0,   164,   222,   147,   127,   128,     1,   129,   130,
      99,   131,   132,   133,   134,   100,     7,   101,     0,   102,
       9,   103,   104,   105,   106,    14,    15,   107,    17,   108,
      19,    20,   109,   110,   111,   112,    24,   113,    26,    27,
     114,   115,   116,   117,     0,     0,     0,     0,    32,    33,
     137,    35,    36,    37,     0,    38,    39,   138,    41,   139,
      43,   140,    45,   141,    47,    48,    49,    50,    51,    52,
       0,    53,    54,    55,    56,    57,    58,   142,    60,    61,
     143,    63,    64,   118,   119,   120,   121,     0,     0,     0,
     122,     0,     0,     0,   144,   145,     0,     0,     0,     0,
       0,   164,   291,   147,   127,   128,     1,   129,   130,    99,
     131,   132,   133,   134,   100,     7,   101,     0,   102,     9,
     103,   104,   105,   106,    14,    15,   107,    17,   108,    19,
      20,   109,   110,   111,   112,    24,   113,    26,    27,   114,
     115,   116,   117,     0,     0,     0,     0,    32,    33,   137,
      35,    36,    37,     0,    38,    39,   138,    41,   139,    43,
     140,    45,   141,    47,    48,    49,    50,    51,    52,     0,
      53,    54,    55,    56,    57,    58,   142,    60,    61,   143,
      63,    64,   118,   119,   120,   121,     0,     0,     0,   122,
       0,     0,     0,   144,   145,     0,     0,     0,     0,     0,
     164,   333,   147,   127,   128,     1,   129,   130,    99,   131,
     132,   133,   134,   100,     7,   101,     0,   102,     9,   103,
     104,   105,   106,    14,    15,   107,    17,   108,    19,    20,
     109,   110,   111,   112,    24,   113,    26,    27,   114,   115,
     116,   117,     0,     0,     0,     0,    32,    33,   137,    35,
      36,    37,     0,    38,    39,   138,    41,   139,    43,   140,
      45,   141,    47,    48,    49,    50,    51,    52,     0,    53,
      54,    55,    56,    57,    58,   142,    60,    61,   143,    63,
      64,   118,   119,   120,   121,     0,     0,     0,   122,     0,
       0,     0,   144,   145,     0,     0,     0,     0,     0,   164,
     430,   147,   127,   128,     1,   129,   130,    99,   131,   132,
     133,   134,   100,     7,   101,     0,   102,     9,   103,   104,
     105,   106,    14,    15,   107,    17,   108,    19,    20,   109,
     110,   111,   112,    24,   113,    26,    27,   114,   115,   116,
     117,     0,     0,     0,     0,    32,    33,   137,    35,    36,
      37,     0,    38,    39,   138,    41,   139,    43,   140,    45,
     141,    47,    48,    49,    50,    51,    52,     0,    53,    54,
      55,    56,    57,    58,   142,    60,    61,   143,    63,    64,
     118,   119,   120,   121,     0,     0,     0,   122,     0,     0,
       0,   144,   145,     0,     0,     0,     0,     0,   164,   488,
     147,   127,   128,     1,   129,   130,    99,   131,   132,   133,
     134,   100,     7,   101,     0,   102,   135,   103,   104,   105,
     106,    14,    15,   107,   136,   108,    19,    20,   109,   110,
     111,   112,    24,   113,    26,    27,   114,   115,   116,   117,
       0,     0,     0,     0,    32,    33,   137,    35,    36,    37,
       0,    38,    39,   138,    41,   139,    43,   140,    45,   141,
      47,    48,    49,    50,    51,    52,     0,    53,    54,    55,
      56,    57,    58,   142,    60,    61,   143,    63,    64,   118,
     119,   120,   121,     0,     0,     0,   122,     0,     0,     0,
     144,   145,     0,     0,     0,     0,     0,   146,     0,   147,
     127,   128,     1,   129,   130,    99,   131,   132,   133,   134,
     100,     7,   101,     0,   102,     9,   103,   104,   105,   106,
      14,    15,   107,    17,   108,    19,    20,   109,   110,   111,
     112,    24,   113,    26,    27,   114,   115,   116,   117,     0,
       0,     0,     0,    32,    33,   137,    35,    36,    37,     0,
      38,    39,   138,    41,   139,    43,   140,    45,   141,    47,
      48,    49,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,   142,    60,    61,   143,    63,    64,   118,   119,
     120,   121,     0,     0,     0,   122,     0,     0,     0,   144,
     145,     0,     0,     0,     0,     0,   164,     0,   147,   127,
     128,     1,   129,   130,    99,   131,   132,   133,   134,   100,
       7,   101,     0,   102,     9,   103,   104,   105,   106,    14,
      15,   107,    17,   108,    19,    20,   109,   110,   111,   112,
      24,   113,    26,    27,   114,   115,   116,   117,     0,     0,
       0,     0,    32,    33,   137,    35,    36,    37,     0,    38,
      39,   138,    41,   139,    43,   140,    45,   141,    47,    48,
      49,    50,    51,    52,     0,    53,    54,    55,    56,    57,
      58,   142,    60,    61,   143,    63,    64,   118,   119,   120,
     121,     0,     0,     0,   122,     0,     0,     0,   144,   145,
       0,     0,     0,     0,     0,   146,     0,   147,   127,   128,
       1,   129,   130,    99,   131,   132,   133,   134,   100,     7,
     101,     0,   102,     9,   103,   104,   105,   106,    14,    15,
     107,    17,   108,    19,    20,   109,   110,   111,   112,    24,
     113,    26,    27,   114,   115,   116,   117,     0,     0,     0,
       0,    32,    33,   137,    35,    36,    37,     0,    38,    39,
     138,    41,   139,    43,   140,    45,   141,    47,    48,    49,
      50,    51,    52,     0,    53,    54,    55,    56,    57,    58,
     142,    60,    61,    62,    63,    64,   118,   119,   120,   121,
       0,     0,     0,   122,     0,     0,     0,   144,   145,     0,
       0,     0,     0,     0,   193,     0,   147,   127,   128,     1,
     129,   130,    99,   131,   132,   133,   134,   100,     7,   101,
       0,   102,     9,   103,   104,   105,   106,    14,    15,   107,
      17,   108,    19,    20,   109,   110,   111,   112,    24,   113,
      26,    27,   114,   115,   116,   117,     0,     0,     0,     0,
      32,    33,   137,    35,    36,    37,     0,    38,    39,   138,
      41,   139,    43,   140,    45,   141,    47,    48,    49,    50,
      51,    52,     0,    53,    54,    55,    56,    57,    58,   142,
      60,    61,    62,    63,    64,   118,   119,   120,   121,     0,
       0,     0,   122,     0,     0,     0,   144,   145,     0,     0,
       0,     0,     0,   164,     0,   147,   127,   128,     1,   129,
     130,    99,   131,   132,   133,   134,   100,     7,   101,     0,
     102,     9,   103,   104,   105,   106,    14,    15,   107,    17,
     108,    19,    20,   109,   110,   111,   112,    24,   113,    26,
      27,   114,   115,   116,   117,     0,     0,     0,     0,    32,
      33,   137,    35,    36,    37,     0,    38,    39,   138,   415,
     139,   416,   140,   417,   141,   418,    48,    49,    50,    51,
      52,     0,    53,    54,   419,   420,   421,    58,   142,    60,
     422,   423,    63,    64,   118,   119,   120,   121,     0,     0,
       0,   122,     0,     0,     0,   144,   145,     0,     0,     0,
       0,     0,   164,     0,   147,   127,   128,     1,   129,   130,
      99,   131,   132,   133,   134,   100,     7,   101,     0,   102,
       9,   103,   104,   105,   106,    14,    15,   107,    17,   108,
      19,    20,   109,   110,   111,   112,    24,   113,    26,    27,
     114,   115,   116,   117,     0,     0,     0,     0,    32,    33,
     137,    35,    36,    37,     0,    38,    39,   138,    41,   139,
      43,   140,    45,   141,    47,    48,    49,    50,    51,    52,
       0,    53,    54,   419,   420,    57,    58,   142,    60,   422,
     423,    63,    64,   118,   119,   120,   121,     0,     0,     0,
     122,     0,     0,     0,   144,   145,     0,     0,     0,     0,
       0,   164,     0,   147,   127,   128,     1,   129,   130,    99,
     131,   132,   133,   134,   100,     7,   101,     0,   102,     9,
     103,   104,   105,   106,    14,    15,   107,    17,   108,    19,
      20,   109,   110,   111,   112,    24,   113,    26,    27,   114,
     115,   116,   117,     0,     0,     0,     0,    32,    33,   137,
      35,    36,    37,     0,    38,    39,   138,    41,   139,    43,
     140,    45,   141,    47,    48,    49,    50,    51,    52,     0,
      53,    54,   475,   476,    57,    58,   142,    60,   477,   478,
      63,    64,   118,   119,   120,   121,     0,     0,     0,   122,
       0,     0,     0,   144,   145,     0,     0,     0,     0,     0,
     164,     0,   147,   127,   128,     1,   129,   130,    99,   131,
     132,   133,   134,   100,     7,   101,     0,   102,     9,   103,
     104,   105,   106,    14,    15,   107,    17,   108,    19,    20,
     109,   110,   111,   112,    24,   113,    26,    27,   114,   115,
     116,   117,     0,     0,     0,     0,     0,     0,   137,    35,
      36,    37,     0,    38,    39,   138,    41,   139,    43,   140,
      45,   141,    47,    48,    49,    50,    51,    52,     0,    53,
      54,    55,    56,    57,    58,   142,    60,    61,   143,    63,
      64,   118,   119,   120,   121,     0,     0,     0,   122,     0,
       0,     0,   144,   145,     0,     0,     0,     0,     0,   164,
       0,   147,   127,   128,     1,   129,   130,    99,   131,   132,
     133,   134,   100,     7,   101,     0,   102,     9,   103,   104,
     105,   106,    14,    15,   107,    17,   108,    19,    20,   109,
     110,   111,   112,    24,   113,    26,    27,   114,   115,   116,
     117,     0,     0,     0,     0,     0,     0,   137,    35,    36,
      37,     0,     0,     0,   138,    41,   139,    43,   140,    45,
     141,    47,    48,    49,    50,    51,    52,     0,    53,    54,
      55,    56,    57,    58,   142,    60,    61,   143,    63,    64,
     118,   119,   120,   121,     0,     0,     0,   122,     0,     0,
       0,   144,   145,     0,     0,     0,     0,     0,   164,     0,
     147,   127,   128,     1,   129,   130,    99,   131,   132,   133,
     134,   100,     7,   101,     0,   102,     9,   103,   104,   105,
     106,    14,    15,   107,    17,   108,    19,    20,   109,   110,
     111,   112,    24,   113,    26,    27,   114,   115,   116,   117,
       0,     0,     0,     0,     0,     0,   137,     0,    36,    37,
       0,     0,     0,   138,    41,   139,    43,   140,    45,   141,
      47,    48,    49,    50,    51,    52,     0,    53,    54,    55,
      56,    57,    58,   142,    60,    61,   143,    63,    64,   118,
     119,   120,   121,     0,     0,     0,   122,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     1,   164,     0,   147,
       2,     3,     4,     5,     6,     7,     0,     0,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,     0,    24,    25,    26,    27,    28,
      29,    30,    31,     0,     0,     0,     0,    32,    33,    34,
      35,    36,    37,     0,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,     0,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,     1,     0,    69,
       0,     2,     3,     4,     5,     6,     7,     0,    70,     8,
       9,    10,    11,    12,    13,    14,    15,     0,    17,    18,
      19,    20,    21,    22,    23,     0,    24,    25,    26,    27,
       0,    29,    30,    31,     0,     0,     0,     0,    32,    33,
      34,    35,    36,    37,     0,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
       0,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,     0,     0,    68,     1,     0,
      69,     0,     2,     3,     4,     5,     6,     7,     0,   373,
       8,     9,    10,    11,    12,    13,    14,    15,     0,    17,
      18,    19,    20,    21,    22,    23,     0,    24,    25,    26,
      27,     0,    29,    30,    31,     0,     0,     0,     0,    32,
      33,    34,    35,    36,    37,     0,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,     0,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,     0,     0,    68,     1,
       0,    69,     0,     2,     3,     4,     5,     6,     7,     0,
     396,     8,     9,    10,    11,    12,    13,    14,    15,     0,
      17,    18,    19,    20,    21,    22,    23,     0,    24,    25,
      26,    27,     0,    29,    30,    31,     0,     0,     0,     0,
      32,    33,    34,    35,    36,    37,     0,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,     0,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,     0,     0,    68,
       1,     0,    69,     0,     2,     3,     4,     5,     6,     7,
       0,   512,     8,     9,    10,    11,    12,    13,    14,    15,
       0,    17,    18,    19,    20,    21,    22,    23,     0,    24,
      25,    26,    27,     0,    29,    30,    31,     0,     0,     0,
       0,    32,    33,    34,    35,    36,    37,     0,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,     0,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,     0,     0,
      68,     1,     0,    69,     0,     2,     3,     4,     5,     6,
       7,     0,   540,     8,     9,    10,    11,    12,    13,    14,
      15,     0,    17,    18,    19,    20,    21,    22,    23,     0,
      24,    25,    26,    27,     0,    29,    30,    31,     0,     0,
       0,     0,    32,    33,    34,    35,    36,    37,     0,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,     0,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,     0,
       0,     0,     0,     0,    69,     0,     0,     0,     0,     0,
       0,     0,     0,   462,     1,   435,   436,    99,     2,     3,
       4,     5,   100,     7,   101,     0,   102,     9,   103,   104,
     105,   106,    14,    15,   107,    17,   108,    19,    20,   109,
     110,   111,   112,    24,   113,    26,    27,   114,   115,   116,
     117,     0,     0,     0,     0,    32,    33,    34,    35,    36,
      37,     0,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,     0,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
     118,   119,   120,   121,     0,     1,     0,   122,    99,     2,
       3,     4,     5,   100,     7,   101,     0,   102,     9,   103,
     104,   105,   106,    14,    15,   107,    17,   108,    19,    20,
     109,   110,   111,   112,    24,   113,    26,    27,   114,   115,
     116,   117,     0,     0,     0,     0,    32,    33,    34,    35,
      36,    37,     0,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,     0,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,   118,   119,   120,   121,     0,     1,     0,   122,    99,
       2,     3,     4,     5,   100,     7,   101,     0,   102,     9,
     103,   104,   105,   106,    14,    15,   107,    17,   108,    19,
      20,   109,   110,   111,   112,    24,   113,    26,    27,   114,
     115,   116,   117,     0,     0,     0,     0,    32,    33,    34,
      35,    36,    37,     0,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,     0,
      53,    54,    55,    56,    57,    58,   172,    60,    61,    62,
      63,    64,   118,   119,   120,   121,     0,     1,     0,   122,
      99,     2,     3,     4,     5,   100,     7,   101,     0,   102,
       9,   103,   104,   105,   106,    14,    15,   107,    17,   108,
      19,    20,   109,   110,   111,   112,    24,   113,    26,    27,
     114,   115,   116,   117,     0,     0,     0,     0,    32,    33,
      34,    35,    36,    37,     0,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,   211,   212,   213,   214,   215,
       0,    53,   216,    55,    56,    57,   217,    59,    60,    61,
      62,    63,    64,   118,   119,   120,   121,     0,     1,     0,
     122,    99,   131,   132,     4,     5,   100,     7,   101,     0,
     102,     9,   103,   104,   105,   106,    14,    15,   107,    17,
     108,    19,    20,   109,   110,   111,   112,    24,   113,    26,
      27,   114,   115,   116,   117,     0,     0,     0,     0,    32,
      33,    34,    35,    36,    37,     0,    38,    39,   138,    41,
     139,    43,   140,    45,   141,    47,    48,    49,    50,    51,
      52,     0,    53,    54,    55,    56,    57,    58,   376,    60,
      61,    62,    63,    64,   118,   119,   120,   121,     1,     0,
       0,   122,     2,     3,     4,     5,     6,     7,     0,     0,
       8,     9,    10,    11,    12,     0,    14,    15,     0,    17,
      18,    19,    20,    21,   110,    23,     0,    24,     0,    26,
      27,     0,    29,    30,    31,     0,     0,     0,     0,    32,
      33,    34,    35,    36,    37,     0,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,     0,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,   299,    66,     0,     0,     0,     0,
     300,    69,     0,     0,     0,     0,     0,   490,   237,   238,
     239,   240,   262,   263,     0,   264,     0,     0,   244,   265,
     266,   237,   238,   239,   240,   262,   263,   491,   264,     0,
       0,   244,   265,   266,     0,     0,     0,     0,     0,     0,
       0,   237,   238,   239,   240,   262,   263,     0,   264,     0,
       0,   244,   265,   266,     0,   247,   248,   249,   250,   251,
     252,   253,     0,     0,     0,     0,     0,     0,   247,   248,
     249,   250,   251,   252,   253,     0,     0,     0,     0,     0,
       0,   492,     0,     0,     0,     0,     0,     0,   247,   248,
     249,   250,   251,   252,   253,   237,   238,   239,   240,   262,
     263,   493,   264,     0,     0,   244,   265,   266,     0,     0,
       0,     0,     0,     0,   564,   237,   238,   239,   240,   262,
     263,     0,   264,     0,     0,   244,   265,   266,   548,   549,
       0,   550,   551,   552,     0,   553,     0,     0,   554,   555,
     556,   286,   247,   248,   249,   250,   251,   252,   253,   237,
     238,   239,   240,   262,   263,     0,   264,     0,     0,   244,
     265,   266,   247,   248,   249,   250,   251,   252,   253,     0,
       0,     0,     0,   261,     0,   557,   558,   559,   560,   561,
     562,   563,   237,   238,   239,   240,   262,   263,     0,   264,
       0,     0,   244,   265,   266,     0,   247,   248,   249,   250,
     251,   252,   253,     0,   287,   237,   238,   239,   240,   262,
     263,     0,   264,     0,     0,   244,   265,   266,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   247,
     248,   249,   250,   251,   252,   253,   237,   238,   239,   240,
     262,   263,     0,   264,     0,     0,   244,   265,   266,     0,
       0,     0,   247,   248,   249,   250,   251,   252,   253,     0,
       0,   402,     0,   324,   354,   355,   356,   357,   358,   359,
       0,   360,     0,     0,   361,   362,   363,     0,     0,     0,
       0,     0,     0,   247,   248,   249,   250,   251,   252,   253,
       0,     0,     0,     0,   324,   237,   238,   239,   240,   262,
     263,     0,   264,     0,     0,   244,   265,   266,     0,     0,
       0,   364,   365,   366,   367,   368,   369,   370,     0,     0,
     371,   237,   238,   239,   240,   262,   263,     0,   264,     0,
       0,   244,   265,   266,     0,     0,     0,     0,     0,     0,
       0,     0,   247,   248,   249,   250,   251,   252,   253,     0,
     384,   237,   238,   239,   240,   262,   263,     0,   264,     0,
       0,   244,   265,   266,     0,     0,     0,     0,   247,   248,
     249,   250,   251,   252,   253,     0,   481,   237,   238,   239,
     240,   262,   263,     0,   264,     0,     0,   244,   265,   266,
       0,     0,     0,     0,     0,     0,     0,     0,   247,   248,
     249,   250,   251,   252,   253,     0,   484,   237,   238,   239,
     240,   262,   263,     0,   264,     0,     0,   244,   265,   266,
       0,     0,     0,     0,   247,   248,   249,   250,   251,   252,
     253,     0,   514,   237,   238,   239,   240,   262,   263,     0,
     264,     0,     0,   244,   265,   266,     0,     0,     0,     0,
       0,     0,     0,     0,   247,   248,   249,   250,   251,   252,
     253,     0,   574,   237,   238,   239,   240,   262,   263,     0,
     264,     0,     0,   244,   265,   266,     0,     0,     0,     0,
     247,   248,   249,   250,   251,   252,   253,     0,   598,     0,
       0,     0,   237,   238,   239,   240,   262,   263,     0,   264,
     321,   322,   244,   265,   266,     0,     0,     0,     0,     0,
     247,   248,   249,   250,   251,   252,   253,   354,   355,   356,
     357,   358,   359,     0,   360,     0,     0,   361,   362,   363,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   247,
     248,   249,   250,   251,   252,   253,   548,   549,     0,   550,
     551,   552,     0,   553,     0,     0,   554,   555,   556,     0,
       0,     0,     0,     0,   364,   365,   366,   367,   368,   369,
     370,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   557,   558,   559,   560,   561,   562,   563
};

static const yytype_int16 yycheck[] =
{
       0,   257,    10,     6,    23,     0,    31,    69,    16,    12,
     233,    17,    90,   373,   161,    23,    33,    25,    51,    21,
      28,   375,   169,    31,   512,    51,   543,    29,    30,    25,
     384,   254,    36,   180,   476,   477,   478,    11,    12,    71,
      33,    66,    46,    25,    86,   192,    78,   401,    30,    99,
      46,     0,   540,   570,   132,   133,   134,    99,    66,    67,
      71,   208,    99,    96,    97,    72,    71,    78,    99,    69,
      96,    97,    72,    78,    11,    77,    13,    72,   334,    38,
      39,    87,    99,    57,    21,    59,    51,    61,    71,    63,
     100,    71,    29,    30,   104,    78,    35,   353,    78,   100,
     323,    99,   100,    77,   101,   465,   548,   549,   550,   551,
     552,   553,   554,   555,   556,   557,   558,   559,   560,   561,
     562,   563,    99,    51,    99,    30,    54,    51,    65,    94,
      95,    96,    97,    99,   142,    44,    45,    46,    47,   147,
      77,   166,    51,   271,   272,    54,    55,    56,   371,   591,
      51,    99,   512,    54,    91,    92,    93,    94,   166,   101,
     514,   186,   170,   105,   172,    93,    94,    95,    96,    97,
      94,    95,    96,    97,    99,   431,    99,    99,   186,   402,
     540,    99,    91,    92,    93,    94,    95,    96,    97,    99,
     105,    99,    93,    94,    95,    96,    97,   205,   135,   136,
      99,   138,   139,   140,   141,   461,   143,    99,    99,   146,
      30,   565,   415,   416,   417,   418,   105,    57,   421,    59,
     574,    61,   104,    63,   286,   104,   100,   164,    44,    45,
      99,    47,    28,   489,   171,    51,    30,    99,    54,    55,
      56,   161,    28,   100,   598,    17,    99,    99,    99,   169,
      89,   270,   271,   272,   102,   105,   193,    88,    99,    10,
     180,   261,   270,   271,   272,    30,   261,   270,   271,   272,
      73,    99,   192,   102,    36,    91,    92,    93,    94,    95,
      96,    97,    99,   102,    99,    73,   286,   287,   208,    88,
      87,    89,    90,    73,    99,    88,    88,    99,    99,    72,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   392,    30,   399,
     465,   329,   502,   208,    -1,   262,   263,   264,   265,   266,
     320,    -1,    -1,   131,   132,   133,   134,   329,    -1,   137,
      -1,   278,   279,    -1,    -1,   282,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   294,    -1,   296,
      -1,   298,    -1,   300,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,    16,   375,    -1,    -1,    -1,    -1,
     388,    -1,    -1,    -1,   384,    -1,    -1,    -1,   325,   326,
      -1,   328,    -1,    -1,    36,   332,    -1,    -1,   335,   399,
      -1,   401,    44,    45,    46,    47,    48,    49,    -1,    51,
      -1,    -1,    54,    55,    56,    -1,    -1,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   382,   383,    -1,    -1,    91,
      92,    93,    94,    95,    96,    97,    -1,   482,    -1,    -1,
      -1,    -1,    -1,    -1,    30,   465,    -1,   467,    -1,    -1,
      36,    -1,   467,    -1,   482,    -1,    -1,   414,    44,    45,
      46,    47,    48,    49,    -1,    51,    -1,    -1,    54,    55,
      56,    -1,    -1,    -1,    -1,   293,    -1,   295,    -1,   297,
      -1,   299,    -1,   440,    -1,    -1,    -1,    -1,   508,    -1,
      -1,    -1,   512,   508,   514,    -1,    -1,    -1,   526,    -1,
      -1,   319,   320,   321,   322,    91,    92,    93,    94,    95,
      96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     540,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   475,   476,   477,   478,    -1,
      -1,    -1,    -1,    -1,    -1,   565,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   574,    -1,    -1,    -1,    -1,   516,
      -1,    -1,    -1,    -1,    -1,    30,   594,   385,    -1,    -1,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,   598,    44,
      45,    46,    47,    48,    49,    -1,    51,    -1,   545,    54,
      55,    56,    -1,    -1,    -1,   413,    -1,    -1,    -1,    -1,
      -1,   419,   420,    -1,   422,   423,    -1,    -1,   548,   549,
     550,   551,   552,   553,   554,   555,   556,   557,   558,   559,
     560,   561,   562,   563,    -1,    -1,    91,    92,    93,    94,
      95,    96,    97,    -1,    -1,    -1,    44,    45,    46,    47,
      48,    49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,
      -1,   591,    -1,    -1,    -1,    -1,    -1,   475,   476,   477,
     478,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   490,   491,   492,   493,    -1,    -1,   496,   497,
     498,   499,    -1,    91,    92,    93,    94,    95,    96,    97,
      -1,    30,   100,    -1,   102,    -1,    -1,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    46,    47,    48,
      49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,   537,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     548,   549,   550,   551,   552,   553,   554,   555,   556,   557,
     558,   559,   560,   561,   562,   563,    -1,    -1,    -1,    -1,
      -1,   569,    91,    92,    93,    94,    95,    96,    97,    -1,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,   591,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    49,    50,    51,    52,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,
      95,    -1,    -1,    -1,    -1,    -1,   101,    -1,   103,    -1,
     105,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    91,    92,    93,
      94,    95,    96,    97,    -1,    -1,   100,   101,    -1,   103,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    97,    -1,    -1,   100,   101,    -1,   103,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    -1,    -1,   100,   101,    -1,   103,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    -1,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    -1,    -1,   100,   101,    -1,   103,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    -1,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
      -1,    -1,    -1,   101,    -1,   103,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,    -1,   101,   102,   103,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,
      50,    51,    52,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
      -1,   101,   102,   103,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,    50,
      51,    52,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,
     101,   102,   103,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    49,    50,    51,
      52,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    -1,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,   101,
     102,   103,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    49,    50,    51,    52,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    -1,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,
      -1,    94,    95,    -1,    -1,    -1,    -1,    -1,   101,   102,
     103,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    49,    50,    51,    52,    53,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      94,    95,    -1,    -1,    -1,    -1,    -1,   101,    -1,   103,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    49,    50,    51,    52,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,
      95,    -1,    -1,    -1,    -1,    -1,   101,    -1,   103,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    49,    50,    51,    52,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,    -1,   101,    -1,   103,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    49,    50,    51,    52,    53,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    -1,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,
      -1,    -1,    -1,    -1,   101,    -1,   103,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    -1,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,
      -1,    -1,    -1,   101,    -1,   103,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,    -1,   101,    -1,   103,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,
      50,    51,    52,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
      -1,   101,    -1,   103,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,    50,
      51,    52,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,
     101,    -1,   103,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      52,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    -1,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,   101,
      -1,   103,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,
      53,    -1,    -1,    -1,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    -1,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,
      -1,    94,    95,    -1,    -1,    -1,    -1,    -1,   101,    -1,
     103,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    52,    53,
      -1,    -1,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     7,   101,    -1,   103,
      11,    12,    13,    14,    15,    16,    -1,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,    50,
      51,    52,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    -1,    87,     7,    -1,    90,
      -1,    11,    12,    13,    14,    15,    16,    -1,    99,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,
      50,    51,    52,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    87,     7,    -1,
      90,    -1,    11,    12,    13,    14,    15,    16,    -1,    99,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    87,     7,
      -1,    90,    -1,    11,    12,    13,    14,    15,    16,    -1,
      99,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    -1,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    87,
       7,    -1,    90,    -1,    11,    12,    13,    14,    15,    16,
      -1,    99,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    49,    50,    51,    52,    53,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    -1,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      87,     7,    -1,    90,    -1,    11,    12,    13,    14,    15,
      16,    -1,    99,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    49,    50,    51,    52,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    49,    50,    51,    52,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    -1,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,     7,    -1,    90,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    49,    50,    51,
      52,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    -1,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,     7,    -1,    90,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,    50,
      51,    52,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,     7,    -1,    90,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,
      50,    51,    52,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,     7,    -1,
      90,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,     7,    -1,
      -1,    90,    11,    12,    13,    14,    15,    16,    -1,    -1,
      19,    20,    21,    22,    23,    -1,    25,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    -1,    38,
      39,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    30,    84,    -1,    -1,    -1,    -1,
      36,    90,    -1,    -1,    -1,    -1,    -1,    30,    44,    45,
      46,    47,    48,    49,    -1,    51,    -1,    -1,    54,    55,
      56,    44,    45,    46,    47,    48,    49,    30,    51,    -1,
      -1,    54,    55,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    46,    47,    48,    49,    -1,    51,    -1,
      -1,    54,    55,    56,    -1,    91,    92,    93,    94,    95,
      96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    44,    45,    46,    47,    48,
      49,    30,    51,    -1,    -1,    54,    55,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    44,    45,    46,    47,    48,
      49,    -1,    51,    -1,    -1,    54,    55,    56,    44,    45,
      -1,    47,    48,    49,    -1,    51,    -1,    -1,    54,    55,
      56,    36,    91,    92,    93,    94,    95,    96,    97,    44,
      45,    46,    47,    48,    49,    -1,    51,    -1,    -1,    54,
      55,    56,    91,    92,    93,    94,    95,    96,    97,    -1,
      -1,    -1,    -1,    35,    -1,    91,    92,    93,    94,    95,
      96,    97,    44,    45,    46,    47,    48,    49,    -1,    51,
      -1,    -1,    54,    55,    56,    -1,    91,    92,    93,    94,
      95,    96,    97,    -1,    99,    44,    45,    46,    47,    48,
      49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    96,    97,    44,    45,    46,    47,
      48,    49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    -1,
      -1,   100,    -1,   102,    44,    45,    46,    47,    48,    49,
      -1,    51,    -1,    -1,    54,    55,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    97,
      -1,    -1,    -1,    -1,   102,    44,    45,    46,    47,    48,
      49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    -1,    -1,
     100,    44,    45,    46,    47,    48,    49,    -1,    51,    -1,
      -1,    54,    55,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    -1,
      99,    44,    45,    46,    47,    48,    49,    -1,    51,    -1,
      -1,    54,    55,    56,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    -1,    99,    44,    45,    46,
      47,    48,    49,    -1,    51,    -1,    -1,    54,    55,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    -1,    99,    44,    45,    46,
      47,    48,    49,    -1,    51,    -1,    -1,    54,    55,    56,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    -1,    99,    44,    45,    46,    47,    48,    49,    -1,
      51,    -1,    -1,    54,    55,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    -1,    99,    44,    45,    46,    47,    48,    49,    -1,
      51,    -1,    -1,    54,    55,    56,    -1,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    -1,    99,    -1,
      -1,    -1,    44,    45,    46,    47,    48,    49,    -1,    51,
      81,    82,    54,    55,    56,    -1,    -1,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    44,    45,    46,
      47,    48,    49,    -1,    51,    -1,    -1,    54,    55,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    96,    97,    44,    45,    -1,    47,
      48,    49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    97
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     7,    11,    12,    13,    14,    15,    16,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    36,    37,    38,    39,    40,    41,
      42,    43,    48,    49,    50,    51,    52,    53,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    87,    90,
      99,   107,   108,   109,   110,   111,   116,   118,   122,   123,
     124,   125,   129,   131,   132,   133,   135,   136,   137,    11,
      12,    57,    59,    61,    63,    77,   148,    33,    99,    10,
      15,    17,    19,    21,    22,    23,    24,    27,    29,    32,
      33,    34,    35,    37,    40,    41,    42,    43,    83,    84,
      85,    86,    90,   118,   119,   120,   121,     5,     6,     8,
       9,    11,    12,    13,    14,    20,    28,    50,    57,    59,
      61,    63,    77,    80,    94,    95,   101,   103,   119,   126,
     128,   144,   145,   148,   150,   152,   155,   156,   160,   163,
     164,   165,   168,   148,   101,   165,   119,    33,   126,   165,
      38,    39,    77,   119,   145,   147,   158,   119,   119,   126,
     165,   126,   128,   120,   165,   120,   119,    99,   124,     0,
     109,   126,   165,   101,   151,   164,   151,   165,   165,   165,
     165,    72,    99,    99,    99,   100,   151,   151,   165,   165,
     164,    65,    66,    67,    68,    69,    72,    76,   119,   165,
     164,   164,   102,   165,     8,     9,   105,   119,   161,   162,
     165,   169,   170,   101,    99,    99,    30,    44,    45,    46,
      47,    48,    49,    51,    54,    55,    56,    91,    92,    93,
      94,    95,    96,    97,   100,   127,   144,   166,   168,    99,
     165,    35,    48,    49,    51,    55,    56,   120,    99,    99,
      26,    52,    53,   119,   165,   119,    36,    46,   130,    35,
      99,    99,    38,    99,    99,    99,    36,    99,    99,   120,
      99,   102,   165,    30,    36,    30,    36,    30,    36,    30,
      36,    57,    59,    61,    63,   149,   119,    30,   166,    71,
      78,    71,    78,    71,    78,    71,    78,    71,    78,    30,
      30,    81,    82,   100,   102,   105,   105,   104,   105,   100,
     104,   104,   100,   102,   170,    28,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   170,    44,    45,    46,    47,    48,    49,
      51,    54,    55,    56,    91,    92,    93,    94,    95,    96,
      97,   100,   127,    99,   122,    99,    77,   145,   146,   148,
     146,   146,    25,    46,    99,    30,   165,   165,    86,    99,
     112,   113,   114,   115,   165,   124,    99,   122,   138,   139,
     140,    99,   100,   164,   165,   164,   165,   164,   165,   164,
     165,    25,    30,   157,    28,    58,    60,    62,    64,    73,
      74,    75,    79,    80,   153,   154,   164,   153,   164,   164,
     102,   170,   165,   165,   165,     8,     9,   119,   162,   165,
     100,   127,   165,   127,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   170,    99,   123,   141,   142,   143,    17,   138,    99,
      99,    99,   165,   165,   138,    73,    74,    79,    80,   159,
     164,    99,   119,   115,    99,    89,   140,   138,   102,   170,
      30,    30,    30,    30,   164,   165,   157,   157,   157,   157,
     164,   164,   157,   164,   164,   127,   102,   127,    17,    87,
     134,   143,    99,   122,    99,    16,    36,    88,   164,   167,
     168,   167,   167,   167,   120,    99,    10,   127,   164,   164,
     164,   164,   164,   164,   164,   164,    30,    73,   154,   102,
      99,   122,    99,   141,   138,    36,   165,    99,    44,    45,
      47,    48,    49,    51,    54,    55,    56,    91,    92,    93,
      94,    95,    96,    97,    30,    99,   117,   119,   102,    73,
     141,   134,    88,   165,    99,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,    73,   138,    99,   100,   164,   134,    99,    99,   138,
     167,   119,   138,    88,    88,    99,    99
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   106,   107,   108,   108,   109,   109,   109,   109,   109,
     110,   111,   112,   113,   113,   114,   114,   115,   115,   116,
     116,   117,   117,   117,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   120,   120,   120,
     121,   121,   122,   122,   123,   123,   123,   123,   123,   123,
     124,   124,   124,   125,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   125,   125,   126,   126,   126,
     126,   126,   126,   126,   126,   127,   127,   128,   128,   128,
     128,   128,   129,   129,   129,   129,   130,   130,   131,   131,
     131,   132,   132,   133,   133,   133,   133,   134,   134,   135,
     135,   135,   135,   136,   136,   137,   138,   138,   139,   139,
     140,   140,   141,   141,   142,   142,   143,   143,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   145,   146,   146,   147,   147,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   149,   149,   149,
     149,   150,   150,   151,   151,   151,   151,   152,   152,   152,
     152,   152,   153,   153,   153,   153,   153,   154,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   156,   156,
     156,   156,   156,   157,   157,   158,   158,   159,   159,   159,
     159,   159,   160,   160,   160,   161,   161,   162,   162,   162,
     163,   163,   164,   164,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     168,   168,   169,   169,   170,   170
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
       1,     1,     1,     1,     1,     1,     1,     0,     1,     2,
       1,     3,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       5,     3,     3,     2,     3,     3,     2,     0,     2,     4,
       3,     5,     2,     4,     6,     0,     1,     2,     2,     5,
       4,     3,     5,     5,     5,     5,     1,     1,     3,     3,
       3,     4,     6,     6,     8,     7,     9,     0,     2,     7,
      11,    12,     9,     4,     6,     4,     0,     1,     1,     2,
       1,     1,     0,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     2,     3,     5,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     4,     6,
       4,     6,     4,     6,     4,     6,     5,     1,     1,     1,
       1,     2,     2,     1,     2,     4,     6,     2,     4,     4,
       1,     1,     1,     1,     5,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     6,     6,
       6,     6,     6,     1,     1,     2,     4,     1,     2,     5,
       2,     2,     3,     3,     3,     1,     3,     3,     3,     3,
       2,     2,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       4,     4,     0,     1,     1,     3
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
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2319 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2325 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2331 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2337 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2343 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 190 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2349 "engines/director/lingo/lingo-gr.cpp"
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
#line 2617 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 198 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2628 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 204 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2638 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 211 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2644 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 244 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2650 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* factory: tFACTORY ID '\n' methodlist  */
#line 248 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2656 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 250 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2662 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* methodlist: %empty  */
#line 252 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2668 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* nonemptymethodlist: methodlistline  */
#line 256 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2679 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 262 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2689 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlistline: '\n'  */
#line 269 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2695 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 275 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2704 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* handler: tON ID idlist '\n' stmtlist  */
#line 279 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* endargdef: ID  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* endargdef: endargdef ',' ID  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2723 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* CMDID: tABBREVIATED  */
#line 293 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* CMDID: tABBREV  */
#line 294 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* CMDID: tABBR  */
#line 295 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* CMDID: tAFTER  */
#line 296 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2747 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* CMDID: tAND  */
#line 297 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 2753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* CMDID: tBEFORE  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* CMDID: tCAST  */
#line 299 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2765 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tCHAR  */
#line 300 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2771 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tCHARS  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2777 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tCONTAINS  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("contains"); }
#line 2783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tDATE  */
#line 303 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tDELETE  */
#line 304 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 2795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tDOWN  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2801 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tFIELD  */
#line 306 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2807 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tFRAME  */
#line 307 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tHILITE  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 2819 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tIN  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2825 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tINTERSECTS  */
#line 310 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2831 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tINTO  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tITEM  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2843 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tITEMS  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2849 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tLAST  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2855 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tLINE  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tLINES  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2867 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tLONG  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2873 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tMENU  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2879 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tMENUITEM  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2885 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tMENUITEMS  */
#line 320 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2891 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tMOD  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2897 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tMOVIE  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2903 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tNEXT  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2909 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tNOT  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2915 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tNUMBER  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2921 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tOF  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2927 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tOR  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2933 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tPREVIOUS  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tREPEAT  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tSCRIPT  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tASSERTERROR  */
#line 331 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tSHORT  */
#line 332 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2963 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tSOUND  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2969 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tSPRITE  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2975 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tSTARTS  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tTHE  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 2987 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tTIME  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 2993 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tTO  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2999 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tWHILE  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 3005 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tWINDOW  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 3011 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* CMDID: tWITH  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 3017 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* CMDID: tWITHIN  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 3023 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* CMDID: tWORD  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 3029 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* CMDID: tWORDS  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 3035 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* ID: tELSE  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 3041 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* ID: tENDCLAUSE  */
#line 349 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); }
#line 3047 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* ID: tEXIT  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 3053 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tFACTORY  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 3059 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tGLOBAL  */
#line 352 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 3065 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tGO  */
#line 353 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 3071 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tIF  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 3077 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tINSTANCE  */
#line 355 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 3083 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tMACRO  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3089 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tMETHOD  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3095 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tON  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3101 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tOPEN  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3107 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tPLAY  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3113 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tPROPERTY  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3119 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tPUT  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3125 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ID: tSET  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3131 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* ID: tTELL  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3137 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* ID: tTHEN  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3143 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* ID: tWHEN  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("when"); }
#line 3149 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* idlist: %empty  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3155 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* nonemptyidlist: ID  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3164 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 378 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* stmt: tENDIF '\n'  */
#line 389 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3178 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* proc: CMDID cmdargs '\n'  */
#line 405 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3184 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* proc: tPUT cmdargs '\n'  */
#line 406 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* proc: tGO cmdargs '\n'  */
#line 407 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3196 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: tGO frameargs '\n'  */
#line 408 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3202 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: tPLAY cmdargs '\n'  */
#line 409 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3208 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tPLAY frameargs '\n'  */
#line 410 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3214 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tOPEN cmdargs '\n'  */
#line 411 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tOPEN expr tWITH expr '\n'  */
#line 412 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3230 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tNEXT tREPEAT '\n'  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3236 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* proc: tEXIT tREPEAT '\n'  */
#line 418 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3242 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proc: tEXIT '\n'  */
#line 419 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3248 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tDELETE chunk '\n'  */
#line 420 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3254 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proc: tHILITE chunk '\n'  */
#line 421 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3260 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* proc: tASSERTERROR stmtoneliner  */
#line 422 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3266 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* cmdargs: %empty  */
#line 425 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3274 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* cmdargs: expr trailingcomma  */
#line 428 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3284 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 433 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd args, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3293 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 437 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 443 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3314 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* cmdargs: '(' ')'  */
#line 448 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* cmdargs: '(' expr ',' ')'  */
#line 451 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(args,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3332 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 456 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(args, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3341 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* frameargs: tFRAME expr  */
#line 468 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3351 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* frameargs: tMOVIE expr  */
#line 473 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3362 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 479 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3373 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* frameargs: expr tOF tMOVIE expr  */
#line 485 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3384 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* frameargs: tFRAME expr expr_nounarymath  */
#line 491 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3395 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 499 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3401 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 500 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3407 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 501 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3413 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* asgn: tSET varorthe to expr '\n'  */
#line 502 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3419 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* definevars: tGLOBAL idlist '\n'  */
#line 507 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3425 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* definevars: tPROPERTY idlist '\n'  */
#line 508 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3431 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* definevars: tINSTANCE idlist '\n'  */
#line 509 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3437 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* ifstmt: tIF expr tTHEN stmt  */
#line 512 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3446 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 516 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 3453 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 520 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3464 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 526 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 3473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 530 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3482 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 534 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 3489 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* endif: %empty  */
#line 538 "engines/director/lingo/lingo-gr.y"
                        { warning("LingoCompiler::parse: no end if"); }
#line 3495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 541 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3502 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 543 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3509 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 545 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 547 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3523 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* tell: tTELL expr tTO stmtoneliner  */
#line 551 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3532 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 555 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3539 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* when: tWHEN ID tTHEN expr  */
#line 559 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new WhenNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3545 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* stmtlist: %empty  */
#line 561 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3551 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* nonemptystmtlist: stmtlistline  */
#line 566 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3562 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 572 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3572 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* stmtlistline: '\n'  */
#line 579 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3578 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* stmtlist_insideif: %empty  */
#line 583 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 3584 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 588 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3595 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 594 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* stmtlistline_insideif: '\n'  */
#line 601 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* simpleexpr_nounarymath: tINT  */
#line 608 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3617 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* simpleexpr_nounarymath: tFLOAT  */
#line 609 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3623 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* simpleexpr_nounarymath: tSYMBOL  */
#line 610 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3629 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* simpleexpr_nounarymath: tSTRING  */
#line 611 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3635 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 612 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3641 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 613 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 3647 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 614 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 3653 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 615 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3659 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 191: /* var: ID  */
#line 623 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3665 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* chunk: tFIELD refargs  */
#line 633 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 3671 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* chunk: tCAST refargs  */
#line 634 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 3677 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* chunk: tCHAR expr tOF simpleexpr  */
#line 635 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3684 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 637 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* chunk: tWORD expr tOF simpleexpr  */
#line 639 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3698 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 641 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* chunk: tITEM expr tOF simpleexpr  */
#line 643 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3712 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 645 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3719 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* chunk: tLINE expr tOF simpleexpr  */
#line 647 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 649 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3733 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 651 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 3739 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* chunktype: tCHAR  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 3745 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* chunktype: tWORD  */
#line 655 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 3751 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* chunktype: tITEM  */
#line 656 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 3757 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* chunktype: tLINE  */
#line 657 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 3763 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* object: tSCRIPT refargs  */
#line 660 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 3769 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* object: tWINDOW refargs  */
#line 661 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 3775 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* refargs: simpleexpr  */
#line 664 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3785 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* refargs: '(' ')'  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 3793 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* refargs: '(' expr ',' ')'  */
#line 672 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(args,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3803 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 677 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(args, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3812 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* the: tTHE ID  */
#line 683 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* the: tTHE ID tOF theobj  */
#line 684 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3824 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* the: tTHE tNUMBER tOF theobj  */
#line 685 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 3830 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 224: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 692 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3836 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 225: /* theobj: tSOUND simpleexpr  */
#line 693 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3842 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 226: /* theobj: tSPRITE simpleexpr  */
#line 694 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3848 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* menu: tMENU simpleexpr  */
#line 697 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3854 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3860 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 229: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 700 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3866 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 230: /* thedatetime: tTHE tABBREV tDATE  */
#line 701 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3872 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* thedatetime: tTHE tABBREV tTIME  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* thedatetime: tTHE tABBR tDATE  */
#line 703 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3884 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* thedatetime: tTHE tABBR tTIME  */
#line 704 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3890 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* thedatetime: tTHE tLONG tDATE  */
#line 705 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 3896 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* thedatetime: tTHE tLONG tTIME  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 3902 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* thedatetime: tTHE tSHORT tDATE  */
#line 707 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 3908 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* thedatetime: tTHE tSHORT tTIME  */
#line 708 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 3914 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 3920 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 713 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 3926 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 714 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 3932 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 715 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 3938 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 716 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 3944 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* writablethe: tTHE ID  */
#line 721 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3950 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* writablethe: tTHE ID tOF writabletheobj  */
#line 722 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3956 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* writabletheobj: tMENU expr_noeq  */
#line 726 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3962 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 727 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3968 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* writabletheobj: tSOUND expr_noeq  */
#line 728 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3974 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* writabletheobj: tSPRITE expr_noeq  */
#line 729 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3980 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* list: '[' exprlist ']'  */
#line 732 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 3986 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* list: '[' ':' ']'  */
#line 733 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 3992 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* list: '[' proplist ']'  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 3998 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* proplist: proppair  */
#line 737 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* proplist: proplist ',' proppair  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4015 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* proppair: tSYMBOL ':' expr  */
#line 746 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4021 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* proppair: ID ':' expr  */
#line 747 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4027 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* proppair: tSTRING ':' expr  */
#line 748 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4033 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* unarymath: '+' simpleexpr  */
#line 751 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 4039 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 261: /* unarymath: '-' simpleexpr  */
#line 752 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 4045 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* expr: expr '+' expr  */
#line 762 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4051 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* expr: expr '-' expr  */
#line 763 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4057 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* expr: expr '*' expr  */
#line 764 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4063 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* expr: expr '/' expr  */
#line 765 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4069 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 270: /* expr: expr tMOD expr  */
#line 766 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4075 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 271: /* expr: expr '>' expr  */
#line 767 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4081 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* expr: expr '<' expr  */
#line 768 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4087 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* expr: expr tEQ expr  */
#line 769 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4093 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* expr: expr tNEQ expr  */
#line 770 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4099 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* expr: expr tGE expr  */
#line 771 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4105 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* expr: expr tLE expr  */
#line 772 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4111 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* expr: expr tAND expr  */
#line 773 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4117 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* expr: expr tOR expr  */
#line 774 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4123 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* expr: expr '&' expr  */
#line 775 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4129 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* expr: expr tCONCAT expr  */
#line 776 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4135 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* expr: expr tCONTAINS expr  */
#line 777 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* expr: expr tSTARTS expr  */
#line 778 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4147 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 787 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4153 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 788 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4159 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 287: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 789 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4165 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 288: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 790 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4171 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 289: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 791 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4177 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 290: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 792 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 291: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 793 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4189 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 292: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 794 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4195 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 795 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4201 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 294: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 796 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4207 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 295: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 797 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 296: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 798 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4219 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 799 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4225 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 800 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4231 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 801 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4237 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 802 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4243 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 301: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 803 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4249 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 304: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 808 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4255 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 809 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4261 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 810 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4267 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 307: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 811 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4273 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 308: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 812 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4279 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 309: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 813 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4285 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 310: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 814 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4291 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 311: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 815 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4297 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 312: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 816 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4303 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 313: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 817 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4309 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 314: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 818 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4315 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 315: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 819 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4321 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4327 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 821 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 823 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 826 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4351 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 827 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4357 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 322: /* exprlist: %empty  */
#line 830 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 4363 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 324: /* nonemptyexprlist: expr  */
#line 834 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 325: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 838 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4380 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 4384 "engines/director/lingo/lingo-gr.cpp"

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

#line 843 "engines/director/lingo/lingo-gr.y"


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
