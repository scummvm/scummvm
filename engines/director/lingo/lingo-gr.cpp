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

// if we have the trim garbage flag on, and we encounter
// an unwelcome surprise in an expression, use everything up until
// the first failing token.
#define TRIM_GARBAGE(src, target) \
	if (g_lingo->_compiler->_trimGarbage) { \
		warning("Trimming garbage and continuing.");	\
		src = target;	\
		yyerrok; \
	} else { \
		src = nullptr; \
		YYABORT; \
	}	\


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


#line 150 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  197
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4899

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  108
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  371
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  729

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
       0,   225,   225,   227,   233,   240,   241,   242,   243,   244,
     245,   274,   275,   280,   281,   284,   285,   288,   289,   292,
     298,   305,   306,   307,   312,   316,   320,   322,   326,   327,
     328,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     408,   409,   410,   413,   417,   427,   428,   431,   432,   433,
     434,   435,   436,   439,   440,   441,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   480,   483,   488,   492,
     498,   503,   506,   511,   515,   521,   528,   528,   534,   539,
     545,   551,   557,   565,   566,   567,   568,   569,   570,   571,
     572,   575,   575,   577,   578,   579,   580,   581,   582,   585,
     589,   591,   595,   601,   605,   609,   613,   620,   622,   624,
     626,   628,   630,   632,   634,   636,   640,   644,   646,   650,
     652,   653,   657,   663,   670,   671,   674,   675,   679,   685,
     692,   693,   699,   700,   701,   702,   703,   704,   705,   706,
     712,   717,   718,   719,   720,   721,   722,   725,   727,   728,
     731,   732,   735,   736,   741,   742,   747,   748,   749,   751,
     753,   755,   757,   759,   761,   763,   765,   768,   769,   770,
     771,   774,   775,   778,   783,   786,   791,   797,   798,   799,
     800,   801,   804,   805,   806,   807,   808,   811,   813,   814,
     815,   816,   817,   818,   819,   820,   821,   822,   826,   827,
     828,   829,   830,   831,   832,   833,   836,   836,   838,   839,
     842,   843,   844,   845,   846,   849,   850,   851,   857,   861,
     864,   869,   870,   871,   872,   873,   876,   877,   880,   881,
     885,   886,   887,   888,   889,   890,   891,   892,   893,   894,
     895,   896,   897,   898,   899,   900,   901,   902,   903,   910,
     911,   912,   913,   914,   915,   916,   917,   918,   919,   920,
     921,   922,   923,   924,   925,   926,   927,   928,   931,   932,
     933,   934,   935,   936,   937,   938,   939,   940,   941,   942,
     943,   944,   945,   946,   947,   948,   951,   952,   955,   956,
     959,   963
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

