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
#define YYSTYPE         HYPNO_MIS_STYPE
/* Substitute the variable and function names.  */
#define yyparse         HYPNO_MIS_parse
#define yylex           HYPNO_MIS_lex
#define yyerror         HYPNO_MIS_error
#define yydebug         HYPNO_MIS_debug
#define yynerrs         HYPNO_MIS_nerrs
#define yylval          HYPNO_MIS_lval
#define yychar          HYPNO_MIS_char

/* First part of user prologue.  */
#line 27 "engines/hypno/grammar_mis.y"


#include "common/array.h"
#include "hypno/hypno.h"
//#include <stdio.h>

#undef yyerror
#define yyerror	 HYPNO_MIS_xerror

extern int HYPNO_MIS_lex();
extern int yylineno;

Common::Array<uint32> *smenu_idx = nullptr;
Hypno::HotspotsStack *stack = nullptr;
Hypno::Talk *talk_action = nullptr;

void HYPNO_MIS_xerror(const char *str) {
	error("ERROR: %s", str);
}

int HYPNO_MIS_wrap() {
	return 1;
}

using namespace Hypno;


#line 108 "engines/hypno/grammar_mis.cpp"

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

#include "tokens_mis.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NAME = 3,                       /* NAME  */
  YYSYMBOL_FILENAME = 4,                   /* FILENAME  */
  YYSYMBOL_FLAG = 5,                       /* FLAG  */
  YYSYMBOL_COMMENT = 6,                    /* COMMENT  */
  YYSYMBOL_GSSWITCH = 7,                   /* GSSWITCH  */
  YYSYMBOL_COMMAND = 8,                    /* COMMAND  */
  YYSYMBOL_WALNTOK = 9,                    /* WALNTOK  */
  YYSYMBOL_NUM = 10,                       /* NUM  */
  YYSYMBOL_HOTSTOK = 11,                   /* HOTSTOK  */
  YYSYMBOL_CUTSTOK = 12,                   /* CUTSTOK  */
  YYSYMBOL_BACKTOK = 13,                   /* BACKTOK  */
  YYSYMBOL_INTRTOK = 14,                   /* INTRTOK  */
  YYSYMBOL_RETTOK = 15,                    /* RETTOK  */
  YYSYMBOL_TIMETOK = 16,                   /* TIMETOK  */
  YYSYMBOL_PALETOK = 17,                   /* PALETOK  */
  YYSYMBOL_BBOXTOK = 18,                   /* BBOXTOK  */
  YYSYMBOL_OVERTOK = 19,                   /* OVERTOK  */
  YYSYMBOL_MICETOK = 20,                   /* MICETOK  */
  YYSYMBOL_PLAYTOK = 21,                   /* PLAYTOK  */
  YYSYMBOL_ENDTOK = 22,                    /* ENDTOK  */
  YYSYMBOL_MENUTOK = 23,                   /* MENUTOK  */
  YYSYMBOL_SMENTOK = 24,                   /* SMENTOK  */
  YYSYMBOL_ESCPTOK = 25,                   /* ESCPTOK  */
  YYSYMBOL_NRTOK = 26,                     /* NRTOK  */
  YYSYMBOL_AMBITOK = 27,                   /* AMBITOK  */
  YYSYMBOL_SWPTTOK = 28,                   /* SWPTTOK  */
  YYSYMBOL_MPTRTOK = 29,                   /* MPTRTOK  */
  YYSYMBOL_GLOBTOK = 30,                   /* GLOBTOK  */
  YYSYMBOL_TONTOK = 31,                    /* TONTOK  */
  YYSYMBOL_TOFFTOK = 32,                   /* TOFFTOK  */
  YYSYMBOL_TALKTOK = 33,                   /* TALKTOK  */
  YYSYMBOL_INACTOK = 34,                   /* INACTOK  */
  YYSYMBOL_FDTOK = 35,                     /* FDTOK  */
  YYSYMBOL_BOXXTOK = 36,                   /* BOXXTOK  */
  YYSYMBOL_ESCAPETOK = 37,                 /* ESCAPETOK  */
  YYSYMBOL_SECONDTOK = 38,                 /* SECONDTOK  */
  YYSYMBOL_INTROTOK = 39,                  /* INTROTOK  */
  YYSYMBOL_DEFAULTTOK = 40,                /* DEFAULTTOK  */
  YYSYMBOL_PG = 41,                        /* PG  */
  YYSYMBOL_PA = 42,                        /* PA  */
  YYSYMBOL_PD = 43,                        /* PD  */
  YYSYMBOL_PH = 44,                        /* PH  */
  YYSYMBOL_PF = 45,                        /* PF  */
  YYSYMBOL_PE = 46,                        /* PE  */
  YYSYMBOL_PP = 47,                        /* PP  */
  YYSYMBOL_PI = 48,                        /* PI  */
  YYSYMBOL_PL = 49,                        /* PL  */
  YYSYMBOL_PS = 50,                        /* PS  */
  YYSYMBOL_YYACCEPT = 51,                  /* $accept  */
  YYSYMBOL_start = 52,                     /* start  */
  YYSYMBOL_init = 53,                      /* init  */
  YYSYMBOL_lines = 54,                     /* lines  */
  YYSYMBOL_line = 55,                      /* line  */
  YYSYMBOL_anything = 56,                  /* anything  */
  YYSYMBOL_alloctalk = 57,                 /* alloctalk  */
  YYSYMBOL_talk = 58,                      /* talk  */
  YYSYMBOL_mflag = 59,                     /* mflag  */
  YYSYMBOL_flag = 60,                      /* flag  */
  YYSYMBOL_gsswitch = 61                   /* gsswitch  */
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
typedef yytype_uint8 yy_state_t;

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
         || (defined HYPNO_MIS_STYPE_IS_TRIVIAL && HYPNO_MIS_STYPE_IS_TRIVIAL)))

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
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   125

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  51
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  11
/* YYNRULES -- Number of rules.  */
#define YYNRULES  54
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  137

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   305


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
      45,    46,    47,    48,    49,    50
};

