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

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_fread
#define FORBIDDEN_SYMBOL_EXCEPTION_stdin
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr
#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#define FORBIDDEN_SYMBOL_EXCEPTION_getc

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


#line 137 "engines/director/lingo/lingo-gr.cpp"

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
  YYSYMBOL_tRETURN = 26,                   /* tRETURN  */
  YYSYMBOL_tMOVIE = 27,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 28,                     /* tNEXT  */
  YYSYMBOL_tOF = 29,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 30,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 31,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 32,                   /* tREPEAT  */
  YYSYMBOL_tSET = 33,                      /* tSET  */
  YYSYMBOL_tTHEN = 34,                     /* tTHEN  */
  YYSYMBOL_tTO = 35,                       /* tTO  */
  YYSYMBOL_tWHEN = 36,                     /* tWHEN  */
  YYSYMBOL_tWITH = 37,                     /* tWITH  */
  YYSYMBOL_tWHILE = 38,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 39,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 40,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 41,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 42,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 43,                       /* tGE  */
  YYSYMBOL_tLE = 44,                       /* tLE  */
  YYSYMBOL_tEQ = 45,                       /* tEQ  */
  YYSYMBOL_tNEQ = 46,                      /* tNEQ  */
  YYSYMBOL_tAND = 47,                      /* tAND  */
  YYSYMBOL_tOR = 48,                       /* tOR  */
  YYSYMBOL_tNOT = 49,                      /* tNOT  */
  YYSYMBOL_tMOD = 50,                      /* tMOD  */
  YYSYMBOL_tAFTER = 51,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 52,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 53,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 54,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 55,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 56,                     /* tCHAR  */
  YYSYMBOL_tCHARS = 57,                    /* tCHARS  */
  YYSYMBOL_tITEM = 58,                     /* tITEM  */
  YYSYMBOL_tITEMS = 59,                    /* tITEMS  */
  YYSYMBOL_tLINE = 60,                     /* tLINE  */
  YYSYMBOL_tLINES = 61,                    /* tLINES  */
  YYSYMBOL_tWORD = 62,                     /* tWORD  */
  YYSYMBOL_tWORDS = 63,                    /* tWORDS  */
  YYSYMBOL_tABBREVIATED = 64,              /* tABBREVIATED  */
  YYSYMBOL_tABBREV = 65,                   /* tABBREV  */
  YYSYMBOL_tABBR = 66,                     /* tABBR  */
  YYSYMBOL_tLONG = 67,                     /* tLONG  */
  YYSYMBOL_tSHORT = 68,                    /* tSHORT  */
  YYSYMBOL_tDATE = 69,                     /* tDATE  */
  YYSYMBOL_tLAST = 70,                     /* tLAST  */
  YYSYMBOL_tMENU = 71,                     /* tMENU  */
  YYSYMBOL_tMENUS = 72,                    /* tMENUS  */
  YYSYMBOL_tMENUITEM = 73,                 /* tMENUITEM  */
  YYSYMBOL_tMENUITEMS = 74,                /* tMENUITEMS  */
  YYSYMBOL_tNUMBER = 75,                   /* tNUMBER  */
  YYSYMBOL_tTHE = 76,                      /* tTHE  */
  YYSYMBOL_tTIME = 77,                     /* tTIME  */
  YYSYMBOL_tXTRAS = 78,                    /* tXTRAS  */
  YYSYMBOL_tCASTLIBS = 79,                 /* tCASTLIBS  */
  YYSYMBOL_tSOUND = 80,                    /* tSOUND  */
  YYSYMBOL_tSPRITE = 81,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 82,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 83,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 84,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 85,                 /* tPROPERTY  */
  YYSYMBOL_tON = 86,                       /* tON  */
  YYSYMBOL_tMETHOD = 87,                   /* tMETHOD  */
  YYSYMBOL_tENDIF = 88,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 89,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 90,                  /* tENDTELL  */
  YYSYMBOL_tASSERTERROR = 91,              /* tASSERTERROR  */
  YYSYMBOL_92_ = 92,                       /* '<'  */
  YYSYMBOL_93_ = 93,                       /* '>'  */
  YYSYMBOL_94_ = 94,                       /* '&'  */
  YYSYMBOL_95_ = 95,                       /* '+'  */
  YYSYMBOL_96_ = 96,                       /* '-'  */
  YYSYMBOL_97_ = 97,                       /* '*'  */
  YYSYMBOL_98_ = 98,                       /* '/'  */
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
#define YYFINAL  189
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4573

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  333
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  627

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   346


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
       2,     2,     2,     2,     2,     2,     2,     2,    94,     2,
     101,   102,    97,    95,   100,    96,     2,    98,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   105,     2,
      92,     2,    93,     2,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   208,   208,   210,   216,   223,   224,   225,   226,   227,
     228,   257,   261,   263,   265,   266,   269,   275,   282,   283,
     284,   289,   293,   297,   298,   299,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   377,   378,   379,   382,   386,   396,   397,
     400,   401,   402,   403,   404,   405,   408,   409,   410,   413,
     414,   415,   416,   417,   418,   419,   420,   425,   426,   427,
     428,   429,   430,   431,   432,   435,   438,   443,   447,   453,
     458,   461,   466,   470,   476,   483,   483,   489,   494,   500,
     506,   512,   520,   521,   522,   523,   526,   526,   528,   529,
     530,   533,   537,   541,   547,   551,   555,   559,   566,   568,
     570,   572,   574,   578,   582,   586,   588,   589,   593,   599,
     606,   607,   610,   611,   615,   621,   628,   629,   635,   636,
     637,   638,   639,   640,   641,   642,   648,   653,   654,   655,
     656,   657,   658,   661,   663,   664,   667,   668,   671,   672,
     673,   675,   677,   679,   681,   683,   685,   687,   689,   692,
     693,   694,   695,   698,   699,   702,   707,   710,   715,   721,
     722,   723,   724,   725,   728,   729,   730,   731,   732,   735,
     737,   738,   739,   740,   741,   742,   743,   744,   745,   746,
     750,   751,   752,   753,   754,   755,   756,   757,   760,   760,
     762,   763,   766,   767,   768,   769,   770,   773,   774,   775,
     781,   785,   788,   793,   794,   795,   796,   797,   800,   801,
     804,   805,   809,   810,   811,   812,   813,   814,   815,   816,
     817,   818,   819,   820,   821,   822,   823,   824,   825,   826,
     827,   834,   835,   836,   837,   838,   839,   840,   841,   842,
     843,   844,   845,   846,   847,   848,   849,   850,   851,   852,
     855,   856,   857,   858,   859,   860,   861,   862,   863,   864,
     865,   866,   867,   868,   869,   870,   871,   872,   875,   876,
     879,   880,   883,   887
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
  "tGLOBAL", "tGO", "tHILITE", "tIF", "tIN", "tINTO", "tMACRO", "tRETURN",
  "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET",
  "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tFACTORY", "tOPEN", "tPLAY",
  "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT", "tMOD",
  "tAFTER", "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR",
  "tCHARS", "tITEM", "tITEMS", "tLINE", "tLINES", "tWORD", "tWORDS",
  "tABBREVIATED", "tABBREV", "tABBR", "tLONG", "tSHORT", "tDATE", "tLAST",
  "tMENU", "tMENUS", "tMENUITEM", "tMENUITEMS", "tNUMBER", "tTHE", "tTIME",
  "tXTRAS", "tCASTLIBS", "tSOUND", "tSPRITE", "tINTERSECTS", "tWITHIN",
  "tTELL", "tPROPERTY", "tON", "tMETHOD", "tENDIF", "tENDREPEAT",
  "tENDTELL", "tASSERTERROR", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'\\n'", "','", "'('", "')'", "'['", "']'", "':'", "$accept",
  "script", "scriptpartlist", "scriptpart", "macro", "factory", "method",
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

