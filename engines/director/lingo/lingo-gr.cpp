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
  YYSYMBOL_tCASTLIBS = 78,                 /* tCASTLIBS  */
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
  YYSYMBOL_98_n_ = 98,                     /* '\n'  */
  YYSYMBOL_99_ = 99,                       /* ','  */
  YYSYMBOL_100_ = 100,                     /* '('  */
  YYSYMBOL_101_ = 101,                     /* ')'  */
  YYSYMBOL_102_ = 102,                     /* '['  */
  YYSYMBOL_103_ = 103,                     /* ']'  */
  YYSYMBOL_104_ = 104,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 105,                 /* $accept  */
  YYSYMBOL_script = 106,                   /* script  */
  YYSYMBOL_scriptpartlist = 107,           /* scriptpartlist  */
  YYSYMBOL_scriptpart = 108,               /* scriptpart  */
  YYSYMBOL_macro = 109,                    /* macro  */
  YYSYMBOL_factory = 110,                  /* factory  */
  YYSYMBOL_method = 111,                   /* method  */
  YYSYMBOL_methodlist = 112,               /* methodlist  */
  YYSYMBOL_nonemptymethodlist = 113,       /* nonemptymethodlist  */
  YYSYMBOL_methodlistline = 114,           /* methodlistline  */
  YYSYMBOL_handler = 115,                  /* handler  */
  YYSYMBOL_endargdef = 116,                /* endargdef  */
  YYSYMBOL_CMDID = 117,                    /* CMDID  */
  YYSYMBOL_ID = 118,                       /* ID  */
  YYSYMBOL_idlist = 119,                   /* idlist  */
  YYSYMBOL_nonemptyidlist = 120,           /* nonemptyidlist  */
  YYSYMBOL_stmt = 121,                     /* stmt  */
  YYSYMBOL_stmt_insideif = 122,            /* stmt_insideif  */
  YYSYMBOL_stmtoneliner = 123,             /* stmtoneliner  */
  YYSYMBOL_proc = 124,                     /* proc  */
  YYSYMBOL_cmdargs = 125,                  /* cmdargs  */
  YYSYMBOL_trailingcomma = 126,            /* trailingcomma  */
  YYSYMBOL_frameargs = 127,                /* frameargs  */
  YYSYMBOL_asgn = 128,                     /* asgn  */
  YYSYMBOL_to = 129,                       /* to  */
  YYSYMBOL_definevars = 130,               /* definevars  */
  YYSYMBOL_ifstmt = 131,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 132,               /* ifelsestmt  */
  YYSYMBOL_endif = 133,                    /* endif  */
  YYSYMBOL_loop = 134,                     /* loop  */
  YYSYMBOL_tell = 135,                     /* tell  */
  YYSYMBOL_when = 136,                     /* when  */
  YYSYMBOL_stmtlist = 137,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 138,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 139,             /* stmtlistline  */
  YYSYMBOL_stmtlist_insideif = 140,        /* stmtlist_insideif  */
  YYSYMBOL_nonemptystmtlist_insideif = 141, /* nonemptystmtlist_insideif  */
  YYSYMBOL_stmtlistline_insideif = 142,    /* stmtlistline_insideif  */
  YYSYMBOL_simpleexpr_nounarymath = 143,   /* simpleexpr_nounarymath  */
  YYSYMBOL_var = 144,                      /* var  */
  YYSYMBOL_varorchunk = 145,               /* varorchunk  */
  YYSYMBOL_varorthe = 146,                 /* varorthe  */
  YYSYMBOL_chunk = 147,                    /* chunk  */
  YYSYMBOL_chunktype = 148,                /* chunktype  */
  YYSYMBOL_object = 149,                   /* object  */
  YYSYMBOL_refargs = 150,                  /* refargs  */
  YYSYMBOL_the = 151,                      /* the  */
  YYSYMBOL_theobj = 152,                   /* theobj  */
  YYSYMBOL_menu = 153,                     /* menu  */
  YYSYMBOL_thedatetime = 154,              /* thedatetime  */
  YYSYMBOL_thenumberof = 155,              /* thenumberof  */
  YYSYMBOL_inof = 156,                     /* inof  */
  YYSYMBOL_writablethe = 157,              /* writablethe  */
  YYSYMBOL_writabletheobj = 158,           /* writabletheobj  */
  YYSYMBOL_list = 159,                     /* list  */
  YYSYMBOL_proplist = 160,                 /* proplist  */
  YYSYMBOL_proppair = 161,                 /* proppair  */
  YYSYMBOL_unarymath = 162,                /* unarymath  */
  YYSYMBOL_simpleexpr = 163,               /* simpleexpr  */
  YYSYMBOL_expr = 164,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 165,         /* expr_nounarymath  */
  YYSYMBOL_expr_noeq = 166,                /* expr_noeq  */
  YYSYMBOL_sprite = 167,                   /* sprite  */
  YYSYMBOL_exprlist = 168,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 169          /* nonemptyexprlist  */
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
#define YYFINAL  185
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4449

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  105
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  330
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  622

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
      98,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    93,     2,
     100,   101,    96,    94,    99,    95,     2,    97,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   104,     2,
      91,     2,    92,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   102,     2,   103,     2,     2,     2,     2,     2,     2,
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
       0,   208,   208,   210,   216,   223,   224,   225,   226,   227,
     228,   257,   261,   263,   265,   266,   269,   275,   282,   283,
     284,   289,   293,   297,   298,   299,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   376,   377,   378,   381,   385,   395,   396,   399,
     400,   401,   402,   403,   404,   407,   408,   409,   412,   413,
     414,   415,   416,   417,   418,   419,   424,   425,   426,   427,
     428,   429,   432,   435,   440,   444,   450,   455,   458,   463,
     467,   473,   480,   480,   486,   491,   497,   503,   509,   517,
     518,   519,   520,   523,   523,   525,   526,   527,   530,   534,
     538,   544,   548,   552,   556,   563,   565,   567,   569,   571,
     575,   579,   583,   585,   586,   590,   596,   603,   604,   607,
     608,   612,   618,   625,   626,   632,   633,   634,   635,   636,
     637,   638,   639,   645,   650,   651,   652,   653,   654,   655,
     658,   660,   661,   664,   665,   668,   669,   670,   672,   674,
     676,   678,   680,   682,   684,   686,   689,   690,   691,   692,
     695,   696,   699,   704,   707,   712,   718,   719,   720,   721,
     722,   725,   726,   727,   728,   729,   732,   734,   735,   736,
     737,   738,   739,   740,   741,   742,   743,   747,   748,   749,
     750,   751,   752,   753,   754,   757,   757,   759,   760,   763,
     764,   765,   766,   767,   770,   771,   772,   778,   782,   785,
     790,   791,   792,   793,   794,   797,   798,   801,   802,   806,
     807,   808,   809,   810,   811,   812,   813,   814,   815,   816,
     817,   818,   819,   820,   821,   822,   823,   824,   831,   832,
     833,   834,   835,   836,   837,   838,   839,   840,   841,   842,
     843,   844,   845,   846,   847,   848,   849,   852,   853,   854,
     855,   856,   857,   858,   859,   860,   861,   862,   863,   864,
     865,   866,   867,   868,   869,   872,   873,   876,   877,   880,
     884
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