#if HYPNO_MIS_DEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    73,    73,    76,    87,    88,    92,   108,   114,   129,
     135,   141,   142,   149,   156,   162,   168,   174,   181,   188,
     195,   202,   208,   214,   217,   223,   229,   232,   233,   236,
     243,   246,   247,   251,   254,   257,   261,   265,   270,   276,
     277,   283,   289,   295,   302,   309,   315,   320,   321,   324,
     325,   328,   329,   332,   333
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if HYPNO_MIS_DEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NAME", "FILENAME",
  "FLAG", "COMMENT", "GSSWITCH", "COMMAND", "WALNTOK", "NUM", "HOTSTOK",
  "CUTSTOK", "BACKTOK", "INTRTOK", "RETTOK", "TIMETOK", "PALETOK",
  "BBOXTOK", "OVERTOK", "MICETOK", "PLAYTOK", "ENDTOK", "MENUTOK",
  "SMENTOK", "ESCPTOK", "NRTOK", "AMBITOK", "SWPTTOK", "MPTRTOK",
  "GLOBTOK", "TONTOK", "TOFFTOK", "TALKTOK", "INACTOK", "FDTOK", "BOXXTOK",
  "ESCAPETOK", "SECONDTOK", "INTROTOK", "DEFAULTTOK", "PG", "PA", "PD",
  "PH", "PF", "PE", "PP", "PI", "PL", "PS", "$accept", "start", "init",
  "lines", "line", "anything", "alloctalk", "talk", "mflag", "flag",
  "gsswitch", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-85)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -85,     5,    77,   -85,     9,    -4,    13,    14,    15,   -85,
      10,    18,    20,    21,    23,    25,    27,   -85,   -85,    28,
      26,    29,    24,   -85,   -85,    77,    30,    31,   -85,    33,
      34,    27,   -85,    36,    40,    41,    25,    22,   -85,    27,
      42,   -85,    43,    32,    35,   -85,    44,    45,    46,    47,
     -85,    49,   -85,    50,   -85,   -85,    27,    51,    52,   -85,
      59,    35,    35,    54,   -85,    61,    62,    63,    35,    35,
      35,    35,    35,   -85,    85,    93,    35,    35,   -85,    38,
      98,    38,   -85,    82,    38,   -85,    82,    99,   101,   -85,
     -85,   102,   103,   104,   105,   -85,   -85,   -85,   -85,   -85,
     106,   107,   -85,   -85,   -85,    82,   108,    82,   -85,   -85,
      82,   -85,   109,   110,   -85,   111,   112,   113,    82,    35,
     -85,   -85,    82,   -85,   114,    38,    82,   -85,   -85,    35,
     -85,   -85,   -85,    82,   -85,   -85,   -85
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     5,     1,     0,     0,     0,     0,     0,    26,
       0,     0,     0,     0,     0,    28,    50,     8,     9,     0,
       0,     0,     0,    29,     2,     5,     0,     0,    20,     0,
      19,    50,    17,     0,     0,     0,    28,     0,    49,    50,
       0,    11,     0,     0,    48,     4,     0,     0,     0,     0,
      10,     0,    22,     0,    27,    25,    50,     0,     0,    13,
       0,    48,    48,     0,    34,     0,     0,     0,    48,    48,
      48,    48,    48,    47,     0,     0,    48,    48,    24,    54,
       0,    54,    18,    52,    54,     6,    52,     0,     0,    30,
      31,     0,     0,     0,     0,    38,    41,    42,    39,    40,
       0,     0,    46,    45,    53,    52,     0,    52,    51,    16,
      52,    14,     0,     0,    33,     0,     0,     0,    52,    48,
      21,     7,    52,    15,     0,    54,    52,    36,    37,    48,
      44,    12,    23,    52,    35,    43,    32
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -85,   -85,   -85,   100,   -85,     3,   -85,   -61,   -27,   -84,
     -78
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,    24,    25,    37,    44,    78,    39,   109,
     105
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      89,    90,   111,   107,    50,     3,   110,    95,    96,    97,
      98,    99,    56,    26,    27,   102,   103,    28,    29,    30,
      31,   120,    32,   122,    33,    34,   123,    35,    36,    85,
      38,    43,    40,    42,   129,    59,    41,    55,   131,    54,
      46,    47,   134,    48,    49,   104,    51,   133,    60,   136,
      52,    53,    57,    58,    79,    80,    81,    82,   130,    83,
      84,    86,    87,    88,    91,    92,    93,    94,   135,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,     4,   108,     5,     6,
       7,     8,     9,    10,    11,   100,    12,    13,    14,    15,
      16,    17,    18,   101,    19,    20,    21,    22,   106,   112,
      23,   113,   114,   115,   116,   117,   118,   119,   121,   124,
     125,   126,   127,   128,   132,    45
};

