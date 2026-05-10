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

/* Substitute the type names.  */
#define YYSTYPE         PRIVATE_STYPE
/* Substitute the variable and function names.  */
#define yyparse         PRIVATE_parse
#define yylex           PRIVATE_lex
#define yyerror         PRIVATE_error
#define yydebug         PRIVATE_debug
#define yynerrs         PRIVATE_nerrs
#define yylval          PRIVATE_lval
#define yychar          PRIVATE_char

/* First part of user prologue.  */
#line 50 "engines/private/grammar.y"


#include "private/private.h"
#include "private/grammar.h"

#undef yyerror
#define yyerror	 PRIVATE_xerror

#define code1(c1)       code(c1);
#define code2(c1,c2)    code(c1); code(c2)
#define code3(c1,c2,c3) code(c1); code(c2); code(c3)

using namespace Private;
using namespace Gen;
using namespace Settings;

extern int PRIVATE_lex();
//extern int PRIVATE_parse();

int markplus();

void PRIVATE_xerror(const char *str) {
}

int PRIVATE_wrap() {
    return 1;
}



#line 110 "engines/private/grammar.cpp"

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

#include "tokens.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NAME = 3,                       /* NAME  */
  YYSYMBOL_STRING = 4,                     /* STRING  */
  YYSYMBOL_NUM = 5,                        /* NUM  */
  YYSYMBOL_NUM_PLUS = 6,                   /* NUM_PLUS  */
  YYSYMBOL_LTE = 7,                        /* LTE  */
  YYSYMBOL_GTE = 8,                        /* GTE  */
  YYSYMBOL_NEQ = 9,                        /* NEQ  */
  YYSYMBOL_EQ = 10,                        /* EQ  */
  YYSYMBOL_FALSETOK = 11,                  /* FALSETOK  */
  YYSYMBOL_TRUETOK = 12,                   /* TRUETOK  */
  YYSYMBOL_NULLTOK = 13,                   /* NULLTOK  */
  YYSYMBOL_IFTOK = 14,                     /* IFTOK  */
  YYSYMBOL_ELSETOK = 15,                   /* ELSETOK  */
  YYSYMBOL_RECT = 16,                      /* RECT  */
  YYSYMBOL_GOTOTOK = 17,                   /* GOTOTOK  */
  YYSYMBOL_DEBUGTOK = 18,                  /* DEBUGTOK  */
  YYSYMBOL_EMITCODEONTOK = 19,             /* EMITCODEONTOK  */
  YYSYMBOL_EMITCODEOFFTOK = 20,            /* EMITCODEOFFTOK  */
  YYSYMBOL_RESETIDTOK = 21,                /* RESETIDTOK  */
  YYSYMBOL_DEFINETOK = 22,                 /* DEFINETOK  */
  YYSYMBOL_SETTINGTOK = 23,                /* SETTINGTOK  */
  YYSYMBOL_RANDOMTOK = 24,                 /* RANDOMTOK  */
  YYSYMBOL_25_ = 25,                       /* '{'  */
  YYSYMBOL_26_ = 26,                       /* '}'  */
  YYSYMBOL_27_ = 27,                       /* ','  */
  YYSYMBOL_28_ = 28,                       /* ';'  */
  YYSYMBOL_29_ = 29,                       /* '('  */
  YYSYMBOL_30_ = 30,                       /* ')'  */
  YYSYMBOL_31_ = 31,                       /* '!'  */
  YYSYMBOL_32_ = 32,                       /* '+'  */
  YYSYMBOL_33_ = 33,                       /* '<'  */
  YYSYMBOL_34_ = 34,                       /* '>'  */
  YYSYMBOL_35_ = 35,                       /* '%'  */
  YYSYMBOL_YYACCEPT = 36,                  /* $accept  */
  YYSYMBOL_lines = 37,                     /* lines  */
  YYSYMBOL_line = 38,                      /* line  */
  YYSYMBOL_debug = 39,                     /* debug  */
  YYSYMBOL_statements = 40,                /* statements  */
  YYSYMBOL_statement = 41,                 /* statement  */
  YYSYMBOL_body = 42,                      /* body  */
  YYSYMBOL_end = 43,                       /* end  */
  YYSYMBOL_if = 44,                        /* if  */
  YYSYMBOL_cond = 45,                      /* cond  */
  YYSYMBOL_define = 46,                    /* define  */
  YYSYMBOL_fcall = 47,                     /* fcall  */
  YYSYMBOL_startp = 48,                    /* startp  */
  YYSYMBOL_params = 49,                    /* params  */
  YYSYMBOL_value = 50,                     /* value  */
  YYSYMBOL_expr = 51                       /* expr  */
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
typedef yytype_int8 yy_state_t;

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

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined PRIVATE_STYPE_IS_TRIVIAL && PRIVATE_STYPE_IS_TRIVIAL)))

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
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   127

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  36
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  53
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  116

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   279


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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    31,     2,     2,     2,    35,     2,     2,
      29,    30,     2,    32,    27,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    28,
      33,     2,    34,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    25,     2,    26,     2,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24
};

