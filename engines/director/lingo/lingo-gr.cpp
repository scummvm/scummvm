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
    tINTO = 292,
    tLOOP = 293,
    tMACRO = 294,
    tMOVIE = 295,
    tNEXT = 296,
    tOF = 297,
    tPREVIOUS = 298,
    tPUT = 299,
    tREPEAT = 300,
    tSET = 301,
    tTHEN = 302,
    tTO = 303,
    tWHEN = 304,
    tWITH = 305,
    tWHILE = 306,
    tNLELSE = 307,
    tFACTORY = 308,
    tOPEN = 309,
    tPLAY = 310,
    tDONE = 311,
    tINSTANCE = 312,
    tGE = 313,
    tLE = 314,
    tEQ = 315,
    tNEQ = 316,
    tAND = 317,
    tOR = 318,
    tNOT = 319,
    tMOD = 320,
    tAFTER = 321,
    tBEFORE = 322,
    tCONCAT = 323,
    tCONTAINS = 324,
    tSTARTS = 325,
    tCHAR = 326,
    tITEM = 327,
    tLINE = 328,
    tWORD = 329,
    tSPRITE = 330,
    tINTERSECTS = 331,
    tWITHIN = 332,
    tTELL = 333,
    tPROPERTY = 334,
    tON = 335,
    tENDIF = 336,
    tENDREPEAT = 337,
    tENDTELL = 338
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

#line 252 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  125
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1594

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  99
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  168
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  360

#define YYUNDEFTOK  2
#define YYMAXUTOK   338


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      92,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    91,    86,     2,
      93,    94,    89,    87,    95,    88,     2,    90,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    98,     2,
      84,     2,    85,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    96,     2,    97,     2,     2,     2,     2,     2,     2,
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
      75,    76,    77,    78,    79,    80,    81,    82,    83
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   135,   135,   136,   137,   139,   140,   141,   143,   149,
     152,   153,   154,   160,   167,   173,   180,   186,   193,   203,
     210,   211,   212,   214,   215,   216,   217,   219,   220,   225,
     236,   253,   265,   270,   274,   279,   283,   293,   304,   305,
     307,   314,   324,   335,   337,   343,   349,   356,   358,   360,
     361,   362,   364,   370,   373,   376,   380,   384,   388,   390,
     391,   392,   395,   398,   401,   409,   415,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   445,   446,   447,   448,   449,   450,   451,   452,   454,
     457,   459,   460,   461,   462,   463,   464,   464,   465,   466,
     467,   470,   473,   474,   476,   480,   485,   489,   494,   498,
     510,   511,   512,   513,   517,   521,   526,   527,   529,   530,
     534,   538,   542,   542,   572,   572,   572,   579,   580,   580,
     587,   597,   605,   605,   607,   608,   609,   610,   612,   613,
     614,   616,   618,   626,   627,   628,   630,   631,   633,   635,
     636,   637,   638,   640,   641,   643,   644,   646,   650
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
  "tDOWN", "tELSE", "tELSIF", "tEXIT", "tGLOBAL", "tGO", "tIF", "tINTO",
  "tLOOP", "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT",
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tNLELSE",
  "tFACTORY", "tOPEN", "tPLAY", "tDONE", "tINSTANCE", "tGE", "tLE", "tEQ",
  "tNEQ", "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tENDIF",
  "tENDREPEAT", "tENDTELL", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'\\n'", "'('", "')'", "','", "'['", "']'", "':'",
  "$accept", "program", "programline", "asgn", "stmtoneliner",
  "stmtonelinerwithif", "stmt", "tellstart", "ifstmt", "elseifstmtlist",
  "elseifstmt", "ifoneliner", "repeatwhile", "repeatwith", "if", "elseif",
  "begin", "end", "stmtlist", "when", "simpleexpr", "expr", "chunkexpr",
  "reference", "proc", "$@1", "globallist", "propertylist", "instancelist",
  "gotofunc", "gotomovie", "playfunc", "$@2", "defn", "$@3", "$@4", "$@5",
  "on", "$@6", "argdef", "endargdef", "argstore", "macro", "arglist",
  "nonemptyarglist", "list", "valuelist", "linearlist", "proplist",
  "proppair", YY_NULLPTR
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
     335,   336,   337,   338,    60,    62,    38,    43,    45,    42,
      47,    37,    10,    40,    41,    44,    91,    93,    58
};
# endif

