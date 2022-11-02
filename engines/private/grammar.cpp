/* A Bison parser, made by GNU Bison 3.7.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.7.2"

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
#line 51 "engines/private/grammar.y"


#include "private/private.h"
#include "private/grammar.h"

#undef yyerror
#define yyerror         PRIVATE_xerror

#define code1(c1)       code(c1);
#define code2(c1,c2)    code(c1); code(c2)
#define code3(c1,c2,c3) code(c1); code(c2); code(c3)

using namespace Private;
using namespace Gen;
using namespace Settings;

extern int PRIVATE_lex();
//extern int PRIVATE_parse();

void PRIVATE_xerror(const char *str) {
    assert(0);
}

int PRIVATE_wrap() {
    return 1;
}



#line 108 "engines/private/grammar.cpp"

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
  YYSYMBOL_LTE = 6,                        /* LTE  */
  YYSYMBOL_GTE = 7,                        /* GTE  */
  YYSYMBOL_NEQ = 8,                        /* NEQ  */
  YYSYMBOL_EQ = 9,                         /* EQ  */
  YYSYMBOL_FALSETOK = 10,                  /* FALSETOK  */
  YYSYMBOL_TRUETOK = 11,                   /* TRUETOK  */
  YYSYMBOL_NULLTOK = 12,                   /* NULLTOK  */
  YYSYMBOL_IFTOK = 13,                     /* IFTOK  */
  YYSYMBOL_ELSETOK = 14,                   /* ELSETOK  */
  YYSYMBOL_RECT = 15,                      /* RECT  */
  YYSYMBOL_GOTOTOK = 16,                   /* GOTOTOK  */
  YYSYMBOL_DEBUGTOK = 17,                  /* DEBUGTOK  */
  YYSYMBOL_DEFINETOK = 18,                 /* DEFINETOK  */
  YYSYMBOL_SETTINGTOK = 19,                /* SETTINGTOK  */
  YYSYMBOL_RANDOMTOK = 20,                 /* RANDOMTOK  */
  YYSYMBOL_21_ = 21,                       /* '{'  */
  YYSYMBOL_22_ = 22,                       /* '}'  */
  YYSYMBOL_23_ = 23,                       /* ','  */
  YYSYMBOL_24_ = 24,                       /* ';'  */
  YYSYMBOL_25_ = 25,                       /* '('  */
  YYSYMBOL_26_ = 26,                       /* ')'  */
  YYSYMBOL_27_ = 27,                       /* '!'  */
  YYSYMBOL_28_ = 28,                       /* '+'  */
  YYSYMBOL_29_ = 29,                       /* '<'  */
  YYSYMBOL_30_ = 30,                       /* '>'  */
  YYSYMBOL_31_ = 31,                       /* '%'  */
  YYSYMBOL_YYACCEPT = 32,                  /* $accept  */
  YYSYMBOL_lines = 33,                     /* lines  */
  YYSYMBOL_line = 34,                      /* line  */
  YYSYMBOL_debug = 35,                     /* debug  */
  YYSYMBOL_statements = 36,                /* statements  */
  YYSYMBOL_statement = 37,                 /* statement  */
  YYSYMBOL_body = 38,                      /* body  */
  YYSYMBOL_end = 39,                       /* end  */
  YYSYMBOL_if = 40,                        /* if  */
  YYSYMBOL_cond = 41,                      /* cond  */
  YYSYMBOL_define = 42,                    /* define  */
  YYSYMBOL_fcall = 43,                     /* fcall  */
  YYSYMBOL_startp = 44,                    /* startp  */
  YYSYMBOL_params = 45,                    /* params  */
  YYSYMBOL_value = 46,                     /* value  */
  YYSYMBOL_expr = 47                       /* expr  */
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
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
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
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   125

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  32
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  50
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  113

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   275


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
       2,     2,     2,    27,     2,     2,     2,    31,     2,     2,
      25,    26,     2,    28,    23,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    24,
      29,     2,    30,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    21,     2,    22,     2,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20
};

#if PRIVATE_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    96,    96,    97,   100,   101,   102,   106,   107,   110,
     111,   114,   121,   122,   127,   135,   136,   139,   142,   145,
     148,   149,   154,   158,   159,   162,   170,   171,   179,   182,
     183,   184,   185,   186,   189,   190,   191,   192,   193,   194,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207
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
  "NUM", "LTE", "GTE", "NEQ", "EQ", "FALSETOK", "TRUETOK", "NULLTOK",
  "IFTOK", "ELSETOK", "RECT", "GOTOTOK", "DEBUGTOK", "DEFINETOK",
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

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   123,   125,    44,    59,    40,    41,    33,    43,    60,
      62,    37
};
#endif

