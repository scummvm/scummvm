/* A Bison parser, made by GNU Bison 3.5.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 55 "engines/director/lingo/lingo-gr.y"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/endian.h"
#include "common/hash-str.h"
#include "common/rect.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-gr.h"

extern int yylex();
extern int yyparse();

using namespace Director;
void yyerror(const char *s) {
	g_lingo->_hadError = true;
	warning("######################  LINGO: %s at line %d col %d", s, g_lingo->_linenumber, g_lingo->_colnumber);
}

void checkEnd(Common::String *token, const char *expect, bool required) {
	if (required) {
		if (token->compareToIgnoreCase(expect)) {
			Common::String err = Common::String::format("end mismatch. Expected %s but got %s", expect, token->c_str());
			yyerror(err.c_str());
		}
	}
}


#line 101 "engines/director/lingo/lingo-gr.cpp"

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

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED
# define YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    UNARY = 258,
    CASTREF = 259,
    VOID = 260,
    VAR = 261,
    POINT = 262,
    RECT = 263,
    ARRAY = 264,
    OBJECT = 265,
    REFERENCE = 266,
    INT = 267,
    ARGC = 268,
    ARGCNORET = 269,
    THEENTITY = 270,
    THEENTITYWITHID = 271,
    FLOAT = 272,
    BLTIN = 273,
    FBLTIN = 274,
    RBLTIN = 275,
    ID = 276,
    STRING = 277,
    HANDLER = 278,
    SYMBOL = 279,
    ENDCLAUSE = 280,
    tPLAYACCEL = 281,
    tMETHOD = 282,
    THEOBJECTFIELD = 283,
    tDOWN = 284,
    tELSE = 285,
    tELSIF = 286,
    tEXIT = 287,
    tGLOBAL = 288,
    tGO = 289,
    tIF = 290,
    tINTO = 291,
    tLOOP = 292,
    tMACRO = 293,
    tMOVIE = 294,
    tNEXT = 295,
    tOF = 296,
    tPREVIOUS = 297,
    tPUT = 298,
    tREPEAT = 299,
    tSET = 300,
    tTHEN = 301,
    tTO = 302,
    tWHEN = 303,
    tWITH = 304,
    tWHILE = 305,
    tNLELSE = 306,
    tFACTORY = 307,
    tOPEN = 308,
    tPLAY = 309,
    tDONE = 310,
    tINSTANCE = 311,
    tGE = 312,
    tLE = 313,
    tEQ = 314,
    tNEQ = 315,
    tAND = 316,
    tOR = 317,
    tNOT = 318,
    tMOD = 319,
    tAFTER = 320,
    tBEFORE = 321,
    tCONCAT = 322,
    tCONTAINS = 323,
    tSTARTS = 324,
    tCHAR = 325,
    tITEM = 326,
    tLINE = 327,
    tWORD = 328,
    tSPRITE = 329,
    tINTERSECTS = 330,
    tWITHIN = 331,
    tTELL = 332,
    tPROPERTY = 333,
    tON = 334,
    tENDIF = 335,
    tENDREPEAT = 336,
    tENDTELL = 337
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 86 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Common::Array<double> *arr;

	struct {
		Common::String *s;
		int e;
	} objectfield;

#line 251 "engines/director/lingo/lingo-gr.cpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */



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

#if ! defined yyoverflow || YYERROR_VERBOSE

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
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


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
#define YYFINAL  122
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1507

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  98
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  161
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  341

#define YYUNDEFTOK  2
#define YYMAXUTOK   337


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      91,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    90,    85,     2,
      92,    93,    88,    86,    94,    87,     2,    89,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    97,     2,
      83,     2,    84,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    95,     2,    96,     2,     2,     2,     2,     2,     2,
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
      75,    76,    77,    78,    79,    80,    81,    82
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   135,   135,   136,   137,   140,   141,   142,   145,   151,
     154,   155,   156,   162,   169,   175,   182,   188,   194,   202,
     203,   204,   207,   208,   209,   210,   213,   214,   219,   230,
     247,   259,   264,   266,   270,   280,   292,   293,   296,   304,
     314,   327,   330,   337,   344,   352,   355,   358,   359,   360,
     363,   369,   373,   376,   379,   383,   387,   391,   394,   395,
     396,   399,   402,   410,   416,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   455,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   471,   472,   475,
     476,   479,   480,   483,   484,   495,   496,   497,   498,   502,
     506,   512,   513,   516,   517,   521,   525,   529,   529,   560,
     560,   567,   568,   568,   575,   585,   593,   595,   596,   597,
     598,   601,   602,   603,   606,   609,   618,   619,   620,   623,
     624,   627,   630,   631,   632,   633,   636,   637,   640,   641,
     644,   648
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNARY", "CASTREF", "VOID", "VAR",
  "POINT", "RECT", "ARRAY", "OBJECT", "REFERENCE", "INT", "ARGC",
  "ARGCNORET", "THEENTITY", "THEENTITYWITHID", "FLOAT", "BLTIN", "FBLTIN",
  "RBLTIN", "ID", "STRING", "HANDLER", "SYMBOL", "ENDCLAUSE", "tPLAYACCEL",
  "tMETHOD", "THEOBJECTFIELD", "tDOWN", "tELSE", "tELSIF", "tEXIT",
  "tGLOBAL", "tGO", "tIF", "tINTO", "tLOOP", "tMACRO", "tMOVIE", "tNEXT",
  "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN",
  "tWITH", "tWHILE", "tNLELSE", "tFACTORY", "tOPEN", "tPLAY", "tDONE",
  "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT", "tMOD",
  "tAFTER", "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR", "tITEM",
  "tLINE", "tWORD", "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL",
  "tPROPERTY", "tON", "tENDIF", "tENDREPEAT", "tENDTELL", "'<'", "'>'",
  "'&'", "'+'", "'-'", "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "','",
  "'['", "']'", "':'", "$accept", "program", "programline", "asgn",
  "stmtoneliner", "stmtonelinerwithif", "stmt", "ifstmt", "elseifstmtlist",
  "elseifstmt", "ifoneliner", "repeatwhile", "repeatwith", "if", "elseif",
  "begin", "end", "stmtlist", "when", "tell", "simpleexpr", "expr",
  "reference", "proc", "globallist", "propertylist", "instancelist",
  "gotofunc", "gotomovie", "playfunc", "$@1", "defn", "$@2", "$@3", "on",
  "argdef", "endargdef", "argstore", "macro", "arglist", "nonemptyarglist",
  "list", "valuelist", "linearlist", "proplist", "proppair", YY_NULLPTR
};
#endif

