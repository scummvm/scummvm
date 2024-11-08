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
	warning("%s  LINGO: %s at line %d col %d in %s id: %d",
		(g_director->_noFatalLingoError ? "####" : "######################"), s, compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
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


#line 136 "engines/director/lingo/lingo-gr.cpp"

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
  YYSYMBOL_tMEMBER = 14,                   /* tMEMBER  */
  YYSYMBOL_tCASTLIB = 15,                  /* tCASTLIB  */
  YYSYMBOL_tDELETE = 16,                   /* tDELETE  */
  YYSYMBOL_tDOWN = 17,                     /* tDOWN  */
  YYSYMBOL_tELSE = 18,                     /* tELSE  */
  YYSYMBOL_tEXIT = 19,                     /* tEXIT  */
  YYSYMBOL_tFRAME = 20,                    /* tFRAME  */
  YYSYMBOL_tGLOBAL = 21,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 22,                       /* tGO  */
  YYSYMBOL_tHILITE = 23,                   /* tHILITE  */
  YYSYMBOL_tIF = 24,                       /* tIF  */
  YYSYMBOL_tIN = 25,                       /* tIN  */
  YYSYMBOL_tINTO = 26,                     /* tINTO  */
  YYSYMBOL_tMACRO = 27,                    /* tMACRO  */
  YYSYMBOL_tRETURN = 28,                   /* tRETURN  */
  YYSYMBOL_tMOVIE = 29,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 30,                     /* tNEXT  */
  YYSYMBOL_tOF = 31,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 32,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 33,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 34,                   /* tREPEAT  */
  YYSYMBOL_tSET = 35,                      /* tSET  */
  YYSYMBOL_tTHEN = 36,                     /* tTHEN  */
  YYSYMBOL_tTO = 37,                       /* tTO  */
  YYSYMBOL_tWHEN = 38,                     /* tWHEN  */
  YYSYMBOL_tWITH = 39,                     /* tWITH  */
  YYSYMBOL_tWHILE = 40,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 41,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 42,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 43,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 44,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 45,                       /* tGE  */
  YYSYMBOL_tLE = 46,                       /* tLE  */
  YYSYMBOL_tEQ = 47,                       /* tEQ  */
  YYSYMBOL_tNEQ = 48,                      /* tNEQ  */
  YYSYMBOL_tAND = 49,                      /* tAND  */
  YYSYMBOL_tOR = 50,                       /* tOR  */
  YYSYMBOL_tNOT = 51,                      /* tNOT  */
  YYSYMBOL_tMOD = 52,                      /* tMOD  */
  YYSYMBOL_tAFTER = 53,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 54,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 55,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 56,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 57,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 58,                     /* tCHAR  */
  YYSYMBOL_tCHARS = 59,                    /* tCHARS  */
  YYSYMBOL_tITEM = 60,                     /* tITEM  */
  YYSYMBOL_tITEMS = 61,                    /* tITEMS  */
  YYSYMBOL_tLINE = 62,                     /* tLINE  */
  YYSYMBOL_tLINES = 63,                    /* tLINES  */
  YYSYMBOL_tWORD = 64,                     /* tWORD  */
  YYSYMBOL_tWORDS = 65,                    /* tWORDS  */
  YYSYMBOL_tABBREVIATED = 66,              /* tABBREVIATED  */
  YYSYMBOL_tABBREV = 67,                   /* tABBREV  */
  YYSYMBOL_tABBR = 68,                     /* tABBR  */
  YYSYMBOL_tLONG = 69,                     /* tLONG  */
  YYSYMBOL_tSHORT = 70,                    /* tSHORT  */
  YYSYMBOL_tDATE = 71,                     /* tDATE  */
  YYSYMBOL_tLAST = 72,                     /* tLAST  */
  YYSYMBOL_tMENU = 73,                     /* tMENU  */
  YYSYMBOL_tMENUS = 74,                    /* tMENUS  */
  YYSYMBOL_tMENUITEM = 75,                 /* tMENUITEM  */
  YYSYMBOL_tMENUITEMS = 76,                /* tMENUITEMS  */
  YYSYMBOL_tNUMBER = 77,                   /* tNUMBER  */
  YYSYMBOL_tTHE = 78,                      /* tTHE  */
  YYSYMBOL_tTIME = 79,                     /* tTIME  */
  YYSYMBOL_tXTRAS = 80,                    /* tXTRAS  */
  YYSYMBOL_tCASTLIBS = 81,                 /* tCASTLIBS  */
  YYSYMBOL_tSOUND = 82,                    /* tSOUND  */
  YYSYMBOL_tSPRITE = 83,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 84,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 85,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 86,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 87,                 /* tPROPERTY  */
  YYSYMBOL_tON = 88,                       /* tON  */
  YYSYMBOL_tMETHOD = 89,                   /* tMETHOD  */
  YYSYMBOL_tENDIF = 90,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 91,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 92,                  /* tENDTELL  */
  YYSYMBOL_tASSERTERROR = 93,              /* tASSERTERROR  */
  YYSYMBOL_94_ = 94,                       /* '<'  */
  YYSYMBOL_95_ = 95,                       /* '>'  */
  YYSYMBOL_96_ = 96,                       /* '&'  */
  YYSYMBOL_97_ = 97,                       /* '+'  */
  YYSYMBOL_98_ = 98,                       /* '-'  */
  YYSYMBOL_99_ = 99,                       /* '*'  */
  YYSYMBOL_100_ = 100,                     /* '/'  */
  YYSYMBOL_101_n_ = 101,                   /* '\n'  */
  YYSYMBOL_102_ = 102,                     /* ','  */
  YYSYMBOL_103_ = 103,                     /* '('  */
  YYSYMBOL_104_ = 104,                     /* ')'  */
  YYSYMBOL_105_ = 105,                     /* '['  */
  YYSYMBOL_106_ = 106,                     /* ']'  */
  YYSYMBOL_107_ = 107,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 108,                 /* $accept  */
  YYSYMBOL_script = 109,                   /* script  */
  YYSYMBOL_scriptpartlist = 110,           /* scriptpartlist  */
  YYSYMBOL_scriptpart = 111,               /* scriptpart  */
  YYSYMBOL_macro = 112,                    /* macro  */
  YYSYMBOL_factory = 113,                  /* factory  */
  YYSYMBOL_method = 114,                   /* method  */
  YYSYMBOL_methodlist = 115,               /* methodlist  */
  YYSYMBOL_nonemptymethodlist = 116,       /* nonemptymethodlist  */
  YYSYMBOL_methodlistline = 117,           /* methodlistline  */
  YYSYMBOL_handler = 118,                  /* handler  */
  YYSYMBOL_endargdef = 119,                /* endargdef  */
  YYSYMBOL_CMDID = 120,                    /* CMDID  */
  YYSYMBOL_ID = 121,                       /* ID  */
  YYSYMBOL_idlist = 122,                   /* idlist  */
  YYSYMBOL_nonemptyidlist = 123,           /* nonemptyidlist  */
  YYSYMBOL_stmt = 124,                     /* stmt  */
  YYSYMBOL_stmt_insideif = 125,            /* stmt_insideif  */
  YYSYMBOL_stmtoneliner = 126,             /* stmtoneliner  */
  YYSYMBOL_proc = 127,                     /* proc  */
  YYSYMBOL_cmdargs = 128,                  /* cmdargs  */
  YYSYMBOL_trailingcomma = 129,            /* trailingcomma  */
  YYSYMBOL_frameargs = 130,                /* frameargs  */
  YYSYMBOL_asgn = 131,                     /* asgn  */
  YYSYMBOL_to = 132,                       /* to  */
  YYSYMBOL_definevars = 133,               /* definevars  */
  YYSYMBOL_ifstmt = 134,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 135,               /* ifelsestmt  */
  YYSYMBOL_endif = 136,                    /* endif  */
  YYSYMBOL_loop = 137,                     /* loop  */
  YYSYMBOL_tell = 138,                     /* tell  */
  YYSYMBOL_when = 139,                     /* when  */
  YYSYMBOL_stmtlist = 140,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 141,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 142,             /* stmtlistline  */
  YYSYMBOL_stmtlist_insideif = 143,        /* stmtlist_insideif  */
  YYSYMBOL_nonemptystmtlist_insideif = 144, /* nonemptystmtlist_insideif  */
  YYSYMBOL_stmtlistline_insideif = 145,    /* stmtlistline_insideif  */
  YYSYMBOL_simpleexpr_nounarymath = 146,   /* simpleexpr_nounarymath  */
  YYSYMBOL_var = 147,                      /* var  */
  YYSYMBOL_varorchunk = 148,               /* varorchunk  */
  YYSYMBOL_varorthe = 149,                 /* varorthe  */
  YYSYMBOL_chunk = 150,                    /* chunk  */
  YYSYMBOL_chunktype = 151,                /* chunktype  */
  YYSYMBOL_object = 152,                   /* object  */
  YYSYMBOL_refargs = 153,                  /* refargs  */
  YYSYMBOL_the = 154,                      /* the  */
  YYSYMBOL_theobj = 155,                   /* theobj  */
  YYSYMBOL_menu = 156,                     /* menu  */
  YYSYMBOL_thedatetime = 157,              /* thedatetime  */
  YYSYMBOL_thenumberof = 158,              /* thenumberof  */
  YYSYMBOL_inof = 159,                     /* inof  */
  YYSYMBOL_writablethe = 160,              /* writablethe  */
  YYSYMBOL_writabletheobj = 161,           /* writabletheobj  */
  YYSYMBOL_list = 162,                     /* list  */
  YYSYMBOL_proplist = 163,                 /* proplist  */
  YYSYMBOL_proppair = 164,                 /* proppair  */
  YYSYMBOL_unarymath = 165,                /* unarymath  */
  YYSYMBOL_simpleexpr = 166,               /* simpleexpr  */
  YYSYMBOL_expr = 167,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 168,         /* expr_nounarymath  */
  YYSYMBOL_expr_noeq = 169,                /* expr_noeq  */
  YYSYMBOL_sprite = 170,                   /* sprite  */
  YYSYMBOL_exprlist = 171,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 172          /* nonemptyexprlist  */
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
#define YYFINAL  195
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4723

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  108
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  337
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  635

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   348


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
     101,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    96,     2,
     103,   104,    99,    97,   102,    98,     2,   100,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   107,     2,
      94,     2,    95,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   105,     2,   106,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92,    93
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
     350,   351,   352,   353,   354,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   379,   380,   381,   384,   388,
     398,   399,   402,   403,   404,   405,   406,   407,   410,   411,
     412,   415,   416,   417,   418,   419,   420,   421,   422,   427,
     428,   429,   430,   431,   432,   433,   434,   437,   440,   445,
     449,   455,   460,   463,   468,   472,   478,   485,   485,   491,
     496,   502,   508,   514,   522,   523,   524,   525,   528,   528,
     530,   531,   532,   535,   539,   543,   549,   553,   557,   561,
     568,   570,   572,   574,   576,   580,   584,   588,   590,   591,
     595,   601,   608,   609,   612,   613,   617,   623,   630,   631,
     637,   638,   639,   640,   641,   642,   643,   644,   650,   655,
     656,   657,   658,   659,   660,   663,   665,   666,   669,   670,
     673,   674,   675,   676,   677,   679,   681,   683,   685,   687,
     689,   691,   693,   696,   697,   698,   699,   702,   703,   706,
     711,   714,   719,   725,   726,   727,   728,   729,   732,   733,
     734,   735,   736,   739,   741,   742,   743,   744,   745,   746,
     747,   748,   749,   750,   754,   755,   756,   757,   758,   759,
     760,   761,   764,   764,   766,   767,   770,   771,   772,   773,
     774,   777,   778,   779,   785,   789,   792,   797,   798,   799,
     800,   801,   804,   805,   808,   809,   813,   814,   815,   816,
     817,   818,   819,   820,   821,   822,   823,   824,   825,   826,
     827,   828,   829,   830,   831,   838,   839,   840,   841,   842,
     843,   844,   845,   846,   847,   848,   849,   850,   851,   852,
     853,   854,   855,   856,   859,   860,   861,   862,   863,   864,
     865,   866,   867,   868,   869,   870,   871,   872,   873,   874,
     875,   876,   879,   880,   883,   884,   887,   891
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
  "tSCRIPT", "tWINDOW", "tMEMBER", "tCASTLIB", "tDELETE", "tDOWN", "tELSE",
  "tEXIT", "tFRAME", "tGLOBAL", "tGO", "tHILITE", "tIF", "tIN", "tINTO",
  "tMACRO", "tRETURN", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT",
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE",
  "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tCHARS", "tITEM", "tITEMS", "tLINE",
  "tLINES", "tWORD", "tWORDS", "tABBREVIATED", "tABBREV", "tABBR", "tLONG",
  "tSHORT", "tDATE", "tLAST", "tMENU", "tMENUS", "tMENUITEM", "tMENUITEMS",
  "tNUMBER", "tTHE", "tTIME", "tXTRAS", "tCASTLIBS", "tSOUND", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tMETHOD",
  "tENDIF", "tENDREPEAT", "tENDTELL", "tASSERTERROR", "'<'", "'>'", "'&'",
  "'+'", "'-'", "'*'", "'/'", "'\\n'", "','", "'('", "')'", "'['", "']'",
  "':'", "$accept", "script", "scriptpartlist", "scriptpart", "macro",
  "factory", "method", "methodlist", "nonemptymethodlist",
  "methodlistline", "handler", "endargdef", "CMDID", "ID", "idlist",
  "nonemptyidlist", "stmt", "stmt_insideif", "stmtoneliner", "proc",
  "cmdargs", "trailingcomma", "frameargs", "asgn", "to", "definevars",
  "ifstmt", "ifelsestmt", "endif", "loop", "tell", "when", "stmtlist",
  "nonemptystmtlist", "stmtlistline", "stmtlist_insideif",
  "nonemptystmtlist_insideif", "stmtlistline_insideif",
  "simpleexpr_nounarymath", "var", "varorchunk", "varorthe", "chunk",
  "chunktype", "object", "refargs", "the", "theobj", "menu", "thedatetime",
  "thenumberof", "inof", "writablethe", "writabletheobj", "list",
  "proplist", "proppair", "unarymath", "simpleexpr", "expr",
  "expr_nounarymath", "expr_noeq", "sprite", "exprlist",
  "nonemptyexprlist", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-534)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    3033,  -534,  3586,  -534,  -534,  -534,  -534,  -534,  -534,   108,
    -534,   -23,  -534,  3586,  2119,   108,  2221,  -534,  -534,  3586,
    1507,  -534,     6,  -534,  -534,  2323,    32,  3671,  -534,   -59,
    -534,  -534,  3586,  2323,  2119,  3586,  -534,  -534,  -534,  -534,
    -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,
    -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,
    -534,  -534,  -534,  2221,  3586,  3586,   -56,  3925,  -534,    61,
    3033,  -534,  -534,  -534,  -534,  2323,  -534,  -534,  -534,  -534,
    -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,
    -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,
    -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,
    -534,     9,  -534,  -534,  2425,  2425,  2425,  2425,  2221,  2221,
    2221,  2221,     7,   -17,     5,  -534,  -534,    24,    47,  -534,
    -534,  -534,  -534,  2425,  2425,  2425,  2425,  2425,  2425,  2221,
    2221,  2527,  2221,  2221,  2221,  2221,  3756,  2221,  2527,  2527,
    1609,   791,   -12,    39,    62,  -534,  -534,  -534,  -534,  -534,
    -534,  -534,  -534,  -534,  -534,   895,  -534,    66,  2221,  4157,
    3586,  -534,  4357,    73,    79,   997,  3586,  2221,  3586,  -534,
    -534,    11,  -534,  -534,    80,    81,  1099,    82,    83,    88,
    4122,    92,  3586,  -534,  -534,  -534,  -534,    97,  1201,  -534,
    3586,  1711,  -534,  -534,  -534,  -534,  -534,   609,   682,  3974,
    3996,   132,  -534,  -534,  -534,  3586,  -534,  -534,  1303,  4554,
    -534,   -20,    38,    45,    56,    72,   132,    65,    71,  4541,
    -534,  -534,  -534,  2935,  4179,    50,    90,    93,    94,     2,
     -82,   -46,  -534,  4554,    67,   100,  1813,  -534,  -534,   176,
    2221,  2221,  2221,  2221,  2221,  2221,  2221,  2221,  2221,  2221,
    2221,  2221,  2221,  2221,  2221,  2221,  2221,  2221,  -534,  -534,
    4273,  -534,  -534,  4213,  3125,   105,  -534,  -534,  -534,  3841,
    3841,  3841,    29,  4370,   177,  -534,  -534,  2221,     4,  -534,
    2221,  -534,  -534,  -534,  3925,  3217,  -534,   106,  -534,  -534,
    -534,  4192,  2527,  2221,  2527,  2221,  2527,  2221,  2527,  2221,
    -534,  -534,  -534,  -534,    43,  -534,   186,  4610,  -534,  -534,
    -534,  -534,  -534,  -534,  -534,  -534,  -534,  -534,  1405,  2629,
    2527,  2527,  4286,  1915,  -534,  2221,  2221,  2221,  2221,  -534,
    2221,  2731,  -534,  -534,  2221,  -534,  2935,   115,  2221,   -11,
     -11,   -11,   -11,   363,   363,  -534,     1,   -11,   -11,   -11,
     -11,     1,   -19,   -19,  -534,  -534,   115,  2221,  2221,  2221,
    2221,  2221,  2221,  2221,  2221,  2221,  2221,  2221,  2221,  2221,
    2221,  2221,  2221,  2221,  2221,  -534,  3493,   201,  3217,     7,
    -534,   125,  -534,   126,   134,  2221,  2221,  3217,  2833,  4383,
    3586,  3586,  -534,  -534,  -534,     4,  -534,  4440,  -534,  -534,
    -534,   133,  3217,  -534,  3217,  2017,  -534,  4009,  -534,  4031,
    -534,  4066,  -534,  4088,  -534,  -534,  2527,  2221,    43,    43,
      43,    43,  2527,  -534,  2527,    43,  -534,  -534,  2527,  2527,
    -534,  -534,  -534,  -534,  -534,  -534,  2221,   135,  -534,   115,
    4554,  4554,  4554,  4554,  4554,  -534,  4554,  4554,  4299,  2221,
     137,  4554,  -534,   -11,   -11,   -11,   -11,   363,   363,  -534,
       1,   -11,   -11,   -11,   -11,     1,   -19,   -19,  -534,  -534,
     115,  -534,  -534,   -13,  3493,  -534,  3309,  -534,  -534,  -534,
    -534,  4453,   539,   145,  2221,  2221,  2221,  2221,  -534,  -534,
    -534,    19,  3586,  -534,  -534,   136,  -534,   229,  -534,   115,
    2527,  2527,  2527,  2527,  -534,  4554,  2527,  2527,  2527,  2527,
    -534,   211,   170,  -534,  -534,   115,  -534,   160,  2221,   161,
    -534,  -534,  3401,   165,  -534,  -534,  3493,  -534,  3217,   230,
    2221,   167,  -534,   525,  -534,  4101,   525,   525,  -534,   169,
    -534,  3586,   168,  -534,  -534,  -534,  -534,  -534,  -534,  -534,
    -534,   198,  2527,  -534,   171,  -534,   115,  -534,  3493,  -534,
    -534,   183,   185,  2221,  4466,  -534,  2221,  2221,  2221,  2221,
    2221,  2221,  2221,  2221,  2221,  2221,  2221,  2221,  2221,  2221,
    2221,  2221,   209,  3217,    51,  -534,  2527,  -534,   179,   183,
    -534,   191,  4523,  3217,   124,   124,   124,  4623,  4623,  -534,
      31,   124,   124,   124,   124,    31,    -5,    -5,  -534,  -534,
    2221,  -534,  -534,  -534,  -534,  -534,  -534,  3217,   195,   525,
     196,   192,   199,  -534,  -534
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    26,    23,    32,    39,    61,    70,    51,    33,    37,
      38,     0,    40,    95,   127,    41,     0,    42,    44,     0,
       0,    55,    56,    58,    59,   127,    60,     0,    68,     0,
      71,    69,     0,   127,   127,    95,    30,    31,    34,    35,
      45,    46,    48,    49,    73,    74,    27,    28,    29,    50,
      63,    36,    47,    52,    53,    54,    57,    66,    67,    64,
      65,    43,    72,     0,    95,     0,     0,    62,     5,     0,
       2,     3,     6,     7,     8,   127,     9,   100,   102,   108,
     109,   110,   103,   104,   105,   106,   107,    77,    37,    76,
      78,    80,    81,    41,    82,    84,    91,    56,    90,    60,
      92,    94,    79,    87,    88,    83,    93,    89,    86,    85,
      62,     0,    75,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   121,    98,     0,    96,   180,
     181,   183,   182,    32,    39,    61,    70,    51,    33,    40,
      55,     0,    34,    45,    48,    73,    66,    65,     0,     0,
       0,   334,   195,     0,     0,   274,   190,   191,   192,   193,
     226,   227,   194,   275,   276,   137,   277,     0,     0,     0,
      95,   122,     0,     0,     0,   137,     0,     0,    66,   195,
     198,     0,   199,   167,     0,     0,   137,     0,     0,     0,
       0,     0,    95,   101,   126,     1,     4,     0,   137,    10,
       0,     0,   201,   219,   200,   202,   203,     0,     0,     0,
       0,     0,   124,   120,   150,    97,   217,   218,   139,   140,
     184,    27,    28,    29,    50,    63,    47,    57,   223,     0,
     272,   273,   132,   190,     0,   180,   181,   183,   182,     0,
     195,     0,   264,   336,     0,   335,     0,   113,   114,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   138,   128,   295,
     137,   296,   125,     0,     0,     0,   123,   119,   112,    44,
      30,    31,     0,     0,   254,   148,   149,     0,    14,   117,
      71,   115,   116,   152,     0,   168,   151,     0,   111,    25,
     220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,   215,   216,   214,     0,    99,    58,   143,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,     0,     0,
       0,     0,   137,     0,   189,     0,     0,     0,     0,   262,
       0,     0,   263,   261,     0,   185,   190,   137,     0,   287,
     288,   285,   286,   289,   290,   282,   292,   293,   294,   284,
     283,   291,   278,   279,   280,   281,   137,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   138,   130,   174,   153,   168,    66,
     196,     0,   197,     0,     0,     0,     0,   168,     0,     0,
      23,     0,    18,    19,    12,    15,    16,     0,   165,   172,
     173,     0,   169,   170,   168,     0,   204,     0,   208,     0,
     210,     0,   206,     0,   252,   253,     0,     0,    35,    46,
      49,    74,    52,   249,    53,    54,   250,   251,    64,    65,
     225,   229,   228,   224,   332,   333,   138,     0,   133,   137,
     270,   271,   269,   267,   268,   265,   266,   337,   137,   138,
       0,   142,   129,   306,   307,   304,   305,   308,   309,   301,
     311,   312,   313,   303,   302,   310,   297,   298,   299,   300,
     137,   178,   179,   159,   175,   176,     0,    11,   144,   145,
     146,     0,     0,     0,    52,    53,    64,    65,   255,   256,
     147,     0,    95,    17,   118,     0,   171,    22,   221,   137,
       0,     0,     0,     0,   212,   141,     0,     0,     0,     0,
     233,     0,     0,   231,   232,   137,   135,     0,   138,     0,
     186,   131,     0,     0,   154,   177,   174,   155,   168,     0,
       0,     0,   314,   257,   315,     0,   259,   260,    20,     0,
     166,    23,     0,   205,   209,   211,   207,   244,   246,   247,
     245,     0,     0,   248,     0,   134,   137,   187,   174,   157,
     160,   159,     0,     0,     0,   161,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   168,     0,   222,     0,   136,     0,   159,
     156,     0,     0,   168,   324,   325,   323,   326,   327,   320,
     329,   330,   331,   322,   321,   328,   316,   317,   318,   319,
       0,    13,    21,   230,   188,   158,   164,   168,     0,   258,
       0,     0,     0,   162,   163
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -534,  -534,  -534,   226,  -534,  -534,  -534,  -534,  -534,  -103,
    -534,  -396,     0,    -1,   -28,  -534,     3,  -380,   -65,  -534,
     -10,  -240,   270,  -534,  -534,  -534,  -534,  -534,  -533,  -534,
    -534,  -534,  -210,  -534,  -107,  -519,  -534,  -178,  -136,   -18,
    -134,  -534,    37,  -534,  -534,    96,  -534,   -16,  -208,  -534,
    -534,  -270,  -534,  -534,  -534,  -534,   -22,  -534,   174,    -6,
    -198,   -69,  -143,  -534,  -230
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    69,    70,    71,    72,    73,   403,   404,   405,   406,
      74,   111,   112,   152,   127,   128,   410,    77,    78,    79,
     153,   268,   154,    80,   287,    81,    82,    83,   534,    84,
      85,    86,   411,   412,   413,   483,   484,   485,   155,   156,
     391,   181,   157,   314,   158,   202,   159,   440,   441,   160,
     161,   426,   182,   498,   162,   241,   242,   163,   164,   243,
     270,   543,   166,   244,   245
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      75,   113,   194,    76,   501,   532,   482,   189,   165,   180,
     169,   124,   126,   400,   172,   174,   347,   571,   170,   175,
     317,   246,   271,   185,   187,   340,   179,   186,   165,   269,
     385,   184,   271,   256,   126,   332,   191,   366,   600,   269,
     173,   256,   183,   271,   257,   193,   123,   581,   285,   599,
     269,   318,   167,   256,   395,   271,   341,   190,   286,   319,
     342,   195,   269,   126,   192,   197,   625,    75,   424,   198,
      75,   176,   177,    76,   425,   271,   396,   533,   125,   211,
     265,   266,   269,   581,   212,   262,   263,   264,   265,   266,
     271,   246,   447,   401,   590,   591,   328,   269,   263,   264,
     265,   266,   329,   449,   482,   402,   213,   460,   339,   320,
     199,   200,   207,   208,   209,   210,   322,   321,   114,   115,
     548,   200,   116,   117,   323,   214,   462,   324,   588,   589,
     590,   591,   233,   218,   219,   325,   207,   208,   209,   210,
     247,   229,   275,   326,   234,   228,   393,   394,   458,   215,
     240,   327,   622,   200,   480,   594,   482,   335,   516,   517,
     518,   519,   273,   248,   297,   522,   118,   272,   119,   126,
     120,   283,   121,   343,   277,   282,   581,   284,   487,   582,
     278,   288,   289,   291,   292,   509,   122,   493,   482,   293,
     310,   126,   311,   296,   312,   301,   313,   336,   298,   299,
     337,   338,   344,   271,   507,   348,   388,   414,   398,   527,
     269,   204,   205,   206,   315,   427,   525,   459,   529,   486,
     587,   588,   589,   590,   591,   505,   488,   489,   346,   408,
     204,   216,   217,   205,   206,   490,   541,   550,   551,   526,
     531,   530,   561,   562,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   390,   390,   390,   565,   567,   570,   573,   575,   552,
     593,   596,   595,   533,    75,   597,   601,   387,   179,   179,
     179,   399,   620,   624,   407,   564,   631,   632,   203,   203,
     203,   203,   626,   633,    75,    75,   196,   417,   566,   419,
     634,   421,   503,   423,   188,   506,   535,   203,   203,   203,
     203,   203,   203,   443,   563,   220,   392,   392,   392,   455,
       0,     0,   230,   231,     0,     0,   598,     0,   572,   450,
     451,   452,   453,     0,   454,   456,     0,     0,   457,     0,
     240,     0,   461,     0,     0,     0,     0,     0,     0,     0,
       0,   544,   544,   544,   544,     0,     0,     0,     0,     0,
       0,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,     0,     0,
       0,     0,     0,   621,     0,     0,    75,     0,    75,   491,
     492,     0,     0,   628,     0,     0,     0,    75,     0,   113,
     502,     0,     0,     0,     0,     0,     0,     0,   250,   251,
     252,   253,    75,     0,    75,   256,     0,   630,   257,   258,
     259,   515,     0,     0,     0,     0,   545,   546,   547,     0,
       0,     0,     0,   544,   544,   544,   544,   544,   544,   544,
     544,   544,   544,   544,   544,   544,   544,   544,   544,     0,
       0,     0,     0,   457,     0,     0,     0,   260,   261,   262,
     263,   264,   265,   266,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   549,     0,   416,   544,   418,     0,
     420,     0,   422,     0,    75,     0,    75,     0,     0,   537,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   126,   442,   442,   444,   445,     0,   604,   605,   606,
     607,   608,   609,   610,   611,   612,   613,   614,   615,   616,
     617,   618,   619,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,     0,   574,   569,    75,     0,    75,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     113,   629,     0,     0,     0,     0,   539,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   602,    75,     0,
     576,   577,   499,   578,   579,   580,   540,   581,     0,     0,
     582,   583,   584,     0,   250,   251,   252,   253,   254,   255,
       0,   256,     0,    75,   257,   258,   259,     0,     0,     0,
     514,     0,     0,    75,     0,     0,   520,     0,   521,     0,
       0,     0,   523,   524,     0,     0,     0,     0,     0,   585,
     586,   587,   588,   589,   590,   591,     0,    75,     0,     0,
       0,     0,     0,   260,   261,   262,   263,   264,   265,   266,
     302,     0,     0,     0,     0,     0,   303,     0,     0,     0,
       0,     0,     0,     0,   250,   251,   252,   253,   254,   255,
       0,   256,     0,     0,   257,   258,   259,     0,   542,   542,
     542,   542,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   553,   554,   555,   556,     0,     0,
     557,   558,   559,   560,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   260,   261,   262,   263,   264,   265,   266,
       0,     0,     0,   304,     0,     0,     0,     0,     0,   305,
       0,     0,     0,     0,     0,     0,     0,   250,   251,   252,
     253,   254,   255,     0,   256,     0,   520,   257,   258,   259,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     542,   542,   542,   542,   542,   542,   542,   542,   542,   542,
     542,   542,   542,   542,   542,   542,     0,     0,     0,     0,
     623,     0,     0,     0,     0,     0,   260,   261,   262,   263,
     264,   265,   266,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   542,   235,   236,     1,   237,   238,
      87,   133,   134,   135,   136,   137,   138,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   141,     0,    36,    37,     0,     0,     0,   142,
      39,   143,    41,   144,    43,   145,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   146,
      58,     0,     0,    59,   147,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   148,   149,
       0,     0,     0,     0,   168,     0,   151,     0,   239,   129,
     130,     1,   131,   132,    87,   133,   134,   135,   136,   137,
     138,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,   249,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
     250,   251,   252,   253,   254,   255,   141,   256,    36,    37,
     257,   258,   259,   142,    39,   143,    41,   144,    43,   145,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   146,    58,     0,     0,    59,   147,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,   260,
     261,   262,   263,   264,   265,   266,     0,   267,   168,     0,
     151,   129,   130,     1,   131,   132,    87,   133,   134,   135,
     136,   137,   138,    88,    10,    89,    90,    12,    91,    92,
      93,    94,    17,   279,    95,    96,    21,    97,    23,    24,
      98,    99,   100,   101,    28,     0,    30,    31,   102,   103,
     104,   105,   250,   251,   252,   253,   254,   255,   141,   256,
     280,   281,   257,   258,   259,   142,    39,   143,    41,   144,
      43,   145,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,   146,    58,     0,     0,    59,
     147,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,   260,   261,   262,   263,   264,   265,   266,     0,   267,
     168,     0,   151,   129,   130,     1,   131,   132,    87,   133,
     134,   135,   136,   137,   138,    88,    10,    89,    90,    12,
      91,    92,    93,    94,    17,    18,    95,    96,    21,    97,
      23,    24,    98,    99,   100,   101,    28,     0,   290,    31,
     102,   103,   104,   105,   250,   251,   252,   253,   254,   255,
     141,   256,    36,    37,   257,   258,   259,   142,    39,   143,
      41,   144,    43,   145,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,   146,    58,     0,
       0,    59,   147,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,   260,   261,   262,   263,   264,   265,   266,
       0,   267,   168,     0,   151,   129,   130,     1,   131,   132,
      87,   133,   134,   135,   136,   137,   138,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,   250,   251,   252,   253,
     254,   255,   141,   256,    36,    37,   257,   258,   259,   142,
      39,   143,    41,   144,    43,   145,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   146,
      58,     0,     0,    59,   147,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,   260,   261,   262,   263,   264,
     265,   266,     0,   267,   168,     0,   151,   129,   130,     1,
     131,   132,    87,   133,   134,   135,   136,   137,   138,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,   316,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,   250,   251,
     252,   253,   254,   255,   141,   256,    36,    37,   257,   258,
     259,   142,    39,   143,    41,   144,    43,   145,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   146,    58,     0,     0,    59,   147,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,   260,   261,   262,
     263,   264,   265,   266,     0,     0,   168,     0,   151,   129,
     130,     1,   131,   132,    87,   133,   134,   135,   136,   137,
     138,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   141,     0,    36,    37,
       0,     0,     0,   142,   428,   143,   429,   144,   430,   145,
     431,    46,    47,    48,    49,    50,    51,    52,   432,   433,
     434,   435,    56,   146,    58,   436,   437,   438,   439,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,   148,   149,     0,     0,     0,     0,   168,     0,
     151,   129,   130,     1,   131,   132,    87,   133,   134,   135,
     136,   137,   138,    88,    10,    89,    90,    12,    91,    92,
      93,    94,    17,    18,    95,    96,    21,    97,    23,    24,
      98,    99,   100,   101,    28,     0,    30,    31,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   141,     0,
      36,    37,     0,     0,     0,   142,    39,   143,    41,   144,
      43,   145,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,   146,    58,     0,     0,    59,
     147,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,     0,     0,     0,   148,   149,     0,     0,   171,     0,
     168,     0,   151,   129,   130,     1,   131,   132,    87,   133,
     134,   135,   136,   137,   138,    88,    10,    89,    90,    12,
      91,    92,    93,    94,    17,    18,    95,    96,    21,    97,
      23,    24,    98,    99,   100,   101,    28,     0,    30,    31,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     141,     0,    36,    37,     0,     0,     0,   142,    39,   143,
      41,   144,    43,   145,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,   146,    58,     0,
       0,    59,   147,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,     0,     0,     0,   148,   149,     0,     0,
       0,     0,   168,   232,   151,   129,   130,     1,   131,   132,
      87,   133,   134,   135,   136,   137,   138,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   141,     0,    36,    37,     0,     0,     0,   142,
      39,   143,    41,   144,    43,   145,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   146,
      58,     0,     0,    59,   147,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   148,   149,
       0,     0,     0,     0,   168,   300,   151,   129,   130,     1,
     131,   132,    87,   133,   134,   135,   136,   137,   138,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,    23,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   141,     0,    36,    37,     0,     0,
       0,   142,    39,   143,    41,   144,    43,   145,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   146,    58,     0,     0,    59,   147,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,     0,     0,     0,
     148,   149,     0,     0,     0,     0,   168,   345,   151,   129,
     130,     1,   131,   132,    87,   133,   134,   135,   136,   137,
     138,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   141,     0,    36,    37,
       0,     0,     0,   142,    39,   143,    41,   144,    43,   145,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   146,    58,     0,     0,    59,   147,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,   148,   149,     0,     0,     0,     0,   168,   448,
     151,   129,   130,     1,   131,   132,    87,   133,   134,   135,
     136,   137,   138,    88,    10,    89,    90,    12,    91,    92,
      93,    94,    17,    18,    95,    96,    21,    97,    23,    24,
      98,    99,   100,   101,    28,     0,    30,    31,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   141,     0,
      36,    37,     0,     0,     0,   142,    39,   143,    41,   144,
      43,   145,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,   146,    58,     0,     0,    59,
     147,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,     0,     0,     0,   148,   149,     0,     0,     0,     0,
     168,   508,   151,   129,   130,     1,   131,   132,    87,   133,
     134,   135,   136,   137,   138,    88,    10,    89,    90,   139,
      91,    92,    93,    94,    17,    18,    95,    96,   140,    97,
      23,    24,    98,    99,   100,   101,    28,     0,    30,    31,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     141,     0,    36,    37,     0,     0,     0,   142,    39,   143,
      41,   144,    43,   145,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,   146,    58,     0,
       0,    59,   147,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,     0,     0,     0,   148,   149,     0,     0,
       0,     0,   150,     0,   151,   129,   130,     1,   131,   132,
      87,   133,   134,   135,   136,   137,   138,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   141,     0,    36,    37,     0,     0,     0,   142,
      39,   143,    41,   144,    43,   145,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   146,
      58,     0,     0,    59,   147,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   148,   149,
       0,     0,     0,     0,   168,     0,   151,   129,   130,     1,
     131,   132,    87,   133,   134,   135,   136,   137,   138,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,    23,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   141,     0,    36,    37,     0,     0,
       0,   142,    39,   143,    41,   144,    43,   145,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   146,    58,     0,     0,    59,   147,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,     0,     0,     0,
     148,   149,     0,     0,     0,     0,   150,     0,   151,   129,
     130,     1,   131,   132,    87,   133,   134,   135,   136,   137,
     138,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   141,     0,    36,    37,
       0,     0,     0,   142,    39,   143,    41,   144,    43,   145,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   146,    58,     0,     0,    59,    60,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,   148,   149,     0,     0,     0,     0,   201,     0,
     151,   129,   130,     1,   131,   132,    87,   133,   134,   135,
     136,   137,   138,    88,    10,    89,    90,    12,    91,    92,
      93,    94,    17,    18,    95,    96,    21,    97,    23,    24,
      98,    99,   100,   101,    28,     0,    30,    31,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   141,     0,
      36,    37,     0,     0,     0,   142,    39,   143,    41,   144,
      43,   145,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,   146,    58,     0,     0,    59,
      60,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,     0,     0,     0,   148,   149,     0,     0,     0,     0,
     168,     0,   151,   129,   130,     1,   131,   132,    87,   133,
     134,   135,   136,   137,   138,    88,    10,    89,    90,    12,
      91,    92,    93,    94,    17,    18,    95,    96,    21,    97,
      23,    24,    98,    99,   100,   101,    28,     0,    30,    31,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     141,     0,    36,    37,     0,     0,     0,   142,    39,   143,
      41,   144,    43,   145,    45,    46,    47,    48,    49,    50,
      51,    52,   432,     0,   434,    55,    56,   146,    58,     0,
       0,   438,   439,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,     0,     0,     0,   148,   149,     0,     0,
       0,     0,   168,     0,   151,   235,   236,     1,   237,   238,
      87,   133,   134,   135,   136,   137,   138,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   141,     0,    36,    37,     0,     0,     0,   142,
      39,   143,    41,   144,    43,   145,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   146,
      58,     0,     0,    59,   147,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   148,   149,
       0,     0,     0,     0,   168,     0,   151,   129,   130,     1,
     131,   132,    87,   133,   134,   135,   136,   137,   138,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,    23,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   141,     0,    36,    37,     0,     0,
       0,   142,    39,   143,    41,   144,    43,   145,    45,    46,
      47,    48,    49,    50,    51,    52,   494,     0,   495,    55,
      56,   146,    58,     0,     0,   496,   497,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,     0,     0,     0,
     148,   149,     0,     0,     0,     0,   168,     0,   151,   129,
     130,     1,   131,   132,    87,   133,   134,   135,   136,   137,
     138,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   141,     0,    36,    37,
       0,     0,     0,   142,    39,   143,    41,   144,    43,   145,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   146,    58,     0,     0,    59,   147,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   168,     1,
     151,     0,     2,     3,     4,     5,     6,     7,     8,     9,
      10,     0,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,     0,     0,     0,    36,    37,     0,     0,
       0,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,    57,    58,     0,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,     0,     0,    67,     0,     0,     0,
       0,     1,     0,     0,    68,     3,     4,     5,     6,     7,
       8,     9,    10,     0,    11,    12,    13,    14,    15,    16,
      17,    18,     0,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,    33,    34,    35,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,    57,    58,     0,     0,    59,    60,    61,
      62,    63,    64,     0,     0,    66,     0,     0,    67,     0,
       0,     0,     0,     1,     0,     0,   386,     3,     4,     5,
       6,     7,     8,     9,    10,     0,    11,    12,    13,    14,
      15,    16,    17,    18,     0,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,    33,
      34,    35,     0,     0,     0,     0,     0,     0,     0,     0,
      36,    37,     0,     0,     0,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,    57,    58,     0,     0,    59,
      60,    61,    62,    63,    64,     0,     0,    66,     0,     0,
      67,     0,     0,     0,     0,     1,     0,     0,   409,     3,
       4,     5,     6,     7,     8,     9,    10,     0,    11,    12,
      13,    14,    15,    16,    17,    18,     0,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,    33,    34,    35,     0,     0,     0,     0,     0,     0,
       0,     0,    36,    37,     0,     0,     0,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,    57,    58,     0,
       0,    59,    60,    61,    62,    63,    64,     0,     0,    66,
       0,     0,    67,     0,     0,     0,     0,     1,     0,     0,
     536,     3,     4,     5,     6,     7,     8,     9,    10,     0,
      11,    12,    13,    14,    15,    16,    17,    18,     0,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,    33,    34,    35,     0,     0,     0,     0,
       0,     0,     0,     0,    36,    37,     0,     0,     0,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,    57,
      58,     0,     0,    59,    60,    61,    62,    63,    64,     0,
       0,    66,     0,     0,    67,     0,     0,     0,     0,     1,
       0,     0,   568,     3,     4,     5,     6,     7,     8,     9,
      10,     0,    11,    12,    13,    14,    15,    16,    17,    18,
       0,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,    33,    34,    35,     0,     0,
       0,     0,     0,     0,     0,     0,    36,    37,     0,     0,
       0,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,    57,    58,     0,     0,    59,    60,    61,    62,    63,
      64,     0,     0,     0,     0,     0,    67,     0,     0,     0,
       0,     0,     1,     0,   481,    87,     3,     4,     5,     6,
       7,     8,    88,    10,    89,    90,    12,    91,    92,    93,
      94,    17,    18,    95,    96,    21,    97,    23,    24,    98,
      99,   100,   101,    28,     0,    30,    31,   102,   103,   104,
     105,     0,     0,     0,     0,     0,     0,     0,     0,    36,
      37,     0,     0,     0,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,    54,    55,    56,    57,    58,     0,     0,    59,    60,
      61,    62,   106,   107,   108,   109,     0,     1,     0,   110,
      87,     3,     4,     5,     6,     7,     8,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,     0,     0,    36,    37,     0,     0,     0,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   178,
      58,     0,     0,    59,    60,    61,    62,   106,   107,   108,
     109,     0,     1,     0,   110,    87,     3,     4,     5,     6,
       7,     8,    88,    10,    89,    90,    12,    91,    92,    93,
      94,    17,    18,    95,    96,    21,    97,    23,    24,    98,
      99,   100,   101,    28,     0,    30,    31,   102,   103,   104,
     105,     0,     0,     0,     0,     0,     0,     0,     0,    36,
      37,     0,     0,     0,    38,    39,    40,    41,    42,    43,
      44,    45,   221,   222,   223,   224,   225,    51,   226,    53,
       0,    54,    55,   227,    57,    58,     0,     0,    59,    60,
      61,    62,   106,   107,   108,   109,     0,     1,     0,   110,
      87,   133,   134,     5,     6,   137,   138,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,     0,     0,    36,    37,     0,     0,     0,   142,
      39,   143,    41,   144,    43,   145,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   389,
      58,     0,     0,    59,    60,    61,    62,   106,   107,   108,
     109,     1,     0,     0,   110,     3,     4,     5,     6,     7,
       8,     9,    10,     0,    11,    12,    13,    14,    15,     0,
      17,    18,     0,    20,    21,    22,    23,    24,    25,    99,
      27,     0,    28,     0,    30,    31,     0,    33,    34,    35,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,    57,    58,   306,     0,    59,    60,    61,
      62,   307,    64,     0,     0,     0,     0,     0,    67,   250,
     251,   252,   253,   254,   255,     0,   256,   308,     0,   257,
     258,   259,     0,   309,     0,     0,     0,     0,     0,     0,
     510,   250,   251,   252,   253,   254,   255,     0,   256,     0,
       0,   257,   258,   259,   250,   251,   252,   253,   254,   255,
       0,   256,   511,     0,   257,   258,   259,     0,   260,   261,
     262,   263,   264,   265,   266,     0,   250,   251,   252,   253,
     254,   255,     0,   256,     0,     0,   257,   258,   259,     0,
     260,   261,   262,   263,   264,   265,   266,   512,     0,     0,
       0,     0,     0,   260,   261,   262,   263,   264,   265,   266,
       0,   250,   251,   252,   253,   254,   255,     0,   256,   513,
       0,   257,   258,   259,     0,   260,   261,   262,   263,   264,
     265,   266,   592,   250,   251,   252,   253,   254,   255,     0,
     256,     0,     0,   257,   258,   259,   576,   577,     0,   578,
     579,   580,     0,   581,     0,     0,   582,   583,   584,   294,
     260,   261,   262,   263,   264,   265,   266,   250,   251,   252,
     253,   254,   255,     0,   256,     0,     0,   257,   258,   259,
       0,     0,   260,   261,   262,   263,   264,   265,   266,     0,
       0,     0,     0,   274,     0,   585,   586,   587,   588,   589,
     590,   591,   250,   251,   252,   253,   254,   255,     0,   256,
       0,     0,   257,   258,   259,     0,   260,   261,   262,   263,
     264,   265,   266,   295,   250,   251,   252,   253,   254,   255,
       0,   256,     0,     0,   257,   258,   259,   250,   251,   252,
     253,   254,   255,     0,   256,     0,     0,   257,   258,   259,
       0,   260,   261,   262,   263,   264,   265,   266,   250,   251,
     252,   253,   254,   255,     0,   256,     0,     0,   257,   258,
     259,     0,     0,   260,   261,   262,   263,   264,   265,   266,
       0,   333,     0,   334,     0,     0,   260,   261,   262,   263,
     264,   265,   266,     0,   415,     0,   334,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   260,   261,   262,
     263,   264,   265,   266,     0,     0,     0,   334,   367,   368,
     369,   370,   371,   372,     0,   373,     0,     0,   374,   375,
     376,   367,   368,   369,   370,   371,   372,     0,   373,     0,
       0,   374,   375,   376,   367,   368,   369,   370,   371,   372,
       0,   373,     0,     0,   374,   375,   376,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   377,   378,   379,
     380,   381,   382,   383,     0,   384,     0,     0,     0,     0,
     377,   378,   379,   380,   381,   382,   383,     0,   446,     0,
       0,     0,     0,   377,   378,   379,   380,   381,   382,   383,
       0,   528,   250,   251,   252,   253,   254,   255,     0,   256,
       0,     0,   257,   258,   259,   250,   251,   252,   253,   254,
     255,     0,   256,     0,     0,   257,   258,   259,   250,   251,
     252,   253,   254,   255,     0,   256,     0,     0,   257,   258,
     259,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   260,   261,   262,   263,   264,   265,   266,   276,     0,
       0,     0,     0,     0,   260,   261,   262,   263,   264,   265,
     266,   397,     0,     0,     0,     0,     0,   260,   261,   262,
     263,   264,   265,   266,   500,   250,   251,   252,   253,   254,
     255,     0,   256,     0,     0,   257,   258,   259,   250,   251,
     252,   253,   254,   255,     0,   256,     0,     0,   257,   258,
     259,   250,   251,   252,   253,   254,   255,     0,   256,     0,
       0,   257,   258,   259,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   260,   261,   262,   263,   264,   265,
     266,   504,     0,     0,     0,     0,     0,   260,   261,   262,
     263,   264,   265,   266,   538,     0,     0,     0,     0,     0,
     260,   261,   262,   263,   264,   265,   266,   603,   250,   251,
     252,   253,   254,   255,     0,   256,     0,     0,   257,   258,
     259,     0,     0,     0,     0,     0,   250,   251,   252,   253,
     254,   255,     0,   256,     0,     0,   257,   258,   259,   250,
     251,   252,   253,   254,   255,     0,   256,     0,     0,   257,
     258,   259,     0,     0,     0,     0,     0,   260,   261,   262,
     263,   264,   265,   266,   627,   330,   331,     0,     0,     0,
       0,     0,     0,     0,     0,   260,   261,   262,   263,   264,
     265,   266,     0,     0,     0,     0,     0,     0,   260,   261,
     262,   263,   264,   265,   266,   367,   368,   369,   370,   371,
     372,     0,   373,     0,     0,   374,   375,   376,   576,   577,
       0,   578,     0,     0,     0,   581,     0,     0,   582,   583,
     584,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   377,   378,   379,   380,   381,   382,
     383,     0,     0,     0,     0,     0,     0,   585,   586,   587,
     588,   589,   590,   591
};

