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
    LEXERROR = 267,
    INT = 268,
    ARGC = 269,
    ARGCNORET = 270,
    THEENTITY = 271,
    THEENTITYWITHID = 272,
    THEMENUITEMENTITY = 273,
    FLOAT = 274,
    BLTIN = 275,
    FBLTIN = 276,
    RBLTIN = 277,
    ID = 278,
    STRING = 279,
    HANDLER = 280,
    SYMBOL = 281,
    ENDCLAUSE = 282,
    tPLAYACCEL = 283,
    tMETHOD = 284,
    THEOBJECTFIELD = 285,
    tDOWN = 286,
    tELSE = 287,
    tELSIF = 288,
    tEXIT = 289,
    tGLOBAL = 290,
    tGO = 291,
    tIF = 292,
    tIN = 293,
    tINTO = 294,
    tLOOP = 295,
    tMACRO = 296,
    tMOVIE = 297,
    tNEXT = 298,
    tOF = 299,
    tPREVIOUS = 300,
    tPUT = 301,
    tREPEAT = 302,
    tSET = 303,
    tTHEN = 304,
    tTO = 305,
    tWHEN = 306,
    tWITH = 307,
    tWHILE = 308,
    tNLELSE = 309,
    tFACTORY = 310,
    tOPEN = 311,
    tPLAY = 312,
    tDONE = 313,
    tINSTANCE = 314,
    tGE = 315,
    tLE = 316,
    tEQ = 317,
    tNEQ = 318,
    tAND = 319,
    tOR = 320,
    tNOT = 321,
    tMOD = 322,
    tAFTER = 323,
    tBEFORE = 324,
    tCONCAT = 325,
    tCONTAINS = 326,
    tSTARTS = 327,
    tCHAR = 328,
    tITEM = 329,
    tLINE = 330,
    tWORD = 331,
    tSPRITE = 332,
    tINTERSECTS = 333,
    tWITHIN = 334,
    tTELL = 335,
    tPROPERTY = 336,
    tON = 337,
    tENDIF = 338,
    tENDREPEAT = 339,
    tENDTELL = 340
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
		Common::String *os;
		int oe;
	} objectfield;

#line 254 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  126
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1689

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  101
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  51
/* YYNRULES -- Number of rules.  */
#define YYNRULES  171
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  369

#define YYUNDEFTOK  2
#define YYMAXUTOK   340


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      94,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    93,    88,     2,
      95,    96,    91,    89,    97,    90,     2,    92,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   100,     2,
      86,     2,    87,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    98,     2,    99,     2,     2,     2,     2,     2,     2,
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
      85
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   135,   135,   136,   137,   139,   140,   141,   143,   149,
     152,   153,   154,   160,   167,   173,   180,   186,   193,   204,
     211,   212,   213,   215,   216,   217,   218,   220,   221,   226,
     237,   254,   266,   277,   279,   284,   288,   293,   297,   307,
     318,   319,   321,   328,   338,   349,   351,   357,   363,   370,
     372,   374,   375,   376,   378,   384,   387,   390,   394,   398,
     402,   404,   405,   406,   409,   412,   415,   423,   429,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   459,   460,   461,   462,   463,   464,   465,
     466,   468,   471,   473,   474,   475,   476,   477,   478,   478,
     479,   479,   480,   481,   484,   487,   488,   490,   494,   499,
     503,   508,   512,   524,   525,   526,   527,   531,   535,   540,
     541,   543,   544,   548,   552,   556,   556,   586,   586,   586,
     593,   594,   594,   601,   611,   619,   619,   621,   622,   623,
     624,   626,   627,   628,   630,   632,   640,   641,   642,   644,
     645,   647,   649,   650,   651,   652,   654,   655,   657,   658,
     660,   664
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNARY", "CASTREF", "VOID", "VAR",
  "POINT", "RECT", "ARRAY", "OBJECT", "REFERENCE", "LEXERROR", "INT",
  "ARGC", "ARGCNORET", "THEENTITY", "THEENTITYWITHID", "THEMENUITEMENTITY",
  "FLOAT", "BLTIN", "FBLTIN", "RBLTIN", "ID", "STRING", "HANDLER",
  "SYMBOL", "ENDCLAUSE", "tPLAYACCEL", "tMETHOD", "THEOBJECTFIELD",
  "tDOWN", "tELSE", "tELSIF", "tEXIT", "tGLOBAL", "tGO", "tIF", "tIN",
  "tINTO", "tLOOP", "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS",
  "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE",
  "tNLELSE", "tFACTORY", "tOPEN", "tPLAY", "tDONE", "tINSTANCE", "tGE",
  "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE",
  "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD",
  "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON",
  "tENDIF", "tENDREPEAT", "tENDTELL", "'<'", "'>'", "'&'", "'+'", "'-'",
  "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "','", "'['", "']'", "':'",
  "$accept", "program", "programline", "asgn", "stmtoneliner",
  "stmtonelinerwithif", "stmt", "tellstart", "ifstmt", "elseifstmtlist",
  "elseifstmt", "ifoneliner", "repeatwhile", "repeatwith", "if", "elseif",
  "begin", "end", "stmtlist", "when", "simpleexpr", "expr", "chunkexpr",
  "reference", "proc", "$@1", "$@2", "globallist", "propertylist",
  "instancelist", "gotofunc", "gotomovie", "playfunc", "$@3", "defn",
  "$@4", "$@5", "$@6", "on", "$@7", "argdef", "endargdef", "argstore",
  "macro", "arglist", "nonemptyarglist", "list", "valuelist", "linearlist",
  "proplist", "proppair", YY_NULLPTR
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
     335,   336,   337,   338,   339,   340,    60,    62,    38,    43,
      45,    42,    47,    37,    10,    40,    41,    44,    91,    93,
      58
};
# endif