# ifdef YYPRINT
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
     335,   336,   337,    60,    62,    38,    43,    45,    42,    47,
      37,    10,    40,    41,    44,    91,    93,    58
};
# endif

#define YYPACT_NINF (-272)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-6)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     279,   -82,  -272,  -272,   882,  -272,   882,   882,    65,   916,
    -272,  -272,  -272,  -272,  -272,    22,    23,   765,  -272,    67,
     882,   -35,    84,    71,    72,   882,   801,    75,   882,   882,
     882,   882,   882,   882,  -272,    80,    81,   882,   882,   882,
      57,     6,  -272,  -272,  -272,  -272,  -272,   882,    44,   882,
     603,   882,  -272,  1405,  -272,  -272,  -272,  -272,  -272,  -272,
    -272,  -272,  -272,    12,   882,  1405,  1405,    13,    13,  -272,
    -272,   882,  1405,    15,   882,  -272,  -272,  -272,    16,  -272,
     882,  -272,    74,  -272,   968,  -272,  -272,   955,    85,  -272,
     -17,    65,    -9,    64,    68,  -272,  1322,  -272,   968,  -272,
    -272,    28,  -272,  1017,  1050,  1083,  1116,  1372,  -272,    30,
    -272,  -272,  -272,  1335,    32,    50,  -272,  -272,    55,    34,
      61,  -272,  -272,   360,  1405,   882,  1405,  -272,  -272,   882,
    1405,  -272,  -272,  1281,   882,   882,   882,   882,   882,   882,
     882,   882,   882,   882,   882,   882,   882,   882,   882,   882,
     882,   128,   882,   955,   882,  1335,   -45,   882,    13,   128,
     135,  1405,   882,  -272,  -272,    38,   882,   882,  -272,   882,
     882,    -6,   882,   882,   882,  -272,   882,  -272,   136,   882,
     882,   882,   882,   882,   882,   882,   882,   882,   882,   138,
    -272,    65,    65,  -272,    65,    49,  -272,  -272,  1405,   115,
    -272,  1405,   882,  -272,    31,    31,    31,    31,  1418,  1418,
    -272,   -25,    31,    31,    31,    31,   -25,   -33,   -33,  -272,
    -272,  -272,   -71,  1405,  -272,  1405,   -26,  -272,  1405,   128,
    -272,  -272,  1405,  1405,  1405,    31,   882,   882,  1405,    31,
    1405,  1405,  -272,  1405,  1149,  1405,  1182,  1405,  1215,  1405,
    1248,  1405,  1405,  -272,  -272,  -272,  -272,    32,    50,  -272,
     522,   -10,  -272,   118,  1405,   522,    73,   150,    73,   -19,
    1405,    31,   882,   882,   882,   882,  -272,  -272,    92,   139,
     882,   522,   684,   105,   167,  -272,  -272,  -272,    73,  1405,
    1405,  1405,  1405,  -272,   882,  1405,  -272,  -272,  -272,  -272,
     441,   522,  -272,  1405,  -272,    53,   -23,   168,   522,  -272,
     522,  -272,  -272,  -272,   882,   113,  -272,  -272,  -272,   101,
     522,   119,  -272,  1405,  -272,   684,   175,   120,  -272,   522,
     156,  -272,  -272,  -272,   123,  -272,   124,  -272,   522,  -272,
    -272
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    52,    62,     0,    53,   146,   146,     0,    56,
      55,    54,   127,   132,    64,   102,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
     152,     0,     3,    65,    26,     7,    27,     0,     0,     0,
       0,     0,    58,    20,    59,    21,    99,   100,     6,    45,
      19,    57,     4,    56,     0,    63,   147,   106,    60,    56,
      97,   146,   149,   145,   146,    45,   101,   109,   103,   115,
       0,   116,     0,   117,   118,   120,   129,    98,     0,    41,
       0,     0,     0,     0,     0,   131,   108,   123,   124,   126,
     113,   105,    79,     0,     0,     0,     0,     0,   111,   104,
     136,    84,    85,     0,    55,    54,   153,   156,     0,   154,
     155,   158,     1,     5,    46,     0,    46,    46,    25,     0,
      23,    24,    22,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   137,   146,     0,     0,   147,     0,     0,   128,   137,
       0,   122,     0,   119,    45,     0,     0,     0,    42,     0,
       0,     0,     0,     0,     0,    50,     0,   125,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      86,     0,     0,   151,     0,     0,     2,    47,    46,     0,
      31,    46,     0,    47,    75,    76,    73,    74,    77,    78,
      70,    81,    82,    83,    72,    71,    80,    66,    67,    68,
      69,   138,     0,   148,    61,   150,     0,   110,   121,   137,
       8,     9,    10,    11,    15,    13,     0,     0,    14,    12,
      18,   107,   114,    89,     0,    91,     0,    93,     0,    95,
       0,    87,    88,   112,   161,   160,   157,     0,     0,   159,
      46,     0,    47,     0,    33,    46,   144,     0,   144,     0,
      16,    17,     0,     0,     0,     0,    48,    49,     0,     0,
       0,    46,     0,     0,     0,    47,   139,    47,   144,    90,
      92,    94,    96,    28,     0,    46,    36,    46,    32,   140,
     135,   133,    47,    46,    47,    46,     0,   141,   130,    47,
      46,    45,    44,    37,     0,     0,    45,    40,   142,   134,
      46,     0,    47,    46,    34,     0,     0,     0,    29,    46,
       0,    46,   143,    30,     0,    47,     0,    35,    46,    39,
      38
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -272,  -272,    82,  -272,  -271,  -272,     4,  -272,  -272,  -272,
    -272,  -272,  -272,   157,  -272,   -74,   -75,   -88,  -272,  -272,
       0,    -4,    43,   159,  -272,  -272,  -272,  -272,    10,  -272,
    -272,  -272,  -272,  -272,  -272,  -149,  -272,  -256,   160,    11,
    -272,  -272,  -272,  -272,  -272,    17
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    41,    42,    43,    44,   127,   277,    46,   305,   313,
     128,    47,    48,    49,   314,   151,   197,   260,    50,    51,
      52,    53,    54,    55,    78,   109,   101,    56,    85,    57,
      74,    58,   164,    75,    59,   222,   319,   285,    60,   156,
      73,    61,   118,   119,   120,   121
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      65,   159,    66,    66,    45,    72,   122,   316,    70,    62,
     226,   297,   287,    84,    88,    89,    87,    67,    68,   279,
     266,    96,    98,   267,   102,   103,   104,   105,   106,   107,
     169,   140,   302,   111,   112,   113,    99,   280,   172,   140,
     117,   236,   170,   124,    77,   126,   130,   133,   224,   154,
     173,   199,   200,   237,   331,   149,   150,   317,     8,   230,
     153,   147,   148,   149,   150,   268,    76,   155,   267,     2,
      66,   257,   288,   258,     5,   267,   161,     2,    69,   114,
     269,   115,     5,   311,   312,   158,    69,    10,    86,    11,
     229,   171,    94,    95,   163,   140,   100,   123,   141,    90,
      91,   108,   110,   125,   152,    92,   168,   154,   177,   157,
     160,   174,    93,   162,   175,   265,   146,   147,   148,   149,
     150,   198,   178,   261,   189,   201,   263,    45,   194,   191,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   192,    66,   221,
     223,   193,    40,   225,   116,   195,   227,   242,   228,   253,
      40,   262,   232,   233,   282,   234,   235,   284,   238,   239,
     240,   286,   241,   293,   281,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   278,   294,   298,   299,   318,
     283,   254,   255,   324,   256,   326,   332,   300,   264,   301,
     328,   333,   335,   337,   339,   196,   296,   129,   231,   131,
     132,     0,   259,     0,   308,     0,   310,     0,     0,     0,
     304,   320,   306,     0,     0,     0,     0,     0,   309,     0,
     315,     0,   270,   271,   329,   321,     0,   322,     0,     0,
       0,     0,   325,     0,     0,   327,     0,   338,   330,     0,
       0,     0,     0,     0,   334,     0,   336,     0,     0,     0,
       0,     0,     0,   340,     0,     0,     0,     0,   289,   290,
     291,   292,     0,     0,     0,     0,   295,     0,     0,    -5,
       1,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     303,     2,     0,     0,     3,     4,     5,     6,     7,     8,
       9,    10,     0,    11,     0,    12,    13,    14,     0,     0,
     323,    15,    16,    17,    18,     0,     0,    19,     0,     0,
       0,     0,    20,    21,    22,     0,     0,    23,     0,     0,
       0,    24,    25,    26,     0,    27,     0,     0,     0,     0,
       0,     0,    28,     0,     0,     0,     0,     0,     0,    29,
      30,    31,    32,    33,     0,     0,    34,    35,    36,     0,
       0,     0,     0,     0,     0,    37,    38,     0,     0,     0,
      -5,    39,     2,     0,    40,     3,     4,     5,     6,     7,
       8,     9,    10,     0,    11,     0,    12,    13,    14,     0,
       0,     0,    15,    16,    17,    18,     0,     0,    19,     0,
       0,     0,     0,    20,    21,    22,     0,     0,    23,     0,
       0,     0,    24,    25,    26,     0,    27,     0,     0,     0,
       0,     0,     0,    28,     0,     0,     0,     0,     0,     0,
      29,    30,    31,    32,    33,     0,     0,    34,    35,    36,
       0,     0,     0,     0,     0,     0,    37,    38,     0,     0,
       0,     0,    39,     2,     0,    40,     3,     4,     5,     6,
       7,     8,     9,    10,     0,    11,   307,    12,     0,    14,
       0,     0,     0,    15,    16,    17,    18,     0,     0,     0,
       0,     0,     0,     0,    20,    21,    22,     0,     0,    23,
       0,     0,     0,     0,    25,    26,     0,    27,     0,     0,
       0,     0,     0,     0,    28,     0,     0,     0,     0,     0,
       0,    29,    30,    31,    32,    33,     0,     0,    34,    35,
       0,     0,     0,     0,     0,     0,     0,    37,    38,     0,
       0,     0,   276,    39,     2,     0,    40,     3,     4,     5,
       6,     7,     8,     9,    10,     0,    11,     0,    12,     0,
      14,     0,     0,     0,    15,    16,    17,    18,     0,     0,
       0,     0,     0,     0,     0,    20,    21,    22,     0,     0,
      23,     0,     0,     0,     0,    25,    26,     0,    27,     0,
       0,     0,     0,     0,     0,    28,     0,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,     0,    34,
      35,     0,     0,     0,     0,     0,     0,     0,    37,    38,
       0,     0,     0,   276,    39,     2,     0,    40,     3,     4,
       5,     6,     7,     8,     9,    10,     0,    11,     0,    12,
       0,    14,     0,     0,     0,    15,    16,    17,    18,     0,
       0,     0,     0,     0,     0,     0,    20,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    25,    26,     0,    27,
       0,     0,     0,     0,     0,     0,    28,     0,     0,     0,
       0,     0,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    35,     0,     0,     0,     0,     0,     0,     0,    37,
      38,     0,     0,     0,     0,    39,     2,     0,    40,     3,
       4,     5,     6,     7,     8,     9,    10,     0,    11,     0,
      12,     0,    14,     0,     0,     0,    15,    16,    17,     0,
       0,     0,     0,     0,     0,     0,     0,    20,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    25,    26,     0,
      27,     0,     0,     0,     0,     0,     0,    28,     0,     0,
       0,     0,     0,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    35,     0,     0,     0,     0,     0,     0,     0,
      37,    38,     0,     0,     0,     0,    39,     2,     0,    40,
       3,     4,     5,     0,     7,     8,    63,    10,     0,    11,
       0,     0,     0,    14,     0,     0,     0,     0,     0,     0,
       0,     0,    79,     0,    80,    81,    82,    83,    64,     0,
      22,     0,     0,     2,     0,     0,     3,     4,     5,     0,
       7,     8,    63,    10,     0,    11,     0,     0,    28,    14,
       0,     0,     0,     0,     0,    29,    30,    31,    32,    33,
      80,     0,    82,     0,    64,     0,    22,     0,     0,     0,
       0,    37,    38,     0,     0,     0,    97,    39,     0,     0,
      40,     0,     0,     0,    28,     0,     0,     0,     0,     0,
       0,    29,    30,    31,    32,    33,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,    38,     0,
       0,     0,     0,    39,     2,     0,    40,     3,     4,     5,
       0,     7,     8,    63,    10,     0,    11,     0,     0,     0,
      14,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    64,     0,    22,     2,     0,
       0,     3,     4,     5,     0,     7,     8,    63,    10,     0,
      11,     0,     0,     0,    14,    28,     0,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,     0,    64,
       0,    22,     0,     0,     0,     0,     0,     0,    37,    38,
       0,     0,     0,     0,    39,     0,     0,    40,     0,    28,
       0,     0,     0,     0,     0,     0,    29,    30,    31,    32,
      33,   165,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    37,    38,     0,     0,     0,    80,    71,    82,
       0,    40,   134,   135,   136,   137,   138,   139,     0,   140,
     166,   167,   141,   142,   143,   134,   135,   136,   137,   138,
     139,     0,   140,     0,     0,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,     0,     0,     0,     0,     0,
       0,   144,   145,   146,   147,   148,   149,   150,   179,     0,
       0,     0,     0,     0,   180,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   134,   135,   136,   137,   138,   139,
       0,   140,     0,     0,   141,   142,   143,     0,     0,     0,
       0,   181,     0,     0,     0,     0,     0,   182,     0,     0,
     144,   145,   146,   147,   148,   149,   150,   134,   135,   136,
     137,   138,   139,     0,   140,     0,     0,   141,   142,   143,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
     184,     0,     0,   144,   145,   146,   147,   148,   149,   150,
     134,   135,   136,   137,   138,   139,     0,   140,     0,     0,
     141,   142,   143,     0,     0,     0,     0,   185,     0,     0,
       0,     0,     0,   186,     0,     0,   144,   145,   146,   147,
     148,   149,   150,   134,   135,   136,   137,   138,   139,     0,
     140,     0,     0,   141,   142,   143,     0,     0,     0,     0,
     272,     0,     0,     0,     0,     0,     0,     0,     0,   144,
     145,   146,   147,   148,   149,   150,   134,   135,   136,   137,
     138,   139,     0,   140,     0,     0,   141,   142,   143,     0,
       0,     0,     0,   273,     0,     0,     0,     0,     0,     0,
       0,     0,   144,   145,   146,   147,   148,   149,   150,   134,
     135,   136,   137,   138,   139,     0,   140,     0,     0,   141,
     142,   143,     0,     0,     0,     0,   274,     0,     0,     0,
       0,     0,     0,     0,     0,   144,   145,   146,   147,   148,
     149,   150,   134,   135,   136,   137,   138,   139,     0,   140,
       0,     0,   141,   142,   143,     0,     0,     0,     0,   275,
       0,     0,     0,     0,     0,     0,     0,     0,   144,   145,
     146,   147,   148,   149,   150,   134,   135,   136,   137,   138,
     139,     0,   140,     0,     0,   141,   142,   143,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   202,     0,
       0,   144,   145,   146,   147,   148,   149,   150,   134,   135,
     136,   137,   138,   139,     0,   140,     0,     0,   141,   142,
     143,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   144,   145,   146,   147,   148,   149,
     150,   176,   203,     0,     0,     0,     0,     0,     0,   134,
     135,   136,   137,   138,   139,     0,   140,     0,     0,   141,
     142,   143,   134,   135,   136,   137,   138,   139,     0,   140,
       0,     0,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,     0,     0,     0,     0,     0,     0,   144,   145,
     146,   147,   148,   149,   150,     0,     0,     0,   190,   134,
     135,   136,   137,   138,   139,     0,   140,     0,     0,   141,
     142,   143,     0,     0,     0,     0,     0,   187,   188,     0,
       0,     0,     0,     0,     0,   144,   145,   146,   147,   148,
     149,   150,   134,   135,   136,   137,   138,   139,     0,   140,
       0,     0,   141,   142,   143,   134,   135,   136,   137,     0,
       0,     0,   140,     0,     0,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,     0,     0,     0,     0,     0,
       0,   144,   145,   146,   147,   148,   149,   150
};

