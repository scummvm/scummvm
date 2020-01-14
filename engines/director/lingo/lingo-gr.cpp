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
    BLTINNOARGS = 274,
    BLTINNOARGSORONE = 275,
    BLTINONEARG = 276,
    BLTINARGLIST = 277,
    FBLTIN = 278,
    FBLTINNOARGS = 279,
    FBLTINONEARG = 280,
    FBLTINARGLIST = 281,
    RBLTIN = 282,
    RBLTINONEARG = 283,
    ID = 284,
    STRING = 285,
    HANDLER = 286,
    SYMBOL = 287,
    ENDCLAUSE = 288,
    tPLAYACCEL = 289,
    tMETHOD = 290,
    THEOBJECTFIELD = 291,
    tDOWN = 292,
    tELSE = 293,
    tELSIF = 294,
    tEXIT = 295,
    tGLOBAL = 296,
    tGO = 297,
    tIF = 298,
    tINTO = 299,
    tLOOP = 300,
    tMACRO = 301,
    tMOVIE = 302,
    tNEXT = 303,
    tOF = 304,
    tPREVIOUS = 305,
    tPUT = 306,
    tREPEAT = 307,
    tSET = 308,
    tTHEN = 309,
    tTO = 310,
    tWHEN = 311,
    tWITH = 312,
    tWHILE = 313,
    tNLELSE = 314,
    tFACTORY = 315,
    tOPEN = 316,
    tPLAY = 317,
    tDONE = 318,
    tINSTANCE = 319,
    tGE = 320,
    tLE = 321,
    tEQ = 322,
    tNEQ = 323,
    tAND = 324,
    tOR = 325,
    tNOT = 326,
    tMOD = 327,
    tAFTER = 328,
    tBEFORE = 329,
    tCONCAT = 330,
    tCONTAINS = 331,
    tSTARTS = 332,
    tCHAR = 333,
    tITEM = 334,
    tLINE = 335,
    tWORD = 336,
    tSPRITE = 337,
    tINTERSECTS = 338,
    tWITHIN = 339,
    tTELL = 340,
    tPROPERTY = 341,
    tON = 342,
    tENDIF = 343,
    tENDREPEAT = 344,
    tENDTELL = 345
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

#line 259 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  130
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1513

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  167
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  349

#define YYUNDEFTOK  2
#define YYMAXUTOK   345


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      99,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    98,    93,     2,
     100,   101,    96,    94,   102,    95,     2,    97,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   105,     2,
      91,     2,    92,     2,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   136,   136,   137,   138,   141,   142,   143,   146,   152,
     155,   156,   157,   163,   170,   176,   183,   189,   195,   203,
     204,   205,   208,   209,   210,   211,   214,   215,   220,   231,
     248,   260,   265,   267,   272,   282,   294,   295,   298,   306,
     316,   329,   332,   339,   346,   354,   357,   360,   361,   362,
     365,   371,   375,   378,   381,   385,   389,   393,   396,   397,
     398,   401,   404,   406,   409,   417,   423,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   462,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   478,
     481,   484,   488,   490,   491,   494,   495,   498,   499,   502,
     503,   514,   515,   516,   517,   521,   525,   531,   532,   535,
     536,   540,   544,   548,   548,   579,   579,   586,   587,   587,
     594,   604,   612,   614,   615,   616,   617,   620,   621,   622,
     625,   628,   637,   638,   639,   642,   643,   646,   649,   650,
     651,   652,   655,   656,   659,   660,   663,   667
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNARY", "CASTREF", "VOID", "VAR",
  "POINT", "RECT", "ARRAY", "OBJECT", "REFERENCE", "INT", "ARGC",
  "ARGCNORET", "THEENTITY", "THEENTITYWITHID", "FLOAT", "BLTIN",
  "BLTINNOARGS", "BLTINNOARGSORONE", "BLTINONEARG", "BLTINARGLIST",
  "FBLTIN", "FBLTINNOARGS", "FBLTINONEARG", "FBLTINARGLIST", "RBLTIN",
  "RBLTINONEARG", "ID", "STRING", "HANDLER", "SYMBOL", "ENDCLAUSE",
  "tPLAYACCEL", "tMETHOD", "THEOBJECTFIELD", "tDOWN", "tELSE", "tELSIF",
  "tEXIT", "tGLOBAL", "tGO", "tIF", "tINTO", "tLOOP", "tMACRO", "tMOVIE",
  "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO",
  "tWHEN", "tWITH", "tWHILE", "tNLELSE", "tFACTORY", "tOPEN", "tPLAY",
  "tDONE", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT",
  "tMOD", "tAFTER", "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR",
  "tITEM", "tLINE", "tWORD", "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL",
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
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,    60,    62,    38,    43,    45,    42,    47,    37,    10,
      40,    41,    44,    91,    93,    58
};
# endif

