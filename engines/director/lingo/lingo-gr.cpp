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
#define YYFINAL  187
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4569

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  324
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  603

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
       0,   202,   202,   204,   210,   217,   218,   219,   220,   221,
     250,   254,   256,   258,   259,   262,   268,   275,   276,   281,
     285,   289,   290,   291,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   374,   375,   376,   379,
     383,   393,   394,   397,   398,   399,   400,   401,   402,   405,
     406,   407,   410,   411,   412,   413,   414,   415,   416,   417,
     422,   423,   424,   425,   426,   427,   430,   433,   438,   442,
     448,   453,   456,   461,   467,   467,   473,   478,   484,   490,
     496,   504,   505,   506,   507,   510,   510,   512,   513,   514,
     517,   521,   525,   531,   535,   539,   543,   544,   546,   548,
     550,   552,   556,   560,   564,   566,   567,   571,   577,   584,
     585,   588,   589,   593,   599,   606,   607,   613,   614,   615,
     616,   617,   618,   619,   620,   621,   622,   623,   624,   625,
     628,   630,   631,   634,   635,   638,   639,   640,   642,   644,
     646,   648,   650,   652,   654,   656,   659,   660,   661,   662,
     665,   666,   669,   674,   677,   682,   688,   689,   690,   691,
     692,   695,   696,   697,   698,   699,   702,   704,   705,   706,
     707,   708,   709,   710,   711,   712,   713,   717,   718,   719,
     720,   721,   724,   724,   726,   727,   730,   731,   732,   733,
     734,   737,   738,   739,   742,   746,   751,   752,   753,   756,
     757,   760,   761,   765,   766,   767,   768,   769,   770,   771,
     772,   773,   774,   775,   776,   777,   778,   779,   780,   781,
     782,   783,   790,   791,   792,   793,   794,   795,   796,   797,
     798,   799,   800,   801,   802,   803,   804,   805,   806,   807,
     808,   811,   812,   813,   814,   815,   816,   817,   818,   819,
     820,   821,   822,   823,   824,   825,   826,   827,   828,   831,
     832,   835,   836,   839,   843
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
    2942,  -524,  -524,  -524,  -524,  -524,     7,  -524,   -19,  -524,
    3521,  1859,     7,  1958,  -524,  -524,  3521,  -524,     3,  -524,
    -524,  2057,    83,  3602,  -524,  -524,  -524,  -524,  3521,  2057,
    1859,  3521,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  1958,  3521,  3521,   -73,  3844,
    -524,    46,  2942,  -524,  -524,  -524,  -524,  2057,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  2156,
    2156,  1958,  1958,  1958,  1958,     4,   -28,   -17,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,    25,   -49,  -524,  -524,  -524,  -524,
    2156,  2156,  2156,  2156,  1958,  1958,  2255,  1958,  1958,  1958,
    1958,  3683,  1958,  2255,  2255,  1364,   768,   -36,    27,    30,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,  -524,
     869,  -524,    33,  1958,   355,  3521,    50,    53,   968,  3521,
    1958,  3521,  -524,  -524,   -14,  -524,    66,    80,  1067,    81,
      93,    94,  4081,    99,  3521,  -524,  -524,  -524,  -524,   104,
    1166,  1463,  -524,  -524,  -524,   606,   674,  3897,  3919,   101,
    -524,  -524,  -524,  3521,  -524,  -524,  1265,  4446,  -524,   -21,
      -4,    24,    26,    52,   101,    48,    55,  4392,  -524,  -524,
    -524,  4113,    -9,   103,    47,   -76,   -48,  -524,  4446,   102,
     109,  1562,  -524,  -524,   182,  1958,  1958,  1958,  1958,  2651,
    2651,  2849,  1958,  2750,  2750,  1958,  1958,  1958,  1958,  1958,
    1958,  1958,  1958,  -524,  -524,  4209,  -524,  -524,  4196,  3023,
    1958,  1958,  1958,  1958,  1958,   113,  -524,  -524,  3764,  3764,
    3764,    -1,  4222,   184,  -524,  -524,  1958,   -59,  -524,  1958,
    -524,  -524,  -524,  3844,  3104,  -524,   114,  -524,  -524,  4137,
    2255,  1958,  2255,  1958,  2255,  1958,  2255,  1958,  -524,  -524,
    -524,  -524,    17,  -524,   187,  4459,  -524,  -524,  -524,  -524,
    -524,  -524,  -524,  -524,  -524,  -524,  2354,  2453,  2255,  2255,
    1661,  -524,  1958,  1958,  -524,  1958,  3440,  -524,  -524,  1958,
    -524,   116,  1958,    74,    74,    74,    74,    90,    90,  -524,
     -34,    74,    74,    74,    74,   -34,   -42,   -42,  -524,  -524,
     116,  1958,  1958,  1958,  1958,  1958,  1958,  1958,  1958,  1958,
    1958,  1958,  1958,  1958,  1958,  1958,  1958,  1958,  1958,  -524,
    3347,   203,  3104,     4,  -524,   122,  -524,   123,   124,  1958,
    1958,  3104,  2552,  4278,  3521,  -524,  -524,  -524,   -59,  -524,
    4291,  -524,  -524,  -524,   135,  3104,  -524,  3104,  1760,  -524,
    3951,  -524,  3973,  -524,  4005,  -524,  4027,  -524,  -524,  2255,
    1958,    17,    17,    17,    17,  2255,  2255,    17,  2255,  2255,
    -524,  -524,  -524,  -524,  -524,  -524,  -524,   116,  4446,  4446,
    4446,    -9,   103,   126,  -524,  4446,  1958,   133,  4446,  -524,
      74,    74,    74,    74,    90,    90,  -524,   -34,    74,    74,
      74,    74,   -34,   -42,   -42,  -524,  -524,   116,  -524,  -524,
      -6,  3347,  -524,  3185,  -524,  -524,  -524,  -524,  4304,   429,
     152,  1958,  1958,  1958,  1958,  -524,  -524,  -524,  3521,  -524,
    -524,   143,  -524,   233,  -524,   116,  2255,  2255,  2255,  2255,
    -524,  4446,  2255,  2255,  2255,  2255,  -524,   215,   178,  -524,
    -524,   153,  -524,  -524,  3266,   157,  -524,  -524,  3347,  -524,
    3104,   222,  1958,   164,  -524,  4472,  -524,  4059,  4472,  4472,
     166,  -524,  3521,   165,  -524,  -524,  -524,  -524,  -524,  -524,
    -524,  -524,   195,  2255,  -524,  -524,  3347,  -524,  -524,   185,
     193,  1958,  4360,  -524,  1958,  1958,  1958,  1958,  1958,  1958,
    1958,  1958,  1958,  1958,  1958,  1958,  1958,  1958,  1958,  1958,
     200,  3104,    43,  -524,  -524,  2255,   185,  -524,   186,  4373,
    3104,   198,   198,   198,   183,   183,  -524,    -7,   198,   198,
     198,   198,    -7,    -3,    -3,  -524,  -524,  1958,  -524,  -524,
    3521,  -524,  -524,  -524,  3104,   202,  4472,  -524,   208,   188,
     216,  -524,  -524
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    24,    31,    38,    62,    72,    36,    37,     0,    39,
      96,   126,    40,     0,    41,    43,     0,    54,    55,    58,
      60,   126,    61,     0,    70,   164,    73,    71,     0,   126,
     126,    96,    29,    59,    56,    53,    28,    30,    34,    67,
      32,    33,    44,    45,    47,    48,    75,    76,    25,    26,
      27,    49,    64,    35,    46,    50,    51,    52,    57,    68,
      69,    65,    66,    42,    74,     0,    96,     0,     0,    63,
       5,     0,     2,     3,     6,     7,     8,   126,     9,   101,
     103,   109,   110,   111,   104,   105,   106,   107,   108,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   122,    79,
      36,    78,    80,    82,    83,    40,    84,    86,    55,    92,
      61,    93,    95,    81,    89,    90,    85,    94,    91,    88,
      87,    63,    77,    99,     0,    97,   177,   178,   180,   179,
      31,    38,    62,    72,    39,    54,    56,    32,    44,    47,
      75,    68,    66,     0,     0,     0,   321,   190,     0,     0,
     261,   185,   186,   187,   188,   219,   220,   189,   262,   263,
     134,   264,     0,     0,     0,    96,     0,     0,   134,     0,
       0,    68,   190,   193,     0,   194,     0,     0,   134,     0,
       0,     0,     0,     0,    96,   102,   125,     1,     4,     0,
     134,     0,   196,   212,   195,     0,     0,     0,     0,     0,
     123,   121,   147,    98,   210,   211,   136,   137,   181,    25,
      26,    27,    49,    64,    46,    57,   216,     0,   259,   260,
     131,     0,   180,   179,     0,   190,     0,   254,   323,     0,
     322,     0,   114,   115,    58,     0,     0,     0,     0,    29,
      59,    53,     0,    34,    67,     0,     0,     0,     0,     0,
       0,     0,   135,   127,   282,   134,   283,   124,     0,     0,
       0,     0,     0,     0,     0,     0,   120,   113,    43,    28,
      30,     0,     0,   244,   145,   146,     0,    13,   118,    73,
     116,   117,   149,     0,   165,   148,     0,   112,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   206,   208,
     209,   207,     0,   100,    58,   140,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,     0,     0,     0,     0,
       0,   184,     0,     0,   252,     0,     0,   253,   251,     0,
     182,   134,     0,   274,   275,   272,   273,   276,   277,   269,
     279,   280,   281,   271,   270,   278,   265,   266,   267,   268,
     134,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   135,   129,
     171,   150,   165,    68,   191,     0,   192,     0,     0,     0,
       0,   165,     0,     0,     0,    17,    18,    11,    14,    15,
       0,   162,   169,   170,     0,   166,   167,   165,     0,   197,
       0,   201,     0,   203,     0,   199,     0,   242,   243,     0,
       0,    33,    45,    48,    76,    50,    51,    52,    65,    66,
     218,   222,   221,   217,   319,   320,   132,   134,   258,   256,
     257,     0,     0,     0,   255,   324,   135,     0,   139,   128,
     293,   294,   291,   292,   295,   296,   288,   298,   299,   300,
     290,   289,   297,   284,   285,   286,   287,   134,   175,   176,
     156,   172,   173,     0,    10,   141,   142,   143,     0,     0,
       0,    50,    51,    65,    66,   245,   246,   144,    96,    16,
     119,     0,   168,    20,   214,   134,     0,     0,     0,     0,
     205,   138,     0,     0,     0,     0,   226,     0,     0,   224,
     225,     0,   183,   130,     0,     0,   151,   174,   171,   152,
     165,     0,     0,     0,   301,   247,   302,     0,   249,   250,
       0,   163,    21,     0,   198,   202,   204,   200,   237,   239,
     240,   238,     0,     0,   241,   133,   171,   154,   157,   156,
       0,     0,     0,   158,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   165,     0,    22,   215,     0,   156,   153,     0,     0,
     165,   311,   312,   310,   313,   314,   307,   316,   317,   318,
     309,   308,   315,   303,   304,   305,   306,     0,    12,    19,
       0,   223,   155,   161,   165,     0,   248,    23,     0,     0,
       0,   159,   160
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -524,  -524,  -524,   214,  -524,  -524,  -524,  -524,  -524,   -74,
    -524,  -524,     0,    -8,   -25,  -524,     5,  -358,   -65,  -524,
       9,  -252,   286,  -524,  -524,  -524,  -524,  -524,  -523,  -524,
    -524,  -524,  -344,  -524,   -78,  -487,  -524,  -142,    51,   -22,
    -169,  -524,     1,  -524,  -524,    41,  -524,    10,  -178,  -524,
    -524,  -223,  -524,  -524,  -524,  -524,    -5,  -524,   199,    62,
     127,  -439,    76,  -524,  -221
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    71,    72,    73,    74,    75,   386,   387,   388,   389,
      76,   562,   122,   147,   124,   125,   393,    79,    80,    81,
     148,   253,   149,    82,   276,    83,    84,    85,   506,    86,
      87,    88,   394,   395,   396,   460,   461,   462,   150,   151,
     375,   174,   152,   302,   153,   192,   154,   420,   421,   155,
     156,   409,   175,   475,   157,   226,   227,   158,   159,   228,
     255,   515,   161,   229,   230
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      77,   173,   123,   369,   186,    78,   181,    96,   165,   262,
     331,   504,   459,   162,    97,   172,   567,   262,    89,    90,
     176,   539,   274,   123,   379,   231,   185,   384,   464,   325,
     167,   350,   275,   517,   518,   519,   166,   470,   177,   179,
     385,   183,   407,   592,   549,   380,   187,   408,   549,   566,
     306,   203,   326,   483,   250,   251,   327,   307,   123,   184,
     248,   249,   250,   251,    91,   231,    92,   308,    93,    77,
      94,   200,    77,   160,   309,   164,   199,    78,   316,   437,
      98,   505,   201,   168,    95,   317,   189,   556,   557,   558,
     559,   178,   160,   558,   559,   310,   322,   312,   439,   427,
     377,   378,   311,   459,   313,   571,   572,   573,   574,   575,
     576,   577,   578,   579,   580,   581,   582,   583,   584,   585,
     586,   169,   170,   314,   202,   262,   232,   182,   242,   233,
     315,   194,   257,   216,   235,   236,   237,   238,   225,   190,
     265,   262,   589,   590,   242,   263,   264,   457,   596,   266,
     459,   324,   267,   195,   196,   197,   198,   123,   298,   286,
     299,   271,   300,   273,   301,   277,   540,   247,   248,   249,
     250,   251,   194,   204,   205,   501,   123,   485,   459,   278,
     280,   245,   246,   247,   248,   249,   250,   251,   492,   493,
     494,   495,   281,   282,   498,   303,   206,   207,   285,   195,
     196,   197,   198,   287,   217,   503,   328,   221,   323,   329,
     332,   254,   372,   397,   382,   410,   436,   588,   391,   254,
     463,   465,   466,   467,   481,   258,   595,   544,   545,   254,
     546,   325,   272,   523,   549,   502,   256,   550,   551,   552,
     513,   254,   521,   522,   256,   532,   374,   374,   374,   549,
     598,   533,   550,   289,   256,   535,   538,   254,   541,    77,
     172,   172,   172,   543,   371,   561,   256,   564,   565,   376,
     376,   376,   505,   587,   553,   554,   555,   556,   557,   558,
     559,   568,   256,    77,    77,   593,   188,   601,   193,   193,
     599,   555,   556,   557,   558,   559,   600,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   479,   602,   180,   482,   433,   507,
     534,   434,   337,   338,   339,   341,   342,   423,     0,   193,
     193,   193,   193,   305,     0,   208,     0,     0,   383,     0,
       0,   390,   218,   219,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   400,     0,   402,     0,   404,     0,   406,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      77,     0,    77,     0,     0,     0,   478,     0,     0,     0,
       0,    77,     0,     0,   428,   429,     0,   430,     0,     0,
     259,   435,     0,     0,   438,    77,     0,    77,     0,   235,
     236,   237,   238,   260,   261,     0,   262,     0,     0,   242,
     263,   264,     0,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   468,   469,     0,     0,   511,   245,   246,   247,   248,
     249,   250,   251,   520,     0,     0,     0,     0,     0,     0,
       0,    77,     0,    77,     0,   512,     0,     0,   509,     0,
     123,     0,   491,   235,   236,   237,   238,   260,   261,     0,
     262,     0,     0,   242,   263,   264,     0,     0,     0,   399,
       0,   401,     0,   403,     0,   405,     0,     0,   435,     0,
       0,     0,     0,     0,    77,     0,     0,     0,    77,   537,
      77,     0,     0,     0,   563,   422,   422,   424,   425,     0,
     245,   246,   247,   248,   249,   250,   251,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   516,   516,   516,
     516,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,   542,     0,     0,     0,     0,     0,
       0,   476,   597,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,     0,
       0,     0,     0,   569,     0,     0,     0,     0,   490,     0,
       0,     0,     0,     0,   496,   497,     0,   499,   500,     0,
     516,   516,   516,   516,   516,   516,   516,   516,   516,   516,
     516,   516,   516,   516,   516,   516,   290,     0,     0,     0,
       0,     0,   291,     0,     0,     0,     0,     0,     0,     0,
     235,   236,   237,   238,   260,   261,     0,   262,     0,     0,
     242,   263,   264,   516,     0,     0,     0,     0,     0,     0,
     514,   514,   514,   514,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   524,   525,   526,   527,     0,
       0,   528,   529,   530,   531,     0,     0,   245,   246,   247,
     248,   249,   250,   251,   292,     0,     0,     0,     0,     0,
     293,     0,     0,     0,     0,     0,     0,     0,   235,   236,
     237,   238,   260,   261,     0,   262,     0,     0,   242,   263,
     264,     0,   496,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   514,   514,   514,   514,   514,   514,   514,
     514,   514,   514,   514,   514,   514,   514,   514,   514,     0,
       0,     0,     0,     0,   591,   245,   246,   247,   248,   249,
     250,   251,     0,   126,   127,     1,   222,   223,    99,   130,
     131,   132,   133,   100,     7,   101,   514,   102,     9,   103,
     104,   105,   106,    14,    15,   107,    17,   108,    19,    20,
     109,   110,   111,   112,    24,     0,    26,    27,   113,   114,
     115,   116,     0,     0,     0,     0,    32,    33,   136,    35,
      36,    37,     0,    38,    39,   137,    41,   138,    43,   139,
      45,   140,    47,    48,    49,    50,    51,    52,     0,    53,
      54,    55,    56,    57,    58,   141,    60,    61,   142,    63,
      64,   117,   118,   119,   120,     0,     0,     0,   121,     0,
       0,     0,   143,   144,     0,     0,     0,     0,     0,   163,
       0,   146,     0,   224,   126,   127,     1,   128,   129,    99,
     130,   131,   132,   133,   100,     7,   101,     0,   102,     9,
     103,   104,   105,   106,    14,    15,   107,    17,   108,   234,
      20,   109,   110,   111,   112,    24,     0,    26,    27,   113,
     114,   115,   116,   235,   236,   237,   238,   239,   240,   136,
     241,    36,    37,   242,   243,   244,   137,    41,   138,    43,
     139,    45,   140,    47,    48,    49,    50,    51,    52,     0,
      53,    54,    55,    56,    57,    58,   141,    60,    61,   142,
      63,    64,   117,   118,   119,   120,     0,     0,     0,   121,
     245,   246,   247,   248,   249,   250,   251,     0,     0,   252,
     163,     0,   146,   126,   127,     1,   128,   129,    99,   130,
     131,   132,   133,   100,     7,   101,     0,   102,     9,   103,
     104,   105,   106,    14,   268,   107,    17,   108,    19,    20,
     109,   110,   111,   112,    24,     0,    26,    27,   113,   114,
     115,   116,   235,   236,   237,   238,   239,   240,   136,   241,
     269,   270,   242,   243,   244,   137,    41,   138,    43,   139,
      45,   140,    47,    48,    49,    50,    51,    52,     0,    53,
      54,    55,    56,    57,    58,   141,    60,    61,   142,    63,
      64,   117,   118,   119,   120,     0,     0,     0,   121,   245,
     246,   247,   248,   249,   250,   251,     0,     0,   252,   163,
       0,   146,   126,   127,     1,   128,   129,    99,   130,   131,
     132,   133,   100,     7,   101,     0,   102,     9,   103,   104,
     105,   106,    14,    15,   107,    17,   108,    19,    20,   109,
     110,   111,   112,    24,     0,   279,    27,   113,   114,   115,
     116,   235,   236,   237,   238,   239,   240,   136,   241,    36,
      37,   242,   243,   244,   137,    41,   138,    43,   139,    45,
     140,    47,    48,    49,    50,    51,    52,     0,    53,    54,
      55,    56,    57,    58,   141,    60,    61,   142,    63,    64,
     117,   118,   119,   120,     0,     0,     0,   121,   245,   246,
     247,   248,   249,   250,   251,     0,     0,   252,   163,     0,
     146,   126,   127,     1,   128,   129,    99,   130,   131,   132,
     133,   100,     7,   101,     0,   102,     9,   103,   104,   105,
     106,    14,    15,   107,    17,   108,    19,    20,   109,   110,
     111,   112,    24,     0,    26,    27,   113,   114,   115,   116,
     235,   236,   237,   238,   239,   240,   136,   241,    36,    37,
     242,   243,   244,   137,    41,   138,    43,   139,    45,   140,
      47,    48,    49,    50,    51,    52,     0,    53,    54,    55,
      56,    57,    58,   141,    60,    61,   142,    63,    64,   117,
     118,   119,   120,     0,     0,     0,   121,   245,   246,   247,
     248,   249,   250,   251,     0,     0,   252,   163,     0,   146,
     126,   127,     1,   128,   129,    99,   130,   131,   132,   133,
     100,     7,   101,     0,   102,     9,   103,   104,   105,   106,
      14,    15,   107,    17,   108,   304,    20,   109,   110,   111,
     112,    24,     0,    26,    27,   113,   114,   115,   116,   235,
     236,   237,   238,   239,   240,   136,   241,    36,    37,   242,
     243,   244,   137,    41,   138,    43,   139,    45,   140,    47,
      48,    49,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,   141,    60,    61,   142,    63,    64,   117,   118,
     119,   120,     0,     0,     0,   121,   245,   246,   247,   248,
     249,   250,   251,     0,     0,     0,   163,     0,   146,   126,
     127,     1,   128,   129,    99,   130,   131,   132,   133,   100,
       7,   101,     0,   102,     9,   103,   104,   105,   106,    14,
      15,   107,    17,   108,    19,    20,   109,   110,   111,   112,
      24,     0,    26,    27,   113,   114,   115,   116,     0,     0,
       0,     0,    32,    33,   136,    35,    36,    37,     0,    38,
      39,   137,    41,   138,    43,   139,    45,   140,    47,    48,
      49,    50,    51,    52,     0,    53,    54,    55,    56,    57,
      58,   141,    60,    61,   142,    63,    64,   117,   118,   119,
     120,     0,     0,     0,   121,     0,     0,     0,   143,   144,
       0,     0,     0,     0,     0,   163,   220,   146,   126,   127,
       1,   128,   129,    99,   130,   131,   132,   133,   100,     7,
     101,     0,   102,     9,   103,   104,   105,   106,    14,    15,
     107,    17,   108,    19,    20,   109,   110,   111,   112,    24,
       0,    26,    27,   113,   114,   115,   116,     0,     0,     0,
       0,    32,    33,   136,    35,    36,    37,     0,    38,    39,
     137,    41,   138,    43,   139,    45,   140,    47,    48,    49,
      50,    51,    52,     0,    53,    54,    55,    56,    57,    58,
     141,    60,    61,   142,    63,    64,   117,   118,   119,   120,
       0,     0,     0,   121,     0,     0,     0,   143,   144,     0,
       0,     0,     0,     0,   163,   288,   146,   126,   127,     1,
     128,   129,    99,   130,   131,   132,   133,   100,     7,   101,
       0,   102,     9,   103,   104,   105,   106,    14,    15,   107,
      17,   108,    19,    20,   109,   110,   111,   112,    24,     0,
      26,    27,   113,   114,   115,   116,     0,     0,     0,     0,
      32,    33,   136,    35,    36,    37,     0,    38,    39,   137,
      41,   138,    43,   139,    45,   140,    47,    48,    49,    50,
      51,    52,     0,    53,    54,    55,    56,    57,    58,   141,
      60,    61,   142,    63,    64,   117,   118,   119,   120,     0,
       0,     0,   121,     0,     0,     0,   143,   144,     0,     0,
       0,     0,     0,   163,   330,   146,   126,   127,     1,   128,
     129,    99,   130,   131,   132,   133,   100,     7,   101,     0,
     102,     9,   103,   104,   105,   106,    14,    15,   107,    17,
     108,    19,    20,   109,   110,   111,   112,    24,     0,    26,
      27,   113,   114,   115,   116,     0,     0,     0,     0,    32,
      33,   136,    35,    36,    37,     0,    38,    39,   137,    41,
     138,    43,   139,    45,   140,    47,    48,    49,    50,    51,
      52,     0,    53,    54,    55,    56,    57,    58,   141,    60,
      61,   142,    63,    64,   117,   118,   119,   120,     0,     0,
       0,   121,     0,     0,     0,   143,   144,     0,     0,     0,
       0,     0,   163,   426,   146,   126,   127,     1,   128,   129,
      99,   130,   131,   132,   133,   100,     7,   101,     0,   102,
       9,   103,   104,   105,   106,    14,    15,   107,    17,   108,
      19,    20,   109,   110,   111,   112,    24,     0,    26,    27,
     113,   114,   115,   116,     0,     0,     0,     0,    32,    33,
     136,    35,    36,    37,     0,    38,    39,   137,    41,   138,
      43,   139,    45,   140,    47,    48,    49,    50,    51,    52,
       0,    53,    54,    55,    56,    57,    58,   141,    60,    61,
     142,    63,    64,   117,   118,   119,   120,     0,     0,     0,
     121,     0,     0,     0,   143,   144,     0,     0,     0,     0,
       0,   163,   484,   146,   126,   127,     1,   128,   129,    99,
     130,   131,   132,   133,   100,     7,   101,     0,   102,   134,
     103,   104,   105,   106,    14,    15,   107,   135,   108,    19,
      20,   109,   110,   111,   112,    24,     0,    26,    27,   113,
     114,   115,   116,     0,     0,     0,     0,    32,    33,   136,
      35,    36,    37,     0,    38,    39,   137,    41,   138,    43,
     139,    45,   140,    47,    48,    49,    50,    51,    52,     0,
      53,    54,    55,    56,    57,    58,   141,    60,    61,   142,
      63,    64,   117,   118,   119,   120,     0,     0,     0,   121,
       0,     0,     0,   143,   144,     0,     0,     0,     0,     0,
     145,     0,   146,   126,   127,     1,   128,   129,    99,   130,
     131,   132,   133,   100,     7,   101,     0,   102,     9,   103,
     104,   105,   106,    14,    15,   107,    17,   108,    19,    20,
     109,   110,   111,   112,    24,     0,    26,    27,   113,   114,
     115,   116,     0,     0,     0,     0,    32,    33,   136,    35,
      36,    37,     0,    38,    39,   137,    41,   138,    43,   139,
      45,   140,    47,    48,    49,    50,    51,    52,     0,    53,
      54,    55,    56,    57,    58,   141,    60,    61,   142,    63,
      64,   117,   118,   119,   120,     0,     0,     0,   121,     0,
       0,     0,   143,   144,     0,     0,     0,     0,     0,   163,
       0,   146,   126,   127,     1,   128,   129,    99,   130,   131,
     132,   133,   100,     7,   101,     0,   102,     9,   103,   104,
     105,   106,    14,    15,   107,    17,   108,    19,    20,   109,
     110,   111,   112,    24,     0,    26,    27,   113,   114,   115,
     116,     0,     0,     0,     0,    32,    33,   136,    35,    36,
      37,     0,    38,    39,   137,    41,   138,    43,   139,    45,
     140,    47,    48,    49,    50,    51,    52,     0,    53,    54,
      55,    56,    57,    58,   141,    60,    61,   142,    63,    64,
     117,   118,   119,   120,     0,     0,     0,   121,     0,     0,
       0,   143,   144,     0,     0,     0,     0,     0,   145,     0,
     146,   126,   127,     1,   128,   129,    99,   130,   131,   132,
     133,   100,     7,   101,     0,   102,     9,   103,   104,   105,
     106,    14,    15,   107,    17,   108,    19,    20,   109,   110,
     111,   112,    24,     0,    26,    27,   113,   114,   115,   116,
       0,     0,     0,     0,    32,    33,   136,    35,    36,    37,
       0,    38,    39,   137,    41,   138,    43,   139,    45,   140,
      47,    48,    49,    50,    51,    52,     0,    53,    54,    55,
      56,    57,    58,   141,    60,    61,    62,    63,    64,   117,
     118,   119,   120,     0,     0,     0,   121,     0,     0,     0,
     143,   144,     0,     0,     0,     0,     0,   191,     0,   146,
     126,   127,     1,   128,   129,    99,   130,   131,   132,   133,
     100,     7,   101,     0,   102,     9,   103,   104,   105,   106,
      14,    15,   107,    17,   108,    19,    20,   109,   110,   111,
     112,    24,     0,    26,    27,   113,   114,   115,   116,     0,
       0,     0,     0,    32,    33,   136,    35,    36,    37,     0,
      38,    39,   137,    41,   138,    43,   139,    45,   140,    47,
      48,    49,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,   141,    60,    61,    62,    63,    64,   117,   118,
     119,   120,     0,     0,     0,   121,     0,     0,     0,   143,
     144,     0,     0,     0,     0,     0,   163,     0,   146,   126,
     127,     1,   128,   129,    99,   130,   131,   132,   133,   100,
       7,   101,     0,   102,     9,   103,   104,   105,   106,    14,
      15,   107,    17,   108,    19,    20,   109,   110,   111,   112,
      24,     0,    26,    27,   113,   114,   115,   116,     0,     0,
       0,     0,    32,    33,   136,    35,    36,    37,     0,    38,
      39,   137,   411,   138,   412,   139,   413,   140,   414,    48,
      49,    50,    51,    52,     0,    53,    54,   415,   416,   417,
      58,   141,    60,   418,   419,    63,    64,   117,   118,   119,
     120,     0,     0,     0,   121,     0,     0,     0,   143,   144,
       0,     0,     0,     0,     0,   163,     0,   146,   126,   127,
       1,   128,   129,    99,   130,   131,   132,   133,   100,     7,
     101,     0,   102,     9,   103,   104,   105,   106,    14,    15,
     107,    17,   108,    19,    20,   109,   110,   111,   112,    24,
       0,    26,    27,   113,   114,   115,   116,     0,     0,     0,
       0,    32,    33,   136,    35,    36,    37,     0,    38,    39,
     137,    41,   138,    43,   139,    45,   140,    47,    48,    49,
      50,    51,    52,     0,    53,    54,   415,   416,    57,    58,
     141,    60,   418,   419,    63,    64,   117,   118,   119,   120,
       0,     0,     0,   121,     0,     0,     0,   143,   144,     0,
       0,     0,     0,     0,   163,     0,   146,   126,   127,     1,
     128,   129,    99,   130,   131,   132,   133,   100,     7,   101,
       0,   102,     9,   103,   104,   105,   106,    14,    15,   107,
      17,   108,    19,    20,   109,   110,   111,   112,    24,     0,
      26,    27,   113,   114,   115,   116,     0,     0,     0,     0,
      32,    33,   136,    35,    36,    37,     0,    38,    39,   137,
      41,   138,    43,   139,    45,   140,    47,    48,    49,    50,
      51,    52,     0,    53,    54,   471,   472,    57,    58,   141,
      60,   473,   474,    63,    64,   117,   118,   119,   120,     0,
       0,     0,   121,     0,     0,     0,   143,   144,     0,     0,
       0,     0,     0,   163,     0,   146,   126,   127,     1,   128,
     129,    99,   130,   131,   132,   133,   100,     7,   101,     0,
     102,     9,   103,   104,   105,   106,    14,    15,   107,    17,
     108,    19,    20,   109,   110,   111,   112,    24,     0,    26,
      27,   113,   114,   115,   116,     0,     0,     0,     0,     0,
       0,   136,    35,    36,    37,     0,    38,    39,   137,    41,
     138,    43,   139,    45,   140,    47,    48,    49,    50,    51,
      52,     0,    53,    54,    55,    56,    57,    58,   141,    60,
      61,   142,    63,    64,   117,   118,   119,   120,     0,     0,
       0,   121,     0,     0,     0,   143,   144,     0,     0,     0,
       0,     0,   163,     0,   146,   126,   127,     1,   128,   129,
      99,   130,   131,   132,   133,   100,     7,   101,     0,   102,
       9,   103,   104,   105,   106,    14,    15,   107,    17,   108,
      19,    20,   109,   110,   111,   112,    24,     0,    26,    27,
     113,   114,   115,   116,     0,     0,     0,     0,     0,     0,
     136,    35,    36,    37,     0,     0,     0,   137,    41,   138,
      43,   139,    45,   140,    47,    48,    49,    50,    51,    52,
       0,    53,    54,    55,    56,    57,    58,   141,    60,    61,
     142,    63,    64,   117,   118,   119,   120,     0,     0,     0,
     121,     0,     0,     0,   143,   144,     0,     0,     0,     0,
       0,   163,     0,   146,   126,   127,     1,   128,   129,    99,
     130,   131,   132,   133,   100,     7,   101,     0,   102,     9,
     103,   104,   105,   106,    14,    15,   107,    17,   108,    19,
      20,   109,   110,   111,   112,    24,     0,    26,    27,   113,
     114,   115,   116,     0,     0,     0,     0,     0,     0,   136,
       0,    36,    37,     0,     0,     0,   137,    41,   138,    43,
     139,    45,   140,    47,    48,    49,    50,    51,    52,     0,
      53,    54,    55,    56,    57,    58,   141,    60,    61,   142,
      63,    64,   117,   118,   119,   120,     0,     0,     0,   121,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     1,
     163,     0,   146,     2,     3,     4,     5,     6,     7,     0,
       0,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,    25,
      26,    27,    28,    29,    30,    31,     0,     0,     0,     0,
      32,    33,    34,    35,    36,    37,     0,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,     0,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,    68,
       1,     0,    69,     0,     2,     3,     4,     5,     6,     7,
       0,    70,     8,     9,    10,    11,    12,    13,    14,    15,
       0,    17,    18,    19,    20,    21,    22,    23,     0,    24,
      25,    26,    27,     0,    29,    30,    31,     0,     0,     0,
       0,    32,    33,    34,    35,    36,    37,     0,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,     0,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,     0,     0,
      68,     1,     0,    69,     0,     2,     3,     4,     5,     6,
       7,     0,   370,     8,     9,    10,    11,    12,    13,    14,
      15,     0,    17,    18,    19,    20,    21,    22,    23,     0,
      24,    25,    26,    27,     0,    29,    30,    31,     0,     0,
       0,     0,    32,    33,    34,    35,    36,    37,     0,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,     0,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,     0,
       0,    68,     1,     0,    69,     0,     2,     3,     4,     5,
       6,     7,     0,   392,     8,     9,    10,    11,    12,    13,
      14,    15,     0,    17,    18,    19,    20,    21,    22,    23,
       0,    24,    25,    26,    27,     0,    29,    30,    31,     0,
       0,     0,     0,    32,    33,    34,    35,    36,    37,     0,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
       0,     0,    68,     1,     0,    69,     0,     2,     3,     4,
       5,     6,     7,     0,   508,     8,     9,    10,    11,    12,
      13,    14,    15,     0,    17,    18,    19,    20,    21,    22,
      23,     0,    24,    25,    26,    27,     0,    29,    30,    31,
       0,     0,     0,     0,    32,    33,    34,    35,    36,    37,
       0,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,     0,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,     0,     0,    68,     1,     0,    69,     0,     2,     3,
       4,     5,     6,     7,     0,   536,     8,     9,    10,    11,
      12,    13,    14,    15,     0,    17,    18,    19,    20,    21,
      22,    23,     0,    24,    25,    26,    27,     0,    29,    30,
      31,     0,     0,     0,     0,    32,    33,    34,    35,    36,
      37,     0,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,     0,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,     0,     0,     0,     0,     0,    69,     0,     0,
       0,     0,     0,     0,     0,     0,   458,     1,   431,   432,
      99,     2,     3,     4,     5,   100,     7,   101,     0,   102,
       9,   103,   104,   105,   106,    14,    15,   107,    17,   108,
      19,    20,   109,   110,   111,   112,    24,     0,    26,    27,
     113,   114,   115,   116,     0,     0,     0,     0,    32,    33,
      34,    35,    36,    37,     0,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
       0,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,   117,   118,   119,   120,     0,     1,     0,
     121,    99,     2,     3,     4,     5,   100,     7,   101,     0,
     102,     9,   103,   104,   105,   106,    14,    15,   107,    17,
     108,    19,    20,   109,   110,   111,   112,    24,     0,    26,
      27,   113,   114,   115,   116,     0,     0,     0,     0,    32,
      33,    34,    35,    36,    37,     0,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,     0,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,   117,   118,   119,   120,     0,     1,
       0,   121,    99,     2,     3,     4,     5,   100,     7,   101,
       0,   102,     9,   103,   104,   105,   106,    14,    15,   107,
      17,   108,    19,    20,   109,   110,   111,   112,    24,     0,
      26,    27,   113,   114,   115,   116,     0,     0,     0,     0,
      32,    33,    34,    35,    36,    37,     0,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,     0,    53,    54,    55,    56,    57,    58,   171,
      60,    61,    62,    63,    64,   117,   118,   119,   120,     0,
       1,     0,   121,    99,     2,     3,     4,     5,   100,     7,
     101,     0,   102,     9,   103,   104,   105,   106,    14,    15,
     107,    17,   108,    19,    20,   109,   110,   111,   112,    24,
       0,    26,    27,   113,   114,   115,   116,     0,     0,     0,
       0,    32,    33,    34,    35,    36,    37,     0,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   209,   210,
     211,   212,   213,     0,    53,   214,    55,    56,    57,   215,
      59,    60,    61,    62,    63,    64,   117,   118,   119,   120,
       0,     1,     0,   121,    99,   130,   131,     4,     5,   100,
       7,   101,     0,   102,     9,   103,   104,   105,   106,    14,
      15,   107,    17,   108,    19,    20,   109,   110,   111,   112,
      24,     0,    26,    27,   113,   114,   115,   116,     0,     0,
       0,     0,    32,    33,    34,    35,    36,    37,     0,    38,
      39,   137,    41,   138,    43,   139,    45,   140,    47,    48,
      49,    50,    51,    52,     0,    53,    54,    55,    56,    57,
      58,   373,    60,    61,    62,    63,    64,   117,   118,   119,
     120,     1,     0,     0,   121,     2,     3,     4,     5,     6,
       7,     0,     0,     8,     9,    10,    11,    12,     0,    14,
      15,     0,    17,    18,    19,    20,    21,   110,    23,     0,
      24,     0,    26,    27,     0,    29,    30,    31,     0,     0,
       0,     0,    32,    33,    34,    35,    36,    37,     0,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,     0,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,   294,    66,     0,
       0,     0,     0,   295,    69,     0,     0,     0,     0,     0,
       0,   235,   236,   237,   238,   260,   261,     0,   262,   296,
       0,   242,   263,   264,     0,   297,     0,     0,     0,     0,
       0,     0,     0,   235,   236,   237,   238,   260,   261,     0,
     262,     0,     0,   242,   263,   264,     0,     0,     0,     0,
       0,   486,     0,     0,     0,     0,     0,     0,   245,   246,
     247,   248,   249,   250,   251,   235,   236,   237,   238,   260,
     261,     0,   262,   487,     0,   242,   263,   264,     0,     0,
     245,   246,   247,   248,   249,   250,   251,   235,   236,   237,
     238,   260,   261,     0,   262,     0,     0,   242,   263,   264,
       0,     0,     0,     0,     0,   488,     0,     0,     0,     0,
       0,     0,   245,   246,   247,   248,   249,   250,   251,   235,
     236,   237,   238,   260,   261,     0,   262,   489,     0,   242,
     263,   264,     0,     0,   245,   246,   247,   248,   249,   250,
     251,   235,   236,   237,   238,   260,   261,     0,   262,     0,
       0,   242,   263,   264,     0,     0,     0,     0,     0,   560,
       0,     0,     0,     0,     0,     0,   245,   246,   247,   248,
     249,   250,   251,   544,   545,     0,   546,   547,   548,     0,
     549,     0,     0,   550,   551,   552,     0,   283,   245,   246,
     247,   248,   249,   250,   251,   235,   236,   237,   238,   260,
     261,     0,   262,     0,     0,   242,   263,   264,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     553,   554,   555,   556,   557,   558,   559,   235,   236,   237,
     238,   260,   261,     0,   262,     0,     0,   242,   263,   264,
       0,     0,   245,   246,   247,   248,   249,   250,   251,     0,
     284,   235,   236,   237,   238,   260,   261,     0,   262,     0,
       0,   242,   263,   264,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   245,   246,   247,   248,   249,   250,
     251,     0,     0,   320,     0,   321,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   245,   246,
     247,   248,   249,   250,   251,     0,     0,   398,     0,   321,
     235,   236,   237,   238,   260,   261,     0,   262,     0,     0,
     242,   263,   264,   351,   352,   353,   354,   355,   356,     0,
     357,     0,     0,   358,   359,   360,   235,   236,   237,   238,
     260,   261,     0,   262,     0,     0,   242,   263,   264,     0,
       0,     0,     0,     0,     0,     0,     0,   245,   246,   247,
     248,   249,   250,   251,     0,     0,     0,     0,   321,     0,
     361,   362,   363,   364,   365,   366,   367,     0,     0,   368,
       0,     0,     0,   245,   246,   247,   248,   249,   250,   251,
       0,   381,   235,   236,   237,   238,   260,   261,     0,   262,
       0,     0,   242,   263,   264,   235,   236,   237,   238,   260,
     261,     0,   262,     0,     0,   242,   263,   264,   235,   236,
     237,   238,   260,   261,     0,   262,     0,     0,   242,   263,
     264,     0,     0,     0,     0,     0,     0,     0,     0,   245,
     246,   247,   248,   249,   250,   251,     0,   477,     0,     0,
       0,     0,   245,   246,   247,   248,   249,   250,   251,     0,
     480,     0,     0,     0,     0,   245,   246,   247,   248,   249,
     250,   251,     0,   510,   235,   236,   237,   238,   260,   261,
       0,   262,     0,     0,   242,   263,   264,   235,   236,   237,
     238,   260,   261,     0,   262,     0,     0,   242,   263,   264,
       0,     0,     0,     0,     0,     0,   235,   236,   237,   238,
     260,   261,     0,   262,     0,     0,   242,   263,   264,     0,
       0,   245,   246,   247,   248,   249,   250,   251,     0,   570,
       0,     0,     0,     0,   245,   246,   247,   248,   249,   250,
     251,     0,   594,   318,   319,     0,     0,     0,     0,     0,
       0,     0,     0,   245,   246,   247,   248,   249,   250,   251,
     235,   236,   237,   238,   260,   261,     0,   262,     0,     0,
     242,   263,   264,   351,   352,   353,   354,   355,   356,     0,
     357,     0,     0,   358,   359,   360,   544,   545,     0,   546,
     547,   548,     0,   549,     0,     0,   550,   551,   552,     0,
       0,     0,     0,     0,     0,     0,     0,   245,   246,   247,
     248,   249,   250,   251,     0,     0,     0,     0,     0,     0,
     361,   362,   363,   364,   365,   366,   367,     0,     0,     0,
       0,     0,     0,   553,   554,   555,   556,   557,   558,   559
};