static const yytype_int16 yycheck[] =
{
       4,    75,     6,     7,     0,     9,     0,    30,     8,    91,
     159,   282,   268,    17,    49,    50,    20,     6,     7,    29,
      91,    25,    26,    94,    28,    29,    30,    31,    32,    33,
      47,    64,   288,    37,    38,    39,    26,    47,    47,    64,
      40,    47,    59,    47,    21,    49,    50,    51,    93,    94,
      59,   126,   127,    59,   325,    88,    89,    80,    20,    21,
      64,    86,    87,    88,    89,    91,    44,    71,    94,    12,
      74,    22,    91,    24,    17,    94,    80,    12,    21,    22,
     229,    24,    17,    30,    31,    74,    21,    22,    21,    24,
     164,    91,    21,    21,    84,    64,    21,    91,    67,    15,
      16,    21,    21,    59,    92,    21,    21,    94,    98,    94,
      94,    47,    28,    39,    46,   203,    85,    86,    87,    88,
      89,   125,    94,   198,    94,   129,   201,   123,    94,    97,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,    97,   152,    21,
     154,    96,    95,   157,    97,    94,    21,    21,   162,    21,
      95,    46,   166,   167,    46,   169,   170,    94,   172,   173,
     174,    21,   176,    81,   262,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   260,    47,    82,    21,    21,
     265,   191,   192,    80,   194,    94,    21,   285,   202,   287,
      81,    81,    46,    80,    80,   123,   281,    50,   165,    50,
      50,    -1,   195,    -1,   302,    -1,   304,    -1,    -1,    -1,
     295,   309,   297,    -1,    -1,    -1,    -1,    -1,   303,    -1,
     305,    -1,   236,   237,   322,   310,    -1,   311,    -1,    -1,
      -1,    -1,   316,    -1,    -1,   320,    -1,   335,   323,    -1,
      -1,    -1,    -1,    -1,   329,    -1,   331,    -1,    -1,    -1,
      -1,    -1,    -1,   338,    -1,    -1,    -1,    -1,   272,   273,
     274,   275,    -1,    -1,    -1,    -1,   280,    -1,    -1,     0,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     294,    12,    -1,    -1,    15,    16,    17,    18,    19,    20,
      21,    22,    -1,    24,    -1,    26,    27,    28,    -1,    -1,
     314,    32,    33,    34,    35,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    43,    44,    45,    -1,    -1,    48,    -1,    -1,
      -1,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    12,    -1,    95,    15,    16,    17,    18,    19,
      20,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      -1,    -1,    32,    33,    34,    35,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,    -1,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    -1,    -1,
      -1,    -1,    92,    12,    -1,    95,    15,    16,    17,    18,
      19,    20,    21,    22,    -1,    24,    25,    26,    -1,    28,
      -1,    -1,    -1,    32,    33,    34,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,
      -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    12,    -1,    95,    15,    16,    17,
      18,    19,    20,    21,    22,    -1,    24,    -1,    26,    -1,
      28,    -1,    -1,    -1,    32,    33,    34,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
      48,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    12,    -1,    95,    15,    16,
      17,    18,    19,    20,    21,    22,    -1,    24,    -1,    26,
      -1,    28,    -1,    -1,    -1,    32,    33,    34,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,    -1,    -1,    -1,    -1,    92,    12,    -1,    95,    15,
      16,    17,    18,    19,    20,    21,    22,    -1,    24,    -1,
      26,    -1,    28,    -1,    -1,    -1,    32,    33,    34,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    -1,    -1,    -1,    -1,    92,    12,    -1,    95,
      15,    16,    17,    -1,    19,    20,    21,    22,    -1,    24,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    -1,    39,    40,    41,    42,    43,    -1,
      45,    -1,    -1,    12,    -1,    -1,    15,    16,    17,    -1,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    63,    28,
      -1,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      39,    -1,    41,    -1,    43,    -1,    45,    -1,    -1,    -1,
      -1,    86,    87,    -1,    -1,    -1,    55,    92,    -1,    -1,
      95,    -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    -1,
      -1,    -1,    -1,    92,    12,    -1,    95,    15,    16,    17,
      -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    -1,    45,    12,    -1,
      -1,    15,    16,    17,    -1,    19,    20,    21,    22,    -1,
      24,    -1,    -1,    -1,    28,    63,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,    43,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      -1,    -1,    -1,    -1,    92,    -1,    -1,    95,    -1,    63,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    -1,    -1,    -1,    39,    92,    41,
      -1,    95,    57,    58,    59,    60,    61,    62,    -1,    64,
      65,    66,    67,    68,    69,    57,    58,    59,    60,    61,
      62,    -1,    64,    -1,    -1,    67,    68,    69,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89,    41,    -1,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    58,    59,    60,    61,    62,
      -1,    64,    -1,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    41,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      83,    84,    85,    86,    87,    88,    89,    57,    58,    59,
      60,    61,    62,    -1,    64,    -1,    -1,    67,    68,    69,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    83,    84,    85,    86,    87,    88,    89,
      57,    58,    59,    60,    61,    62,    -1,    64,    -1,    -1,
      67,    68,    69,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,    57,    58,    59,    60,    61,    62,    -1,
      64,    -1,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,
      84,    85,    86,    87,    88,    89,    57,    58,    59,    60,
      61,    62,    -1,    64,    -1,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    83,    84,    85,    86,    87,    88,    89,    57,
      58,    59,    60,    61,    62,    -1,    64,    -1,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,
      88,    89,    57,    58,    59,    60,    61,    62,    -1,    64,
      -1,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,    84,
      85,    86,    87,    88,    89,    57,    58,    59,    60,    61,
      62,    -1,    64,    -1,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89,    57,    58,
      59,    60,    61,    62,    -1,    64,    -1,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,    88,
      89,    49,    91,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      58,    59,    60,    61,    62,    -1,    64,    -1,    -1,    67,
      68,    69,    57,    58,    59,    60,    61,    62,    -1,    64,
      -1,    -1,    67,    68,    69,    83,    84,    85,    86,    87,
      88,    89,    -1,    -1,    -1,    -1,    -1,    -1,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    93,    57,
      58,    59,    60,    61,    62,    -1,    64,    -1,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    -1,    75,    76,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,
      88,    89,    57,    58,    59,    60,    61,    62,    -1,    64,
      -1,    -1,    67,    68,    69,    57,    58,    59,    60,    -1,
      -1,    -1,    64,    -1,    -1,    67,    68,    69,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    15,    16,    17,    18,    19,    20,    21,
      22,    24,    26,    27,    28,    32,    33,    34,    35,    38,
      43,    44,    45,    48,    52,    53,    54,    56,    63,    70,
      71,    72,    73,    74,    77,    78,    79,    86,    87,    92,
      95,    99,   100,   101,   102,   104,   105,   109,   110,   111,
     116,   117,   118,   119,   120,   121,   125,   127,   129,   132,
     136,   139,    91,    21,    43,   119,   119,   137,   137,    21,
     118,    92,   119,   138,   128,   131,    44,    21,   122,    37,
      39,    40,    41,    42,   119,   126,    21,   119,    49,    50,
      15,    16,    21,    28,    21,    21,   119,    55,   119,   126,
      21,   124,   119,   119,   119,   119,   119,   119,    21,   123,
      21,   119,   119,   119,    22,    24,    97,   118,   140,   141,
     142,   143,     0,    91,   119,    59,   119,   103,   108,   111,
     119,   121,   136,   119,    57,    58,    59,    60,    61,    62,
      64,    67,    68,    69,    83,    84,    85,    86,    87,    88,
      89,   113,    92,   119,    94,   119,   137,    94,   137,   113,
      94,   119,    39,   126,   130,    36,    65,    66,    21,    47,
      59,   118,    47,    59,    47,    46,    49,   126,    94,    41,
      47,    41,    47,    41,    47,    41,    47,    75,    76,    94,
      93,    97,    97,    96,    94,    94,   100,   114,   119,   114,
     114,   119,    47,    91,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,    21,   133,   119,    93,   119,   133,    21,   119,   113,
      21,   120,   119,   119,   119,   119,    47,    59,   119,   119,
     119,   119,    21,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,    21,   118,   118,   118,    22,    24,   143,
     115,   114,    46,   114,   119,   115,    91,    94,    91,   133,
     119,   119,    41,    41,    41,    41,    91,   104,   114,    29,
      47,   115,    46,   114,    94,   135,    21,   135,    91,   119,
     119,   119,   119,    81,    47,   119,   114,   102,    82,    21,
     115,   115,   135,   119,   114,   106,   114,    25,   115,   114,
     115,    30,    31,   107,   112,   114,    30,    80,    21,   134,
     115,   114,   113,   119,    80,   113,    94,   114,    81,   115,
     114,   102,    21,    81,   114,    46,   114,    80,   115,    80,
     114
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    98,    99,    99,    99,   100,   100,   100,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   102,
     102,   102,   103,   103,   103,   103,   104,   104,   104,   104,
     104,   104,   104,   104,   105,   105,   106,   106,   107,   108,
     108,   109,   110,   111,   112,   113,   114,   115,   115,   115,
     116,   117,   118,   118,   118,   118,   118,   118,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   120,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   122,
     122,   123,   123,   124,   124,   125,   125,   125,   125,   125,
     125,   126,   126,   127,   127,   127,   127,   128,   127,   130,
     129,   129,   131,   129,   129,   129,   132,   133,   133,   133,
     133,   134,   134,   134,   135,   136,   137,   137,   137,   138,
     138,   139,   140,   140,   140,   140,   141,   141,   142,   142,
     143,   143
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     1,     2,     0,     1,     1,     4,     4,
       4,     4,     4,     4,     4,     4,     5,     5,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     6,    10,
      11,     3,     6,     4,     9,    12,     0,     2,     6,    11,
       7,     2,     3,     1,     1,     0,     0,     0,     2,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     4,     1,     2,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     2,     2,     3,     4,     4,     4,
       6,     4,     6,     4,     6,     4,     6,     2,     2,     1,
       1,     2,     1,     2,     2,     2,     2,     4,     2,     1,
       3,     1,     3,     1,     3,     2,     2,     2,     2,     3,
       2,     3,     2,     2,     2,     3,     2,     0,     3,     0,
       8,     2,     0,     7,     8,     6,     2,     0,     1,     3,
       4,     0,     1,     3,     0,     2,     0,     1,     3,     1,
       3,     3,     0,     1,     1,     1,     1,     3,     1,     3,
       3,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

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

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
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
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
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
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
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
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
    case 18: /* BLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1647 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 19: /* FBLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1653 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 20: /* RBLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1659 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 21: /* ID  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1665 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 22: /* STRING  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1671 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* HANDLER  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1677 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* SYMBOL  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1683 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* ENDCLAUSE  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1689 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* tPLAYACCEL  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1695 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* tMETHOD  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1701 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* THEOBJECTFIELD  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).s; }
#line 1707 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 132: /* on  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1713 "engines/director/lingo/lingo-gr.cpp"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
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
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
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
# undef YYSTACK_RELOCATE
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

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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
  case 4:
#line 137 "engines/director/lingo/lingo-gr.y"
                                { yyerrok; }
#line 1983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 145 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 1994 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 151 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2002 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 154 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_after); }
#line 2008 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 155 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_before); }
#line 2014 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 156 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 162 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 169 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2048 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 175 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2060 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 182 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2071 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 188 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2082 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 194 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).s->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).e);
		delete (yyvsp[-2].objectfield).s;
		(yyval.code) = (yyvsp[0].code); }
#line 2093 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28:
#line 219 "engines/director/lingo/lingo-gr.y"
                                                                        {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[-3].code) - (yyvsp[-5].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 2] = end; }
#line 2104 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 230 "engines/director/lingo/lingo-gr.y"
                                                                                           {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[-7].code) - (yyvsp[-9].code));
		WRITE_UINT32(&finish, (yyvsp[-4].code) - (yyvsp[-9].code));
		WRITE_UINT32(&body, (yyvsp[-2].code) - (yyvsp[-9].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-9].code));
		WRITE_UINT32(&inc, 1);
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 1] = init;		/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 2] = finish;	/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 3] = body;		/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 4] = inc;		/* increment */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 5] = end; }
#line 2121 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 247 "engines/director/lingo/lingo-gr.y"
                                                                                                 {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[-8].code) - (yyvsp[-10].code));
		WRITE_UINT32(&finish, (yyvsp[-4].code) - (yyvsp[-10].code));
		WRITE_UINT32(&body, (yyvsp[-2].code) - (yyvsp[-10].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-10].code));
		WRITE_UINT32(&inc, (uint32)-1);
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 1] = init;		/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 2] = finish;	/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 3] = body;		/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 4] = inc;		/* increment */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 5] = end; }
#line 2138 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 259 "engines/director/lingo/lingo-gr.y"
                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 264 "engines/director/lingo/lingo-gr.y"
                                               {
		warning("STUB: TELL is not implemented"); }