static const yytype_uint8 yycheck[] =
{
      61,    62,    86,    81,    31,     0,    84,    68,    69,    70,
      71,    72,    39,     4,    18,    76,    77,     4,     4,     4,
      10,   105,     4,   107,     4,     4,   110,     4,     3,    56,
       3,     7,     4,     4,   118,     3,    10,    15,   122,    36,
      10,    10,   126,    10,    10,     7,    10,   125,    13,   133,
      10,    10,    10,    10,    10,    10,    10,    10,   119,    10,
      10,    10,    10,     4,    10,     4,     4,     4,   129,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,     9,     5,    11,    12,
      13,    14,    15,    16,    17,    10,    19,    20,    21,    22,
      23,    24,    25,    10,    27,    28,    29,    30,    10,    10,
      33,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    25
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    52,    53,     0,     9,    11,    12,    13,    14,    15,
      16,    17,    19,    20,    21,    22,    23,    24,    25,    27,
      28,    29,    30,    33,    54,    55,     4,    18,     4,     4,
       4,    10,     4,     4,     4,     4,     3,    56,     3,    59,
       4,    10,     4,     7,    57,    54,    10,    10,    10,    10,
      59,    10,    10,    10,    56,    15,    59,    10,    10,     3,
      13,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    58,    10,
      10,    10,    10,    10,    10,    59,    10,    10,     4,    58,
      58,    10,     4,     4,     4,    58,    58,    58,    58,    58,
      10,    10,    58,    58,     7,    61,    10,    61,     5,    60,
      61,    60,    10,    10,    10,    10,    10,    10,    10,    10,
      60,    10,    60,    60,    10,    10,    10,    10,    10,    60,
      58,    60,    10,    61,    60,    58,    60
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    51,    52,    53,    54,    54,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    56,    56,    57,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    59,
      59,    60,    60,    61,    61
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     0,     2,     0,     4,     6,     1,     1,
       3,     2,     7,     3,     5,     6,     5,     2,     4,     2,
       2,     6,     3,     7,     3,     3,     1,     2,     0,     0,
       2,     2,     6,     3,     1,     5,     4,     4,     2,     2,
       2,     2,     2,     5,     4,     2,     2,     1,     0,     1,
       0,     1,     0,     1,     0
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = HYPNO_MIS_EMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == HYPNO_MIS_EMPTY)                                        \
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
   Use HYPNO_MIS_error or HYPNO_MIS_UNDEF. */