#define YYPACT_NINF (-541)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2886,  -541,  3392,  -541,  -541,  -541,  -541,    38,  -541,   -10,
    -541,  3392,  1990,    38,  2090,  -541,  -541,  3392,  1390,  -541,
      -5,  -541,  -541,  2190,    53,  3475,  -541,   -54,  -541,  -541,
    3392,  2190,  1990,  3392,  -541,  -541,  -541,  -541,  -541,  -541,
    -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,
    -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,
    -541,  2090,  3392,  3392,   -49,  3723,  -541,    73,  2886,  -541,
    -541,  -541,  -541,  2190,  -541,  -541,  -541,  -541,  -541,  -541,
    -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,
    -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,
    -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,    52,
    -541,  -541,  2290,  2290,  2090,  2090,  2090,  2090,     6,    16,
      68,  -541,  -541,    84,    26,  -541,  -541,  -541,  -541,  2290,
    2290,  2290,  2290,  2090,  2090,  2390,  2090,  2090,  2090,  2090,
    3558,  2090,  2390,  2390,  1490,   688,   -14,    88,    91,  -541,
    -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,   790,
    -541,    95,  2090,  3996,  3392,  -541,  4196,    97,    99,   890,
    3392,  2090,  3392,  -541,  -541,    -2,  -541,  -541,   108,   113,
     990,   114,   118,   119,   591,   121,  3392,  -541,  -541,  -541,
    -541,   125,  1090,  -541,  3392,  1590,  -541,  -541,  -541,  3772,
    3794,  3828,  3850,   139,  -541,  -541,  -541,  3392,  -541,  -541,
    1190,  4393,  -541,   -13,    -3,    36,    60,    65,   139,    64,
      70,  4380,  -541,  -541,  -541,  2790,  4018,    -8,    45,    56,
      59,    39,   -42,    66,  -541,  4393,   122,    72,  1690,  -541,
    -541,   200,  2090,  2090,  2090,  2090,  2090,  2090,  2090,  2090,
    2090,  2090,  2090,  2090,  2090,  2090,  2090,  2090,  2090,  2090,
    -541,  -541,  4112,  -541,  -541,  4052,  2969,   129,  -541,  -541,
    -541,  3641,  3641,  3641,    12,  4209,   201,  -541,  -541,  2090,
      -7,  -541,  2090,  -541,  -541,  -541,  3723,  3052,  -541,   130,
    -541,  -541,  -541,  4031,  2390,  2090,  2390,  2090,  2390,  2090,
    2390,  2090,  -541,  -541,  -541,  -541,    23,  -541,   204,  4449,
    -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,  -541,
    1290,  2490,  2390,  2390,  4125,  1790,  -541,  2090,  2090,  2090,
    2090,  -541,  2090,  2590,  -541,  -541,  2090,  -541,  2790,   133,
    2090,   -12,   -12,   -12,   -12,  4475,  4475,  -541,    22,   -12,
     -12,   -12,   -12,    22,    57,    57,  -541,  -541,   133,  2090,
    2090,  2090,  2090,  2090,  2090,  2090,  2090,  2090,  2090,  2090,
    2090,  2090,  2090,  2090,  2090,  2090,  2090,  -541,  3301,   218,
    3052,     6,  -541,   136,  -541,   154,   156,  2090,  2090,  3052,
    2690,  4222,  3392,  3392,  -541,  -541,  -541,    -7,  -541,  4279,
    -541,  -541,  -541,   166,  3052,  -541,  3052,  1890,  -541,   441,
    -541,  3884,  -541,  3906,  -541,  3940,  -541,  -541,  2390,  2090,
      23,    23,    23,    23,  2390,  -541,  2390,    23,  -541,  -541,
    2390,  2390,  -541,  -541,  -541,  -541,  -541,  -541,  2090,   155,
    -541,   133,  4393,  4393,  4393,  4393,  4393,  -541,  4393,  4393,
    4138,  2090,   157,  4393,  -541,   -12,   -12,   -12,   -12,  4475,
    4475,  -541,    22,   -12,   -12,   -12,   -12,    22,    57,    57,
    -541,  -541,   133,  -541,  -541,    -9,  3301,  -541,  3135,  -541,
    -541,  -541,  -541,  4292,   577,   169,  2090,  2090,  2090,  2090,
    -541,  -541,  -541,    78,  3392,  -541,  -541,   161,  -541,   252,
    -541,   133,  2390,  2390,  2390,  2390,  -541,  4393,  2390,  2390,
    2390,  2390,  -541,   233,   192,  -541,  -541,   133,  -541,   162,
    2090,   163,  -541,  -541,  3218,   176,  -541,  -541,  3301,  -541,
    3052,   242,  2090,   182,  -541,  4462,  -541,  3962,  4462,  4462,
    -541,   183,  -541,  3392,   186,  -541,  -541,  -541,  -541,  -541,
    -541,  -541,  -541,   197,  2390,  -541,   188,  -541,   133,  -541,
    3301,  -541,  -541,   206,   196,  2090,  4305,  -541,  2090,  2090,
    2090,  2090,  2090,  2090,  2090,  2090,  2090,  2090,  2090,  2090,
    2090,  2090,  2090,  2090,   221,  3052,   103,  -541,  2390,  -541,
     194,   206,  -541,   199,  4362,  3052,    51,    51,    51,   458,
     458,  -541,    27,    51,    51,    51,    51,    27,    62,    62,
    -541,  -541,  2090,  -541,  -541,  -541,  -541,  -541,  -541,  3052,
     214,  4462,   215,   207,   208,  -541,  -541
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    26,    23,    32,    38,    59,    68,    36,    37,     0,
      39,    93,   125,    40,     0,    41,    43,     0,     0,    53,
      54,    56,    57,   125,    58,     0,    66,     0,    69,    67,
       0,   125,   125,    93,    30,    31,    33,    34,    44,    45,
      47,    48,    71,    72,    27,    28,    29,    49,    61,    35,
      46,    50,    51,    52,    55,    64,    65,    62,    63,    42,
      70,     0,    93,     0,     0,    60,     5,     0,     2,     3,
       6,     7,     8,   125,     9,    98,   100,   106,   107,   108,
     101,   102,   103,   104,   105,    75,    36,    74,    76,    78,
      79,    40,    80,    82,    89,    54,    88,    58,    90,    92,
      77,    85,    86,    81,    91,    87,    84,    83,    60,     0,
      73,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   119,    96,     0,    94,   178,   179,   181,   180,    32,
      38,    59,    68,    39,    53,     0,    33,    44,    47,    71,
      64,    63,     0,     0,     0,   330,   193,     0,     0,   270,
     188,   189,   190,   191,   222,   223,   192,   271,   272,   135,
     273,     0,     0,     0,    93,   120,     0,     0,     0,   135,
       0,     0,    64,   193,   196,     0,   197,   165,     0,     0,
     135,     0,     0,     0,     0,     0,    93,    99,   124,     1,
       4,     0,   135,    10,     0,     0,   199,   215,   198,     0,
       0,     0,     0,     0,   122,   118,   148,    95,   213,   214,
     137,   138,   182,    27,    28,    29,    49,    61,    46,    55,
     219,     0,   268,   269,   130,   188,     0,   178,   179,   181,
     180,     0,   193,     0,   260,   332,     0,   331,     0,   111,
     112,    56,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   136,
     126,   291,   135,   292,   123,     0,     0,     0,   121,   117,
     110,    43,    30,    31,     0,     0,   250,   146,   147,     0,
      14,   115,    69,   113,   114,   150,     0,   166,   149,     0,
     109,    25,   216,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   209,   211,   212,   210,     0,    97,    56,   141,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
       0,     0,     0,     0,   135,     0,   187,     0,     0,     0,
       0,   258,     0,     0,   259,   257,     0,   183,   188,   135,
       0,   283,   284,   281,   282,   285,   286,   278,   288,   289,
     290,   280,   279,   287,   274,   275,   276,   277,   135,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   136,   128,   172,   151,
     166,    64,   194,     0,   195,     0,     0,     0,     0,   166,
       0,     0,    23,     0,    18,    19,    12,    15,    16,     0,
     163,   170,   171,     0,   167,   168,   166,     0,   200,     0,
     204,     0,   206,     0,   202,     0,   248,   249,     0,     0,
      34,    45,    48,    72,    50,   245,    51,    52,   246,   247,
      62,    63,   221,   225,   224,   220,   328,   329,   136,     0,
     131,   135,   266,   267,   265,   263,   264,   261,   262,   333,
     135,   136,     0,   140,   127,   302,   303,   300,   301,   304,
     305,   297,   307,   308,   309,   299,   298,   306,   293,   294,
     295,   296,   135,   176,   177,   157,   173,   174,     0,    11,
     142,   143,   144,     0,     0,     0,    50,    51,    62,    63,
     251,   252,   145,     0,    93,    17,   116,     0,   169,    22,
     217,   135,     0,     0,     0,     0,   208,   139,     0,     0,
       0,     0,   229,     0,     0,   227,   228,   135,   133,     0,
     136,     0,   184,   129,     0,     0,   152,   175,   172,   153,
     166,     0,     0,     0,   310,   253,   311,     0,   255,   256,
      20,     0,   164,    23,     0,   201,   205,   207,   203,   240,
     242,   243,   241,     0,     0,   244,     0,   132,   135,   185,
     172,   155,   158,   157,     0,     0,     0,   159,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   166,     0,   218,     0,   134,
       0,   157,   154,     0,     0,   166,   320,   321,   319,   322,
     323,   316,   325,   326,   327,   318,   317,   324,   312,   313,
     314,   315,     0,    13,    21,   226,   186,   156,   162,   166,
       0,   254,     0,     0,     0,   160,   161
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -541,  -541,  -541,   237,  -541,  -541,  -541,  -541,  -541,   -88,
    -541,  -381,     0,    -1,   -28,  -541,     3,  -360,   -61,  -541,
       8,  -218,   278,  -541,  -541,  -541,  -541,  -541,  -540,  -541,
    -541,  -541,  -311,  -541,   -93,  -507,  -541,  -164,  -122,    29,
     -68,  -541,     7,  -541,  -541,    44,  -541,    -4,  -201,  -541,
    -541,  -241,  -541,  -541,  -541,  -541,   -19,  -541,    79,    -6,
    -197,    28,  -150,  -541,  -223
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    67,    68,    69,    70,    71,   395,   396,   397,   398,
      72,   109,   110,   146,   123,   124,   402,    75,    76,    77,
     147,   260,   148,    78,   279,    79,    80,    81,   526,    82,
      83,    84,   403,   404,   405,   475,   476,   477,   149,   150,
     383,   175,   151,   306,   152,   196,   153,   432,   433,   154,
     155,   418,   176,   490,   156,   233,   234,   157,   158,   235,
     262,   535,   160,   236,   237
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      73,   111,   392,    74,   188,   183,   159,   524,   163,   263,
     122,   493,   166,   309,   119,   339,   164,   169,   474,   263,
     161,   563,   120,   592,   173,   180,   159,   167,   324,   178,
     263,   168,   122,   277,   185,   387,   358,   261,   248,   179,
     181,   249,   263,   278,   377,   177,   416,   261,   112,   113,
     187,   617,   417,   591,   174,   184,   310,   388,   261,   238,
     263,   122,   186,   332,   311,    73,   312,   192,    73,   479,
     261,    74,   248,   189,   313,   263,   203,   573,   485,   525,
     393,   191,   254,   255,   256,   257,   258,   238,   261,   121,
     170,   171,   394,   320,   114,   499,   115,   327,   116,   321,
     117,   573,   441,   261,   574,   314,   439,   248,   199,   200,
     201,   202,   573,   315,   118,   204,   474,   255,   256,   257,
     258,   452,   580,   581,   582,   583,   207,   210,   211,   316,
     199,   200,   201,   202,   318,   221,   267,   317,   226,   220,
     454,   450,   319,   331,   232,   579,   580,   581,   582,   583,
     328,   193,   194,   472,   257,   258,   265,   198,   289,   582,
     583,   329,   586,   122,   330,   275,   333,   205,   474,   274,
     334,   276,   336,   225,   198,   208,   209,   540,   194,   508,
     509,   510,   511,   206,   501,   122,   514,   239,   263,   293,
     240,   197,   197,   291,   264,   302,   269,   303,   270,   304,
     474,   305,   614,   194,   385,   386,   307,   280,   197,   197,
     197,   197,   281,   283,   212,   517,   261,   284,   285,   564,
     288,   222,   223,   519,   290,   400,   335,   340,   380,   406,
     390,   419,   521,   451,   478,   480,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   481,   523,   482,   497,   518,   533,   522,
     542,   543,   553,   554,   557,   559,    73,   338,   588,   379,
     173,   173,   173,   391,   613,   562,   399,   565,   384,   384,
     384,   567,   585,   544,   620,   593,    73,    73,   587,   409,
     589,   411,   612,   413,   525,   415,   616,   558,   618,   556,
     382,   382,   382,   623,   624,   190,   625,   626,   622,   495,
     182,   498,   527,   555,   447,     0,     0,   435,     0,     0,
       0,   442,   443,   444,   445,     0,   446,   448,     0,     0,
     449,     0,   232,     0,   453,     0,   536,   536,   536,   536,
     590,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
       0,     0,     0,   408,     0,   410,     0,   412,    73,   414,
      73,   483,   484,     0,     0,     0,     0,     0,     0,    73,
       0,   111,   494,     0,     0,     0,     0,     0,     0,   434,
     434,   436,   437,     0,    73,     0,    73,     0,     0,     0,
       0,     0,     0,   507,     0,     0,     0,     0,   536,   536,
     536,   536,   536,   536,   536,   536,   536,   536,   536,   536,
     536,   536,   536,   536,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   449,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   536,     0,     0,     0,   541,     0,     0,   491,
     502,     0,     0,     0,     0,     0,    73,     0,    73,     0,
       0,   529,     0,     0,   242,   243,   244,   245,   246,   247,
       0,   248,     0,   122,   249,   250,   251,   506,     0,     0,
       0,   568,   569,   512,   570,   513,     0,     0,   573,   515,
     516,   574,   575,   576,     0,   537,   538,   539,     0,     0,
       0,     0,     0,     0,    73,     0,   566,   561,    73,     0,
      73,     0,     0,   252,   253,   254,   255,   256,   257,   258,
       0,     0,   111,     0,     0,     0,     0,     0,     0,     0,
     577,   578,   579,   580,   581,   582,   583,     0,     0,   594,
      73,     0,     0,     0,     0,   534,   534,   534,   534,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   545,   546,   547,   548,    73,     0,   549,   550,   551,
     552,     0,   531,     0,     0,    73,   596,   597,   598,   599,
     600,   601,   602,   603,   604,   605,   606,   607,   608,   609,
     610,   611,   532,     0,     0,     0,     0,     0,     0,    73,
     242,   243,   244,   245,   246,   247,   286,   248,     0,     0,
     249,   250,   251,   512,   242,   243,   244,   245,   246,   247,
     621,   248,     0,     0,   249,   250,   251,   534,   534,   534,
     534,   534,   534,   534,   534,   534,   534,   534,   534,   534,
     534,   534,   534,     0,     0,     0,     0,   615,     0,   252,
     253,   254,   255,   256,   257,   258,     0,     0,     0,     0,
       0,     0,     0,   252,   253,   254,   255,   256,   257,   258,
     287,   534,   227,   228,     1,   229,   230,    85,   129,   130,
     131,   132,    86,     8,    87,    88,    10,    89,    90,    91,
      92,    15,    16,    93,    94,    19,    95,    21,    22,    96,
      97,    98,    99,    26,     0,    28,    29,   100,   101,   102,
     103,     0,     0,     0,     0,     0,     0,   135,     0,    34,
      35,     0,     0,     0,   136,    37,   137,    39,   138,    41,
     139,    43,    44,    45,    46,    47,    48,    49,    50,    51,
       0,    52,    53,    54,   140,    56,     0,     0,    57,   141,
      59,    60,   104,   105,   106,   107,     0,     0,     0,   108,
       0,     0,     0,   142,   143,     0,     0,     0,     0,   162,
       0,   145,     0,   231,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,   241,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,   242,   243,   244,   245,   246,   247,   135,
     248,    34,    35,   249,   250,   251,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,   252,   253,   254,   255,   256,   257,   258,     0,
     259,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,   271,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,   242,   243,   244,   245,   246,   247,   135,
     248,   272,   273,   249,   250,   251,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,   252,   253,   254,   255,   256,   257,   258,     0,
     259,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,   282,    29,   100,
     101,   102,   103,   242,   243,   244,   245,   246,   247,   135,
     248,    34,    35,   249,   250,   251,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,   252,   253,   254,   255,   256,   257,   258,     0,
     259,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,   242,   243,   244,   245,   246,   247,   135,
     248,    34,    35,   249,   250,   251,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,   252,   253,   254,   255,   256,   257,   258,     0,
     259,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,   308,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,   242,   243,   244,   245,   246,   247,   135,
     248,    34,    35,   249,   250,   251,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,   252,   253,   254,   255,   256,   257,   258,     0,
       0,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,   420,   137,   421,
     138,   422,   139,   423,    44,    45,    46,    47,    48,    49,
      50,   424,   425,   426,   427,    54,   140,    56,   428,   429,
     430,   431,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,   165,
       0,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,   224,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,   292,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,   337,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,   440,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,   500,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,   133,    89,
      90,    91,    92,    15,    16,    93,    94,   134,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   144,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   144,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,    58,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   195,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,    58,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,   424,     0,   426,    53,    54,   140,    56,     0,     0,
     430,   431,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,     0,   145,   227,   228,     1,   229,   230,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,   486,     0,   487,    53,    54,   140,    56,     0,     0,
     488,   489,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,   142,   143,     0,     0,     0,
       0,   162,     0,   145,   125,   126,     1,   127,   128,    85,
     129,   130,   131,   132,    86,     8,    87,    88,    10,    89,
      90,    91,    92,    15,    16,    93,    94,    19,    95,    21,
      22,    96,    97,    98,    99,    26,     0,    28,    29,   100,
     101,   102,   103,     0,     0,     0,     0,     0,     0,   135,
       0,    34,    35,     0,     0,     0,   136,    37,   137,    39,
     138,    41,   139,    43,    44,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,   140,    56,     0,     0,
      57,   141,    59,    60,   104,   105,   106,   107,     0,     0,
       0,   108,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   162,     1,   145,     0,     2,     3,     4,     5,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
       0,    26,    27,    28,    29,    30,    31,    32,    33,     0,
       0,     0,     0,     0,     0,     0,     0,    34,    35,     0,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,     0,    52,
      53,    54,    55,    56,     0,     0,    57,    58,    59,    60,
      61,    62,    63,     0,    64,     1,     0,    65,     0,     3,
       4,     5,     6,     7,     8,    66,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,    31,
      32,    33,     0,     0,     0,     0,     0,     0,     0,     0,
      34,    35,     0,     0,     0,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,     0,    52,    53,    54,    55,    56,     0,     0,    57,
      58,    59,    60,    61,    62,     0,     0,    64,     1,     0,
      65,     0,     3,     4,     5,     6,     7,     8,   378,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    18,    19,
      20,    21,    22,    23,    24,    25,     0,    26,    27,    28,
      29,     0,    31,    32,    33,     0,     0,     0,     0,     0,
       0,     0,     0,    34,    35,     0,     0,     0,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,     0,    52,    53,    54,    55,    56,
       0,     0,    57,    58,    59,    60,    61,    62,     0,     0,
      64,     1,     0,    65,     0,     3,     4,     5,     6,     7,
       8,   401,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,    27,    28,    29,     0,    31,    32,    33,     0,     0,
       0,     0,     0,     0,     0,     0,    34,    35,     0,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,     0,    52,    53,
      54,    55,    56,     0,     0,    57,    58,    59,    60,    61,
      62,     0,     0,    64,     1,     0,    65,     0,     3,     4,
       5,     6,     7,     8,   528,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    18,    19,    20,    21,    22,    23,
      24,    25,     0,    26,    27,    28,    29,     0,    31,    32,
      33,     0,     0,     0,     0,     0,     0,     0,     0,    34,
      35,     0,     0,     0,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
       0,    52,    53,    54,    55,    56,     0,     0,    57,    58,
      59,    60,    61,    62,     0,     0,    64,     1,     0,    65,
       0,     3,     4,     5,     6,     7,     8,   560,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,    29,
       0,    31,    32,    33,     0,     0,     0,     0,     0,     0,
       0,     0,    34,    35,     0,     0,     0,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,     0,    52,    53,    54,    55,    56,     0,
       0,    57,    58,    59,    60,    61,    62,     0,     0,     0,
       0,     0,    65,     0,     0,     0,     0,     0,     1,     0,
     473,    85,     3,     4,     5,     6,    86,     8,    87,    88,
      10,    89,    90,    91,    92,    15,    16,    93,    94,    19,
      95,    21,    22,    96,    97,    98,    99,    26,     0,    28,
      29,   100,   101,   102,   103,     0,     0,     0,     0,     0,
       0,     0,     0,    34,    35,     0,     0,     0,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,     0,    52,    53,    54,    55,    56,
       0,     0,    57,    58,    59,    60,   104,   105,   106,   107,
       0,     1,     0,   108,    85,     3,     4,     5,     6,    86,
       8,    87,    88,    10,    89,    90,    91,    92,    15,    16,
      93,    94,    19,    95,    21,    22,    96,    97,    98,    99,
      26,     0,    28,    29,   100,   101,   102,   103,     0,     0,
       0,     0,     0,     0,     0,     0,    34,    35,     0,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,     0,    52,    53,
      54,   172,    56,     0,     0,    57,    58,    59,    60,   104,
     105,   106,   107,     0,     1,     0,   108,    85,     3,     4,
       5,     6,    86,     8,    87,    88,    10,    89,    90,    91,
      92,    15,    16,    93,    94,    19,    95,    21,    22,    96,
      97,    98,    99,    26,     0,    28,    29,   100,   101,   102,
     103,     0,     0,     0,     0,     0,     0,     0,     0,    34,
      35,     0,     0,     0,    36,    37,    38,    39,    40,    41,
      42,    43,   213,   214,   215,   216,   217,    49,   218,    51,
       0,    52,    53,   219,    55,    56,     0,     0,    57,    58,
      59,    60,   104,   105,   106,   107,     0,     1,     0,   108,
      85,   129,   130,     5,     6,    86,     8,    87,    88,    10,
      89,    90,    91,    92,    15,    16,    93,    94,    19,    95,
      21,    22,    96,    97,    98,    99,    26,     0,    28,    29,
     100,   101,   102,   103,     0,     0,     0,     0,     0,     0,
       0,     0,    34,    35,     0,     0,     0,   136,    37,   137,
      39,   138,    41,   139,    43,    44,    45,    46,    47,    48,
      49,    50,    51,     0,    52,    53,    54,   381,    56,     0,
       0,    57,    58,    59,    60,   104,   105,   106,   107,     1,
       0,     0,   108,     3,     4,     5,     6,     7,     8,     0,
       9,    10,    11,    12,    13,     0,    15,    16,     0,    18,
      19,    20,    21,    22,    23,    97,    25,     0,    26,     0,
      28,    29,     0,    31,    32,    33,     0,     0,     0,     0,
       0,     0,     0,     0,    34,    35,     0,     0,     0,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,     0,    52,    53,    54,    55,
      56,   294,     0,    57,    58,    59,    60,   295,    62,     0,
       0,     0,     0,     0,    65,   242,   243,   244,   245,   246,
     247,     0,   248,   296,     0,   249,   250,   251,     0,   297,
       0,     0,     0,     0,     0,     0,     0,   242,   243,   244,
     245,   246,   247,     0,   248,     0,     0,   249,   250,   251,
       0,     0,     0,     0,     0,     0,     0,   298,     0,     0,
       0,     0,     0,   299,   252,   253,   254,   255,   256,   257,
     258,   242,   243,   244,   245,   246,   247,     0,   248,   300,
       0,   249,   250,   251,     0,   301,   252,   253,   254,   255,
     256,   257,   258,   242,   243,   244,   245,   246,   247,     0,
     248,     0,     0,   249,   250,   251,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
     252,   253,   254,   255,   256,   257,   258,   242,   243,   244,
     245,   246,   247,     0,   248,   504,     0,   249,   250,   251,
       0,     0,   252,   253,   254,   255,   256,   257,   258,   242,
     243,   244,   245,   246,   247,     0,   248,     0,     0,   249,
     250,   251,     0,     0,     0,     0,     0,     0,     0,   505,
       0,     0,     0,     0,     0,     0,   252,   253,   254,   255,
     256,   257,   258,   242,   243,   244,   245,   246,   247,     0,
     248,   584,     0,   249,   250,   251,     0,     0,   252,   253,
     254,   255,   256,   257,   258,   568,   569,     0,   570,   571,
     572,     0,   573,     0,     0,   574,   575,   576,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     266,     0,   252,   253,   254,   255,   256,   257,   258,   242,
     243,   244,   245,   246,   247,     0,   248,     0,     0,   249,
     250,   251,     0,     0,   577,   578,   579,   580,   581,   582,
     583,   242,   243,   244,   245,   246,   247,     0,   248,     0,
       0,   249,   250,   251,   242,   243,   244,   245,   246,   247,
       0,   248,     0,     0,   249,   250,   251,     0,   252,   253,
     254,   255,   256,   257,   258,   242,   243,   244,   245,   246,
     247,     0,   248,     0,     0,   249,   250,   251,     0,     0,
     252,   253,   254,   255,   256,   257,   258,     0,   325,     0,
     326,     0,     0,   252,   253,   254,   255,   256,   257,   258,
       0,   407,     0,   326,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   252,   253,   254,   255,   256,   257,
     258,     0,     0,     0,   326,   359,   360,   361,   362,   363,
     364,     0,   365,     0,     0,   366,   367,   368,   359,   360,
     361,   362,   363,   364,     0,   365,     0,     0,   366,   367,
     368,   359,   360,   361,   362,   363,   364,     0,   365,     0,
       0,   366,   367,   368,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   369,   370,   371,   372,   373,   374,
     375,     0,   376,     0,     0,     0,     0,   369,   370,   371,
     372,   373,   374,   375,     0,   438,     0,     0,     0,     0,
     369,   370,   371,   372,   373,   374,   375,     0,   520,   242,
     243,   244,   245,   246,   247,     0,   248,     0,     0,   249,
     250,   251,   242,   243,   244,   245,   246,   247,     0,   248,
       0,     0,   249,   250,   251,   242,   243,   244,   245,   246,
     247,     0,   248,     0,     0,   249,   250,   251,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   252,   253,
     254,   255,   256,   257,   258,   268,     0,     0,     0,     0,
       0,   252,   253,   254,   255,   256,   257,   258,   389,     0,
       0,     0,     0,     0,   252,   253,   254,   255,   256,   257,
     258,   492,   242,   243,   244,   245,   246,   247,     0,   248,
       0,     0,   249,   250,   251,   242,   243,   244,   245,   246,
     247,     0,   248,     0,     0,   249,   250,   251,   242,   243,
     244,   245,   246,   247,     0,   248,     0,     0,   249,   250,
     251,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   252,   253,   254,   255,   256,   257,   258,   496,     0,
       0,     0,     0,     0,   252,   253,   254,   255,   256,   257,
     258,   530,     0,     0,     0,     0,     0,   252,   253,   254,
     255,   256,   257,   258,   595,   242,   243,   244,   245,   246,
     247,     0,   248,     0,     0,   249,   250,   251,     0,     0,
       0,     0,     0,   242,   243,   244,   245,   246,   247,     0,
     248,     0,     0,   249,   250,   251,   242,   243,   244,   245,
     246,   247,     0,   248,     0,     0,   249,   250,   251,     0,
       0,     0,     0,     0,   252,   253,   254,   255,   256,   257,
     258,   619,   322,   323,     0,     0,     0,     0,     0,     0,
       0,     0,   252,   253,   254,   255,   256,   257,   258,     0,
       0,     0,     0,     0,     0,   252,   253,   254,   255,   256,
     257,   258,   359,   360,   361,   362,   363,   364,     0,   365,
       0,     0,   366,   367,   368,   568,   569,     0,   570,   571,
     572,     0,   573,     0,     0,   574,   575,   576,   242,   243,
     244,   245,     0,     0,     0,   248,     0,     0,   249,   250,
     251,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   369,   370,   371,   372,   373,   374,   375,     0,     0,
       0,     0,     0,     0,   577,   578,   579,   580,   581,   582,
     583,     0,     0,     0,     0,     0,     0,   252,   253,   254,
     255,   256,   257,   258
};