#define YYPACT_NINF (-267)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-6)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     330,   -82,  -267,  -267,   199,  -267,   919,   952,    30,  1031,
    -267,  -267,  -267,  -267,  -267,   -30,  -267,   804,  -267,  -267,
     199,    40,   176,     9,    39,   199,   840,    41,   199,   199,
     199,   199,   199,   199,   199,    54,  -267,   199,   199,   199,
      62,     7,  -267,  -267,  -267,  -267,  -267,   199,   -37,   199,
     646,  -267,  1491,  -267,  -267,  -267,  -267,  -267,  -267,  -267,
    -267,  -267,  -267,    -7,   199,  1491,   199,  1491,   -12,   199,
     -12,  -267,  -267,   199,  1491,     0,   199,  -267,  -267,    79,
    -267,   199,  -267,    65,  -267,  1070,  -267,    88,  1004,    93,
    -267,    -1,    30,    30,     6,    77,    80,  -267,  1408,  -267,
    1070,  -267,  -267,    36,  -267,  1103,  1136,  1169,  1202,  1458,
    1367,  -267,    58,   127,  -267,  -267,  1421,    57,    61,  -267,
    -267,    59,    66,    69,  -267,  -267,   409,  1491,   199,  1491,
    -267,  -267,   199,  1491,  -267,  -267,   199,   199,   199,   199,
     199,   199,   199,   199,   199,   199,   199,   199,   199,   199,
     199,   199,   199,   143,   199,  1004,  1421,     3,   199,    23,
      27,   199,   -12,   143,  -267,    71,  1491,   199,  -267,  -267,
      35,   199,   199,  -267,   199,   199,    10,   130,   199,   199,
     199,  -267,   199,  -267,   147,   199,   199,   199,   199,   199,
     199,   199,   199,   199,   199,  -267,  -267,   151,  -267,  -267,
      30,    30,  -267,    30,    13,  -267,  -267,  1491,   132,  -267,
    1491,   144,   144,   144,   144,  1504,  1504,  -267,    24,   144,
     144,   144,   144,    24,    14,    14,  -267,  -267,  -267,   -75,
    -267,  1491,  -267,  -267,  1491,   -55,   155,  1491,  -267,  -267,
    -267,  1491,  1491,  1491,   144,   199,   199,   158,  1491,   144,
    1491,  1491,  -267,  1491,  1235,  1491,  1268,  1491,  1301,  1491,
    1334,  1491,  1491,   725,  -267,  -267,  -267,  -267,  -267,    57,
      61,  -267,   567,     2,  -267,   148,    99,   174,    99,  -267,
     143,  1491,   144,    30,   199,   199,   199,   199,  -267,   567,
    -267,  -267,   117,   159,   199,   567,   725,   181,  -267,  -267,
    -267,   -24,   160,  1491,  1491,  1491,  1491,  -267,   123,  -267,
     199,  1491,  -267,  -267,  -267,   488,   567,    99,   199,  -267,
    1491,  -267,   120,   -17,   188,  -267,  1491,  -267,   567,  -267,
    -267,  -267,   199,   135,  -267,  -267,  -267,   118,   567,   567,
     136,  -267,  1491,  -267,   725,   201,   145,  -267,   567,   178,
    -267,  -267,  -267,   154,  -267,   156,  -267,   567,  -267,  -267
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    53,    64,     0,    54,   153,   153,     0,    57,
      56,    55,   132,   138,    66,   105,   106,     0,    45,   134,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   142,     0,     0,     0,
     159,     0,     3,    67,    27,     7,    28,     0,     0,     0,
       0,    59,    21,   100,    60,    22,   102,   103,     6,    47,
      20,    58,     4,    57,     0,    65,   153,   154,   111,   153,
      62,    57,    99,   153,   156,   152,   153,    47,   104,     0,
     120,     0,   121,     0,   122,   123,   125,     0,   101,     0,
      43,     0,     0,     0,     0,     0,     0,   137,   113,   128,
     129,   131,   118,   109,    81,     0,     0,     0,     0,     0,
       0,   116,   108,     0,    86,    87,     0,    56,    55,   160,
     163,     0,   161,   162,   165,     1,     5,    48,     0,    48,
      48,    26,     0,    24,    25,    23,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   144,   153,     0,   154,     0,     0,     0,
       0,     0,   133,   144,   114,   107,   127,     0,   124,   135,
       0,     0,     0,    44,     0,     0,     0,     0,     0,     0,
       0,    52,     0,   130,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    35,    35,     0,   143,    88,
       0,     0,   158,     0,     0,     2,    49,    48,     0,    32,
      48,    77,    78,    75,    76,    79,    80,    72,    83,    84,
      85,    74,    73,    82,    68,    69,    70,    71,   145,     0,
     110,   155,    61,    63,   157,     0,     0,   126,    47,     8,
       9,    10,    11,    15,    13,     0,     0,     0,    14,    12,
      19,   112,   119,    91,     0,    93,     0,    95,     0,    97,
       0,    89,    90,     0,    49,   117,   168,   167,   164,     0,
       0,   166,    48,     0,    49,     0,   151,     0,   151,   115,
     144,    16,    17,     0,     0,     0,     0,     0,    48,    48,
      50,    51,     0,     0,     0,    48,     0,     0,    49,   146,
      49,     0,     0,    92,    94,    96,    98,    34,     0,    29,
       0,    48,    38,    48,   147,   141,   139,   151,     0,    33,
      48,    49,    48,     0,   148,    49,    18,    49,    48,    47,
      46,    39,     0,     0,    47,    42,   149,   140,   136,    48,
       0,    49,    48,    36,     0,     0,     0,    30,    48,     0,
      48,   150,    31,     0,    49,     0,    37,    48,    41,    40
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -267,  -267,   100,  -267,  -252,  -267,     4,    33,  -267,  -267,
    -267,  -267,  -267,  -267,   186,  -267,   -76,   -10,  -225,  -267,
       1,    -4,  -267,    68,   189,  -267,  -267,  -267,  -267,  -267,
     -18,  -267,  -267,  -267,  -267,  -267,  -267,  -267,  -267,  -157,
    -267,  -266,   190,    12,  -267,  -267,  -267,  -267,  -267,    42
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    41,    42,    43,    44,   130,   291,   263,    46,   322,
     331,   131,    47,    48,    49,   332,   153,   206,   272,    50,
      51,    52,    53,    54,    55,    79,   165,   112,   103,    56,
      86,    57,    76,    58,    87,   238,    77,    59,   113,   229,
     337,   298,    60,   160,    75,    61,   121,   122,   123,   124
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      65,   163,    67,    67,    45,    74,   235,   125,   101,    72,
      62,   288,   300,    85,   334,    78,    88,   276,    68,    70,
     277,    98,   100,   128,   104,   105,   106,   107,   108,   109,
     110,    96,   293,   114,   115,   116,   269,   278,   270,   289,
     277,   120,     2,   127,   313,   129,   133,   174,     5,   295,
     294,   325,    71,    10,   178,    11,     8,   239,   245,   175,
     155,    97,   156,   102,   335,   156,   179,   168,   317,   156,
     246,   277,    67,   315,     2,   316,   111,   166,   157,   142,
       5,   159,   183,   158,    71,   117,   154,   118,   162,   142,
      89,    90,   350,   176,   177,   161,   328,   230,   158,   126,
     338,   164,   339,   151,   152,   167,    29,    30,    31,    32,
     169,   149,   150,   151,   152,   173,   348,   232,   158,   208,
     209,   233,   158,   301,   207,   180,    40,   181,   210,   357,
      45,   184,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   198,
      67,   329,   330,   197,   231,   200,   202,   234,    40,   201,
     119,   203,   280,   237,   204,   228,   236,   241,   242,   252,
     243,   244,   247,   265,   248,   249,   250,   279,   251,   274,
     283,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,    91,    92,    93,   297,   296,   299,   273,    94,   309,
     275,   266,   267,   314,   268,    95,   319,   310,   318,   142,
     336,     2,   143,   345,     3,     4,   343,     5,   347,     7,
       8,    63,    10,   351,    11,   354,   205,   352,    14,   264,
     148,   149,   150,   151,   152,   356,   132,   358,   240,   134,
     135,   281,   282,    64,     0,    22,   271,     0,     0,     0,
       0,     0,     0,   341,     0,     0,     0,     0,   344,     0,
       0,     0,   292,    28,     0,     0,     0,     0,     0,     0,
      29,    30,    31,    32,    33,     0,     0,     0,   307,   308,
     303,   304,   305,   306,   302,   312,    37,    38,     0,     0,
     311,     0,    39,     0,     0,    40,     0,     0,     0,     0,
       0,   321,     0,   323,     0,     0,   320,     0,     0,     0,
     327,     0,   333,     0,   326,     0,     0,     0,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   342,   346,
      -5,     1,   349,     0,     0,     0,     0,     0,   353,     0,
     355,     0,     2,     0,     0,     3,     4,   359,     5,     6,
       7,     8,     9,    10,     0,    11,     0,    12,    13,    14,
       0,     0,     0,    15,    16,    17,    18,     0,     0,    19,
       0,     0,     0,     0,    20,    21,    22,     0,     0,    23,
       0,     0,     0,    24,    25,    26,     0,    27,     0,     0,
       0,     0,     0,     0,    28,     0,     0,     0,     0,     0,
       0,    29,    30,    31,    32,    33,     0,     0,    34,    35,
      36,     0,     0,     0,     0,     0,     0,    37,    38,     0,
       0,     2,    -5,    39,     3,     4,    40,     5,     6,     7,
       8,     9,    10,     0,    11,     0,    12,    13,    14,     0,
       0,     0,    15,    16,    17,    18,     0,     0,    19,     0,
       0,     0,     0,    20,    21,    22,     0,     0,    23,     0,
       0,     0,    24,    25,    26,     0,    27,     0,     0,     0,
       0,     0,     0,    28,     0,     0,     0,     0,     0,     0,
      29,    30,    31,    32,    33,     0,     0,    34,    35,    36,
       0,     0,     0,     0,     0,     0,    37,    38,     0,     0,
       2,     0,    39,     3,     4,    40,     5,     6,     7,     8,
       9,    10,     0,    11,   324,    12,     0,    14,     0,     0,
       0,    15,    16,    17,    18,     0,     0,     0,     0,     0,
       0,     0,    20,    21,    22,     0,     0,    23,     0,     0,
       0,     0,    25,    26,     0,    27,     0,     0,     0,     0,
       0,     0,    28,     0,     0,     0,     0,     0,     0,    29,
      30,    31,    32,    33,     0,     0,    34,    35,     0,     0,
       0,     0,     0,     0,     0,    37,    38,     0,     0,     2,
     290,    39,     3,     4,    40,     5,     6,     7,     8,     9,
      10,     0,    11,     0,    12,     0,    14,     0,     0,     0,
      15,    16,    17,    18,     0,     0,     0,     0,     0,     0,
       0,    20,    21,    22,     0,     0,    23,     0,     0,     0,
       0,    25,    26,     0,    27,     0,     0,     0,     0,     0,
       0,    28,     0,     0,     0,     0,     0,     0,    29,    30,
      31,    32,    33,     0,     0,    34,    35,     0,     0,     0,
       0,     0,     0,     0,    37,    38,     0,     0,     2,   290,
      39,     3,     4,    40,     5,     6,     7,     8,     9,    10,
       0,    11,     0,    12,     0,    14,     0,     0,     0,    15,
      16,    17,    18,     0,     0,     0,     0,     0,     0,     0,
      20,     0,    22,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,    27,     0,     0,     0,     0,     0,     0,
      28,     0,     0,     0,     0,     0,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    35,     0,     0,     0,     0,
       0,     0,     0,    37,    38,     0,     0,     2,     0,    39,
       3,     4,    40,     5,     6,     7,     8,     9,    10,     0,
      11,     0,    12,     0,    14,     0,     0,     0,    15,    16,
      17,     0,     0,     0,     0,     0,     0,     0,     0,    20,
       0,    22,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,    27,     0,     0,     0,     0,     0,     0,    28,
       0,     0,     0,     0,     0,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    35,     0,     0,     0,     0,     0,
       0,     0,    37,    38,     0,     0,     2,     0,    39,     3,
       4,    40,     5,     0,     7,     8,    63,    10,     0,    11,
       0,     0,     0,    14,     0,     0,     0,     0,     0,     0,
       0,     0,    80,     0,    81,    82,    83,    84,    64,     0,
      22,     0,     2,     0,     0,     3,     4,     0,     5,     0,
       7,     8,    63,    10,     0,    11,     0,     0,    28,    14,
       0,     0,     0,     0,     0,    29,    30,    31,    32,    33,
      81,     0,    83,     0,    64,     0,    22,     0,     0,     0,
       0,    37,    38,     0,     0,     0,    99,    39,     0,     0,
      40,     0,     0,     0,    28,     0,     0,     0,     0,     0,
       0,    29,    30,    31,    32,    33,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,    38,     0,
       0,     2,     0,    39,     3,     4,    40,     5,     0,     7,
       8,    63,    10,     0,    11,     0,     0,     0,    14,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    64,     2,    22,     0,     3,     4,     0,
       5,     0,     7,     8,    63,    10,     0,    11,     0,     0,
       0,    14,     0,    28,     0,     0,     0,     0,     0,     0,
      29,    30,    31,    32,    33,     0,    64,     0,    22,     0,
       0,     0,     0,     0,     0,     0,    37,    38,     0,     0,
       0,     0,    66,     0,     0,    40,    28,     0,     0,     0,
       0,     0,     0,    29,    30,    31,    32,    33,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    37,
      38,   170,     0,     2,     0,    69,     3,     4,    40,     5,
       0,     7,     8,    63,    10,     0,    11,     0,     0,     0,
      14,     0,   136,   137,   138,   139,   140,   141,     0,   142,
     171,   172,   143,   144,   145,    64,     0,    22,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   146,   147,
     148,   149,   150,   151,   152,    28,     0,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      81,     0,    83,     0,     0,     0,     0,     0,    37,    38,
       0,     0,     0,     0,    73,     0,     0,    40,   136,   137,
     138,   139,   140,   141,     0,   142,     0,     0,   143,   144,
     145,     0,     0,     0,     0,   185,     0,     0,     0,     0,
       0,   186,     0,     0,   146,   147,   148,   149,   150,   151,
     152,   136,   137,   138,   139,   140,   141,     0,   142,     0,
       0,   143,   144,   145,     0,     0,     0,     0,   187,     0,
       0,     0,     0,     0,   188,     0,     0,   146,   147,   148,
     149,   150,   151,   152,   136,   137,   138,   139,   140,   141,
       0,   142,     0,     0,   143,   144,   145,     0,     0,     0,
       0,   189,     0,     0,     0,     0,     0,   190,     0,     0,
     146,   147,   148,   149,   150,   151,   152,   136,   137,   138,
     139,   140,   141,     0,   142,     0,     0,   143,   144,   145,
       0,     0,     0,     0,   191,     0,     0,     0,     0,     0,
     192,     0,     0,   146,   147,   148,   149,   150,   151,   152,
     136,   137,   138,   139,   140,   141,     0,   142,     0,     0,
     143,   144,   145,     0,     0,     0,     0,   284,     0,     0,
       0,     0,     0,     0,     0,     0,   146,   147,   148,   149,
     150,   151,   152,   136,   137,   138,   139,   140,   141,     0,
     142,     0,     0,   143,   144,   145,     0,     0,     0,     0,
     285,     0,     0,     0,     0,     0,     0,     0,     0,   146,
     147,   148,   149,   150,   151,   152,   136,   137,   138,   139,
     140,   141,     0,   142,     0,     0,   143,   144,   145,     0,
       0,     0,     0,   286,     0,     0,     0,     0,     0,     0,
       0,     0,   146,   147,   148,   149,   150,   151,   152,   136,
     137,   138,   139,   140,   141,     0,   142,     0,     0,   143,
     144,   145,     0,     0,     0,     0,   287,     0,     0,     0,
       0,     0,     0,     0,     0,   146,   147,   148,   149,   150,
     151,   152,   136,   137,   138,   139,   140,   141,     0,   142,
       0,     0,   143,   144,   145,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   195,     0,     0,   146,   147,
     148,   149,   150,   151,   152,   136,   137,   138,   139,   140,
     141,     0,   142,     0,     0,   143,   144,   145,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   146,   147,   148,   149,   150,   151,   152,   182,   196,
       0,     0,     0,     0,     0,     0,   136,   137,   138,   139,
     140,   141,     0,   142,     0,     0,   143,   144,   145,   136,
     137,   138,   139,   140,   141,     0,   142,     0,     0,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,     0,
       0,     0,     0,     0,     0,   146,   147,   148,   149,   150,
     151,   152,     0,     0,     0,   199,   136,   137,   138,   139,
     140,   141,     0,   142,     0,     0,   143,   144,   145,     0,
       0,     0,     0,     0,   193,   194,     0,     0,     0,     0,
       0,     0,   146,   147,   148,   149,   150,   151,   152,   136,
     137,   138,   139,   140,   141,     0,   142,     0,     0,   143,
     144,   145,   136,   137,   138,   139,     0,     0,     0,   142,
       0,     0,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,     0,     0,     0,     0,     0,   146,   147,
     148,   149,   150,   151,   152
};