#define YYPACT_NINF (-271)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-6)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     285,   -80,  -271,  -271,   911,  -271,  -271,   911,   911,   911,
    -271,   911,   911,    32,   971,  -271,  -271,  -271,  -271,  -271,
     -28,    12,   811,  -271,    29,   911,    50,     7,    53,    58,
     911,   871,    97,   911,   911,   911,   911,   911,   911,  -271,
     103,   107,   911,   911,   911,    25,     2,  -271,  -271,  -271,
    -271,  -271,   911,    88,   911,   641,   911,  -271,  1403,  -271,
    -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,   -27,   911,
    1403,  1403,  1403,  1403,    55,  1403,    55,  -271,  -271,   911,
    1403,    63,   911,  -271,  -271,  -271,    65,  -271,   911,  -271,
     121,  -271,  1015,  -271,  -271,   964,   140,  -271,    47,    32,
      51,   117,   127,  -271,  1320,  -271,  1015,  -271,  -271,    73,
    -271,    28,  1048,  1081,  1114,  1370,  -271,    77,  -271,  -271,
    -271,  1333,    89,    90,  -271,  -271,    78,    94,    95,  -271,
    -271,   374,  1403,   911,  1403,  -271,  -271,   911,  1403,  -271,
    -271,  1279,   911,   911,   911,   911,   911,   911,   911,   911,
     911,   911,   911,   911,   911,   911,   911,   911,   911,   172,
     911,   964,   911,  1333,    15,   911,    55,   172,   174,  1403,
     911,  -271,  -271,   131,   911,   911,  -271,   911,   911,    60,
     911,   911,   911,  -271,   911,  -271,   175,   911,   911,   911,
     911,   911,   911,   911,   911,   911,   911,   176,  -271,    32,
      32,  -271,    32,    44,  -271,  -271,  1403,   153,  -271,  1403,
     911,  -271,    16,    16,    16,    16,  1416,  1416,  -271,   -26,
      16,    16,    16,    16,   -26,   -16,   -16,  -271,  -271,  -271,
     -85,  1403,  -271,  1403,   -74,  -271,  1403,   172,  -271,  -271,
    1403,  1403,  1403,    16,   911,   911,  1403,    16,  1403,  1403,
    -271,  1403,  1147,  1403,  1180,  1403,  1213,  1403,  1246,  1403,
    1403,  -271,  -271,  -271,  -271,    89,    90,  -271,   552,    -2,
    -271,   154,  1403,   552,   108,   180,   108,   -10,  1403,    16,
     911,   911,   911,   911,  -271,  -271,   122,   157,   911,   552,
     730,   123,   186,  -271,  -271,  -271,   108,  1403,  1403,  1403,
    1403,  -271,   911,  1403,  -271,  -271,  -271,  -271,   463,   552,
    -271,  1403,  -271,   125,   -22,   187,   552,  -271,   552,  -271,
    -271,  -271,   911,   129,  -271,  -271,  -271,   116,   552,   130,
    -271,  1403,  -271,   730,   191,   133,  -271,   552,   169,  -271,
    -271,  -271,   136,  -271,   137,  -271,   552,  -271,  -271
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    52,    64,     0,    53,   108,   111,     0,   152,
      60,     0,   152,     0,    56,    55,    54,   133,   138,    66,
     104,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,   158,     0,     3,    67,    26,
       7,    27,     0,     0,     0,     0,     0,    58,    20,    59,
      21,   101,   102,     6,    45,    19,    57,     4,    56,     0,
      65,   110,   109,   153,   112,    61,    62,    56,    99,   152,
     155,   151,   152,    45,   103,   115,   105,   121,     0,   122,
       0,   123,   124,   126,   135,   100,     0,    41,     0,     0,
       0,     0,     0,   137,   114,   129,   130,   132,   119,   107,
      81,     0,     0,     0,     0,     0,   117,   106,   142,    86,
      87,     0,    55,    54,   159,   162,     0,   160,   161,   164,
       1,     5,    46,     0,    46,    46,    25,     0,    23,    24,
      22,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   143,
     152,     0,     0,   153,     0,     0,   134,   143,     0,   128,
       0,   125,    45,     0,     0,     0,    42,     0,     0,     0,
       0,     0,     0,    50,     0,   131,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    88,     0,
       0,   157,     0,     0,     2,    47,    46,     0,    31,    46,
       0,    47,    77,    78,    75,    76,    79,    80,    72,    83,
      84,    85,    74,    73,    82,    68,    69,    70,    71,   144,
       0,   154,    63,   156,     0,   116,   127,   143,     8,     9,
      10,    11,    15,    13,     0,     0,    14,    12,    18,   113,
     120,    91,     0,    93,     0,    95,     0,    97,     0,    89,
      90,   118,   167,   166,   163,     0,     0,   165,    46,     0,
      47,     0,    33,    46,   150,     0,   150,     0,    16,    17,
       0,     0,     0,     0,    48,    49,     0,     0,     0,    46,
       0,     0,     0,    47,   145,    47,   150,    92,    94,    96,
      98,    28,     0,    46,    36,    46,    32,   146,   141,   139,
      47,    46,    47,    46,     0,   147,   136,    47,    46,    45,
      44,    37,     0,     0,    45,    40,   148,   140,    46,     0,
      47,    46,    34,     0,     0,     0,    29,    46,     0,    46,
     149,    30,     0,    47,     0,    35,    46,    39,    38
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -271,  -271,    96,  -271,  -270,  -271,     6,  -271,  -271,  -271,
    -271,  -271,  -271,   171,  -271,   -82,   -75,   -49,  -271,  -271,
       0,    -4,    56,   177,  -271,  -271,  -271,  -271,   -20,  -271,
    -271,  -271,  -271,  -271,  -271,  -158,  -271,  -229,   178,     3,
    -271,  -271,  -271,  -271,  -271,    31
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    46,    47,    48,    49,   135,   285,    51,   313,   321,
     136,    52,    53,    54,   322,   159,   205,   268,    55,    56,
      57,    58,    59,    60,    86,   117,   109,    61,    93,    62,
      82,    63,   172,    83,    64,   230,   327,   293,    65,   164,
      81,    66,   126,   127,   128,   129
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      70,   167,   130,    71,    72,    73,    50,    75,    73,   234,
      80,   107,    74,    78,   274,    76,   324,   275,    92,    67,
     305,    95,    98,    99,    84,   276,   104,   106,   275,   110,
     111,   112,   113,   114,   115,   287,   100,     2,   119,   120,
     121,    85,     5,   101,     2,   125,   148,   295,   132,     5,
     134,   138,   141,   288,    77,   122,   148,   123,    94,   207,
     208,    77,    15,   339,    16,   161,   325,   310,   155,   156,
     157,   158,   171,   160,   265,   163,   266,   187,    73,   277,
     157,   158,   102,   188,   169,   166,   185,   103,   148,   296,
     237,   149,   275,   142,   143,   144,   145,   146,   147,   179,
     148,   131,   177,   149,   150,   151,   180,    96,    97,   154,
     155,   156,   157,   158,   178,   244,   232,   162,   181,   152,
     153,   154,   155,   156,   157,   158,   108,   245,    45,   206,
     124,   269,   116,   209,   271,    45,   118,    50,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   133,    73,   162,   231,    13,
     238,   233,   273,   319,   320,   165,   236,   168,   170,   176,
     240,   241,   182,   242,   243,   186,   246,   247,   248,   197,
     249,   183,   201,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   286,   199,   200,   202,   203,   291,   262,
     263,   229,   264,   235,   250,   261,   272,   270,   290,   294,
     292,   301,   302,   306,   304,   307,   326,   332,   334,   336,
     340,   289,   341,   343,   345,   347,   137,   204,   312,   239,
     314,     0,   139,   140,   267,     0,   317,   330,   323,     0,
     278,   279,   333,   329,   308,     0,   309,     0,     0,     0,
       0,     0,     0,   335,     0,     0,   338,     0,     0,     0,
       0,   316,   342,   318,   344,     0,     0,     0,   328,     0,
       0,   348,     0,     0,     0,     0,   297,   298,   299,   300,
       0,   337,     0,     0,   303,    -5,     1,     0,     0,     0,
       0,     0,     0,     0,   346,     0,     0,     2,   311,     0,
       3,     4,     5,     0,     6,     7,     8,     9,     0,    10,
      11,    12,     0,    13,    14,    15,     0,    16,   331,    17,
      18,    19,     0,     0,     0,    20,    21,    22,    23,     0,
       0,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,    29,    30,    31,     0,    32,
       0,     0,     0,     0,     0,     0,    33,     0,     0,     0,
       0,     0,     0,    34,    35,    36,    37,    38,     0,     0,
      39,    40,    41,     0,     0,     0,     0,     0,     0,    42,
      43,     0,     0,     0,    -5,    44,     2,     0,    45,     3,
       4,     5,     0,     6,     7,     8,     9,     0,    10,    11,
      12,     0,    13,    14,    15,     0,    16,     0,    17,    18,
      19,     0,     0,     0,    20,    21,    22,    23,     0,     0,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,    29,    30,    31,     0,    32,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,     0,    39,
      40,    41,     0,     0,     0,     0,     0,     0,    42,    43,
       0,     0,     0,     0,    44,     2,     0,    45,     3,     4,
       5,     0,     6,     7,     8,     9,     0,    10,    11,    12,
       0,    13,    14,    15,     0,    16,   315,    17,     0,    19,
       0,     0,     0,    20,    21,    22,    23,     0,     0,     0,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    30,    31,     0,    32,     0,     0,
       0,     0,     0,     0,    33,     0,     0,     0,     0,     0,
       0,    34,    35,    36,    37,    38,     0,     0,    39,    40,
       0,     0,     0,     0,     0,     0,     0,    42,    43,     0,
       0,     0,   284,    44,     2,     0,    45,     3,     4,     5,
       0,     6,     7,     8,     9,     0,    10,    11,    12,     0,
      13,    14,    15,     0,    16,     0,    17,     0,    19,     0,
       0,     0,    20,    21,    22,    23,     0,     0,     0,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    30,    31,     0,    32,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    42,    43,     0,     0,
       0,   284,    44,     2,     0,    45,     3,     4,     5,     0,
       6,     7,     8,     9,     0,    10,    11,    12,     0,    13,
      14,    15,     0,    16,     0,    17,     0,    19,     0,     0,
       0,    20,    21,    22,    23,     0,     0,     0,     0,     0,
       0,     0,    25,     0,    27,     0,     0,     0,     0,     0,
       0,     0,    30,    31,     0,    32,     0,     0,     0,     0,
       0,     0,    33,     0,     0,     0,     0,     0,     0,    34,
      35,    36,    37,    38,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,    42,    43,     0,     0,     0,
       0,    44,     2,     0,    45,     3,     4,     5,     0,     6,
       7,     8,     9,     0,    10,    11,    12,     0,    13,    14,
      15,     0,    16,     0,    17,     0,    19,     0,     0,     0,
      20,    21,    22,     0,     0,     0,     0,     0,     0,     0,
       0,    25,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    30,    31,     0,    32,     0,     0,     0,     0,     0,
       0,    33,     0,     0,     0,     0,     0,     0,    34,    35,
      36,    37,    38,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     2,    42,    43,     3,     4,     5,     0,
      44,     0,     0,    45,     0,    10,    11,    12,     0,    13,
      68,    15,     0,    16,     0,     0,     0,    19,     0,     0,
       0,     0,     0,     0,     0,     0,    87,     0,    88,    89,
      90,    91,    69,     0,    27,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    33,     2,     0,     0,     3,     4,     5,    34,
      35,    36,    37,    38,     0,    10,    11,    12,     0,    13,
      68,    15,     0,    16,     0,    42,    43,    19,     0,     0,
       0,    44,     0,     0,    45,     0,     0,     0,    88,     0,
      90,     0,    69,     2,    27,     0,     3,     4,     5,     0,
       0,     0,     0,     0,   105,    10,    11,    12,     0,    13,
      68,    15,    33,    16,     0,     0,     0,    19,     0,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
       0,     0,    69,     0,    27,    42,    43,     0,     0,     0,
       0,    44,     0,     0,    45,     0,     0,     0,     0,     0,
       0,     0,    33,     2,     0,     0,     3,     4,     5,    34,
      35,    36,    37,    38,     0,    10,    11,    12,     0,    13,
      68,    15,     0,    16,     0,    42,    43,    19,   173,     0,
       0,    44,     0,     0,    45,     0,     0,     0,     0,     0,
       0,     0,    69,     0,    27,     0,     0,     0,     0,   142,
     143,   144,   145,   146,   147,     0,   148,   174,   175,   149,
     150,   151,    33,     0,     0,     0,     0,     0,     0,    34,
      35,    36,    37,    38,     0,   152,   153,   154,   155,   156,
     157,   158,    88,     0,    90,    42,    43,     0,     0,     0,
       0,    79,     0,     0,    45,     0,     0,     0,     0,     0,
     142,   143,   144,   145,   146,   147,     0,   148,     0,     0,
     149,   150,   151,     0,     0,     0,     0,   189,     0,     0,
       0,     0,     0,   190,     0,     0,   152,   153,   154,   155,
     156,   157,   158,   142,   143,   144,   145,   146,   147,     0,
     148,     0,     0,   149,   150,   151,     0,     0,     0,     0,
     191,     0,     0,     0,     0,     0,   192,     0,     0,   152,
     153,   154,   155,   156,   157,   158,   142,   143,   144,   145,
     146,   147,     0,   148,     0,     0,   149,   150,   151,     0,
       0,     0,     0,   193,     0,     0,     0,     0,     0,   194,
       0,     0,   152,   153,   154,   155,   156,   157,   158,   142,
     143,   144,   145,   146,   147,     0,   148,     0,     0,   149,
     150,   151,     0,     0,     0,     0,   280,     0,     0,     0,
       0,     0,     0,     0,     0,   152,   153,   154,   155,   156,
     157,   158,   142,   143,   144,   145,   146,   147,     0,   148,
       0,     0,   149,   150,   151,     0,     0,     0,     0,   281,
       0,     0,     0,     0,     0,     0,     0,     0,   152,   153,
     154,   155,   156,   157,   158,   142,   143,   144,   145,   146,
     147,     0,   148,     0,     0,   149,   150,   151,     0,     0,
       0,     0,   282,     0,     0,     0,     0,     0,     0,     0,
       0,   152,   153,   154,   155,   156,   157,   158,   142,   143,
     144,   145,   146,   147,     0,   148,     0,     0,   149,   150,
     151,     0,     0,     0,     0,   283,     0,     0,     0,     0,
       0,     0,     0,     0,   152,   153,   154,   155,   156,   157,
     158,   142,   143,   144,   145,   146,   147,     0,   148,     0,
       0,   149,   150,   151,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   210,     0,     0,   152,   153,   154,
     155,   156,   157,   158,   142,   143,   144,   145,   146,   147,
       0,   148,     0,     0,   149,   150,   151,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     152,   153,   154,   155,   156,   157,   158,   184,   211,     0,
       0,     0,     0,     0,     0,   142,   143,   144,   145,   146,
     147,     0,   148,     0,     0,   149,   150,   151,   142,   143,
     144,   145,   146,   147,     0,   148,     0,     0,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,     0,     0,
       0,     0,     0,     0,   152,   153,   154,   155,   156,   157,
     158,     0,     0,     0,   198,   142,   143,   144,   145,   146,
     147,     0,   148,     0,     0,   149,   150,   151,     0,     0,
       0,     0,     0,   195,   196,     0,     0,     0,     0,     0,
       0,   152,   153,   154,   155,   156,   157,   158,   142,   143,
     144,   145,   146,   147,     0,   148,     0,     0,   149,   150,
     151,   142,   143,   144,   145,     0,     0,     0,   148,     0,
       0,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,     0,     0,     0,     0,     0,     0,   152,   153,   154,
     155,   156,   157,   158
};

