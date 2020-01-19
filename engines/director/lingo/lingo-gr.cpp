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
    THEMENUITEMENTITY = 272,
    FLOAT = 273,
    BLTIN = 274,
    FBLTIN = 275,
    RBLTIN = 276,
    ID = 277,
    STRING = 278,
    HANDLER = 279,
    SYMBOL = 280,
    ENDCLAUSE = 281,
    tPLAYACCEL = 282,
    tMETHOD = 283,
    THEOBJECTFIELD = 284,
    tDOWN = 285,
    tELSE = 286,
    tELSIF = 287,
    tEXIT = 288,
    tGLOBAL = 289,
    tGO = 290,
    tIF = 291,
    tIN = 292,
    tINTO = 293,
    tLOOP = 294,
    tMACRO = 295,
    tMOVIE = 296,
    tNEXT = 297,
    tOF = 298,
    tPREVIOUS = 299,
    tPUT = 300,
    tREPEAT = 301,
    tSET = 302,
    tTHEN = 303,
    tTO = 304,
    tWHEN = 305,
    tWITH = 306,
    tWHILE = 307,
    tNLELSE = 308,
    tFACTORY = 309,
    tOPEN = 310,
    tPLAY = 311,
    tDONE = 312,
    tINSTANCE = 313,
    tGE = 314,
    tLE = 315,
    tEQ = 316,
    tNEQ = 317,
    tAND = 318,
    tOR = 319,
    tNOT = 320,
    tMOD = 321,
    tAFTER = 322,
    tBEFORE = 323,
    tCONCAT = 324,
    tCONTAINS = 325,
    tSTARTS = 326,
    tCHAR = 327,
    tITEM = 328,
    tLINE = 329,
    tWORD = 330,
    tSPRITE = 331,
    tINTERSECTS = 332,
    tWITHIN = 333,
    tTELL = 334,
    tPROPERTY = 335,
    tON = 336,
    tENDIF = 337,
    tENDREPEAT = 338,
    tENDTELL = 339
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

#line 253 "engines/director/lingo/lingo-gr.cpp"

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
#define YYLAST   1685

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  100
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  51
/* YYNRULES -- Number of rules.  */
#define YYNRULES  171
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  369