#define YYPACT_NINF (-542)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    3349,  -542,  3810,  -542,  -542,  -542,  -542,  -542,  -542,   332,
    -542,    10,  -542,  3810,  2291,   332,  2393,  -542,  -542,  3810,
     963,  -542,    26,  -542,  -542,  2495,   105,  3895,  -542,   -62,
    -542,  -542,  3810,  2495,  2291,  3810,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  2393,  3810,  3810,   -30,  4149,  -542,    85,
    3349,  -542,  -542,  -542,  -542,  2495,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,
    -542,   -10,  -542,  -542,  2597,  2597,  2597,  2597,  2393,  2393,
    2393,  2393,    32,    16,    -2,    18,  -542,  -542,    20,     8,
    -542,  -542,  -542,  -542,  2597,  2597,  2597,  2597,  2597,  2597,
    2393,  2393,  2699,  2393,  2393,  2393,  2393,  3980,  2393,  2699,
    2699,  1781,  1065,     9,    22,    23,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,  1169,  -542,    24,  2393,
    4486,  3810,    15,  -542,   560,    27,    34,  1271,  3810,  2393,
    3810,  -542,  -542,    28,  -542,  -542,    36,    39,  1373,    40,
      41,    42,   453,    46,  3810,  -542,  -542,  -542,  -542,    47,
    1475,  -542,  3810,  1883,  -542,    87,  -542,  -542,  -542,    99,
    -542,  4228,  4262,  4284,  4318,    45,    77,  -542,  -542,    79,
    -542,    88,  -542,  3810,  -542,  -542,  1577,  4717,  -542,     3,
       5,     7,    19,    35,    45,   134,   138,  4704,  -542,  -542,
    -542,  3107,  4508,    29,    84,    89,    91,    94,   -41,    58,
    -542,  4717,    96,    92,  1985,   104,  -542,   111,  -542,   191,
    2393,  2393,  2393,  2393,  2393,  2393,  2393,  2393,  2393,  2393,
    2393,  2393,  2393,  2393,  2393,  2393,  2393,  2393,  -542,  -542,
    4602,  -542,   122,  -542,  4542,  3203,    51,  -542,   127,  -542,
     130,  -542,   137,  -542,  4065,  4065,  4065,    21,   711,   209,
    -542,  -542,  2393,   140,    60,   142,  -542,  2393,   143,  -542,
     146,  -542,   150,  -542,   151,  4149,  3441,   153,  -542,    52,
     154,  -542,  -542,  -542,  4521,   242,   243,  2699,  2393,  2699,
    2393,  2699,  2393,  2699,  2393,  -542,  -542,  -542,  -542,    64,
    -542,  -542,  -542,  -542,   233,  4773,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,  1679,  2801,  2699,  2699,
    4615,  2087,  -542,  2393,  2393,  2393,  2393,  -542,  2393,  2903,
    -542,  -542,  2393,  -542,  3107,   161,  -542,  -542,  2393,   119,
     119,   119,   119,  4192,  4192,  -542,   182,   119,   119,   119,
     119,   182,   -19,   -19,  -542,  -542,   161,  2393,  2393,  2393,
    2393,  2393,  2393,  2393,  2393,  2393,  2393,  2393,  2393,  2393,
    2393,  2393,  2393,  2393,  2393,  -542,  -542,   163,  3717,   247,
     174,  3441,  -542,  -542,  -542,    32,  -542,    53,  -542,    54,
      55,  2393,  2393,   183,  3441,  3005,   725,    60,  3810,  3810,
    -542,  -542,  -542,    60,  -542,  -542,  4686,  -542,  -542,  -542,
    3441,  -542,  -542,  -542,   181,  3441,  -542,  -542,   187,  3441,
    -542,  2189,  2699,  2699,  -542,  4340,  -542,  4374,  -542,  4396,
    -542,  4430,  -542,  -542,  2699,  2393,    64,    64,    64,    64,
    2699,  -542,  2699,    64,  -542,  -542,  2699,  2699,  -542,  -542,
    -542,  -542,  -542,  -542,  2393,   179,  -542,   161,  4717,  4717,
    4717,  4717,  4717,  -542,  4717,  4717,  4628,  2393,   188,  4717,
    -542,   119,   119,   119,   119,  4192,  4192,  -542,   182,   119,
     119,   119,   119,   182,   -19,   -19,  -542,  -542,   161,  3717,
    -542,  -542,     4,  3717,  -542,  3533,  3441,  -542,   197,  -542,
     198,  -542,   200,  -542,   812,  4206,  3441,   211,  2393,  2393,
    2393,  2393,  -542,  -542,   203,  -542,  -542,    57,  3810,  -542,
    -542,   217,   210,  -542,  3441,   301,  -542,   161,  -542,  -542,
    2699,  2699,  2699,  2699,  -542,  4717,  2699,  2699,  2699,  2699,
    -542,   281,   240,  -542,  -542,   161,  -542,   213,  2393,   215,
    -542,  -542,   224,  3625,   219,  -542,  -542,  3717,  -542,  -542,
    -542,  -542,  -542,   220,  3441,   285,  2393,   250,   244,  -542,
    4786,  -542,  4452,  4786,  4786,  -542,  -542,    56,   263,  -542,
     335,  3810,   262,  -542,  -542,  -542,  -542,  -542,  -542,  -542,
    -542,   294,  2699,  -542,   268,  -542,   161,  -542,  -542,  3717,
    -542,  -542,   224,  3441,   280,  2393,   846,   272,  -542,  2393,
    2393,  2393,  2393,  2393,  2393,  2393,  2393,  2393,  2393,  2393,
    2393,  2393,  2393,  2393,  2393,   302,   273,  3441,  -542,  3810,
      71,  -542,  2699,  -542,   274,   224,  -542,   286,   275,  3253,
     278,  3441,  -542,   172,   172,   172,  4799,  4799,  -542,   208,
     172,   172,   172,   172,   208,    68,    68,  -542,  -542,  2393,
    3441,  -542,    74,  -542,  -542,  -542,  -542,   279,  -542,   284,
    3441,  3441,   295,  4786,  -542,  -542,  -542,  3441,   296,   297,
     288,   308,   299,   310,  -542,   311,  -542,  -542,  -542
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    31,    28,    37,    44,    66,    75,    56,    38,    42,
      43,     0,    45,   100,   146,    46,     0,    47,    49,     0,
       0,    60,    61,    63,    64,   146,    65,     0,    73,     0,
      76,    74,     0,   146,   146,   100,    35,    36,    39,    40,
      50,    51,    53,    54,    78,    79,    32,    33,    34,    55,
      68,    41,    52,    57,    58,    59,    62,    71,    72,    69,
      70,    48,    77,     0,   100,     0,     0,    67,     5,     0,
       2,     3,     6,     7,     8,   146,     9,   105,   107,   113,
     114,   115,   108,   109,   110,   111,   112,    82,    42,    81,
      83,    85,    86,    46,    87,    89,    96,    61,    95,    65,
      97,    99,    84,    92,    93,    88,    98,    94,    91,    90,
      67,     0,    80,    29,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   135,   103,     0,   101,
     212,   213,   215,   214,    37,    44,    66,    75,    56,    38,
      45,    60,     0,    39,    50,    53,    78,    71,    70,     0,
       0,     0,   368,   227,     0,     0,   308,   222,   223,   224,
     225,   260,   261,   226,   309,   310,   156,   311,     0,     0,
       0,   100,     0,   137,     0,     0,     0,   156,     0,     0,
      71,   227,   230,     0,   231,   199,     0,     0,   156,     0,
       0,     0,     0,     0,   100,   106,   145,     1,     4,     0,
     156,    10,     0,     0,   234,   253,   232,   253,   236,   253,
     237,     0,     0,     0,     0,     0,     0,   141,   136,     0,
     133,     0,   173,   102,   251,   252,   158,   159,   216,    32,
      33,    34,    55,    68,    52,    62,   257,     0,   306,   307,
     151,   222,     0,   212,   213,   215,   214,     0,   227,     0,
     298,   370,     0,   369,     0,     0,   120,     0,   122,    63,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   157,   147,   329,
     156,   330,     0,   143,     0,     0,     0,   138,     0,   139,
       0,   131,     0,   118,    49,    35,    36,     0,     0,   288,
     171,   172,     0,     0,    17,     0,   128,    76,     0,   124,
       0,   126,     0,   177,     0,     0,   200,     0,   175,     0,
       0,   116,    30,   254,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   247,   249,   250,   248,     0,
     142,   134,   174,   104,    63,   162,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,     0,     0,     0,     0,
     156,     0,   221,     0,     0,     0,     0,   296,     0,     0,
     297,   295,     0,   217,   222,   156,   121,   123,     0,   321,
     322,   319,   320,   323,   324,   316,   326,   327,   328,   318,
     317,   325,   312,   313,   314,   315,   156,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   157,   149,   144,     0,   206,   179,
       0,   200,   140,   132,   119,    71,   228,     0,   229,     0,
       0,     0,     0,     0,   200,     0,     0,    17,    28,     0,
      21,    22,    13,    18,    19,   129,     0,   125,   127,   178,
     200,   196,   204,   205,     0,   201,   202,   176,     0,   200,
     117,     0,     0,     0,   238,     0,   242,     0,   244,     0,
     240,     0,   286,   287,     0,     0,    40,    51,    54,    79,
      57,   283,    58,    59,   284,   285,    69,    70,   259,   263,
     262,   258,   366,   367,   157,     0,   152,   156,   304,   305,
     303,   301,   302,   299,   300,   371,   156,   157,     0,   161,
     148,   340,   341,   338,   339,   342,   343,   335,   345,   346,
     347,   337,   336,   344,   331,   332,   333,   334,   156,   206,
     210,   211,   186,   207,   208,     0,   200,    11,     0,   163,
       0,   165,     0,   167,     0,     0,   200,     0,    57,    58,
      69,    70,   289,   290,     0,   169,    14,     0,   100,    20,
     130,     0,     0,   203,   200,    26,   255,   156,   233,   235,
       0,     0,     0,     0,   246,   160,     0,     0,     0,     0,
     267,     0,     0,   265,   266,   156,   154,     0,   157,     0,
     218,   150,   186,     0,     0,   180,   209,   206,   182,    12,
     164,   166,   168,     0,   200,     0,     0,     0,     0,   348,
     291,   349,     0,   293,   294,   170,    23,     0,     0,   197,
      27,    28,     0,   239,   243,   245,   241,   278,   280,   281,
     279,     0,     0,   282,     0,   153,   156,   219,   181,   206,
     184,   187,   186,   200,     0,     0,     0,     0,   188,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   200,   198,    28,
       0,   256,     0,   155,     0,   186,   183,     0,     0,     0,
       0,   200,   189,   358,   359,   357,   360,   361,   354,   363,
     364,   365,   356,   355,   362,   350,   351,   352,   353,     0,
     200,    15,     0,    24,   264,   220,   185,     0,   194,     0,
     200,   200,     0,   292,    16,    25,   195,   200,     0,     0,
       0,     0,     0,     0,   190,     0,   192,   191,   193
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -542,  -542,  -542,   338,  -542,  -542,  -542,   -24,  -542,   -29,
    -542,  -424,     0,    -1,   -32,  -542,     2,  -402,   -59,  -542,
      25,  -267,   381,  -542,  -542,  -542,  -542,  -542,  -541,  -542,
    -542,  -542,  -414,  -542,   -36,  -524,  -542,  -113,  -139,   -18,
     -70,  -542,    -5,  -542,  -542,   307,  -542,    73,  -157,  -542,
    -542,  -291,  -542,  -542,  -542,  -542,    67,  -542,   267,    63,
    -211,  -301,    33,  -542,  -248
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    69,    70,    71,    72,    73,   441,   442,   443,   444,
      74,   111,   112,   153,   128,   129,   453,    77,    78,    79,
     154,   278,   155,    80,   302,    81,    82,    83,   595,    84,
      85,    86,   454,   455,   456,   532,   533,   534,   156,   157,
     427,   183,   158,   339,   159,   204,   160,   488,   489,   161,
     162,   474,   184,   552,   163,   249,   250,   164,   165,   251,
     280,   610,   167,   252,   253
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      75,   113,    76,   191,   123,   592,   375,   537,   196,   182,
     168,   124,   127,   415,   557,   345,   531,   216,   171,   219,
     547,   221,   593,   255,   257,   282,   181,   279,   290,   396,
     360,   186,   193,   266,   127,   292,   561,   303,   279,   185,
     305,   308,   310,   312,   125,   565,   431,   317,   320,   279,
     176,   638,   420,   458,   538,   540,   542,   666,   187,   189,
     175,   279,   254,   127,   194,   300,   368,    75,   432,   438,
      75,   195,    76,   642,   346,   301,   348,   166,   350,   170,
     275,   276,   347,   174,   349,   197,   351,   279,   177,   472,
     352,   201,   202,   495,   594,   473,   188,   166,   353,   218,
     199,   676,   279,   335,   215,   336,   354,   337,   508,   338,
     223,   126,   254,   497,   355,   675,   287,   217,   325,   220,
     654,   222,   599,   256,   258,   283,   192,   531,   291,   510,
     326,   531,   607,   241,   706,   293,   363,   304,   200,   286,
     306,   309,   311,   313,   178,   179,   236,   318,   321,   439,
     620,   248,   421,   459,   539,   541,   543,   667,   616,   202,
     369,   440,   319,   506,   370,   356,   528,   663,   664,   357,
     127,   266,   703,   202,   267,   715,   202,   297,   340,   299,
     341,   211,   212,   213,   214,   576,   577,   578,   579,   342,
     644,   364,   582,   127,   372,   531,   365,   670,   366,   281,
     367,   322,   371,   226,   227,   376,   211,   212,   213,   214,
     281,   237,   377,   567,   242,   272,   273,   274,   275,   276,
     378,   281,   343,   416,   654,   429,   430,   655,   422,   677,
     587,   423,   284,   281,   266,   279,   374,   531,   424,   589,
     435,   437,   298,   445,   447,   702,   585,   448,   612,   613,
     614,   449,   450,   701,   457,   460,   451,   462,   463,   281,
     654,   591,   475,   507,   529,   535,   324,   712,   660,   661,
     662,   663,   664,   562,   281,   536,   426,   426,   426,   273,
     274,   275,   276,   586,   546,    75,   714,   419,   564,   428,
     428,   428,   590,   181,   181,   181,   718,   719,   600,   601,
     622,   602,   608,   721,   615,   661,   662,   663,   664,   618,
     621,   619,   631,   632,   594,    75,    75,   635,   634,   637,
     641,   643,   645,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     636,   647,   114,   115,   669,   648,   116,   117,   683,   684,
     685,   686,   687,   688,   689,   690,   691,   692,   693,   694,
     695,   696,   697,   698,   668,   436,   671,   672,   248,   674,
     446,   678,   673,   682,   700,   699,   708,   707,   705,   711,
     716,   205,   207,   209,   207,   717,   720,   722,   723,   724,
     118,   465,   119,   467,   120,   469,   121,   471,   713,   725,
     726,   205,   207,   207,   207,   209,   207,   281,   198,   228,
     122,   727,   728,   556,   559,   190,   238,   239,    75,   563,
     596,    75,   206,   208,   210,   633,   498,   499,   500,   501,
     491,   502,   504,     0,    75,   505,   503,   113,   558,     0,
       0,   509,   206,   224,   225,   208,   210,     0,     0,     0,
      75,     0,     0,     0,   314,    75,     0,     0,     0,    75,
     511,   512,   513,   514,   515,   516,   517,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     315,     0,     0,     0,   544,   545,     0,     0,   260,   261,
     262,   263,   264,   265,     0,   266,     0,     0,   267,   268,
     269,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   617,     0,     0,    75,
       0,     0,     0,    75,     0,    75,    75,   598,   575,     0,
       0,     0,     0,     0,     0,     0,    75,   270,   271,   272,
     273,   274,   275,   276,   316,     0,     0,   127,     0,     0,
       0,   288,     0,     0,    75,     0,     0,     0,     0,     0,
     505,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   611,   611,   611,   611,     0,     0,     0,     0,     0,
       0,     0,     0,    75,   464,   640,   466,    75,   468,     0,
     470,     0,     0,     0,    75,   260,   261,   262,   263,   264,
     265,     0,   266,     0,     0,   267,   268,   269,     0,     0,
     113,     0,     0,   490,   490,   492,   493,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,    75,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   270,   271,   272,   273,   274,   275,
     276,   289,     0,     0,     0,     0,     0,    75,   113,   646,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,   611,   611,   611,   611,   611,   611,   611,   611,
     611,   611,   611,   611,   611,   611,   611,   611,     0,     0,
      75,     0,   553,     0,     0,     0,     0,     0,   679,     0,
      75,    75,   433,     0,     0,     0,     0,    75,     0,     0,
       0,     0,     0,     0,     0,     0,   554,     0,     0,   568,
     569,     0,   611,     0,     0,     0,     0,     0,     0,     0,
       0,   574,     0,     0,     0,     0,     0,   580,     0,   581,
       0,     0,     0,   583,   584,     0,   260,   261,   262,   263,
     264,   265,     0,   266,     0,     0,   267,   268,   269,     0,
     260,   261,   262,   263,   264,   265,     0,   266,     0,     0,
     267,   268,   269,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   270,   271,   272,   273,   274,
     275,   276,   434,   603,     0,   609,   609,   609,   609,   270,
     271,   272,   273,   274,   275,   276,   555,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   623,   624,   625,
     626,     0,     0,   627,   628,   629,   630,   680,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   260,   261,   262,
     263,   264,   265,     0,   266,     0,     0,   267,   268,   269,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   260,   261,   262,   263,   264,   265,     0,   266,   580,
       0,   267,   268,   269,     0,     0,   270,   271,   272,   273,
     274,   275,   276,   604,     0,     0,   609,   609,   609,   609,
     609,   609,   609,   609,   609,   609,   609,   609,   609,   609,
     609,   609,     0,     0,     0,     0,     0,     0,     0,   704,
     270,   271,   272,   273,   274,   275,   276,   681,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   172,     0,   609,   130,   131,     1,
     132,   133,    87,   134,   135,   136,   137,   138,   139,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,    23,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   142,     0,    36,    37,     0,     0,
       0,   143,    39,   144,    41,   145,    43,   146,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   147,    58,     0,     0,    59,   148,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,     0,     0,     0,
     149,   150,     0,     0,   173,     0,   169,     0,   152,   243,
     244,     1,   245,   246,    87,   134,   135,   136,   137,   138,
     139,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   142,     0,    36,    37,
       0,     0,     0,   143,    39,   144,    41,   145,    43,   146,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   147,    58,     0,     0,    59,   148,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,   149,   150,     0,     0,     0,     0,   169,     0,
     152,     0,   247,   130,   131,     1,   132,   133,    87,   134,
     135,   136,   137,   138,   139,    88,    10,    89,    90,    12,
      91,    92,    93,    94,    17,    18,    95,    96,    21,    97,
     259,    24,    98,    99,   100,   101,    28,     0,    30,    31,
     102,   103,   104,   105,   260,   261,   262,   263,   264,   265,
     142,   266,    36,    37,   267,   268,   269,   143,    39,   144,
      41,   145,    43,   146,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,   147,    58,     0,
       0,    59,   148,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,   270,   271,   272,   273,   274,   275,   276,
       0,   277,   169,     0,   152,   130,   131,     1,   132,   133,
      87,   134,   135,   136,   137,   138,   139,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,   294,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,   260,   261,   262,   263,
     264,   265,   142,   266,   295,   296,   267,   268,   269,   143,
      39,   144,    41,   145,    43,   146,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   147,
      58,     0,     0,    59,   148,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,   270,   271,   272,   273,   274,
     275,   276,     0,   277,   169,     0,   152,   130,   131,     1,
     132,   133,    87,   134,   135,   136,   137,   138,   139,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,    23,    24,    98,    99,   100,   101,
      28,     0,   307,    31,   102,   103,   104,   105,   260,   261,
     262,   263,   264,   265,   142,   266,    36,    37,   267,   268,
     269,   143,    39,   144,    41,   145,    43,   146,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   147,    58,     0,     0,    59,   148,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,   270,   271,   272,
     273,   274,   275,   276,     0,   277,   169,     0,   152,   130,
     131,     1,   132,   133,    87,   134,   135,   136,   137,   138,
     139,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
     260,   261,   262,   263,   264,   265,   142,   266,    36,    37,
     267,   268,   269,   143,    39,   144,    41,   145,    43,   146,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   147,    58,     0,     0,    59,   148,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,   270,
     271,   272,   273,   274,   275,   276,     0,   277,   169,     0,
     152,   130,   131,     1,   132,   133,    87,   134,   135,   136,
     137,   138,   139,    88,    10,    89,    90,    12,    91,    92,
      93,    94,    17,    18,    95,    96,    21,    97,   344,    24,
      98,    99,   100,   101,    28,     0,    30,    31,   102,   103,
     104,   105,   260,   261,   262,   263,   264,   265,   142,   266,
      36,    37,   267,   268,   269,   143,    39,   144,    41,   145,
      43,   146,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,   147,    58,     0,     0,    59,
     148,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,   270,   271,   272,   273,   274,   275,   276,     0,     0,
     169,     0,   152,   130,   131,     1,   132,   133,    87,   134,
     135,   136,   137,   138,   139,    88,    10,    89,    90,    12,
      91,    92,    93,    94,    17,    18,    95,    96,    21,    97,
      23,    24,    98,    99,   100,   101,    28,     0,    30,    31,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     142,     0,    36,    37,     0,     0,     0,   143,   476,   144,
     477,   145,   478,   146,   479,    46,    47,    48,    49,    50,
      51,    52,   480,   481,   482,   483,    56,   147,    58,   484,
     485,   486,   487,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,     0,     0,     0,   149,   150,     0,     0,
       0,     0,   169,     0,   152,   130,   131,     1,   132,   133,
      87,   134,   135,   136,   137,   138,   139,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   142,     0,    36,    37,     0,     0,     0,   143,
      39,   144,    41,   145,    43,   146,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   147,
      58,     0,     0,    59,   148,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   149,   150,
       0,     0,     0,     0,   169,   240,   152,   130,   131,     1,
     132,   133,    87,   134,   135,   136,   137,   138,   139,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,    23,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   142,     0,    36,    37,     0,     0,
       0,   143,    39,   144,    41,   145,    43,   146,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   147,    58,     0,     0,    59,   148,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,     0,     0,     0,
     149,   150,     0,     0,     0,     0,   169,   323,   152,   130,
     131,     1,   132,   133,    87,   134,   135,   136,   137,   138,
     139,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   142,     0,    36,    37,
       0,     0,     0,   143,    39,   144,    41,   145,    43,   146,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   147,    58,     0,     0,    59,   148,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,   149,   150,     0,     0,     0,     0,   169,   373,
     152,   130,   131,     1,   132,   133,    87,   134,   135,   136,
     137,   138,   139,    88,    10,    89,    90,    12,    91,    92,
      93,    94,    17,    18,    95,    96,    21,    97,    23,    24,
      98,    99,   100,   101,    28,     0,    30,    31,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   142,     0,
      36,    37,     0,     0,     0,   143,    39,   144,    41,   145,
      43,   146,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,   147,    58,     0,     0,    59,
     148,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,     0,     0,     0,   149,   150,     0,     0,     0,     0,
     169,   496,   152,   130,   131,     1,   132,   133,    87,   134,
     135,   136,   137,   138,   139,    88,    10,    89,    90,    12,
      91,    92,    93,    94,    17,    18,    95,    96,    21,    97,
      23,    24,    98,    99,   100,   101,    28,     0,    30,    31,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     142,     0,    36,    37,     0,     0,     0,   143,    39,   144,
      41,   145,    43,   146,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,   147,    58,     0,
       0,    59,   148,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,     0,     0,     0,   149,   150,     0,     0,
       0,     0,   169,   566,   152,   130,   131,     1,   132,   133,
      87,   134,   135,   136,   137,   138,   139,    88,    10,    89,
      90,   140,    91,    92,    93,    94,    17,    18,    95,    96,
     141,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   142,     0,    36,    37,     0,     0,     0,   143,
      39,   144,    41,   145,    43,   146,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   147,
      58,     0,     0,    59,   148,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   149,   150,
       0,     0,     0,     0,   151,     0,   152,   130,   131,     1,
     132,   133,    87,   134,   135,   136,   137,   138,   139,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,    23,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   142,     0,    36,    37,     0,     0,
       0,   143,    39,   144,    41,   145,    43,   146,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   147,    58,     0,     0,    59,   148,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,     0,     0,     0,
     149,   150,     0,     0,     0,     0,   169,     0,   152,   130,
     131,     1,   132,   133,    87,   134,   135,   136,   137,   138,
     139,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   142,     0,    36,    37,
       0,     0,     0,   143,    39,   144,    41,   145,    43,   146,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   147,    58,     0,     0,    59,   148,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,   149,   150,     0,     0,     0,     0,   151,     0,
     152,   130,   131,     1,   132,   133,    87,   134,   135,   136,
     137,   138,   139,    88,    10,    89,    90,    12,    91,    92,
      93,    94,    17,    18,    95,    96,    21,    97,    23,    24,
      98,    99,   100,   101,    28,     0,    30,    31,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   142,     0,
      36,    37,     0,     0,     0,   143,    39,   144,    41,   145,
      43,   146,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,   147,    58,     0,     0,    59,
      60,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,     0,     0,     0,   149,   150,     0,     0,     0,     0,
     203,     0,   152,   130,   131,     1,   132,   133,    87,   134,
     135,   136,   137,   138,   139,    88,    10,    89,    90,    12,
      91,    92,    93,    94,    17,    18,    95,    96,    21,    97,
      23,    24,    98,    99,   100,   101,    28,     0,    30,    31,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     142,     0,    36,    37,     0,     0,     0,   143,    39,   144,
      41,   145,    43,   146,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,   147,    58,     0,
       0,    59,    60,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,     0,     0,     0,   149,   150,     0,     0,
       0,     0,   169,     0,   152,   130,   131,     1,   132,   133,
      87,   134,   135,   136,   137,   138,   139,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   142,     0,    36,    37,     0,     0,     0,   143,
      39,   144,    41,   145,    43,   146,    45,    46,    47,    48,
      49,    50,    51,    52,   480,     0,   482,    55,    56,   147,
      58,     0,     0,   486,   487,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   149,   150,
       0,     0,     0,     0,   169,     0,   152,   243,   244,     1,
     245,   246,    87,   134,   135,   136,   137,   138,   139,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,    23,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   142,     0,    36,    37,     0,     0,
       0,   143,    39,   144,    41,   145,    43,   146,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   147,    58,     0,     0,    59,   148,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,     0,     0,     0,
     149,   150,     0,     0,     0,     0,   169,     0,   152,   130,
     131,     1,   132,   133,    87,   134,   135,   136,   137,   138,
     139,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   142,     0,    36,    37,
       0,     0,     0,   143,    39,   144,    41,   145,    43,   146,
      45,    46,    47,    48,    49,    50,    51,    52,   548,     0,
     549,    55,    56,   147,    58,     0,     0,   550,   551,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,   149,   150,     0,     0,     0,     0,   169,     0,
     152,   130,   131,     1,   132,   133,    87,   134,   135,   136,
     137,   138,   139,    88,    10,    89,    90,    12,    91,    92,
      93,    94,    17,    18,    95,    96,    21,    97,    23,    24,
      98,    99,   100,   101,    28,     0,    30,    31,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   142,     0,
      36,    37,     0,     0,     0,   143,    39,   144,    41,   145,
      43,   146,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,   147,    58,     0,     0,    59,
     148,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,     0,     0,     0,   417,     0,     0,     0,     0,     1,
     169,     0,   152,     3,     4,     5,     6,     7,     8,     9,
      10,     0,    11,    12,    13,    14,    15,    16,    17,    18,
       0,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,    33,    34,    35,     0,     0,
       0,     0,     0,     0,   709,     0,    36,    37,     0,     0,
       0,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,    57,    58,     0,     0,    59,    60,    61,    62,    63,
      64,     0,     0,    66,     0,     0,    67,     0,   260,   261,
     262,   263,   264,   265,   418,   266,     0,     0,   267,   268,
     269,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   270,   271,   272,
     273,   274,   275,   276,   710,     1,     0,     0,     2,     3,
       4,     5,     6,     7,     8,     9,    10,     0,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,     0,
       0,     0,    36,    37,     0,     0,     0,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,    57,    58,     0,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
       0,     0,    67,     0,     0,     0,     0,     1,     0,     0,
      68,     3,     4,     5,     6,     7,     8,     9,    10,     0,
      11,    12,    13,    14,    15,    16,    17,    18,     0,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,    33,    34,    35,     0,     0,     0,     0,
       0,     0,     0,     0,    36,    37,     0,     0,     0,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,    57,
      58,     0,     0,    59,    60,    61,    62,    63,    64,     0,
       0,    66,     0,     0,    67,     0,     0,     0,     0,     1,
       0,     0,   452,     3,     4,     5,     6,     7,     8,     9,
      10,     0,    11,    12,    13,    14,    15,    16,    17,    18,
       0,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,    33,    34,    35,     0,     0,
       0,     0,     0,     0,     0,     0,    36,    37,     0,     0,
       0,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,    57,    58,     0,     0,    59,    60,    61,    62,    63,
      64,     0,     0,    66,     0,     0,    67,     0,     0,     0,
       0,     1,     0,     0,   597,     3,     4,     5,     6,     7,
       8,     9,    10,     0,    11,    12,    13,    14,    15,    16,
      17,    18,     0,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,    33,    34,    35,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,    57,    58,     0,     0,    59,    60,    61,
      62,    63,    64,     0,     0,    66,     0,     0,    67,     0,
       0,     0,     0,     1,     0,     0,   639,     3,     4,     5,
       6,     7,     8,     9,    10,     0,    11,    12,    13,    14,
      15,    16,    17,    18,     0,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,    33,
      34,    35,     0,     0,     0,     0,     0,     0,     0,     0,
      36,    37,     0,     0,     0,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,    57,    58,     0,     0,    59,
      60,    61,    62,    63,    64,     0,     0,     0,     0,     0,
      67,     0,     0,     0,     0,     0,     1,     0,   530,    87,
       3,     4,     5,     6,     7,     8,    88,    10,    89,    90,
      12,    91,    92,    93,    94,    17,    18,    95,    96,    21,
      97,    23,    24,    98,    99,   100,   101,    28,     0,    30,
      31,   102,   103,   104,   105,     0,     0,     0,     0,     0,
       0,     0,     0,    36,    37,     0,     0,     0,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,    54,    55,    56,    57,    58,
       0,     0,    59,    60,    61,    62,   106,   107,   108,   109,
       0,     1,     0,   110,    87,     3,     4,     5,     6,     7,
       8,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   180,    58,     0,     0,    59,    60,    61,
      62,   106,   107,   108,   109,     0,     1,     0,   110,    87,
       3,     4,     5,     6,     7,     8,    88,    10,    89,    90,
      12,    91,    92,    93,    94,    17,    18,    95,    96,    21,
      97,    23,    24,    98,    99,   100,   101,    28,     0,    30,
      31,   102,   103,   104,   105,     0,     0,     0,     0,     0,
       0,     0,     0,    36,    37,     0,     0,     0,    38,    39,
      40,    41,    42,    43,    44,    45,   229,   230,   231,   232,
     233,    51,   234,    53,     0,    54,    55,   235,    57,    58,
       0,     0,    59,    60,    61,    62,   106,   107,   108,   109,
       0,     1,     0,   110,    87,   134,   135,     5,     6,   138,
     139,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,   143,    39,   144,    41,   145,    43,   146,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   425,    58,     0,     0,    59,    60,    61,
      62,   106,   107,   108,   109,     1,     0,     0,   110,     3,
       4,     5,     6,     7,     8,     9,    10,     0,    11,    12,
      13,    14,    15,     0,    17,    18,     0,    20,    21,    22,
      23,    24,    25,    99,    27,     0,    28,     0,    30,    31,
       0,    33,    34,    35,     0,     0,     0,     0,     0,     0,
       0,     0,    36,    37,     0,     0,     0,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,   605,    54,    55,    56,    57,    58,     0,
       0,    59,    60,    61,    62,     0,    64,   260,   261,   262,
     263,     0,    67,   606,   266,     0,     0,   267,   268,   269,
       0,   260,   261,   262,   263,   264,   265,     0,   266,   327,
       0,   267,   268,   269,     0,   328,     0,     0,     0,     0,
       0,     0,     0,   260,   261,   262,   263,   264,   265,     0,
     266,     0,     0,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   329,     0,     0,     0,     0,     0,   330,
     270,   271,   272,   273,   274,   275,   276,   260,   261,   262,
     263,   264,   265,     0,   266,   331,     0,   267,   268,   269,
       0,   332,   270,   271,   272,   273,   274,   275,   276,   260,
     261,   262,   263,   264,   265,     0,   266,     0,     0,   267,
     268,   269,     0,     0,     0,     0,     0,     0,     0,   333,
       0,     0,     0,     0,     0,   334,   270,   271,   272,   273,
     274,   275,   276,   260,   261,   262,   263,   264,   265,     0,
     266,   570,     0,   267,   268,   269,     0,     0,   270,   271,
     272,   273,   274,   275,   276,   260,   261,   262,   263,   264,
     265,     0,   266,     0,     0,   267,   268,   269,     0,     0,
       0,     0,     0,     0,     0,   571,     0,     0,     0,     0,
       0,     0,   270,   271,   272,   273,   274,   275,   276,   260,
     261,   262,   263,   264,   265,     0,   266,   572,     0,   267,
     268,   269,     0,     0,   270,   271,   272,   273,   274,   275,
     276,   260,   261,   262,   263,   264,   265,     0,   266,     0,
       0,   267,   268,   269,     0,     0,     0,     0,     0,     0,
       0,   573,     0,     0,     0,     0,     0,     0,   270,   271,
     272,   273,   274,   275,   276,   260,   261,   262,   263,   264,
     265,     0,   266,   665,     0,   267,   268,   269,     0,     0,
     270,   271,   272,   273,   274,   275,   276,   649,   650,     0,
     651,   652,   653,     0,   654,     0,     0,   655,   656,   657,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   285,     0,   270,   271,   272,   273,   274,   275,
     276,   260,   261,   262,   263,   264,   265,     0,   266,     0,
       0,   267,   268,   269,     0,     0,   658,   659,   660,   661,
     662,   663,   664,   260,   261,   262,   263,   264,   265,     0,
     266,     0,     0,   267,   268,   269,   260,   261,   262,   263,
     264,   265,     0,   266,     0,     0,   267,   268,   269,     0,
     270,   271,   272,   273,   274,   275,   276,   260,   261,   262,
     263,   264,   265,     0,   266,     0,     0,   267,   268,   269,
       0,     0,   270,   271,   272,   273,   274,   275,   276,     0,
     361,     0,   362,     0,     0,   270,   271,   272,   273,   274,
     275,   276,     0,   461,     0,   362,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   270,   271,   272,   273,
     274,   275,   276,     0,     0,     0,   362,   397,   398,   399,
     400,   401,   402,     0,   403,     0,     0,   404,   405,   406,
     397,   398,   399,   400,   401,   402,     0,   403,     0,     0,
     404,   405,   406,   397,   398,   399,   400,   401,   402,     0,
     403,     0,     0,   404,   405,   406,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   407,   408,   409,   410,
     411,   412,   413,     0,   414,     0,     0,     0,     0,   407,
     408,   409,   410,   411,   412,   413,     0,   494,     0,     0,
       0,     0,   407,   408,   409,   410,   411,   412,   413,     0,
     588,   260,   261,   262,   263,   264,   265,     0,   266,     0,
       0,   267,   268,   269,     0,     0,     0,     0,     0,   260,
     261,   262,   263,   264,   265,     0,   266,     0,     0,   267,
     268,   269,   260,   261,   262,   263,   264,   265,     0,   266,
       0,     0,   267,   268,   269,     0,     0,     0,     0,     0,
     270,   271,   272,   273,   274,   275,   276,   560,   358,   359,
       0,     0,     0,     0,     0,     0,     0,     0,   270,   271,
     272,   273,   274,   275,   276,     0,     0,     0,     0,     0,
       0,   270,   271,   272,   273,   274,   275,   276,   397,   398,
     399,   400,   401,   402,     0,   403,     0,     0,   404,   405,
     406,   649,   650,     0,   651,   652,   653,     0,   654,     0,
       0,   655,   656,   657,   649,   650,     0,   651,     0,     0,
       0,   654,     0,     0,   655,   656,   657,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   407,   408,   409,
     410,   411,   412,   413,     0,     0,     0,     0,     0,     0,
     658,   659,   660,   661,   662,   663,   664,     0,     0,     0,
       0,     0,     0,   658,   659,   660,   661,   662,   663,   664
};