#define YYPACT_NINF (-269)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-6)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     342,   -70,  -269,  -269,  1026,  -269,  1060,  1140,    30,  1174,
    -269,  -269,  -269,  -269,  -269,    27,  -269,   908,  -269,  -269,
      33,  1026,   -33,    93,    54,    73,  1026,   946,    91,  1026,
    1026,  1026,  1026,  1026,  1026,  1026,  -269,  -269,  1026,  1026,
    1026,   422,     8,  -269,  -269,  -269,  -269,  -269,  1026,   -20,
    1026,   748,  -269,  1584,  -269,  -269,  -269,  -269,  -269,  -269,
    -269,  -269,  -269,  -269,   -12,  1026,  1584,  1026,  1584,    18,
    1026,    18,  -269,  -269,  1026,  1584,    20,  1026,  -269,  -269,
      95,  -269,  1026,  -269,    77,  -269,   414,  -269,    97,  -269,
     166,   101,  -269,   -29,    30,    30,    -2,    79,    82,  -269,
    1501,  -269,   414,  -269,  -269,    35,  -269,  1195,  1229,  1262,
    1295,  1551,  1460,   128,   130,  -269,  -269,  1514,    55,    57,
    -269,  1584,    59,    63,    64,  -269,  -269,   508,  1584,  1026,
    1026,  1584,  -269,  -269,  1026,  1584,  -269,  -269,  1026,  1026,
    1026,  1026,  1026,  1026,  1026,  1026,  1026,  1026,  1026,  1026,
    1026,  1026,  1026,  1026,  1026,   131,  1026,   166,  1514,   -38,
    1026,    -3,    11,  1026,    18,   131,  -269,    65,  1584,  1026,
    -269,  -269,    16,  1026,  1026,  -269,  1026,  1026,     0,   119,
    1026,  1026,  1026,  -269,  1026,  -269,   141,  1026,  1026,  1026,
    1026,  1026,  1026,  1026,  1026,  1026,  1026,  -269,  -269,  -269,
      69,  -269,  -269,    30,    30,  -269,  1026,    60,  -269,  -269,
    1584,  1584,   118,  -269,  1584,     9,     9,     9,     9,  1597,
    1597,  -269,    14,     9,     9,     9,     9,    14,   -51,   -51,
    -269,  -269,  -269,   -42,  -269,  1584,  -269,  -269,  1584,   -30,
     148,  1584,  -269,  -269,  -269,  1584,  1584,  1584,     9,  1026,
    1026,   151,  1584,     9,  1584,  1584,  -269,  1584,  1328,  1584,
    1361,  1584,  1394,  1584,  1427,  1584,  1584,   828,  -269,   152,
    -269,  -269,  1584,    55,    57,  -269,   668,  -269,     1,  -269,
     132,    85,   156,    85,  -269,   131,  1584,     9,    30,  1026,
    1026,  1026,  1026,  -269,   668,  -269,  -269,  -269,   109,   668,
     144,  1026,   668,   828,   172,  -269,  -269,  -269,   -26,   149,
    1584,  1584,  1584,  1584,  -269,   113,  -269,   122,  1026,  1584,
    -269,  -269,  -269,   588,   668,    85,  1026,  -269,  -269,  1584,
    -269,    80,   -18,   180,  -269,  1584,  -269,   668,  -269,  -269,
    -269,  1026,   124,  -269,  -269,  -269,   111,   668,   668,   125,
    -269,  1584,  -269,   828,   187,   129,  -269,   668,   163,  -269,
    -269,  -269,   133,  -269,   134,  -269,   668,  -269,  -269
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    55,    66,     0,    56,   156,   156,     0,    59,
      58,    57,   135,   141,    68,   107,   108,     0,    47,   137,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   110,   145,     0,     0,
       0,   162,     0,     3,    69,    27,     7,    28,     0,     0,
       0,     0,    61,    21,   102,    62,    22,   104,   105,     6,
      49,    20,    60,     4,    59,     0,    67,   156,   157,   114,
     156,    64,    59,   101,   156,   159,   155,   156,    49,   106,
       0,   123,     0,   124,     0,   125,   126,   128,     0,    33,
     103,     0,    45,     0,     0,     0,     0,     0,     0,   140,
     116,   131,   132,   134,   121,   112,    83,     0,     0,     0,
       0,     0,     0,     0,     0,    88,    89,     0,    58,    57,
     163,   166,     0,   164,   165,   168,     1,     5,    50,     0,
       0,    50,    50,    26,     0,    24,    25,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   147,   156,     0,   157,     0,
       0,     0,     0,     0,   136,   147,   117,   109,   130,     0,
     127,   138,     0,     0,     0,    46,     0,     0,     0,     0,
       0,     0,     0,    54,     0,   133,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,    37,   119,
     111,   146,    90,     0,     0,   161,     0,     0,     2,    51,
      50,    50,     0,    34,    50,    79,    80,    77,    78,    81,
      82,    74,    85,    86,    87,    76,    75,    84,    70,    71,
      72,    73,   148,     0,   113,   158,    63,    65,   160,     0,
       0,   129,    49,     8,     9,    10,    11,    15,    13,     0,
       0,     0,    14,    12,    19,   115,   122,    93,     0,    95,
       0,    97,     0,    99,     0,    91,    92,     0,    51,     0,
     171,   170,   167,     0,     0,   169,    50,    51,     0,    51,
       0,   154,     0,   154,   118,   147,    16,    17,     0,     0,
       0,     0,     0,    50,    50,   120,    52,    53,     0,    50,
       0,     0,    50,     0,     0,    51,   149,    51,     0,     0,
      94,    96,    98,   100,    36,     0,    29,     0,     0,    50,
      40,    50,   150,   144,   142,   154,     0,    35,    32,    50,
      51,    50,     0,   151,    51,    18,    51,    50,    49,    48,
      41,     0,     0,    49,    44,   152,   143,   139,    50,     0,
      51,    50,    38,     0,     0,     0,    30,    50,     0,    50,
     153,    31,     0,    51,     0,    39,    50,    43,    42
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -269,  -269,    87,  -269,  -258,  -269,     6,    17,  -269,  -269,
    -269,  -269,  -269,  -269,   167,  -269,   -74,   -10,   -57,  -269,
      -7,    -4,  -269,    47,   170,  -269,  -269,  -269,  -269,  -269,
    -269,   -17,  -269,  -269,  -269,  -269,  -269,  -269,  -269,  -269,
    -158,  -269,  -268,   173,     5,  -269,  -269,  -269,  -269,  -269,
      25
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    42,    43,    44,    45,   132,   297,   267,    47,   331,
     340,   133,    48,    49,    50,   341,   155,   209,   276,    51,
      52,    53,    54,    55,    56,    80,   113,   167,   200,   105,
      57,    87,    58,    77,    59,    88,   242,    78,    60,   114,
     233,   346,   305,    61,   162,    76,    62,   122,   123,   124,
     125
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      66,    73,    68,    68,   165,    75,    46,   239,   126,   293,
     103,    69,    71,    86,   343,   307,   144,    90,   129,    91,
      92,   176,   100,   102,    63,   106,   107,   108,   109,   110,
     111,   112,   300,   177,   115,   116,   117,   121,     8,   243,
     153,   154,   130,     2,   128,   321,   131,   135,   180,     5,
     249,   301,   281,    72,    10,   282,    11,   334,   234,   160,
     181,   157,   250,   158,   283,   344,   158,   282,   325,   170,
     158,   282,   159,    68,    79,   161,   144,    98,   168,   145,
      89,   144,   164,   156,   273,   185,   274,   178,   179,    30,
      31,    32,    33,   236,   160,   359,    99,   150,   151,   152,
     153,   154,   127,   151,   152,   153,   154,   237,   160,    93,
      94,    95,   338,   339,   104,   160,    96,   163,   166,   169,
     171,   212,   213,    97,   175,   210,   211,   308,    41,   182,
     214,   183,   186,    46,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   199,    68,   201,   232,   203,   235,   204,   205,   238,
     206,   207,   240,   251,   256,   241,   269,   279,   285,   245,
     246,   284,   247,   248,   288,   295,   252,   253,   254,   306,
     255,   303,   304,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   316,   318,   322,   270,   271,   327,   326,
     277,   278,   272,   345,   280,   172,   328,   352,   354,   356,
     360,   294,   363,   361,   208,   268,   365,   367,   134,   244,
     299,   136,   302,     0,   137,     0,   138,   139,   140,   141,
     142,   143,   275,   144,   173,   174,   145,   146,   147,     0,
       0,     0,     0,     0,     0,   286,   287,     0,   323,     0,
     324,     0,   148,   149,   150,   151,   152,   153,   154,     0,
       0,     0,     0,     0,   350,     0,   298,     0,     0,   353,
       0,     0,     0,   337,     0,     0,     0,   347,     0,   348,
       0,   309,     0,   314,   315,   310,   311,   312,   313,   317,
       0,     0,   320,   357,     0,     0,     0,   319,     0,     0,
       0,     0,     0,     0,     0,     0,   366,     0,     0,   330,
       0,   332,     0,     0,   329,     0,     0,     0,     0,   336,
       0,   342,   335,     0,     0,     0,     0,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   351,   355,     0,
       0,   358,    -5,     1,     0,     0,     0,   362,     0,   364,
       0,     0,     0,     0,     0,     2,   368,     0,     3,     4,
       0,     5,     6,     7,     8,     9,    10,     0,    11,     0,
      12,    13,    14,     0,     0,     0,    15,    16,    17,    18,
       0,     0,     0,    19,     0,    20,     0,     0,    21,    22,
      23,     0,     0,    24,     0,     0,     0,    25,    26,    27,
       0,    28,     0,     0,     0,     0,     0,     0,    29,     0,
       0,     0,     0,     0,     0,    30,    31,    32,    33,    34,
       0,     0,    35,    36,    37,     0,     0,     0,     0,     0,
       0,    38,    39,     0,     0,     2,    -5,    40,     3,     4,
      41,     5,     0,     7,     8,    64,   118,     0,   119,     0,
       0,     0,    14,     0,     0,     0,    82,     0,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,     0,
      23,     0,     0,     0,   138,   139,   140,   141,   142,   143,
       0,   144,     0,     0,   145,   146,   147,     0,    29,     0,
       0,     0,     0,     0,     0,    30,    31,    32,    33,    34,
     148,   149,   150,   151,   152,   153,   154,     0,     0,     0,
       0,    38,    39,     0,     0,     0,     0,    40,     0,     0,
      41,     2,   120,     0,     3,     4,     0,     5,     6,     7,
       8,     9,    10,     0,    11,     0,    12,    13,    14,     0,
       0,     0,    15,    16,    17,    18,     0,     0,     0,    19,
       0,    20,     0,     0,    21,    22,    23,     0,     0,    24,
       0,     0,     0,    25,    26,    27,     0,    28,     0,     0,
       0,     0,     0,     0,    29,     0,     0,     0,     0,     0,
       0,    30,    31,    32,    33,    34,     0,     0,    35,    36,
      37,     0,     0,     0,     0,     0,     0,    38,    39,     0,
       0,     2,     0,    40,     3,     4,    41,     5,     6,     7,
       8,     9,    10,     0,    11,   333,    12,     0,    14,     0,
       0,     0,    15,    16,    17,    18,     0,     0,     0,     0,
       0,    20,     0,     0,    21,    22,    23,     0,     0,    24,
       0,     0,     0,     0,    26,    27,     0,    28,     0,     0,
       0,     0,     0,     0,    29,     0,     0,     0,     0,     0,
       0,    30,    31,    32,    33,    34,     0,     0,    35,    36,
       0,     0,     0,     0,     0,     0,     0,    38,    39,     0,
       0,     2,   296,    40,     3,     4,    41,     5,     6,     7,
       8,     9,    10,     0,    11,     0,    12,     0,    14,     0,
       0,     0,    15,    16,    17,    18,     0,     0,     0,     0,
       0,    20,     0,     0,    21,    22,    23,     0,     0,    24,
       0,     0,     0,     0,    26,    27,     0,    28,     0,     0,
       0,     0,     0,     0,    29,     0,     0,     0,     0,     0,
       0,    30,    31,    32,    33,    34,     0,     0,    35,    36,
       0,     0,     0,     0,     0,     0,     0,    38,    39,     0,
       0,     2,   296,    40,     3,     4,    41,     5,     6,     7,
       8,     9,    10,     0,    11,     0,    12,     0,    14,     0,
       0,     0,    15,    16,    17,    18,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,    23,     0,     0,     0,
       0,     0,     0,     0,    26,    27,     0,    28,     0,     0,
       0,     0,     0,     0,    29,     0,     0,     0,     0,     0,
       0,    30,    31,    32,    33,    34,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,    38,    39,     0,
       0,     2,     0,    40,     3,     4,    41,     5,     6,     7,
       8,     9,    10,     0,    11,     0,    12,     0,    14,     0,
       0,     0,    15,    16,    17,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,    23,     0,     0,     0,
       0,     0,     0,     0,    26,    27,     0,    28,     0,     0,
       0,     0,     0,     0,    29,     0,     0,     0,     0,     0,
       0,    30,    31,    32,    33,    34,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,    38,    39,     0,
       0,     2,     0,    40,     3,     4,    41,     5,     0,     7,
       8,    64,    10,     0,    11,     0,     0,     0,    14,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,    84,    85,    65,     0,    23,     0,     0,     2,
       0,     0,     3,     4,     0,     5,     0,     7,     8,    64,
      10,     0,    11,     0,    29,     0,    14,     0,     0,     0,
       0,    30,    31,    32,    33,    34,     0,     0,    82,     0,
      84,     0,    65,     0,    23,     0,     0,    38,    39,     0,
       0,     0,     0,    40,   101,     0,    41,     0,     0,     0,
       0,     0,    29,     0,     0,     0,     0,     0,     0,    30,
      31,    32,    33,    34,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    38,    39,     0,     0,     2,
       0,    40,     3,     4,    41,     5,     0,     7,     8,    64,
      10,     0,    11,     0,     0,     0,    14,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    65,     2,    23,     0,     3,     4,     0,     5,
       0,     7,     8,    64,    10,     0,    11,     0,     0,     0,
      14,     0,    29,     0,     0,     0,     0,     0,     0,    30,
      31,    32,    33,    34,     0,     0,    65,     0,    23,     0,
       0,     0,     0,     0,     0,    38,    39,     0,     0,     0,
       0,    40,     0,     0,    41,     0,    29,     0,     0,     0,
       0,     0,     0,    30,    31,    32,    33,    34,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    38,
      39,     0,     0,     2,     0,    67,     3,     4,    41,     5,
       0,     7,     8,    64,    10,     0,    11,     0,     0,     0,
      14,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    65,     2,    23,     0,
       3,     4,     0,     5,     0,     7,     8,    64,    10,     0,
      11,     0,     0,     0,    14,     0,    29,     0,     0,     0,
       0,     0,     0,    30,    31,    32,    33,    34,     0,     0,
      65,     0,    23,     0,     0,     0,     0,     0,     0,    38,
      39,     0,     0,     0,     0,    70,     0,     0,    41,   187,
      29,     0,     0,     0,     0,   188,     0,    30,    31,    32,
      33,    34,     0,     0,     0,   138,   139,   140,   141,   142,
     143,     0,   144,    38,    39,   145,   146,   147,     0,    74,
       0,     0,    41,   189,     0,     0,     0,     0,     0,   190,
       0,   148,   149,   150,   151,   152,   153,   154,     0,   138,
     139,   140,   141,   142,   143,     0,   144,     0,     0,   145,
     146,   147,     0,     0,     0,     0,   191,     0,     0,     0,
       0,     0,   192,     0,     0,   148,   149,   150,   151,   152,
     153,   154,   138,   139,   140,   141,   142,   143,     0,   144,
       0,     0,   145,   146,   147,     0,     0,     0,     0,   193,
       0,     0,     0,     0,     0,   194,     0,     0,   148,   149,
     150,   151,   152,   153,   154,   138,   139,   140,   141,   142,
     143,     0,   144,     0,     0,   145,   146,   147,     0,     0,
       0,     0,   289,     0,     0,     0,     0,     0,     0,     0,
       0,   148,   149,   150,   151,   152,   153,   154,   138,   139,
     140,   141,   142,   143,     0,   144,     0,     0,   145,   146,
     147,     0,     0,     0,     0,   290,     0,     0,     0,     0,
       0,     0,     0,     0,   148,   149,   150,   151,   152,   153,
     154,   138,   139,   140,   141,   142,   143,     0,   144,     0,
       0,   145,   146,   147,     0,     0,     0,     0,   291,     0,
       0,     0,     0,     0,     0,     0,     0,   148,   149,   150,
     151,   152,   153,   154,   138,   139,   140,   141,   142,   143,
       0,   144,     0,     0,   145,   146,   147,     0,     0,     0,
       0,   292,     0,     0,     0,     0,     0,     0,     0,     0,
     148,   149,   150,   151,   152,   153,   154,   138,   139,   140,
     141,   142,   143,     0,   144,     0,     0,   145,   146,   147,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     197,     0,     0,   148,   149,   150,   151,   152,   153,   154,
     138,   139,   140,   141,   142,   143,     0,   144,     0,     0,
     145,   146,   147,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   148,   149,   150,   151,
     152,   153,   154,   184,   198,     0,     0,     0,     0,     0,
       0,   138,   139,   140,   141,   142,   143,     0,   144,     0,
       0,   145,   146,   147,   138,   139,   140,   141,   142,   143,
       0,   144,     0,     0,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,     0,     0,     0,     0,     0,     0,
     148,   149,   150,   151,   152,   153,   154,     0,     0,     0,
     202,   138,   139,   140,   141,   142,   143,     0,   144,     0,
       0,   145,   146,   147,     0,     0,     0,     0,     0,   195,
     196,     0,     0,     0,     0,     0,     0,   148,   149,   150,
     151,   152,   153,   154,   138,   139,   140,   141,   142,   143,
       0,   144,     0,     0,   145,   146,   147,   138,   139,   140,
     141,     0,     0,     0,   144,     0,     0,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,     0,     0,
       0,     0,     0,   148,   149,   150,   151,   152,   153,   154
};