#define YYUNDEFTOK  2
#define YYMAXUTOK   339


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      93,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    92,    87,     2,
      94,    95,    90,    88,    96,    89,     2,    91,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    99,     2,
      85,     2,    86,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    97,     2,    98,     2,     2,     2,     2,     2,     2,
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
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   135,   135,   136,   137,   139,   140,   141,   143,   149,
     152,   153,   154,   160,   167,   173,   180,   186,   193,   203,
     210,   211,   212,   214,   215,   216,   217,   219,   220,   225,
     236,   253,   265,   276,   278,   283,   287,   292,   296,   306,
     317,   318,   320,   327,   337,   348,   350,   356,   362,   369,
     371,   373,   374,   375,   377,   383,   386,   389,   393,   397,
     401,   403,   404,   405,   408,   411,   414,   422,   428,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   458,   459,   460,   461,   462,   463,   464,
     465,   467,   470,   472,   473,   474,   475,   476,   477,   477,
     478,   478,   479,   480,   483,   486,   487,   489,   493,   498,
     502,   507,   511,   523,   524,   525,   526,   530,   534,   539,
     540,   542,   543,   547,   551,   555,   555,   585,   585,   585,
     592,   593,   593,   600,   610,   618,   618,   620,   621,   622,
     623,   625,   626,   627,   629,   631,   639,   640,   641,   643,
     644,   646,   648,   649,   650,   651,   653,   654,   656,   657,
     659,   663
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNARY", "CASTREF", "VOID", "VAR",
  "POINT", "RECT", "ARRAY", "OBJECT", "REFERENCE", "INT", "ARGC",
  "ARGCNORET", "THEENTITY", "THEENTITYWITHID", "THEMENUITEMENTITY",
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
     335,   336,   337,   338,   339,    60,    62,    38,    43,    45,
      42,    47,    37,    10,    40,    41,    44,    91,    93,    58
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
     339,   -52,  -269,  -269,  1023,  -269,  1057,  1137,    31,  1171,
    -269,  -269,  -269,  -269,  -269,   -27,  -269,   905,  -269,  -269,
      37,  1023,     7,   102,    29,    55,  1023,   943,    74,  1023,
    1023,  1023,  1023,  1023,  1023,  1023,  -269,  -269,  1023,  1023,
    1023,   419,     8,  -269,  -269,  -269,  -269,  -269,  1023,   -19,
    1023,   745,  -269,  1581,  -269,  -269,  -269,  -269,  -269,  -269,
    -269,  -269,  -269,  -269,    13,  1023,  1581,  1023,  1581,    16,
    1023,    16,  -269,  -269,  1023,  1581,    18,  1023,  -269,  -269,
      91,  -269,  1023,  -269,    75,  -269,   411,  -269,    93,  -269,
     167,    98,  -269,   -29,    31,    31,    -1,    80,    84,  -269,
    1498,  -269,   411,  -269,  -269,    27,  -269,  1192,  1226,  1259,
    1292,  1548,  1457,   129,   131,  -269,  -269,  1511,    56,    58,
    -269,  1581,    60,    64,    65,  -269,  -269,   505,  1581,  1023,
    1023,  1581,  -269,  -269,  1023,  1581,  -269,  -269,  1023,  1023,
    1023,  1023,  1023,  1023,  1023,  1023,  1023,  1023,  1023,  1023,
    1023,  1023,  1023,  1023,  1023,   132,  1023,   167,  1511,    -2,
    1023,     2,     4,  1023,    16,   132,  -269,    66,  1581,  1023,
    -269,  -269,    17,  1023,  1023,  -269,  1023,  1023,     1,   120,
    1023,  1023,  1023,  -269,  1023,  -269,   142,  1023,  1023,  1023,
    1023,  1023,  1023,  1023,  1023,  1023,  1023,  -269,  -269,  -269,
      70,  -269,  -269,    31,    31,  -269,  1023,    51,  -269,  -269,
    1581,  1581,   119,  -269,  1581,    15,    15,    15,    15,  1594,
    1594,  -269,    20,    15,    15,    15,    15,    20,   -26,   -26,
    -269,  -269,  -269,   -72,  -269,  1581,  -269,  -269,  1581,   -41,
     149,  1581,  -269,  -269,  -269,  1581,  1581,  1581,    15,  1023,
    1023,   152,  1581,    15,  1581,  1581,  -269,  1581,  1325,  1581,
    1358,  1581,  1391,  1581,  1424,  1581,  1581,   825,  -269,   153,
    -269,  -269,  1581,    56,    58,  -269,   665,  -269,   -16,  -269,
     133,    83,   160,    83,  -269,   132,  1581,    15,    31,  1023,
    1023,  1023,  1023,  -269,   665,  -269,  -269,  -269,   110,   665,
     145,  1023,   665,   825,   173,  -269,  -269,  -269,   -25,   150,
    1581,  1581,  1581,  1581,  -269,   114,  -269,   123,  1023,  1581,
    -269,  -269,  -269,   585,   665,    83,  1023,  -269,  -269,  1581,
    -269,    48,   -15,   181,  -269,  1581,  -269,   665,  -269,  -269,
    -269,  1023,   125,  -269,  -269,  -269,   112,   665,   665,   126,
    -269,  1581,  -269,   825,   188,   130,  -269,   665,   164,  -269,
    -269,  -269,   134,  -269,   135,  -269,   665,  -269,  -269
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
    -269,  -269,    87,  -269,  -258,  -269,     6,    21,  -269,  -269,
    -269,  -269,  -269,  -269,   170,  -269,   -74,   -10,   -57,  -269,
      -7,    -4,  -269,    43,   172,  -269,  -269,  -269,  -269,  -269,
    -269,   -17,  -269,  -269,  -269,  -269,  -269,  -269,  -269,  -269,
    -158,  -269,  -268,   174,     5,  -269,  -269,  -269,  -269,  -269,
      11
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
     103,    69,    71,    86,   300,   307,   343,    90,   129,    79,
     176,   281,   100,   102,   282,   106,   107,   108,   109,   110,
     111,   112,   177,   301,   115,   116,   117,   121,     8,   243,
     144,    63,   130,     2,   128,   321,   131,   135,   180,     5,
     249,    98,   283,    72,    10,   282,    11,   334,    91,    92,
     181,   157,   250,   158,   153,   154,   158,   344,   325,   170,
     158,   282,   159,    68,   273,   161,   274,    99,   168,   338,
     339,   144,   164,    89,   145,   185,   144,   178,   179,    30,
      31,    32,    33,   234,   160,   359,   104,   236,   160,   237,
     160,   127,   150,   151,   152,   153,   154,   156,   151,   152,
     153,   154,   160,   166,   163,   171,   169,    93,    94,    95,
     175,   212,   213,   186,    96,   210,   211,   308,    41,   182,
     214,    97,   183,    46,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   199,    68,   201,   232,   203,   235,   204,   205,   238,
     206,   207,   240,   251,   256,   241,   269,   279,   285,   245,
     246,   284,   247,   248,   288,   295,   252,   253,   254,   304,
     255,   303,   306,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   316,   318,   322,   270,   271,   327,   326,
     277,   278,   272,   345,   280,   172,   328,   352,   354,   356,
     360,   294,   363,   361,   208,   244,   365,   367,   275,   268,
     299,   134,   302,   136,     0,   137,   138,   139,   140,   141,
     142,   143,     0,   144,   173,   174,   145,   146,   147,     0,
       0,     0,     0,     0,     0,   286,   287,     0,   323,     0,
     324,     0,   148,   149,   150,   151,   152,   153,   154,     0,
       0,     0,     0,     0,   350,     0,   298,     0,     0,   353,
       0,     0,     0,   337,     0,     0,     0,   347,     0,   348,
       0,   309,     0,   314,   315,   310,   311,   312,   313,   317,
       0,     0,   320,   357,     0,     0,     0,   319,     0,     0,
       0,     0,     0,     0,     0,     0,   366,     0,     0,   330,
       0,   332,     0,     0,   329,     0,     0,     0,     0,   336,
       0,   342,   335,     0,     0,     0,     0,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   351,   355,    -5,
       1,   358,     0,     0,     0,     0,     0,   362,     0,   364,
       0,     2,     0,     0,     3,     4,   368,     5,     6,     7,
       8,     9,    10,     0,    11,     0,    12,    13,    14,     0,
       0,     0,    15,    16,    17,    18,     0,     0,     0,    19,
       0,    20,     0,     0,    21,    22,    23,     0,     0,    24,
       0,     0,     0,    25,    26,    27,     0,    28,     0,     0,
       0,     0,     0,     0,    29,     0,     0,     0,     0,     0,
       0,    30,    31,    32,    33,    34,     0,     0,    35,    36,
      37,     0,     0,     0,     0,     0,     0,    38,    39,     0,
       0,     2,    -5,    40,     3,     4,    41,     5,     0,     7,
       8,    64,   118,     0,   119,     0,     0,     0,    14,     0,
       0,     0,    82,     0,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    65,     0,    23,     0,     0,     0,
     138,   139,   140,   141,   142,   143,     0,   144,     0,     0,
     145,   146,   147,     0,    29,     0,     0,     0,     0,     0,
       0,    30,    31,    32,    33,    34,   148,   149,   150,   151,
     152,   153,   154,     0,     0,     0,     0,    38,    39,     0,
       0,     0,     0,    40,     0,     0,    41,     2,   120,     0,
       3,     4,     0,     5,     6,     7,     8,     9,    10,     0,
      11,     0,    12,    13,    14,     0,     0,     0,    15,    16,
      17,    18,     0,     0,     0,    19,     0,    20,     0,     0,
      21,    22,    23,     0,     0,    24,     0,     0,     0,    25,
      26,    27,     0,    28,     0,     0,     0,     0,     0,     0,
      29,     0,     0,     0,     0,     0,     0,    30,    31,    32,
      33,    34,     0,     0,    35,    36,    37,     0,     0,     0,
       0,     0,     0,    38,    39,     0,     0,     2,     0,    40,
       3,     4,    41,     5,     6,     7,     8,     9,    10,     0,
      11,   333,    12,     0,    14,     0,     0,     0,    15,    16,
      17,    18,     0,     0,     0,     0,     0,    20,     0,     0,
      21,    22,    23,     0,     0,    24,     0,     0,     0,     0,
      26,    27,     0,    28,     0,     0,     0,     0,     0,     0,
      29,     0,     0,     0,     0,     0,     0,    30,    31,    32,
      33,    34,     0,     0,    35,    36,     0,     0,     0,     0,
       0,     0,     0,    38,    39,     0,     0,     2,   296,    40,
       3,     4,    41,     5,     6,     7,     8,     9,    10,     0,
      11,     0,    12,     0,    14,     0,     0,     0,    15,    16,
      17,    18,     0,     0,     0,     0,     0,    20,     0,     0,
      21,    22,    23,     0,     0,    24,     0,     0,     0,     0,
      26,    27,     0,    28,     0,     0,     0,     0,     0,     0,
      29,     0,     0,     0,     0,     0,     0,    30,    31,    32,
      33,    34,     0,     0,    35,    36,     0,     0,     0,     0,
       0,     0,     0,    38,    39,     0,     0,     2,   296,    40,
       3,     4,    41,     5,     6,     7,     8,     9,    10,     0,
      11,     0,    12,     0,    14,     0,     0,     0,    15,    16,
      17,    18,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,    23,     0,     0,     0,     0,     0,     0,     0,
      26,    27,     0,    28,     0,     0,     0,     0,     0,     0,
      29,     0,     0,     0,     0,     0,     0,    30,    31,    32,
      33,    34,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,    38,    39,     0,     0,     2,     0,    40,
       3,     4,    41,     5,     6,     7,     8,     9,    10,     0,
      11,     0,    12,     0,    14,     0,     0,     0,    15,    16,
      17,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,    23,     0,     0,     0,     0,     0,     0,     0,
      26,    27,     0,    28,     0,     0,     0,     0,     0,     0,
      29,     0,     0,     0,     0,     0,     0,    30,    31,    32,
      33,    34,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,    38,    39,     0,     0,     2,     0,    40,
       3,     4,    41,     5,     0,     7,     8,    64,    10,     0,
      11,     0,     0,     0,    14,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    81,     0,    82,    83,    84,    85,
      65,     0,    23,     0,     0,     2,     0,     0,     3,     4,
       0,     5,     0,     7,     8,    64,    10,     0,    11,     0,
      29,     0,    14,     0,     0,     0,     0,    30,    31,    32,
      33,    34,     0,     0,    82,     0,    84,     0,    65,     0,
      23,     0,     0,    38,    39,     0,     0,     0,     0,    40,
     101,     0,    41,     0,     0,     0,     0,     0,    29,     0,
       0,     0,     0,     0,     0,    30,    31,    32,    33,    34,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    38,    39,     0,     0,     2,     0,    40,     3,     4,
      41,     5,     0,     7,     8,    64,    10,     0,    11,     0,
       0,     0,    14,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,     2,
      23,     0,     3,     4,     0,     5,     0,     7,     8,    64,
      10,     0,    11,     0,     0,     0,    14,     0,    29,     0,
       0,     0,     0,     0,     0,    30,    31,    32,    33,    34,
       0,     0,    65,     0,    23,     0,     0,     0,     0,     0,
       0,    38,    39,     0,     0,     0,     0,    40,     0,     0,
      41,     0,    29,     0,     0,     0,     0,     0,     0,    30,
      31,    32,    33,    34,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    38,    39,     0,     0,     2,
       0,    67,     3,     4,    41,     5,     0,     7,     8,    64,
      10,     0,    11,     0,     0,     0,    14,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    65,     2,    23,     0,     3,     4,     0,     5,
       0,     7,     8,    64,    10,     0,    11,     0,     0,     0,
      14,     0,    29,     0,     0,     0,     0,     0,     0,    30,
      31,    32,    33,    34,     0,     0,    65,     0,    23,     0,
       0,     0,     0,     0,     0,    38,    39,     0,     0,     0,
       0,    70,     0,     0,    41,   187,    29,     0,     0,     0,
       0,   188,     0,    30,    31,    32,    33,    34,     0,     0,
       0,   138,   139,   140,   141,   142,   143,     0,   144,    38,
      39,   145,   146,   147,     0,    74,     0,     0,    41,   189,
       0,     0,     0,     0,     0,   190,     0,   148,   149,   150,
     151,   152,   153,   154,     0,   138,   139,   140,   141,   142,
     143,     0,   144,     0,     0,   145,   146,   147,     0,     0,
       0,     0,   191,     0,     0,     0,     0,     0,   192,     0,
       0,   148,   149,   150,   151,   152,   153,   154,   138,   139,
     140,   141,   142,   143,     0,   144,     0,     0,   145,   146,
     147,     0,     0,     0,     0,   193,     0,     0,     0,     0,
       0,   194,     0,     0,   148,   149,   150,   151,   152,   153,
     154,   138,   139,   140,   141,   142,   143,     0,   144,     0,
       0,   145,   146,   147,     0,     0,     0,     0,   289,     0,
       0,     0,     0,     0,     0,     0,     0,   148,   149,   150,
     151,   152,   153,   154,   138,   139,   140,   141,   142,   143,
       0,   144,     0,     0,   145,   146,   147,     0,     0,     0,
       0,   290,     0,     0,     0,     0,     0,     0,     0,     0,
     148,   149,   150,   151,   152,   153,   154,   138,   139,   140,
     141,   142,   143,     0,   144,     0,     0,   145,   146,   147,
       0,     0,     0,     0,   291,     0,     0,     0,     0,     0,
       0,     0,     0,   148,   149,   150,   151,   152,   153,   154,
     138,   139,   140,   141,   142,   143,     0,   144,     0,     0,
     145,   146,   147,     0,     0,     0,     0,   292,     0,     0,
       0,     0,     0,     0,     0,     0,   148,   149,   150,   151,
     152,   153,   154,   138,   139,   140,   141,   142,   143,     0,
     144,     0,     0,   145,   146,   147,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   197,     0,     0,   148,
     149,   150,   151,   152,   153,   154,   138,   139,   140,   141,
     142,   143,     0,   144,     0,     0,   145,   146,   147,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   148,   149,   150,   151,   152,   153,   154,   184,
     198,     0,     0,     0,     0,     0,     0,   138,   139,   140,
     141,   142,   143,     0,   144,     0,     0,   145,   146,   147,
     138,   139,   140,   141,   142,   143,     0,   144,     0,     0,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,     0,     0,     0,     0,     0,   148,   149,   150,   151,
     152,   153,   154,     0,     0,     0,   202,   138,   139,   140,
     141,   142,   143,     0,   144,     0,     0,   145,   146,   147,
       0,     0,     0,     0,     0,   195,   196,     0,     0,     0,
       0,     0,     0,   148,   149,   150,   151,   152,   153,   154,
     138,   139,   140,   141,   142,   143,     0,   144,     0,     0,
     145,   146,   147,   138,   139,   140,   141,     0,     0,     0,
     144,     0,     0,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,     0,     0,     0,     0,     0,     0,   148,
     149,   150,   151,   152,   153,   154
};