#define YYERRCODE HYPNO_MIS_UNDEF


/* Enable debugging if requested.  */
#if HYPNO_MIS_DEBUG

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
#else /* !HYPNO_MIS_DEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !HYPNO_MIS_DEBUG */


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

  yychar = HYPNO_MIS_EMPTY; /* Cause a token to be read.  */

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
  if (yychar == HYPNO_MIS_EMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= HYPNO_MIS_EOF)
    {
      yychar = HYPNO_MIS_EOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == HYPNO_MIS_error)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = HYPNO_MIS_UNDEF;
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
  yychar = HYPNO_MIS_EMPTY;
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
  case 3: /* init: %empty  */
#line 76 "engines/hypno/grammar_mis.y"
      { 
	if (smenu_idx)
		delete smenu_idx;
	smenu_idx = new Common::Array<uint32>();
	smenu_idx->push_back(-1);
	if (stack)
		delete stack;
	stack = new Hypno::HotspotsStack();
	stack->push_back(new Hotspots());
}
#line 1244 "engines/hypno/grammar_mis.cpp"
    break;

  case 6: /* line: MENUTOK mflag mflag mflag  */
#line 92 "engines/hypno/grammar_mis.y"
                                {
		Hotspot hot(MakeMenu); 
		debugC(1, kHypnoDebugParser, "MENU %s %s", (yyvsp[-2].s), (yyvsp[-1].s));
		hot.flags[0] = (yyvsp[-2].s);
		hot.flags[1] = (yyvsp[-1].s);
		hot.flags[2] = (yyvsp[0].s);

		Hotspots *cur = stack->back();
		cur->push_back(hot);

		// We don't care about menus, only hotspots
		int idx = smenu_idx->back();
		idx++;
		smenu_idx->pop_back();
		smenu_idx->push_back(idx);
	}
#line 1265 "engines/hypno/grammar_mis.cpp"
    break;

  case 7: /* line: HOTSTOK BBOXTOK NUM NUM NUM NUM  */
#line 108 "engines/hypno/grammar_mis.y"
                                           {  
		Hotspot hot(MakeHotspot, Common::Rect((yyvsp[-3].i), (yyvsp[-2].i), (yyvsp[-1].i), (yyvsp[0].i))); 
		debugC(1, kHypnoDebugParser, "HOTS %d.", hot.type);
		Hotspots *cur = stack->back();
		cur->push_back(hot); 
	}
#line 1276 "engines/hypno/grammar_mis.cpp"
    break;

  case 8: /* line: SMENTOK  */
#line 114 "engines/hypno/grammar_mis.y"
                   { 
		// This should always point to a hotspot
		int idx = smenu_idx->back();
		idx++;
		smenu_idx->pop_back();
		smenu_idx->push_back(idx);

		Hotspots *cur = stack->back();
		Hotspot *hot = &(*cur)[idx];

		smenu_idx->push_back(-1);
		hot->smenu = new Hotspots();
		stack->push_back(hot->smenu);
		debugC(1, kHypnoDebugParser, "SUBMENU");
	}
#line 1296 "engines/hypno/grammar_mis.cpp"
    break;

  case 9: /* line: ESCPTOK  */
#line 129 "engines/hypno/grammar_mis.y"
                    {
		Escape *a = new Escape();
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "ESC SUBMENU"); }
#line 1307 "engines/hypno/grammar_mis.cpp"
    break;

  case 10: /* line: TIMETOK NUM mflag  */
#line 135 "engines/hypno/grammar_mis.y"
                              { 
		Timer *a = new Timer((yyvsp[-1].i));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "TIME %d", (yyvsp[-1].i)); }
