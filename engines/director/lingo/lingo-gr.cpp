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
  YYSYMBOL_tSOUND = 78,                    /* tSOUND  */
  YYSYMBOL_tSPRITE = 79,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 80,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 81,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 82,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 83,                 /* tPROPERTY  */
  YYSYMBOL_tON = 84,                       /* tON  */
  YYSYMBOL_tMETHOD = 85,                   /* tMETHOD  */
  YYSYMBOL_tENDIF = 86,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 87,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 88,                  /* tENDTELL  */
  YYSYMBOL_tASSERTERROR = 89,              /* tASSERTERROR  */
  YYSYMBOL_90_ = 90,                       /* '<'  */
  YYSYMBOL_91_ = 91,                       /* '>'  */
  YYSYMBOL_92_ = 92,                       /* '&'  */
  YYSYMBOL_93_ = 93,                       /* '+'  */
  YYSYMBOL_94_ = 94,                       /* '-'  */
  YYSYMBOL_95_ = 95,                       /* '*'  */
  YYSYMBOL_96_ = 96,                       /* '/'  */
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
  YYSYMBOL_trailingcomma = 125,            /* trailingcomma  */
  YYSYMBOL_frameargs = 126,                /* frameargs  */
  YYSYMBOL_asgn = 127,                     /* asgn  */
  YYSYMBOL_to = 128,                       /* to  */
  YYSYMBOL_definevars = 129,               /* definevars  */
  YYSYMBOL_ifstmt = 130,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 131,               /* ifelsestmt  */
  YYSYMBOL_endif = 132,                    /* endif  */
  YYSYMBOL_loop = 133,                     /* loop  */
  YYSYMBOL_tell = 134,                     /* tell  */
  YYSYMBOL_when = 135,                     /* when  */
  YYSYMBOL_stmtlist = 136,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 137,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 138,             /* stmtlistline  */
  YYSYMBOL_stmtlist_insideif = 139,        /* stmtlist_insideif  */
  YYSYMBOL_nonemptystmtlist_insideif = 140, /* nonemptystmtlist_insideif  */
  YYSYMBOL_stmtlistline_insideif = 141,    /* stmtlistline_insideif  */
  YYSYMBOL_simpleexpr_nounarymath = 142,   /* simpleexpr_nounarymath  */
  YYSYMBOL_var = 143,                      /* var  */
  YYSYMBOL_varorchunk = 144,               /* varorchunk  */
  YYSYMBOL_varorthe = 145,                 /* varorthe  */
  YYSYMBOL_chunk = 146,                    /* chunk  */
  YYSYMBOL_chunktype = 147,                /* chunktype  */
  YYSYMBOL_object = 148,                   /* object  */
  YYSYMBOL_refargs = 149,                  /* refargs  */
  YYSYMBOL_the = 150,                      /* the  */
  YYSYMBOL_theobj = 151,                   /* theobj  */
  YYSYMBOL_menu = 152,                     /* menu  */
  YYSYMBOL_thedatetime = 153,              /* thedatetime  */
  YYSYMBOL_thenumberof = 154,              /* thenumberof  */
  YYSYMBOL_inof = 155,                     /* inof  */
  YYSYMBOL_writablethe = 156,              /* writablethe  */
  YYSYMBOL_writabletheobj = 157,           /* writabletheobj  */
  YYSYMBOL_list = 158,                     /* list  */
  YYSYMBOL_proplist = 159,                 /* proplist  */
  YYSYMBOL_proppair = 160,                 /* proppair  */
  YYSYMBOL_unarymath = 161,                /* unarymath  */
  YYSYMBOL_simpleexpr = 162,               /* simpleexpr  */
  YYSYMBOL_expr = 163,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 164,         /* expr_nounarymath  */
  YYSYMBOL_expr_noeq = 165,                /* expr_noeq  */
  YYSYMBOL_sprite = 166,                   /* sprite  */
  YYSYMBOL_exprlist = 167,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 168          /* nonemptyexprlist  */
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
#define YYLAST   4535

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  331
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  616

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   344


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
       2,     2,     2,     2,     2,     2,     2,     2,    92,     2,
      99,   100,    95,    93,    98,    94,     2,    96,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   103,     2,
      90,     2,    91,     2,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89
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
     438,   442,   448,   453,   456,   461,   467,   467,   473,   478,
     484,   490,   496,   504,   505,   506,   507,   510,   510,   512,
     513,   514,   517,   521,   525,   531,   535,   539,   543,   544,
     546,   548,   550,   552,   556,   560,   564,   566,   567,   571,
     577,   584,   585,   588,   589,   593,   599,   606,   607,   613,
     614,   615,   616,   617,   618,   619,   620,   621,   622,   623,
     624,   625,   628,   630,   631,   634,   635,   638,   639,   640,
     642,   644,   646,   648,   650,   652,   654,   656,   659,   660,
     661,   662,   665,   666,   669,   674,   677,   682,   688,   689,
     690,   691,   692,   695,   696,   697,   698,   699,   702,   704,
     705,   706,   707,   708,   709,   710,   711,   712,   713,   717,
     718,   719,   720,   721,   722,   723,   726,   726,   728,   729,
     732,   733,   734,   735,   736,   739,   740,   741,   747,   751,
     754,   759,   760,   761,   762,   763,   766,   767,   770,   771,
     775,   776,   777,   778,   779,   780,   781,   782,   783,   784,
     785,   786,   787,   788,   789,   790,   791,   792,   793,   800,
     801,   802,   803,   804,   805,   806,   807,   808,   809,   810,
     811,   812,   813,   814,   815,   816,   817,   818,   821,   822,
     823,   824,   825,   826,   827,   828,   829,   830,   831,   832,
     833,   834,   835,   836,   837,   838,   841,   842,   845,   846,
     849,   853
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
  "tXTRAS", "tSOUND", "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL",
  "tPROPERTY", "tON", "tMETHOD", "tENDIF", "tENDREPEAT", "tENDTELL",
  "tASSERTERROR", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'", "'/'", "'\\n'",
  "','", "'('", "')'", "'['", "']'", "':'", "$accept", "script",
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