static const yytype_int16 yycheck[] =
{
       4,     8,     6,     7,    78,     9,     0,   165,     0,   267,
      27,     6,     7,    17,    30,   283,    31,    21,    37,    46,
      49,    93,    26,    27,    96,    29,    30,    31,    32,    33,
      34,    35,    61,    49,    38,    39,    40,    41,    21,    22,
      66,    93,    61,    12,    48,   303,    50,    51,    49,    18,
      49,    22,    93,    22,    23,    96,    25,   325,    51,    52,
      61,    65,    61,    67,    90,    91,    70,    82,    93,    86,
      74,    96,    67,    77,    23,    70,    25,    22,    82,    31,
      32,    66,    77,    46,    69,   102,    66,    94,    95,    72,
      73,    74,    75,    95,    96,   353,    22,    95,    96,    95,
      96,    93,    87,    88,    89,    90,    91,    94,    88,    89,
      90,    91,    96,    22,    96,    22,    41,    15,    16,    17,
      22,   131,   132,    96,    22,   129,   130,   285,    97,    49,
     134,    29,    48,   127,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,    22,   156,    22,    22,    99,   160,    99,    98,   163,
      96,    96,    96,    43,    22,   169,    96,    48,   242,   173,
     174,    22,   176,   177,    22,    22,   180,   181,   182,    96,
     184,    48,    22,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    83,    49,    22,   203,   204,    84,    49,
     210,   211,   206,    22,   214,    38,    83,    82,    96,    83,
      22,   268,    48,    83,   127,   172,    82,    82,   207,   198,
     277,    51,   279,    51,    -1,    51,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    71,    -1,
      -1,    -1,    -1,    -1,    -1,   249,   250,    -1,   305,    -1,
     307,    -1,    85,    86,    87,    88,    89,    90,    91,    -1,
      -1,    -1,    -1,    -1,   338,    -1,   276,    -1,    -1,   343,
      -1,    -1,    -1,   330,    -1,    -1,    -1,   334,    -1,   336,
      -1,   288,    -1,   293,   294,   289,   290,   291,   292,   299,
      -1,    -1,   302,   350,    -1,    -1,    -1,   301,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   363,    -1,    -1,   319,
      -1,   321,    -1,    -1,   318,    -1,    -1,    -1,    -1,   329,
      -1,   331,   326,    -1,    -1,    -1,    -1,   337,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   341,   348,     0,
       1,   351,    -1,    -1,    -1,    -1,    -1,   357,    -1,   359,
      -1,    12,    -1,    -1,    15,    16,   366,    18,    19,    20,
      21,    22,    23,    -1,    25,    -1,    27,    28,    29,    -1,
      -1,    -1,    33,    34,    35,    36,    -1,    -1,    -1,    40,
      -1,    42,    -1,    -1,    45,    46,    47,    -1,    -1,    50,
      -1,    -1,    -1,    54,    55,    56,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    72,    73,    74,    75,    76,    -1,    -1,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    -1,
      -1,    12,    93,    94,    15,    16,    97,    18,    -1,    20,
      21,    22,    23,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    41,    -1,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    47,    -1,    -1,    -1,
      59,    60,    61,    62,    63,    64,    -1,    66,    -1,    -1,
      69,    70,    71,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    72,    73,    74,    75,    76,    85,    86,    87,    88,
      89,    90,    91,    -1,    -1,    -1,    -1,    88,    89,    -1,
      -1,    -1,    -1,    94,    -1,    -1,    97,    12,    99,    -1,
      15,    16,    -1,    18,    19,    20,    21,    22,    23,    -1,
      25,    -1,    27,    28,    29,    -1,    -1,    -1,    33,    34,
      35,    36,    -1,    -1,    -1,    40,    -1,    42,    -1,    -1,
      45,    46,    47,    -1,    -1,    50,    -1,    -1,    -1,    54,
      55,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    -1,    -1,    12,    -1,    94,
      15,    16,    97,    18,    19,    20,    21,    22,    23,    -1,
      25,    26,    27,    -1,    29,    -1,    -1,    -1,    33,    34,
      35,    36,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,
      45,    46,    47,    -1,    -1,    50,    -1,    -1,    -1,    -1,
      55,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    -1,    -1,    12,    93,    94,
      15,    16,    97,    18,    19,    20,    21,    22,    23,    -1,
      25,    -1,    27,    -1,    29,    -1,    -1,    -1,    33,    34,
      35,    36,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,
      45,    46,    47,    -1,    -1,    50,    -1,    -1,    -1,    -1,
      55,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    -1,    -1,    12,    93,    94,
      15,    16,    97,    18,    19,    20,    21,    22,    23,    -1,
      25,    -1,    27,    -1,    29,    -1,    -1,    -1,    33,    34,
      35,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    -1,    -1,    12,    -1,    94,
      15,    16,    97,    18,    19,    20,    21,    22,    23,    -1,
      25,    -1,    27,    -1,    29,    -1,    -1,    -1,    33,    34,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    -1,    -1,    12,    -1,    94,
      15,    16,    97,    18,    -1,    20,    21,    22,    23,    -1,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    39,    -1,    41,    42,    43,    44,
      45,    -1,    47,    -1,    -1,    12,    -1,    -1,    15,    16,
      -1,    18,    -1,    20,    21,    22,    23,    -1,    25,    -1,
      65,    -1,    29,    -1,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    -1,    -1,    41,    -1,    43,    -1,    45,    -1,
      47,    -1,    -1,    88,    89,    -1,    -1,    -1,    -1,    94,
      57,    -1,    97,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    -1,    -1,    12,    -1,    94,    15,    16,
      97,    18,    -1,    20,    21,    22,    23,    -1,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    12,
      47,    -1,    15,    16,    -1,    18,    -1,    20,    21,    22,
      23,    -1,    25,    -1,    -1,    -1,    29,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    73,    74,    75,    76,
      -1,    -1,    45,    -1,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    -1,    -1,    -1,    -1,    94,    -1,    -1,
      97,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      73,    74,    75,    76,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    -1,    -1,    12,
      -1,    94,    15,    16,    97,    18,    -1,    20,    21,    22,
      23,    -1,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    12,    47,    -1,    15,    16,    -1,    18,
      -1,    20,    21,    22,    23,    -1,    25,    -1,    -1,    -1,
      29,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      73,    74,    75,    76,    -1,    -1,    45,    -1,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    -1,    -1,    -1,
      -1,    94,    -1,    -1,    97,    43,    65,    -1,    -1,    -1,
      -1,    49,    -1,    72,    73,    74,    75,    76,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    64,    -1,    66,    88,
      89,    69,    70,    71,    -1,    94,    -1,    -1,    97,    43,
      -1,    -1,    -1,    -1,    -1,    49,    -1,    85,    86,    87,
      88,    89,    90,    91,    -1,    59,    60,    61,    62,    63,
      64,    -1,    66,    -1,    -1,    69,    70,    71,    -1,    -1,
      -1,    -1,    43,    -1,    -1,    -1,    -1,    -1,    49,    -1,
      -1,    85,    86,    87,    88,    89,    90,    91,    59,    60,
      61,    62,    63,    64,    -1,    66,    -1,    -1,    69,    70,
      71,    -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,    -1,
      -1,    49,    -1,    -1,    85,    86,    87,    88,    89,    90,
      91,    59,    60,    61,    62,    63,    64,    -1,    66,    -1,
      -1,    69,    70,    71,    -1,    -1,    -1,    -1,    43,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,    86,    87,
      88,    89,    90,    91,    59,    60,    61,    62,    63,    64,
      -1,    66,    -1,    -1,    69,    70,    71,    -1,    -1,    -1,
      -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      85,    86,    87,    88,    89,    90,    91,    59,    60,    61,
      62,    63,    64,    -1,    66,    -1,    -1,    69,    70,    71,
      -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    85,    86,    87,    88,    89,    90,    91,
      59,    60,    61,    62,    63,    64,    -1,    66,    -1,    -1,
      69,    70,    71,    -1,    -1,    -1,    -1,    43,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    85,    86,    87,    88,
      89,    90,    91,    59,    60,    61,    62,    63,    64,    -1,
      66,    -1,    -1,    69,    70,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,    85,
      86,    87,    88,    89,    90,    91,    59,    60,    61,    62,
      63,    64,    -1,    66,    -1,    -1,    69,    70,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    85,    86,    87,    88,    89,    90,    91,    51,
      93,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,
      62,    63,    64,    -1,    66,    -1,    -1,    69,    70,    71,
      59,    60,    61,    62,    63,    64,    -1,    66,    -1,    -1,
      69,    70,    71,    85,    86,    87,    88,    89,    90,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    85,    86,    87,    88,
      89,    90,    91,    -1,    -1,    -1,    95,    59,    60,    61,
      62,    63,    64,    -1,    66,    -1,    -1,    69,    70,    71,
      -1,    -1,    -1,    -1,    -1,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    85,    86,    87,    88,    89,    90,    91,
      59,    60,    61,    62,    63,    64,    -1,    66,    -1,    -1,
      69,    70,    71,    59,    60,    61,    62,    -1,    -1,    -1,
      66,    -1,    -1,    69,    70,    71,    85,    86,    87,    88,
      89,    90,    91,    -1,    -1,    -1,    -1,    -1,    -1,    85,
      86,    87,    88,    89,    90,    91
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    15,    16,    18,    19,    20,    21,    22,
      23,    25,    27,    28,    29,    33,    34,    35,    36,    40,
      42,    45,    46,    47,    50,    54,    55,    56,    58,    65,
      72,    73,    74,    75,    76,    79,    80,    81,    88,    89,
      94,    97,   101,   102,   103,   104,   106,   108,   112,   113,
     114,   119,   120,   121,   122,   123,   124,   130,   132,   134,
     138,   143,   146,    93,    22,    45,   121,    94,   121,   144,
      94,   144,    22,   120,    94,   121,   145,   133,   137,    46,
     125,    39,    41,    42,    43,    44,   121,   131,   135,    46,
     121,    51,    52,    15,    16,    17,    22,    29,    22,    22,
     121,    57,   121,   131,    22,   129,   121,   121,   121,   121,
     121,   121,   121,   126,   139,   121,   121,   121,    23,    25,
      99,   121,   147,   148,   149,   150,     0,    93,   121,    37,
      61,   121,   105,   111,   114,   121,   124,   143,    59,    60,
      61,    62,    63,    64,    66,    69,    70,    71,    85,    86,
      87,    88,    89,    90,    91,   116,    94,   121,   121,   144,
      96,   144,   144,    96,   144,   116,    22,   127,   121,    41,
     131,    22,    38,    67,    68,    22,    49,    61,   120,   120,
      49,    61,    49,    48,    51,   131,    96,    43,    49,    43,
      49,    43,    49,    43,    49,    77,    78,    49,    93,    22,
     128,    22,    95,    99,    99,    98,    96,    96,   102,   117,
     121,   121,   117,   117,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,    22,   140,    95,   121,    95,    95,   121,   140,
      96,   121,   136,    22,   123,   121,   121,   121,   121,    49,
      61,    43,   121,   121,   121,   121,    22,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   107,   107,    96,
     120,   120,   121,    23,    25,   150,   118,   117,   117,    48,
     117,    93,    96,    93,    22,   116,   121,   121,    22,    43,
      43,    43,    43,   104,   118,    22,    93,   106,   117,   118,
      30,    49,   118,    48,    96,   142,    22,   142,   140,   120,
     121,   121,   121,   121,   117,   117,    83,   117,    49,   121,
     117,   104,    22,   118,   118,    93,    49,    84,    83,   121,
     117,   109,   117,    26,   142,   121,   117,   118,    31,    32,
     110,   115,   117,    31,    82,    22,   141,   118,   118,   117,
     116,   121,    82,   116,    96,   117,    83,   118,   117,   104,
      22,    83,   117,    48,   117,    82,   118,    82,   117
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   100,   101,   101,   101,   102,   102,   102,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     104,   104,   104,   105,   105,   105,   105,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   107,   108,   108,
     109,   109,   110,   111,   111,   112,   113,   114,   115,   116,
     117,   118,   118,   118,   119,   120,   120,   120,   120,   120,
     120,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   122,   122,   122,   122,   122,   122,   122,
     122,   123,   123,   124,   124,   124,   124,   124,   125,   124,
     126,   124,   124,   124,   124,   124,   124,   127,   127,   128,
     128,   129,   129,   130,   130,   130,   130,   130,   130,   131,
     131,   132,   132,   132,   132,   133,   132,   135,   136,   134,
     134,   137,   134,   134,   134,   139,   138,   140,   140,   140,
     140,   141,   141,   141,   142,   143,   144,   144,   144,   145,
     145,   146,   147,   147,   147,   147,   148,   148,   149,   149,
     150,   150
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
    case 19: /* BLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1698 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 20: /* FBLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1704 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 21: /* RBLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1710 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 22: /* ID  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1716 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* STRING  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1722 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* HANDLER  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1728 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* SYMBOL  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1734 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* ENDCLAUSE  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1740 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* tPLAYACCEL  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1746 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* tMETHOD  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1752 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* THEOBJECTFIELD  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).os; }
#line 1758 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 138: /* on  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1764 "engines/director/lingo/lingo-gr.cpp"
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
#line 2034 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 143 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2045 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 149 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2053 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 152 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_after); }
#line 2059 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 153 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_before); }
#line 2065 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 154 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2076 "engines/director/lingo/lingo-gr.cpp"
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
#line 2088 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 167 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2099 "engines/director/lingo/lingo-gr.cpp"
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
#line 2111 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 180 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2122 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 186 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 193 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if (!(yyvsp[-3].s)->equalsIgnoreCase("menu")) {
			error("LEXER: keyword 'menu' expected");
		}

		warning("STUB: menuItem entity");
		g_lingo->code1(LC::c_themenuitementityassign);
		g_lingo->codeInt((yyvsp[-6].e)[0]);
		g_lingo->codeInt((yyvsp[-6].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 203 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).oe);
		delete (yyvsp[-2].objectfield).os;
		(yyval.code) = (yyvsp[0].code); }
#line 2159 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 225 "engines/director/lingo/lingo-gr.y"
                                                                        {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[-3].code) - (yyvsp[-5].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 2] = end; }
#line 2170 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 236 "engines/director/lingo/lingo-gr.y"
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
#line 2187 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 253 "engines/director/lingo/lingo-gr.y"
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
#line 2204 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 265 "engines/director/lingo/lingo-gr.y"
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
#line 2219 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 276 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_nextRepeat); }
#line 2226 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 278 "engines/director/lingo/lingo-gr.y"
                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2236 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 283 "engines/director/lingo/lingo-gr.y"
                                                          {
		inst end;
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code));
		(*g_lingo->_currentScript)[(yyvsp[-3].code) + 1] = end; }
#line 2245 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 287 "engines/director/lingo/lingo-gr.y"
                                                    {
		inst end;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2254 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 292 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }
#line 2262 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 296 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-6].code) - (yyvsp[-8].code));
		WRITE_UINT32(&else1, (yyvsp[-3].code) - (yyvsp[-8].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-8].code));
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-8].code), (yyvsp[-1].code) - (yyvsp[-8].code), 0); }
#line 2277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 306 "engines/director/lingo/lingo-gr.y"
                                                                                                              {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-9].code) - (yyvsp[-11].code));
		WRITE_UINT32(&else1, (yyvsp[-6].code) - (yyvsp[-11].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-11].code));
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-11].code), (yyvsp[-1].code) - (yyvsp[-11].code), (yyvsp[-3].code) - (yyvsp[-11].code)); }
#line 2292 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 320 "engines/director/lingo/lingo-gr.y"
                                                        {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-3].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2303 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 327 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-8].code) - (yyvsp[-10].code));
		WRITE_UINT32(&else1, (yyvsp[-5].code) - (yyvsp[-10].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-10].code));
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-10].code), (yyvsp[-1].code) - (yyvsp[-10].code), (yyvsp[-3].code) - (yyvsp[-10].code)); }
#line 2318 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 337 "engines/director/lingo/lingo-gr.y"
                                                                   {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-4].code) - (yyvsp[-6].code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code));
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code)); }
#line 2333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 348 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code3(LC::c_repeatwhilecode, STOP, STOP); }
#line 2339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 350 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code3(LC::c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2349 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 356 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); }
#line 2359 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 362 "engines/director/lingo/lingo-gr.y"
                                        {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); }
#line 2370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 369 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2376 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 371 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2382 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 373 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 377 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2398 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 383 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2406 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 386 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2414 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 389 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2423 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 393 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2432 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 397 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 403 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2447 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 405 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2455 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 408 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2463 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 411 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2471 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 414 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2484 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 422 "engines/director/lingo/lingo-gr.y"
                               {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 428 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).oe);
		delete (yyvsp[0].objectfield).os; }
#line 2505 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 434 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2511 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 435 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2517 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 436 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2523 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 437 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2529 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 438 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2535 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 439 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2541 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 440 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2547 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 441 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2553 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 442 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2559 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 443 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2565 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 444 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2571 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 445 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2577 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 446 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2583 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 447 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2589 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 448 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2595 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 449 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2601 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 450 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2607 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 451 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2613 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 452 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); }
#line 2619 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 453 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 454 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 455 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_intersects); }
#line 2637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 456 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_within); }
#line 2643 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 458 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_charOf); }
#line 2649 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 459 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charToOf); }
#line 2655 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 460 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_itemOf); }
#line 2661 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 461 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemToOf); }
#line 2667 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 462 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_lineOf); }
#line 2673 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 463 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineToOf); }
#line 2679 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 464 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_wordOf); }
#line 2685 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 465 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordToOf); }
#line 2691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 467 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2699 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 472 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_printtop); }
#line 2705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 475 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_exitRepeat); }
#line 2711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 476 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_procret); }
#line 2717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 477 "engines/director/lingo/lingo-gr.y"
                  { g_lingo->_indef = kStateInArgs; }
#line 2723 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 477 "engines/director/lingo/lingo-gr.y"
                                                                 { g_lingo->_indef = kStateNone; }
#line 2729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 478 "engines/director/lingo/lingo-gr.y"
                    { g_lingo->_indef = kStateInArgs; }
#line 2735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 478 "engines/director/lingo/lingo-gr.y"
                                                                     { g_lingo->_indef = kStateNone; }
#line 2741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 480 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 483 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2757 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 486 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_open); }
#line 2763 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 487 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 2769 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 489 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2778 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 493 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2787 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 498 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2796 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 502 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2805 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 507 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2814 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 511 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2823 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 523 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 2829 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 524 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 2835 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 525 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 2841 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 526 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 2850 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 530 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 2859 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 534 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 2868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 542 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_playdone); }
#line 2874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 543 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 2883 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 547 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 2892 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 551 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 2901 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 555 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 2907 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 555 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 2916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 585 "engines/director/lingo/lingo-gr.y"
             { g_lingo->_indef = kStateInArgs; }
#line 2922 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 585 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->_currentFactory.clear(); }
#line 2928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 586 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete (yyvsp[-6].s); }
#line 2939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 592 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 2945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 593 "engines/director/lingo/lingo-gr.y"
                  { g_lingo->_indef = kStateInArgs; }
#line 2951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 594 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete (yyvsp[-6].s); }
#line 2962 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 600 "engines/director/lingo/lingo-gr.y"
                                                                     {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 2977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 610 "engines/director/lingo/lingo-gr.y"
                                                 {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->_indef = kStateNone;
		g_lingo->clearArgStack();
		g_lingo->_ignoreMe = false;
		delete (yyvsp[-5].s); }
#line 2989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 618 "engines/director/lingo/lingo-gr.y"
         { g_lingo->_indef = kStateInArgs; }
#line 2995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 618 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.s) = (yyvsp[0].s); g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 3001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 620 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 3007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 621 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 622 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 623 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-3].narg) + 1; delete (yyvsp[0].s); }
#line 3025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 626 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 627 "engines/director/lingo/lingo-gr.y"
                                        { delete (yyvsp[0].s); }
#line 3037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 629 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArgStore(); g_lingo->_indef = kStateInDef; }
#line 3043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 631 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 3055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 639 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 3061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 640 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3067 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 641 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3073 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 643 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 644 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 646 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = (yyvsp[-1].code); }
#line 3091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 648 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3097 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 649 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3103 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 650 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3109 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 651 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3115 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 653 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3121 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 654 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3127 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 656 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3133 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 657 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3139 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 659 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 663 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3157 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3161 "engines/director/lingo/lingo-gr.cpp"

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
#line 669 "engines/director/lingo/lingo-gr.y"