static const yytype_int16 yycheck[] =
{
       4,    83,     0,     7,     8,     9,     0,    11,    12,   167,
      14,    31,     9,    13,    99,    12,    38,   102,    22,    99,
     290,    25,    15,    16,    52,    99,    30,    31,   102,    33,
      34,    35,    36,    37,    38,    37,    29,    12,    42,    43,
      44,    29,    17,    36,    12,    45,    72,   276,    52,    17,
      54,    55,    56,    55,    29,    30,    72,    32,    29,   134,
     135,    29,    30,   333,    32,    69,    88,   296,    94,    95,
      96,    97,    92,   100,    30,    79,    32,    49,    82,   237,
      96,    97,    29,    55,    88,    82,   106,    29,    72,    99,
     172,    75,   102,    65,    66,    67,    68,    69,    70,    99,
      72,    99,    55,    75,    76,    77,    55,    57,    58,    93,
      94,    95,    96,    97,    67,    55,   101,   102,    67,    91,
      92,    93,    94,    95,    96,    97,    29,    67,   103,   133,
     105,   206,    29,   137,   209,   103,    29,   131,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,    67,   160,   102,   162,    28,
      29,   165,   211,    38,    39,   102,   170,   102,    47,    29,
     174,   175,    55,   177,   178,   102,   180,   181,   182,   102,
     184,    54,   104,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   268,   105,   105,   102,   102,   273,   199,
     200,    29,   202,    29,    29,    29,   210,    54,    54,    29,
     102,    89,    55,    90,   289,    29,    29,    88,   102,    89,
      29,   270,    89,    54,    88,    88,    55,   131,   303,   173,
     305,    -1,    55,    55,   203,    -1,   311,   319,   313,    -1,
     244,   245,   324,   318,   293,    -1,   295,    -1,    -1,    -1,
      -1,    -1,    -1,   328,    -1,    -1,   331,    -1,    -1,    -1,
      -1,   310,   337,   312,   339,    -1,    -1,    -1,   317,    -1,
      -1,   346,    -1,    -1,    -1,    -1,   280,   281,   282,   283,
      -1,   330,    -1,    -1,   288,     0,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   343,    -1,    -1,    12,   302,    -1,
      15,    16,    17,    -1,    19,    20,    21,    22,    -1,    24,
      25,    26,    -1,    28,    29,    30,    -1,    32,   322,    34,
      35,    36,    -1,    -1,    -1,    40,    41,    42,    43,    -1,
      -1,    46,    -1,    -1,    -1,    -1,    51,    52,    53,    -1,
      -1,    56,    -1,    -1,    -1,    60,    61,    62,    -1,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    -1,    -1,
      85,    86,    87,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    -1,    -1,    -1,    99,   100,    12,    -1,   103,    15,
      16,    17,    -1,    19,    20,    21,    22,    -1,    24,    25,
      26,    -1,    28,    29,    30,    -1,    32,    -1,    34,    35,
      36,    -1,    -1,    -1,    40,    41,    42,    43,    -1,    -1,
      46,    -1,    -1,    -1,    -1,    51,    52,    53,    -1,    -1,
      56,    -1,    -1,    -1,    60,    61,    62,    -1,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    -1,    -1,    85,
      86,    87,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,   100,    12,    -1,   103,    15,    16,
      17,    -1,    19,    20,    21,    22,    -1,    24,    25,    26,
      -1,    28,    29,    30,    -1,    32,    33,    34,    -1,    36,
      -1,    -1,    -1,    40,    41,    42,    43,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    52,    53,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    61,    62,    -1,    64,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    -1,    -1,    85,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    -1,
      -1,    -1,    99,   100,    12,    -1,   103,    15,    16,    17,
      -1,    19,    20,    21,    22,    -1,    24,    25,    26,    -1,
      28,    29,    30,    -1,    32,    -1,    34,    -1,    36,    -1,
      -1,    -1,    40,    41,    42,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    52,    53,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    -1,    -1,    85,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    -1,    -1,
      -1,    99,   100,    12,    -1,   103,    15,    16,    17,    -1,
      19,    20,    21,    22,    -1,    24,    25,    26,    -1,    28,
      29,    30,    -1,    32,    -1,    34,    -1,    36,    -1,    -1,
      -1,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    61,    62,    -1,    64,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,   100,    12,    -1,   103,    15,    16,    17,    -1,    19,
      20,    21,    22,    -1,    24,    25,    26,    -1,    28,    29,
      30,    -1,    32,    -1,    34,    -1,    36,    -1,    -1,    -1,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    62,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    12,    94,    95,    15,    16,    17,    -1,
     100,    -1,    -1,   103,    -1,    24,    25,    26,    -1,    28,
      29,    30,    -1,    32,    -1,    -1,    -1,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    12,    -1,    -1,    15,    16,    17,    78,
      79,    80,    81,    82,    -1,    24,    25,    26,    -1,    28,
      29,    30,    -1,    32,    -1,    94,    95,    36,    -1,    -1,
      -1,   100,    -1,    -1,   103,    -1,    -1,    -1,    47,    -1,
      49,    -1,    51,    12,    53,    -1,    15,    16,    17,    -1,
      -1,    -1,    -1,    -1,    63,    24,    25,    26,    -1,    28,
      29,    30,    71,    32,    -1,    -1,    -1,    36,    -1,    78,
      79,    80,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    53,    94,    95,    -1,    -1,    -1,
      -1,   100,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    12,    -1,    -1,    15,    16,    17,    78,
      79,    80,    81,    82,    -1,    24,    25,    26,    -1,    28,
      29,    30,    -1,    32,    -1,    94,    95,    36,    44,    -1,
      -1,   100,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    53,    -1,    -1,    -1,    -1,    65,
      66,    67,    68,    69,    70,    -1,    72,    73,    74,    75,
      76,    77,    71,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    -1,    91,    92,    93,    94,    95,
      96,    97,    47,    -1,    49,    94,    95,    -1,    -1,    -1,
      -1,   100,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      65,    66,    67,    68,    69,    70,    -1,    72,    -1,    -1,
      75,    76,    77,    -1,    -1,    -1,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    91,    92,    93,    94,
      95,    96,    97,    65,    66,    67,    68,    69,    70,    -1,
      72,    -1,    -1,    75,    76,    77,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    91,
      92,    93,    94,    95,    96,    97,    65,    66,    67,    68,
      69,    70,    -1,    72,    -1,    -1,    75,    76,    77,    -1,
      -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    91,    92,    93,    94,    95,    96,    97,    65,
      66,    67,    68,    69,    70,    -1,    72,    -1,    -1,    75,
      76,    77,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,
      96,    97,    65,    66,    67,    68,    69,    70,    -1,    72,
      -1,    -1,    75,    76,    77,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    97,    65,    66,    67,    68,    69,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    65,    66,
      67,    68,    69,    70,    -1,    72,    -1,    -1,    75,    76,
      77,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    65,    66,    67,    68,    69,    70,    -1,    72,    -1,
      -1,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    -1,    91,    92,    93,
      94,    95,    96,    97,    65,    66,    67,    68,    69,    70,
      -1,    72,    -1,    -1,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    97,    57,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    69,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    65,    66,
      67,    68,    69,    70,    -1,    72,    -1,    -1,    75,    76,
      77,    91,    92,    93,    94,    95,    96,    97,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      97,    -1,    -1,    -1,   101,    65,    66,    67,    68,    69,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    96,    97,    65,    66,
      67,    68,    69,    70,    -1,    72,    -1,    -1,    75,    76,
      77,    65,    66,    67,    68,    -1,    -1,    -1,    72,    -1,
      -1,    75,    76,    77,    91,    92,    93,    94,    95,    96,
      97,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,
      94,    95,    96,    97
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    15,    16,    17,    19,    20,    21,    22,
      24,    25,    26,    28,    29,    30,    32,    34,    35,    36,
      40,    41,    42,    43,    46,    51,    52,    53,    56,    60,
      61,    62,    64,    71,    78,    79,    80,    81,    82,    85,
      86,    87,    94,    95,   100,   103,   107,   108,   109,   110,
     112,   113,   117,   118,   119,   124,   125,   126,   127,   128,
     129,   133,   135,   137,   140,   144,   147,    99,    29,    51,
     127,   127,   127,   127,   145,   127,   145,    29,   126,   100,
     127,   146,   136,   139,    52,    29,   130,    45,    47,    48,
      49,    50,   127,   134,    29,   127,    57,    58,    15,    16,
      29,    36,    29,    29,   127,    63,   127,   134,    29,   132,
     127,   127,   127,   127,   127,   127,    29,   131,    29,   127,
     127,   127,    30,    32,   105,   126,   148,   149,   150,   151,
       0,    99,   127,    67,   127,   111,   116,   119,   127,   129,
     144,   127,    65,    66,    67,    68,    69,    70,    72,    75,
      76,    77,    91,    92,    93,    94,    95,    96,    97,   121,
     100,   127,   102,   127,   145,   102,   145,   121,   102,   127,
      47,   134,   138,    44,    73,    74,    29,    55,    67,   126,
      55,    67,    55,    54,    57,   134,   102,    49,    55,    49,
      55,    49,    55,    49,    55,    83,    84,   102,   101,   105,
     105,   104,   102,   102,   108,   122,   127,   122,   122,   127,
      55,    99,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,    29,
     141,   127,   101,   127,   141,    29,   127,   121,    29,   128,
     127,   127,   127,   127,    55,    67,   127,   127,   127,   127,
      29,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,    29,   126,   126,   126,    30,    32,   151,   123,   122,
      54,   122,   127,   123,    99,   102,    99,   141,   127,   127,
      49,    49,    49,    49,    99,   112,   122,    37,    55,   123,
      54,   122,   102,   143,    29,   143,    99,   127,   127,   127,
     127,    89,    55,   127,   122,   110,    90,    29,   123,   123,
     143,   127,   122,   114,   122,    33,   123,   122,   123,    38,
      39,   115,   120,   122,    38,    88,    29,   142,   123,   122,
     121,   127,    88,   121,   102,   122,    89,   123,   122,   110,
      29,    89,   122,    54,   122,    88,   123,    88,   122
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   106,   107,   107,   107,   108,   108,   108,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   110,
     110,   110,   111,   111,   111,   111,   112,   112,   112,   112,
     112,   112,   112,   112,   113,   113,   114,   114,   115,   116,
     116,   117,   118,   119,   120,   121,   122,   123,   123,   123,
     124,   125,   126,   126,   126,   126,   126,   126,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   128,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   130,   130,   131,   131,   132,
     132,   133,   133,   133,   133,   133,   133,   134,   134,   135,
     135,   135,   135,   136,   135,   138,   137,   137,   139,   137,
     137,   137,   140,   141,   141,   141,   141,   142,   142,   142,
     143,   144,   145,   145,   145,   146,   146,   147,   148,   148,
     148,   148,   149,   149,   150,   150,   151,   151
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
       1,     2,     2,     4,     1,     2,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     3,     3,     3,     2,     2,     3,     4,
       4,     4,     6,     4,     6,     4,     6,     4,     6,     2,
       2,     1,     1,     2,     1,     2,     2,     2,     1,     2,
       2,     1,     2,     4,     2,     1,     3,     1,     3,     1,
       3,     2,     2,     2,     2,     3,     2,     3,     2,     2,
       2,     3,     2,     0,     3,     0,     8,     2,     0,     7,
       8,     6,     2,     0,     1,     3,     4,     0,     1,     3,
       0,     2,     0,     1,     3,     1,     3,     3,     0,     1,
       1,     1,     1,     3,     1,     3,     3,     3
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
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1661 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 19: /* BLTINNOARGS  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1667 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 20: /* BLTINNOARGSORONE  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1673 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 21: /* BLTINONEARG  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1679 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 22: /* BLTINARGLIST  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1685 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* FBLTIN  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1691 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* FBLTINNOARGS  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1697 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* FBLTINONEARG  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1703 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* FBLTINARGLIST  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1709 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* RBLTIN  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1715 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* RBLTINONEARG  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1721 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* ID  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1727 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 30: /* STRING  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1733 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 31: /* HANDLER  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1739 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 32: /* SYMBOL  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1745 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 33: /* ENDCLAUSE  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1751 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 34: /* tPLAYACCEL  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1757 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 35: /* tMETHOD  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1763 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 36: /* THEOBJECTFIELD  */
#line 132 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).s; }
#line 1769 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 140: /* on  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1775 "engines/director/lingo/lingo-gr.cpp"
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
#line 138 "engines/director/lingo/lingo-gr.y"
                                { yyerrok; }
#line 2045 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 146 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2056 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 152 "engines/director/lingo/lingo-gr.y"
                                                {
			g_lingo->code1(LC::c_assign);
			(yyval.code) = (yyvsp[-2].code); }
#line 2064 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 155 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_after); }
#line 2070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 156 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_before); }
#line 2076 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 157 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2087 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 163 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2099 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 170 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2110 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 176 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2122 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 183 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 189 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2144 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 195 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).s->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).e);
		delete (yyvsp[-2].objectfield).s;
		(yyval.code) = (yyvsp[0].code); }