#define YYPACT_NINF (-537)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2849,  -537,  3349,  -537,  -537,  -537,  -537,    41,  -537,   -14,
    -537,  3349,  1962,    41,  2061,  -537,  -537,  3349,  -537,    -3,
    -537,  -537,  2160,    86,  3431,  -537,   -40,  -537,  -537,  3349,
    2160,  1962,  3349,  -537,  -537,  -537,  -537,  -537,  -537,  -537,
    -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,
    -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,
    2061,  3349,  3349,   -29,  3676,  -537,    72,  2849,  -537,  -537,
    -537,  -537,  2160,  -537,  -537,  -537,  -537,  -537,  -537,  -537,
    -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,
    -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,
    -537,  -537,  -537,  -537,  -537,  -537,  -537,   -38,  -537,  -537,
    2259,  2259,  2061,  2061,  2061,  2061,     5,   -13,     7,  -537,
    -537,    16,   -16,  -537,  -537,  -537,  -537,  2259,  2259,  2259,
    2259,  2061,  2061,  2358,  2061,  2061,  2061,  2061,  3513,  2061,
    2358,  2358,  1467,   772,    -5,    34,    40,  -537,  -537,  -537,
    -537,  -537,  -537,  -537,  -537,  -537,  -537,   873,  -537,    42,
    2061,   676,  3349,    45,    55,   972,  3349,  2061,  3349,  -537,
    -537,    -4,  -537,  -537,    58,    61,  1071,    66,    76,    78,
    3873,    89,  3349,  -537,  -537,  -537,  -537,    97,  1170,  -537,
    3349,  1566,  -537,  -537,  -537,   584,   661,  3725,  3747,   129,
    -537,  -537,  -537,  3349,  -537,  -537,  1269,  4257,  -537,   -37,
     -21,   -11,    10,    23,   129,   118,   155,  4201,  -537,  -537,
    -537,  2754,  3908,    96,   100,   102,   103,   110,    11,    70,
    -537,  4257,   111,   117,  1665,  -537,  -537,   191,  2061,  2061,
    2061,  2061,  2061,  2061,  2061,  2061,  2061,  2061,  2061,  2061,
    2061,  2061,  2061,  2061,  2061,  2061,  -537,  -537,  4003,  -537,
    -537,  3943,  2931,   120,  -537,  -537,  3595,  3595,  3595,    12,
     399,   193,  -537,  -537,  2061,     6,  -537,  2061,  -537,  -537,
    -537,  3676,  3013,  -537,   124,  -537,  -537,  -537,  3930,  2358,
    2061,  2358,  2061,  2358,  2061,  2358,  2061,  -537,  -537,  -537,
    -537,    25,  -537,   197,  4270,  -537,  -537,  -537,  -537,  -537,
    -537,  -537,  -537,  -537,  -537,  1368,  2457,  2358,  2358,  4016,
    1764,  -537,  2061,  2061,  2061,  2061,  -537,  2061,  2556,  -537,
    -537,  2061,  -537,  2754,   125,  2061,    24,    24,    24,    24,
    4339,  4339,  -537,    54,    24,    24,    24,    24,    54,    -7,
      -7,  -537,  -537,   125,  2061,  2061,  2061,  2061,  2061,  2061,
    2061,  2061,  2061,  2061,  2061,  2061,  2061,  2061,  2061,  2061,
    2061,  2061,  -537,  3259,   210,  3013,     5,  -537,   130,  -537,
     131,   132,  2061,  2061,  3013,  2655,  4087,  3349,  3349,  -537,
    -537,  -537,     6,  -537,  4100,  -537,  -537,  -537,   142,  3013,
    -537,  3013,  1863,  -537,  3760,  -537,  3782,  -537,  3817,  -537,
    3839,  -537,  -537,  2358,  2061,    25,    25,    25,    25,  2358,
    -537,  2358,    25,  -537,  -537,  2358,  2358,  -537,  -537,  -537,
    -537,  -537,  -537,  2061,   149,  -537,   125,  4257,  4257,  4257,
    4257,  4257,  -537,  4257,  4257,  4029,  2061,   153,  4257,  -537,
      24,    24,    24,    24,  4339,  4339,  -537,    54,    24,    24,
      24,    24,    54,    -7,    -7,  -537,  -537,   125,  -537,  -537,
     -12,  3259,  -537,  3095,  -537,  -537,  -537,  -537,  4113,   514,
     167,  2061,  2061,  2061,  2061,  -537,  -537,  -537,    94,  3349,
    -537,  -537,   159,  -537,   249,  -537,   125,  2358,  2358,  2358,
    2358,  -537,  4257,  2358,  2358,  2358,  2358,  -537,   231,   190,
    -537,  -537,   125,  -537,   160,  2061,   163,  -537,  -537,  3177,
     169,  -537,  -537,  3259,  -537,  3013,   232,  2061,   172,  -537,
    4283,  -537,  3852,  4283,  4283,  -537,   175,  -537,  3349,   164,
    -537,  -537,  -537,  -537,  -537,  -537,  -537,  -537,   204,  2358,
    -537,   179,  -537,   125,  -537,  3259,  -537,  -537,   198,   199,
    2061,  4170,  -537,  2061,  2061,  2061,  2061,  2061,  2061,  2061,
    2061,  2061,  2061,  2061,  2061,  2061,  2061,  2061,  2061,   219,
    3013,    99,  -537,  2358,  -537,   192,   198,  -537,   194,  4183,
    3013,    85,    85,    85,  4352,  4352,  -537,   114,    85,    85,
      85,    85,   114,    48,    48,  -537,  -537,  2061,  -537,  -537,
    -537,  -537,  -537,  -537,  3013,   203,  4283,   206,   201,   202,
    -537,  -537
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    26,    23,    32,    38,    59,    68,    36,    37,     0,
      39,    92,   122,    40,     0,    41,    43,     0,    53,    54,
      56,    57,   122,    58,     0,    66,     0,    69,    67,     0,
     122,   122,    92,    30,    31,    33,    34,    44,    45,    47,
      48,    71,    72,    27,    28,    29,    49,    61,    35,    46,
      50,    51,    52,    55,    64,    65,    62,    63,    42,    70,
       0,    92,     0,     0,    60,     5,     0,     2,     3,     6,
       7,     8,   122,     9,    97,    99,   105,   106,   107,   100,
     101,   102,   103,   104,    75,    36,    74,    76,    78,    79,
      40,    80,    82,    54,    88,    58,    89,    91,    77,    85,
      86,    81,    90,    87,    84,    83,    60,     0,    73,    24,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   118,
      95,     0,    93,   175,   176,   178,   177,    32,    38,    59,
      68,    39,    53,     0,    33,    44,    47,    71,    64,    63,
       0,     0,     0,   327,   190,     0,     0,   267,   185,   186,
     187,   188,   219,   220,   189,   268,   269,   132,   270,     0,
       0,     0,    92,     0,     0,   132,     0,     0,    64,   190,
     193,     0,   194,   162,     0,     0,   132,     0,     0,     0,
       0,     0,    92,    98,   121,     1,     4,     0,   132,    10,
       0,     0,   196,   212,   195,     0,     0,     0,     0,     0,
     119,   117,   145,    94,   210,   211,   134,   135,   179,    27,
      28,    29,    49,    61,    46,    55,   216,     0,   265,   266,
     127,   185,     0,   175,   176,   178,   177,     0,   190,     0,
     257,   329,     0,   328,     0,   110,   111,    56,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   133,   123,   288,   132,   289,
     120,     0,     0,     0,   116,   109,    43,    30,    31,     0,
       0,   247,   143,   144,     0,    14,   114,    69,   112,   113,
     147,     0,   163,   146,     0,   108,    25,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   206,   208,   209,
     207,     0,    96,    56,   138,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,     0,     0,     0,     0,   132,
       0,   184,     0,     0,     0,     0,   255,     0,     0,   256,
     254,     0,   180,   185,   132,     0,   280,   281,   278,   279,
     282,   283,   275,   285,   286,   287,   277,   276,   284,   271,
     272,   273,   274,   132,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   133,   125,   169,   148,   163,    64,   191,     0,   192,
       0,     0,     0,     0,   163,     0,     0,    23,     0,    18,
      19,    12,    15,    16,     0,   160,   167,   168,     0,   164,
     165,   163,     0,   197,     0,   201,     0,   203,     0,   199,
       0,   245,   246,     0,     0,    34,    45,    48,    72,    50,
     242,    51,    52,   243,   244,    62,    63,   218,   222,   221,
     217,   325,   326,   133,     0,   128,   132,   263,   264,   262,
     260,   261,   258,   259,   330,   132,   133,     0,   137,   124,
     299,   300,   297,   298,   301,   302,   294,   304,   305,   306,
     296,   295,   303,   290,   291,   292,   293,   132,   173,   174,
     154,   170,   171,     0,    11,   139,   140,   141,     0,     0,
       0,    50,    51,    62,    63,   248,   249,   142,     0,    92,
      17,   115,     0,   166,    22,   214,   132,     0,     0,     0,
       0,   205,   136,     0,     0,     0,     0,   226,     0,     0,
     224,   225,   132,   130,     0,   133,     0,   181,   126,     0,
       0,   149,   172,   169,   150,   163,     0,     0,     0,   307,
     250,   308,     0,   252,   253,    20,     0,   161,    23,     0,
     198,   202,   204,   200,   237,   239,   240,   238,     0,     0,
     241,     0,   129,   132,   182,   169,   152,   155,   154,     0,
       0,     0,   156,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     163,     0,   215,     0,   131,     0,   154,   151,     0,     0,
     163,   317,   318,   316,   319,   320,   313,   322,   323,   324,
     315,   314,   321,   309,   310,   311,   312,     0,    13,    21,
     223,   183,   153,   159,   163,     0,   251,     0,     0,     0,
     157,   158
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -537,  -537,  -537,   238,  -537,  -537,  -537,  -537,  -537,   -86,
    -537,  -380,     0,     9,   -27,  -537,     1,  -361,   -61,  -537,
      15,  -240,   276,  -537,  -537,  -537,  -537,  -537,  -536,  -537,
    -537,  -537,  -365,  -537,   -89,  -496,  -537,  -160,  -144,   -15,
     -65,  -537,    36,  -537,  -537,   -48,  -537,    -2,  -197,  -537,
    -537,  -250,  -537,  -537,  -537,  -537,    -8,  -537,   168,    -6,
    -192,  -142,   -64,  -537,  -232
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    66,    67,    68,    69,    70,   390,   391,   392,   393,
      71,   107,   108,   144,   121,   122,   397,    74,    75,    76,
     145,   256,   146,    77,   274,    78,    79,    80,   521,    81,
      82,    83,   398,   399,   400,   470,   471,   472,   147,   148,
     378,   171,   149,   301,   150,   192,   151,   427,   428,   152,
     153,   413,   172,   485,   154,   229,   230,   155,   156,   231,
     258,   530,   158,   232,   233
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      72,    73,   334,   184,   519,   179,   157,   488,   161,   170,
     474,   109,   469,   257,   304,   387,   165,   118,   372,   480,
     120,   257,   587,   353,   176,   157,   162,   558,   163,   319,
     272,   305,   257,   169,   181,   382,   494,   164,   174,   306,
     273,   120,   244,   117,   257,   175,   177,   307,   411,   159,
     612,   110,   111,   412,   180,   308,   383,   309,   173,   586,
     189,   190,   257,   194,    72,   310,   188,    72,    73,   183,
     120,   182,   185,   244,   199,   520,   245,   257,   311,   434,
     194,   204,   205,   203,   119,   200,   312,   187,   436,   253,
     254,   313,   388,   259,   447,   234,   112,   568,   113,   314,
     114,   259,   115,   244,   389,   201,   195,   196,   197,   198,
     469,   234,   259,   449,   202,   327,   116,   250,   251,   252,
     253,   254,   166,   167,   259,   206,   207,   221,   195,   196,
     197,   198,   235,   217,   568,   263,   222,   569,   236,   467,
     260,   445,   259,   264,   577,   578,   315,   216,   251,   252,
     253,   254,   228,   265,   261,   284,   275,   259,   581,   276,
     559,   270,   469,   568,   278,   503,   504,   505,   506,   328,
     496,   120,   509,   329,   279,   269,   280,   271,   574,   575,
     576,   577,   578,   316,   297,   288,   298,   283,   299,   257,
     300,   120,   535,   190,   469,   285,   514,   609,   190,   286,
     322,   512,   380,   381,   323,   516,   324,   325,   575,   576,
     577,   578,   302,   326,   330,   608,   331,   335,   375,   333,
     395,   385,   401,   414,   446,   615,   473,   518,   475,   476,
     477,   492,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   617,
     513,   377,   377,   377,   517,   528,   539,   537,   538,   548,
     549,   552,    72,   374,   554,   582,   560,   557,   386,   259,
     562,   394,   551,   580,   583,   169,   169,   169,   193,   193,
     584,    72,    72,   553,   404,   520,   406,   588,   408,   607,
     410,   618,   613,   611,   619,   193,   193,   193,   193,   620,
     621,   208,   379,   379,   379,   186,   490,   178,   218,   219,
     493,   522,   550,   585,   430,     0,   437,   438,   439,   440,
     442,   441,   443,     0,     0,   444,     0,     0,     0,   448,
       0,     0,     0,     0,     0,     0,     0,   228,     0,     0,
     532,   533,   534,     0,     0,     0,     0,     0,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,     0,     0,     0,     0,     0,
       0,     0,     0,    72,     0,    72,   478,   479,     0,     0,
       0,     0,     0,     0,    72,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   109,   489,     0,    72,
       0,    72,     0,     0,     0,     0,     0,     0,   502,     0,
       0,     0,     0,     0,     0,     0,     0,   531,   531,   531,
     531,   591,   592,   593,   594,   595,   596,   597,   598,   599,
     600,   601,   602,   603,   604,   605,   606,     0,     0,     0,
     444,   238,   239,   240,   241,   242,   243,     0,   244,     0,
       0,   245,   246,   247,     0,     0,     0,   403,     0,   405,
       0,   407,   536,   409,     0,   616,     0,     0,     0,     0,
       0,    72,     0,    72,   524,     0,     0,     0,     0,     0,
       0,     0,     0,   429,   429,   431,   432,     0,     0,     0,
     248,   249,   250,   251,   252,   253,   254,   384,   120,   531,
     531,   531,   531,   531,   531,   531,   531,   531,   531,   531,
     531,   531,   531,   531,   531,     0,     0,     0,     0,    72,
     556,   561,     0,    72,     0,    72,     0,     0,     0,   526,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   531,     0,     0,     0,   109,   527,     0,
       0,     0,     0,   486,   589,    72,   238,   239,   240,   241,
     242,   243,     0,   244,     0,     0,   245,   246,   247,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      72,   501,     0,     0,     0,     0,     0,   507,     0,   508,
      72,     0,     0,   510,   511,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   248,   249,   250,   251,   252,
     253,   254,   289,     0,    72,     0,     0,     0,   290,     0,
       0,     0,     0,     0,     0,     0,   238,   239,   240,   241,
     242,   243,     0,   244,     0,     0,   245,   246,   247,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   529,
     529,   529,   529,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   540,   541,   542,   543,     0,
       0,   544,   545,   546,   547,   248,   249,   250,   251,   252,
     253,   254,     0,     0,     0,     0,     0,     0,     0,   291,
       0,     0,     0,     0,     0,   292,     0,     0,     0,     0,
       0,     0,     0,   238,   239,   240,   241,   242,   243,   262,
     244,     0,     0,   245,   246,   247,     0,   507,   238,   239,
     240,   241,   242,   243,     0,   244,     0,     0,   245,   246,
     247,   529,   529,   529,   529,   529,   529,   529,   529,   529,
     529,   529,   529,   529,   529,   529,   529,     0,     0,     0,
       0,   610,   248,   249,   250,   251,   252,   253,   254,     0,
       0,     0,     0,     0,     0,     0,     0,   248,   249,   250,
     251,   252,   253,   254,     0,   529,   223,   224,     1,   225,
     226,    84,   127,   128,   129,   130,    85,     8,    86,    87,
      10,    88,    89,    90,    91,    15,    16,    92,    18,    93,
      20,    21,    94,    95,    96,    97,    25,     0,    27,    28,
      98,    99,   100,   101,     0,     0,     0,     0,     0,     0,
     133,     0,    33,    34,     0,     0,     0,   134,    36,   135,
      38,   136,    40,   137,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,   138,    55,     0,
       0,    56,   139,    58,    59,   102,   103,   104,   105,     0,
       0,     0,   106,     0,     0,     0,   140,   141,     0,     0,
       0,     0,   160,     0,   143,     0,   227,   123,   124,     1,
     125,   126,    84,   127,   128,   129,   130,    85,     8,    86,
      87,    10,    88,    89,    90,    91,    15,    16,    92,    18,
      93,   237,    21,    94,    95,    96,    97,    25,     0,    27,
      28,    98,    99,   100,   101,   238,   239,   240,   241,   242,
     243,   133,   244,    33,    34,   245,   246,   247,   134,    36,
     135,    38,   136,    40,   137,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,   138,    55,
       0,     0,    56,   139,    58,    59,   102,   103,   104,   105,
       0,     0,     0,   106,   248,   249,   250,   251,   252,   253,
     254,     0,   255,   160,     0,   143,   123,   124,     1,   125,
     126,    84,   127,   128,   129,   130,    85,     8,    86,    87,
      10,    88,    89,    90,    91,    15,   266,    92,    18,    93,
      20,    21,    94,    95,    96,    97,    25,     0,    27,    28,
      98,    99,   100,   101,   238,   239,   240,   241,   242,   243,
     133,   244,   267,   268,   245,   246,   247,   134,    36,   135,
      38,   136,    40,   137,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,   138,    55,     0,
       0,    56,   139,    58,    59,   102,   103,   104,   105,     0,
       0,     0,   106,   248,   249,   250,   251,   252,   253,   254,
       0,   255,   160,     0,   143,   123,   124,     1,   125,   126,
      84,   127,   128,   129,   130,    85,     8,    86,    87,    10,
      88,    89,    90,    91,    15,    16,    92,    18,    93,    20,
      21,    94,    95,    96,    97,    25,     0,   277,    28,    98,
      99,   100,   101,   238,   239,   240,   241,   242,   243,   133,
     244,    33,    34,   245,   246,   247,   134,    36,   135,    38,
     136,    40,   137,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,   138,    55,     0,     0,
      56,   139,    58,    59,   102,   103,   104,   105,     0,     0,
       0,   106,   248,   249,   250,   251,   252,   253,   254,     0,
     255,   160,     0,   143,   123,   124,     1,   125,   126,    84,
     127,   128,   129,   130,    85,     8,    86,    87,    10,    88,
      89,    90,    91,    15,    16,    92,    18,    93,    20,    21,
      94,    95,    96,    97,    25,     0,    27,    28,    98,    99,
     100,   101,   238,   239,   240,   241,   242,   243,   133,   244,
      33,    34,   245,   246,   247,   134,    36,   135,    38,   136,
      40,   137,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,   138,    55,     0,     0,    56,
     139,    58,    59,   102,   103,   104,   105,     0,     0,     0,
     106,   248,   249,   250,   251,   252,   253,   254,     0,   255,
     160,     0,   143,   123,   124,     1,   125,   126,    84,   127,
     128,   129,   130,    85,     8,    86,    87,    10,    88,    89,
      90,    91,    15,    16,    92,    18,    93,   303,    21,    94,
      95,    96,    97,    25,     0,    27,    28,    98,    99,   100,
     101,   238,   239,   240,   241,   242,   243,   133,   244,    33,
      34,   245,   246,   247,   134,    36,   135,    38,   136,    40,
     137,    42,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,   138,    55,     0,     0,    56,   139,
      58,    59,   102,   103,   104,   105,     0,     0,     0,   106,
     248,   249,   250,   251,   252,   253,   254,     0,     0,   160,
       0,   143,   123,   124,     1,   125,   126,    84,   127,   128,
     129,   130,    85,     8,    86,    87,    10,    88,    89,    90,
      91,    15,    16,    92,    18,    93,    20,    21,    94,    95,
      96,    97,    25,     0,    27,    28,    98,    99,   100,   101,
       0,     0,     0,     0,     0,     0,   133,     0,    33,    34,
       0,     0,     0,   134,   415,   135,   416,   136,   417,   137,
     418,    43,    44,    45,    46,    47,    48,    49,   419,   420,
     421,   422,    53,   138,    55,   423,   424,   425,   426,    58,
      59,   102,   103,   104,   105,     0,     0,     0,   106,     0,
       0,     0,   140,   141,     0,     0,     0,     0,   160,     0,
     143,   123,   124,     1,   125,   126,    84,   127,   128,   129,
     130,    85,     8,    86,    87,    10,    88,    89,    90,    91,
      15,    16,    92,    18,    93,    20,    21,    94,    95,    96,
      97,    25,     0,    27,    28,    98,    99,   100,   101,     0,
       0,     0,     0,     0,     0,   133,     0,    33,    34,     0,
       0,     0,   134,    36,   135,    38,   136,    40,   137,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,   138,    55,     0,     0,    56,   139,    58,    59,
     102,   103,   104,   105,     0,     0,     0,   106,     0,     0,
       0,   140,   141,     0,     0,     0,     0,   160,   220,   143,
     123,   124,     1,   125,   126,    84,   127,   128,   129,   130,
      85,     8,    86,    87,    10,    88,    89,    90,    91,    15,
      16,    92,    18,    93,    20,    21,    94,    95,    96,    97,
      25,     0,    27,    28,    98,    99,   100,   101,     0,     0,
       0,     0,     0,     0,   133,     0,    33,    34,     0,     0,
       0,   134,    36,   135,    38,   136,    40,   137,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,   138,    55,     0,     0,    56,   139,    58,    59,   102,
     103,   104,   105,     0,     0,     0,   106,     0,     0,     0,
     140,   141,     0,     0,     0,     0,   160,   287,   143,   123,
     124,     1,   125,   126,    84,   127,   128,   129,   130,    85,
       8,    86,    87,    10,    88,    89,    90,    91,    15,    16,
      92,    18,    93,    20,    21,    94,    95,    96,    97,    25,
       0,    27,    28,    98,    99,   100,   101,     0,     0,     0,
       0,     0,     0,   133,     0,    33,    34,     0,     0,     0,
     134,    36,   135,    38,   136,    40,   137,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
     138,    55,     0,     0,    56,   139,    58,    59,   102,   103,
     104,   105,     0,     0,     0,   106,     0,     0,     0,   140,
     141,     0,     0,     0,     0,   160,   332,   143,   123,   124,
       1,   125,   126,    84,   127,   128,   129,   130,    85,     8,
      86,    87,    10,    88,    89,    90,    91,    15,    16,    92,
      18,    93,    20,    21,    94,    95,    96,    97,    25,     0,
      27,    28,    98,    99,   100,   101,     0,     0,     0,     0,
       0,     0,   133,     0,    33,    34,     0,     0,     0,   134,
      36,   135,    38,   136,    40,   137,    42,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,   138,
      55,     0,     0,    56,   139,    58,    59,   102,   103,   104,
     105,     0,     0,     0,   106,     0,     0,     0,   140,   141,
       0,     0,     0,     0,   160,   435,   143,   123,   124,     1,
     125,   126,    84,   127,   128,   129,   130,    85,     8,    86,
      87,    10,    88,    89,    90,    91,    15,    16,    92,    18,
      93,    20,    21,    94,    95,    96,    97,    25,     0,    27,
      28,    98,    99,   100,   101,     0,     0,     0,     0,     0,
       0,   133,     0,    33,    34,     0,     0,     0,   134,    36,
     135,    38,   136,    40,   137,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,   138,    55,
       0,     0,    56,   139,    58,    59,   102,   103,   104,   105,
       0,     0,     0,   106,     0,     0,     0,   140,   141,     0,
       0,     0,     0,   160,   495,   143,   123,   124,     1,   125,
     126,    84,   127,   128,   129,   130,    85,     8,    86,    87,
     131,    88,    89,    90,    91,    15,    16,    92,   132,    93,
      20,    21,    94,    95,    96,    97,    25,     0,    27,    28,
      98,    99,   100,   101,     0,     0,     0,     0,     0,     0,
     133,     0,    33,    34,     0,     0,     0,   134,    36,   135,
      38,   136,    40,   137,    42,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,   138,    55,     0,
       0,    56,   139,    58,    59,   102,   103,   104,   105,     0,
       0,     0,   106,     0,     0,     0,   140,   141,     0,     0,
       0,     0,   142,     0,   143,   123,   124,     1,   125,   126,
      84,   127,   128,   129,   130,    85,     8,    86,    87,    10,
      88,    89,    90,    91,    15,    16,    92,    18,    93,    20,
      21,    94,    95,    96,    97,    25,     0,    27,    28,    98,
      99,   100,   101,     0,     0,     0,     0,     0,     0,   133,
       0,    33,    34,     0,     0,     0,   134,    36,   135,    38,
     136,    40,   137,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,   138,    55,     0,     0,
      56,   139,    58,    59,   102,   103,   104,   105,     0,     0,
       0,   106,     0,     0,     0,   140,   141,     0,     0,     0,
       0,   160,     0,   143,   123,   124,     1,   125,   126,    84,
     127,   128,   129,   130,    85,     8,    86,    87,    10,    88,
      89,    90,    91,    15,    16,    92,    18,    93,    20,    21,
      94,    95,    96,    97,    25,     0,    27,    28,    98,    99,
     100,   101,     0,     0,     0,     0,     0,     0,   133,     0,
      33,    34,     0,     0,     0,   134,    36,   135,    38,   136,
      40,   137,    42,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,    52,    53,   138,    55,     0,     0,    56,
     139,    58,    59,   102,   103,   104,   105,     0,     0,     0,
     106,     0,     0,     0,   140,   141,     0,     0,     0,     0,
     142,     0,   143,   123,   124,     1,   125,   126,    84,   127,
     128,   129,   130,    85,     8,    86,    87,    10,    88,    89,
      90,    91,    15,    16,    92,    18,    93,    20,    21,    94,
      95,    96,    97,    25,     0,    27,    28,    98,    99,   100,
     101,     0,     0,     0,     0,     0,     0,   133,     0,    33,
      34,     0,     0,     0,   134,    36,   135,    38,   136,    40,
     137,    42,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,   138,    55,     0,     0,    56,    57,
      58,    59,   102,   103,   104,   105,     0,     0,     0,   106,
       0,     0,     0,   140,   141,     0,     0,     0,     0,   191,
       0,   143,   123,   124,     1,   125,   126,    84,   127,   128,
     129,   130,    85,     8,    86,    87,    10,    88,    89,    90,
      91,    15,    16,    92,    18,    93,    20,    21,    94,    95,
      96,    97,    25,     0,    27,    28,    98,    99,   100,   101,
       0,     0,     0,     0,     0,     0,   133,     0,    33,    34,
       0,     0,     0,   134,    36,   135,    38,   136,    40,   137,
      42,    43,    44,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,   138,    55,     0,     0,    56,    57,    58,
      59,   102,   103,   104,   105,     0,     0,     0,   106,     0,
       0,     0,   140,   141,     0,     0,     0,     0,   160,     0,
     143,   123,   124,     1,   125,   126,    84,   127,   128,   129,
     130,    85,     8,    86,    87,    10,    88,    89,    90,    91,
      15,    16,    92,    18,    93,    20,    21,    94,    95,    96,
      97,    25,     0,    27,    28,    98,    99,   100,   101,     0,
       0,     0,     0,     0,     0,   133,     0,    33,    34,     0,
       0,     0,   134,    36,   135,    38,   136,    40,   137,    42,
      43,    44,    45,    46,    47,    48,    49,   419,     0,   421,
      52,    53,   138,    55,     0,     0,   425,   426,    58,    59,
     102,   103,   104,   105,     0,     0,     0,   106,     0,     0,
       0,   140,   141,     0,     0,     0,     0,   160,     0,   143,
     223,   224,     1,   225,   226,    84,   127,   128,   129,   130,
      85,     8,    86,    87,    10,    88,    89,    90,    91,    15,
      16,    92,    18,    93,    20,    21,    94,    95,    96,    97,
      25,     0,    27,    28,    98,    99,   100,   101,     0,     0,
       0,     0,     0,     0,   133,     0,    33,    34,     0,     0,
       0,   134,    36,   135,    38,   136,    40,   137,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,   138,    55,     0,     0,    56,   139,    58,    59,   102,
     103,   104,   105,     0,     0,     0,   106,     0,     0,     0,
     140,   141,     0,     0,     0,     0,   160,     0,   143,   123,
     124,     1,   125,   126,    84,   127,   128,   129,   130,    85,
       8,    86,    87,    10,    88,    89,    90,    91,    15,    16,
      92,    18,    93,    20,    21,    94,    95,    96,    97,    25,
       0,    27,    28,    98,    99,   100,   101,     0,     0,     0,
       0,     0,     0,   133,     0,    33,    34,     0,     0,     0,
     134,    36,   135,    38,   136,    40,   137,    42,    43,    44,
      45,    46,    47,    48,    49,   481,     0,   482,    52,    53,
     138,    55,     0,     0,   483,   484,    58,    59,   102,   103,
     104,   105,     0,     0,     0,   106,     0,     0,     0,   140,
     141,     0,     0,     0,     0,   160,     0,   143,   123,   124,
       1,   125,   126,    84,   127,   128,   129,   130,    85,     8,
      86,    87,    10,    88,    89,    90,    91,    15,    16,    92,
      18,    93,    20,    21,    94,    95,    96,    97,    25,     0,
      27,    28,    98,    99,   100,   101,     0,     0,     0,     0,
       0,     0,   133,     0,    33,    34,     0,     0,     0,   134,
      36,   135,    38,   136,    40,   137,    42,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,   138,
      55,     0,     0,    56,   139,    58,    59,   102,   103,   104,
     105,     0,     0,     0,   106,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   160,     1,   143,     0,     2,     3,
       4,     5,     6,     7,     8,     0,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,    29,    30,    31,
      32,     0,     0,     0,     0,     0,     0,     0,     0,    33,
      34,     0,     0,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,     0,     0,    56,    57,
      58,    59,    60,    61,    62,     0,    63,     1,     0,    64,
       0,     3,     4,     5,     6,     7,     8,    65,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,     0,
      30,    31,    32,     0,     0,     0,     0,     0,     0,     0,
       0,    33,    34,     0,     0,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,    54,    55,     0,     0,
      56,    57,    58,    59,    60,    61,     0,     0,    63,     1,
       0,    64,     0,     3,     4,     5,     6,     7,     8,   373,
       9,    10,    11,    12,    13,    14,    15,    16,     0,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
      28,     0,    30,    31,    32,     0,     0,     0,     0,     0,
       0,     0,     0,    33,    34,     0,     0,     0,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,    54,    55,
       0,     0,    56,    57,    58,    59,    60,    61,     0,     0,
      63,     1,     0,    64,     0,     3,     4,     5,     6,     7,
       8,   396,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,    28,     0,    30,    31,    32,     0,     0,     0,
       0,     0,     0,     0,     0,    33,    34,     0,     0,     0,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
      54,    55,     0,     0,    56,    57,    58,    59,    60,    61,
       0,     0,    63,     1,     0,    64,     0,     3,     4,     5,
       6,     7,     8,   523,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,    28,     0,    30,    31,    32,     0,
       0,     0,     0,     0,     0,     0,     0,    33,    34,     0,
       0,     0,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,     0,    51,
      52,    53,    54,    55,     0,     0,    56,    57,    58,    59,
      60,    61,     0,     0,    63,     1,     0,    64,     0,     3,
       4,     5,     6,     7,     8,   555,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,     0,    30,    31,
      32,     0,     0,     0,     0,     0,     0,     0,     0,    33,
      34,     0,     0,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,     0,     0,    56,    57,
      58,    59,    60,    61,     0,     0,     0,     0,     0,    64,
       0,     0,     0,     0,     0,     1,     0,   468,    84,     3,
       4,     5,     6,    85,     8,    86,    87,    10,    88,    89,
      90,    91,    15,    16,    92,    18,    93,    20,    21,    94,
      95,    96,    97,    25,     0,    27,    28,    98,    99,   100,
     101,     0,     0,     0,     0,     0,     0,     0,     0,    33,
      34,     0,     0,     0,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,    52,    53,    54,    55,     0,     0,    56,    57,
      58,    59,   102,   103,   104,   105,     0,     1,     0,   106,
      84,     3,     4,     5,     6,    85,     8,    86,    87,    10,
      88,    89,    90,    91,    15,    16,    92,    18,    93,    20,
      21,    94,    95,    96,    97,    25,     0,    27,    28,    98,
      99,   100,   101,     0,     0,     0,     0,     0,     0,     0,
       0,    33,    34,     0,     0,     0,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,    52,    53,   168,    55,     0,     0,
      56,    57,    58,    59,   102,   103,   104,   105,     0,     1,
       0,   106,    84,     3,     4,     5,     6,    85,     8,    86,
      87,    10,    88,    89,    90,    91,    15,    16,    92,    18,
      93,    20,    21,    94,    95,    96,    97,    25,     0,    27,
      28,    98,    99,   100,   101,     0,     0,     0,     0,     0,
       0,     0,     0,    33,    34,     0,     0,     0,    35,    36,
      37,    38,    39,    40,    41,    42,   209,   210,   211,   212,
     213,    48,   214,    50,     0,    51,    52,   215,    54,    55,
       0,     0,    56,    57,    58,    59,   102,   103,   104,   105,
       0,     1,     0,   106,    84,   127,   128,     5,     6,    85,
       8,    86,    87,    10,    88,    89,    90,    91,    15,    16,
      92,    18,    93,    20,    21,    94,    95,    96,    97,    25,
       0,    27,    28,    98,    99,   100,   101,     0,     0,     0,
       0,     0,     0,     0,     0,    33,    34,     0,     0,     0,
     134,    36,   135,    38,   136,    40,   137,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,    52,    53,
     376,    55,     0,     0,    56,    57,    58,    59,   102,   103,
     104,   105,     1,     0,     0,   106,     3,     4,     5,     6,
       7,     8,     0,     9,    10,    11,    12,    13,     0,    15,
      16,     0,    18,    19,    20,    21,    22,    95,    24,     0,
      25,     0,    27,    28,     0,    30,    31,    32,     0,     0,
       0,     0,     0,     0,     0,     0,    33,    34,     0,     0,
       0,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,    52,
      53,    54,    55,   293,     0,    56,    57,    58,    59,   294,
      61,     0,     0,     0,     0,     0,    64,   238,   239,   240,
     241,   242,   243,     0,   244,   295,     0,   245,   246,   247,
       0,   296,     0,     0,     0,     0,     0,     0,   497,   238,
     239,   240,   241,   242,   243,     0,   244,     0,     0,   245,
     246,   247,   238,   239,   240,   241,   242,   243,     0,   244,
     498,     0,   245,   246,   247,     0,   248,   249,   250,   251,
     252,   253,   254,     0,   238,   239,   240,   241,   242,   243,
       0,   244,     0,     0,   245,   246,   247,     0,   248,   249,
     250,   251,   252,   253,   254,   499,     0,     0,     0,     0,
       0,   248,   249,   250,   251,   252,   253,   254,     0,   238,
     239,   240,   241,   242,   243,     0,   244,   500,     0,   245,
     246,   247,     0,   248,   249,   250,   251,   252,   253,   254,
     579,   238,   239,   240,   241,   242,   243,     0,   244,     0,
       0,   245,   246,   247,   563,   564,     0,   565,   566,   567,
       0,   568,     0,     0,   569,   570,   571,   281,   248,   249,
     250,   251,   252,   253,   254,   238,   239,   240,   241,   242,
     243,     0,   244,     0,     0,   245,   246,   247,     0,     0,
     248,   249,   250,   251,   252,   253,   254,     0,     0,     0,
       0,     0,     0,   572,   573,   574,   575,   576,   577,   578,
     238,   239,   240,   241,   242,   243,     0,   244,     0,     0,
     245,   246,   247,     0,   248,   249,   250,   251,   252,   253,
     254,   282,   238,   239,   240,   241,   242,   243,     0,   244,
       0,     0,   245,   246,   247,   238,   239,   240,   241,   242,
     243,     0,   244,     0,     0,   245,   246,   247,     0,   248,
     249,   250,   251,   252,   253,   254,     0,   320,     0,   321,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   248,   249,   250,   251,   252,   253,   254,     0,   402,
       0,   321,     0,     0,   248,   249,   250,   251,   252,   253,
     254,     0,     0,     0,   321,   354,   355,   356,   357,   358,
     359,     0,   360,     0,     0,   361,   362,   363,   354,   355,
     356,   357,   358,   359,     0,   360,     0,     0,   361,   362,
     363,   354,   355,   356,   357,   358,   359,     0,   360,     0,
       0,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   364,   365,   366,   367,   368,   369,
     370,     0,   371,     0,     0,     0,     0,   364,   365,   366,
     367,   368,   369,   370,     0,   433,     0,     0,     0,     0,
     364,   365,   366,   367,   368,   369,   370,     0,   515,   238,
     239,   240,   241,   242,   243,     0,   244,     0,     0,   245,
     246,   247,   238,   239,   240,   241,   242,   243,     0,   244,
       0,     0,   245,   246,   247,   238,   239,   240,   241,   242,
     243,     0,   244,     0,     0,   245,   246,   247,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   248,   249,
     250,   251,   252,   253,   254,   487,     0,     0,     0,     0,
       0,   248,   249,   250,   251,   252,   253,   254,   491,     0,
       0,     0,     0,     0,   248,   249,   250,   251,   252,   253,
     254,   525,   238,   239,   240,   241,   242,   243,     0,   244,
       0,     0,   245,   246,   247,   238,   239,   240,   241,   242,
     243,     0,   244,     0,     0,   245,   246,   247,     0,     0,
       0,     0,     0,   238,   239,   240,   241,   242,   243,     0,
     244,     0,     0,   245,   246,   247,     0,     0,     0,     0,
       0,   248,   249,   250,   251,   252,   253,   254,   590,     0,
       0,     0,     0,     0,   248,   249,   250,   251,   252,   253,
     254,   614,   317,   318,     0,     0,     0,     0,     0,     0,
       0,     0,   248,   249,   250,   251,   252,   253,   254,   238,
     239,   240,   241,   242,   243,     0,   244,     0,     0,   245,
     246,   247,   354,   355,   356,   357,   358,   359,     0,   360,
       0,     0,   361,   362,   363,   563,   564,     0,   565,   566,
     567,     0,   568,     0,     0,   569,   570,   571,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   248,   249,
     250,   251,   252,   253,   254,     0,     0,     0,     0,     0,
       0,   364,   365,   366,   367,   368,   369,   370,     0,     0,
       0,     0,     0,     0,   572,   573,   574,   575,   576,   577,
     578,   238,   239,   240,   241,     0,     0,     0,   244,     0,
       0,   245,   246,   247,   563,   564,     0,   565,     0,     0,
       0,   568,     0,     0,   569,   570,   571,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     248,   249,   250,   251,   252,   253,   254,     0,     0,     0,
       0,     0,     0,   572,   573,   574,   575,   576,   577,   578
};