#line 1318 "engines/hypno/grammar_mis.cpp"
    break;

  case 11: /* line: SWPTTOK NUM  */
#line 141 "engines/hypno/grammar_mis.y"
                       { debugC(1, kHypnoDebugParser, "SWPT %d", (yyvsp[0].i)); }
#line 1324 "engines/hypno/grammar_mis.cpp"
    break;

  case 12: /* line: BACKTOK FILENAME NUM NUM gsswitch flag flag  */
#line 142 "engines/hypno/grammar_mis.y"
                                                       {
		Background *a = new Background((yyvsp[-5].s), Common::Point((yyvsp[-4].i), (yyvsp[-3].i)), (yyvsp[-2].s), (yyvsp[-1].s), (yyvsp[0].s));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "BACK");
	}
#line 1336 "engines/hypno/grammar_mis.cpp"
    break;

  case 13: /* line: GLOBTOK GSSWITCH NAME  */
#line 149 "engines/hypno/grammar_mis.y"
                                  { 
		Global *a = new Global((yyvsp[-1].s), (yyvsp[0].s));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "GLOB"); 
	}
#line 1348 "engines/hypno/grammar_mis.cpp"
    break;

  case 14: /* line: AMBITOK FILENAME NUM NUM flag  */
#line 156 "engines/hypno/grammar_mis.y"
                                         { 
		Ambient *a = new Ambient((yyvsp[-3].s), Common::Point((yyvsp[-2].i), (yyvsp[-1].i)), (yyvsp[0].s));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);			
		debugC(1, kHypnoDebugParser, "AMBI %d %d", (yyvsp[-2].i), (yyvsp[-1].i)); }
#line 1359 "engines/hypno/grammar_mis.cpp"
    break;

  case 15: /* line: PLAYTOK FILENAME NUM NUM gsswitch flag  */
#line 162 "engines/hypno/grammar_mis.y"
                                                  { 
		Play *a = new Play((yyvsp[-4].s), Common::Point((yyvsp[-3].i), (yyvsp[-2].i)), (yyvsp[-1].s), (yyvsp[0].s));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);		  
		debugC(1, kHypnoDebugParser, "PLAY %s.", (yyvsp[-4].s)); }
#line 1370 "engines/hypno/grammar_mis.cpp"
    break;

  case 16: /* line: OVERTOK FILENAME NUM NUM flag  */
#line 168 "engines/hypno/grammar_mis.y"
                                         { 
		Overlay *a = new Overlay((yyvsp[-3].s), Common::Point((yyvsp[-2].i), (yyvsp[-1].i)), (yyvsp[0].s));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
	}
#line 1381 "engines/hypno/grammar_mis.cpp"
    break;

  case 17: /* line: PALETOK FILENAME  */
#line 174 "engines/hypno/grammar_mis.y"
                            {
		Palette *a = new Palette((yyvsp[0].s));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "PALE");
	}
#line 1393 "engines/hypno/grammar_mis.cpp"
    break;

  case 18: /* line: INTRTOK FILENAME NUM NUM  */
#line 181 "engines/hypno/grammar_mis.y"
                                    { 
		Intro *a = new Intro(Common::String("cine/") + (yyvsp[-2].s));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "INTRO %s %d %d", (yyvsp[-2].s), (yyvsp[-1].i), (yyvsp[0].i)); 
	}
#line 1405 "engines/hypno/grammar_mis.cpp"
    break;

  case 19: /* line: INTRTOK FILENAME  */
#line 188 "engines/hypno/grammar_mis.y"
                            { 
		Intro *a = new Intro(Common::String("cine/") + (yyvsp[0].s));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
		debugC(1, kHypnoDebugParser, "INTRO %s", (yyvsp[0].s)); 
	}
#line 1417 "engines/hypno/grammar_mis.cpp"
    break;

  case 20: /* line: CUTSTOK FILENAME  */
#line 195 "engines/hypno/grammar_mis.y"
                            { 
		Cutscene *a = new Cutscene((yyvsp[0].s));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);		  
		debugC(1, kHypnoDebugParser, "CUTS %s", (yyvsp[0].s)); 
	}
#line 1429 "engines/hypno/grammar_mis.cpp"
    break;

  case 21: /* line: WALNTOK FILENAME NUM NUM gsswitch flag  */