#line 2155 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28:
#line 220 "engines/director/lingo/lingo-gr.y"
                                                                        {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[-3].code) - (yyvsp[-5].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 2] = end; }
#line 2166 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 231 "engines/director/lingo/lingo-gr.y"
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
#line 2183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 248 "engines/director/lingo/lingo-gr.y"
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
#line 2200 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 260 "engines/director/lingo/lingo-gr.y"
                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2210 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 265 "engines/director/lingo/lingo-gr.y"
                                               {
			warning("STUB: TELL is not implemented"); }
#line 2217 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 267 "engines/director/lingo/lingo-gr.y"
                             {
			warning("STUB: TELL is not implemented");
		}
#line 2225 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 272 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-6].code) - (yyvsp[-8].code));
		WRITE_UINT32(&else1, (yyvsp[-3].code) - (yyvsp[-8].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-8].code));
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-8].code), (yyvsp[-1].code) - (yyvsp[-8].code), 0); }
#line 2240 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 282 "engines/director/lingo/lingo-gr.y"
                                                                                                              {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-9].code) - (yyvsp[-11].code));
		WRITE_UINT32(&else1, (yyvsp[-6].code) - (yyvsp[-11].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-11].code));
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-11].code), (yyvsp[-1].code) - (yyvsp[-11].code), (yyvsp[-3].code) - (yyvsp[-11].code)); }
#line 2255 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 298 "engines/director/lingo/lingo-gr.y"
                                                        {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-3].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2266 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 306 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-8].code) - (yyvsp[-10].code));
		WRITE_UINT32(&else1, (yyvsp[-5].code) - (yyvsp[-10].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-10].code));
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-10].code), (yyvsp[-1].code) - (yyvsp[-10].code), (yyvsp[-3].code) - (yyvsp[-10].code)); }
#line 2281 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 316 "engines/director/lingo/lingo-gr.y"
                                                                   {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-4].code) - (yyvsp[-6].code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code));
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code)); }
#line 2296 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 329 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code3(LC::c_repeatwhilecode, STOP, STOP); }
#line 2302 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 332 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code3(LC::c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2312 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 339 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); }
#line 2322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 346 "engines/director/lingo/lingo-gr.y"
                                        {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); }