static const yytype_int16 yycheck[] =
{
       4,     8,     6,     7,    78,     9,     0,   165,     0,   267,
      27,     6,     7,    17,    32,   283,    67,    21,    38,    52,
      53,    50,    26,    27,    94,    29,    30,    31,    32,    33,
      34,    35,    31,    62,    38,    39,    40,    41,    22,    23,
      91,    92,    62,    13,    48,   303,    50,    51,    50,    19,
      50,    50,    94,    23,    24,    97,    26,   325,    96,    97,
      62,    65,    62,    67,    94,    83,    70,    97,    94,    86,
      74,    97,    67,    77,    47,    70,    67,    23,    82,    70,
      47,    67,    77,    95,    24,   102,    26,    94,    95,    73,
      74,    75,    76,    96,    97,   353,    23,    88,    89,    90,
      91,    92,    94,    89,    90,    91,    92,    96,    97,    16,
      17,    18,    32,    33,    23,    97,    23,    97,    23,    42,
      23,   131,   132,    30,    23,   129,   130,   285,    98,    50,
     134,    49,    97,   127,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,    23,   156,    23,    23,   100,   160,   100,    99,   163,
      97,    97,    97,    44,    23,   169,    97,    49,   242,   173,
     174,    23,   176,   177,    23,    23,   180,   181,   182,    23,
     184,    49,    97,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    84,    50,    23,   203,   204,    85,    50,
     210,   211,   206,    23,   214,    39,    84,    83,    97,    84,
      23,   268,    49,    84,   127,   198,    83,    83,    51,   172,
     277,    51,   279,    -1,    51,    -1,    60,    61,    62,    63,
      64,    65,   207,    67,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    -1,    -1,    -1,   249,   250,    -1,   305,    -1,
     307,    -1,    86,    87,    88,    89,    90,    91,    92,    -1,
      -1,    -1,    -1,    -1,   338,    -1,   276,    -1,    -1,   343,
      -1,    -1,    -1,   330,    -1,    -1,    -1,   334,    -1,   336,
      -1,   288,    -1,   293,   294,   289,   290,   291,   292,   299,
      -1,    -1,   302,   350,    -1,    -1,    -1,   301,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   363,    -1,    -1,   319,
      -1,   321,    -1,    -1,   318,    -1,    -1,    -1,    -1,   329,
      -1,   331,   326,    -1,    -1,    -1,    -1,   337,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   341,   348,    -1,
      -1,   351,     0,     1,    -1,    -1,    -1,   357,    -1,   359,
      -1,    -1,    -1,    -1,    -1,    13,   366,    -1,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    -1,    26,    -1,
      28,    29,    30,    -1,    -1,    -1,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    -1,    43,    -1,    -1,    46,    47,
      48,    -1,    -1,    51,    -1,    -1,    -1,    55,    56,    57,
      -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      -1,    -1,    80,    81,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    -1,    -1,    13,    94,    95,    16,    17,
      98,    19,    -1,    21,    22,    23,    24,    -1,    26,    -1,
      -1,    -1,    30,    -1,    -1,    -1,    42,    -1,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,
      48,    -1,    -1,    -1,    60,    61,    62,    63,    64,    65,
      -1,    67,    -1,    -1,    70,    71,    72,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      86,    87,    88,    89,    90,    91,    92,    -1,    -1,    -1,
      -1,    89,    90,    -1,    -1,    -1,    -1,    95,    -1,    -1,
      98,    13,   100,    -1,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    26,    -1,    28,    29,    30,    -1,
      -1,    -1,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      -1,    43,    -1,    -1,    46,    47,    48,    -1,    -1,    51,
      -1,    -1,    -1,    55,    56,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    80,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    -1,
      -1,    13,    -1,    95,    16,    17,    98,    19,    20,    21,
      22,    23,    24,    -1,    26,    27,    28,    -1,    30,    -1,
      -1,    -1,    34,    35,    36,    37,    -1,    -1,    -1,    -1,
      -1,    43,    -1,    -1,    46,    47,    48,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    56,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    -1,
      -1,    13,    94,    95,    16,    17,    98,    19,    20,    21,
      22,    23,    24,    -1,    26,    -1,    28,    -1,    30,    -1,
      -1,    -1,    34,    35,    36,    37,    -1,    -1,    -1,    -1,
      -1,    43,    -1,    -1,    46,    47,    48,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    56,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    -1,
      -1,    13,    94,    95,    16,    17,    98,    19,    20,    21,
      22,    23,    24,    -1,    26,    -1,    28,    -1,    30,    -1,
      -1,    -1,    34,    35,    36,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    -1,
      -1,    13,    -1,    95,    16,    17,    98,    19,    20,    21,
      22,    23,    24,    -1,    26,    -1,    28,    -1,    30,    -1,
      -1,    -1,    34,    35,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    -1,
      -1,    13,    -1,    95,    16,    17,    98,    19,    -1,    21,
      22,    23,    24,    -1,    26,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,
      42,    43,    44,    45,    46,    -1,    48,    -1,    -1,    13,
      -1,    -1,    16,    17,    -1,    19,    -1,    21,    22,    23,
      24,    -1,    26,    -1,    66,    -1,    30,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    42,    -1,
      44,    -1,    46,    -1,    48,    -1,    -1,    89,    90,    -1,
      -1,    -1,    -1,    95,    58,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    -1,    -1,    13,
      -1,    95,    16,    17,    98,    19,    -1,    21,    22,    23,
      24,    -1,    26,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    13,    48,    -1,    16,    17,    -1,    19,
      -1,    21,    22,    23,    24,    -1,    26,    -1,    -1,    -1,
      30,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    -1,    -1,    46,    -1,    48,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    -1,    -1,    -1,
      -1,    95,    -1,    -1,    98,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    -1,    -1,    13,    -1,    95,    16,    17,    98,    19,
      -1,    21,    22,    23,    24,    -1,    26,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    13,    48,    -1,
      16,    17,    -1,    19,    -1,    21,    22,    23,    24,    -1,
      26,    -1,    -1,    -1,    30,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    -1,    -1,
      46,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    -1,    -1,    -1,    -1,    95,    -1,    -1,    98,    44,
      66,    -1,    -1,    -1,    -1,    50,    -1,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    89,    90,    70,    71,    72,    -1,    95,
      -1,    -1,    98,    44,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    86,    87,    88,    89,    90,    91,    92,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    -1,    -1,    70,
      71,    72,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    86,    87,    88,    89,    90,
      91,    92,    60,    61,    62,    63,    64,    65,    -1,    67,
      -1,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    86,    87,
      88,    89,    90,    91,    92,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    71,    72,    -1,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    87,    88,    89,    90,    91,    92,    60,    61,
      62,    63,    64,    65,    -1,    67,    -1,    -1,    70,    71,
      72,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,    91,
      92,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    71,    72,    -1,    -1,    -1,    -1,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,
      89,    90,    91,    92,    60,    61,    62,    63,    64,    65,
      -1,    67,    -1,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    92,    60,    61,    62,
      63,    64,    65,    -1,    67,    -1,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    -1,    86,    87,    88,    89,    90,    91,    92,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      70,    71,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,
      90,    91,    92,    52,    94,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    71,    72,    60,    61,    62,    63,    64,    65,
      -1,    67,    -1,    -1,    70,    71,    72,    86,    87,    88,
      89,    90,    91,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    92,    -1,    -1,    -1,
      96,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,
      89,    90,    91,    92,    60,    61,    62,    63,    64,    65,
      -1,    67,    -1,    -1,    70,    71,    72,    60,    61,    62,
      63,    -1,    -1,    -1,    67,    -1,    -1,    70,    71,    72,
      86,    87,    88,    89,    90,    91,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    91,    92
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    13,    16,    17,    19,    20,    21,    22,    23,
      24,    26,    28,    29,    30,    34,    35,    36,    37,    41,
      43,    46,    47,    48,    51,    55,    56,    57,    59,    66,
      73,    74,    75,    76,    77,    80,    81,    82,    89,    90,
      95,    98,   102,   103,   104,   105,   107,   109,   113,   114,
     115,   120,   121,   122,   123,   124,   125,   131,   133,   135,
     139,   144,   147,    94,    23,    46,   122,    95,   122,   145,
      95,   145,    23,   121,    95,   122,   146,   134,   138,    47,
     126,    40,    42,    43,    44,    45,   122,   132,   136,    47,
     122,    52,    53,    16,    17,    18,    23,    30,    23,    23,
     122,    58,   122,   132,    23,   130,   122,   122,   122,   122,
     122,   122,   122,   127,   140,   122,   122,   122,    24,    26,
     100,   122,   148,   149,   150,   151,     0,    94,   122,    38,
      62,   122,   106,   112,   115,   122,   125,   144,    60,    61,
      62,    63,    64,    65,    67,    70,    71,    72,    86,    87,
      88,    89,    90,    91,    92,   117,    95,   122,   122,   145,
      97,   145,   145,    97,   145,   117,    23,   128,   122,    42,
     132,    23,    39,    68,    69,    23,    50,    62,   121,   121,
      50,    62,    50,    49,    52,   132,    97,    44,    50,    44,
      50,    44,    50,    44,    50,    78,    79,    50,    94,    23,
     129,    23,    96,   100,   100,    99,    97,    97,   103,   118,
     122,   122,   118,   118,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,    23,   141,    96,   122,    96,    96,   122,   141,
      97,   122,   137,    23,   124,   122,   122,   122,   122,    50,
      62,    44,   122,   122,   122,   122,    23,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   108,   108,    97,
     121,   121,   122,    24,    26,   151,   119,   118,   118,    49,
     118,    94,    97,    94,    23,   117,   122,   122,    23,    44,
      44,    44,    44,   105,   119,    23,    94,   107,   118,   119,
      31,    50,   119,    49,    97,   143,    23,   143,   141,   121,
     122,   122,   122,   122,   118,   118,    84,   118,    50,   122,
     118,   105,    23,   119,   119,    94,    50,    85,    84,   122,
     118,   110,   118,    27,   143,   122,   118,   119,    32,    33,
     111,   116,   118,    32,    83,    23,   142,   119,   119,   118,
     117,   122,    83,   117,    97,   118,    84,   119,   118,   105,
      23,    84,   118,    49,   118,    83,   119,    83,   118
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   101,   102,   102,   102,   103,   103,   103,   104,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   104,
     105,   105,   105,   106,   106,   106,   106,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   108,   109,   109,
     110,   110,   111,   112,   112,   113,   114,   115,   116,   117,
     118,   119,   119,   119,   120,   121,   121,   121,   121,   121,
     121,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   123,   123,   123,   123,   123,   123,   123,
     123,   124,   124,   125,   125,   125,   125,   125,   126,   125,
     127,   125,   125,   125,   125,   125,   125,   128,   128,   129,
     129,   130,   130,   131,   131,   131,   131,   131,   131,   132,
     132,   133,   133,   133,   133,   134,   133,   136,   137,   135,
     135,   138,   135,   135,   135,   140,   139,   141,   141,   141,
     141,   142,   142,   142,   143,   144,   145,   145,   145,   146,
     146,   147,   148,   148,   148,   148,   149,   149,   150,   150,
     151,   151
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     1,     2,     0,     1,     1,     4,     4,
       4,     4,     4,     4,     4,     4,     5,     5,     8,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
      10,    11,     7,     2,     3,     7,     6,     0,     9,    12,
       0,     2,     6,    11,     7,     2,     3,     1,     1,     0,
       0,     0,     2,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     2,     4,     1,     2,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     3,     3,     3,     3,     2,     2,
       3,     4,     4,     4,     6,     4,     6,     4,     6,     4,
       6,     2,     1,     2,     1,     1,     2,     1,     0,     3,
       0,     3,     2,     4,     2,     4,     2,     1,     3,     1,
       3,     1,     3,     2,     2,     2,     2,     3,     2,     3,
       2,     2,     2,     3,     2,     0,     3,     0,     0,     9,
       2,     0,     7,     8,     6,     0,     3,     0,     1,     3,
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
    case 20: /* BLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1701 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 21: /* FBLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1707 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 22: /* RBLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1713 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* ID  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1719 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* STRING  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1725 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* HANDLER  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1731 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* SYMBOL  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1737 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* ENDCLAUSE  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1743 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* tPLAYACCEL  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1749 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* tMETHOD  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1755 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 30: /* THEOBJECTFIELD  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).os; }
#line 1761 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 139: /* on  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1767 "engines/director/lingo/lingo-gr.cpp"
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
#line 2037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 143 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2048 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 149 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2056 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 152 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_after); }
#line 2062 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 153 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_before); }
#line 2068 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 154 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 160 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 167 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2102 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 173 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2114 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 180 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2125 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 186 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2136 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 193 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if (!(yyvsp[-3].s)->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		warning("STUB: menuItem entity");
		g_lingo->code1(LC::c_themenuitementityassign);
		g_lingo->codeInt((yyvsp[-6].e)[0]);
		g_lingo->codeInt((yyvsp[-6].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2152 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 204 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).oe);
		delete (yyvsp[-2].objectfield).os;
		(yyval.code) = (yyvsp[0].code); }
#line 2163 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 226 "engines/director/lingo/lingo-gr.y"
                                                                        {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[-3].code) - (yyvsp[-5].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 2] = end; }
#line 2174 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 237 "engines/director/lingo/lingo-gr.y"
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
#line 2191 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 254 "engines/director/lingo/lingo-gr.y"
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
#line 2208 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 266 "engines/director/lingo/lingo-gr.y"
                                                                      {
		inst list = 0, body = 0, end = 0;
		WRITE_UINT32(&list, (yyvsp[-4].code) - (yyvsp[-6].code));
		WRITE_UINT32(&body, (yyvsp[-2].code) - (yyvsp[-6].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code));
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 1] = list;		/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 2] = 0;		/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 3] = body;		/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 4] = 0;		/* increment */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 5] = end; }
#line 2223 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 277 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_nextRepeat); }
#line 2230 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 279 "engines/director/lingo/lingo-gr.y"
                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2240 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 284 "engines/director/lingo/lingo-gr.y"
                                                          {
		inst end;
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code));
		(*g_lingo->_currentScript)[(yyvsp[-3].code) + 1] = end; }