#if PRIVATE_DEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    98,    98,    99,   102,   103,   104,   105,   106,   107,
     111,   112,   115,   116,   119,   126,   127,   132,   140,   141,
     144,   147,   150,   153,   154,   159,   163,   164,   167,   175,
     176,   184,   187,   188,   189,   190,   191,   194,   195,   196,
     197,   198,   199,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if PRIVATE_DEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NAME", "STRING",
  "NUM", "NUM_PLUS", "LTE", "GTE", "NEQ", "EQ", "FALSETOK", "TRUETOK",
  "NULLTOK", "IFTOK", "ELSETOK", "RECT", "GOTOTOK", "DEBUGTOK",
  "EMITCODEONTOK", "EMITCODEOFFTOK", "RESETIDTOK", "DEFINETOK",
  "SETTINGTOK", "RANDOMTOK", "'{'", "'}'", "','", "';'", "'('", "')'",
  "'!'", "'+'", "'<'", "'>'", "'%'", "$accept", "lines", "line", "debug",
  "statements", "statement", "body", "end", "if", "cond", "define",
  "fcall", "startp", "params", "value", "expr", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-74)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      69,   -17,   -74,   -74,   -74,    12,    13,    19,    69,    21,
       2,     6,   -74,   -74,    -1,    18,    40,    36,    21,   -74,
      22,    20,    16,   -74,    25,     4,    29,    36,    42,    23,
     -74,     7,   -74,   -74,    52,    30,    71,   -74,   -74,    17,
      56,   -74,    46,   -74,     1,    53,   -74,    49,   -74,   -74,
     -74,   -74,   -74,   -74,    64,    73,    28,    65,    36,   -74,
     -74,    77,    16,    67,    70,    68,    72,    89,   -74,    95,
     -74,    73,    73,    73,    73,    73,    73,    73,   -74,    75,
      87,    76,     1,   -74,     1,    78,    74,   -74,   -74,   -74,
     -74,   -74,   -74,   -74,   -74,    56,    99,   -74,   -74,   101,
      80,   -74,    81,    84,   -74,   -74,   102,   107,    86,    85,
     109,   -74,    88,    90,    40,   -74
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     5,     6,     7,     0,     0,     0,     3,    10,
       0,     0,     1,     2,     0,     0,    23,    12,    10,     4,
      27,     0,     0,    21,     0,     0,     0,    12,     0,     0,
      11,    23,     8,    31,     0,     0,     0,     9,    13,     0,
       0,    15,     0,    26,    32,     0,    14,     0,    42,    41,
      40,    38,    39,    37,     0,     0,    43,     0,    12,    18,
      20,     0,    42,     0,    36,     0,    35,     0,    28,     0,
      44,     0,     0,     0,     0,    52,     0,     0,    22,     0,
      16,     0,    32,    30,    32,     0,     0,    50,    51,    46,
      45,    47,    48,    49,    19,     0,     0,    33,    34,     0,
       0,    20,     0,     0,    53,    17,     0,     0,     0,     0,
       0,    29,     0,    25,    23,    24
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -74,   108,   -74,   103,   -24,   -39,    24,    26,   -74,   -74,
     -31,   -42,   -74,   -73,    -8,    83
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     7,     8,    15,    26,    27,    60,    80,    28,    40,
      21,    29,    44,    65,    56,    66
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      43,    59,    64,    38,    62,    49,    50,    35,     9,    97,
      20,    98,    51,    52,    53,    10,    11,    24,    63,    12,
      48,    49,    50,    42,    14,    54,    18,    16,    51,    52,
      53,    17,    55,    36,    79,    71,    72,    73,    74,    22,
      64,    54,    64,    20,    19,    33,    32,    70,    55,    31,
      23,    41,    24,    25,    34,    37,    59,    45,    46,    22,
      75,    76,    77,    87,    88,    89,    90,    91,    92,    93,
      23,    39,    24,    25,    47,    61,    48,    49,    50,    68,
      67,    58,    81,   115,    51,    52,    53,     1,     2,     3,
       4,     5,     6,    69,    85,    78,    36,    82,    83,    84,
      86,    94,    95,    96,   102,    99,   103,   108,   106,   100,
     104,   107,   109,   110,   112,   111,    13,   114,   113,   101,
       0,    30,    57,     0,     0,     0,     0,   105
};