#line 2155 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 266 "engines/director/lingo/lingo-gr.y"
                             {
		warning("STUB: TELL is not implemented"); }
#line 2162 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 270 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-6].code) - (yyvsp[-8].code));
		WRITE_UINT32(&else1, (yyvsp[-3].code) - (yyvsp[-8].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-8].code));
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-8].code), (yyvsp[-1].code) - (yyvsp[-8].code), 0); }
#line 2177 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 280 "engines/director/lingo/lingo-gr.y"
                                                                                                              {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-9].code) - (yyvsp[-11].code));
		WRITE_UINT32(&else1, (yyvsp[-6].code) - (yyvsp[-11].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-11].code));
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-11].code), (yyvsp[-1].code) - (yyvsp[-11].code), (yyvsp[-3].code) - (yyvsp[-11].code)); }
#line 2192 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 296 "engines/director/lingo/lingo-gr.y"
                                                        {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-3].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2203 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 304 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-8].code) - (yyvsp[-10].code));
		WRITE_UINT32(&else1, (yyvsp[-5].code) - (yyvsp[-10].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-10].code));
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-10].code), (yyvsp[-1].code) - (yyvsp[-10].code), (yyvsp[-3].code) - (yyvsp[-10].code)); }
#line 2218 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 314 "engines/director/lingo/lingo-gr.y"
                                                                   {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-4].code) - (yyvsp[-6].code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code));
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code)); }
#line 2233 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 327 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code3(LC::c_repeatwhilecode, STOP, STOP); }
#line 2239 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 330 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code3(LC::c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2249 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 337 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); }
#line 2259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 344 "engines/director/lingo/lingo-gr.y"
                                        {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); }