static const yytype_int16 yycheck[] =
{
       4,    77,     6,     7,     0,     9,   163,     0,    26,     8,
      92,   263,   278,    17,    31,    45,    20,    92,     6,     7,
      95,    25,    26,    60,    28,    29,    30,    31,    32,    33,
      34,    22,    30,    37,    38,    39,    23,    92,    25,   264,
      95,    40,    12,    47,   296,    49,    50,    48,    18,   274,
      48,   317,    22,    23,    48,    25,    21,    22,    48,    60,
      64,    22,    66,    22,    81,    69,    60,    85,    92,    73,
      60,    95,    76,   298,    12,   300,    22,    81,    66,    65,
      18,    69,   100,    95,    22,    23,    93,    25,    76,    65,
      50,    51,   344,    92,    93,    95,   321,    94,    95,    92,
     325,    22,   327,    89,    90,    40,    71,    72,    73,    74,
      22,    87,    88,    89,    90,    22,   341,    94,    95,   129,
     130,    94,    95,   280,   128,    48,    96,    47,   132,   354,
     126,    95,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,    22,
     154,    31,    32,    95,   158,    98,    97,   161,    96,    98,
      98,    95,   238,   167,    95,    22,    95,   171,   172,    22,
     174,   175,    42,    22,   178,   179,   180,    22,   182,    47,
      22,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    15,    16,    17,    95,    47,    22,   207,    22,    82,
     210,   200,   201,    22,   203,    29,    83,    48,    48,    65,
      22,    12,    68,    95,    15,    16,    81,    18,    82,    20,
      21,    22,    23,    22,    25,    47,   126,    82,    29,   196,
      86,    87,    88,    89,    90,    81,    50,    81,   170,    50,
      50,   245,   246,    44,    -1,    46,   204,    -1,    -1,    -1,
      -1,    -1,    -1,   329,    -1,    -1,    -1,    -1,   334,    -1,
      -1,    -1,   272,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    -1,    -1,    -1,   288,   289,
     284,   285,   286,   287,   283,   295,    87,    88,    -1,    -1,
     294,    -1,    93,    -1,    -1,    96,    -1,    -1,    -1,    -1,
      -1,   311,    -1,   313,    -1,    -1,   310,    -1,    -1,    -1,
     320,    -1,   322,    -1,   318,    -1,    -1,    -1,   328,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   332,   339,
       0,     1,   342,    -1,    -1,    -1,    -1,    -1,   348,    -1,
     350,    -1,    12,    -1,    -1,    15,    16,   357,    18,    19,
      20,    21,    22,    23,    -1,    25,    -1,    27,    28,    29,
      -1,    -1,    -1,    33,    34,    35,    36,    -1,    -1,    39,
      -1,    -1,    -1,    -1,    44,    45,    46,    -1,    -1,    49,
      -1,    -1,    -1,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    -1,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    -1,
      -1,    12,    92,    93,    15,    16,    96,    18,    19,    20,
      21,    22,    23,    -1,    25,    -1,    27,    28,    29,    -1,
      -1,    -1,    33,    34,    35,    36,    -1,    -1,    39,    -1,
      -1,    -1,    -1,    44,    45,    46,    -1,    -1,    49,    -1,
      -1,    -1,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    -1,    -1,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    -1,    -1,
      12,    -1,    93,    15,    16,    96,    18,    19,    20,    21,
      22,    23,    -1,    25,    26,    27,    -1,    29,    -1,    -1,
      -1,    33,    34,    35,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    46,    -1,    -1,    49,    -1,    -1,
      -1,    -1,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    -1,    -1,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    -1,    -1,    12,
      92,    93,    15,    16,    96,    18,    19,    20,    21,    22,
      23,    -1,    25,    -1,    27,    -1,    29,    -1,    -1,    -1,
      33,    34,    35,    36,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    46,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    -1,    -1,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    -1,    -1,    12,    92,
      93,    15,    16,    96,    18,    19,    20,    21,    22,    23,
      -1,    25,    -1,    27,    -1,    29,    -1,    -1,    -1,    33,
      34,    35,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    -1,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    88,    -1,    -1,    12,    -1,    93,
      15,    16,    96,    18,    19,    20,    21,    22,    23,    -1,
      25,    -1,    27,    -1,    29,    -1,    -1,    -1,    33,    34,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      -1,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    -1,    -1,    12,    -1,    93,    15,
      16,    96,    18,    -1,    20,    21,    22,    23,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    -1,    40,    41,    42,    43,    44,    -1,
      46,    -1,    12,    -1,    -1,    15,    16,    -1,    18,    -1,
      20,    21,    22,    23,    -1,    25,    -1,    -1,    64,    29,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      40,    -1,    42,    -1,    44,    -1,    46,    -1,    -1,    -1,
      -1,    87,    88,    -1,    -1,    -1,    56,    93,    -1,    -1,
      96,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    -1,
      -1,    12,    -1,    93,    15,    16,    96,    18,    -1,    20,
      21,    22,    23,    -1,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    12,    46,    -1,    15,    16,    -1,
      18,    -1,    20,    21,    22,    23,    -1,    25,    -1,    -1,
      -1,    29,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    -1,    44,    -1,    46,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    -1,    -1,
      -1,    -1,    93,    -1,    -1,    96,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    37,    -1,    12,    -1,    93,    15,    16,    96,    18,
      -1,    20,    21,    22,    23,    -1,    25,    -1,    -1,    -1,
      29,    -1,    58,    59,    60,    61,    62,    63,    -1,    65,
      66,    67,    68,    69,    70,    44,    -1,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    84,    85,
      86,    87,    88,    89,    90,    64,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      40,    -1,    42,    -1,    -1,    -1,    -1,    -1,    87,    88,
      -1,    -1,    -1,    -1,    93,    -1,    -1,    96,    58,    59,
      60,    61,    62,    63,    -1,    65,    -1,    -1,    68,    69,
      70,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    84,    85,    86,    87,    88,    89,
      90,    58,    59,    60,    61,    62,    63,    -1,    65,    -1,
      -1,    68,    69,    70,    -1,    -1,    -1,    -1,    42,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    84,    85,    86,
      87,    88,    89,    90,    58,    59,    60,    61,    62,    63,
      -1,    65,    -1,    -1,    68,    69,    70,    -1,    -1,    -1,
      -1,    42,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      84,    85,    86,    87,    88,    89,    90,    58,    59,    60,
      61,    62,    63,    -1,    65,    -1,    -1,    68,    69,    70,
      -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    84,    85,    86,    87,    88,    89,    90,
      58,    59,    60,    61,    62,    63,    -1,    65,    -1,    -1,
      68,    69,    70,    -1,    -1,    -1,    -1,    42,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    84,    85,    86,    87,
      88,    89,    90,    58,    59,    60,    61,    62,    63,    -1,
      65,    -1,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    84,
      85,    86,    87,    88,    89,    90,    58,    59,    60,    61,
      62,    63,    -1,    65,    -1,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    84,    85,    86,    87,    88,    89,    90,    58,
      59,    60,    61,    62,    63,    -1,    65,    -1,    -1,    68,
      69,    70,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    84,    85,    86,    87,    88,
      89,    90,    58,    59,    60,    61,    62,    63,    -1,    65,
      -1,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    84,    85,
      86,    87,    88,    89,    90,    58,    59,    60,    61,    62,
      63,    -1,    65,    -1,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    84,    85,    86,    87,    88,    89,    90,    50,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    -1,    65,    -1,    -1,    68,    69,    70,    58,
      59,    60,    61,    62,    63,    -1,    65,    -1,    -1,    68,
      69,    70,    84,    85,    86,    87,    88,    89,    90,    -1,
      -1,    -1,    -1,    -1,    -1,    84,    85,    86,    87,    88,
      89,    90,    -1,    -1,    -1,    94,    58,    59,    60,    61,
      62,    63,    -1,    65,    -1,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    -1,    76,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    84,    85,    86,    87,    88,    89,    90,    58,
      59,    60,    61,    62,    63,    -1,    65,    -1,    -1,    68,
      69,    70,    58,    59,    60,    61,    -1,    -1,    -1,    65,
      -1,    -1,    68,    69,    70,    84,    85,    86,    87,    88,
      89,    90,    -1,    -1,    -1,    -1,    -1,    -1,    84,    85,
      86,    87,    88,    89,    90
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    15,    16,    18,    19,    20,    21,    22,
      23,    25,    27,    28,    29,    33,    34,    35,    36,    39,
      44,    45,    46,    49,    53,    54,    55,    57,    64,    71,
      72,    73,    74,    75,    78,    79,    80,    87,    88,    93,
      96,   100,   101,   102,   103,   105,   107,   111,   112,   113,
     118,   119,   120,   121,   122,   123,   128,   130,   132,   136,
     141,   144,    92,    22,    44,   120,    93,   120,   142,    93,
     142,    22,   119,    93,   120,   143,   131,   135,    45,   124,
      38,    40,    41,    42,    43,   120,   129,   133,   120,    50,
      51,    15,    16,    17,    22,    29,    22,    22,   120,    56,
     120,   129,    22,   127,   120,   120,   120,   120,   120,   120,
     120,    22,   126,   137,   120,   120,   120,    23,    25,    98,
     119,   145,   146,   147,   148,     0,    92,   120,    60,   120,
     104,   110,   113,   120,   123,   141,    58,    59,    60,    61,
      62,    63,    65,    68,    69,    70,    84,    85,    86,    87,
      88,    89,    90,   115,    93,   120,   120,   142,    95,   142,
     142,    95,   142,   115,    22,   125,   120,    40,   129,    22,
      37,    66,    67,    22,    48,    60,   119,   119,    48,    60,
      48,    47,    50,   129,    95,    42,    48,    42,    48,    42,
      48,    42,    48,    76,    77,    48,    92,    95,    22,    94,
      98,    98,    97,    95,    95,   101,   116,   120,   116,   116,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,    22,   138,
      94,   120,    94,    94,   120,   138,    95,   120,   134,    22,
     122,   120,   120,   120,   120,    48,    60,    42,   120,   120,
     120,   120,    22,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   106,   106,    22,   119,   119,   119,    23,
      25,   148,   117,   116,    47,   116,    92,    95,    92,    22,
     115,   120,   120,    22,    42,    42,    42,    42,   103,   117,
      92,   105,   116,    30,    48,   117,    47,    95,   140,    22,
     140,   138,   119,   120,   120,   120,   120,   116,   116,    82,
      48,   120,   116,   103,    22,   117,   117,    92,    48,    83,
     120,   116,   108,   116,    26,   140,   120,   116,   117,    31,
      32,   109,   114,   116,    31,    81,    22,   139,   117,   117,
     116,   115,   120,    81,   115,    95,   116,    82,   117,   116,
     103,    22,    82,   116,    47,   116,    81,   117,    81,   116
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    99,   100,   100,   100,   101,   101,   101,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     103,   103,   103,   104,   104,   104,   104,   105,   105,   105,
     105,   105,   105,   105,   105,   106,   107,   107,   108,   108,
     109,   110,   110,   111,   112,   113,   114,   115,   116,   117,
     117,   117,   118,   119,   119,   119,   119,   119,   119,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   121,   121,   121,   121,   121,   121,   121,   121,   122,
     122,   123,   123,   123,   123,   123,   124,   123,   123,   123,
     123,   123,   123,   123,   125,   125,   126,   126,   127,   127,
     128,   128,   128,   128,   128,   128,   129,   129,   130,   130,
     130,   130,   131,   130,   133,   134,   132,   132,   135,   132,
     132,   132,   137,   136,   138,   138,   138,   138,   139,   139,
     139,   140,   141,   142,   142,   142,   143,   143,   144,   145,
     145,   145,   145,   146,   146,   147,   147,   148,   148
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     1,     2,     0,     1,     1,     4,     4,
       4,     4,     4,     4,     4,     4,     5,     5,     8,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
      10,    11,     3,     7,     6,     0,     9,    12,     0,     2,
       6,    11,     7,     2,     3,     1,     1,     0,     0,     0,
       2,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     2,     4,     1,     2,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     3,     3,     3,     2,     2,     3,     4,
       4,     4,     6,     4,     6,     4,     6,     4,     6,     2,
       1,     2,     1,     1,     2,     1,     0,     3,     2,     2,
       4,     2,     4,     2,     1,     3,     1,     3,     1,     3,
       2,     2,     2,     2,     3,     2,     3,     2,     2,     2,
       3,     2,     0,     3,     0,     0,     9,     2,     0,     7,
       8,     6,     0,     3,     0,     1,     3,     4,     0,     1,
       3,     0,     2,     0,     1,     3,     1,     3,     3,     0,
       1,     1,     1,     1,     3,     1,     3,     3,     3
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
#line 1671 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 20: /* FBLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1677 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 21: /* RBLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1683 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 22: /* ID  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1689 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* STRING  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1695 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* HANDLER  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1701 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* SYMBOL  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1707 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* ENDCLAUSE  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1713 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* tPLAYACCEL  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1719 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* tMETHOD  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1725 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* THEOBJECTFIELD  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).os; }
#line 1731 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 136: /* on  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1737 "engines/director/lingo/lingo-gr.cpp"
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
#line 2007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 143 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2018 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 149 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2026 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 152 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_after); }
#line 2032 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 153 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code1(LC::c_before); }
#line 2038 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 154 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2049 "engines/director/lingo/lingo-gr.cpp"
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
#line 2061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 167 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2072 "engines/director/lingo/lingo-gr.cpp"
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
#line 2084 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 180 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2095 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 186 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2106 "engines/director/lingo/lingo-gr.cpp"
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
#line 2121 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 203 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).oe);
		delete (yyvsp[-2].objectfield).os;
		(yyval.code) = (yyvsp[0].code); }