static const yytype_int16 yycheck[] =
{
       0,     0,   234,    64,    16,    32,    12,   387,    14,    24,
     375,     2,   373,   157,   206,     9,    22,    31,   258,   384,
      11,   165,   558,   255,    30,    31,    17,   523,    31,   221,
      34,    68,   176,    24,    61,    23,   401,    22,    29,    76,
      44,    32,    49,     7,   188,    30,    31,    68,    23,    13,
     586,    10,    11,    28,    60,    76,    44,    68,    98,   555,
      98,    99,   206,   111,    64,    76,    72,    67,    67,    98,
      61,    62,     0,    49,    69,    87,    52,   221,    68,   319,
     128,   129,   130,    99,    98,    98,    76,    72,   320,    96,
      97,    68,    86,   157,   334,   100,    55,    49,    57,    76,
      59,   165,    61,    49,    98,    98,   112,   113,   114,   115,
     471,   100,   176,   353,    98,   104,    75,    93,    94,    95,
      96,    97,    36,    37,   188,   131,   132,   142,   134,   135,
     136,   137,    98,   139,    49,   162,   142,    52,    98,   371,
      98,   333,   206,    98,    96,    97,    28,   138,    94,    95,
      96,    97,   143,    98,   160,   182,    98,   221,   538,    98,
     525,   167,   523,    49,    98,   415,   416,   417,   418,    99,
     402,   162,   422,   103,    98,   166,    98,   168,    93,    94,
      95,    96,    97,    28,    55,   191,    57,    98,    59,   333,
      61,   182,    98,    99,   555,    98,   436,    98,    99,   190,
     104,   433,   267,   268,   104,   445,   104,   104,    94,    95,
      96,    97,   203,   103,   103,   580,    99,    26,    98,   234,
     281,    28,    98,    26,    99,   590,    16,   467,    98,    98,
      98,    89,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   614,
     101,   266,   267,   268,   101,    88,   496,    98,     9,    28,
      70,   101,   262,   262,   101,   101,    34,    98,   274,   333,
      98,   277,   512,    98,    70,   266,   267,   268,   110,   111,
     101,   281,   282,   515,   290,    87,   292,    88,   294,    70,
     296,    88,    98,   101,    88,   127,   128,   129,   130,    98,
      98,   133,   266,   267,   268,    67,   392,    31,   140,   141,
     399,   471,   509,   553,   316,    -1,   322,   323,   324,   325,
     328,   327,   328,    -1,    -1,   331,    -1,    -1,    -1,   335,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   328,    -1,    -1,
     482,   483,   484,    -1,    -1,    -1,    -1,    -1,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   373,    -1,   375,   382,   383,    -1,    -1,
      -1,    -1,    -1,    -1,   384,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   387,   388,    -1,   399,
      -1,   401,    -1,    -1,    -1,    -1,    -1,    -1,   414,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   481,   482,   483,
     484,   563,   564,   565,   566,   567,   568,   569,   570,   571,
     572,   573,   574,   575,   576,   577,   578,    -1,    -1,    -1,
     446,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    -1,    -1,    -1,   289,    -1,   291,
      -1,   293,   489,   295,    -1,   607,    -1,    -1,    -1,    -1,
      -1,   471,    -1,   473,   473,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   315,   316,   317,   318,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    98,   489,   563,
     564,   565,   566,   567,   568,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   578,    -1,    -1,    -1,    -1,   519,
     519,   527,    -1,   523,    -1,   525,    -1,    -1,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   607,    -1,    -1,    -1,   538,    34,    -1,
      -1,    -1,    -1,   385,   560,   555,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     580,   413,    -1,    -1,    -1,    -1,    -1,   419,    -1,   421,
     590,    -1,    -1,   425,   426,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,
      96,    97,    28,    -1,   614,    -1,    -1,    -1,    34,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   481,
     482,   483,   484,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   497,   498,   499,   500,    -1,
      -1,   503,   504,   505,   506,    91,    92,    93,    94,    95,
      96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    45,    46,    47,    33,
      49,    -1,    -1,    52,    53,    54,    -1,   549,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,   563,   564,   565,   566,   567,   568,   569,   570,   571,
     572,   573,   574,   575,   576,   577,   578,    -1,    -1,    -1,
      -1,   583,    91,    92,    93,    94,    95,    96,    97,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,
      94,    95,    96,    97,    -1,   607,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,
      -1,    -1,   100,    -1,   102,    -1,   104,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    -1,    99,   100,    -1,   102,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
      -1,    99,   100,    -1,   102,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    90,    91,    92,    93,    94,    95,    96,    97,    -1,
      99,   100,    -1,   102,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      90,    91,    92,    93,    94,    95,    96,    97,    -1,    99,
     100,    -1,   102,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    95,    96,    97,    -1,    -1,   100,
      -1,   102,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,   100,    -1,
     102,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,
      -1,    94,    95,    -1,    -1,    -1,    -1,   100,   101,   102,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      94,    95,    -1,    -1,    -1,    -1,   100,   101,   102,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,
      95,    -1,    -1,    -1,    -1,   100,   101,   102,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,   100,   101,   102,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,
      -1,    -1,    -1,   100,   101,   102,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    74,    75,    76,    -1,
      -1,    79,    80,    81,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,
      -1,    -1,   100,    -1,   102,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,   100,    -1,   102,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    -1,    -1,    79,
      80,    81,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      90,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
     100,    -1,   102,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,   100,
      -1,   102,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    -1,
      72,    73,    74,    75,    76,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,   100,    -1,
     102,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    -1,    -1,    79,    80,    81,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,
      -1,    94,    95,    -1,    -1,    -1,    -1,   100,    -1,   102,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    -1,    -1,    79,    80,    81,    82,    83,
      84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      94,    95,    -1,    -1,    -1,    -1,   100,    -1,   102,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    94,
      95,    -1,    -1,    -1,    -1,   100,    -1,   102,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    -1,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    -1,    -1,    79,    80,    81,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,     6,   102,    -1,     9,    10,
      11,    12,    13,    14,    15,    -1,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    -1,    87,     6,    -1,    90,
      -1,    10,    11,    12,    13,    14,    15,    98,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    36,    37,    -1,
      39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    -1,
      79,    80,    81,    82,    83,    84,    -1,    -1,    87,     6,
      -1,    90,    -1,    10,    11,    12,    13,    14,    15,    98,
      17,    18,    19,    20,    21,    22,    23,    24,    -1,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    36,
      37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    -1,    -1,
      87,     6,    -1,    90,    -1,    10,    11,    12,    13,    14,
      15,    98,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    36,    37,    -1,    39,    40,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    81,    82,    83,    84,
      -1,    -1,    87,     6,    -1,    90,    -1,    10,    11,    12,
      13,    14,    15,    98,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    36,    37,    -1,    39,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,    72,
      73,    74,    75,    76,    -1,    -1,    79,    80,    81,    82,
      83,    84,    -1,    -1,    87,     6,    -1,    90,    -1,    10,
      11,    12,    13,    14,    15,    98,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    36,    37,    -1,    39,    40,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    -1,    -1,     6,    -1,    98,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    82,    83,    84,    85,    86,    -1,     6,    -1,    90,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    74,    75,    76,    -1,    -1,
      79,    80,    81,    82,    83,    84,    85,    86,    -1,     6,
      -1,    90,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    79,    80,    81,    82,    83,    84,    85,    86,
      -1,     6,    -1,    90,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,     6,    -1,    -1,    90,    10,    11,    12,    13,
      14,    15,    -1,    17,    18,    19,    20,    21,    -1,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    -1,    36,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    28,    -1,    79,    80,    81,    82,    34,
      84,    -1,    -1,    -1,    -1,    -1,    90,    42,    43,    44,
      45,    46,    47,    -1,    49,    28,    -1,    52,    53,    54,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    28,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    42,    43,    44,    45,    46,    47,    -1,    49,
      28,    -1,    52,    53,    54,    -1,    91,    92,    93,    94,
      95,    96,    97,    -1,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    -1,    91,    92,
      93,    94,    95,    96,    97,    28,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    -1,    42,
      43,    44,    45,    46,    47,    -1,    49,    28,    -1,    52,
      53,    54,    -1,    91,    92,    93,    94,    95,    96,    97,
      28,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    42,    43,    -1,    45,    46,    47,
      -1,    49,    -1,    -1,    52,    53,    54,    34,    91,    92,
      93,    94,    95,    96,    97,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    97,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    53,    54,    -1,    91,    92,    93,    94,    95,    96,
      97,    98,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    91,
      92,    93,    94,    95,    96,    97,    -1,    99,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    -1,    99,
      -1,   101,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    -1,    -1,    -1,   101,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    52,    53,
      54,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    -1,    99,    -1,    -1,    -1,    -1,    91,    92,    93,
      94,    95,    96,    97,    -1,    99,    -1,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    -1,    99,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    98,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    98,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    98,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    98,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      53,    54,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    53,    54,    42,    43,    -1,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    42,    43,    44,    45,    -1,    -1,    -1,    49,    -1,
      -1,    52,    53,    54,    42,    43,    -1,    45,    -1,    -1,
      -1,    49,    -1,    -1,    52,    53,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    93,    94,    95,    96,    97
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     6,     9,    10,    11,    12,    13,    14,    15,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    34,    35,    36,    37,    38,
      39,    40,    41,    50,    51,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    72,    73,    74,    75,    76,    79,    80,    81,    82,
      83,    84,    85,    87,    90,    98,   106,   107,   108,   109,
     110,   115,   117,   121,   122,   123,   124,   128,   130,   131,
     132,   134,   135,   136,     9,    14,    16,    17,    19,    20,
      21,    22,    25,    27,    30,    31,    32,    33,    38,    39,
      40,    41,    83,    84,    85,    86,    90,   116,   117,   118,
      10,    11,    55,    57,    59,    61,    75,   147,    31,    98,
     118,   119,   120,     4,     5,     7,     8,    10,    11,    12,
      13,    18,    26,    48,    55,    57,    59,    61,    75,    80,
      94,    95,   100,   102,   118,   125,   127,   143,   144,   147,
     149,   151,   154,   155,   159,   162,   163,   164,   167,   147,
     100,   164,   118,    31,   125,   164,    36,    37,    75,   118,
     144,   146,   157,    98,   118,   125,   164,   125,   127,   119,
     164,   119,   118,    98,   123,     0,   108,   125,   164,    98,
      99,   100,   150,   163,   150,   164,   164,   164,   164,    69,
      98,    98,    98,    99,   150,   150,   164,   164,   163,    63,
      64,    65,    66,    67,    69,    74,   118,   164,   163,   163,
     101,   144,   164,     4,     5,     7,     8,   104,   118,   160,
     161,   164,   168,   169,   100,    98,    98,    28,    42,    43,
      44,    45,    46,    47,    49,    52,    53,    54,    91,    92,
      93,    94,    95,    96,    97,    99,   126,   143,   165,   167,
      98,   164,    33,   119,    98,    98,    24,    50,    51,   118,
     164,   118,    34,    44,   129,    98,    98,    36,    98,    98,
      98,    34,    98,    98,   119,    98,   118,   101,   164,    28,
      34,    28,    34,    28,    34,    28,    34,    55,    57,    59,
      61,   148,   118,    28,   165,    68,    76,    68,    76,    68,
      76,    68,    76,    68,    76,    28,    28,    81,    82,   165,
      99,   101,   104,   104,   104,   104,   103,   104,    99,   103,
     103,    99,   101,   144,   169,    26,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   169,    42,    43,    44,    45,    46,    47,
      49,    52,    53,    54,    91,    92,    93,    94,    95,    96,
      97,    99,   126,    98,   121,    98,    75,   144,   145,   147,
     145,   145,    23,    44,    98,    28,   164,     9,    86,    98,
     111,   112,   113,   114,   164,   123,    98,   121,   137,   138,
     139,    98,    99,   163,   164,   163,   164,   163,   164,   163,
     164,    23,    28,   156,    26,    56,    58,    60,    62,    70,
      71,    72,    73,    77,    78,    79,    80,   152,   153,   163,
     152,   163,   163,    99,   126,   101,   169,   164,   164,   164,
     164,   164,   161,   164,   164,   165,    99,   126,   164,   126,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   169,    98,   122,
     140,   141,   142,    16,   137,    98,    98,    98,   164,   164,
     137,    70,    72,    79,    80,   158,   163,    98,   116,   118,
     114,    98,    89,   139,   137,   101,   169,    28,    28,    28,
      28,   163,   164,   156,   156,   156,   156,   163,   163,   156,
     163,   163,   169,   101,   126,    99,   126,   101,   126,    16,
      87,   133,   142,    98,   121,    98,    15,    34,    88,   163,
     166,   167,   166,   166,   166,    98,   119,    98,     9,   126,
     163,   163,   163,   163,   163,   163,   163,   163,    28,    70,
     153,   126,   101,   169,   101,    98,   121,    98,   140,   137,
      34,   164,    98,    42,    43,    45,    46,    47,    49,    52,
      53,    54,    91,    92,    93,    94,    95,    96,    97,    28,
      98,   116,   101,    70,   101,   126,   140,   133,    88,   164,
      98,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   166,   166,    70,   137,    98,
     163,   101,   133,    98,    98,   137,   166,   137,    88,    88,
      98,    98
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   105,   106,   107,   107,   108,   108,   108,   108,   108,
     108,   109,   110,   111,   112,   112,   113,   113,   114,   114,
     114,   115,   115,   116,   116,   116,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   119,   119,   119,   120,   120,   121,   121,   122,
     122,   122,   122,   122,   122,   123,   123,   123,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   125,   125,   125,   125,   125,   125,   125,   125,
     125,   125,   126,   126,   127,   127,   127,   127,   127,   128,
     128,   128,   128,   129,   129,   130,   130,   130,   131,   131,
     132,   132,   132,   132,   133,   133,   134,   134,   134,   134,
     135,   135,   136,   137,   137,   138,   138,   139,   139,   140,
     140,   141,   141,   142,   142,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     144,   145,   145,   146,   146,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   148,   148,   148,   148,
     149,   149,   150,   150,   150,   150,   151,   151,   151,   151,
     151,   152,   152,   152,   152,   152,   153,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   155,   155,   155,
     155,   155,   155,   155,   155,   156,   156,   157,   157,   158,
     158,   158,   158,   158,   159,   159,   159,   160,   160,   160,
     161,   161,   161,   161,   161,   162,   162,   163,   163,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   166,   166,   166,
     166,   166,   166,   166,   166,   166,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   167,   167,   168,   168,   169,
     169
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
       1,     1,     0,     1,     2,     1,     3,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     5,     3,     3,     2,     3,
       3,     2,     0,     2,     4,     3,     5,     2,     4,     6,
       5,     7,     0,     1,     2,     2,     5,     4,     3,     5,
       5,     5,     5,     1,     1,     3,     3,     3,     4,     6,
       6,     8,     7,     9,     0,     2,     7,    11,    12,     9,
       4,     6,     2,     0,     1,     1,     2,     1,     1,     0,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     2,
       3,     5,     6,     8,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     4,     6,     4,
       6,     4,     6,     4,     6,     5,     1,     1,     1,     1,
       2,     2,     1,     2,     4,     6,     2,     4,     4,     1,
       1,     1,     1,     5,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     6,     6,     6,
       6,     6,     4,     4,     4,     1,     1,     2,     4,     1,
       2,     5,     2,     2,     3,     3,     3,     1,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     4,     4,     0,     1,     1,
       3
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
#line 2298 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2304 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2310 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2316 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2322 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 202 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2328 "engines/director/lingo/lingo-gr.cpp"
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
#line 2598 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 210 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2609 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 216 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2619 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 223 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 2625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* scriptpart: tENDCLAUSE endargdef '\n'  */
#line 228 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 257 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* factory: tFACTORY ID '\n' methodlist  */
#line 261 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 2643 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 263 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2649 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* methodlist: %empty  */
#line 265 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 2655 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* nonemptymethodlist: methodlistline  */
#line 269 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 2666 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 275 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 2676 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18: /* methodlistline: '\n'  */
#line 282 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 2682 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* methodlistline: tENDCLAUSE endargdef '\n'  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 2688 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 289 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 2697 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22: /* handler: tON ID idlist '\n' stmtlist  */
#line 293 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 2704 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* endargdef: ID  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 2710 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* endargdef: endargdef ',' ID  */
#line 299 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2716 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* CMDID: tABBREVIATED  */
#line 307 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 2722 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28: /* CMDID: tABBREV  */
#line 308 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* CMDID: tABBR  */
#line 309 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 2734 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* CMDID: tAFTER  */
#line 310 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 2740 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31: /* CMDID: tBEFORE  */
#line 311 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 2746 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tCAST  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2752 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tCHAR  */
#line 313 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 2758 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tCHARS  */
#line 314 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 2764 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tDATE  */
#line 315 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 2770 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tDELETE  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 2776 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tDOWN  */
#line 317 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 2782 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tFIELD  */
#line 318 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2788 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tFRAME  */
#line 319 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 2794 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tHILITE  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 2800 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tIN  */
#line 321 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 2806 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tINTERSECTS  */
#line 322 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 2812 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tINTO  */
#line 323 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 2818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tITEM  */
#line 324 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 2824 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tITEMS  */
#line 325 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 2830 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tLAST  */
#line 326 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 2836 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tLINE  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 2842 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tLINES  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 2848 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tLONG  */
#line 329 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 2854 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tMENU  */
#line 330 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 2860 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tMENUITEM  */
#line 331 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 2866 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tMENUITEMS  */
#line 332 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 2872 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tMOVIE  */
#line 333 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 2878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tNEXT  */
#line 334 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 2884 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tNUMBER  */
#line 335 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 2890 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tOF  */
#line 336 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 2896 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tPREVIOUS  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 2902 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tREPEAT  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 2908 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tSCRIPT  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2914 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tASSERTERROR  */
#line 340 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 2920 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tSHORT  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 2926 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tSOUND  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 2932 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tSPRITE  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 2938 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tTHE  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 2944 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tTIME  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 2950 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tTO  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 2956 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tWHILE  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 2962 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tWINDOW  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2968 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tWITH  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 2974 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tWITHIN  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 2980 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tWORD  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 2986 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tWORDS  */
#line 352 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 2992 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* ID: tELSE  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 2998 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* ID: tENDCLAUSE  */
#line 357 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); delete (yyvsp[0].s); }
#line 3004 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* ID: tEXIT  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 3010 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* ID: tFACTORY  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 3016 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* ID: tGLOBAL  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 3022 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* ID: tGO  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 3028 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80: /* ID: tIF  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 3034 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tINSTANCE  */
#line 363 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 3040 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tMACRO  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3046 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tMETHOD  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3052 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tON  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tOPEN  */
#line 367 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3064 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tPLAY  */
#line 368 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tPROPERTY  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3076 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tPUT  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tSET  */
#line 371 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3088 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tTELL  */
#line 372 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tTHEN  */
#line 373 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* idlist: %empty  */
#line 376 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3106 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* nonemptyidlist: ID  */
#line 381 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3115 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 385 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3123 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* stmt: tENDIF '\n'  */
#line 396 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3129 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108: /* proc: CMDID cmdargs '\n'  */
#line 412 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3135 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109: /* proc: tPUT cmdargs '\n'  */
#line 413 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110: /* proc: tGO cmdargs '\n'  */
#line 414 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3147 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111: /* proc: tGO frameargs '\n'  */
#line 415 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3153 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112: /* proc: tPLAY cmdargs '\n'  */
#line 416 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3159 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113: /* proc: tPLAY frameargs '\n'  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3165 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114: /* proc: tOPEN cmdargs '\n'  */
#line 418 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3171 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115: /* proc: tOPEN expr tWITH expr '\n'  */
#line 419 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3181 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: tNEXT tREPEAT '\n'  */
#line 424 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3187 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: tEXIT tREPEAT '\n'  */
#line 425 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3193 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tEXIT '\n'  */
#line 426 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3199 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tDELETE chunk '\n'  */
#line 427 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tHILITE chunk '\n'  */
#line 428 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3211 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tASSERTERROR stmtoneliner  */
#line 429 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3217 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* cmdargs: %empty  */
#line 432 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3225 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* cmdargs: expr trailingcomma  */
#line 435 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3235 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 440 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd arg, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3244 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 444 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3255 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 450 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* cmdargs: '(' ')'  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3273 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* cmdargs: '(' expr ',' ')'  */
#line 458 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3283 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 463 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3292 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* cmdargs: '(' var expr_nounarymath trailingcomma ')'  */
#line 467 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3303 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* cmdargs: '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 473 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* frameargs: tFRAME expr  */
#line 486 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3323 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* frameargs: tMOVIE expr  */
#line 491 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 497 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* frameargs: expr tOF tMOVIE expr  */
#line 503 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3356 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* frameargs: tFRAME expr expr_nounarymath  */
#line 509 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3367 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 517 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3373 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 518 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 519 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3385 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* asgn: tSET varorthe to expr '\n'  */
#line 520 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3391 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* definevars: tGLOBAL idlist '\n'  */
#line 525 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3397 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* definevars: tPROPERTY idlist '\n'  */
#line 526 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3403 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* definevars: tINSTANCE idlist '\n'  */
#line 527 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3409 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* ifstmt: tIF expr tTHEN stmt  */
#line 530 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 3418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 534 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 3425 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 538 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 3436 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 544 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 3445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 548 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 3454 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 552 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 3461 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* endif: %empty  */
#line 556 "engines/director/lingo/lingo-gr.y"
                        {
		LingoCompiler *compiler = g_lingo->_compiler;
		warning("LingoCompiler::parse: no end if at line %d col %d in %s id: %d",
			compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
			compiler->_assemblyContext->_id);

		}
#line 3473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 565 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3480 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 567 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3487 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 569 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3494 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 571 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3501 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* tell: tTELL expr tTO stmtoneliner  */
#line 575 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 3510 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 579 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 3517 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* when: tWHEN '\n'  */
#line 583 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WhenNode((yyvsp[-1].w).eventName, (yyvsp[-1].w).stmt); }
#line 3523 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* stmtlist: %empty  */
#line 585 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3529 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* nonemptystmtlist: stmtlistline  */
#line 590 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3540 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 596 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3550 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* stmtlistline: '\n'  */
#line 603 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 3556 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* stmtlist_insideif: %empty  */
#line 607 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 3562 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 612 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3573 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 618 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3583 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* stmtlistline_insideif: '\n'  */
#line 625 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3589 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* simpleexpr_nounarymath: tINT  */
#line 632 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 3595 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* simpleexpr_nounarymath: tFLOAT  */
#line 633 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 3601 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* simpleexpr_nounarymath: tSYMBOL  */
#line 634 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 3607 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* simpleexpr_nounarymath: tSTRING  */
#line 635 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 3613 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 636 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 3619 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 637 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 3625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 638 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 3631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath trailingcomma ')'  */
#line 639 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.node) = new FuncNode((yyvsp[-5].s), args); }
#line 3642 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 645 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.node) = new FuncNode((yyvsp[-7].s), (yyvsp[-2].nodelist)); }
#line 3652 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 3658 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* var: ID  */
#line 658 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 3664 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* chunk: tFIELD refargs  */
#line 668 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 3670 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* chunk: tCAST refargs  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 3676 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* chunk: tCHAR expr tOF simpleexpr  */
#line 670 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3683 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 672 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3690 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* chunk: tWORD expr tOF simpleexpr  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3697 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 676 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3704 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* chunk: tITEM expr tOF simpleexpr  */
#line 678 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 680 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3718 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* chunk: tLINE expr tOF simpleexpr  */
#line 682 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 3725 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 684 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3732 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 205: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 686 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 3738 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* chunktype: tCHAR  */
#line 689 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 3744 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* chunktype: tWORD  */
#line 690 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 3750 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* chunktype: tITEM  */
#line 691 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 3756 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* chunktype: tLINE  */
#line 692 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 3762 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* object: tSCRIPT refargs  */
#line 695 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 3768 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 211: /* object: tWINDOW refargs  */
#line 696 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 3774 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* refargs: simpleexpr  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3784 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* refargs: '(' ')'  */
#line 704 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 3792 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* refargs: '(' expr ',' ')'  */
#line 707 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3802 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3811 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* the: tTHE ID  */
#line 718 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3817 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* the: tTHE ID tOF theobj  */
#line 719 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3823 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* the: tTHE tNUMBER tOF theobj  */
#line 720 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 3829 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 223: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 727 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3835 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 224: /* theobj: tSOUND simpleexpr  */
#line 728 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3841 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 225: /* theobj: tSPRITE simpleexpr  */
#line 729 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3847 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 226: /* menu: tMENU simpleexpr  */
#line 732 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3853 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 227: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 734 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3859 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 735 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3865 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 229: /* thedatetime: tTHE tABBREV tDATE  */
#line 736 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3871 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 230: /* thedatetime: tTHE tABBREV tTIME  */
#line 737 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3877 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* thedatetime: tTHE tABBR tDATE  */
#line 738 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 3883 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 232: /* thedatetime: tTHE tABBR tTIME  */
#line 739 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 3889 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* thedatetime: tTHE tLONG tDATE  */
#line 740 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 3895 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* thedatetime: tTHE tLONG tTIME  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 3901 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* thedatetime: tTHE tSHORT tDATE  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 3907 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* thedatetime: tTHE tSHORT tTIME  */
#line 743 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 3913 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 747 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 3919 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 748 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 3925 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 749 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 3931 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 750 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 3937 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 751 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 3943 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* thenumberof: tTHE tNUMBER tOF tMENUS  */
#line 752 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenus, nullptr); }
#line 3949 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* thenumberof: tTHE tNUMBER tOF tXTRAS  */
#line 753 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfXtras, nullptr); }
#line 3955 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* thenumberof: tTHE tNUMBER tOF tCASTLIBS  */
#line 754 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNumberOfNode(kNumberOfCastlibs, nullptr); }
#line 3961 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* writablethe: tTHE ID  */
#line 759 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 3967 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* writablethe: tTHE ID tOF writabletheobj  */
#line 760 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 3973 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* writabletheobj: tMENU expr_noeq  */
#line 764 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 3979 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 765 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 3985 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* writabletheobj: tSOUND expr_noeq  */
#line 766 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 3991 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* writabletheobj: tSPRITE expr_noeq  */
#line 767 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 3997 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* list: '[' exprlist ']'  */
#line 770 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 4003 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* list: '[' ':' ']'  */
#line 771 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 4009 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* list: '[' proplist ']'  */
#line 772 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 4015 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* proplist: proppair  */
#line 778 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* proplist: proplist ',' proppair  */
#line 782 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4032 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* proplist: proplist ',' expr  */
#line 785 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4040 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* proppair: tSYMBOL ':' expr  */
#line 790 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4046 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 261: /* proppair: ID ':' expr  */
#line 791 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4052 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 262: /* proppair: tSTRING ':' expr  */
#line 792 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 263: /* proppair: tINT ':' expr  */
#line 793 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new IntNode((yyvsp[-2].i)), (yyvsp[0].node)); }
#line 4064 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 264: /* proppair: tFLOAT ':' expr  */
#line 794 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new FloatNode((yyvsp[-2].f)), (yyvsp[0].node)); }
#line 4070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 265: /* unarymath: '+' simpleexpr  */
#line 797 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 4076 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* unarymath: '-' simpleexpr  */
#line 798 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 4082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 271: /* expr: expr '+' expr  */
#line 808 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4088 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* expr: expr '-' expr  */
#line 809 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* expr: expr '*' expr  */
#line 810 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* expr: expr '/' expr  */
#line 811 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4106 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* expr: expr tMOD expr  */
#line 812 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4112 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* expr: expr '>' expr  */
#line 813 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4118 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* expr: expr '<' expr  */
#line 814 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* expr: expr tEQ expr  */
#line 815 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4130 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* expr: expr tNEQ expr  */
#line 816 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4136 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* expr: expr tGE expr  */
#line 817 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* expr: expr tLE expr  */
#line 818 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* expr: expr tAND expr  */
#line 819 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4154 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 283: /* expr: expr tOR expr  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4160 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 284: /* expr: expr '&' expr  */
#line 821 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4166 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* expr: expr tCONCAT expr  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* expr: expr tCONTAINS expr  */
#line 823 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4178 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 287: /* expr: expr tSTARTS expr  */
#line 824 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4184 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 290: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 833 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 291: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 834 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4196 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 292: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 835 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4202 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 836 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4208 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 294: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 837 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4214 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 295: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 838 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 296: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 839 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4226 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 840 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4232 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 841 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4238 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 842 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4244 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 843 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4250 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 301: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 844 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4256 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 302: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 845 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4262 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 303: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 846 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4268 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 304: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 847 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4274 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 848 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4280 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 849 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4286 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 309: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 854 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4292 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 310: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 855 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4298 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 311: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 856 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 312: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 857 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4310 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 313: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 858 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4316 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 314: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 859 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 315: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 860 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4328 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 861 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 862 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4340 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 863 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4346 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 864 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4352 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 865 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 866 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4364 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 322: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 867 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 323: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 868 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4376 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 324: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 869 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4382 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 325: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 872 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 326: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 873 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 4394 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 327: /* exprlist: %empty  */
#line 876 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 4400 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 329: /* nonemptyexprlist: expr  */
#line 880 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4409 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 330: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 884 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4417 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 4421 "engines/director/lingo/lingo-gr.cpp"

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

#line 889 "engines/director/lingo/lingo-gr.y"


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