#line 202 "engines/hypno/grammar_mis.y"
                                                   { 
		WalN *a = new WalN((yyvsp[-5].s), (yyvsp[-4].s), Common::Point((yyvsp[-3].i), (yyvsp[-2].i)), (yyvsp[-1].s), (yyvsp[0].s));
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);		  
		debugC(1, kHypnoDebugParser, "WALN %s %d %d", (yyvsp[-4].s), (yyvsp[-3].i), (yyvsp[-2].i)); }
#line 1440 "engines/hypno/grammar_mis.cpp"
    break;

  case 22: /* line: MICETOK FILENAME NUM  */
#line 208 "engines/hypno/grammar_mis.y"
                                {
		Mice *a = new Mice((yyvsp[-1].s), (yyvsp[0].i)-1);
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(a);
	}
#line 1451 "engines/hypno/grammar_mis.cpp"
    break;

  case 23: /* line: MPTRTOK FILENAME NUM NUM NUM NUM NUM  */
#line 214 "engines/hypno/grammar_mis.y"
                                                {
		debugC(1, kHypnoDebugParser, "MPTR %s %d %d %d %d %d", (yyvsp[-5].s), (yyvsp[-4].i), (yyvsp[-3].i), (yyvsp[-2].i), (yyvsp[-1].i), (yyvsp[0].i));
	}
#line 1459 "engines/hypno/grammar_mis.cpp"
    break;

  case 24: /* line: TALKTOK alloctalk talk  */
#line 217 "engines/hypno/grammar_mis.y"
                                  { 
		Hotspots *cur = stack->back();
		Hotspot *hot = &cur->back();
		hot->actions.push_back(talk_action);
		talk_action = nullptr;
		debugC(1, kHypnoDebugParser, "TALK"); }
#line 1470 "engines/hypno/grammar_mis.cpp"
    break;

  case 25: /* line: ENDTOK anything RETTOK  */
#line 223 "engines/hypno/grammar_mis.y"
                                  { 
		debugC(1, kHypnoDebugParser, "explicit END");
		g_parsedHots = stack->back();
		stack->pop_back();
		smenu_idx->pop_back();
	}
#line 1481 "engines/hypno/grammar_mis.cpp"
    break;

  case 26: /* line: RETTOK  */
#line 229 "engines/hypno/grammar_mis.y"
                       { debugC(1, kHypnoDebugParser, "implicit END"); }
#line 1487 "engines/hypno/grammar_mis.cpp"
    break;

  case 29: /* alloctalk: %empty  */
#line 236 "engines/hypno/grammar_mis.y"
           { 
	assert(talk_action == nullptr);
	talk_action = new Talk();
	talk_action->escape = false;
	talk_action->active = true; 
}
#line 1498 "engines/hypno/grammar_mis.cpp"
    break;

  case 30: /* talk: INACTOK talk  */
#line 243 "engines/hypno/grammar_mis.y"
                   {
		talk_action->active = false; 
		debugC(1, kHypnoDebugParser, "inactive"); }
#line 1506 "engines/hypno/grammar_mis.cpp"
    break;

  case 31: /* talk: FDTOK talk  */
#line 246 "engines/hypno/grammar_mis.y"
                     { debugC(1, kHypnoDebugParser, "inactive"); }
#line 1512 "engines/hypno/grammar_mis.cpp"
    break;

  case 32: /* talk: BACKTOK FILENAME NUM NUM gsswitch flag  */
#line 247 "engines/hypno/grammar_mis.y"
                                                 { 
		talk_action->background = (yyvsp[-4].s);
		talk_action->backgroundPos = Common::Point((yyvsp[-3].i), (yyvsp[-2].i));
		debugC(1, kHypnoDebugParser, "BACK in TALK"); }
#line 1521 "engines/hypno/grammar_mis.cpp"
    break;

  case 33: /* talk: BOXXTOK NUM NUM  */
#line 251 "engines/hypno/grammar_mis.y"
                          {
		talk_action->boxPos = Common::Point((yyvsp[-1].i), (yyvsp[0].i)); 
		debugC(1, kHypnoDebugParser, "BOXX %d %d", (yyvsp[-1].i), (yyvsp[0].i)); }