static const yytype_int16 yycheck[] =
{
       0,     2,    67,     0,   400,    18,   386,    35,    14,    27,
      16,    34,    13,     9,    20,    25,   246,   536,    19,    25,
     218,   103,   165,    33,    34,   107,    27,    33,    34,   165,
     270,    32,   175,    52,    35,   233,    64,   267,   571,   175,
      34,    52,   101,   186,    55,   101,     9,    52,    37,   568,
     186,    71,    15,    52,    25,   198,   102,    63,    47,    79,
     106,     0,   198,    64,    65,    75,   599,    67,    25,    75,
      70,    39,    40,    70,    31,   218,    47,    90,   101,    72,
      99,   100,   218,    52,   101,    96,    97,    98,    99,   100,
     233,   103,   332,    89,    99,   100,    31,   233,    97,    98,
      99,   100,    31,   333,   484,   101,   101,   347,   106,    71,
     101,   102,   118,   119,   120,   121,    71,    79,    10,    11,
     101,   102,    14,    15,    79,   101,   366,    71,    97,    98,
      99,   100,   150,   139,   140,    79,   142,   143,   144,   145,
     101,   147,   170,    71,   150,   146,   280,   281,   346,   102,
     151,    79,   101,   102,   384,   551,   536,   107,   428,   429,
     430,   431,   168,   101,   192,   435,    58,   101,    60,   170,
      62,   177,    64,   106,   101,   176,    52,   178,   388,    55,
     101,   101,   101,   101,   101,   415,    78,   397,   568,   101,
      58,   192,    60,   101,    62,   201,    64,   107,   101,   200,
     107,   107,   102,   346,   414,    29,   101,   101,    31,   449,
     346,   115,   116,   117,   215,    29,   446,   102,   458,    18,
      96,    97,    98,    99,   100,    92,   101,   101,   246,   294,
     134,   135,   136,   137,   138,   101,    91,   101,     9,   104,
     480,   104,    31,    73,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   279,   280,   281,   104,   104,   101,    37,   101,   509,
     101,    73,   104,    90,   274,   104,    91,   274,   279,   280,
     281,   287,    73,   104,   290,   525,    91,    91,   114,   115,
     116,   117,   101,   101,   294,   295,    70,   303,   528,   305,
     101,   307,   405,   309,    34,   412,   484,   133,   134,   135,
     136,   137,   138,   329,   522,   141,   279,   280,   281,   341,
      -1,    -1,   148,   149,    -1,    -1,   566,    -1,   538,   335,
     336,   337,   338,    -1,   340,   341,    -1,    -1,   344,    -1,
     341,    -1,   348,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   494,   495,   496,   497,    -1,    -1,    -1,    -1,    -1,
      -1,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,    -1,    -1,
      -1,    -1,    -1,   593,    -1,    -1,   386,    -1,   388,   395,
     396,    -1,    -1,   603,    -1,    -1,    -1,   397,    -1,   400,
     401,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    48,   412,    -1,   414,    52,    -1,   627,    55,    56,
      57,   427,    -1,    -1,    -1,    -1,   495,   496,   497,    -1,
      -1,    -1,    -1,   576,   577,   578,   579,   580,   581,   582,
     583,   584,   585,   586,   587,   588,   589,   590,   591,    -1,
      -1,    -1,    -1,   459,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   502,    -1,   302,   620,   304,    -1,
     306,    -1,   308,    -1,   484,    -1,   486,    -1,    -1,   486,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   502,   328,   329,   330,   331,    -1,   576,   577,   578,
     579,   580,   581,   582,   583,   584,   585,   586,   587,   588,
     589,   590,   591,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   532,    -1,   540,   532,   536,    -1,   538,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     551,   620,    -1,    -1,    -1,    -1,    17,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   573,   568,    -1,
      45,    46,   398,    48,    49,    50,    37,    52,    -1,    -1,
      55,    56,    57,    -1,    45,    46,    47,    48,    49,    50,
      -1,    52,    -1,   593,    55,    56,    57,    -1,    -1,    -1,
     426,    -1,    -1,   603,    -1,    -1,   432,    -1,   434,    -1,
      -1,    -1,   438,   439,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,    -1,   627,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
      31,    -1,    -1,    -1,    -1,    -1,    37,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    48,    49,    50,
      -1,    52,    -1,    -1,    55,    56,    57,    -1,   494,   495,
     496,   497,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   510,   511,   512,   513,    -1,    -1,
     516,   517,   518,   519,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,   562,    55,    56,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     576,   577,   578,   579,   580,   581,   582,   583,   584,   585,
     586,   587,   588,   589,   590,   591,    -1,    -1,    -1,    -1,
     596,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   620,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      -1,    -1,    -1,    -1,   103,    -1,   105,    -1,   107,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,    -1,   102,   103,    -1,
     105,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    89,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,    -1,   102,
     103,    -1,   105,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
      -1,   102,   103,    -1,   105,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    -1,    -1,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,    -1,   102,   103,    -1,   105,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,   103,    -1,   105,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    -1,    -1,    -1,    -1,   103,    -1,
     105,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      53,    54,    -1,    -1,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    89,    -1,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    -1,    -1,   101,    -1,
     103,    -1,   105,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    -1,    -1,
      -1,    -1,   103,   104,   105,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      -1,    -1,    -1,    -1,   103,   104,   105,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    -1,    -1,    -1,    -1,   103,   104,   105,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    -1,    -1,    -1,    -1,   103,   104,
     105,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      53,    54,    -1,    -1,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    89,    -1,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    -1,    -1,    -1,    -1,
     103,   104,   105,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    -1,    -1,
      -1,    -1,   103,    -1,   105,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      -1,    -1,    -1,    -1,   103,    -1,   105,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    -1,    -1,    -1,    -1,   103,    -1,   105,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    -1,    -1,    -1,    -1,   103,    -1,
     105,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      53,    54,    -1,    -1,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    89,    -1,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    -1,    -1,    -1,    -1,
     103,    -1,   105,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    -1,    -1,
      -1,    -1,   103,    -1,   105,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      -1,    -1,    -1,    -1,   103,    -1,   105,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    -1,    -1,    -1,    -1,   103,    -1,   105,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,     6,
     105,    -1,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
      -1,     6,    -1,    -1,   101,    10,    11,    12,    13,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    -1,    37,    38,    39,    40,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    -1,    -1,    90,    -1,    -1,    93,    -1,
      -1,    -1,    -1,     6,    -1,    -1,   101,    10,    11,    12,
      13,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    -1,    37,    38,    39,    40,    -1,    42,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    54,    -1,    -1,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    84,    85,    86,    87,    -1,    -1,    90,    -1,    -1,
      93,    -1,    -1,    -1,    -1,     6,    -1,    -1,   101,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    -1,    37,    38,    39,    40,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    84,    85,    86,    87,    -1,    -1,    90,
      -1,    -1,    93,    -1,    -1,    -1,    -1,     6,    -1,    -1,
     101,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    -1,    37,    38,
      39,    40,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    -1,
      -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    -1,     6,
      -1,    -1,   101,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    -1,
      37,    38,    39,    40,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    -1,    -1,    -1,    -1,    -1,    93,    -1,    -1,    -1,
      -1,    -1,     6,    -1,   101,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,     6,    -1,    93,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    -1,     6,    -1,    93,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,     6,    -1,    93,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,     6,    -1,    -1,    93,    10,    11,    12,    13,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    -1,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    -1,    37,    -1,    39,    40,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    31,    -1,    82,    83,    84,
      85,    37,    87,    -1,    -1,    -1,    -1,    -1,    93,    45,
      46,    47,    48,    49,    50,    -1,    52,    31,    -1,    55,
      56,    57,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    45,    46,    47,    48,    49,    50,    -1,    52,    -1,
      -1,    55,    56,    57,    45,    46,    47,    48,    49,    50,
      -1,    52,    31,    -1,    55,    56,    57,    -1,    94,    95,
      96,    97,    98,    99,   100,    -1,    45,    46,    47,    48,
      49,    50,    -1,    52,    -1,    -1,    55,    56,    57,    -1,
      94,    95,    96,    97,    98,    99,   100,    31,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
      -1,    45,    46,    47,    48,    49,    50,    -1,    52,    31,
      -1,    55,    56,    57,    -1,    94,    95,    96,    97,    98,
      99,   100,    31,    45,    46,    47,    48,    49,    50,    -1,
      52,    -1,    -1,    55,    56,    57,    45,    46,    -1,    48,
      49,    50,    -1,    52,    -1,    -1,    55,    56,    57,    37,
      94,    95,    96,    97,    98,    99,   100,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,    -1,    36,    -1,    94,    95,    96,    97,    98,
      99,   100,    45,    46,    47,    48,    49,    50,    -1,    52,
      -1,    -1,    55,    56,    57,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,    45,    46,    47,    48,    49,    50,
      -1,    52,    -1,    -1,    55,    56,    57,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      -1,    94,    95,    96,    97,    98,    99,   100,    45,    46,
      47,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
      -1,   102,    -1,   104,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,    -1,   102,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,    -1,   104,    45,    46,
      47,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    45,    46,    47,    48,    49,    50,    -1,    52,    -1,
      -1,    55,    56,    57,    45,    46,    47,    48,    49,    50,
      -1,    52,    -1,    -1,    55,    56,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,    -1,   102,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,    -1,   102,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
      -1,   102,    45,    46,    47,    48,    49,    50,    -1,    52,
      -1,    -1,    55,    56,    57,    45,    46,    47,    48,    49,
      50,    -1,    52,    -1,    -1,    55,    56,    57,    45,    46,
      47,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,   101,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   101,    45,    46,    47,    48,    49,
      50,    -1,    52,    -1,    -1,    55,    56,    57,    45,    46,
      47,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    45,    46,    47,    48,    49,    50,    -1,    52,    -1,
      -1,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   101,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,   101,    45,    46,
      47,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    48,
      49,    50,    -1,    52,    -1,    -1,    55,    56,    57,    45,
      46,    47,    48,    49,    50,    -1,    52,    -1,    -1,    55,
      56,    57,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   101,    84,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,    45,    46,    47,    48,    49,
      50,    -1,    52,    -1,    -1,    55,    56,    57,    45,    46,
      -1,    48,    -1,    -1,    -1,    52,    -1,    -1,    55,    56,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     6,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    37,    38,
      39,    40,    41,    42,    43,    44,    53,    54,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    75,    76,    77,    78,    79,    82,
      83,    84,    85,    86,    87,    88,    90,    93,   101,   109,
     110,   111,   112,   113,   118,   120,   124,   125,   126,   127,
     131,   133,   134,   135,   137,   138,   139,     9,    16,    18,
      19,    21,    22,    23,    24,    27,    28,    30,    33,    34,
      35,    36,    41,    42,    43,    44,    86,    87,    88,    89,
      93,   119,   120,   121,    10,    11,    14,    15,    58,    60,
      62,    64,    78,   150,    34,   101,   121,   122,   123,     4,
       5,     7,     8,    10,    11,    12,    13,    14,    15,    20,
      29,    51,    58,    60,    62,    64,    78,    83,    97,    98,
     103,   105,   121,   128,   130,   146,   147,   150,   152,   154,
     157,   158,   162,   165,   166,   167,   170,   150,   103,   167,
     121,   101,   167,    34,   128,   167,    39,    40,    78,   121,
     147,   149,   160,   101,   121,   128,   167,   128,   130,   122,
     167,   122,   121,   101,   126,     0,   111,   128,   167,   101,
     102,   103,   153,   166,   153,   153,   153,   167,   167,   167,
     167,    72,   101,   101,   101,   102,   153,   153,   167,   167,
     166,    66,    67,    68,    69,    70,    72,    77,   121,   167,
     166,   166,   104,   147,   167,     4,     5,     7,     8,   107,
     121,   163,   164,   167,   171,   172,   103,   101,   101,    31,
      45,    46,    47,    48,    49,    50,    52,    55,    56,    57,
      94,    95,    96,    97,    98,    99,   100,   102,   129,   146,
     168,   170,   101,   167,    36,   122,   101,   101,   101,    26,
      53,    54,   121,   167,   121,    37,    47,   132,   101,   101,
      39,   101,   101,   101,    37,   101,   101,   122,   101,   121,
     104,   167,    31,    37,    31,    37,    31,    37,    31,    37,
      58,    60,    62,    64,   151,   121,    31,   168,    71,    79,
      71,    79,    71,    79,    71,    79,    71,    79,    31,    31,
      84,    85,   168,   102,   104,   107,   107,   107,   107,   106,
     107,   102,   106,   106,   102,   104,   147,   172,    29,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   172,    45,    46,    47,
      48,    49,    50,    52,    55,    56,    57,    94,    95,    96,
      97,    98,    99,   100,   102,   129,   101,   124,   101,    78,
     147,   148,   150,   148,   148,    25,    47,   101,    31,   167,
       9,    89,   101,   114,   115,   116,   117,   167,   126,   101,
     124,   140,   141,   142,   101,   102,   166,   167,   166,   167,
     166,   167,   166,   167,    25,    31,   159,    29,    59,    61,
      63,    65,    73,    74,    75,    76,    80,    81,    82,    83,
     155,   156,   166,   155,   166,   166,   102,   129,   104,   172,
     167,   167,   167,   167,   167,   164,   167,   167,   168,   102,
     129,   167,   129,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     172,   101,   125,   143,   144,   145,    18,   140,   101,   101,
     101,   167,   167,   140,    73,    75,    82,    83,   161,   166,
     101,   119,   121,   117,   101,    92,   142,   140,   104,   172,
      31,    31,    31,    31,   166,   167,   159,   159,   159,   159,
     166,   166,   159,   166,   166,   172,   104,   129,   102,   129,
     104,   129,    18,    90,   136,   145,   101,   124,   101,    17,
      37,    91,   166,   169,   170,   169,   169,   169,   101,   122,
     101,     9,   129,   166,   166,   166,   166,   166,   166,   166,
     166,    31,    73,   156,   129,   104,   172,   104,   101,   124,
     101,   143,   140,    37,   167,   101,    45,    46,    48,    49,
      50,    52,    55,    56,    57,    94,    95,    96,    97,    98,
      99,   100,    31,   101,   119,   104,    73,   104,   129,   143,
     136,    91,   167,   101,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
      73,   140,   101,   166,   104,   136,   101,   101,   140,   169,
     140,    91,    91,   101,   101
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   108,   109,   110,   110,   111,   111,   111,   111,   111,
     111,   112,   113,   114,   115,   115,   116,   116,   117,   117,
     117,   118,   118,   119,   119,   119,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   122,   122,   122,   123,   123,
     124,   124,   125,   125,   125,   125,   125,   125,   126,   126,
     126,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   129,   129,   130,
     130,   130,   130,   130,   131,   131,   131,   131,   132,   132,
     133,   133,   133,   134,   134,   135,   135,   135,   135,   136,
     136,   137,   137,   137,   137,   138,   138,   139,   140,   140,
     141,   141,   142,   142,   143,   143,   144,   144,   145,   145,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   147,   148,   148,   149,   149,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   151,   151,   151,   151,   152,   152,   153,
     153,   153,   153,   154,   154,   154,   154,   154,   155,   155,
     155,   155,   155,   156,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   158,   158,   158,   158,   158,   158,
     158,   158,   159,   159,   160,   160,   161,   161,   161,   161,
     161,   162,   162,   162,   163,   163,   163,   164,   164,   164,
     164,   164,   165,   165,   166,   166,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   170,   170,   171,   171,   172,   172
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
       1,     1,     1,     1,     1,     0,     1,     2,     1,     3,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     5,     3,
       3,     2,     2,     3,     3,     3,     2,     0,     2,     4,
       3,     5,     2,     4,     6,     5,     7,     0,     1,     2,
       2,     5,     4,     3,     5,     5,     5,     5,     1,     1,
       3,     3,     3,     4,     6,     6,     8,     7,     9,     0,
       2,     7,    11,    12,     9,     4,     6,     2,     0,     1,
       1,     2,     1,     1,     0,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     2,     3,     5,     6,     8,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     4,     6,     4,     6,     4,     6,
       4,     6,     5,     1,     1,     1,     1,     2,     2,     1,
       2,     4,     6,     2,     4,     4,     1,     1,     1,     1,
       5,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     6,     6,     6,     6,     6,     4,
       4,     4,     1,     1,     2,     4,     1,     2,     5,     2,
       2,     3,     3,     3,     1,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     4,     4,     0,     1,     1,     3
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
#line 201 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2360 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 201 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2366 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 201 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2372 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 201 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2378 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_script: /* script  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2384 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_scriptpart: /* scriptpart  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2390 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_macro: /* macro  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2396 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_factory: /* factory  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2402 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_method: /* method  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2408 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_methodlistline: /* methodlistline  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2414 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_handler: /* handler  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2420 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 201 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2426 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 201 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2432 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmt: /* stmt  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2438 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmt_insideif: /* stmt_insideif  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2444 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtoneliner: /* stmtoneliner  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2450 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_proc: /* proc  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2456 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_asgn: /* asgn  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2462 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_definevars: /* definevars  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2468 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ifstmt: /* ifstmt  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2474 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ifelsestmt: /* ifelsestmt  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2480 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_loop: /* loop  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2486 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tell: /* tell  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2492 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_when: /* when  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2498 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtlistline: /* stmtlistline  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2504 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtlistline_insideif: /* stmtlistline_insideif  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2510 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_simpleexpr_nounarymath: /* simpleexpr_nounarymath  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2516 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_var: /* var  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2522 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_varorchunk: /* varorchunk  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2528 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_varorthe: /* varorthe  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2534 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_chunk: /* chunk  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2540 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_object: /* object  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2546 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_the: /* the  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2552 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_theobj: /* theobj  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2558 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_menu: /* menu  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2564 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_thedatetime: /* thedatetime  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2570 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_thenumberof: /* thenumberof  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2576 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_writablethe: /* writablethe  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2582 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_writabletheobj: /* writabletheobj  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2588 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_list: /* list  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2594 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_proppair: /* proppair  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2600 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_unarymath: /* unarymath  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2606 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_simpleexpr: /* simpleexpr  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2612 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr: /* expr  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2618 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr_nounarymath: /* expr_nounarymath  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2624 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr_noeq: /* expr_noeq  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2630 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_sprite: /* sprite  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2636 "engines/director/lingo/lingo-gr.cpp"
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
                                                        { g_lingo->_compiler->_assemblyAST = Common::SharedPtr<Node>(new ScriptNode((yyvsp[0].nodelist))); (yyval.node) = nullptr; }
#line 2906 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 210 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 216 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2927 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 223 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2933 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* scriptpart: tENDCLAUSE endargdef '\n'  */
#line 228 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 257 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* factory: tFACTORY ID '\n' methodlist  */
#line 261 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 263 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* methodlist: %empty  */
#line 265 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2963 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlistline  */
#line 269 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2974 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 275 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2984 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18: /* methodlistline: '\n'  */
#line 282 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2990 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* methodlistline: tENDCLAUSE endargdef '\n'  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2996 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 289 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 3005 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* handler: tON ID idlist '\n' stmtlist  */
#line 293 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 3012 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* endargdef: ID  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3018 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* endargdef: endargdef ',' ID  */
#line 299 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* CMDID: tABBREVIATED  */
#line 307 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 3030 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* CMDID: tABBREV  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 3036 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* CMDID: tABBR  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 3042 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* CMDID: tAFTER  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 3048 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* CMDID: tBEFORE  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 3054 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tCAST  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 3060 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tCASTLIB  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("castLib"); }
#line 3066 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tCHAR  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 3072 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tCHARS  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 3078 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tDATE  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 3084 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tDELETE  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 3090 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tDOWN  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 3096 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tFIELD  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 3102 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tFRAME  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 3108 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tHILITE  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 3114 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tIN  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 3120 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tINTERSECTS  */
#line 323 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 3126 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tINTO  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 3132 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tITEM  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 3138 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tITEMS  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 3144 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tLAST  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 3150 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tLINE  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 3156 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tLINES  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 3162 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tLONG  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 3168 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tMEMBER  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("member"); }
#line 3174 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tMENU  */
#line 332 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 3180 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tMENUITEM  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 3186 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tMENUITEMS  */
#line 334 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 3192 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tMOVIE  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 3198 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tNEXT  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 3204 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tNUMBER  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 3210 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tOF  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 3216 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tPREVIOUS  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 3222 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tREPEAT  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 3228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tSCRIPT  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 3234 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tASSERTERROR  */
#line 342 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 3240 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tSHORT  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 3246 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tSOUND  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 3252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tSPRITE  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 3258 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tTHE  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 3264 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tTIME  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 3270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tTO  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 3276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tWHILE  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 3282 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tWINDOW  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 3288 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tWITH  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 3294 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tWITHIN  */
#line 352 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 3300 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* CMDID: tWORD  */
#line 353 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 3306 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* CMDID: tWORDS  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 3312 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* ID: tELSE  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 3318 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* ID: tENDCLAUSE  */
#line 359 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); delete (yyvsp[0].s); }
#line 3324 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* ID: tEXIT  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 3330 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* ID: tFACTORY  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 3336 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* ID: tGLOBAL  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 3342 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tGO  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 3348 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tIF  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 3354 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tINSTANCE  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 3360 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tMACRO  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tMETHOD  */
#line 367 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tON  */
#line 368 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3378 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tOPEN  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3384 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tPLAY  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3390 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tPROPERTY  */
#line 371 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3396 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tPUT  */
#line 372 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3402 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tRETURN  */
#line 373 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("return"); }
#line 3408 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tSET  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3414 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ID: tTELL  */
#line 375 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3420 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* ID: tTHEN  */
#line 376 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3426 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* idlist: %empty  */
#line 379 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3432 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* nonemptyidlist: ID  */
#line 384 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 388 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3449 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101: /* stmt: tENDIF '\n'  */
#line 399 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3455 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* proc: CMDID cmdargs '\n'  */
#line 415 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3461 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* proc: tPUT cmdargs '\n'  */
#line 416 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3467 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* proc: tGO cmdargs '\n'  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* proc: tGO frameargs '\n'  */
#line 418 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3479 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* proc: tPLAY cmdargs '\n'  */
#line 419 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3485 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: tPLAY frameargs '\n'  */
#line 420 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3491 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: tOPEN cmdargs '\n'  */
#line 421 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3497 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tOPEN expr tWITH expr '\n'  */
#line 422 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3507 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tNEXT tREPEAT '\n'  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3513 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tEXIT tREPEAT '\n'  */
#line 428 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3519 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tEXIT '\n'  */
#line 429 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* proc: tRETURN '\n'  */
#line 430 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ReturnNode(nullptr); }
#line 3531 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proc: tRETURN expr '\n'  */
#line 431 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ReturnNode((yyvsp[-1].node)); }
#line 3537 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tDELETE chunk '\n'  */
#line 432 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3543 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proc: tHILITE chunk '\n'  */
#line 433 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3549 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* proc: tASSERTERROR stmtoneliner  */
#line 434 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3555 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* cmdargs: %empty  */
#line 437 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3563 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* cmdargs: expr trailingcomma  */
#line 440 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3573 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 445 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd arg, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 449 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3593 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3603 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* cmdargs: '(' ')'  */
#line 460 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* cmdargs: '(' expr ',' ')'  */
#line 463 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3621 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 468 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3630 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* cmdargs: '(' var expr_nounarymath trailingcomma ')'  */
#line 472 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3641 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* cmdargs: '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 478 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3651 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* frameargs: tFRAME expr  */
#line 491 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3661 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* frameargs: tMOVIE expr  */
#line 496 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3672 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 502 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3683 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* frameargs: expr tOF tMOVIE expr  */
#line 508 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3694 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* frameargs: tFRAME expr expr_nounarymath  */
#line 514 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 522 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 523 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 524 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3723 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* asgn: tSET varorthe to expr '\n'  */
#line 525 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* definevars: tGLOBAL idlist '\n'  */
#line 530 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* definevars: tPROPERTY idlist '\n'  */
#line 531 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* definevars: tINSTANCE idlist '\n'  */
#line 532 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3747 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* ifstmt: tIF expr tTHEN stmt  */
#line 535 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3756 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 3763 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 543 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3774 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 549 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 3783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 553 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3792 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 557 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 3799 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* endif: %empty  */
#line 561 "engines/director/lingo/lingo-gr.y"
                        {
		LingoCompiler *compiler = g_lingo->_compiler;
		warning("LingoCompiler::parse: no end if at line %d col %d in %s id: %d",
			compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
			compiler->_assemblyContext->_id);

		}
#line 3811 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 570 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 572 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3825 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 574 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3832 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 576 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3839 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* tell: tTELL expr tTO stmtoneliner  */
#line 580 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3848 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 584 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3855 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* when: tWHEN '\n'  */
#line 588 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WhenNode((yyvsp[-1].w).eventName, (yyvsp[-1].w).stmt); }
#line 3861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* stmtlist: %empty  */
#line 590 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3867 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* nonemptystmtlist: stmtlistline  */
#line 595 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 601 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3888 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* stmtlistline: '\n'  */
#line 608 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3894 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* stmtlist_insideif: %empty  */
#line 612 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 3900 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 617 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 623 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3921 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* stmtlistline_insideif: '\n'  */
#line 630 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3927 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* simpleexpr_nounarymath: tINT  */
#line 637 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3933 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* simpleexpr_nounarymath: tFLOAT  */
#line 638 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* simpleexpr_nounarymath: tSYMBOL  */
#line 639 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* simpleexpr_nounarymath: tSTRING  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 641 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 3963 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 643 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 3969 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath trailingcomma ')'  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.node) = new FuncNode((yyvsp[-5].s), args); }
#line 3980 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.node) = new FuncNode((yyvsp[-7].s), (yyvsp[-2].nodelist)); }
#line 3990 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 655 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3996 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* var: ID  */
#line 663 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 4002 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* chunk: tFIELD refargs  */
#line 673 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 4008 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* chunk: tCAST refargs  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 4014 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* chunk: tMEMBER refargs  */
#line 675 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("member"), (yyvsp[0].nodelist)); }
#line 4020 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* chunk: tCASTLIB refargs  */
#line 676 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("castLib"), (yyvsp[0].nodelist)); }
#line 4026 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* chunk: tCHAR expr tOF simpleexpr  */
#line 677 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4033 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 679 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4040 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* chunk: tWORD expr tOF simpleexpr  */
#line 681 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4047 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 683 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4054 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* chunk: tITEM expr tOF simpleexpr  */
#line 685 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 687 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4068 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* chunk: tLINE expr tOF simpleexpr  */
#line 689 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4075 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 691 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 693 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 4088 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* chunktype: tCHAR  */
#line 696 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 4094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* chunktype: tWORD  */
#line 697 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 4100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* chunktype: tITEM  */
#line 698 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 4106 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* chunktype: tLINE  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 4112 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* object: tSCRIPT refargs  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 4118 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* object: tWINDOW refargs  */
#line 703 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 4124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* refargs: simpleexpr  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 4134 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* refargs: '(' ')'  */
#line 711 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 4142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 221: /* refargs: '(' expr ',' ')'  */
#line 714 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 4152 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 222: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 719 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4161 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 223: /* the: tTHE ID  */
#line 725 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 4167 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 224: /* the: tTHE ID tOF theobj  */
#line 726 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 4173 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 225: /* the: tTHE tNUMBER tOF theobj  */
#line 727 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 4179 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 230: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 4185 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* theobj: tSOUND simpleexpr  */
#line 735 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 4191 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* theobj: tSPRITE simpleexpr  */
#line 736 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 4197 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* menu: tMENU simpleexpr  */
#line 739 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 4203 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4209 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4215 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* thedatetime: tTHE tABBREV tDATE  */
#line 743 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4221 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* thedatetime: tTHE tABBREV tTIME  */
#line 744 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4227 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* thedatetime: tTHE tABBR tDATE  */
#line 745 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4233 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* thedatetime: tTHE tABBR tTIME  */
#line 746 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4239 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* thedatetime: tTHE tLONG tDATE  */
#line 747 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 4245 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* thedatetime: tTHE tLONG tTIME  */
#line 748 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 4251 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* thedatetime: tTHE tSHORT tDATE  */
#line 749 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 4257 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* thedatetime: tTHE tSHORT tTIME  */
#line 750 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 4263 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 754 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 4269 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 755 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 4275 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 756 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 4281 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 757 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 4287 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 758 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 4293 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* thenumberof: tTHE tNUMBER tOF tMENUS  */
#line 759 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenus, nullptr); }
#line 4299 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* thenumberof: tTHE tNUMBER tOF tXTRAS  */
#line 760 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfXtras, nullptr); }
#line 4305 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* thenumberof: tTHE tNUMBER tOF tCASTLIBS  */
#line 761 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNumberOfNode(kNumberOfCastlibs, nullptr); }
#line 4311 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* writablethe: tTHE ID  */
#line 766 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 4317 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* writablethe: tTHE ID tOF writabletheobj  */
#line 767 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 4323 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* writabletheobj: tMENU expr_noeq  */
#line 771 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 4329 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 772 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 4335 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* writabletheobj: tSOUND expr_noeq  */
#line 773 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 4341 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* writabletheobj: tSPRITE expr_noeq  */
#line 774 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 4347 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 261: /* list: '[' exprlist ']'  */
#line 777 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 4353 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 262: /* list: '[' ':' ']'  */
#line 778 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 4359 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 263: /* list: '[' proplist ']'  */
#line 779 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 4365 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 264: /* proplist: proppair  */
#line 785 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4374 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 265: /* proplist: proplist ',' proppair  */
#line 789 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4382 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* proplist: proplist ',' expr  */
#line 792 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4390 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* proppair: tSYMBOL ':' expr  */
#line 797 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4396 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* proppair: ID ':' expr  */
#line 798 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4402 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* proppair: tSTRING ':' expr  */
#line 799 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4408 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 270: /* proppair: tINT ':' expr  */
#line 800 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new IntNode((yyvsp[-2].i)), (yyvsp[0].node)); }
#line 4414 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 271: /* proppair: tFLOAT ':' expr  */
#line 801 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new FloatNode((yyvsp[-2].f)), (yyvsp[0].node)); }
#line 4420 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* unarymath: '+' simpleexpr  */
#line 804 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 4426 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* unarymath: '-' simpleexpr  */
#line 805 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 4432 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* expr: expr '+' expr  */
#line 815 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4438 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* expr: expr '-' expr  */
#line 816 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4444 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* expr: expr '*' expr  */
#line 817 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4450 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* expr: expr '/' expr  */
#line 818 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4456 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* expr: expr tMOD expr  */
#line 819 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4462 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 283: /* expr: expr '>' expr  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4468 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 284: /* expr: expr '<' expr  */
#line 821 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4474 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* expr: expr tEQ expr  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4480 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* expr: expr tNEQ expr  */
#line 823 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4486 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 287: /* expr: expr tGE expr  */
#line 824 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 288: /* expr: expr tLE expr  */
#line 825 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4498 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 289: /* expr: expr tAND expr  */
#line 826 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4504 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 290: /* expr: expr tOR expr  */
#line 827 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4510 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 291: /* expr: expr '&' expr  */
#line 828 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 292: /* expr: expr tCONCAT expr  */
#line 829 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4522 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* expr: expr tCONTAINS expr  */
#line 830 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4528 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 294: /* expr: expr tSTARTS expr  */
#line 831 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4534 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 840 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4540 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 841 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4546 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 842 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4552 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 843 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4558 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 301: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 844 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4564 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 302: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 845 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4570 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 303: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 846 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 304: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 847 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 848 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 849 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4594 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 307: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 850 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4600 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 308: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 851 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4606 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 309: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 852 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4612 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 310: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 853 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 311: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 854 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4624 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 312: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 855 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4630 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 313: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 856 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 861 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4642 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 862 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4648 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 863 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4654 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 864 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4660 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 865 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4666 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 866 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4672 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 322: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 867 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4678 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 323: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 868 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4684 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 324: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 869 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4690 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 325: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 870 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4696 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 326: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 871 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4702 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 327: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 872 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4708 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 328: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 873 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4714 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 329: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 874 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4720 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 330: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 875 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 331: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 876 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4732 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 332: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 879 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4738 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 333: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 880 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4744 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 334: /* exprlist: %empty  */
#line 883 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 4750 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 336: /* nonemptyexprlist: expr  */
#line 887 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 337: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 891 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4767 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 4771 "engines/director/lingo/lingo-gr.cpp"

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

#line 896 "engines/director/lingo/lingo-gr.y"


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