static const yytype_int8 yycheck[] =
{
      31,    40,    44,    27,     3,     4,     5,     3,    25,    82,
       3,    84,    11,    12,    13,     3,     3,    16,    17,     0,
       3,     4,     5,    16,     3,    24,    27,    25,    11,    12,
      13,    25,    31,    29,    58,     7,     8,     9,    10,     3,
      82,    24,    84,     3,    26,    29,    26,    55,    31,    27,
      14,    28,    16,    17,    29,    26,    95,     5,    28,     3,
      32,    33,    34,    71,    72,    73,    74,    75,    76,    77,
      14,    29,    16,    17,     3,    29,     3,     4,     5,    30,
      27,    25,     5,   114,    11,    12,    13,    18,    19,    20,
      21,    22,    23,    29,     5,    30,    29,    27,    30,    27,
       5,    26,    15,    27,     5,    27,     5,     5,    27,    35,
      30,    27,     5,    27,     5,    30,     8,    27,    30,    95,
      -1,    18,    39,    -1,    -1,    -1,    -1,   101
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    18,    19,    20,    21,    22,    23,    37,    38,    25,
       3,     3,     0,    37,     3,    39,    25,    25,    27,    26,
       3,    46,     3,    14,    16,    17,    40,    41,    44,    47,
      39,    27,    26,    29,    29,     3,    29,    26,    40,    29,
      45,    28,    16,    46,    48,     5,    28,     3,     3,     4,
       5,    11,    12,    13,    24,    31,    50,    51,    25,    41,
      42,    29,     3,    17,    47,    49,    51,    27,    30,    29,
      50,     7,     8,     9,    10,    32,    33,    34,    30,    40,
      43,     5,    27,    30,    27,     5,     5,    50,    50,    50,
      50,    50,    50,    50,    26,    15,    27,    49,    49,    27,
      35,    42,     5,     5,    30,    43,    27,    27,     5,     5,
      27,    30,     5,    30,    27,    46
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    36,    37,    37,    38,    38,    38,    38,    38,    38,
      39,    39,    40,    40,    41,    41,    41,    41,    42,    42,
      43,    44,    45,    46,    46,    46,    46,    46,    47,    47,
      47,    48,    49,    49,    49,    49,    49,    50,    50,    50,
      50,    50,    50,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    51,    51
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     4,     1,     1,     1,     5,     5,
       0,     3,     0,     2,     3,     2,     4,     7,     1,     3,
       0,     1,     3,     0,    14,    12,     3,     1,     4,    10,
       5,     0,     0,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     3,     3,     3,     3,
       3,     3,     2,     5
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = PRIVATE_EMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == PRIVATE_EMPTY)                                        \
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
   Use PRIVATE_error or PRIVATE_UNDEF. */
#define YYERRCODE PRIVATE_UNDEF


/* Enable debugging if requested.  */
#if PRIVATE_DEBUG

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
#else /* !PRIVATE_DEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !PRIVATE_DEBUG */


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
  YY_USE (yykind);
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

  yychar = PRIVATE_EMPTY; /* Cause a token to be read.  */

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
  if (yychar == PRIVATE_EMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= PRIVATE_EOF)
    {
      yychar = PRIVATE_EOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == PRIVATE_error)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = PRIVATE_UNDEF;
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
  yychar = PRIVATE_EMPTY;
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
  case 4: /* line: DEBUGTOK '{' debug '}'  */