#line 2333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 357 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 360 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2351 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 365 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2361 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 371 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }
#line 2369 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 375 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2377 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 378 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2385 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 381 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2394 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 385 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2403 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 389 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2412 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 396 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 398 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->codeFunc((yyvsp[0].s), 0);
		delete (yyvsp[0].s); }
#line 2426 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 401 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2434 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 404 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 406 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2449 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 409 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2462 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 417 "engines/director/lingo/lingo-gr.y"
                               {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 423 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).s->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).e);
		delete (yyvsp[0].objectfield).s; }
#line 2483 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 429 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2489 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 430 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 431 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2501 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 432 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2507 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 433 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2513 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 434 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2519 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 435 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 436 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2531 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 437 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2537 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 438 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2543 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 439 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2549 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 440 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2555 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 441 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2561 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 442 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2567 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 443 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2573 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 444 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2579 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 445 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2585 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 446 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2591 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 447 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); }
#line 2597 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 448 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2603 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 449 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2609 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 450 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_intersects); }
#line 2615 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 451 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_within); }
#line 2621 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 452 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_charOf); }
#line 2627 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 453 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charToOf); }
#line 2633 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 454 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_itemOf); }
#line 2639 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 455 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemToOf); }
#line 2645 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 456 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_lineOf); }
#line 2651 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 457 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineToOf); }
#line 2657 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 458 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_wordOf); }
#line 2663 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 459 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordToOf); }
#line 2669 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 462 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2677 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 467 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_printtop); }
#line 2683 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 470 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_exitRepeat); }
#line 2689 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 471 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_procret); }
#line 2695 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 475 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->codeFunc((yyvsp[0].s), 0);
		delete (yyvsp[0].s); }