#line 2249 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 288 "engines/director/lingo/lingo-gr.y"
                                                    {
		inst end;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2258 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 293 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }
#line 2266 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 297 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-6].code) - (yyvsp[-8].code));
		WRITE_UINT32(&else1, (yyvsp[-3].code) - (yyvsp[-8].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-8].code));
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-8].code), (yyvsp[-1].code) - (yyvsp[-8].code), 0); }
#line 2281 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 307 "engines/director/lingo/lingo-gr.y"
                                                                                                              {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-9].code) - (yyvsp[-11].code));
		WRITE_UINT32(&else1, (yyvsp[-6].code) - (yyvsp[-11].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-11].code));
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-11].code), (yyvsp[-1].code) - (yyvsp[-11].code), (yyvsp[-3].code) - (yyvsp[-11].code)); }
#line 2296 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 321 "engines/director/lingo/lingo-gr.y"
                                                        {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-3].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 328 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-8].code) - (yyvsp[-10].code));
		WRITE_UINT32(&else1, (yyvsp[-5].code) - (yyvsp[-10].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-10].code));
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-10].code), (yyvsp[-1].code) - (yyvsp[-10].code), (yyvsp[-3].code) - (yyvsp[-10].code)); }
#line 2322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 338 "engines/director/lingo/lingo-gr.y"
                                                                   {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-4].code) - (yyvsp[-6].code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code));
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code)); }
#line 2337 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 349 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code3(LC::c_repeatwhilecode, STOP, STOP); }
#line 2343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 351 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code3(LC::c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2353 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 357 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); }
#line 2363 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 363 "engines/director/lingo/lingo-gr.y"
                                        {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); }