#define YYPACT_NINF (-539)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    3056,  -539,  3550,  -539,  -539,  -539,  -539,    35,  -539,   -13,
    -539,  3550,  1982,    35,  2080,  -539,  -539,  3550,  -539,    36,
    -539,  -539,  2178,    51,  3631,  -539,   -70,  -539,  -539,  3550,
    2178,  1982,  3550,  -539,  -539,  -539,  -539,  -539,  -539,  -539,
    -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,
    -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,
    -539,  -539,  -539,  -539,  -539,  -539,  2080,  3550,  3550,   -68,
    3873,  -539,    98,  3056,  -539,  -539,  -539,  -539,  2178,  -539,
    -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,
    -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,
    -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,
    -539,  -539,  -539,    19,  -539,  -539,  2276,  2276,  2080,  2080,
    2080,  2080,    33,   -28,    12,  -539,  -539,    14,    32,  -539,
    -539,  -539,  -539,  2276,  2276,  2276,  2276,  2080,  2080,  2374,
    2080,  2080,  2080,  2080,  3712,  2080,  2374,  2374,  1492,   804,
       8,    30,    43,  -539,  -539,  -539,  -539,  -539,  -539,  -539,
    -539,  -539,  -539,   904,  -539,    62,  2080,  4045,  3550,    71,
      82,  1002,  3550,  2080,  3550,  -539,  -539,    -2,  -539,  -539,
      83,    85,  1100,    87,    93,    95,   527,    97,  3550,  -539,
    -539,  -539,  -539,    99,  1198,  -539,  3550,  1590,  -539,  -539,
    -539,   389,   605,   627,   696,     7,  -539,  -539,  -539,  3550,
    -539,  -539,  1296,  4406,  -539,   -45,   -18,   -15,   -11,     3,
       7,   105,   138,  4351,  -539,  -539,  -539,  4065,    73,    92,
     100,   103,    96,   -69,   -16,  -539,  4406,   106,   102,  1688,
    -539,  -539,   175,  2080,  2080,  2080,  2080,  2766,  2766,  2962,
    2080,  2864,  2864,  2080,  2080,  2080,  2080,  2080,  2080,  2080,
    2080,  -539,  -539,  4159,  -539,  -539,  4124,  3137,  2080,  2080,
    2080,  2080,  2080,   110,  -539,  -539,  3793,  3793,  3793,     5,
    4183,   174,  -539,  -539,  2080,     6,  -539,  2080,  -539,  -539,
    -539,  3873,  3218,  -539,   112,  -539,  -539,  -539,  4100,  2374,
    2080,  2374,  2080,  2374,  2080,  2374,  2080,  -539,  -539,  -539,
    -539,    57,  -539,   184,  4419,  -539,  -539,  -539,  -539,  -539,
    -539,  -539,  -539,  -539,  -539,  1394,  2472,  2374,  2374,  1786,
    -539,  2080,  2080,  2080,  2080,  -539,  2080,  2570,  -539,  -539,
    2080,  -539,   113,  2080,    29,    29,    29,    29,  4384,  4384,
    -539,    50,    29,    29,    29,    29,    50,    10,    10,  -539,
    -539,   113,  2080,  2080,  2080,  2080,  2080,  2080,  2080,  2080,
    2080,  2080,  2080,  2080,  2080,  2080,  2080,  2080,  2080,  2080,
    -539,  3461,   197,  3218,    33,  -539,   117,  -539,   118,   119,
    2080,  2080,  3218,  2668,  4216,  3550,  3550,  -539,  -539,  -539,
       6,  -539,  4239,  -539,  -539,  -539,   129,  3218,  -539,  3218,
    1884,  -539,  3922,  -539,  3935,  -539,  3955,  -539,  4010,  -539,
    -539,  2374,  2080,    57,    57,    57,    57,  2374,  -539,  2374,
      57,  -539,  2374,  2374,  -539,  -539,  -539,  -539,  -539,  -539,
    -539,   113,  4406,  4406,  4406,  4406,  4406,  -539,  4406,  4406,
    2080,   120,  4406,  -539,    29,    29,    29,    29,  4384,  4384,
    -539,    50,    29,    29,    29,    29,    50,    10,    10,  -539,
    -539,   113,  -539,  -539,    -3,  3461,  -539,  3299,  -539,  -539,
    -539,  -539,  4272,   511,   132,  2080,  2080,  2080,  2080,  -539,
    -539,  -539,    53,  3550,  -539,  -539,   124,  -539,   213,  -539,
     113,  2374,  2374,  2374,  2374,  -539,  4406,  2374,  2374,  2374,
    2374,  -539,   195,   154,  -539,  -539,   126,  -539,  -539,  3380,
     130,  -539,  -539,  3461,  -539,  3218,   194,  2080,   133,  -539,
     710,  -539,  4030,   710,   710,  -539,   137,  -539,  3550,   136,
    -539,  -539,  -539,  -539,  -539,  -539,  -539,  -539,   187,  2374,
    -539,  -539,  3461,  -539,  -539,   173,   182,  2080,  4295,  -539,
    2080,  2080,  2080,  2080,  2080,  2080,  2080,  2080,  2080,  2080,
    2080,  2080,  2080,  2080,  2080,  2080,   190,  3218,    59,  -539,
    2374,   173,  -539,   164,  4328,  3218,    77,    77,    77,  4439,
    4439,  -539,    69,    77,    77,    77,    77,    69,    79,    79,
    -539,  -539,  2080,  -539,  -539,  -539,  -539,  -539,  3218,   183,
     710,   188,   179,   180,  -539,  -539
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
       0,     0,     0,     0,     0,   124,   101,     0,    99,   179,
     180,   182,   181,    33,    40,    64,    74,    41,    56,    58,
      34,    46,    49,    77,    70,    68,     0,     0,     0,   328,
     192,     0,     0,   268,   187,   188,   189,   190,   221,   222,
     191,   269,   270,   136,   271,     0,     0,     0,    98,     0,
       0,   136,     0,     0,    70,   192,   195,     0,   196,   166,
       0,     0,   136,     0,     0,     0,     0,     0,    98,   104,
     127,     1,     4,     0,   136,    10,     0,     0,   198,   214,
     197,     0,     0,     0,     0,     0,   125,   123,   149,   100,
     212,   213,   138,   139,   183,    27,    28,    29,    51,    66,
      48,    59,   218,     0,   266,   267,   133,     0,   179,   180,
     182,   181,     0,   192,     0,   258,   330,     0,   329,     0,
     116,   117,    60,     0,     0,     0,     0,    31,    61,    55,
       0,    36,    69,     0,     0,     0,     0,     0,     0,     0,
     137,   129,   289,   136,   290,   126,     0,     0,     0,     0,
       0,     0,     0,     0,   122,   115,    45,    30,    32,     0,
       0,   248,   147,   148,     0,    14,   120,    75,   118,   119,
     151,     0,   167,   150,     0,   114,    25,   215,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   208,   210,   211,
     209,     0,   102,    60,   142,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,     0,     0,     0,     0,     0,
     186,     0,     0,     0,     0,   256,     0,     0,   257,   255,
       0,   184,   136,     0,   281,   282,   279,   280,   283,   284,
     276,   286,   287,   288,   278,   277,   285,   272,   273,   274,
     275,   136,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   137,
     131,   173,   152,   167,    70,   193,     0,   194,     0,     0,
       0,     0,   167,     0,     0,    23,     0,    18,    19,    12,
      15,    16,     0,   164,   171,   172,     0,   168,   169,   167,
       0,   199,     0,   203,     0,   205,     0,   201,     0,   246,
     247,     0,     0,    35,    47,    50,    78,    52,   244,    53,
      54,   245,    67,    68,   220,   224,   223,   219,   326,   327,
     134,   136,   264,   265,   263,   261,   262,   259,   260,   331,
     137,     0,   141,   130,   300,   301,   298,   299,   302,   303,
     295,   305,   306,   307,   297,   296,   304,   291,   292,   293,
     294,   136,   177,   178,   158,   174,   175,     0,    11,   143,
     144,   145,     0,     0,     0,    52,    53,    67,    68,   249,
     250,   146,     0,    98,    17,   121,     0,   170,    22,   216,
     136,     0,     0,     0,     0,   207,   140,     0,     0,     0,
       0,   228,     0,     0,   226,   227,     0,   185,   132,     0,
       0,   153,   176,   173,   154,   167,     0,     0,     0,   308,
     251,   309,     0,   253,   254,    20,     0,   165,    23,     0,
     200,   204,   206,   202,   239,   241,   242,   240,     0,     0,
     243,   135,   173,   156,   159,   158,     0,     0,     0,   160,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   167,     0,   217,
       0,   158,   155,     0,     0,   167,   318,   319,   317,   320,
     321,   314,   323,   324,   325,   316,   315,   322,   310,   311,
     312,   313,     0,    13,    21,   225,   157,   163,   167,     0,
     252,     0,     0,     0,   161,   162
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -539,  -539,  -539,   198,  -539,  -539,  -539,  -539,  -539,  -121,
    -539,  -383,     0,     9,   -27,  -539,     1,  -374,   -66,  -539,
      17,  -242,   249,  -539,  -539,  -539,  -539,  -539,  -538,  -539,
    -539,  -539,  -373,  -539,  -125,  -501,  -539,  -192,  -119,   -22,
    -130,  -539,    -4,  -539,  -539,   -80,  -539,   -42,  -224,  -539,
    -539,  -237,  -539,  -539,  -539,  -539,   -49,  -539,   205,    -6,
      78,  -255,   -74,  -539,  -225
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    72,    73,    74,    75,    76,   398,   399,   400,   401,
      77,   113,   114,   150,   127,   128,   405,    80,    81,    82,
     151,   261,   152,    83,   284,    84,    85,    86,   521,    87,
      88,    89,   406,   407,   408,   474,   475,   476,   153,   154,
     386,   177,   155,   311,   156,   198,   157,   434,   435,   158,
     159,   421,   178,   489,   160,   234,   235,   161,   162,   236,
     263,   530,   164,   237,   238
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      78,    79,   176,   123,   190,   185,   163,   473,   167,   165,
     478,   115,   492,   519,   342,   395,   171,   582,   124,   484,
     126,   380,   555,   315,   182,   163,   168,   179,   390,   189,
     239,   316,   282,   175,   336,   361,   498,   200,   180,   170,
     187,   126,   283,   606,   262,   116,   117,   181,   183,   391,
     317,   581,   262,   319,   200,   210,   211,   321,   318,   270,
     186,   320,   307,   262,   308,   322,   309,   169,   310,   206,
      78,   323,   194,    78,    79,   262,   126,   188,   270,   324,
     419,   250,   337,   520,   125,   420,   338,   172,   173,   264,
     118,   396,   119,   262,   120,   193,   121,   264,   191,   270,
     451,   473,   205,   397,   441,   258,   259,   239,   264,   207,
     122,   208,   201,   202,   203,   204,   195,   196,   565,   453,
     264,   255,   256,   257,   258,   259,   565,   240,   565,   566,
     209,   212,   213,   325,   201,   202,   203,   204,   264,   223,
     241,   273,   227,   256,   257,   258,   259,   388,   389,   473,
     535,   196,   556,   222,   471,   578,   604,   196,   233,   265,
     266,   294,   572,   573,   574,   575,   326,   280,   274,   571,
     572,   573,   574,   575,   574,   575,   331,   126,   473,   275,
     285,   279,   286,   281,   288,   500,   507,   508,   509,   510,
     289,   298,   290,   513,   293,   332,   295,   126,   335,   516,
     340,   343,   393,   333,   603,   296,   334,   383,   339,   409,
     422,   450,   609,   477,   479,   480,   481,   496,   312,   528,
     517,   537,   538,   548,   549,   403,   551,   554,   557,   518,
     559,   532,   533,   534,   577,   611,   579,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   385,   385,   385,   580,   539,   520,
     602,   607,   348,   349,   350,   352,   353,    78,   382,   583,
     612,   192,   387,   387,   387,   613,   614,   615,   394,   494,
     184,   402,   497,   522,   437,   175,   175,   175,   447,   550,
     314,    78,    78,     0,   412,     0,   414,     0,   416,     0,
     418,     0,     0,     0,     0,   586,   587,   588,   589,   590,
     591,   592,   593,   594,   595,   596,   597,   598,   599,   600,
     601,   199,   199,     0,     0,   442,   443,   444,   445,     0,
     446,   448,     0,     0,   449,     0,     0,   452,   199,   199,
     199,   199,     0,     0,   214,     0,   233,   610,     0,     0,
       0,   224,   225,     0,     0,     0,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,     0,     0,     0,     0,     0,     0,
       0,    78,     0,    78,   482,   483,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   115,   493,     0,    78,     0,    78,
       0,   531,   531,   531,   531,     0,   506,   299,     0,     0,
       0,     0,     0,   300,     0,     0,     0,     0,     0,     0,
       0,   243,   244,   245,   246,   268,   269,     0,   270,     0,
       0,   250,   271,   272,   449,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   536,     0,     0,     0,
       0,     0,     0,     0,     0,    78,     0,    78,   524,   253,
     254,   255,   256,   257,   258,   259,   531,   531,   531,   531,
     531,   531,   531,   531,   531,   531,   531,   531,   531,   531,
     531,   531,   126,     0,   411,     0,   413,     0,   415,     0,
     417,     0,     0,     0,     0,     0,     0,     0,     0,    78,
     553,   558,     0,    78,     0,    78,   526,     0,   531,     0,
     436,   436,   438,   439,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   527,     0,   115,     0,     0,
       0,   584,    78,   243,   244,   245,   246,   268,   269,     0,
     270,   291,     0,   250,   271,   272,     0,     0,     0,   243,
     244,   245,   246,   268,   269,     0,   270,    78,     0,   250,
     271,   272,     0,     0,     0,    78,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   490,     0,
       0,   253,   254,   255,   256,   257,   258,   259,    78,     0,
       0,     0,     0,     0,     0,     0,     0,   253,   254,   255,
     256,   257,   258,   259,   292,     0,   505,     0,     0,     0,
       0,     0,   511,   301,   512,     0,     0,   514,   515,   302,
       0,     0,     0,     0,     0,     0,     0,   243,   244,   245,
     246,   268,   269,     0,   270,   303,     0,   250,   271,   272,
       0,   304,     0,     0,     0,     0,     0,     0,     0,   243,
     244,   245,   246,   268,   269,     0,   270,     0,     0,   250,
     271,   272,     0,     0,     0,     0,     0,     0,     0,     0,
     529,   529,   529,   529,     0,   253,   254,   255,   256,   257,
     258,   259,     0,     0,     0,     0,   540,   541,   542,   543,
       0,     0,   544,   545,   546,   547,     0,   253,   254,   255,
     256,   257,   258,   259,   305,     0,     0,     0,     0,     0,
     306,     0,     0,     0,     0,     0,     0,     0,   243,   244,
     245,   246,   268,   269,     0,   270,     0,     0,   250,   271,
     272,     0,   560,   561,   511,   562,   563,   564,     0,   565,
       0,     0,   566,   567,   568,   529,   529,   529,   529,   529,
     529,   529,   529,   529,   529,   529,   529,   529,   529,   529,
     529,     0,     0,     0,     0,   605,   253,   254,   255,   256,
     257,   258,   259,     0,     0,     0,     0,     0,     0,     0,
     569,   570,   571,   572,   573,   574,   575,   529,   228,   229,
       1,   230,   231,    90,   133,   134,   135,   136,    91,     8,
      92,    93,    10,    94,    95,    96,    97,    15,    16,    98,
      18,    99,    20,    21,   100,   101,   102,   103,    25,     0,
      27,    28,   104,   105,   106,   107,     0,     0,     0,     0,
      33,    34,   139,    36,    37,    38,     0,    39,    40,   140,
      42,   141,    44,   142,    46,   143,    48,    49,    50,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,   144,
      61,     0,    62,   145,    64,    65,   108,   109,   110,   111,
       0,     0,     0,   112,     0,     0,     0,   146,   147,     0,
       0,     0,     0,   166,     0,   149,     0,   232,   129,   130,
       1,   131,   132,    90,   133,   134,   135,   136,    91,     8,
      92,    93,    10,    94,    95,    96,    97,    15,    16,    98,
      18,    99,   242,    21,   100,   101,   102,   103,    25,     0,
      27,    28,   104,   105,   106,   107,   243,   244,   245,   246,
     247,   248,   139,   249,    37,    38,   250,   251,   252,   140,
      42,   141,    44,   142,    46,   143,    48,    49,    50,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,   144,
      61,     0,    62,   145,    64,    65,   108,   109,   110,   111,
       0,     0,     0,   112,   253,   254,   255,   256,   257,   258,
     259,     0,   260,   166,     0,   149,   129,   130,     1,   131,
     132,    90,   133,   134,   135,   136,    91,     8,    92,    93,
      10,    94,    95,    96,    97,    15,   276,    98,    18,    99,
      20,    21,   100,   101,   102,   103,    25,     0,    27,    28,
     104,   105,   106,   107,   243,   244,   245,   246,   247,   248,
     139,   249,   277,   278,   250,   251,   252,   140,    42,   141,
      44,   142,    46,   143,    48,    49,    50,    51,    52,    53,
      54,    55,    56,     0,    57,    58,    59,   144,    61,     0,
      62,   145,    64,    65,   108,   109,   110,   111,     0,     0,
       0,   112,   253,   254,   255,   256,   257,   258,   259,     0,
     260,   166,     0,   149,   129,   130,     1,   131,   132,    90,
     133,   134,   135,   136,    91,     8,    92,    93,    10,    94,
      95,    96,    97,    15,    16,    98,    18,    99,    20,    21,
     100,   101,   102,   103,    25,     0,   287,    28,   104,   105,
     106,   107,   243,   244,   245,   246,   247,   248,   139,   249,
      37,    38,   250,   251,   252,   140,    42,   141,    44,   142,
      46,   143,    48,    49,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,   144,    61,     0,    62,   145,
      64,    65,   108,   109,   110,   111,     0,     0,     0,   112,
     253,   254,   255,   256,   257,   258,   259,     0,   260,   166,
       0,   149,   129,   130,     1,   131,   132,    90,   133,   134,
     135,   136,    91,     8,    92,    93,    10,    94,    95,    96,
      97,    15,    16,    98,    18,    99,    20,    21,   100,   101,
     102,   103,    25,     0,    27,    28,   104,   105,   106,   107,
     243,   244,   245,   246,   247,   248,   139,   249,    37,    38,
     250,   251,   252,   140,    42,   141,    44,   142,    46,   143,
      48,    49,    50,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,   144,    61,     0,    62,   145,    64,    65,
     108,   109,   110,   111,     0,     0,     0,   112,   253,   254,
     255,   256,   257,   258,   259,     0,   260,   166,     0,   149,
     129,   130,     1,   131,   132,    90,   133,   134,   135,   136,
      91,     8,    92,    93,    10,    94,    95,    96,    97,    15,
      16,    98,    18,    99,   313,    21,   100,   101,   102,   103,
      25,     0,    27,    28,   104,   105,   106,   107,   243,   244,
     245,   246,   247,   248,   139,   249,    37,    38,   250,   251,
     252,   140,    42,   141,    44,   142,    46,   143,    48,    49,
      50,    51,    52,    53,    54,    55,    56,     0,    57,    58,
      59,   144,    61,     0,    62,   145,    64,    65,   108,   109,
     110,   111,     0,     0,     0,   112,   253,   254,   255,   256,
     257,   258,   259,     0,     0,   166,     0,   149,   129,   130,
       1,   131,   132,    90,   133,   134,   135,   136,    91,     8,
      92,    93,    10,    94,    95,    96,    97,    15,    16,    98,
      18,    99,    20,    21,   100,   101,   102,   103,    25,     0,
      27,    28,   104,   105,   106,   107,     0,     0,     0,     0,
      33,    34,   139,    36,    37,    38,     0,    39,    40,   140,
     423,   141,   424,   142,   425,   143,   426,    49,    50,    51,
      52,    53,    54,    55,   427,   428,   429,   430,    59,   144,
      61,   431,   432,   433,    64,    65,   108,   109,   110,   111,
       0,     0,     0,   112,     0,     0,     0,   146,   147,     0,
       0,     0,     0,   166,     0,   149,   129,   130,     1,   131,
     132,    90,   133,   134,   135,   136,    91,     8,    92,    93,
      10,    94,    95,    96,    97,    15,    16,    98,    18,    99,
      20,    21,   100,   101,   102,   103,    25,     0,    27,    28,
     104,   105,   106,   107,     0,     0,     0,     0,    33,    34,
     139,    36,    37,    38,     0,    39,    40,   140,    42,   141,
      44,   142,    46,   143,    48,    49,    50,    51,    52,    53,
      54,    55,    56,     0,    57,    58,    59,   144,    61,     0,
      62,   145,    64,    65,   108,   109,   110,   111,     0,     0,
       0,   112,     0,     0,     0,   146,   147,     0,     0,     0,
       0,   166,   226,   149,   129,   130,     1,   131,   132,    90,
     133,   134,   135,   136,    91,     8,    92,    93,    10,    94,
      95,    96,    97,    15,    16,    98,    18,    99,    20,    21,
     100,   101,   102,   103,    25,     0,    27,    28,   104,   105,
     106,   107,     0,     0,     0,     0,    33,    34,   139,    36,
      37,    38,     0,    39,    40,   140,    42,   141,    44,   142,
      46,   143,    48,    49,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,   144,    61,     0,    62,   145,
      64,    65,   108,   109,   110,   111,     0,     0,     0,   112,
       0,     0,     0,   146,   147,     0,     0,     0,     0,   166,
     297,   149,   129,   130,     1,   131,   132,    90,   133,   134,
     135,   136,    91,     8,    92,    93,    10,    94,    95,    96,
      97,    15,    16,    98,    18,    99,    20,    21,   100,   101,
     102,   103,    25,     0,    27,    28,   104,   105,   106,   107,
       0,     0,     0,     0,    33,    34,   139,    36,    37,    38,
       0,    39,    40,   140,    42,   141,    44,   142,    46,   143,
      48,    49,    50,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,   144,    61,     0,    62,   145,    64,    65,
     108,   109,   110,   111,     0,     0,     0,   112,     0,     0,
       0,   146,   147,     0,     0,     0,     0,   166,   341,   149,
     129,   130,     1,   131,   132,    90,   133,   134,   135,   136,
      91,     8,    92,    93,    10,    94,    95,    96,    97,    15,
      16,    98,    18,    99,    20,    21,   100,   101,   102,   103,
      25,     0,    27,    28,   104,   105,   106,   107,     0,     0,
       0,     0,    33,    34,   139,    36,    37,    38,     0,    39,
      40,   140,    42,   141,    44,   142,    46,   143,    48,    49,
      50,    51,    52,    53,    54,    55,    56,     0,    57,    58,
      59,   144,    61,     0,    62,   145,    64,    65,   108,   109,
     110,   111,     0,     0,     0,   112,     0,     0,     0,   146,
     147,     0,     0,     0,     0,   166,   440,   149,   129,   130,
       1,   131,   132,    90,   133,   134,   135,   136,    91,     8,
      92,    93,    10,    94,    95,    96,    97,    15,    16,    98,
      18,    99,    20,    21,   100,   101,   102,   103,    25,     0,
      27,    28,   104,   105,   106,   107,     0,     0,     0,     0,
      33,    34,   139,    36,    37,    38,     0,    39,    40,   140,
      42,   141,    44,   142,    46,   143,    48,    49,    50,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,   144,
      61,     0,    62,   145,    64,    65,   108,   109,   110,   111,
       0,     0,     0,   112,     0,     0,     0,   146,   147,     0,
       0,     0,     0,   166,   499,   149,   129,   130,     1,   131,
     132,    90,   133,   134,   135,   136,    91,     8,    92,    93,
     137,    94,    95,    96,    97,    15,    16,    98,   138,    99,
      20,    21,   100,   101,   102,   103,    25,     0,    27,    28,
     104,   105,   106,   107,     0,     0,     0,     0,    33,    34,
     139,    36,    37,    38,     0,    39,    40,   140,    42,   141,
      44,   142,    46,   143,    48,    49,    50,    51,    52,    53,
      54,    55,    56,     0,    57,    58,    59,   144,    61,     0,
      62,   145,    64,    65,   108,   109,   110,   111,     0,     0,
       0,   112,     0,     0,     0,   146,   147,     0,     0,     0,
       0,   148,     0,   149,   129,   130,     1,   131,   132,    90,
     133,   134,   135,   136,    91,     8,    92,    93,    10,    94,
      95,    96,    97,    15,    16,    98,    18,    99,    20,    21,
     100,   101,   102,   103,    25,     0,    27,    28,   104,   105,
     106,   107,     0,     0,     0,     0,    33,    34,   139,    36,
      37,    38,     0,    39,    40,   140,    42,   141,    44,   142,
      46,   143,    48,    49,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,   144,    61,     0,    62,   145,
      64,    65,   108,   109,   110,   111,     0,     0,     0,   112,
       0,     0,     0,   146,   147,     0,     0,     0,     0,   166,
       0,   149,   129,   130,     1,   131,   132,    90,   133,   134,
     135,   136,    91,     8,    92,    93,    10,    94,    95,    96,
      97,    15,    16,    98,    18,    99,    20,    21,   100,   101,
     102,   103,    25,     0,    27,    28,   104,   105,   106,   107,
       0,     0,     0,     0,    33,    34,   139,    36,    37,    38,
       0,    39,    40,   140,    42,   141,    44,   142,    46,   143,
      48,    49,    50,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,   144,    61,     0,    62,   145,    64,    65,
     108,   109,   110,   111,     0,     0,     0,   112,     0,     0,
       0,   146,   147,     0,     0,     0,     0,   148,     0,   149,
     129,   130,     1,   131,   132,    90,   133,   134,   135,   136,
      91,     8,    92,    93,    10,    94,    95,    96,    97,    15,
      16,    98,    18,    99,    20,    21,   100,   101,   102,   103,
      25,     0,    27,    28,   104,   105,   106,   107,     0,     0,
       0,     0,    33,    34,   139,    36,    37,    38,     0,    39,
      40,   140,    42,   141,    44,   142,    46,   143,    48,    49,
      50,    51,    52,    53,    54,    55,    56,     0,    57,    58,
      59,   144,    61,     0,    62,    63,    64,    65,   108,   109,
     110,   111,     0,     0,     0,   112,     0,     0,     0,   146,
     147,     0,     0,     0,     0,   197,     0,   149,   129,   130,
       1,   131,   132,    90,   133,   134,   135,   136,    91,     8,
      92,    93,    10,    94,    95,    96,    97,    15,    16,    98,
      18,    99,    20,    21,   100,   101,   102,   103,    25,     0,
      27,    28,   104,   105,   106,   107,     0,     0,     0,     0,
      33,    34,   139,    36,    37,    38,     0,    39,    40,   140,
      42,   141,    44,   142,    46,   143,    48,    49,    50,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,   144,
      61,     0,    62,    63,    64,    65,   108,   109,   110,   111,
       0,     0,     0,   112,     0,     0,     0,   146,   147,     0,
       0,     0,     0,   166,     0,   149,   129,   130,     1,   131,
     132,    90,   133,   134,   135,   136,    91,     8,    92,    93,
      10,    94,    95,    96,    97,    15,    16,    98,    18,    99,
      20,    21,   100,   101,   102,   103,    25,     0,    27,    28,
     104,   105,   106,   107,     0,     0,     0,     0,    33,    34,
     139,    36,    37,    38,     0,    39,    40,   140,    42,   141,
      44,   142,    46,   143,    48,    49,    50,    51,    52,    53,
      54,    55,   427,     0,   429,    58,    59,   144,    61,     0,
     432,   433,    64,    65,   108,   109,   110,   111,     0,     0,
       0,   112,     0,     0,     0,   146,   147,     0,     0,     0,
       0,   166,     0,   149,   228,   229,     1,   230,   231,    90,
     133,   134,   135,   136,    91,     8,    92,    93,    10,    94,
      95,    96,    97,    15,    16,    98,    18,    99,    20,    21,
     100,   101,   102,   103,    25,     0,    27,    28,   104,   105,
     106,   107,     0,     0,     0,     0,    33,    34,   139,    36,
      37,    38,     0,    39,    40,   140,    42,   141,    44,   142,
      46,   143,    48,    49,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,   144,    61,     0,    62,   145,
      64,    65,   108,   109,   110,   111,     0,     0,     0,   112,
       0,     0,     0,   146,   147,     0,     0,     0,     0,   166,
       0,   149,   129,   130,     1,   131,   132,    90,   133,   134,
     135,   136,    91,     8,    92,    93,    10,    94,    95,    96,
      97,    15,    16,    98,    18,    99,    20,    21,   100,   101,
     102,   103,    25,     0,    27,    28,   104,   105,   106,   107,
       0,     0,     0,     0,    33,    34,   139,    36,    37,    38,
       0,    39,    40,   140,    42,   141,    44,   142,    46,   143,
      48,    49,    50,    51,    52,    53,    54,    55,   485,     0,
     486,    58,    59,   144,    61,     0,   487,   488,    64,    65,
     108,   109,   110,   111,     0,     0,     0,   112,     0,     0,
       0,   146,   147,     0,     0,     0,     0,   166,     0,   149,
     129,   130,     1,   131,   132,    90,   133,   134,   135,   136,
      91,     8,    92,    93,    10,    94,    95,    96,    97,    15,
      16,    98,    18,    99,    20,    21,   100,   101,   102,   103,
      25,     0,    27,    28,   104,   105,   106,   107,     0,     0,
       0,     0,     0,     0,   139,    36,    37,    38,     0,    39,
      40,   140,    42,   141,    44,   142,    46,   143,    48,    49,
      50,    51,    52,    53,    54,    55,    56,     0,    57,    58,
      59,   144,    61,     0,    62,   145,    64,    65,   108,   109,
     110,   111,     0,     0,     0,   112,     0,     0,     0,   146,
     147,     0,     0,     0,     0,   166,     0,   149,   129,   130,
       1,   131,   132,    90,   133,   134,   135,   136,    91,     8,
      92,    93,    10,    94,    95,    96,    97,    15,    16,    98,
      18,    99,    20,    21,   100,   101,   102,   103,    25,     0,
      27,    28,   104,   105,   106,   107,     0,     0,     0,     0,
       0,     0,   139,    36,    37,    38,     0,     0,     0,   140,
      42,   141,    44,   142,    46,   143,    48,    49,    50,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,   144,
      61,     0,    62,   145,    64,    65,   108,   109,   110,   111,
       0,     0,     0,   112,     0,     0,     0,   146,   147,     0,
       0,     0,     0,   166,     0,   149,   129,   130,     1,   131,
     132,    90,   133,   134,   135,   136,    91,     8,    92,    93,
      10,    94,    95,    96,    97,    15,    16,    98,    18,    99,
      20,    21,   100,   101,   102,   103,    25,     0,    27,    28,
     104,   105,   106,   107,     0,     0,     0,     0,     0,     0,
     139,     0,    37,    38,     0,     0,     0,   140,    42,   141,
      44,   142,    46,   143,    48,    49,    50,    51,    52,    53,
      54,    55,    56,     0,    57,    58,    59,   144,    61,     0,
      62,   145,    64,    65,   108,   109,   110,   111,     0,     0,
       0,   112,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   166,     1,   149,     0,     2,     3,     4,     5,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,     0,
      25,    26,    27,    28,    29,    30,    31,    32,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,     0,    57,    58,
      59,    60,    61,     0,    62,    63,    64,    65,    66,    67,
      68,     0,    69,     1,     0,    70,     0,     3,     4,     5,
       6,     7,     8,    71,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,    28,     0,    30,    31,    32,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,     0,    62,    63,    64,    65,    66,
      67,     0,     0,    69,     1,     0,    70,     0,     3,     4,
       5,     6,     7,     8,   381,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,    28,     0,    30,    31,    32,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,    60,    61,     0,    62,    63,    64,    65,
      66,    67,     0,     0,    69,     1,     0,    70,     0,     3,
       4,     5,     6,     7,     8,   404,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,     0,    30,    31,
      32,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,    60,    61,     0,    62,    63,    64,
      65,    66,    67,     0,     0,    69,     1,     0,    70,     0,
       3,     4,     5,     6,     7,     8,   523,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    18,    19,    20,    21,
      22,    23,    24,     0,    25,    26,    27,    28,     0,    30,
      31,    32,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,    60,    61,     0,    62,    63,
      64,    65,    66,    67,     0,     0,    69,     1,     0,    70,
       0,     3,     4,     5,     6,     7,     8,   552,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,     0,
      30,    31,    32,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,    60,    61,     0,    62,
      63,    64,    65,    66,    67,     0,     0,     0,     0,     0,
      70,     0,     0,     0,     0,     0,     1,     0,   472,    90,
       3,     4,     5,     6,    91,     8,    92,    93,    10,    94,
      95,    96,    97,    15,    16,    98,    18,    99,    20,    21,
     100,   101,   102,   103,    25,     0,    27,    28,   104,   105,
     106,   107,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,    60,    61,     0,    62,    63,
      64,    65,   108,   109,   110,   111,     0,     1,     0,   112,
      90,     3,     4,     5,     6,    91,     8,    92,    93,    10,
      94,    95,    96,    97,    15,    16,    98,    18,    99,    20,
      21,   100,   101,   102,   103,    25,     0,    27,    28,   104,
     105,   106,   107,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,   174,    61,     0,    62,
      63,    64,    65,   108,   109,   110,   111,     0,     1,     0,
     112,    90,     3,     4,     5,     6,    91,     8,    92,    93,
      10,    94,    95,    96,    97,    15,    16,    98,    18,    99,
      20,    21,   100,   101,   102,   103,    25,     0,    27,    28,
     104,   105,   106,   107,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,   215,   216,   217,   218,   219,
      54,   220,    56,     0,    57,    58,   221,    60,    61,     0,
      62,    63,    64,    65,   108,   109,   110,   111,     0,     1,
       0,   112,    90,   133,   134,     5,     6,    91,     8,    92,
      93,    10,    94,    95,    96,    97,    15,    16,    98,    18,
      99,    20,    21,   100,   101,   102,   103,    25,     0,    27,
      28,   104,   105,   106,   107,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,   140,    42,
     141,    44,   142,    46,   143,    48,    49,    50,    51,    52,
      53,    54,    55,    56,     0,    57,    58,    59,   384,    61,
       0,    62,    63,    64,    65,   108,   109,   110,   111,     1,
       0,     0,   112,     3,     4,     5,     6,     7,     8,     0,
       9,    10,    11,    12,    13,     0,    15,    16,     0,    18,
      19,    20,    21,    22,   101,    24,     0,    25,     0,    27,
      28,     0,    30,    31,    32,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,     0,    57,    58,    59,    60,    61,
     501,    62,    63,    64,    65,     0,    67,     0,     0,     0,
       0,     0,    70,   502,   243,   244,   245,   246,   268,   269,
       0,   270,     0,     0,   250,   271,   272,   243,   244,   245,
     246,   268,   269,   503,   270,     0,     0,   250,   271,   272,
       0,     0,     0,     0,     0,     0,     0,   243,   244,   245,
     246,   268,   269,     0,   270,     0,     0,   250,   271,   272,
       0,     0,   253,   254,   255,   256,   257,   258,   259,     0,
       0,     0,     0,     0,     0,   253,   254,   255,   256,   257,
     258,   259,     0,     0,     0,     0,     0,     0,   504,     0,
       0,     0,     0,     0,     0,   253,   254,   255,   256,   257,
     258,   259,   243,   244,   245,   246,   268,   269,   576,   270,
       0,     0,   250,   271,   272,     0,     0,     0,     0,     0,
       0,     0,   560,   561,     0,   562,   563,   564,   267,   565,
       0,     0,   566,   567,   568,     0,     0,   243,   244,   245,
     246,   268,   269,     0,   270,     0,     0,   250,   271,   272,
     253,   254,   255,   256,   257,   258,   259,   243,   244,   245,
     246,   268,   269,     0,   270,     0,     0,   250,   271,   272,
     569,   570,   571,   572,   573,   574,   575,     0,     0,     0,
       0,     0,     0,     0,     0,   253,   254,   255,   256,   257,
     258,   259,   243,   244,   245,   246,   268,   269,     0,   270,
       0,     0,   250,   271,   272,   253,   254,   255,   256,   257,
     258,   259,     0,   329,     0,   330,   243,   244,   245,   246,
     268,   269,     0,   270,     0,     0,   250,   271,   272,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     253,   254,   255,   256,   257,   258,   259,     0,   410,     0,
     330,   362,   363,   364,   365,   366,   367,     0,   368,     0,
       0,   369,   370,   371,   253,   254,   255,   256,   257,   258,
     259,     0,     0,     0,   330,   243,   244,   245,   246,   268,
     269,     0,   270,     0,     0,   250,   271,   272,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   372,
     373,   374,   375,   376,   377,   378,     0,   379,   243,   244,
     245,   246,   268,   269,     0,   270,     0,     0,   250,   271,
     272,     0,     0,   253,   254,   255,   256,   257,   258,   259,
     392,   243,   244,   245,   246,   268,   269,     0,   270,     0,
       0,   250,   271,   272,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   253,   254,   255,   256,
     257,   258,   259,   491,   243,   244,   245,   246,   268,   269,
       0,   270,     0,     0,   250,   271,   272,     0,     0,   253,
     254,   255,   256,   257,   258,   259,   495,   243,   244,   245,
     246,   268,   269,     0,   270,     0,     0,   250,   271,   272,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   253,   254,   255,   256,   257,   258,   259,   525,
     243,   244,   245,   246,   268,   269,     0,   270,     0,     0,
     250,   271,   272,     0,     0,   253,   254,   255,   256,   257,
     258,   259,   585,   243,   244,   245,   246,   268,   269,     0,
     270,     0,     0,   250,   271,   272,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   253,   254,
     255,   256,   257,   258,   259,   608,   243,   244,   245,   246,
       0,   327,   328,   270,     0,     0,   250,   271,   272,     0,
       0,   253,   254,   255,   256,   257,   258,   259,   243,   244,
     245,   246,   268,   269,     0,   270,     0,     0,   250,   271,
     272,   362,   363,   364,   365,   366,   367,     0,   368,     0,
       0,   369,   370,   371,   253,   254,   255,   256,   257,   258,
     259,   560,   561,     0,   562,     0,     0,     0,   565,     0,
       0,   566,   567,   568,     0,     0,   253,   254,   255,   256,
     257,   258,   259,     0,     0,     0,     0,     0,     0,   372,
     373,   374,   375,   376,   377,   378,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   569,
     570,   571,   572,   573,   574,   575
};