#line 102 "engines/private/grammar.y"
                             { /* Not used in the game */ }
#line 1216 "engines/private/grammar.cpp"
    break;

  case 5: /* line: EMITCODEONTOK  */
#line 103 "engines/private/grammar.y"
                        { /* Unclear what this is */ }
#line 1222 "engines/private/grammar.cpp"
    break;

  case 6: /* line: EMITCODEOFFTOK  */
#line 104 "engines/private/grammar.y"
                         { /* Unclear what this is */ }
#line 1228 "engines/private/grammar.cpp"
    break;

  case 7: /* line: RESETIDTOK  */
#line 105 "engines/private/grammar.y"
                     { /* Unclear what this is */ }
#line 1234 "engines/private/grammar.cpp"
    break;

  case 8: /* line: DEFINETOK NAME '{' define '}'  */
#line 106 "engines/private/grammar.y"
                                        { g_private->maps.installAll((yyvsp[-3].s)); }
#line 1240 "engines/private/grammar.cpp"
    break;

  case 9: /* line: SETTINGTOK NAME '{' statements '}'  */
#line 107 "engines/private/grammar.y"
                                             { g_setts->save((yyvsp[-3].s));
					       g_setts->init(); }
#line 1247 "engines/private/grammar.cpp"
    break;

  case 12: /* statements: %empty  */
#line 115 "engines/private/grammar.y"
                               { (yyval.inst) = g_vm->_progp; }
#line 1253 "engines/private/grammar.cpp"
    break;

  case 14: /* statement: GOTOTOK NAME ';'  */
#line 119 "engines/private/grammar.y"
                            {
	(yyval.inst) = g_vm->_progp;
	code2(strpush, (Inst) g_private->maps.constant(STRING, 0, (yyvsp[-1].s)));
	code2(constpush, (Inst) g_private->maps.constant(NUM, 1, NULL));
	code2(strpush, (Inst) g_private->maps.constant(STRING, 0, "goto"));
	code1(funcpush);
	}
#line 1265 "engines/private/grammar.cpp"
    break;

  case 15: /* statement: fcall ';'  */
#line 126 "engines/private/grammar.y"
                         { (yyval.inst) = (yyvsp[-1].inst); }
#line 1271 "engines/private/grammar.cpp"
    break;

  case 16: /* statement: if cond body end  */
#line 127 "engines/private/grammar.y"
                           {
		/* else-less if */
		((yyvsp[-3].inst))[1] = (Inst)(yyvsp[-1].inst);     /* thenpart */
		((yyvsp[-3].inst))[3] = (Inst)(yyvsp[0].inst);
		}
#line 1281 "engines/private/grammar.cpp"
    break;

  case 17: /* statement: if cond body end ELSETOK body end  */
#line 132 "engines/private/grammar.y"
                                            {
		/* if with else */
		((yyvsp[-6].inst))[1] = (Inst)(yyvsp[-4].inst);     /* thenpart */
		((yyvsp[-6].inst))[2] = (Inst)(yyvsp[-1].inst);     /* elsepart */
		((yyvsp[-6].inst))[3] = (Inst)(yyvsp[0].inst);
		}
#line 1292 "engines/private/grammar.cpp"
    break;

  case 18: /* body: statement  */
#line 140 "engines/private/grammar.y"
                        { (yyval.inst) = (yyvsp[0].inst); }
#line 1298 "engines/private/grammar.cpp"
    break;

  case 19: /* body: '{' statements '}'  */
#line 141 "engines/private/grammar.y"
                             { (yyval.inst) = (yyvsp[-1].inst); }
#line 1304 "engines/private/grammar.cpp"
    break;

  case 20: /* end: %empty  */
#line 144 "engines/private/grammar.y"
                             { code1(STOP); (yyval.inst) = g_vm->_progp; }
#line 1310 "engines/private/grammar.cpp"
    break;

  case 21: /* if: IFTOK  */
#line 147 "engines/private/grammar.y"
          { (yyval.inst) = code1(ifcode); code3(STOP, STOP, STOP); }
#line 1316 "engines/private/grammar.cpp"
    break;

  case 22: /* cond: '(' expr ')'  */
#line 150 "engines/private/grammar.y"
                        { code1(STOP); (yyval.inst) = (yyvsp[-1].inst); }