#line 2374 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 370 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2380 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 372 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2386 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 374 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2392 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 378 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2402 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 384 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2410 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 387 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 390 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2427 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 394 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2436 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 398 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 404 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2451 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 406 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2459 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 409 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2467 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 412 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2475 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 415 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2488 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 423 "engines/director/lingo/lingo-gr.y"
                               {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2499 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 429 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).oe);
		delete (yyvsp[0].objectfield).os; }
#line 2509 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 435 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2515 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 436 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2521 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 437 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2527 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 438 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2533 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 439 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2539 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 440 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2545 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 441 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2551 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 442 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2557 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 443 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2563 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 444 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2569 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 445 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2575 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 446 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2581 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 447 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2587 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 448 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2593 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 449 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2599 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 450 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 451 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 452 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2617 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 453 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); }
#line 2623 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 454 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2629 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 455 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2635 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 456 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_intersects); }
#line 2641 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 457 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_within); }
#line 2647 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 459 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_charOf); }
#line 2653 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 460 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charToOf); }
#line 2659 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 461 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_itemOf); }
#line 2665 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 462 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemToOf); }
#line 2671 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 463 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_lineOf); }
#line 2677 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 464 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineToOf); }
#line 2683 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 465 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_wordOf); }
#line 2689 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 466 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordToOf); }
#line 2695 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 468 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2703 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 473 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_printtop); }
#line 2709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 476 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_exitRepeat); }
#line 2715 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 477 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_procret); }
#line 2721 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 478 "engines/director/lingo/lingo-gr.y"
                  { g_lingo->_indef = kStateInArgs; }