static const yytype_int16 yycheck[] =
{
       0,     2,     0,    35,     9,   529,   254,   421,    67,    27,
      15,     1,    13,   280,   438,   226,   418,     1,    19,     1,
     434,     1,    18,     1,     1,     1,    27,   166,     1,   277,
     241,    32,    64,    52,    35,     1,   450,     1,   177,   101,
       1,     1,     1,     1,    34,   459,    25,     1,     1,   188,
      25,   592,     1,     1,     1,     1,     1,     1,    33,    34,
      34,   200,   103,    64,    65,    37,   107,    67,    47,     9,
      70,   101,    70,   597,    71,    47,    71,    14,    71,    16,
      99,   100,    79,    20,    79,     0,    79,   226,    25,    25,
      71,   101,   102,   360,    90,    31,    33,    34,    79,   101,
      75,   642,   241,    58,    72,    60,    71,    62,   375,    64,
     102,   101,   103,   361,    79,   639,   101,   101,    31,   101,
      52,   101,   536,   101,   101,   101,    63,   529,   101,   396,
      31,   533,   546,   151,   675,   101,   107,   101,    75,   171,
     101,   101,   101,   101,    39,    40,   147,   101,   101,    89,
     564,   152,   101,   101,   101,   101,   101,   101,   101,   102,
     102,   101,   194,   374,   106,    31,   414,    99,   100,    31,
     171,    52,   101,   102,    55,   101,   102,   178,   101,   180,
     101,   118,   119,   120,   121,   476,   477,   478,   479,   101,
     604,   107,   483,   194,   102,   597,   107,   621,   107,   166,
     106,   202,   106,   140,   141,   101,   143,   144,   145,   146,
     177,   148,   101,   461,   151,    96,    97,    98,    99,   100,
      29,   188,   223,   101,    52,   295,   296,    55,   101,   643,
     497,   101,   169,   200,    52,   374,   254,   639,   101,   506,
      31,   101,   179,   101,   101,   669,   494,   101,   549,   550,
     551,   101,   101,   667,   101,   101,   315,    15,    15,   226,
      52,   528,    29,   102,   101,    18,   203,   681,    96,    97,
      98,    99,   100,    92,   241,   101,   294,   295,   296,    97,
      98,    99,   100,   104,   101,   285,   700,   285,   101,   294,
     295,   296,   104,   294,   295,   296,   710,   711,   101,   101,
     567,   101,    91,   717,   101,    97,    98,    99,   100,    92,
       9,   101,    31,    73,    90,   315,   316,   104,   585,   104,
     101,   101,    37,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     588,    91,    10,    11,     9,   101,    14,    15,   649,   650,
     651,   652,   653,   654,   655,   656,   657,   658,   659,   660,
     661,   662,   663,   664,   101,   302,   104,    73,   369,   636,
     307,    91,   104,   101,   101,    73,   101,    91,   104,   101,
     101,   114,   115,   116,   117,   101,    91,    91,    91,   101,
      58,   328,    60,   330,    62,   332,    64,   334,   699,    91,
     101,   134,   135,   136,   137,   138,   139,   374,    70,   142,
      78,   101,   101,   437,   443,    34,   149,   150,   418,   455,
     533,   421,   115,   116,   117,   582,   363,   364,   365,   366,
     357,   368,   369,    -1,   434,   372,   369,   438,   439,    -1,
      -1,   378,   135,   136,   137,   138,   139,    -1,    -1,    -1,
     450,    -1,    -1,    -1,     1,   455,    -1,    -1,    -1,   459,
     397,   398,   399,   400,   401,   402,   403,   404,   405,   406,
     407,   408,   409,   410,   411,   412,   413,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      37,    -1,    -1,    -1,   431,   432,    -1,    -1,    45,    46,
      47,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   558,    -1,    -1,   529,
      -1,    -1,    -1,   533,    -1,   535,   536,   535,   475,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   546,    94,    95,    96,
      97,    98,    99,   100,   101,    -1,    -1,   558,    -1,    -1,
      -1,     1,    -1,    -1,   564,    -1,    -1,    -1,    -1,    -1,
     507,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   548,   549,   550,   551,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   593,   327,   593,   329,   597,   331,    -1,
     333,    -1,    -1,    -1,   604,    45,    46,    47,    48,    49,
      50,    -1,    52,    -1,    -1,    55,    56,    57,    -1,    -1,
     621,    -1,    -1,   356,   357,   358,   359,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   639,
      -1,    -1,    -1,   643,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,    -1,    -1,    -1,    -1,    -1,   667,   669,   606,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   681,   649,   650,   651,   652,   653,   654,   655,   656,
     657,   658,   659,   660,   661,   662,   663,   664,    -1,    -1,
     700,    -1,   435,    -1,    -1,    -1,    -1,    -1,   645,    -1,
     710,   711,     1,    -1,    -1,    -1,    -1,   717,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,   462,
     463,    -1,   699,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   474,    -1,    -1,    -1,    -1,    -1,   480,    -1,   482,
      -1,    -1,    -1,   486,   487,    -1,    45,    46,    47,    48,
      49,    50,    -1,    52,    -1,    -1,    55,    56,    57,    -1,
      45,    46,    47,    48,    49,    50,    -1,    52,    -1,    -1,
      55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,   101,     1,    -1,   548,   549,   550,   551,    94,
      95,    96,    97,    98,    99,   100,   101,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   570,   571,   572,
     573,    -1,    -1,   576,   577,   578,   579,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    48,    49,    50,    -1,    52,   632,
      -1,    55,    56,    57,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,    -1,    -1,   649,   650,   651,   652,
     653,   654,   655,   656,   657,   658,   659,   660,   661,   662,
     663,   664,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   672,
      94,    95,    96,    97,    98,    99,   100,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,   699,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    -1,    -1,   101,    -1,   103,    -1,   105,     4,
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
     105,    -1,   107,     4,     5,     6,     7,     8,     9,    10,
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
      97,    98,    99,   100,    -1,   102,   103,    -1,   105,     4,
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
      93,    94,    95,    96,    97,    98,    99,   100,    -1,    -1,
     103,    -1,   105,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
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
      93,    -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,     6,
     103,    -1,   105,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    -1,
      37,    38,    39,    40,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    -1,    -1,    90,    -1,    -1,    93,    -1,    45,    46,
      47,    48,    49,    50,   101,    52,    -1,    -1,    55,    56,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   101,     6,    -1,    -1,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    -1,    90,
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
      87,    -1,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
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
      83,    84,    85,    86,    87,    -1,    -1,    -1,    -1,    -1,
      93,    -1,    -1,    -1,    -1,    -1,     6,    -1,   101,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,     6,    -1,    93,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,     6,    -1,    93,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,     6,    -1,    93,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,     6,    -1,    -1,    93,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    -1,    25,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    -1,    37,    -1,    39,    40,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    17,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    84,    85,    -1,    87,    45,    46,    47,
      48,    -1,    93,    37,    52,    -1,    -1,    55,    56,    57,
      -1,    45,    46,    47,    48,    49,    50,    -1,    52,    31,
      -1,    55,    56,    57,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    48,    49,    50,    -1,
      52,    -1,    -1,    55,    56,    57,    94,    95,    96,    97,
      98,    99,   100,    31,    -1,    -1,    -1,    -1,    -1,    37,
      94,    95,    96,    97,    98,    99,   100,    45,    46,    47,
      48,    49,    50,    -1,    52,    31,    -1,    55,    56,    57,
      -1,    37,    94,    95,    96,    97,    98,    99,   100,    45,
      46,    47,    48,    49,    50,    -1,    52,    -1,    -1,    55,
      56,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    37,    94,    95,    96,    97,
      98,    99,   100,    45,    46,    47,    48,    49,    50,    -1,
      52,    31,    -1,    55,    56,    57,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,    45,    46,    47,    48,    49,
      50,    -1,    52,    -1,    -1,    55,    56,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,    45,
      46,    47,    48,    49,    50,    -1,    52,    31,    -1,    55,
      56,    57,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,    45,    46,    47,    48,    49,    50,    -1,    52,    -1,
      -1,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,    45,    46,    47,    48,    49,
      50,    -1,    52,    31,    -1,    55,    56,    57,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,    45,    46,    -1,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    -1,    94,    95,    96,    97,    98,    99,
     100,    45,    46,    47,    48,    49,    50,    -1,    52,    -1,
      -1,    55,    56,    57,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,    45,    46,    47,    48,    49,    50,    -1,
      52,    -1,    -1,    55,    56,    57,    45,    46,    47,    48,
      49,    50,    -1,    52,    -1,    -1,    55,    56,    57,    -1,
      94,    95,    96,    97,    98,    99,   100,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,    -1,
     102,    -1,   104,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,    -1,   102,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,   104,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      45,    46,    47,    48,    49,    50,    -1,    52,    -1,    -1,
      55,    56,    57,    45,    46,    47,    48,    49,    50,    -1,
      52,    -1,    -1,    55,    56,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,    -1,   102,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,    -1,   102,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,    -1,
     102,    45,    46,    47,    48,    49,    50,    -1,    52,    -1,
      -1,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    48,    49,    50,    -1,    52,    -1,    -1,    55,
      56,    57,    45,    46,    47,    48,    49,    50,    -1,    52,
      -1,    -1,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,   101,    84,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,    45,    46,
      47,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    45,    46,    -1,    48,    49,    50,    -1,    52,    -1,
      -1,    55,    56,    57,    45,    46,    -1,    48,    -1,    -1,
      -1,    52,    -1,    -1,    55,    56,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100
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
      62,    64,    78,   150,     1,    34,   101,   121,   122,   123,
       4,     5,     7,     8,    10,    11,    12,    13,    14,    15,
      20,    29,    51,    58,    60,    62,    64,    78,    83,    97,
      98,   103,   105,   121,   128,   130,   146,   147,   150,   152,
     154,   157,   158,   162,   165,   166,   167,   170,   150,   103,
     167,   121,     1,   101,   167,    34,   128,   167,    39,    40,
      78,   121,   147,   149,   160,   101,   121,   128,   167,   128,
     130,   122,   167,   122,   121,   101,   126,     0,   111,   128,
     167,   101,   102,   103,   153,   166,   153,   166,   153,   166,
     153,   167,   167,   167,   167,    72,     1,   101,   101,     1,
     101,     1,   101,   102,   153,   153,   167,   167,   166,    66,
      67,    68,    69,    70,    72,    77,   121,   167,   166,   166,
     104,   147,   167,     4,     5,     7,     8,   107,   121,   163,
     164,   167,   171,   172,   103,     1,   101,     1,   101,    31,
      45,    46,    47,    48,    49,    50,    52,    55,    56,    57,
      94,    95,    96,    97,    98,    99,   100,   102,   129,   146,
     168,   170,     1,   101,   167,    36,   122,   101,     1,   101,
       1,   101,     1,   101,    26,    53,    54,   121,   167,   121,
      37,    47,   132,     1,   101,     1,   101,    39,     1,   101,
       1,   101,     1,   101,     1,    37,   101,     1,   101,   122,
       1,   101,   121,   104,   167,    31,    31,    31,    37,    31,
      37,    31,    37,    31,    37,    58,    60,    62,    64,   151,
     101,   101,   101,   121,    31,   168,    71,    79,    71,    79,
      71,    79,    71,    79,    71,    79,    31,    31,    84,    85,
     168,   102,   104,   107,   107,   107,   107,   106,   107,   102,
     106,   106,   102,   104,   147,   172,   101,   101,    29,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   172,    45,    46,    47,
      48,    49,    50,    52,    55,    56,    57,    94,    95,    96,
      97,    98,    99,   100,   102,   129,   101,     1,   101,   124,
       1,   101,   101,   101,   101,    78,   147,   148,   150,   148,
     148,    25,    47,     1,   101,    31,   167,   101,     9,    89,
     101,   114,   115,   116,   117,   101,   167,   101,   101,   101,
     101,   126,   101,   124,   140,   141,   142,   101,     1,   101,
     101,   102,    15,    15,   166,   167,   166,   167,   166,   167,
     166,   167,    25,    31,   159,    29,    59,    61,    63,    65,
      73,    74,    75,    76,    80,    81,    82,    83,   155,   156,
     166,   155,   166,   166,   102,   129,   104,   172,   167,   167,
     167,   167,   167,   164,   167,   167,   168,   102,   129,   167,
     129,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   172,   101,
     101,   125,   143,   144,   145,    18,   101,   140,     1,   101,
       1,   101,     1,   101,   167,   167,   101,   140,    73,    75,
      82,    83,   161,   166,     1,   101,   115,   119,   121,   117,
     101,   140,    92,   142,   101,   140,   104,   172,   166,   166,
      31,    31,    31,    31,   166,   167,   159,   159,   159,   159,
     166,   166,   159,   166,   166,   172,   104,   129,   102,   129,
     104,   129,   143,    18,    90,   136,   145,   101,   124,   140,
     101,   101,   101,     1,   101,    17,    37,   140,    91,   166,
     169,   170,   169,   169,   169,   101,   101,   122,    92,   101,
     140,     9,   129,   166,   166,   166,   166,   166,   166,   166,
     166,    31,    73,   156,   129,   104,   172,   104,   136,   101,
     124,   101,   143,   101,   140,    37,   167,    91,   101,    45,
      46,    48,    49,    50,    52,    55,    56,    57,    94,    95,
      96,    97,    98,    99,   100,    31,     1,   101,   101,     9,
     119,   104,    73,   104,   129,   143,   136,   140,    91,   167,
       1,   101,   101,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,    73,
     101,   140,   119,   101,   166,   104,   136,    91,   101,     1,
     101,   101,   140,   169,   140,   101,   101,   101,   140,   140,
      91,   140,    91,    91,   101,    91,   101,   101,   101
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   108,   109,   110,   110,   111,   111,   111,   111,   111,
     111,   112,   112,   113,   113,   114,   114,   115,   115,   116,
     116,   117,   117,   117,   118,   118,   118,   118,   119,   119,
     119,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     122,   122,   122,   123,   123,   124,   124,   125,   125,   125,
     125,   125,   125,   126,   126,   126,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   129,   129,   130,   130,
     130,   130,   130,   131,   131,   131,   131,   131,   131,   131,
     131,   132,   132,   133,   133,   133,   133,   133,   133,   134,
     134,   134,   135,   135,   135,   135,   136,   136,   137,   137,
     137,   137,   137,   137,   137,   137,   138,   138,   138,   139,
     140,   140,   141,   141,   142,   142,   143,   143,   144,   144,
     145,   145,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   147,   148,   148,
     149,   149,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   151,   151,   151,
     151,   152,   152,   153,   153,   153,   153,   154,   154,   154,
     154,   154,   155,   155,   155,   155,   155,   156,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   158,   158,
     158,   158,   158,   158,   158,   158,   159,   159,   160,   160,
     161,   161,   161,   161,   161,   162,   162,   162,   163,   163,
     163,   164,   164,   164,   164,   164,   165,   165,   166,   166,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   170,   170,   171,   171,
     172,   172
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       3,     5,     6,     4,     5,     5,     6,     0,     1,     1,
       2,     1,     1,     3,     8,     9,     5,     6,     0,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     2,     1,     3,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     4,     3,     4,
       3,     4,     3,     4,     3,     4,     3,     4,     3,     4,
       5,     3,     4,     3,     4,     2,     3,     2,     3,     3,
       4,     3,     4,     3,     4,     2,     0,     2,     4,     3,
       5,     2,     4,     6,     5,     7,     0,     1,     2,     2,
       5,     4,     3,     5,     6,     5,     6,     5,     6,     5,
       6,     1,     1,     3,     4,     3,     4,     3,     4,     4,
       6,     7,     6,     8,     7,     9,     0,     2,     7,     8,
      11,    12,    12,    13,     9,    10,     4,     6,     7,     2,
       0,     1,     1,     2,     1,     1,     0,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     2,     3,     5,     6,
       8,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     5,     2,     5,     2,     2,     4,     6,
       4,     6,     4,     6,     4,     6,     5,     1,     1,     1,
       1,     2,     2,     1,     2,     4,     6,     2,     4,     4,
       1,     1,     1,     1,     5,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     6,     6,
       6,     6,     6,     4,     4,     4,     1,     1,     2,     4,
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
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2447 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2453 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2459 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2465 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_script: /* script  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2471 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_scriptpart: /* scriptpart  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2477 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_macro: /* macro  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2483 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_factory: /* factory  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2489 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_method: /* method  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2495 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_methodlistline: /* methodlistline  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2501 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_handler: /* handler  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2507 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2513 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2519 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmt: /* stmt  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2525 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmt_insideif: /* stmt_insideif  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2531 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtoneliner: /* stmtoneliner  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2537 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_proc: /* proc  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2543 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_asgn: /* asgn  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2549 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_definevars: /* definevars  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2555 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ifstmt: /* ifstmt  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2561 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ifelsestmt: /* ifelsestmt  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2567 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_loop: /* loop  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2573 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tell: /* tell  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2579 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_when: /* when  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2585 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtlistline: /* stmtlistline  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2591 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtlistline_insideif: /* stmtlistline_insideif  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2597 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_simpleexpr_nounarymath: /* simpleexpr_nounarymath  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2603 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_var: /* var  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2609 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_varorchunk: /* varorchunk  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2615 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_varorthe: /* varorthe  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2621 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_chunk: /* chunk  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2627 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_object: /* object  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2633 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_the: /* the  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2639 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_theobj: /* theobj  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2645 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_menu: /* menu  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2651 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_thedatetime: /* thedatetime  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2657 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_thenumberof: /* thenumberof  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2663 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_writablethe: /* writablethe  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2669 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_writabletheobj: /* writabletheobj  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2675 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_list: /* list  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2681 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_proppair: /* proppair  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2687 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_unarymath: /* unarymath  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2693 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_simpleexpr: /* simpleexpr  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2699 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr: /* expr  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2705 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr_nounarymath: /* expr_nounarymath  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2711 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr_noeq: /* expr_noeq  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2717 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_sprite: /* sprite  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2723 "engines/director/lingo/lingo-gr.cpp"
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
#line 225 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->_compiler->_assemblyAST = Common::SharedPtr<Node>(new ScriptNode((yyvsp[0].nodelist))); (yyval.node) = nullptr; }
#line 2993 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 227 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3004 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 233 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3014 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 240 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3020 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* scriptpart: tENDCLAUSE endargdef '\n'  */
#line 245 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 3026 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 274 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 3032 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* macro: tMACRO ID idlist error '\n' stmtlist  */
#line 275 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-4].s), (yyvsp[-3].idlist), (yyvsp[0].nodelist))); }
#line 3038 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* factory: tFACTORY ID '\n' methodlist  */
#line 280 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 3044 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* factory: tFACTORY ID error '\n' methodlist  */
#line 281 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new FactoryNode((yyvsp[-3].s), (yyvsp[0].nodelist))); }
#line 3050 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 3056 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* method: tMETHOD ID idlist error '\n' stmtlist  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-4].s), (yyvsp[-3].idlist), (yyvsp[0].nodelist))); }
#line 3062 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlist: %empty  */
#line 288 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3068 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* nonemptymethodlist: methodlistline  */
#line 292 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3089 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* methodlistline: '\n'  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3095 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* methodlistline: tENDCLAUSE endargdef '\n'  */
#line 307 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 3101 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 3110 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* handler: tON ID idlist error '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                                                      {	// D3
		TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-7].s), (yyvsp[-6].idlist), (yyvsp[-3].nodelist)));
		checkEnd((yyvsp[-2].s), (yyvsp[-7].s), false);
		delete (yyvsp[-2].s); }