#line 1322 "engines/private/grammar.cpp"
    break;

  case 24: /* define: NAME ',' RECT '(' NUM ',' NUM ',' NUM ',' NUM ')' ',' define  */
#line 154 "engines/private/grammar.y"
                                                                        {
	  Common::Rect *r = new Common::Rect((yyvsp[-9].sym)->u.val, (yyvsp[-7].sym)->u.val, (yyvsp[-5].sym)->u.val, (yyvsp[-3].sym)->u.val);
	  assert(r->isValidRect());
	  g_private->maps.defineSymbol((yyvsp[-13].s), r);
	  }
#line 1332 "engines/private/grammar.cpp"
    break;

  case 25: /* define: NAME ',' RECT '(' NUM ',' NUM ',' NUM ',' NUM ')'  */
#line 159 "engines/private/grammar.y"
                                                            {
	  Common::Rect *r = new Common::Rect((yyvsp[-7].sym)->u.val, (yyvsp[-5].sym)->u.val, (yyvsp[-3].sym)->u.val, (yyvsp[-1].sym)->u.val);
	  g_private->maps.defineSymbol((yyvsp[-11].s), r);
	  }
#line 1341 "engines/private/grammar.cpp"
    break;

  case 26: /* define: NAME ',' define  */
#line 163 "engines/private/grammar.y"
                          { g_private->maps.defineSymbol((yyvsp[-2].s), NULL); }
#line 1347 "engines/private/grammar.cpp"
    break;

  case 27: /* define: NAME  */
#line 164 "engines/private/grammar.y"
                    { g_private->maps.defineSymbol((yyvsp[0].s), NULL); }
#line 1353 "engines/private/grammar.cpp"
    break;

  case 28: /* fcall: GOTOTOK '(' NAME ')'  */
#line 167 "engines/private/grammar.y"
                               {
			       (yyval.inst) = g_vm->_progp;
			       code2(strpush, (Inst) g_private->maps.constant(STRING, 0, (yyvsp[-1].s)));
			       code2(constpush, (Inst) g_private->maps.constant(NUM, 1, NULL));
			       code2(strpush, (Inst) g_private->maps.constant(STRING, 0, "goto"));
			       code1(funcpush);
			       }
#line 1365 "engines/private/grammar.cpp"
    break;

  case 29: /* fcall: RECT '(' NUM ',' NUM ',' NUM ',' NUM ')'  */
#line 175 "engines/private/grammar.y"
                                                   { (yyval.inst) = g_vm->_progp; }
#line 1371 "engines/private/grammar.cpp"
    break;

  case 30: /* fcall: NAME '(' startp params ')'  */
#line 176 "engines/private/grammar.y"
                                      {
			       (yyval.inst) = (yyvsp[-2].inst);
			       code2(constpush, (Inst) g_private->maps.constant(NUM, (yyvsp[-1].narg), NULL));
			       code2(strpush, (Inst) g_private->maps.constant(STRING, 0, (yyvsp[-4].s)));
			       code1(funcpush);
			       }
#line 1382 "engines/private/grammar.cpp"
    break;

  case 31: /* startp: %empty  */
#line 184 "engines/private/grammar.y"
                    { (yyval.inst) = g_vm->_progp; }
#line 1388 "engines/private/grammar.cpp"
    break;

  case 32: /* params: %empty  */
#line 187 "engines/private/grammar.y"
                            { (yyval.narg) = 0; }
#line 1394 "engines/private/grammar.cpp"
    break;

  case 33: /* params: fcall ',' params  */
#line 188 "engines/private/grammar.y"
                            { (yyval.narg) = (yyvsp[0].narg) + 1; }
#line 1400 "engines/private/grammar.cpp"
    break;

  case 34: /* params: expr ',' params  */
#line 189 "engines/private/grammar.y"
                            { (yyval.narg) = (yyvsp[0].narg) + 1; }
#line 1406 "engines/private/grammar.cpp"
    break;

  case 35: /* params: expr  */
#line 190 "engines/private/grammar.y"
                { (yyval.narg) = 1; }
#line 1412 "engines/private/grammar.cpp"
    break;

  case 36: /* params: fcall  */