#line 2727 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 478 "engines/director/lingo/lingo-gr.y"
                                                                 { g_lingo->_indef = kStateNone; }
#line 2733 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 479 "engines/director/lingo/lingo-gr.y"
                    { g_lingo->_indef = kStateInArgs; }
#line 2739 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 479 "engines/director/lingo/lingo-gr.y"
                                                                     { g_lingo->_indef = kStateNone; }
#line 2745 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 481 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 484 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2761 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 487 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_open); }
#line 2767 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 488 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 2773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 490 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2782 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 494 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2791 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 499 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2800 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 503 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2809 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 508 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 512 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2827 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 524 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 2833 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 525 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 2839 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 526 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 2845 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 527 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 2854 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 531 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 2863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 535 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 2872 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 543 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_playdone); }
#line 2878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 544 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 2887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 548 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 2896 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 552 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 2905 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 556 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 2911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 556 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 2920 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 586 "engines/director/lingo/lingo-gr.y"
             { g_lingo->_indef = kStateInArgs; }
#line 2926 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 586 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->_currentFactory.clear(); }
#line 2932 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 587 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete (yyvsp[-6].s); }
#line 2943 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 593 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 2949 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 594 "engines/director/lingo/lingo-gr.y"
                  { g_lingo->_indef = kStateInArgs; }