#line 3119 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* handler: tON ID idlist '\n' stmtlist  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 3126 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* handler: tON ID idlist error '\n' stmtlist  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                            {	// D4. No 'end' clause
		TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-4].s), (yyvsp[-3].idlist), (yyvsp[0].nodelist))); }
#line 3133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* endargdef: ID  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3139 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* endargdef: endargdef ',' ID  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3145 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tABBREVIATED  */
#line 336 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 3151 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tABBREV  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 3157 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tABBR  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 3163 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tAFTER  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 3169 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tBEFORE  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 3175 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tCAST  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 3181 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tCASTLIB  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("castLib"); }
#line 3187 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tCHAR  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 3193 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tCHARS  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 3199 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tDATE  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 3205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tDELETE  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 3211 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tDOWN  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 3217 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tFIELD  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 3223 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tFRAME  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 3229 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tHILITE  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 3235 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tIN  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 3241 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tINTERSECTS  */
#line 352 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 3247 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tINTO  */
#line 353 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 3253 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tITEM  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 3259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tITEMS  */
#line 355 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 3265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tLAST  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 3271 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tLINE  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 3277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tLINES  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 3283 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tLONG  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 3289 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tMEMBER  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("member"); }
#line 3295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tMENU  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 3301 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tMENUITEM  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 3307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tMENUITEMS  */
#line 363 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 3313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tMOVIE  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 3319 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tNEXT  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 3325 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tNUMBER  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 3331 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tOF  */
#line 367 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 3337 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tPREVIOUS  */
#line 368 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 3343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tREPEAT  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 3349 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tSCRIPT  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 3355 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tASSERTERROR  */
#line 371 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 3361 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tSHORT  */
#line 372 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 3367 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tSOUND  */
#line 373 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 3373 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tSPRITE  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 3379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tTHE  */
#line 375 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 3385 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tTIME  */
#line 376 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 3391 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* CMDID: tTO  */
#line 377 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 3397 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* CMDID: tWHILE  */
#line 378 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 3403 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* CMDID: tWINDOW  */
#line 379 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 3409 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* CMDID: tWITH  */
#line 380 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 3415 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* CMDID: tWITHIN  */
#line 381 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 3421 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* CMDID: tWORD  */
#line 382 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 3427 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* CMDID: tWORDS  */
#line 383 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 3433 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tELSE  */
#line 387 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 3439 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tENDCLAUSE  */
#line 388 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); delete (yyvsp[0].s); }
#line 3445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tEXIT  */
#line 389 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 3451 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tFACTORY  */
#line 390 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 3457 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tGLOBAL  */
#line 391 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 3463 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tGO  */
#line 392 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 3469 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tIF  */
#line 393 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 3475 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tINSTANCE  */
#line 394 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 3481 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tMACRO  */
#line 395 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3487 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tMETHOD  */
#line 396 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3493 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tON  */
#line 397 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3499 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tOPEN  */
#line 398 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3505 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ID: tPLAY  */
#line 399 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3511 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* ID: tPROPERTY  */
#line 400 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3517 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* ID: tPUT  */
#line 401 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3523 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* ID: tRETURN  */
#line 402 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("return"); }
#line 3529 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* ID: tSET  */
#line 403 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3535 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* ID: tTELL  */
#line 404 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3541 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* ID: tTHEN  */
#line 405 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3547 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* idlist: %empty  */
#line 408 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3553 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* nonemptyidlist: ID  */
#line 413 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3562 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3570 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* stmt: tENDIF '\n'  */
#line 428 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: CMDID cmdargs '\n'  */
#line 444 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: CMDID cmdargs error '\n'  */
#line 445 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new CmdNode((yyvsp[-3].s), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tPUT cmdargs '\n'  */
#line 446 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3594 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tPUT cmdargs error '\n'  */
#line 447 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("put"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3600 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tGO cmdargs '\n'  */
#line 448 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3606 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tGO cmdargs error '\n'  */
#line 449 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("go"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3612 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* proc: tGO frameargs '\n'  */
#line 450 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proc: tGO frameargs error '\n'  */
#line 451 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("go"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3624 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tPLAY cmdargs '\n'  */
#line 452 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3630 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proc: tPLAY cmdargs error '\n'  */
#line 453 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("play"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* proc: tPLAY frameargs '\n'  */
#line 454 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3642 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* proc: tPLAY frameargs error '\n'  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("play"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3648 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* proc: tOPEN cmdargs '\n'  */
#line 456 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3654 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* proc: tOPEN cmdargs error '\n'  */
#line 457 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("open"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3660 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* proc: tOPEN expr tWITH expr '\n'  */
#line 458 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3670 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* proc: tNEXT tREPEAT '\n'  */
#line 463 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3676 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* proc: tNEXT tREPEAT error '\n'  */
#line 464 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new NextRepeatNode()); }
#line 3682 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* proc: tEXIT tREPEAT '\n'  */
#line 465 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3688 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* proc: tEXIT tREPEAT error '\n'  */
#line 466 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new ExitRepeatNode()); }
#line 3694 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* proc: tEXIT '\n'  */
#line 467 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3700 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* proc: tEXIT error '\n'  */
#line 468 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new ExitNode()); }
#line 3706 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* proc: tRETURN '\n'  */
#line 469 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ReturnNode(nullptr); }
#line 3712 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* proc: tRETURN error '\n'  */
#line 470 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new ReturnNode(nullptr)); }
#line 3718 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* proc: tRETURN expr '\n'  */
#line 471 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ReturnNode((yyvsp[-1].node)); }
#line 3724 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* proc: tRETURN expr error '\n'  */
#line 472 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new ReturnNode((yyvsp[-2].node))); }
#line 3730 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* proc: tDELETE chunk '\n'  */
#line 473 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3736 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* proc: tDELETE chunk error '\n'  */
#line 474 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new DeleteNode((yyvsp[-2].node))); }
#line 3742 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* proc: tHILITE chunk '\n'  */
#line 475 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3748 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* proc: tHILITE chunk error '\n'  */
#line 476 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new HiliteNode((yyvsp[-2].node))); }
#line 3754 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* proc: tASSERTERROR stmtoneliner  */
#line 477 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3760 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* cmdargs: %empty  */
#line 480 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3768 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* cmdargs: expr trailingcomma  */
#line 483 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3778 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 488 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd arg, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3787 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 492 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3798 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 498 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3808 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* cmdargs: '(' ')'  */
#line 503 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3816 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* cmdargs: '(' expr ',' ')'  */
#line 506 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3826 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 511 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3835 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* cmdargs: '(' var expr_nounarymath trailingcomma ')'  */
#line 515 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3846 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* cmdargs: '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 521 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3856 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* frameargs: tFRAME expr  */
#line 534 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3866 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* frameargs: tMOVIE expr  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3877 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 545 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3888 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* frameargs: expr tOF tMOVIE expr  */
#line 551 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* frameargs: tFRAME expr expr_nounarymath  */
#line 557 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3910 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 565 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* asgn: tPUT expr tINTO varorchunk error '\n'  */
#line 566 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new PutIntoNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3922 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 567 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* asgn: tPUT expr tAFTER varorchunk error '\n'  */
#line 568 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new PutAfterNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3934 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 569 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3940 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* asgn: tPUT expr tBEFORE varorchunk error '\n'  */
#line 570 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new PutBeforeNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* asgn: tSET varorthe to expr '\n'  */
#line 571 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3952 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* asgn: tSET varorthe to expr error '\n'  */
#line 572 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new SetNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3958 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* definevars: tGLOBAL idlist '\n'  */
#line 577 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3964 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* definevars: tGLOBAL idlist error '\n'  */
#line 578 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new GlobalNode((yyvsp[-2].idlist))); }
#line 3970 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* definevars: tPROPERTY idlist '\n'  */
#line 579 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3976 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* definevars: tPROPERTY idlist error '\n'  */
#line 580 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new PropertyNode((yyvsp[-2].idlist))); }
#line 3982 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* definevars: tINSTANCE idlist '\n'  */
#line 581 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3988 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* definevars: tINSTANCE idlist error '\n'  */
#line 582 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new InstanceNode((yyvsp[-2].idlist))); }
#line 3994 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* ifstmt: tIF expr tTHEN stmt  */
#line 585 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 4003 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 589 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 4010 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* ifstmt: tIF expr tTHEN error '\n' stmtlist_insideif endif  */
#line 591 "engines/director/lingo/lingo-gr.y"
                                                            {
		TRIM_GARBAGE((yyval.node), new IfStmtNode((yyvsp[-5].node), (yyvsp[-1].nodelist))); }