static const yytype_int16 yycheck[] =
{
       0,     0,    24,     7,    70,    32,    12,   381,    14,    13,
     383,     2,   395,    16,   239,     9,    22,   555,    31,   392,
      11,   263,   523,    68,    30,    31,    17,    97,    23,    97,
      99,    76,    34,    24,   103,   260,   409,   117,    29,    22,
      67,    32,    44,   581,   163,    10,    11,    30,    31,    44,
      68,   552,   171,    68,   134,   135,   136,    68,    76,    49,
      66,    76,    55,   182,    57,    76,    59,    31,    61,    97,
      70,    68,    78,    73,    73,   194,    67,    68,    49,    76,
      23,    52,    98,    86,    97,    28,   102,    36,    37,   163,
      55,    85,    57,   212,    59,    78,    61,   171,     0,    49,
     342,   475,    69,    97,   329,    95,    96,    99,   182,    97,
      75,    97,   118,   119,   120,   121,    97,    98,    49,   361,
     194,    92,    93,    94,    95,    96,    49,    97,    49,    52,
      98,   137,   138,    28,   140,   141,   142,   143,   212,   145,
      97,   168,   148,    93,    94,    95,    96,   277,   278,   523,
      97,    98,   525,   144,   379,   538,    97,    98,   149,    97,
     166,   188,    93,    94,    95,    96,    28,   173,    97,    92,
      93,    94,    95,    96,    95,    96,   103,   168,   552,    97,
      97,   172,    97,   174,    97,   410,   423,   424,   425,   426,
      97,   197,    97,   430,    97,   103,    97,   188,   102,   441,
      98,    26,    28,   103,   577,   196,   103,    97,   102,    97,
      26,    98,   585,    16,    97,    97,    97,    88,   209,    87,
     100,    97,     9,    28,    70,   291,   100,    97,    34,   471,
      97,   486,   487,   488,    97,   608,   100,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   276,   277,   278,    70,   500,    86,
      70,    97,   268,   269,   270,   271,   272,   267,   267,    87,
      87,    73,   276,   277,   278,    87,    97,    97,   284,   400,
      31,   287,   407,   475,   326,   276,   277,   278,   337,   513,
     212,   291,   292,    -1,   300,    -1,   302,    -1,   304,    -1,
     306,    -1,    -1,    -1,    -1,   560,   561,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   116,   117,    -1,    -1,   331,   332,   333,   334,    -1,
     336,   337,    -1,    -1,   340,    -1,    -1,   343,   133,   134,
     135,   136,    -1,    -1,   139,    -1,   337,   602,    -1,    -1,
      -1,   146,   147,    -1,    -1,    -1,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   381,    -1,   383,   390,   391,    -1,    -1,    -1,    -1,
      -1,    -1,   392,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   395,   396,    -1,   407,    -1,   409,
      -1,   485,   486,   487,   488,    -1,   422,    28,    -1,    -1,
      -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,   450,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   493,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   475,    -1,   477,   477,    90,
      91,    92,    93,    94,    95,    96,   560,   561,   562,   563,
     564,   565,   566,   567,   568,   569,   570,   571,   572,   573,
     574,   575,   493,    -1,   299,    -1,   301,    -1,   303,    -1,
     305,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   519,
     519,   527,    -1,   523,    -1,   525,    15,    -1,   602,    -1,
     325,   326,   327,   328,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    34,    -1,   538,    -1,    -1,
      -1,   557,   552,    42,    43,    44,    45,    46,    47,    -1,
      49,    34,    -1,    52,    53,    54,    -1,    -1,    -1,    42,
      43,    44,    45,    46,    47,    -1,    49,   577,    -1,    52,
      53,    54,    -1,    -1,    -1,   585,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   393,    -1,
      -1,    90,    91,    92,    93,    94,    95,    96,   608,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,
      93,    94,    95,    96,    97,    -1,   421,    -1,    -1,    -1,
      -1,    -1,   427,    28,   429,    -1,    -1,   432,   433,    34,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      45,    46,    47,    -1,    49,    28,    -1,    52,    53,    54,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     485,   486,   487,   488,    -1,    90,    91,    92,    93,    94,
      95,    96,    -1,    -1,    -1,    -1,   501,   502,   503,   504,
      -1,    -1,   507,   508,   509,   510,    -1,    90,    91,    92,
      93,    94,    95,    96,    28,    -1,    -1,    -1,    -1,    -1,
      34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    -1,    42,    43,   549,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,   560,   561,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,    -1,    -1,    -1,    -1,   580,    90,    91,    92,    93,
      94,    95,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    91,    92,    93,    94,    95,    96,   602,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    78,    79,    80,    81,    82,    83,    84,    85,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    93,    94,    -1,
      -1,    -1,    -1,    99,    -1,   101,    -1,   103,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    78,    79,    80,    81,    82,    83,    84,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      96,    -1,    98,    99,    -1,   101,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      78,    79,    80,    81,    82,    83,    84,    85,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    96,    -1,
      98,    99,    -1,   101,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    78,    79,
      80,    81,    82,    83,    84,    85,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    96,    -1,    98,    99,
      -1,   101,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    78,    79,    80,    81,
      82,    83,    84,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    -1,    98,    99,    -1,   101,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    78,    79,    80,    81,    82,    83,
      84,    85,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    96,    -1,    -1,    99,    -1,   101,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    93,    94,    -1,
      -1,    -1,    -1,    99,    -1,   101,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      78,    79,    80,    81,    82,    83,    84,    85,    -1,    -1,
      -1,    89,    -1,    -1,    -1,    93,    94,    -1,    -1,    -1,
      -1,    99,   100,   101,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    78,    79,
      80,    81,    82,    83,    84,    85,    -1,    -1,    -1,    89,
      -1,    -1,    -1,    93,    94,    -1,    -1,    -1,    -1,    99,
     100,   101,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    78,    79,    80,    81,
      82,    83,    84,    85,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    93,    94,    -1,    -1,    -1,    -1,    99,   100,   101,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    78,    79,    80,    81,    82,    83,
      84,    85,    -1,    -1,    -1,    89,    -1,    -1,    -1,    93,
      94,    -1,    -1,    -1,    -1,    99,   100,   101,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    78,    79,    80,    81,    82,    83,    84,    85,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    93,    94,    -1,
      -1,    -1,    -1,    99,   100,   101,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      78,    79,    80,    81,    82,    83,    84,    85,    -1,    -1,
      -1,    89,    -1,    -1,    -1,    93,    94,    -1,    -1,    -1,
      -1,    99,    -1,   101,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    78,    79,
      80,    81,    82,    83,    84,    85,    -1,    -1,    -1,    89,
      -1,    -1,    -1,    93,    94,    -1,    -1,    -1,    -1,    99,
      -1,   101,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    78,    79,    80,    81,
      82,    83,    84,    85,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    93,    94,    -1,    -1,    -1,    -1,    99,    -1,   101,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    78,    79,    80,    81,    82,    83,
      84,    85,    -1,    -1,    -1,    89,    -1,    -1,    -1,    93,
      94,    -1,    -1,    -1,    -1,    99,    -1,   101,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    78,    79,    80,    81,    82,    83,    84,    85,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    93,    94,    -1,
      -1,    -1,    -1,    99,    -1,   101,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      78,    79,    80,    81,    82,    83,    84,    85,    -1,    -1,
      -1,    89,    -1,    -1,    -1,    93,    94,    -1,    -1,    -1,
      -1,    99,    -1,   101,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    78,    79,
      80,    81,    82,    83,    84,    85,    -1,    -1,    -1,    89,
      -1,    -1,    -1,    93,    94,    -1,    -1,    -1,    -1,    99,
      -1,   101,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    78,    79,    80,    81,
      82,    83,    84,    85,    -1,    -1,    -1,    89,    -1,    -1,
      -1,    93,    94,    -1,    -1,    -1,    -1,    99,    -1,   101,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    78,    79,    80,    81,    82,    83,
      84,    85,    -1,    -1,    -1,    89,    -1,    -1,    -1,    93,
      94,    -1,    -1,    -1,    -1,    99,    -1,   101,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    51,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    78,    79,    80,    81,    82,    83,    84,    85,
      -1,    -1,    -1,    89,    -1,    -1,    -1,    93,    94,    -1,
      -1,    -1,    -1,    99,    -1,   101,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      78,    79,    80,    81,    82,    83,    84,    85,    -1,    -1,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,     6,   101,    -1,     9,    10,    11,    12,    13,
      14,    15,    -1,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,     6,    -1,    89,    -1,    10,    11,    12,
      13,    14,    15,    97,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    36,    37,    -1,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    47,    48,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    -1,    78,    79,    80,    81,    82,
      83,    -1,    -1,    86,     6,    -1,    89,    -1,    10,    11,
      12,    13,    14,    15,    97,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,    35,    36,    37,    -1,    39,    40,    41,
      -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    78,    79,    80,    81,
      82,    83,    -1,    -1,    86,     6,    -1,    89,    -1,    10,
      11,    12,    13,    14,    15,    97,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    36,    37,    -1,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    78,    79,    80,
      81,    82,    83,    -1,    -1,    86,     6,    -1,    89,    -1,
      10,    11,    12,    13,    14,    15,    97,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    78,    79,
      80,    81,    82,    83,    -1,    -1,    86,     6,    -1,    89,
      -1,    10,    11,    12,    13,    14,    15,    97,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    36,    37,    -1,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    78,
      79,    80,    81,    82,    83,    -1,    -1,    -1,    -1,    -1,
      89,    -1,    -1,    -1,    -1,    -1,     6,    -1,    97,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    78,    79,
      80,    81,    82,    83,    84,    85,    -1,     6,    -1,    89,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    78,
      79,    80,    81,    82,    83,    84,    85,    -1,     6,    -1,
      89,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      78,    79,    80,    81,    82,    83,    84,    85,    -1,     6,
      -1,    89,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    78,    79,    80,    81,    82,    83,    84,    85,     6,
      -1,    -1,    89,    10,    11,    12,    13,    14,    15,    -1,
      17,    18,    19,    20,    21,    -1,    23,    24,    -1,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    -1,    36,
      37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      28,    78,    79,    80,    81,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    89,    28,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    42,    43,    44,
      45,    46,    47,    28,    49,    -1,    -1,    52,    53,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    42,    43,    44,    45,    46,    47,    28,    49,
      -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    45,    46,    47,    33,    49,
      -1,    -1,    52,    53,    54,    -1,    -1,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      90,    91,    92,    93,    94,    95,    96,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      90,    91,    92,    93,    94,    95,    96,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    90,    91,    92,    93,    94,
      95,    96,    -1,    98,    -1,   100,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    91,    92,    93,    94,    95,    96,    -1,    98,    -1,
     100,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    90,    91,    92,    93,    94,    95,
      96,    -1,    -1,    -1,   100,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    95,    96,    -1,    98,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,    93,
      94,    95,    96,    97,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,    90,
      91,    92,    93,    94,    95,    96,    97,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    53,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    97,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    42,    43,    44,    45,
      -1,    80,    81,    49,    -1,    -1,    52,    53,    54,    -1,
      -1,    90,    91,    92,    93,    94,    95,    96,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    90,    91,    92,    93,    94,    95,
      96,    42,    43,    -1,    45,    -1,    -1,    -1,    49,    -1,
      -1,    52,    53,    54,    -1,    -1,    90,    91,    92,    93,
      94,    95,    96,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    95,    96,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    95,    96
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
      75,    76,    78,    79,    80,    81,    82,    83,    84,    86,
      89,    97,   105,   106,   107,   108,   109,   114,   116,   120,
     121,   122,   123,   127,   129,   130,   131,   133,   134,   135,
       9,    14,    16,    17,    19,    20,    21,    22,    25,    27,
      30,    31,    32,    33,    38,    39,    40,    41,    82,    83,
      84,    85,    89,   115,   116,   117,    10,    11,    55,    57,
      59,    61,    75,   146,    31,    97,   117,   118,   119,     4,
       5,     7,     8,    10,    11,    12,    13,    18,    26,    48,
      55,    57,    59,    61,    75,    79,    93,    94,    99,   101,
     117,   124,   126,   142,   143,   146,   148,   150,   153,   154,
     158,   161,   162,   163,   166,   146,    99,   163,   117,    31,
     124,   163,    36,    37,    75,   117,   143,   145,   156,    97,
     117,   124,   163,   124,   126,   118,   163,   118,   117,    97,
     122,     0,   107,   124,   163,    97,    98,    99,   149,   162,
     149,   163,   163,   163,   163,    69,    97,    97,    97,    98,
     149,   149,   163,   163,   162,    63,    64,    65,    66,    67,
      69,    74,   117,   163,   162,   162,   100,   163,     4,     5,
       7,     8,   103,   117,   159,   160,   163,   167,   168,    99,
      97,    97,    28,    42,    43,    44,    45,    46,    47,    49,
      52,    53,    54,    90,    91,    92,    93,    94,    95,    96,
      98,   125,   142,   164,   166,    97,   163,    33,    46,    47,
      49,    53,    54,   118,    97,    97,    24,    50,    51,   117,
     163,   117,    34,    44,   128,    97,    97,    36,    97,    97,
      97,    34,    97,    97,   118,    97,   117,   100,   163,    28,
      34,    28,    34,    28,    34,    28,    34,    55,    57,    59,
      61,   147,   117,    28,   164,    68,    76,    68,    76,    68,
      76,    68,    76,    68,    76,    28,    28,    80,    81,    98,
     100,   103,   103,   103,   103,   102,   103,    98,   102,   102,
      98,   100,   168,    26,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     163,   168,    42,    43,    44,    45,    46,    47,    49,    52,
      53,    54,    90,    91,    92,    93,    94,    95,    96,    98,
     125,    97,   120,    97,    75,   143,   144,   146,   144,   144,
      23,    44,    97,    28,   163,     9,    85,    97,   110,   111,
     112,   113,   163,   122,    97,   120,   136,   137,   138,    97,
      98,   162,   163,   162,   163,   162,   163,   162,   163,    23,
      28,   155,    26,    56,    58,    60,    62,    70,    71,    72,
      73,    77,    78,    79,   151,   152,   162,   151,   162,   162,
     100,   168,   163,   163,   163,   163,   163,   160,   163,   163,
      98,   125,   163,   125,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     163,   168,    97,   121,   139,   140,   141,    16,   136,    97,
      97,    97,   163,   163,   136,    70,    72,    78,    79,   157,
     162,    97,   115,   117,   113,    97,    88,   138,   136,   100,
     168,    28,    28,    28,    28,   162,   163,   155,   155,   155,
     155,   162,   162,   155,   162,   162,   125,   100,   125,    16,
      86,   132,   141,    97,   120,    97,    15,    34,    87,   162,
     165,   166,   165,   165,   165,    97,   118,    97,     9,   125,
     162,   162,   162,   162,   162,   162,   162,   162,    28,    70,
     152,   100,    97,   120,    97,   139,   136,    34,   163,    97,
      42,    43,    45,    46,    47,    49,    52,    53,    54,    90,
      91,    92,    93,    94,    95,    96,    28,    97,   115,   100,
      70,   139,   132,    87,   163,    97,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,    70,   136,    97,   162,   132,    97,    97,   136,
     165,   136,    87,    87,    97,    97
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   104,   105,   106,   106,   107,   107,   107,   107,   107,
     107,   108,   109,   110,   111,   111,   112,   112,   113,   113,
     113,   114,   114,   115,   115,   115,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   118,   118,
     118,   119,   119,   120,   120,   121,   121,   121,   121,   121,
     121,   122,   122,   122,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   124,   124,
     124,   124,   124,   124,   124,   124,   125,   125,   126,   126,
     126,   126,   126,   127,   127,   127,   127,   128,   128,   129,
     129,   129,   130,   130,   131,   131,   131,   131,   132,   132,
     133,   133,   133,   133,   134,   134,   135,   136,   136,   137,
     137,   138,   138,   139,   139,   140,   140,   141,   141,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   143,   144,   144,   145,   145,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   147,   147,
     147,   147,   148,   148,   149,   149,   149,   149,   150,   150,
     150,   150,   150,   151,   151,   151,   151,   151,   152,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   154,
     154,   154,   154,   154,   154,   154,   155,   155,   156,   156,
     157,   157,   157,   157,   157,   158,   158,   158,   159,   159,
     159,   160,   160,   160,   160,   160,   161,   161,   162,   162,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   166,   166,   167,   167,
     168,   168
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
       4,     3,     5,     2,     4,     6,     0,     1,     2,     2,
       5,     4,     3,     5,     5,     5,     5,     1,     1,     3,
       3,     3,     4,     6,     6,     8,     7,     9,     0,     2,
       7,    11,    12,     9,     4,     6,     2,     0,     1,     1,
       2,     1,     1,     0,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     2,     3,     5,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     4,
       6,     4,     6,     4,     6,     4,     6,     5,     1,     1,
       1,     1,     2,     2,     1,     2,     4,     6,     2,     4,
       4,     1,     1,     1,     1,     5,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     6,
       6,     6,     6,     6,     4,     4,     1,     1,     2,     4,
       1,     2,     5,     2,     2,     3,     3,     3,     1,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     4,     4,     0,     1,
       1,     3
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
#line 2304 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2310 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2316 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2322 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2328 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 194 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2334 "engines/director/lingo/lingo-gr.cpp"
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
#line 2604 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 202 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2615 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 208 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 215 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* scriptpart: tENDCLAUSE endargdef '\n'  */
#line 220 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 249 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2643 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* factory: tFACTORY ID '\n' methodlist  */
#line 253 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2649 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 255 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2655 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* methodlist: %empty  */
#line 257 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2661 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlistline  */
#line 261 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2672 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 267 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2682 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18: /* methodlistline: '\n'  */
#line 274 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2688 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* methodlistline: tENDCLAUSE endargdef '\n'  */
#line 276 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2694 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 281 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2703 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* handler: tON ID idlist '\n' stmtlist  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2710 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* endargdef: ID  */
#line 290 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2716 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* endargdef: endargdef ',' ID  */
#line 291 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2722 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* CMDID: tABBREVIATED  */
#line 299 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* CMDID: tABBREV  */
#line 300 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2734 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* CMDID: tABBR  */
#line 301 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2740 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* CMDID: tAFTER  */
#line 302 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2746 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* CMDID: tAND  */
#line 303 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("and"); }
#line 2752 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tBEFORE  */
#line 304 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2758 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tCAST  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2764 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tCHAR  */
#line 306 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2770 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tCHARS  */
#line 307 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2776 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tCONTAINS  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("contains"); }
#line 2782 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tDATE  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2788 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tDELETE  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 2794 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tDOWN  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2800 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tFIELD  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2806 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tFRAME  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2812 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tHILITE  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 2818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tIN  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2824 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tINTERSECTS  */
#line 316 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2830 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tINTO  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2836 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tITEM  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2842 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tITEMS  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2848 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tLAST  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2854 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tLINE  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2860 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tLINES  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2866 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tLONG  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2872 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tMENU  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tMENUITEM  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2884 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tMENUITEMS  */
#line 326 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2890 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tMOD  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("mod"); }
#line 2896 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tMOVIE  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2902 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tNEXT  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2908 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tNOT  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("not"); }
#line 2914 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tNUMBER  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2920 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tOF  */
#line 332 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2926 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tOR  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("or"); }
#line 2932 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tPREVIOUS  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2938 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tREPEAT  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2944 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tSCRIPT  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2950 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tASSERTERROR  */
#line 337 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2956 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tSHORT  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2962 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tSOUND  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2968 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tSPRITE  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2974 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tSTARTS  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("starts"); }
#line 2980 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tTHE  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 2986 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tTIME  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 2992 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tTO  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2998 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* CMDID: tWHILE  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 3004 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* CMDID: tWINDOW  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 3010 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* CMDID: tWITH  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 3016 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* CMDID: tWITHIN  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 3022 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* CMDID: tWORD  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 3028 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* CMDID: tWORDS  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 3034 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* ID: tELSE  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 3040 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tENDCLAUSE  */
#line 355 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); delete (yyvsp[0].s); }
#line 3046 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tEXIT  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 3052 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tFACTORY  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 3058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tGLOBAL  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 3064 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tGO  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 3070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tIF  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 3076 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tINSTANCE  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 3082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tMACRO  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3088 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tMETHOD  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tON  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tOPEN  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3106 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tPLAY  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3112 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ID: tPROPERTY  */
#line 367 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3118 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* ID: tPUT  */
#line 368 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* ID: tSET  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3130 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* ID: tTELL  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3136 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* ID: tTHEN  */
#line 371 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* idlist: %empty  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* nonemptyidlist: ID  */
#line 379 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3157 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 383 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3165 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* stmt: tENDIF '\n'  */
#line 394 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3171 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* proc: CMDID cmdargs '\n'  */
#line 410 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3177 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* proc: tPUT cmdargs '\n'  */
#line 411 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: tGO cmdargs '\n'  */
#line 412 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3189 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: tGO frameargs '\n'  */
#line 413 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3195 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tPLAY cmdargs '\n'  */
#line 414 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3201 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tPLAY frameargs '\n'  */
#line 415 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3207 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tOPEN cmdargs '\n'  */
#line 416 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tOPEN expr tWITH expr '\n'  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3223 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* proc: tNEXT tREPEAT '\n'  */
#line 422 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3229 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proc: tEXIT tREPEAT '\n'  */
#line 423 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3235 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tEXIT '\n'  */
#line 424 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3241 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proc: tDELETE chunk '\n'  */
#line 425 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* proc: tHILITE chunk '\n'  */
#line 426 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3253 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* proc: tASSERTERROR stmtoneliner  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* cmdargs: %empty  */
#line 430 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3267 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* cmdargs: expr trailingcomma  */
#line 433 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 438 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd args, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3286 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 442 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3297 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 448 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* cmdargs: '(' ')'  */
#line 453 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3315 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* cmdargs: '(' expr ',' ')'  */
#line 456 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(args,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3325 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 461 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(args, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* frameargs: tFRAME expr  */
#line 473 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3344 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* frameargs: tMOVIE expr  */
#line 478 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3355 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 484 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* frameargs: expr tOF tMOVIE expr  */
#line 490 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3377 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* frameargs: tFRAME expr expr_nounarymath  */
#line 496 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 504 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3394 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 505 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3400 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 506 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3406 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* asgn: tSET varorthe to expr '\n'  */
#line 507 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3412 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* definevars: tGLOBAL idlist '\n'  */
#line 512 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* definevars: tPROPERTY idlist '\n'  */
#line 513 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3424 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* definevars: tINSTANCE idlist '\n'  */
#line 514 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3430 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* ifstmt: tIF expr tTHEN stmt  */
#line 517 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3439 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 521 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 3446 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 525 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3457 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 531 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 3466 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 535 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3475 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 3482 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* endif: %empty  */
#line 543 "engines/director/lingo/lingo-gr.y"
                        { warning("LingoCompiler::parse: no end if"); }
#line 3488 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 546 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 548 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3502 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 550 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3509 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 552 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* tell: tTELL expr tTO stmtoneliner  */
#line 556 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 560 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3532 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* when: tWHEN '\n'  */
#line 564 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WhenNode((yyvsp[-1].w).eventName, (yyvsp[-1].w).stmt); }
#line 3538 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* stmtlist: %empty  */
#line 566 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3544 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* nonemptystmtlist: stmtlistline  */
#line 571 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3555 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 577 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3565 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* stmtlistline: '\n'  */
#line 584 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3571 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* stmtlist_insideif: %empty  */
#line 588 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 3577 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 593 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 599 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3598 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* stmtlistline_insideif: '\n'  */
#line 606 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3604 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* simpleexpr_nounarymath: tINT  */
#line 613 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3610 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* simpleexpr_nounarymath: tFLOAT  */
#line 614 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3616 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* simpleexpr_nounarymath: tSYMBOL  */
#line 615 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3622 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* simpleexpr_nounarymath: tSTRING  */
#line 616 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3628 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 617 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3634 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 618 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 3640 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 619 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 3646 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 620 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3652 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* var: ID  */
#line 628 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3658 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* chunk: tFIELD refargs  */
#line 638 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 3664 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* chunk: tCAST refargs  */
#line 639 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 3670 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* chunk: tCHAR expr tOF simpleexpr  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3677 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3684 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* chunk: tWORD expr tOF simpleexpr  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3698 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* chunk: tITEM expr tOF simpleexpr  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3712 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* chunk: tLINE expr tOF simpleexpr  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3719 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 654 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 656 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 3732 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* chunktype: tCHAR  */
#line 659 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 3738 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* chunktype: tWORD  */
#line 660 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 3744 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* chunktype: tITEM  */
#line 661 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 3750 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* chunktype: tLINE  */
#line 662 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 3756 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* object: tSCRIPT refargs  */
#line 665 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 3762 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* object: tWINDOW refargs  */
#line 666 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 3768 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* refargs: simpleexpr  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3778 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* refargs: '(' ')'  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 3786 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* refargs: '(' expr ',' ')'  */
#line 677 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(args,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3796 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 682 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(args, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3805 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* the: tTHE ID  */
#line 688 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3811 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* the: tTHE ID tOF theobj  */
#line 689 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3817 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* the: tTHE tNUMBER tOF theobj  */
#line 690 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 3823 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 225: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 697 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3829 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 226: /* theobj: tSOUND simpleexpr  */
#line 698 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3835 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* theobj: tSPRITE simpleexpr  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3841 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* menu: tMENU simpleexpr  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3847 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 229: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 704 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3853 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 230: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 705 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3859 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* thedatetime: tTHE tABBREV tDATE  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3865 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* thedatetime: tTHE tABBREV tTIME  */
#line 707 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3871 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* thedatetime: tTHE tABBR tDATE  */
#line 708 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3877 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* thedatetime: tTHE tABBR tTIME  */
#line 709 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3883 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* thedatetime: tTHE tLONG tDATE  */
#line 710 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 3889 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* thedatetime: tTHE tLONG tTIME  */
#line 711 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 3895 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* thedatetime: tTHE tSHORT tDATE  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 3901 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* thedatetime: tTHE tSHORT tTIME  */
#line 713 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 3907 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 717 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 3913 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 718 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 3919 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 719 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 3925 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 720 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 3931 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 721 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 3937 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* thenumberof: tTHE tNUMBER tOF tMENUS  */
#line 722 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenus, nullptr); }
#line 3943 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* thenumberof: tTHE tNUMBER tOF tXTRAS  */
#line 723 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfXtras, nullptr); }
#line 3949 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* writablethe: tTHE ID  */
#line 728 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3955 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* writablethe: tTHE ID tOF writabletheobj  */
#line 729 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3961 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* writabletheobj: tMENU expr_noeq  */
#line 733 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3967 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3973 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* writabletheobj: tSOUND expr_noeq  */
#line 735 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3979 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* writabletheobj: tSPRITE expr_noeq  */
#line 736 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3985 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* list: '[' exprlist ']'  */
#line 739 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 3991 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* list: '[' ':' ']'  */
#line 740 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 3997 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* list: '[' proplist ']'  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 4003 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* proplist: proppair  */
#line 747 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4012 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* proplist: proplist ',' proppair  */
#line 751 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4020 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* proplist: proplist ',' expr  */
#line 754 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4028 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 261: /* proppair: tSYMBOL ':' expr  */
#line 759 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4034 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 262: /* proppair: ID ':' expr  */
#line 760 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4040 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 263: /* proppair: tSTRING ':' expr  */
#line 761 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4046 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 264: /* proppair: tINT ':' expr  */
#line 762 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new IntNode((yyvsp[-2].i)), (yyvsp[0].node)); }
#line 4052 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 265: /* proppair: tFLOAT ':' expr  */
#line 763 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new FloatNode((yyvsp[-2].f)), (yyvsp[0].node)); }
#line 4058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* unarymath: '+' simpleexpr  */
#line 766 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 4064 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* unarymath: '-' simpleexpr  */
#line 767 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 4070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* expr: expr '+' expr  */
#line 777 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4076 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* expr: expr '-' expr  */
#line 778 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* expr: expr '*' expr  */
#line 779 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4088 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* expr: expr '/' expr  */
#line 780 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* expr: expr tMOD expr  */
#line 781 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* expr: expr '>' expr  */
#line 782 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4106 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* expr: expr '<' expr  */
#line 783 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4112 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* expr: expr tEQ expr  */
#line 784 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4118 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* expr: expr tNEQ expr  */
#line 785 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* expr: expr tGE expr  */
#line 786 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4130 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* expr: expr tLE expr  */
#line 787 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4136 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 283: /* expr: expr tAND expr  */
#line 788 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 284: /* expr: expr tOR expr  */
#line 789 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* expr: expr '&' expr  */
#line 790 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4154 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* expr: expr tCONCAT expr  */
#line 791 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4160 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 287: /* expr: expr tCONTAINS expr  */
#line 792 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4166 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 288: /* expr: expr tSTARTS expr  */
#line 793 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 291: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 802 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4178 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 292: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 803 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4184 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 804 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 294: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 805 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4196 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 295: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 806 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4202 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 296: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 807 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4208 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 808 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4214 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 809 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 810 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4226 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 811 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4232 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 301: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 812 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4238 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 302: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 813 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4244 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 303: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 814 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4250 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 304: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 815 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4256 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 816 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4262 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 817 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4268 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 307: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 818 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4274 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 310: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 823 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4280 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 311: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 824 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4286 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 312: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 825 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4292 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 313: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 826 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4298 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 314: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 827 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 315: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 828 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4310 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 829 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4316 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 830 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 831 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4328 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 832 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 833 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4340 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 834 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4346 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 322: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 835 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4352 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 323: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 836 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 324: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 837 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4364 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 325: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 838 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 326: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 841 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4376 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 327: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 842 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4382 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 328: /* exprlist: %empty  */
#line 845 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 4388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 330: /* nonemptyexprlist: expr  */
#line 849 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4397 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 331: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 853 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4405 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 4409 "engines/director/lingo/lingo-gr.cpp"

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

#line 858 "engines/director/lingo/lingo-gr.y"


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