#line 2703 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 478 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 481 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2719 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 484 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_voidpush);
		g_lingo->codeFunc((yyvsp[0].s), 1);
		delete (yyvsp[0].s); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 488 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 490 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_open); }
#line 2741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 491 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 2747 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 494 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_global); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 495 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_global); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 498 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_property); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2765 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 499 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_property); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2771 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 502 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_instance); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2777 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 503 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_instance); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 514 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 2789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 515 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 2795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 516 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 2801 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 517 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 2810 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 521 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 2819 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 525 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 2828 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 535 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_playdone); }
#line 2834 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 536 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 2843 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 540 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 2852 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 544 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 2861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 548 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 2867 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 548 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 2876 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 579 "engines/director/lingo/lingo-gr.y"
                { g_lingo->_indef = kStateInArgs; g_lingo->_currentFactory.clear(); }
#line 2882 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 580 "engines/director/lingo/lingo-gr.y"
                                                                {
			g_lingo->code1(LC::c_procret);
			g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
			g_lingo->clearArgStack();
			g_lingo->_indef = kStateNone;
			delete (yyvsp[-6].s); }
#line 2893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 586 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 2899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 587 "engines/director/lingo/lingo-gr.y"
                  { g_lingo->_indef = kStateInArgs; }
#line 2905 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 588 "engines/director/lingo/lingo-gr.y"
                                                                {
			g_lingo->code1(LC::c_procret);
			g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->clearArgStack();
			g_lingo->_indef = kStateNone;
			delete (yyvsp[-6].s); }