#line 2132 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 225 "engines/director/lingo/lingo-gr.y"
                                                                        {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[-3].code) - (yyvsp[-5].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 2] = end; }
#line 2143 "engines/director/lingo/lingo-gr.cpp"
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
#line 2160 "engines/director/lingo/lingo-gr.cpp"
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
#line 2177 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 265 "engines/director/lingo/lingo-gr.y"
                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2187 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 270 "engines/director/lingo/lingo-gr.y"
                                                          {
		inst end;
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code));
		(*g_lingo->_currentScript)[(yyvsp[-3].code) + 1] = end; }
#line 2196 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 274 "engines/director/lingo/lingo-gr.y"
                                                    {
		inst end;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 279 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }
#line 2213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 283 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-6].code) - (yyvsp[-8].code));
		WRITE_UINT32(&else1, (yyvsp[-3].code) - (yyvsp[-8].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-8].code));
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-8].code), (yyvsp[-1].code) - (yyvsp[-8].code), 0); }
#line 2228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 293 "engines/director/lingo/lingo-gr.y"
                                                                                                              {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-9].code) - (yyvsp[-11].code));
		WRITE_UINT32(&else1, (yyvsp[-6].code) - (yyvsp[-11].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-11].code));
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-11].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-11].code), (yyvsp[-1].code) - (yyvsp[-11].code), (yyvsp[-3].code) - (yyvsp[-11].code)); }
#line 2243 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 307 "engines/director/lingo/lingo-gr.y"
                                                        {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-3].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2254 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 314 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-8].code) - (yyvsp[-10].code));
		WRITE_UINT32(&else1, (yyvsp[-5].code) - (yyvsp[-10].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-10].code));
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-10].code), (yyvsp[-1].code) - (yyvsp[-10].code), (yyvsp[-3].code) - (yyvsp[-10].code)); }
#line 2269 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 324 "engines/director/lingo/lingo-gr.y"
                                                                   {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-4].code) - (yyvsp[-6].code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code));
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code)); }
#line 2284 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 335 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code3(LC::c_repeatwhilecode, STOP, STOP); }
#line 2290 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 337 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code3(LC::c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2300 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 343 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); }
#line 2310 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 349 "engines/director/lingo/lingo-gr.y"
                                        {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); }