#define YYPACT_NINF (-73)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
       8,   -11,    11,    12,    18,     8,    30,    22,    23,   -73,
     -73,    24,    38,    42,    59,    30,   -73,    40,    39,    41,
     -73,    54,    16,    58,    59,    56,    60,   -73,     5,   -73,
     -73,    77,    61,    83,   -73,   -73,    19,    52,   -73,    62,
     -73,     1,    65,   -73,    63,   -73,   -73,   -73,   -73,   -73,
     -73,    67,    66,    29,    64,    59,   -73,   -73,    86,    41,
      68,    71,    69,    73,    92,   -73,    93,   -73,    66,    66,
      66,    66,    66,    66,    66,   -73,    78,    50,    76,     1,
     -73,     1,    79,    70,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,    52,    98,   -73,   -73,    99,    80,   -73,    82,
      84,   -73,   -73,   103,   104,    87,    85,   107,   -73,    88,
      90,    42,   -73
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     3,     7,     0,     0,     1,
       2,     0,     0,    20,     9,     7,     4,    24,     0,     0,
      18,     0,     0,     0,     9,     0,     0,     8,    20,     5,
      28,     0,     0,     0,     6,    10,     0,     0,    12,     0,
      23,    29,     0,    11,     0,    39,    38,    37,    35,    36,
      34,     0,     0,    40,     0,     9,    15,    17,     0,    39,
       0,    33,     0,    32,     0,    25,     0,    41,     0,     0,
       0,     0,    49,     0,     0,    19,     0,    13,     0,    29,
      27,    29,     0,     0,    47,    48,    43,    42,    44,    45,
      46,    16,     0,     0,    30,    31,     0,     0,    17,     0,
       0,    50,    14,     0,     0,     0,     0,     0,    26,     0,
      22,    20,    21
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -73,   110,   -73,   101,   -21,   -36,    25,    20,   -73,   -73,
     -28,   -39,   -73,   -72,   -20,    89
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     4,     5,    12,    23,    24,    57,    77,    25,    37,
      18,    26,    41,    62,    53,    63
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      40,    56,    61,    35,    59,    46,    47,    94,    17,    95,
       6,    48,    49,    50,     7,     8,    21,    60,     9,    32,
      39,    51,    45,    46,    47,     1,     2,     3,    52,    48,
      49,    50,    67,    11,    76,    68,    69,    70,    71,    51,
      61,    33,    61,    13,    14,    17,    52,    15,    84,    85,
      86,    87,    88,    89,    90,    19,    56,    72,    73,    74,
      16,    29,    19,    28,    92,    20,    30,    21,    22,    45,
      46,    47,    20,    55,    21,    22,    48,    49,    50,    31,
      34,    36,    42,   112,    38,    43,    44,    58,    64,    65,
      75,    78,    66,    33,    79,    80,    81,    82,    83,    93,
      91,    97,    96,    99,   100,   103,   101,   104,   105,   106,
     107,   108,   109,   111,   110,    10,    27,    98,   102,     0,
       0,     0,     0,     0,     0,    54
};