#line 4017 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 595 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 4028 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 601 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 4037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 605 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 4046 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 609 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 4053 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* endif: %empty  */
#line 613 "engines/director/lingo/lingo-gr.y"
                        {
		LingoCompiler *compiler = g_lingo->_compiler;
		warning("LingoCompiler::parse: no end if at line %d col %d in %s id: %d",
			compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
			compiler->_assemblyContext->_id);

		}
#line 4065 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 622 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4072 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189: /* loop: tREPEAT tWHILE expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 624 "engines/director/lingo/lingo-gr.y"
                                                                  {
		TRIM_GARBAGE((yyval.node), new RepeatWhileNode((yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 626 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4086 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 191: /* loop: tREPEAT tWITH ID tEQ expr tTO expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 628 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		TRIM_GARBAGE((yyval.node), new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), false, (yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4093 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 630 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 632 "engines/director/lingo/lingo-gr.y"
                                                                                                   {
		TRIM_GARBAGE((yyval.node), new RepeatWithToNode((yyvsp[-10].s), (yyvsp[-8].node), true, (yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4107 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 194: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 634 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4114 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* loop: tREPEAT tWITH ID tIN expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 636 "engines/director/lingo/lingo-gr.y"
                                                                        {
		TRIM_GARBAGE((yyval.node), new RepeatWithInNode((yyvsp[-7].s), (yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4121 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* tell: tTELL expr tTO stmtoneliner  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 4130 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4137 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* tell: tTELL expr error '\n' stmtlist tENDTELL '\n'  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                        {
		TRIM_GARBAGE((yyval.node), new TellNode((yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4144 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* when: tWHEN '\n'  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WhenNode((yyvsp[-1].w).eventName, (yyvsp[-1].w).stmt); }
#line 4150 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* stmtlist: %empty  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 4156 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* nonemptystmtlist: stmtlistline  */
#line 657 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 4167 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 663 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 4177 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* stmtlistline: '\n'  */
#line 670 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 4183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* stmtlist_insideif: %empty  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 4189 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 679 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 4200 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 685 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 4210 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* stmtlistline_insideif: '\n'  */
#line 692 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 4216 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* simpleexpr_nounarymath: tINT  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 4222 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* simpleexpr_nounarymath: tFLOAT  */
#line 700 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 4228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* simpleexpr_nounarymath: tSYMBOL  */
#line 701 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 4234 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* simpleexpr_nounarymath: tSTRING  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 4240 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 703 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 4246 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 704 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 4252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 705 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 4258 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath trailingcomma ')'  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.node) = new FuncNode((yyvsp[-5].s), args); }
#line 4269 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.node) = new FuncNode((yyvsp[-7].s), (yyvsp[-2].nodelist)); }
#line 4279 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 221: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 717 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 4285 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* var: ID  */
#line 725 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 4291 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* chunk: tFIELD refargs  */
#line 735 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 4297 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* chunk: tCAST simpleexpr tOF tCASTLIB simpleexpr  */
#line 736 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("cast"), args);	}
#line 4307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* chunk: tCAST refargs  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 4313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* chunk: tMEMBER simpleexpr tOF tCASTLIB simpleexpr  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                                           {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("member"), args);	}
#line 4323 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* chunk: tMEMBER refargs  */
#line 747 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("member"), (yyvsp[0].nodelist)); }
#line 4329 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* chunk: tCASTLIB refargs  */
#line 748 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("castLib"), (yyvsp[0].nodelist)); }
#line 4335 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* chunk: tCHAR expr tOF simpleexpr  */
#line 749 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4342 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 751 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4349 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* chunk: tWORD expr tOF simpleexpr  */
#line 753 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4356 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 755 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4363 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* chunk: tITEM expr tOF simpleexpr  */
#line 757 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 759 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4377 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* chunk: tLINE expr tOF simpleexpr  */
#line 761 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4384 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 763 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4391 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 765 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 4397 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* chunktype: tCHAR  */
#line 768 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 4403 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* chunktype: tWORD  */
#line 769 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 4409 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* chunktype: tITEM  */
#line 770 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 4415 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* chunktype: tLINE  */
#line 771 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 4421 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* object: tSCRIPT refargs  */
#line 774 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 4427 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* object: tWINDOW refargs  */
#line 775 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 4433 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* refargs: simpleexpr  */
#line 778 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 4443 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* refargs: '(' ')'  */
#line 783 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 4451 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* refargs: '(' expr ',' ')'  */
#line 786 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 4461 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 791 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4470 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* the: tTHE ID  */
#line 797 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 4476 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* the: tTHE ID tOF theobj  */
#line 798 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 4482 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* the: tTHE tNUMBER tOF theobj  */
#line 799 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 4488 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 264: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 806 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 4494 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 265: /* theobj: tSOUND simpleexpr  */
#line 807 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 4500 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* theobj: tSPRITE simpleexpr  */
#line 808 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 4506 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* menu: tMENU simpleexpr  */
#line 811 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 4512 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 813 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4518 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 814 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4524 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 270: /* thedatetime: tTHE tABBREV tDATE  */
#line 815 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4530 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 271: /* thedatetime: tTHE tABBREV tTIME  */
#line 816 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4536 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* thedatetime: tTHE tABBR tDATE  */
#line 817 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4542 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* thedatetime: tTHE tABBR tTIME  */
#line 818 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4548 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* thedatetime: tTHE tLONG tDATE  */
#line 819 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 4554 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* thedatetime: tTHE tLONG tTIME  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 4560 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* thedatetime: tTHE tSHORT tDATE  */
#line 821 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 4566 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* thedatetime: tTHE tSHORT tTIME  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 4572 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 826 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 4578 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 827 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 4584 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 828 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 4590 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 829 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 4596 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 830 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 4602 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 283: /* thenumberof: tTHE tNUMBER tOF tMENUS  */
#line 831 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenus, nullptr); }
#line 4608 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 284: /* thenumberof: tTHE tNUMBER tOF tXTRAS  */
#line 832 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfXtras, nullptr); }
#line 4614 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* thenumberof: tTHE tNUMBER tOF tCASTLIBS  */
#line 833 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNumberOfNode(kNumberOfCastlibs, nullptr); }
#line 4620 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 288: /* writablethe: tTHE ID  */
#line 838 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 4626 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 289: /* writablethe: tTHE ID tOF writabletheobj  */
#line 839 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 4632 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 291: /* writabletheobj: tMENU expr_noeq  */
#line 843 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 4638 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 292: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 844 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 4644 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* writabletheobj: tSOUND expr_noeq  */
#line 845 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 4650 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 294: /* writabletheobj: tSPRITE expr_noeq  */
#line 846 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 4656 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 295: /* list: '[' exprlist ']'  */
#line 849 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 4662 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 296: /* list: '[' ':' ']'  */
#line 850 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 4668 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* list: '[' proplist ']'  */
#line 851 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 4674 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* proplist: proppair  */
#line 857 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4683 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* proplist: proplist ',' proppair  */
#line 861 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* proplist: proplist ',' expr  */
#line 864 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4699 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 301: /* proppair: tSYMBOL ':' expr  */
#line 869 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 302: /* proppair: ID ':' expr  */
#line 870 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 303: /* proppair: tSTRING ':' expr  */
#line 871 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 304: /* proppair: tINT ':' expr  */
#line 872 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new IntNode((yyvsp[-2].i)), (yyvsp[0].node)); }
#line 4723 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* proppair: tFLOAT ':' expr  */
#line 873 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new FloatNode((yyvsp[-2].f)), (yyvsp[0].node)); }
#line 4729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* unarymath: '+' simpleexpr  */
#line 876 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 4735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 307: /* unarymath: '-' simpleexpr  */
#line 877 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 4741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 312: /* expr: expr '+' expr  */
#line 887 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4747 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 313: /* expr: expr '-' expr  */
#line 888 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 314: /* expr: expr '*' expr  */
#line 889 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 315: /* expr: expr '/' expr  */
#line 890 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4765 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr: expr tMOD expr  */
#line 891 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4771 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr: expr '>' expr  */
#line 892 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4777 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr: expr '<' expr  */
#line 893 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* expr: expr tEQ expr  */
#line 894 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* expr: expr tNEQ expr  */
#line 895 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* expr: expr tGE expr  */
#line 896 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4801 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 322: /* expr: expr tLE expr  */
#line 897 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4807 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 323: /* expr: expr tAND expr  */
#line 898 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 324: /* expr: expr tOR expr  */
#line 899 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4819 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 325: /* expr: expr '&' expr  */
#line 900 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4825 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 326: /* expr: expr tCONCAT expr  */
#line 901 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4831 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 327: /* expr: expr tCONTAINS expr  */
#line 902 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 328: /* expr: expr tSTARTS expr  */
#line 903 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4843 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 331: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 912 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4849 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 332: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 913 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4855 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 333: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 914 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 334: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 915 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4867 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 335: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 916 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4873 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 336: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 917 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4879 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 337: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 918 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4885 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 338: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 919 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4891 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 339: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 920 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4897 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 340: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 921 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4903 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 341: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 922 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4909 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 342: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 923 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4915 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 343: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 924 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4921 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 344: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 925 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4927 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 345: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 926 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4933 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 346: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 927 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 347: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 928 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 350: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 933 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 351: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 934 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 352: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 935 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4963 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 353: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 936 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4969 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 354: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 937 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4975 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 355: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 938 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 356: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 939 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4987 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 357: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 940 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4993 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 358: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 941 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4999 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 359: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 942 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5005 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 360: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 943 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5011 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 361: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 944 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5017 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 362: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 945 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5023 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 363: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 946 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5029 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 364: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 947 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5035 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 365: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 948 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5041 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 366: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 951 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 5047 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 367: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 952 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 5053 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 368: /* exprlist: %empty  */
#line 955 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 5059 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 370: /* nonemptyexprlist: expr  */
#line 959 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 5068 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 371: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 963 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 5076 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 5080 "engines/director/lingo/lingo-gr.cpp"

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

#line 968 "engines/director/lingo/lingo-gr.y"


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