#line 2270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 352 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 355 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2282 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 358 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2288 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 363 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2298 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 369 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }
#line 2306 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 373 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2314 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 376 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 379 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2331 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 383 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2340 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 387 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2349 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 394 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2355 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 396 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2363 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 399 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2371 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 402 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2384 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 410 "engines/director/lingo/lingo-gr.y"
                               {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2395 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 416 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).s->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).e);
		delete (yyvsp[0].objectfield).s; }
#line 2405 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 422 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2411 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 423 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2417 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 424 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2423 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 425 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2429 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 426 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2435 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 427 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 428 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2447 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 429 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2453 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 430 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2459 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 431 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2465 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 432 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2471 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 433 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2477 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 434 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2483 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 435 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2489 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 436 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 437 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2501 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 438 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2507 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 439 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2513 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 440 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); }
#line 2519 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 441 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 442 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2531 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 443 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_intersects); }
#line 2537 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 444 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_within); }
#line 2543 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 445 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_charOf); }
#line 2549 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 446 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charToOf); }
#line 2555 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 447 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_itemOf); }
#line 2561 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 448 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemToOf); }
#line 2567 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 449 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_lineOf); }
#line 2573 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 450 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineToOf); }
#line 2579 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 451 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_wordOf); }
#line 2585 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 452 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordToOf); }
#line 2591 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 455 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2599 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 460 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_printtop); }
#line 2605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 463 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_exitRepeat); }
#line 2611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 464 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_procret); }
#line 2617 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 468 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 471 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_open); }
#line 2631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 472 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 2637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 475 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_global); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2643 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 476 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_global); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2649 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 479 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_property); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2655 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 480 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_property); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2661 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 483 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_instance); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2667 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 484 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_instance); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2673 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 495 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 2679 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 496 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 2685 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 497 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 2691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 498 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 2700 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 502 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 2709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 506 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 2718 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 516 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_playdone); }
#line 2724 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 517 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 2733 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 521 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 2742 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 525 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 2751 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 529 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 2757 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 529 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 2766 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 560 "engines/director/lingo/lingo-gr.y"
                { g_lingo->_indef = kStateInArgs; g_lingo->_currentFactory.clear(); }