#line 2321 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2327 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 358 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2333 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 360 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 364 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2349 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 370 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2357 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 373 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2365 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 376 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2374 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 380 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2383 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 384 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2392 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 390 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2398 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 392 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2406 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 395 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2414 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 398 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2422 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 401 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2435 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 409 "engines/director/lingo/lingo-gr.y"
                               {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2446 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 415 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).oe);
		delete (yyvsp[0].objectfield).os; }
#line 2456 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 421 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2462 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 422 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2468 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 423 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2474 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 424 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2480 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 425 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2486 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 426 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 427 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2498 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 428 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2504 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 429 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2510 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 430 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 431 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2522 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 432 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2528 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 433 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2534 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 434 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2540 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 435 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2546 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 436 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2552 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 437 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2558 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 438 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2564 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 439 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); }
#line 2570 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 440 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 441 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 442 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_intersects); }
#line 2588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 443 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_within); }
#line 2594 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 445 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_charOf); }
#line 2600 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 446 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charToOf); }
#line 2606 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 447 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_itemOf); }
#line 2612 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 448 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemToOf); }
#line 2618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 449 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_lineOf); }
#line 2624 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 450 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineToOf); }
#line 2630 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 451 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_wordOf); }
#line 2636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 452 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordToOf); }
#line 2642 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 454 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2650 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 459 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_printtop); }
#line 2656 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 462 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_exitRepeat); }
#line 2662 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 463 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_procret); }
#line 2668 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 464 "engines/director/lingo/lingo-gr.y"
                  { g_lingo->_indef = kStateInArgs; }