#line 2916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 594 "engines/director/lingo/lingo-gr.y"
                                                                     {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 2931 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 604 "engines/director/lingo/lingo-gr.y"
                                                 {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->_indef = kStateNone;
		g_lingo->clearArgStack();
		g_lingo->_ignoreMe = false;
		delete (yyvsp[-5].s); }
#line 2943 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 612 "engines/director/lingo/lingo-gr.y"
            { (yyval.s) = (yyvsp[0].s); g_lingo->_indef = kStateInArgs; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 2949 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 614 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 2955 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 615 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 2961 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 616 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 2967 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 617 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-3].narg) + 1; delete (yyvsp[0].s); }
#line 2973 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 621 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2979 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 622 "engines/director/lingo/lingo-gr.y"
                                        { delete (yyvsp[0].s); }
#line 2985 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 625 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArgStore(); g_lingo->_indef = kStateInDef; }
#line 2991 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 628 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 3003 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 637 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 3009 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 638 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3015 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 639 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3021 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 642 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3027 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 643 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3033 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 646 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = (yyvsp[-1].code); }
#line 3039 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 649 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3045 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 650 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3051 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 651 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3057 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 652 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3063 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 655 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3069 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 656 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3075 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 659 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3081 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 660 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3087 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 663 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3096 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 667 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3105 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3109 "engines/director/lingo/lingo-gr.cpp"

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
#line 674 "engines/director/lingo/lingo-gr.y"