#line 2955 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 595 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete (yyvsp[-6].s); }
#line 2966 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 601 "engines/director/lingo/lingo-gr.y"
                                                                     {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 2981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 611 "engines/director/lingo/lingo-gr.y"
                                                 {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->_indef = kStateNone;
		g_lingo->clearArgStack();
		g_lingo->_ignoreMe = false;
		delete (yyvsp[-5].s); }
#line 2993 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 619 "engines/director/lingo/lingo-gr.y"
         { g_lingo->_indef = kStateInArgs; }
#line 2999 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 619 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.s) = (yyvsp[0].s); g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 3005 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 621 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 3011 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 622 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3017 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 623 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3023 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 624 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-3].narg) + 1; delete (yyvsp[0].s); }
#line 3029 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 627 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3035 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 628 "engines/director/lingo/lingo-gr.y"
                                        { delete (yyvsp[0].s); }
#line 3041 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 630 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArgStore(); g_lingo->_indef = kStateInDef; }
#line 3047 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 632 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 3059 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 640 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 3065 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 641 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3071 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 642 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3077 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 644 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3083 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 645 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3089 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 647 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = (yyvsp[-1].code); }
#line 3095 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 649 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3101 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 650 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3107 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 651 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3113 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 652 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3119 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 654 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3125 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 655 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3131 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 657 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3137 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 658 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3143 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 660 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3152 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 664 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3161 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3165 "engines/director/lingo/lingo-gr.cpp"

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
#line 670 "engines/director/lingo/lingo-gr.y"