#line 2674 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 464 "engines/director/lingo/lingo-gr.y"
                                                                 { g_lingo->_indef = kStateNone; }
#line 2680 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 467 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2688 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 470 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2696 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 473 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_open); }
#line 2702 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 474 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 2708 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 476 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 480 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 485 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 489 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2744 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 494 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 498 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2762 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 510 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 2768 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 511 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 2774 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 512 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 2780 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 513 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 2789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 517 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 2798 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 521 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 2807 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 529 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_playdone); }
#line 2813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 530 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 2822 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 534 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 2831 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 538 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 2840 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 542 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 2846 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 542 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 2855 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 572 "engines/director/lingo/lingo-gr.y"
             { g_lingo->_indef = kStateInArgs; }
#line 2861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 572 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->_currentFactory.clear(); }
#line 2867 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 573 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete (yyvsp[-6].s); }
#line 2878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 579 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 2884 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 580 "engines/director/lingo/lingo-gr.y"
                  { g_lingo->_indef = kStateInArgs; }
#line 2890 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 581 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete (yyvsp[-6].s); }
#line 2901 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 587 "engines/director/lingo/lingo-gr.y"
                                                                     {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 2916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 597 "engines/director/lingo/lingo-gr.y"
                                                 {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->_indef = kStateNone;
		g_lingo->clearArgStack();
		g_lingo->_ignoreMe = false;
		delete (yyvsp[-5].s); }
#line 2928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 605 "engines/director/lingo/lingo-gr.y"
         { g_lingo->_indef = kStateInArgs; }
#line 2934 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 605 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.s) = (yyvsp[0].s); g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 2940 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 607 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 2946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 608 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 2952 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 609 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 2958 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 610 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-3].narg) + 1; delete (yyvsp[0].s); }
#line 2964 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 613 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2970 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 614 "engines/director/lingo/lingo-gr.y"
                                        { delete (yyvsp[0].s); }
#line 2976 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 616 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArgStore(); g_lingo->_indef = kStateInDef; }
#line 2982 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 618 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 2994 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 626 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 3000 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 627 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3006 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 628 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3012 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 630 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3018 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 631 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 633 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = (yyvsp[-1].code); }
#line 3030 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 635 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3036 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 636 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3042 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 637 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3048 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 638 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3054 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 640 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3060 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 641 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3066 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 643 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3072 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 644 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3078 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 646 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3087 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 650 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3096 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3100 "engines/director/lingo/lingo-gr.cpp"

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
#line 656 "engines/director/lingo/lingo-gr.y"