static const yytype_int8 yycheck[] =
{
      28,    37,    41,    24,     3,     4,     5,    79,     3,    81,
      21,    10,    11,    12,     3,     3,    15,    16,     0,     3,
      15,    20,     3,     4,     5,    17,    18,    19,    27,    10,
      11,    12,    52,     3,    55,     6,     7,     8,     9,    20,
      79,    25,    81,    21,    21,     3,    27,    23,    68,    69,
      70,    71,    72,    73,    74,     3,    92,    28,    29,    30,
      22,    22,     3,    23,    14,    13,    25,    15,    16,     3,
       4,     5,    13,    21,    15,    16,    10,    11,    12,    25,
      22,    25,     5,   111,    24,    24,     3,    25,    23,    26,
      26,     5,    25,    25,    23,    26,    23,     5,     5,    23,
      22,    31,    23,     5,     5,    23,    26,    23,     5,     5,
      23,    26,     5,    23,    26,     5,    15,    92,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    36
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    17,    18,    19,    33,    34,    21,     3,     3,     0,
      33,     3,    35,    21,    21,    23,    22,     3,    42,     3,
      13,    15,    16,    36,    37,    40,    43,    35,    23,    22,
      25,    25,     3,    25,    22,    36,    25,    41,    24,    15,
      42,    44,     5,    24,     3,     3,     4,     5,    10,    11,
      12,    20,    27,    46,    47,    21,    37,    38,    25,     3,
      16,    43,    45,    47,    23,    26,    25,    46,     6,     7,
       8,     9,    28,    29,    30,    26,    36,    39,     5,    23,
      26,    23,     5,     5,    46,    46,    46,    46,    46,    46,
      46,    22,    14,    23,    45,    45,    23,    31,    38,     5,
       5,    26,    39,    23,    23,     5,     5,    23,    26,     5,
      26,    23,    42
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    32,    33,    33,    34,    34,    34,    35,    35,    36,
      36,    37,    37,    37,    37,    38,    38,    39,    40,    41,
      42,    42,    42,    42,    42,    43,    43,    43,    44,    45,
      45,    45,    45,    45,    46,    46,    46,    46,    46,    46,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     4,     5,     5,     0,     3,     0,
       2,     3,     2,     4,     7,     1,     3,     0,     1,     3,
       0,    14,    12,     3,     1,     4,    10,     5,     0,     0,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     3,     3,     3,     3,     3,     3,     3,     2,
       5
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = PRIVATE_EMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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
#line 100 "engines/private/grammar.y"
                                             { /* Not used in the game */ }
#line 1207 "engines/private/grammar.cpp"
    break;

  case 5: /* line: DEFINETOK NAME '{' define '}'  */
#line 101 "engines/private/grammar.y"
                                             { g_private->maps.installAll((yyvsp[-3].s)); }
#line 1213 "engines/private/grammar.cpp"
    break;

  case 6: /* line: SETTINGTOK NAME '{' statements '}'  */
#line 102 "engines/private/grammar.y"
                                             { g_setts->save((yyvsp[-3].s));
                                               g_setts->init(); }
#line 1220 "engines/private/grammar.cpp"
    break;

  case 9: /* statements: %empty  */
#line 110 "engines/private/grammar.y"
                               { (yyval.inst) = g_vm->_progp; }
#line 1226 "engines/private/grammar.cpp"
    break;

  case 11: /* statement: GOTOTOK NAME ';'  */
#line 114 "engines/private/grammar.y"
                            {
        (yyval.inst) = g_vm->_progp;
        code2(strpush, (Inst) g_private->maps.constant(STRING, 0, (yyvsp[-1].s)));
        code2(constpush, (Inst) g_private->maps.constant(NUM, 1, nullptr));
        code2(strpush, (Inst) g_private->maps.constant(STRING, 0, "goto"));
        code1(funcpush);
        }
#line 1238 "engines/private/grammar.cpp"
    break;

  case 12: /* statement: fcall ';'  */
#line 121 "engines/private/grammar.y"
                            { (yyval.inst) = (yyvsp[-1].inst); }
#line 1244 "engines/private/grammar.cpp"
    break;

  case 13: /* statement: if cond body end  */
#line 122 "engines/private/grammar.y"
                           {
                /* else-less if */
                ((yyvsp[-3].inst))[1] = (Inst)(yyvsp[-1].inst);     /* thenpart */
                ((yyvsp[-3].inst))[3] = (Inst)(yyvsp[0].inst);
                }
#line 1254 "engines/private/grammar.cpp"
    break;

  case 14: /* statement: if cond body end ELSETOK body end  */
#line 127 "engines/private/grammar.y"
                                            {
                /* if with else */
                ((yyvsp[-6].inst))[1] = (Inst)(yyvsp[-4].inst);     /* thenpart */
                ((yyvsp[-6].inst))[2] = (Inst)(yyvsp[-1].inst);     /* elsepart */
                ((yyvsp[-6].inst))[3] = (Inst)(yyvsp[0].inst);
                }
#line 1265 "engines/private/grammar.cpp"
    break;

  case 15: /* body: statement  */
#line 135 "engines/private/grammar.y"
                             { (yyval.inst) = (yyvsp[0].inst); }
#line 1271 "engines/private/grammar.cpp"
    break;

  case 16: /* body: '{' statements '}'  */
#line 136 "engines/private/grammar.y"
                             { (yyval.inst) = (yyvsp[-1].inst); }
#line 1277 "engines/private/grammar.cpp"
    break;

  case 17: /* end: %empty  */
#line 139 "engines/private/grammar.y"
                             { code1(STOP); (yyval.inst) = g_vm->_progp; }
#line 1283 "engines/private/grammar.cpp"
    break;

  case 18: /* if: IFTOK  */
#line 142 "engines/private/grammar.y"
          { (yyval.inst) = code1(ifcode); code3(STOP, STOP, STOP); }
#line 1289 "engines/private/grammar.cpp"
    break;

  case 19: /* cond: '(' expr ')'  */
#line 145 "engines/private/grammar.y"
                        { code1(STOP); (yyval.inst) = (yyvsp[-1].inst); }
#line 1295 "engines/private/grammar.cpp"
    break;

  case 21: /* define: NAME ',' RECT '(' NUM ',' NUM ',' NUM ',' NUM ')' ',' define  */
#line 149 "engines/private/grammar.y"
                                                                        {
          Common::Rect *r = new Common::Rect((yyvsp[-9].sym)->u.val, (yyvsp[-7].sym)->u.val, (yyvsp[-5].sym)->u.val, (yyvsp[-3].sym)->u.val);
          assert(r->isValidRect());
          g_private->maps.defineSymbol((yyvsp[-13].s), r);
          }
#line 1305 "engines/private/grammar.cpp"
    break;

  case 22: /* define: NAME ',' RECT '(' NUM ',' NUM ',' NUM ',' NUM ')'  */
#line 154 "engines/private/grammar.y"
                                                            {
          Common::Rect *r = new Common::Rect((yyvsp[-7].sym)->u.val, (yyvsp[-5].sym)->u.val, (yyvsp[-3].sym)->u.val, (yyvsp[-1].sym)->u.val);
          g_private->maps.defineSymbol((yyvsp[-11].s), r);
          }
#line 1314 "engines/private/grammar.cpp"
    break;

  case 23: /* define: NAME ',' define  */
#line 158 "engines/private/grammar.y"
                          { g_private->maps.defineSymbol((yyvsp[-2].s), nullptr); }
#line 1320 "engines/private/grammar.cpp"
    break;

  case 24: /* define: NAME  */
#line 159 "engines/private/grammar.y"
                          { g_private->maps.defineSymbol((yyvsp[0].s), nullptr); }
#line 1326 "engines/private/grammar.cpp"
    break;

  case 25: /* fcall: GOTOTOK '(' NAME ')'  */
#line 162 "engines/private/grammar.y"
                               {
                               (yyval.inst) = g_vm->_progp;
                               code2(strpush, (Inst) g_private->maps.constant(STRING, 0, (yyvsp[-1].s)));
                               code2(constpush, (Inst) g_private->maps.constant(NUM, 1, nullptr));
                               code2(strpush, (Inst) g_private->maps.constant(STRING, 0, "goto"));
                               code1(funcpush);
                               }
#line 1338 "engines/private/grammar.cpp"
    break;

  case 26: /* fcall: RECT '(' NUM ',' NUM ',' NUM ',' NUM ')'  */
#line 170 "engines/private/grammar.y"
                                                   { (yyval.inst) = g_vm->_progp; }
#line 1344 "engines/private/grammar.cpp"
    break;

  case 27: /* fcall: NAME '(' startp params ')'  */
#line 171 "engines/private/grammar.y"
                                      {
                               (yyval.inst) = (yyvsp[-2].inst);
                               code2(constpush, (Inst) g_private->maps.constant(NUM, (yyvsp[-1].narg), nullptr));
                               code2(strpush, (Inst) g_private->maps.constant(STRING, 0, (yyvsp[-4].s)));
                               code1(funcpush);
                               }
#line 1355 "engines/private/grammar.cpp"
    break;

  case 28: /* startp: %empty  */
#line 179 "engines/private/grammar.y"
                    { (yyval.inst) = g_vm->_progp; }
#line 1361 "engines/private/grammar.cpp"
    break;

  case 29: /* params: %empty  */
#line 182 "engines/private/grammar.y"
                            { (yyval.narg) = 0; }
#line 1367 "engines/private/grammar.cpp"
    break;

  case 30: /* params: fcall ',' params  */
#line 183 "engines/private/grammar.y"
                            { (yyval.narg) = (yyvsp[0].narg) + 1; }
#line 1373 "engines/private/grammar.cpp"
    break;

  case 31: /* params: expr ',' params  */
#line 184 "engines/private/grammar.y"
                            { (yyval.narg) = (yyvsp[0].narg) + 1; }
#line 1379 "engines/private/grammar.cpp"
    break;

  case 32: /* params: expr  */
#line 185 "engines/private/grammar.y"
                      { (yyval.narg) = 1; }
#line 1385 "engines/private/grammar.cpp"
    break;

  case 33: /* params: fcall  */
#line 186 "engines/private/grammar.y"
                      { (yyval.narg) = 1; }
#line 1391 "engines/private/grammar.cpp"
    break;

  case 34: /* value: NULLTOK  */
#line 189 "engines/private/grammar.y"
                   { code2(constpush, (Inst) g_private->maps.constant(NUM, 0, nullptr)); }
#line 1397 "engines/private/grammar.cpp"
    break;

  case 35: /* value: FALSETOK  */
#line 190 "engines/private/grammar.y"
                   { code2(constpush, (Inst) g_private->maps.constant(NUM, 0, nullptr)); }
#line 1403 "engines/private/grammar.cpp"
    break;

  case 36: /* value: TRUETOK  */
#line 191 "engines/private/grammar.y"
                   { code2(constpush, (Inst) g_private->maps.constant(NUM, 1, nullptr)); }
#line 1409 "engines/private/grammar.cpp"
    break;

  case 37: /* value: NUM  */
#line 192 "engines/private/grammar.y"
                   { code2(constpush, (Inst)(yyvsp[0].sym)); }
#line 1415 "engines/private/grammar.cpp"
    break;

  case 38: /* value: STRING  */
#line 193 "engines/private/grammar.y"
                   { code2(strpush, (Inst)(yyvsp[0].sym)); }
#line 1421 "engines/private/grammar.cpp"
    break;

  case 39: /* value: NAME  */
#line 194 "engines/private/grammar.y"
                   { code1(varpush); code1((Inst) g_private->maps.lookupName((yyvsp[0].s))); code1(eval); }
#line 1427 "engines/private/grammar.cpp"
    break;

  case 40: /* expr: value  */
#line 197 "engines/private/grammar.y"
                          { (yyval.inst) = (yyvsp[0].inst); }
#line 1433 "engines/private/grammar.cpp"
    break;

  case 41: /* expr: '!' value  */
#line 198 "engines/private/grammar.y"
                          { code1(negate); (yyval.inst) = (yyvsp[0].inst); }
#line 1439 "engines/private/grammar.cpp"
    break;

  case 42: /* expr: value EQ value  */
#line 199 "engines/private/grammar.y"
                          { code1(eq); }
#line 1445 "engines/private/grammar.cpp"
    break;

  case 43: /* expr: value NEQ value  */
#line 200 "engines/private/grammar.y"
                          { code1(ne); }
#line 1451 "engines/private/grammar.cpp"
    break;

  case 44: /* expr: value '+' value  */
#line 201 "engines/private/grammar.y"
                          { code1(add); }
#line 1457 "engines/private/grammar.cpp"
    break;

  case 45: /* expr: value '<' value  */
#line 202 "engines/private/grammar.y"
                          { code1(lt); }
#line 1463 "engines/private/grammar.cpp"
    break;

  case 46: /* expr: value '>' value  */
#line 203 "engines/private/grammar.y"
                          { code1(gt); }
#line 1469 "engines/private/grammar.cpp"
    break;

  case 47: /* expr: value LTE value  */
#line 204 "engines/private/grammar.y"
                          { code1(le); }
#line 1475 "engines/private/grammar.cpp"
    break;

  case 48: /* expr: value GTE value  */
#line 205 "engines/private/grammar.y"
                          { code1(ge); }
#line 1481 "engines/private/grammar.cpp"
    break;

  case 49: /* expr: value '+'  */
#line 206 "engines/private/grammar.y"
                          { (yyval.inst) = (yyvsp[-1].inst); }
#line 1487 "engines/private/grammar.cpp"
    break;

  case 50: /* expr: RANDOMTOK '(' NUM '%' ')'  */
#line 207 "engines/private/grammar.y"
                                    { code3(constpush, (Inst)(yyvsp[-2].sym), randbool); }
#line 1493 "engines/private/grammar.cpp"
    break;


#line 1497 "engines/private/grammar.cpp"

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


#if !defined yyoverflow
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