#line 2772 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 561 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete (yyvsp[-6].s); }
#line 2783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 567 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 2789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 568 "engines/director/lingo/lingo-gr.y"
                  { g_lingo->_indef = kStateInArgs; }
#line 2795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 569 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete (yyvsp[-6].s); }
#line 2806 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 575 "engines/director/lingo/lingo-gr.y"
                                                                     {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 2821 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 585 "engines/director/lingo/lingo-gr.y"
                                                 {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->_indef = kStateNone;
		g_lingo->clearArgStack();
		g_lingo->_ignoreMe = false;
		delete (yyvsp[-5].s); }
#line 2833 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 593 "engines/director/lingo/lingo-gr.y"
            { (yyval.s) = (yyvsp[0].s); g_lingo->_indef = kStateInArgs; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 2839 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 595 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 2845 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 596 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 2851 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 597 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 2857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 598 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-3].narg) + 1; delete (yyvsp[0].s); }
#line 2863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 602 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2869 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 603 "engines/director/lingo/lingo-gr.y"
                                        { delete (yyvsp[0].s); }
#line 2875 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 606 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArgStore(); g_lingo->_indef = kStateInDef; }
#line 2881 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 609 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 2893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 618 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 2899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 619 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 2905 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 620 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 623 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 2917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 624 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 627 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = (yyvsp[-1].code); }
#line 2929 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 630 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 2935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 631 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 2941 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 632 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 2947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 633 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 2953 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 636 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 2959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 637 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 640 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 2971 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 641 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 644 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 2986 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 648 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 2995 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 2999 "engines/director/lingo/lingo-gr.cpp"

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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

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
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
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
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 655 "engines/director/lingo/lingo-gr.y"