#line 1529 "engines/hypno/grammar_mis.cpp"
    break;

  case 34: /* talk: ESCAPETOK  */
#line 254 "engines/hypno/grammar_mis.y"
                    {
		talk_action->escape = true; 
		debugC(1, kHypnoDebugParser, "ESCAPE"); }
#line 1537 "engines/hypno/grammar_mis.cpp"
    break;

  case 35: /* talk: SECONDTOK FILENAME NUM NUM flag  */
#line 257 "engines/hypno/grammar_mis.y"
                                          {
		talk_action->second = (yyvsp[-3].s);
		talk_action->secondPos = Common::Point((yyvsp[-2].i), (yyvsp[-1].i)); 
		debugC(1, kHypnoDebugParser, "SECOND %s %d %d '%s'", (yyvsp[-3].s), (yyvsp[-2].i), (yyvsp[-1].i), (yyvsp[0].s)); }
#line 1546 "engines/hypno/grammar_mis.cpp"
    break;

  case 36: /* talk: INTROTOK FILENAME NUM NUM  */
#line 261 "engines/hypno/grammar_mis.y"
                                    { 
		talk_action->intro = (yyvsp[-2].s);
		talk_action->introPos = Common::Point((yyvsp[-1].i), (yyvsp[0].i));
		debugC(1, kHypnoDebugParser, "INTRO %s %d %d", (yyvsp[-2].s), (yyvsp[-1].i), (yyvsp[0].i)); }
#line 1555 "engines/hypno/grammar_mis.cpp"
    break;

  case 37: /* talk: DEFAULTTOK FILENAME NUM NUM  */
#line 265 "engines/hypno/grammar_mis.y"
                                      { 
		// Unsure how this is different from second
		talk_action->second = (yyvsp[-2].s);
		talk_action->secondPos = Common::Point((yyvsp[-1].i), (yyvsp[0].i));
		debugC(1, kHypnoDebugParser, "DEFAULT %s %d %d", (yyvsp[-2].s), (yyvsp[-1].i), (yyvsp[0].i)); }
#line 1565 "engines/hypno/grammar_mis.cpp"
    break;

  case 38: /* talk: PG talk  */
#line 270 "engines/hypno/grammar_mis.y"
                  { 
		TalkCommand talk_cmd;
		talk_cmd.command = "G";
		talk_cmd.path = (yyvsp[-1].s)+2;
		talk_action->commands.push_back(talk_cmd); 
		debugC(1, kHypnoDebugParser, "%s", (yyvsp[-1].s)); }
#line 1576 "engines/hypno/grammar_mis.cpp"
    break;

  case 39: /* talk: PH talk  */
#line 276 "engines/hypno/grammar_mis.y"
                  { debugC(1, kHypnoDebugParser, "%s", (yyvsp[-1].s)); }
#line 1582 "engines/hypno/grammar_mis.cpp"
    break;

  case 40: /* talk: PF talk  */
#line 277 "engines/hypno/grammar_mis.y"
                  { 
		TalkCommand talk_cmd;
		talk_cmd.command = "F";
		talk_cmd.num = atoi((yyvsp[-1].s)+2)-1;
		talk_action->commands.push_back(talk_cmd); 
		debugC(1, kHypnoDebugParser, "%s", (yyvsp[-1].s)); }
#line 1593 "engines/hypno/grammar_mis.cpp"
    break;

  case 41: /* talk: PA talk  */
#line 283 "engines/hypno/grammar_mis.y"
                  { 
		TalkCommand talk_cmd;
		talk_cmd.command = "A";
		talk_cmd.num = atoi((yyvsp[-1].s)+2)-1;
		talk_action->commands.push_back(talk_cmd); 
		debugC(1, kHypnoDebugParser, "|A%d", talk_cmd.num); }
#line 1604 "engines/hypno/grammar_mis.cpp"
    break;

  case 42: /* talk: PD talk  */
#line 289 "engines/hypno/grammar_mis.y"
                  { 
		TalkCommand talk_cmd;
		talk_cmd.command = "D";
		talk_cmd.num = atoi((yyvsp[-1].s)+2)-1;
		talk_action->commands.push_back(talk_cmd); 
		debugC(1, kHypnoDebugParser, "%s", (yyvsp[-1].s)); }