#line 191 "engines/private/grammar.y"
                      { (yyval.narg) = 1; }
#line 1418 "engines/private/grammar.cpp"
    break;

  case 37: /* value: NULLTOK  */
#line 194 "engines/private/grammar.y"
                   { code2(constpush, (Inst) g_private->maps.constant(NUM, 0, NULL)); }
#line 1424 "engines/private/grammar.cpp"
    break;

  case 38: /* value: FALSETOK  */
#line 195 "engines/private/grammar.y"
                   { code2(constpush, (Inst) g_private->maps.constant(NUM, 0, NULL)); }
#line 1430 "engines/private/grammar.cpp"
    break;

  case 39: /* value: TRUETOK  */
#line 196 "engines/private/grammar.y"
                   { code2(constpush, (Inst) g_private->maps.constant(NUM, 1, NULL)); }
#line 1436 "engines/private/grammar.cpp"
    break;

  case 40: /* value: NUM  */
#line 197 "engines/private/grammar.y"
                   { code2(constpush, (Inst)(yyvsp[0].sym)); }
#line 1442 "engines/private/grammar.cpp"
    break;

  case 41: /* value: STRING  */
#line 198 "engines/private/grammar.y"
                   { code2(strpush, (Inst)(yyvsp[0].sym)); }
#line 1448 "engines/private/grammar.cpp"
    break;

  case 42: /* value: NAME  */
#line 199 "engines/private/grammar.y"
                   { code1(varpush); code1((Inst) g_private->maps.lookupName((yyvsp[0].s))); code1(eval); }
#line 1454 "engines/private/grammar.cpp"
    break;

  case 43: /* expr: value  */
#line 202 "engines/private/grammar.y"
                   { (yyval.inst) = (yyvsp[0].inst); }
#line 1460 "engines/private/grammar.cpp"
    break;

  case 44: /* expr: '!' value  */
#line 203 "engines/private/grammar.y"
                          { code1(negate); (yyval.inst) = (yyvsp[0].inst); }
#line 1466 "engines/private/grammar.cpp"
    break;

  case 45: /* expr: value EQ value  */
#line 204 "engines/private/grammar.y"
                          { code1(eq); }
#line 1472 "engines/private/grammar.cpp"
    break;

  case 46: /* expr: value NEQ value  */
#line 205 "engines/private/grammar.y"
                          { code1(ne); }
#line 1478 "engines/private/grammar.cpp"
    break;

  case 47: /* expr: value '+' value  */
#line 206 "engines/private/grammar.y"
                          { code1(add); }
#line 1484 "engines/private/grammar.cpp"
    break;

  case 48: /* expr: value '<' value  */
#line 207 "engines/private/grammar.y"
                          { code1(lt); }
#line 1490 "engines/private/grammar.cpp"
    break;

  case 49: /* expr: value '>' value  */
#line 208 "engines/private/grammar.y"
                          { code1(gt); }
#line 1496 "engines/private/grammar.cpp"
    break;

  case 50: /* expr: value LTE value  */
#line 209 "engines/private/grammar.y"
                          { code1(le); }
#line 1502 "engines/private/grammar.cpp"
    break;

  case 51: /* expr: value GTE value  */
#line 210 "engines/private/grammar.y"
                          { code1(ge); }
#line 1508 "engines/private/grammar.cpp"
    break;

  case 52: /* expr: value '+'  */
#line 211 "engines/private/grammar.y"
                          { code1(markplus); (yyval.inst) = (yyvsp[-1].inst); }
#line 1514 "engines/private/grammar.cpp"
    break;

  case 53: /* expr: RANDOMTOK '(' NUM '%' ')'  */
#line 212 "engines/private/grammar.y"
                                    { code3(constpush, (Inst)(yyvsp[-2].sym), randbool); }
#line 1520 "engines/private/grammar.cpp"
    break;


#line 1524 "engines/private/grammar.cpp"

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
  yytoken = yychar == PRIVATE_EMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= PRIVATE_EOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == PRIVATE_EOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = PRIVATE_EMPTY;
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
  if (yychar != PRIVATE_EMPTY)
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

#line 215 "engines/private/grammar.y"


int markplus() {
	Datum d = pop();
	if (d.type == NUM) {
		d.type = NUM_PLUS;
	}
	push(d);
	return 0;
}