static const yytype_int16 yycheck[] =
{
       0,    23,    10,   255,    69,     0,    31,     6,    16,    51,
     231,    17,   370,    12,    33,    23,   539,    51,    11,    12,
      28,   508,    36,    31,    25,   101,    99,    86,   372,   105,
      21,   252,    46,   472,   473,   474,    33,   381,    29,    30,
      99,    66,    25,   566,    51,    46,     0,    30,    51,   536,
      71,   100,   100,   397,    96,    97,   104,    78,    66,    67,
      94,    95,    96,    97,    57,   101,    59,    71,    61,    69,
      63,    99,    72,    11,    78,    13,    72,    72,    30,   331,
      99,    87,    99,    21,    77,    30,    77,    94,    95,    96,
      97,    29,    30,    96,    97,    71,   105,    71,   350,   320,
     269,   270,    78,   461,    78,   544,   545,   546,   547,   548,
     549,   550,   551,   552,   553,   554,   555,   556,   557,   558,
     559,    38,    39,    71,    99,    51,    99,    65,    54,    99,
      78,    90,    99,   141,    44,    45,    46,    47,   146,    77,
     165,    51,    99,   100,    54,    55,    56,   368,   587,    99,
     508,   104,    99,    91,    92,    93,    94,   165,    57,   184,
      59,   169,    61,   171,    63,    99,   510,    93,    94,    95,
      96,    97,   131,   132,   133,   427,   184,   398,   536,    99,
      99,    91,    92,    93,    94,    95,    96,    97,   411,   412,
     413,   414,    99,    99,   417,   203,   134,   135,    99,   137,
     138,   139,   140,    99,   142,   457,   104,   145,   105,   100,
      28,   160,    99,    99,    30,    28,   100,   561,   283,   168,
      17,    99,    99,    99,    89,   163,   570,    44,    45,   178,
      47,   105,   170,   485,    51,   102,   160,    54,    55,    56,
      88,   190,    99,    10,   168,    30,   268,   269,   270,    51,
     594,    73,    54,   191,   178,   102,    99,   206,    36,   259,
     268,   269,   270,    99,   259,    99,   190,   102,    73,   268,
     269,   270,    87,    73,    91,    92,    93,    94,    95,    96,
      97,    88,   206,   283,   284,    99,    72,    99,    89,    90,
      88,    93,    94,    95,    96,    97,    88,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   388,    99,    30,   395,   326,   461,
     498,   326,   260,   261,   262,   263,   264,   317,    -1,   130,
     131,   132,   133,   206,    -1,   136,    -1,    -1,   276,    -1,
      -1,   279,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   291,    -1,   293,    -1,   295,    -1,   297,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     370,    -1,   372,    -1,    -1,    -1,   384,    -1,    -1,    -1,
      -1,   381,    -1,    -1,   322,   323,    -1,   325,    -1,    -1,
      35,   329,    -1,    -1,   332,   395,    -1,   397,    -1,    44,
      45,    46,    47,    48,    49,    -1,    51,    -1,    -1,    54,
      55,    56,    -1,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   379,   380,    -1,    -1,    16,    91,    92,    93,    94,
      95,    96,    97,   478,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   461,    -1,   463,    -1,    36,    -1,    -1,   463,    -1,
     478,    -1,   410,    44,    45,    46,    47,    48,    49,    -1,
      51,    -1,    -1,    54,    55,    56,    -1,    -1,    -1,   290,
      -1,   292,    -1,   294,    -1,   296,    -1,    -1,   436,    -1,
      -1,    -1,    -1,    -1,   504,    -1,    -1,    -1,   508,   504,
     510,    -1,    -1,    -1,   522,   316,   317,   318,   319,    -1,
      91,    92,    93,    94,    95,    96,    97,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   536,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   471,   472,   473,
     474,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   561,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     570,    -1,    -1,    -1,   512,    -1,    -1,    -1,    -1,    -1,
      -1,   382,   590,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   594,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   541,    -1,    -1,    -1,    -1,   409,    -1,
      -1,    -1,    -1,    -1,   415,   416,    -1,   418,   419,    -1,
     544,   545,   546,   547,   548,   549,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,    30,    -1,    -1,    -1,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    46,    47,    48,    49,    -1,    51,    -1,    -1,
      54,    55,    56,   587,    -1,    -1,    -1,    -1,    -1,    -1,
     471,   472,   473,   474,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   486,   487,   488,   489,    -1,
      -1,   492,   493,   494,   495,    -1,    -1,    91,    92,    93,
      94,    95,    96,    97,    30,    -1,    -1,    -1,    -1,    -1,
      36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      46,    47,    48,    49,    -1,    51,    -1,    -1,    54,    55,
      56,    -1,   533,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   544,   545,   546,   547,   548,   549,   550,
     551,   552,   553,   554,   555,   556,   557,   558,   559,    -1,
      -1,    -1,    -1,    -1,   565,    91,    92,    93,    94,    95,
      96,    97,    -1,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,   587,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    38,    39,    40,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    49,    50,    51,
      52,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    -1,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,   101,
      -1,   103,    -1,   105,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    95,    96,    97,    -1,    -1,   100,
     101,    -1,   103,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    -1,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    -1,    -1,   100,   101,
      -1,   103,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    -1,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    90,    91,    92,
      93,    94,    95,    96,    97,    -1,    -1,   100,   101,    -1,
     103,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    -1,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    91,    92,    93,
      94,    95,    96,    97,    -1,    -1,   100,   101,    -1,   103,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    97,    -1,    -1,    -1,   101,    -1,   103,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    -1,    38,    39,    40,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    49,    50,    51,    52,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,    -1,   101,   102,   103,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    49,    50,    51,    52,    53,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    -1,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,
      -1,    -1,    -1,    -1,   101,   102,   103,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    -1,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,
      -1,    -1,    -1,   101,   102,   103,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,    -1,   101,   102,   103,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,
      50,    51,    52,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
      -1,   101,   102,   103,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,    50,
      51,    52,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,
     101,    -1,   103,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    38,    39,    40,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    49,    50,    51,
      52,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    -1,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,   101,
      -1,   103,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    -1,    38,    39,    40,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    49,    50,    51,    52,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    -1,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,
      -1,    94,    95,    -1,    -1,    -1,    -1,    -1,   101,    -1,
     103,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    -1,    38,    39,    40,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    49,    50,    51,    52,    53,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      94,    95,    -1,    -1,    -1,    -1,    -1,   101,    -1,   103,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    -1,    38,    39,    40,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    49,    50,    51,    52,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,
      95,    -1,    -1,    -1,    -1,    -1,   101,    -1,   103,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    -1,    38,    39,    40,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    49,    50,    51,    52,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,    -1,   101,    -1,   103,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    49,    50,    51,    52,    53,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    -1,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,
      -1,    -1,    -1,    -1,   101,    -1,   103,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    -1,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,
      -1,    -1,    -1,   101,    -1,   103,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,    -1,   101,    -1,   103,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    52,    53,    -1,    -1,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
      -1,   101,    -1,   103,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    -1,    38,    39,    40,
      41,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    52,    53,    -1,    -1,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     7,
     101,    -1,   103,    11,    12,    13,    14,    15,    16,    -1,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    -1,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    -1,    87,
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
      -1,    87,     7,    -1,    90,    -1,    11,    12,    13,    14,
      15,    16,    -1,    99,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    49,    50,    51,    52,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    -1,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    87,     7,    -1,    90,    -1,    11,    12,    13,
      14,    15,    16,    -1,    99,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    49,    50,    51,    52,    53,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    87,     7,    -1,    90,    -1,    11,    12,
      13,    14,    15,    16,    -1,    99,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    -1,    36,    37,    38,    39,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    49,    50,    51,    52,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    -1,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    90,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    -1,    38,    39,
      40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    49,
      50,    51,    52,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,     7,    -1,
      90,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    -1,    38,
      39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,     7,
      -1,    90,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    -1,
      38,    39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    -1,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
       7,    -1,    90,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      -1,    38,    39,    40,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    49,    50,    51,    52,    53,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    -1,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,     7,    -1,    90,    10,    11,    12,    13,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    -1,    38,    39,    40,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    49,    50,    51,    52,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,     7,    -1,    -1,    90,    11,    12,    13,    14,    15,
      16,    -1,    -1,    19,    20,    21,    22,    23,    -1,    25,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    -1,    38,    39,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    49,    50,    51,    52,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    30,    84,    -1,
      -1,    -1,    -1,    36,    90,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    46,    47,    48,    49,    -1,    51,    30,
      -1,    54,    55,    56,    -1,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    46,    47,    48,    49,    -1,
      51,    -1,    -1,    54,    55,    56,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    44,    45,    46,    47,    48,
      49,    -1,    51,    30,    -1,    54,    55,    56,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    44,    45,    46,
      47,    48,    49,    -1,    51,    -1,    -1,    54,    55,    56,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    44,
      45,    46,    47,    48,    49,    -1,    51,    30,    -1,    54,
      55,    56,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    44,    45,    46,    47,    48,    49,    -1,    51,    -1,
      -1,    54,    55,    56,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,
      95,    96,    97,    44,    45,    -1,    47,    48,    49,    -1,
      51,    -1,    -1,    54,    55,    56,    -1,    36,    91,    92,
      93,    94,    95,    96,    97,    44,    45,    46,    47,    48,
      49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    44,    45,    46,
      47,    48,    49,    -1,    51,    -1,    -1,    54,    55,    56,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    -1,
      99,    44,    45,    46,    47,    48,    49,    -1,    51,    -1,
      -1,    54,    55,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    -1,    -1,   100,    -1,   102,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    -1,    -1,   100,    -1,   102,
      44,    45,    46,    47,    48,    49,    -1,    51,    -1,    -1,
      54,    55,    56,    44,    45,    46,    47,    48,    49,    -1,
      51,    -1,    -1,    54,    55,    56,    44,    45,    46,    47,
      48,    49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,
      94,    95,    96,    97,    -1,    -1,    -1,    -1,   102,    -1,
      91,    92,    93,    94,    95,    96,    97,    -1,    -1,   100,
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    97,
      -1,    99,    44,    45,    46,    47,    48,    49,    -1,    51,
      -1,    -1,    54,    55,    56,    44,    45,    46,    47,    48,
      49,    -1,    51,    -1,    -1,    54,    55,    56,    44,    45,
      46,    47,    48,    49,    -1,    51,    -1,    -1,    54,    55,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    96,    97,    -1,    99,    -1,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    -1,
      99,    -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,
      96,    97,    -1,    99,    44,    45,    46,    47,    48,    49,
      -1,    51,    -1,    -1,    54,    55,    56,    44,    45,    46,
      47,    48,    49,    -1,    51,    -1,    -1,    54,    55,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    46,    47,
      48,    49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    -1,    99,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    -1,    99,    81,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    97,
      44,    45,    46,    47,    48,    49,    -1,    51,    -1,    -1,
      54,    55,    56,    44,    45,    46,    47,    48,    49,    -1,
      51,    -1,    -1,    54,    55,    56,    44,    45,    -1,    47,
      48,    49,    -1,    51,    -1,    -1,    54,    55,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,
      94,    95,    96,    97,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    -1,    -1,    -1,
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
      33,    34,    35,    40,    41,    42,    43,    83,    84,    85,
      86,    90,   118,   119,   120,   121,     5,     6,     8,     9,
      11,    12,    13,    14,    20,    28,    50,    57,    59,    61,
      63,    77,    80,    94,    95,   101,   103,   119,   126,   128,
     144,   145,   148,   150,   152,   155,   156,   160,   163,   164,
     165,   168,   148,   101,   165,   119,    33,   126,   165,    38,
      39,    77,   119,   145,   147,   158,   119,   126,   165,   126,
     128,   120,   165,   120,   119,    99,   124,     0,   109,   126,
     165,   101,   151,   164,   151,   165,   165,   165,   165,    72,
      99,    99,    99,   100,   151,   151,   165,   165,   164,    65,
      66,    67,    68,    69,    72,    76,   119,   165,   164,   164,
     102,   165,     8,     9,   105,   119,   161,   162,   165,   169,
     170,   101,    99,    99,    30,    44,    45,    46,    47,    48,
      49,    51,    54,    55,    56,    91,    92,    93,    94,    95,
      96,    97,   100,   127,   144,   166,   168,    99,   165,    35,
      48,    49,    51,    55,    56,   120,    99,    99,    26,    52,
      53,   119,   165,   119,    36,    46,   130,    99,    99,    38,
      99,    99,    99,    36,    99,    99,   120,    99,   102,   165,
      30,    36,    30,    36,    30,    36,    30,    36,    57,    59,
      61,    63,   149,   119,    30,   166,    71,    78,    71,    78,
      71,    78,    71,    78,    71,    78,    30,    30,    81,    82,
     100,   102,   105,   105,   104,   105,   100,   104,   104,   100,
     102,   170,    28,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     170,    44,    45,    46,    47,    48,    49,    51,    54,    55,
      56,    91,    92,    93,    94,    95,    96,    97,   100,   127,
      99,   122,    99,    77,   145,   146,   148,   146,   146,    25,
      46,    99,    30,   165,    86,    99,   112,   113,   114,   115,
     165,   124,    99,   122,   138,   139,   140,    99,   100,   164,
     165,   164,   165,   164,   165,   164,   165,    25,    30,   157,
      28,    58,    60,    62,    64,    73,    74,    75,    79,    80,
     153,   154,   164,   153,   164,   164,   102,   170,   165,   165,
     165,     8,     9,   119,   162,   165,   100,   127,   165,   127,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   170,    99,   123,
     141,   142,   143,    17,   138,    99,    99,    99,   165,   165,
     138,    73,    74,    79,    80,   159,   164,    99,   119,   115,
      99,    89,   140,   138,   102,   170,    30,    30,    30,    30,
     164,   165,   157,   157,   157,   157,   164,   164,   157,   164,
     164,   127,   102,   127,    17,    87,   134,   143,    99,   122,
      99,    16,    36,    88,   164,   167,   168,   167,   167,   167,
     120,    99,    10,   127,   164,   164,   164,   164,   164,   164,
     164,   164,    30,    73,   154,   102,    99,   122,    99,   141,
     138,    36,   165,    99,    44,    45,    47,    48,    49,    51,
      54,    55,    56,    91,    92,    93,    94,    95,    96,    97,
      30,    99,   117,   119,   102,    73,   141,   134,    88,   165,
      99,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,    73,   138,    99,
     100,   164,   134,    99,    99,   138,   167,   119,   138,    88,
      88,    99,    99
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
     119,   119,   119,   119,   119,   119,   120,   120,   120,   121,
     121,   122,   122,   123,   123,   123,   123,   123,   123,   124,
     124,   124,   125,   125,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   125,   126,   126,   126,   126,
     126,   126,   126,   126,   127,   127,   128,   128,   128,   128,
     128,   129,   129,   129,   129,   130,   130,   131,   131,   131,
     132,   132,   133,   133,   133,   133,   134,   134,   135,   135,
     135,   135,   136,   136,   137,   138,   138,   139,   139,   140,
     140,   141,   141,   142,   142,   143,   143,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     145,   146,   146,   147,   147,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   149,   149,   149,   149,
     150,   150,   151,   151,   151,   151,   152,   152,   152,   152,
     152,   153,   153,   153,   153,   153,   154,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   156,   156,   156,
     156,   156,   157,   157,   158,   158,   159,   159,   159,   159,
     159,   160,   160,   160,   161,   161,   162,   162,   162,   163,
     163,   164,   164,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   168,
     168,   169,   169,   170,   170
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
       1,     1,     1,     1,     1,     1,     0,     1,     2,     1,
       3,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     5,
       3,     3,     2,     3,     3,     2,     0,     2,     4,     3,
       5,     2,     4,     6,     0,     1,     2,     2,     5,     4,
       3,     5,     5,     5,     5,     1,     1,     3,     3,     3,
       4,     6,     6,     8,     7,     9,     0,     2,     7,    11,
      12,     9,     4,     6,     1,     0,     1,     1,     2,     1,
       1,     0,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     2,     3,     5,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     4,     6,     4,
       6,     4,     6,     4,     6,     5,     1,     1,     1,     1,
       2,     2,     1,     2,     4,     6,     2,     4,     4,     1,
       1,     1,     1,     5,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     6,     6,     6,
       6,     6,     1,     1,     2,     4,     1,     2,     5,     2,
       2,     3,     3,     3,     1,     3,     3,     3,     3,     2,
       2,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     4,
       4,     0,     1,     1,     3
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
#line 196 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2325 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 196 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2331 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 196 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2337 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 196 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2343 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 196 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2349 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 196 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2355 "engines/director/lingo/lingo-gr.cpp"
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
#line 202 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->_compiler->_assemblyAST = new ScriptNode((yyvsp[0].nodelist)); }
#line 2623 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 204 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2634 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 210 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2644 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 217 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2650 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 250 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2656 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* factory: tFACTORY ID '\n' methodlist  */
#line 254 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2662 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 256 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2668 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* methodlist: %empty  */
#line 258 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2674 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* nonemptymethodlist: methodlistline  */
#line 262 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2685 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 268 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2695 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlistline: '\n'  */
#line 275 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2701 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 281 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2710 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* handler: tON ID idlist '\n' stmtlist  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* endargdef: ID  */
#line 290 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2723 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* endargdef: endargdef ',' ID  */
#line 291 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* CMDID: tABBREVIATED  */
#line 299 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* CMDID: tABBREV  */
#line 300 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* CMDID: tABBR  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2747 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* CMDID: tAFTER  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* CMDID: tAND  */
#line 303 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 2759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* CMDID: tBEFORE  */
#line 304 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2765 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* CMDID: tCAST  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2771 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tCHAR  */
#line 306 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2777 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tCHARS  */
#line 307 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tCONTAINS  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("contains"); }
#line 2789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tDATE  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tDELETE  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 2801 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tDOWN  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2807 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tFIELD  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tFRAME  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2819 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tHILITE  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 2825 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tIN  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2831 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tINTERSECTS  */
#line 316 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tINTO  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2843 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tITEM  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2849 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tITEMS  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2855 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tLAST  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tLINE  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2867 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tLINES  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2873 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tLONG  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2879 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tMENU  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2885 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tMENUITEM  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2891 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tMENUITEMS  */
#line 326 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2897 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tMOD  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2903 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tMOVIE  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2909 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tNEXT  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2915 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tNOT  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2921 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tNUMBER  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2927 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tOF  */
#line 332 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2933 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tOR  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tPREVIOUS  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tREPEAT  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tSCRIPT  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tASSERTERROR  */
#line 337 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2963 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tSHORT  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2969 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tSOUND  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2975 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tSPRITE  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tSTARTS  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2987 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tTHE  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 2993 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tTIME  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 2999 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tTO  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 3005 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tWHILE  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 3011 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tWINDOW  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 3017 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* CMDID: tWITH  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 3023 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* CMDID: tWITHIN  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 3029 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* CMDID: tWORD  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 3035 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* CMDID: tWORDS  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 3041 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* ID: tELSE  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 3047 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* ID: tENDCLAUSE  */
#line 355 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); }
#line 3053 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* ID: tEXIT  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 3059 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tFACTORY  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 3065 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tGLOBAL  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 3071 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tGO  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 3077 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tIF  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 3083 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tINSTANCE  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 3089 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tMACRO  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3095 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tMETHOD  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3101 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tON  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3107 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tOPEN  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3113 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tPLAY  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3119 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tPROPERTY  */
#line 367 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3125 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tPUT  */
#line 368 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3131 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ID: tSET  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3137 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* ID: tTELL  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3143 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* ID: tTHEN  */
#line 371 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3149 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* idlist: %empty  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3155 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* nonemptyidlist: ID  */
#line 379 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3164 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 383 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102: /* stmt: tENDIF '\n'  */
#line 394 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3178 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* proc: CMDID cmdargs '\n'  */
#line 410 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3184 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* proc: tPUT cmdargs '\n'  */
#line 411 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* proc: tGO cmdargs '\n'  */
#line 412 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3196 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* proc: tGO frameargs '\n'  */
#line 413 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3202 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: tPLAY cmdargs '\n'  */
#line 414 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3208 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: tPLAY frameargs '\n'  */
#line 415 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3214 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tOPEN cmdargs '\n'  */
#line 416 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tOPEN expr tWITH expr '\n'  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3230 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tNEXT tREPEAT '\n'  */
#line 422 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3236 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tEXIT tREPEAT '\n'  */
#line 423 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3242 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* proc: tEXIT '\n'  */
#line 424 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3248 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proc: tDELETE chunk '\n'  */
#line 425 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3254 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tHILITE chunk '\n'  */
#line 426 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3260 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proc: tASSERTERROR stmtoneliner  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3266 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* cmdargs: %empty  */
#line 430 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3274 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* cmdargs: expr trailingcomma  */
#line 433 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3284 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 438 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd args, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3293 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 442 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 448 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3314 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* cmdargs: '(' ')'  */
#line 453 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* cmdargs: '(' expr ',' ')'  */
#line 456 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(args,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3332 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 461 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(args, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3341 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* frameargs: tFRAME expr  */
#line 473 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3351 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* frameargs: tMOVIE expr  */
#line 478 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3362 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 484 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3373 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* frameargs: expr tOF tMOVIE expr  */
#line 490 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3384 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* frameargs: tFRAME expr expr_nounarymath  */
#line 496 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3395 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 504 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3401 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 505 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3407 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 506 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3413 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* asgn: tSET varorthe to expr '\n'  */
#line 507 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3419 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* definevars: tGLOBAL idlist '\n'  */
#line 512 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3425 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* definevars: tPROPERTY idlist '\n'  */
#line 513 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3431 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* definevars: tINSTANCE idlist '\n'  */
#line 514 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3437 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* ifstmt: tIF expr tTHEN stmt  */
#line 517 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3446 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 521 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 3453 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 525 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3464 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 531 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 3473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 535 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3482 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 3489 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* endif: %empty  */
#line 543 "engines/director/lingo/lingo-gr.y"
                        { warning("LingoCompiler::parse: no end if"); }
#line 3495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 546 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3502 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 548 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3509 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 550 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 552 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3523 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* tell: tTELL expr tTO stmtoneliner  */
#line 556 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3532 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 560 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3539 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* when: tWHEN  */
#line 564 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new WhenNode((yyvsp[0].w).eventName, (yyvsp[0].w).stmt); }
#line 3545 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* stmtlist: %empty  */
#line 566 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3551 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* nonemptystmtlist: stmtlistline  */
#line 571 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3562 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 577 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3572 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* stmtlistline: '\n'  */
#line 584 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3578 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* stmtlist_insideif: %empty  */
#line 588 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 3584 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 593 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3595 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 599 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* stmtlistline_insideif: '\n'  */
#line 606 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* simpleexpr_nounarymath: tINT  */
#line 613 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3617 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* simpleexpr_nounarymath: tFLOAT  */
#line 614 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3623 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* simpleexpr_nounarymath: tSYMBOL  */
#line 615 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3629 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* simpleexpr_nounarymath: tSTRING  */
#line 616 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3635 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 617 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3641 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 618 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 3647 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 619 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 3653 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 620 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3659 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* var: ID  */
#line 628 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3665 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* chunk: tFIELD refargs  */
#line 638 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 3671 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* chunk: tCAST refargs  */
#line 639 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 3677 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* chunk: tCHAR expr tOF simpleexpr  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3684 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* chunk: tWORD expr tOF simpleexpr  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3698 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* chunk: tITEM expr tOF simpleexpr  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3712 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3719 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* chunk: tLINE expr tOF simpleexpr  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3733 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 656 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 3739 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* chunktype: tCHAR  */
#line 659 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 3745 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* chunktype: tWORD  */
#line 660 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 3751 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* chunktype: tITEM  */
#line 661 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 3757 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* chunktype: tLINE  */
#line 662 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 3763 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* object: tSCRIPT refargs  */
#line 665 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 3769 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* object: tWINDOW refargs  */
#line 666 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 3775 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* refargs: simpleexpr  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3785 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* refargs: '(' ')'  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 3793 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* refargs: '(' expr ',' ')'  */
#line 677 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(args,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3803 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 682 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(args, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3812 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* the: tTHE ID  */
#line 688 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* the: tTHE ID tOF theobj  */
#line 689 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3824 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* the: tTHE tNUMBER tOF theobj  */
#line 690 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 3830 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 223: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 697 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3836 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 224: /* theobj: tSOUND simpleexpr  */
#line 698 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3842 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 225: /* theobj: tSPRITE simpleexpr  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3848 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 226: /* menu: tMENU simpleexpr  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3854 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 704 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3860 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 705 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3866 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 229: /* thedatetime: tTHE tABBREV tDATE  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3872 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 230: /* thedatetime: tTHE tABBREV tTIME  */
#line 707 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* thedatetime: tTHE tABBR tDATE  */
#line 708 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3884 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* thedatetime: tTHE tABBR tTIME  */
#line 709 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3890 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* thedatetime: tTHE tLONG tDATE  */
#line 710 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 3896 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* thedatetime: tTHE tLONG tTIME  */
#line 711 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 3902 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* thedatetime: tTHE tSHORT tDATE  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 3908 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* thedatetime: tTHE tSHORT tTIME  */
#line 713 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 3914 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 717 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 3920 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 718 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 3926 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 719 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 3932 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 720 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 3938 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 721 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 3944 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* writablethe: tTHE ID  */
#line 726 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3950 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* writablethe: tTHE ID tOF writabletheobj  */
#line 727 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3956 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* writabletheobj: tMENU expr_noeq  */
#line 731 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3962 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 732 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3968 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* writabletheobj: tSOUND expr_noeq  */
#line 733 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3974 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* writabletheobj: tSPRITE expr_noeq  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3980 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* list: '[' exprlist ']'  */
#line 737 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 3986 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* list: '[' ':' ']'  */
#line 738 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 3992 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* list: '[' proplist ']'  */
#line 739 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 3998 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* proplist: proppair  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* proplist: proplist ',' proppair  */
#line 746 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4015 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* proppair: tSYMBOL ':' expr  */
#line 751 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4021 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* proppair: ID ':' expr  */
#line 752 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4027 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* proppair: tSTRING ':' expr  */
#line 753 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4033 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* unarymath: '+' simpleexpr  */
#line 756 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 4039 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* unarymath: '-' simpleexpr  */
#line 757 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 4045 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 265: /* expr: expr '+' expr  */
#line 767 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4051 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* expr: expr '-' expr  */
#line 768 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4057 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* expr: expr '*' expr  */
#line 769 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4063 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* expr: expr '/' expr  */
#line 770 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4069 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* expr: expr tMOD expr  */
#line 771 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4075 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 270: /* expr: expr '>' expr  */
#line 772 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4081 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 271: /* expr: expr '<' expr  */
#line 773 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4087 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* expr: expr tEQ expr  */
#line 774 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4093 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* expr: expr tNEQ expr  */
#line 775 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4099 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* expr: expr tGE expr  */
#line 776 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4105 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* expr: expr tLE expr  */
#line 777 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4111 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* expr: expr tAND expr  */
#line 778 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4117 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* expr: expr tOR expr  */
#line 779 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4123 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* expr: expr '&' expr  */
#line 780 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4129 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* expr: expr tCONCAT expr  */
#line 781 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4135 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* expr: expr tCONTAINS expr  */
#line 782 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* expr: expr tSTARTS expr  */
#line 783 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4147 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 284: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 792 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4153 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 793 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4159 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 794 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4165 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 287: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 795 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4171 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 288: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 796 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4177 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 289: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 797 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 290: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 798 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4189 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 291: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 799 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4195 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 292: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 800 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4201 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 801 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4207 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 294: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 802 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 295: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 803 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4219 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 296: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 804 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4225 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 805 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4231 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 806 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4237 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 807 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4243 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 808 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4249 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 303: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 813 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4255 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 304: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 814 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4261 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 815 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4267 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 816 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4273 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 307: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 817 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4279 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 308: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 818 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4285 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 309: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 819 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4291 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 310: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4297 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 311: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 821 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4303 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 312: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4309 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 313: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 823 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4315 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 314: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 824 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4321 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 315: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 825 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4327 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 826 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 827 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 828 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 831 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4351 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 832 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4357 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* exprlist: %empty  */
#line 835 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 4363 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 323: /* nonemptyexprlist: expr  */
#line 839 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList; 
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 324: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 843 "engines/director/lingo/lingo-gr.y"
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

#line 848 "engines/director/lingo/lingo-gr.y"


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