static const yytype_int16 yycheck[] =
{
       0,     2,     9,     0,    65,    33,    12,    16,    14,   159,
      11,   392,    18,   210,     7,   238,    17,    23,   378,   169,
      13,   528,    32,   563,    25,    31,    32,    32,   225,    30,
     180,    23,    33,    35,    62,    23,   259,   159,    50,    31,
      32,    53,   192,    45,   262,    99,    23,   169,    10,    11,
      99,   591,    29,   560,    25,    61,    69,    45,   180,   101,
     210,    62,    63,   105,    77,    65,    69,    73,    68,   380,
     192,    68,    50,     0,    77,   225,    70,    50,   389,    88,
      87,    73,    94,    95,    96,    97,    98,   101,   210,    99,
      37,    38,    99,    29,    56,   406,    58,   105,    60,    29,
      62,    50,   325,   225,    53,    69,   324,    50,   114,   115,
     116,   117,    50,    77,    76,    99,   476,    95,    96,    97,
      98,   339,    95,    96,    97,    98,   100,   133,   134,    69,
     136,   137,   138,   139,    69,   141,   164,    77,   144,   140,
     358,   338,    77,   104,   145,    94,    95,    96,    97,    98,
     105,    99,   100,   376,    97,    98,   162,   113,   186,    97,
      98,   105,   543,   164,   105,   171,   100,    99,   528,   170,
     104,   172,   100,   144,   130,   131,   132,    99,   100,   420,
     421,   422,   423,    99,   407,   186,   427,    99,   338,   195,
      99,   112,   113,   194,    99,    56,    99,    58,    99,    60,
     560,    62,    99,   100,   272,   273,   207,    99,   129,   130,
     131,   132,    99,    99,   135,   438,   338,    99,    99,   530,
      99,   142,   143,   441,    99,   286,   104,    27,    99,    99,
      29,    27,   450,   100,    16,    99,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,    99,   472,    99,    90,   102,    89,   102,
      99,     9,    29,    71,   102,   102,   266,   238,    71,   266,
     271,   272,   273,   279,   585,    99,   282,    35,   271,   272,
     273,    99,    99,   501,   595,    89,   286,   287,   102,   295,
     102,   297,    71,   299,    88,   301,   102,   520,    99,   517,
     271,   272,   273,    89,    89,    68,    99,    99,   619,   397,
      32,   404,   476,   514,   333,    -1,    -1,   321,    -1,    -1,
      -1,   327,   328,   329,   330,    -1,   332,   333,    -1,    -1,
     336,    -1,   333,    -1,   340,    -1,   486,   487,   488,   489,
     558,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
      -1,    -1,    -1,   294,    -1,   296,    -1,   298,   378,   300,
     380,   387,   388,    -1,    -1,    -1,    -1,    -1,    -1,   389,
      -1,   392,   393,    -1,    -1,    -1,    -1,    -1,    -1,   320,
     321,   322,   323,    -1,   404,    -1,   406,    -1,    -1,    -1,
      -1,    -1,    -1,   419,    -1,    -1,    -1,    -1,   568,   569,
     570,   571,   572,   573,   574,   575,   576,   577,   578,   579,
     580,   581,   582,   583,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   451,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   612,    -1,    -1,    -1,   494,    -1,    -1,   390,
      29,    -1,    -1,    -1,    -1,    -1,   476,    -1,   478,    -1,
      -1,   478,    -1,    -1,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,   494,    53,    54,    55,   418,    -1,    -1,
      -1,    43,    44,   424,    46,   426,    -1,    -1,    50,   430,
     431,    53,    54,    55,    -1,   487,   488,   489,    -1,    -1,
      -1,    -1,    -1,    -1,   524,    -1,   532,   524,   528,    -1,
     530,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      -1,    -1,   543,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    -1,    -1,   565,
     560,    -1,    -1,    -1,    -1,   486,   487,   488,   489,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   502,   503,   504,   505,   585,    -1,   508,   509,   510,
     511,    -1,    15,    -1,    -1,   595,   568,   569,   570,   571,
     572,   573,   574,   575,   576,   577,   578,   579,   580,   581,
     582,   583,    35,    -1,    -1,    -1,    -1,    -1,    -1,   619,
      43,    44,    45,    46,    47,    48,    35,    50,    -1,    -1,
      53,    54,    55,   554,    43,    44,    45,    46,    47,    48,
     612,    50,    -1,    -1,    53,    54,    55,   568,   569,   570,
     571,   572,   573,   574,   575,   576,   577,   578,   579,   580,
     581,   582,   583,    -1,    -1,    -1,    -1,   588,    -1,    92,
      93,    94,    95,    96,    97,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   612,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    -1,    37,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    80,    81,
      82,    83,    84,    85,    86,    87,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    -1,    -1,    -1,    -1,   101,
      -1,   103,    -1,   105,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    -1,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    99,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,   102,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,   102,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,   102,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,   102,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,   102,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    -1,    -1,    -1,
      -1,   101,    -1,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,     6,   103,    -1,     9,    10,    11,    12,    13,
      14,    15,    -1,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,
      -1,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    -1,    73,
      74,    75,    76,    77,    -1,    -1,    80,    81,    82,    83,
      84,    85,    86,    -1,    88,     6,    -1,    91,    -1,    10,
      11,    12,    13,    14,    15,    99,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    26,    27,    28,    29,    30,
      31,    32,    33,    -1,    35,    36,    37,    38,    -1,    40,
      41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    52,    -1,    -1,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    -1,    73,    74,    75,    76,    77,    -1,    -1,    80,
      81,    82,    83,    84,    85,    -1,    -1,    88,     6,    -1,
      91,    -1,    10,    11,    12,    13,    14,    15,    99,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    35,    36,    37,
      38,    -1,    40,    41,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    -1,    73,    74,    75,    76,    77,
      -1,    -1,    80,    81,    82,    83,    84,    85,    -1,    -1,
      88,     6,    -1,    91,    -1,    10,    11,    12,    13,    14,
      15,    99,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    -1,
      35,    36,    37,    38,    -1,    40,    41,    42,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    -1,    73,    74,
      75,    76,    77,    -1,    -1,    80,    81,    82,    83,    84,
      85,    -1,    -1,    88,     6,    -1,    91,    -1,    10,    11,
      12,    13,    14,    15,    99,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    35,    36,    37,    38,    -1,    40,    41,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      52,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    80,    81,
      82,    83,    84,    85,    -1,    -1,    88,     6,    -1,    91,
      -1,    10,    11,    12,    13,    14,    15,    99,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    35,    36,    37,    38,
      -1,    40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    -1,    73,    74,    75,    76,    77,    -1,
      -1,    80,    81,    82,    83,    84,    85,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,     6,    -1,
      99,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    -1,    37,
      38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    -1,    73,    74,    75,    76,    77,
      -1,    -1,    80,    81,    82,    83,    84,    85,    86,    87,
      -1,     6,    -1,    91,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    -1,    37,    38,    39,    40,    41,    42,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    -1,    73,    74,
      75,    76,    77,    -1,    -1,    80,    81,    82,    83,    84,
      85,    86,    87,    -1,     6,    -1,    91,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    -1,    37,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      52,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    80,    81,
      82,    83,    84,    85,    86,    87,    -1,     6,    -1,    91,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    -1,    37,    38,
      39,    40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    52,    -1,    -1,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    -1,    73,    74,    75,    76,    77,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    87,     6,
      -1,    -1,    91,    10,    11,    12,    13,    14,    15,    -1,
      17,    18,    19,    20,    21,    -1,    23,    24,    -1,    26,
      27,    28,    29,    30,    31,    32,    33,    -1,    35,    -1,
      37,    38,    -1,    40,    41,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    -1,    73,    74,    75,    76,
      77,    29,    -1,    80,    81,    82,    83,    35,    85,    -1,
      -1,    -1,    -1,    -1,    91,    43,    44,    45,    46,    47,
      48,    -1,    50,    29,    -1,    53,    54,    55,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    35,    92,    93,    94,    95,    96,    97,
      98,    43,    44,    45,    46,    47,    48,    -1,    50,    29,
      -1,    53,    54,    55,    -1,    35,    92,    93,    94,    95,
      96,    97,    98,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    43,    44,    45,
      46,    47,    48,    -1,    50,    29,    -1,    53,    54,    55,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    43,    44,    45,    46,    47,    48,    -1,
      50,    29,    -1,    53,    54,    55,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    43,    44,    -1,    46,    47,
      48,    -1,    50,    -1,    -1,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      34,    -1,    92,    93,    94,    95,    96,    97,    98,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    53,
      54,    55,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    53,    54,    55,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,    -1,    53,    54,    55,    -1,    92,    93,
      94,    95,    96,    97,    98,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    53,    54,    55,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    -1,   100,    -1,
     102,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      -1,   100,    -1,   102,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    -1,    -1,    -1,   102,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    53,    54,    55,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    53,    54,
      55,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    -1,   100,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    -1,   100,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    -1,   100,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    53,
      54,    55,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    53,    54,    55,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    53,    54,    55,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    53,    54,    55,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    53,    54,    55,    43,    44,    45,    46,
      47,    48,    -1,    50,    -1,    -1,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,    82,    83,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    53,    54,    55,    43,    44,    -1,    46,    47,
      48,    -1,    50,    -1,    -1,    53,    54,    55,    43,    44,
      45,    46,    -1,    -1,    -1,    50,    -1,    -1,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     6,     9,    10,    11,    12,    13,    14,    15,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    35,    36,    37,    38,
      39,    40,    41,    42,    51,    52,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    73,    74,    75,    76,    77,    80,    81,    82,
      83,    84,    85,    86,    88,    91,    99,   107,   108,   109,
     110,   111,   116,   118,   122,   123,   124,   125,   129,   131,
     132,   133,   135,   136,   137,     9,    14,    16,    17,    19,
      20,    21,    22,    25,    26,    28,    31,    32,    33,    34,
      39,    40,    41,    42,    84,    85,    86,    87,    91,   117,
     118,   119,    10,    11,    56,    58,    60,    62,    76,   148,
      32,    99,   119,   120,   121,     4,     5,     7,     8,    10,
      11,    12,    13,    18,    27,    49,    56,    58,    60,    62,
      76,    81,    95,    96,   101,   103,   119,   126,   128,   144,
     145,   148,   150,   152,   155,   156,   160,   163,   164,   165,
     168,   148,   101,   165,   119,    99,   165,    32,   126,   165,
      37,    38,    76,   119,   145,   147,   158,    99,   119,   126,
     165,   126,   128,   120,   165,   120,   119,    99,   124,     0,
     109,   126,   165,    99,   100,   101,   151,   164,   151,   165,
     165,   165,   165,    70,    99,    99,    99,   100,   151,   151,
     165,   165,   164,    64,    65,    66,    67,    68,    70,    75,
     119,   165,   164,   164,   102,   145,   165,     4,     5,     7,
       8,   105,   119,   161,   162,   165,   169,   170,   101,    99,
      99,    29,    43,    44,    45,    46,    47,    48,    50,    53,
      54,    55,    92,    93,    94,    95,    96,    97,    98,   100,
     127,   144,   166,   168,    99,   165,    34,   120,    99,    99,
      99,    24,    51,    52,   119,   165,   119,    35,    45,   130,
      99,    99,    37,    99,    99,    99,    35,    99,    99,   120,
      99,   119,   102,   165,    29,    35,    29,    35,    29,    35,
      29,    35,    56,    58,    60,    62,   149,   119,    29,   166,
      69,    77,    69,    77,    69,    77,    69,    77,    69,    77,
      29,    29,    82,    83,   166,   100,   102,   105,   105,   105,
     105,   104,   105,   100,   104,   104,   100,   102,   145,   170,
      27,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   170,    43,
      44,    45,    46,    47,    48,    50,    53,    54,    55,    92,
      93,    94,    95,    96,    97,    98,   100,   127,    99,   122,
      99,    76,   145,   146,   148,   146,   146,    23,    45,    99,
      29,   165,     9,    87,    99,   112,   113,   114,   115,   165,
     124,    99,   122,   138,   139,   140,    99,   100,   164,   165,
     164,   165,   164,   165,   164,   165,    23,    29,   157,    27,
      57,    59,    61,    63,    71,    72,    73,    74,    78,    79,
      80,    81,   153,   154,   164,   153,   164,   164,   100,   127,
     102,   170,   165,   165,   165,   165,   165,   162,   165,   165,
     166,   100,   127,   165,   127,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   170,    99,   123,   141,   142,   143,    16,   138,
      99,    99,    99,   165,   165,   138,    71,    73,    80,    81,
     159,   164,    99,   117,   119,   115,    99,    90,   140,   138,
     102,   170,    29,    29,    29,    29,   164,   165,   157,   157,
     157,   157,   164,   164,   157,   164,   164,   170,   102,   127,
     100,   127,   102,   127,    16,    88,   134,   143,    99,   122,
      99,    15,    35,    89,   164,   167,   168,   167,   167,   167,
      99,   120,    99,     9,   127,   164,   164,   164,   164,   164,
     164,   164,   164,    29,    71,   154,   127,   102,   170,   102,
      99,   122,    99,   141,   138,    35,   165,    99,    43,    44,
      46,    47,    48,    50,    53,    54,    55,    92,    93,    94,
      95,    96,    97,    98,    29,    99,   117,   102,    71,   102,
     127,   141,   134,    89,   165,    99,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,    71,   138,    99,   164,   102,   134,    99,    99,
     138,   167,   138,    89,    89,    99,    99
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   106,   107,   108,   108,   109,   109,   109,   109,   109,
     109,   110,   111,   112,   113,   113,   114,   114,   115,   115,
     115,   116,   116,   117,   117,   117,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   120,   120,   120,   121,   121,   122,   122,
     123,   123,   123,   123,   123,   123,   124,   124,   124,   125,
     125,   125,   125,   125,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   127,   127,   128,   128,   128,
     128,   128,   129,   129,   129,   129,   130,   130,   131,   131,
     131,   132,   132,   133,   133,   133,   133,   134,   134,   135,
     135,   135,   135,   136,   136,   137,   138,   138,   139,   139,
     140,   140,   141,   141,   142,   142,   143,   143,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   145,   146,   146,   147,   147,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   149,
     149,   149,   149,   150,   150,   151,   151,   151,   151,   152,
     152,   152,   152,   152,   153,   153,   153,   153,   153,   154,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     156,   156,   156,   156,   156,   156,   156,   156,   157,   157,
     158,   158,   159,   159,   159,   159,   159,   160,   160,   160,
     161,   161,   161,   162,   162,   162,   162,   162,   163,   163,
     164,   164,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   168,   168,
     169,   169,   170,   170
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
       1,     1,     1,     0,     1,     2,     1,     3,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     5,     3,     3,     2,
       2,     3,     3,     3,     2,     0,     2,     4,     3,     5,
       2,     4,     6,     5,     7,     0,     1,     2,     2,     5,
       4,     3,     5,     5,     5,     5,     1,     1,     3,     3,
       3,     4,     6,     6,     8,     7,     9,     0,     2,     7,
      11,    12,     9,     4,     6,     2,     0,     1,     1,     2,
       1,     1,     0,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     2,     3,     5,     6,     8,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       4,     6,     4,     6,     4,     6,     4,     6,     5,     1,
       1,     1,     1,     2,     2,     1,     2,     4,     6,     2,
       4,     4,     1,     1,     1,     1,     5,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       6,     6,     6,     6,     6,     4,     4,     4,     1,     1,
       2,     4,     1,     2,     5,     2,     2,     3,     3,     3,
       1,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     4,     4,
       0,     1,     1,     3
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
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2325 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2331 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2337 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2343 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2349 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 202 "engines/director/lingo/lingo-gr.y"
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
#line 208 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->_compiler->_assemblyAST = new ScriptNode((yyvsp[0].nodelist)); }
#line 2625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 210 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 216 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2646 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 223 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2652 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* scriptpart: tENDCLAUSE endargdef '\n'  */
#line 228 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2658 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 257 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2664 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* factory: tFACTORY ID '\n' methodlist  */
#line 261 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2670 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 263 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2676 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* methodlist: %empty  */
#line 265 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2682 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlistline  */
#line 269 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2693 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 275 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2703 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18: /* methodlistline: '\n'  */
#line 282 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* methodlistline: tENDCLAUSE endargdef '\n'  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2715 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 289 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2724 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* handler: tON ID idlist '\n' stmtlist  */
#line 293 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2731 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* endargdef: ID  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2737 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* endargdef: endargdef ',' ID  */
#line 299 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2743 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* CMDID: tABBREVIATED  */
#line 307 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* CMDID: tABBREV  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2755 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* CMDID: tABBR  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2761 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* CMDID: tAFTER  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2767 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* CMDID: tBEFORE  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tCAST  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2779 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tCHAR  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2785 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tCHARS  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2791 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tDATE  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2797 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tDELETE  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 2803 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tDOWN  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2809 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tFIELD  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2815 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tFRAME  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2821 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tHILITE  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 2827 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tIN  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2833 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tINTERSECTS  */
#line 322 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2839 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tINTO  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2845 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tITEM  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2851 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tITEMS  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tLAST  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tLINE  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2869 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tLINES  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2875 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tLONG  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2881 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tMENU  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tMENUITEM  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tMENUITEMS  */
#line 332 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tMOVIE  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2905 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tNEXT  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tNUMBER  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tOF  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tPREVIOUS  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2929 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tREPEAT  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tSCRIPT  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2941 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tASSERTERROR  */
#line 340 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tSHORT  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2953 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tSOUND  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tSPRITE  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tTHE  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 2971 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tTIME  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 2977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tTO  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tWHILE  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 2989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tWINDOW  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tWITH  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 3001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tWITHIN  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 3007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tWORD  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 3013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tWORDS  */
#line 352 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 3019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* ID: tELSE  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 3025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* ID: tENDCLAUSE  */
#line 357 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); delete (yyvsp[0].s); }
#line 3031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* ID: tEXIT  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 3037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* ID: tFACTORY  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 3043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* ID: tGLOBAL  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 3049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* ID: tGO  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 3055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* ID: tIF  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 3061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tINSTANCE  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 3067 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tMACRO  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3073 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tMETHOD  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tON  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tOPEN  */
#line 367 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tPLAY  */
#line 368 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3097 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tPROPERTY  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3103 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tPUT  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3109 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tRETURN  */
#line 371 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("return"); }
#line 3115 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tSET  */
#line 372 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3121 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tTELL  */
#line 373 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3127 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tTHEN  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* idlist: %empty  */
#line 377 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3139 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* nonemptyidlist: ID  */
#line 382 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 386 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3156 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* stmt: tENDIF '\n'  */
#line 397 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3162 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* proc: CMDID cmdargs '\n'  */
#line 413 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3168 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* proc: tPUT cmdargs '\n'  */
#line 414 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3174 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* proc: tGO cmdargs '\n'  */
#line 415 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3180 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* proc: tGO frameargs '\n'  */
#line 416 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3186 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* proc: tPLAY cmdargs '\n'  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3192 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* proc: tPLAY frameargs '\n'  */
#line 418 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3198 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* proc: tOPEN cmdargs '\n'  */
#line 419 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3204 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: tOPEN expr tWITH expr '\n'  */
#line 420 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3214 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: tNEXT tREPEAT '\n'  */
#line 425 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tEXIT tREPEAT '\n'  */
#line 426 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3226 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tEXIT '\n'  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3232 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tRETURN '\n'  */
#line 428 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ReturnNode(nullptr); }
#line 3238 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tRETURN expr '\n'  */
#line 429 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ReturnNode((yyvsp[-1].node)); }
#line 3244 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* proc: tDELETE chunk '\n'  */
#line 430 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3250 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proc: tHILITE chunk '\n'  */
#line 431 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3256 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tASSERTERROR stmtoneliner  */
#line 432 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3262 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* cmdargs: %empty  */
#line 435 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* cmdargs: expr trailingcomma  */
#line 438 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3280 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 443 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd arg, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3289 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 447 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3300 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 453 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3310 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* cmdargs: '(' ')'  */
#line 458 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3318 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* cmdargs: '(' expr ',' ')'  */
#line 461 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3328 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 466 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3337 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* cmdargs: '(' var expr_nounarymath trailingcomma ')'  */
#line 470 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3348 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* cmdargs: '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 476 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* frameargs: tFRAME expr  */
#line 489 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3368 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* frameargs: tMOVIE expr  */
#line 494 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 500 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3390 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* frameargs: expr tOF tMOVIE expr  */
#line 506 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3401 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* frameargs: tFRAME expr expr_nounarymath  */
#line 512 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3412 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 520 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 521 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3424 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 522 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3430 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* asgn: tSET varorthe to expr '\n'  */
#line 523 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3436 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* definevars: tGLOBAL idlist '\n'  */
#line 528 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3442 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* definevars: tPROPERTY idlist '\n'  */
#line 529 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3448 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* definevars: tINSTANCE idlist '\n'  */
#line 530 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3454 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* ifstmt: tIF expr tTHEN stmt  */
#line 533 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3463 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 537 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 3470 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 541 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3481 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 547 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 3490 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 551 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3499 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 555 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 3506 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* endif: %empty  */
#line 559 "engines/director/lingo/lingo-gr.y"
                        {
		LingoCompiler *compiler = g_lingo->_compiler;
		warning("LingoCompiler::parse: no end if at line %d col %d in %s id: %d",
			compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
			compiler->_assemblyContext->_id);

		}
#line 3518 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 568 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 570 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3532 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 572 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3539 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 574 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3546 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* tell: tTELL expr tTO stmtoneliner  */
#line 578 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3555 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 582 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3562 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* when: tWHEN '\n'  */
#line 586 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WhenNode((yyvsp[-1].w).eventName, (yyvsp[-1].w).stmt); }
#line 3568 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* stmtlist: %empty  */
#line 588 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3574 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* nonemptystmtlist: stmtlistline  */
#line 593 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3585 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 599 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3595 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* stmtlistline: '\n'  */
#line 606 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3601 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* stmtlist_insideif: %empty  */
#line 610 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 3607 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 615 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 621 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3628 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* stmtlistline_insideif: '\n'  */
#line 628 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3634 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* simpleexpr_nounarymath: tINT  */
#line 635 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3640 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* simpleexpr_nounarymath: tFLOAT  */
#line 636 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3646 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* simpleexpr_nounarymath: tSYMBOL  */
#line 637 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3652 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* simpleexpr_nounarymath: tSTRING  */
#line 638 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3658 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 639 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3664 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 3670 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 641 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 3676 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath trailingcomma ')'  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.node) = new FuncNode((yyvsp[-5].s), args); }
#line 3687 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 648 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.node) = new FuncNode((yyvsp[-7].s), (yyvsp[-2].nodelist)); }
#line 3697 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 653 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3703 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* var: ID  */
#line 661 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* chunk: tFIELD refargs  */
#line 671 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 3715 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* chunk: tCAST refargs  */
#line 672 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 3721 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* chunk: tCHAR expr tOF simpleexpr  */
#line 673 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 675 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* chunk: tWORD expr tOF simpleexpr  */
#line 677 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3742 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 679 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* chunk: tITEM expr tOF simpleexpr  */
#line 681 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3756 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 683 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3763 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* chunk: tLINE expr tOF simpleexpr  */
#line 685 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3770 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 687 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3777 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 689 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 3783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* chunktype: tCHAR  */
#line 692 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 3789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* chunktype: tWORD  */
#line 693 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 3795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* chunktype: tITEM  */
#line 694 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 3801 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* chunktype: tLINE  */
#line 695 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 3807 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* object: tSCRIPT refargs  */
#line 698 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 3813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* object: tWINDOW refargs  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 3819 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* refargs: simpleexpr  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3829 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* refargs: '(' ')'  */
#line 707 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 3837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* refargs: '(' expr ',' ')'  */
#line 710 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3847 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 715 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3856 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* the: tTHE ID  */
#line 721 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3862 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* the: tTHE ID tOF theobj  */
#line 722 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 221: /* the: tTHE tNUMBER tOF theobj  */
#line 723 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 3874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 226: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 730 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3880 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* theobj: tSOUND simpleexpr  */
#line 731 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3886 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* theobj: tSPRITE simpleexpr  */
#line 732 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3892 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 229: /* menu: tMENU simpleexpr  */
#line 735 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3898 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 230: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 737 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 738 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3910 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* thedatetime: tTHE tABBREV tDATE  */
#line 739 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* thedatetime: tTHE tABBREV tTIME  */
#line 740 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3922 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* thedatetime: tTHE tABBR tDATE  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* thedatetime: tTHE tABBR tTIME  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3934 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* thedatetime: tTHE tLONG tDATE  */
#line 743 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 3940 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* thedatetime: tTHE tLONG tTIME  */
#line 744 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 3946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* thedatetime: tTHE tSHORT tDATE  */
#line 745 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 3952 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* thedatetime: tTHE tSHORT tTIME  */
#line 746 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 3958 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 750 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 3964 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 751 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 3970 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 752 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 3976 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 753 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 3982 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 754 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 3988 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* thenumberof: tTHE tNUMBER tOF tMENUS  */
#line 755 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenus, nullptr); }
#line 3994 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* thenumberof: tTHE tNUMBER tOF tXTRAS  */
#line 756 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfXtras, nullptr); }
#line 4000 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* thenumberof: tTHE tNUMBER tOF tCASTLIBS  */
#line 757 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNumberOfNode(kNumberOfCastlibs, nullptr); }
#line 4006 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* writablethe: tTHE ID  */
#line 762 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 4012 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* writablethe: tTHE ID tOF writabletheobj  */
#line 763 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 4018 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* writabletheobj: tMENU expr_noeq  */
#line 767 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 4024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 768 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 4030 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* writabletheobj: tSOUND expr_noeq  */
#line 769 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 4036 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* writabletheobj: tSPRITE expr_noeq  */
#line 770 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 4042 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* list: '[' exprlist ']'  */
#line 773 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 4048 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* list: '[' ':' ']'  */
#line 774 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 4054 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* list: '[' proplist ']'  */
#line 775 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 4060 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* proplist: proppair  */
#line 781 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4069 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 261: /* proplist: proplist ',' proppair  */
#line 785 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4077 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 262: /* proplist: proplist ',' expr  */
#line 788 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 263: /* proppair: tSYMBOL ':' expr  */
#line 793 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 264: /* proppair: ID ':' expr  */
#line 794 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4097 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 265: /* proppair: tSTRING ':' expr  */
#line 795 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4103 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* proppair: tINT ':' expr  */
#line 796 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new IntNode((yyvsp[-2].i)), (yyvsp[0].node)); }
#line 4109 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* proppair: tFLOAT ':' expr  */
#line 797 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new FloatNode((yyvsp[-2].f)), (yyvsp[0].node)); }
#line 4115 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* unarymath: '+' simpleexpr  */
#line 800 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 4121 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* unarymath: '-' simpleexpr  */
#line 801 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 4127 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* expr: expr '+' expr  */
#line 811 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* expr: expr '-' expr  */
#line 812 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4139 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* expr: expr '*' expr  */
#line 813 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4145 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* expr: expr '/' expr  */
#line 814 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4151 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* expr: expr tMOD expr  */
#line 815 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4157 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* expr: expr '>' expr  */
#line 816 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4163 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* expr: expr '<' expr  */
#line 817 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4169 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* expr: expr tEQ expr  */
#line 818 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4175 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* expr: expr tNEQ expr  */
#line 819 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4181 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 283: /* expr: expr tGE expr  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4187 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 284: /* expr: expr tLE expr  */
#line 821 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4193 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* expr: expr tAND expr  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4199 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* expr: expr tOR expr  */
#line 823 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 287: /* expr: expr '&' expr  */
#line 824 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4211 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 288: /* expr: expr tCONCAT expr  */
#line 825 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4217 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 289: /* expr: expr tCONTAINS expr  */
#line 826 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4223 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 290: /* expr: expr tSTARTS expr  */
#line 827 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4229 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 836 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4235 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 294: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 837 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4241 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 295: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 838 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 296: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 839 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4253 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 840 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 841 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 842 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4271 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 843 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 301: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 844 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4283 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 302: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 845 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4289 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 303: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 846 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 304: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 847 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4301 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 848 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 849 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 307: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 850 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4319 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 308: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 851 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4325 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 309: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 852 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4331 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 312: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 857 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4337 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 313: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 858 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 314: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 859 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4349 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 315: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 860 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4355 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 861 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4361 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 862 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4367 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 863 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4373 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 864 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 865 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4385 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 866 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4391 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 322: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 867 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4397 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 323: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 868 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4403 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 324: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 869 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4409 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 325: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 870 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4415 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 326: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 871 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4421 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 327: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 872 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4427 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 328: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 875 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4433 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 329: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 876 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4439 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 330: /* exprlist: %empty  */
#line 879 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 4445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 332: /* nonemptyexprlist: expr  */
#line 883 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4454 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 333: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 887 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4462 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 4466 "engines/director/lingo/lingo-gr.cpp"

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

#line 892 "engines/director/lingo/lingo-gr.y"


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