#line 1615 "engines/hypno/grammar_mis.cpp"
    break;

  case 43: /* talk: PP NUM NUM flag talk  */
#line 295 "engines/hypno/grammar_mis.y"
                               { 
		TalkCommand talk_cmd;
		talk_cmd.command = "P";
		talk_cmd.path = (yyvsp[-4].s)+2;
		talk_cmd.position = Common::Point((yyvsp[-3].i), (yyvsp[-2].i));
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "%s %d %d '%s'", (yyvsp[-4].s), (yyvsp[-3].i), (yyvsp[-2].i), (yyvsp[-1].s)); }
#line 1627 "engines/hypno/grammar_mis.cpp"
    break;

  case 44: /* talk: PI NUM NUM talk  */
#line 302 "engines/hypno/grammar_mis.y"
                          { 
		TalkCommand talk_cmd;
		talk_cmd.command = "I";
		talk_cmd.path = (yyvsp[-3].s)+2;
		talk_cmd.position = Common::Point((yyvsp[-2].i), (yyvsp[-1].i));
		talk_action->commands.push_back(talk_cmd);		  
		debugC(1, kHypnoDebugParser, "%s %d %d", (yyvsp[-3].s), (yyvsp[-2].i), (yyvsp[-1].i)); }
#line 1639 "engines/hypno/grammar_mis.cpp"
    break;

  case 45: /* talk: PS talk  */
#line 309 "engines/hypno/grammar_mis.y"
                  { 
		TalkCommand talk_cmd;
		talk_cmd.command = "S";
		talk_cmd.variable = (yyvsp[-1].s)+2;
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "%s", (yyvsp[-1].s)); }
#line 1650 "engines/hypno/grammar_mis.cpp"
    break;

  case 46: /* talk: PL talk  */
#line 315 "engines/hypno/grammar_mis.y"
                  { 
		TalkCommand talk_cmd;
		talk_cmd.command = "L";
		talk_action->commands.push_back(talk_cmd);
		debugC(1, kHypnoDebugParser, "|L"); }
#line 1660 "engines/hypno/grammar_mis.cpp"
    break;

  case 47: /* talk: PE  */
#line 320 "engines/hypno/grammar_mis.y"
             { debugC(1, kHypnoDebugParser, "|E"); }
#line 1666 "engines/hypno/grammar_mis.cpp"
    break;

  case 49: /* mflag: NAME  */
#line 324 "engines/hypno/grammar_mis.y"
            { (yyval.s) = (yyvsp[0].s); }
#line 1672 "engines/hypno/grammar_mis.cpp"
    break;

  case 50: /* mflag: %empty  */
#line 325 "engines/hypno/grammar_mis.y"
                        { (yyval.s) = scumm_strdup(""); }
#line 1678 "engines/hypno/grammar_mis.cpp"
    break;

  case 51: /* flag: FLAG  */
#line 328 "engines/hypno/grammar_mis.y"
                        { (yyval.s) = (yyvsp[0].s); debugC(1, kHypnoDebugParser, "flag: %s", (yyvsp[0].s)); }
#line 1684 "engines/hypno/grammar_mis.cpp"
    break;

  case 52: /* flag: %empty  */
#line 329 "engines/hypno/grammar_mis.y"
                        { (yyval.s) = scumm_strdup(""); }
#line 1690 "engines/hypno/grammar_mis.cpp"
    break;

  case 53: /* gsswitch: GSSWITCH  */
#line 332 "engines/hypno/grammar_mis.y"
                        { (yyval.s) = (yyvsp[0].s); debugC(1, kHypnoDebugParser, "switch %s", (yyvsp[0].s)); }
#line 1696 "engines/hypno/grammar_mis.cpp"
    break;

  case 54: /* gsswitch: %empty  */
#line 333 "engines/hypno/grammar_mis.y"
                        { (yyval.s) = scumm_strdup(""); }
#line 1702 "engines/hypno/grammar_mis.cpp"
    break;


#line 1706 "engines/hypno/grammar_mis.cpp"

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
  yytoken = yychar == HYPNO_MIS_EMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
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

      if (yychar <= HYPNO_MIS_EOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == HYPNO_MIS_EOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = HYPNO_MIS_EMPTY;
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
  if (yychar != HYPNO_MIS_EMPTY)
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

