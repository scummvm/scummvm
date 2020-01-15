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
		Common::String *os;
		int oe;
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
#define YYFINAL  124
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1600

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  98
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  48
/* YYNRULES -- Number of rules.  */
#define YYNRULES  165
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  351

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
       0,   135,   135,   136,   137,   139,   140,   141,   143,   149,
     152,   153,   154,   160,   167,   173,   180,   186,   192,   199,
     200,   201,   203,   204,   205,   206,   208,   209,   214,   225,
     242,   254,   259,   263,   268,   272,   282,   293,   294,   296,
     303,   313,   324,   326,   332,   338,   345,   347,   349,   350,
     351,   353,   359,   362,   365,   369,   373,   377,   379,   380,
     381,   384,   387,   390,   398,   404,   409,   410,   411,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   442,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   457,   460,
     461,   463,   467,   472,   476,   481,   485,   497,   498,   499,
     500,   504,   508,   513,   514,   516,   517,   521,   525,   529,
     529,   559,   559,   559,   566,   567,   567,   574,   584,   592,
     592,   594,   595,   596,   597,   599,   600,   601,   603,   605,
     613,   614,   615,   617,   618,   620,   622,   623,   624,   625,
     627,   628,   630,   631,   633,   637
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
  "stmtoneliner", "stmtonelinerwithif", "stmt", "tellstart", "ifstmt",
  "elseifstmtlist", "elseifstmt", "ifoneliner", "repeatwhile",
  "repeatwith", "if", "elseif", "begin", "end", "stmtlist", "when",
  "simpleexpr", "expr", "reference", "proc", "globallist", "propertylist",
  "instancelist", "gotofunc", "gotomovie", "playfunc", "$@1", "defn",
  "$@2", "$@3", "$@4", "on", "$@5", "argdef", "endargdef", "argstore",
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
     335,   336,   337,    60,    62,    38,    43,    45,    42,    47,
      37,    10,    40,    41,    44,    91,    93,    58
};
# endif

#define YYPACT_NINF (-264)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-6)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     270,   -72,  -264,  -264,   873,  -264,   907,   988,    93,  1022,
    -264,  -264,  -264,  -264,  -264,   -21,    21,   756,  -264,  -264,
     873,     3,    76,    27,    55,   873,   792,    59,   873,   873,
     873,   873,   873,   873,   873,    62,  -264,   873,   873,   873,
      57,     8,  -264,  -264,  -264,  -264,  -264,   873,    37,   873,
     594,  -264,  1511,  -264,  -264,  -264,  -264,  -264,  -264,  -264,
    -264,  -264,    -6,   873,  1511,   873,  1511,    19,   873,    19,
    -264,  -264,   873,  1511,    28,   873,  -264,  -264,  -264,    30,
    -264,   873,  -264,    63,  -264,  1074,  -264,    77,  1061,   100,
    -264,    -8,    93,    11,    79,    84,  -264,  1428,  -264,  1074,
    -264,  -264,    54,  -264,  1123,  1156,  1189,  1222,  1478,  1387,
    -264,    56,   130,  -264,  -264,  1441,    58,    60,  -264,  -264,
      66,    64,    65,  -264,  -264,   351,  1511,   873,  1511,  -264,
    -264,   873,  1511,  -264,  -264,   873,   873,   873,   873,   873,
     873,   873,   873,   873,   873,   873,   873,   873,   873,   873,
     873,   873,   142,   873,  1061,  1441,   -27,   873,     7,    13,
     873,    19,   142,   143,  1511,   873,  -264,  -264,    91,   873,
     873,  -264,   873,   873,    35,   873,   873,   873,  -264,   873,
    -264,   153,   873,   873,   873,   873,   873,   873,   873,   873,
     873,   873,  -264,  -264,   155,  -264,  -264,    93,    93,  -264,
      93,    71,  -264,  -264,  1511,   148,  -264,  1511,   170,   170,
     170,   170,   133,   133,  -264,   -32,   170,   170,   170,   170,
     -32,   -52,   -52,  -264,  -264,  -264,   -76,  -264,  1511,  -264,
    -264,  1511,   -74,  -264,  1511,  -264,  -264,  -264,  1511,  1511,
    1511,   170,   873,   873,  1511,   170,  1511,  1511,  -264,  1511,
    1255,  1511,  1288,  1511,  1321,  1511,  1354,  1511,  1511,   675,
    -264,  -264,  -264,  -264,  -264,    58,    60,  -264,   513,    15,
    -264,   159,   105,   185,   105,   142,  1511,   170,   873,   873,
     873,   873,  -264,   513,  -264,  -264,   126,   161,   873,   513,
     675,   188,  -264,  -264,  -264,   -19,  1511,  1511,  1511,  1511,
    -264,   129,  -264,   873,  1511,  -264,  -264,  -264,   432,   513,
     105,  -264,  1511,  -264,    88,   -20,   191,  -264,  -264,   513,
    -264,  -264,  -264,   873,   134,  -264,  -264,  -264,   119,   513,
     513,   145,  -264,  1511,  -264,   675,   194,   147,  -264,   513,
     177,  -264,  -264,  -264,   144,  -264,   149,  -264,   513,  -264,
    -264
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    52,    63,     0,    53,   150,   150,     0,    56,
      55,    54,   129,   135,    65,   103,     0,     0,    44,   131,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   139,     0,     0,     0,
     156,     0,     3,    66,    26,     7,    27,     0,     0,     0,
       0,    58,    20,    59,    21,   100,   101,     6,    46,    19,
      57,     4,    56,     0,    64,   150,   151,   108,   150,    61,
      56,    98,   150,   153,   149,   150,    46,   102,   111,   104,
     117,     0,   118,     0,   119,   120,   122,     0,    99,     0,
      42,     0,     0,     0,     0,     0,   134,   110,   125,   126,
     128,   115,   106,    80,     0,     0,     0,     0,     0,     0,
     113,   105,     0,    85,    86,     0,    55,    54,   157,   160,
       0,   158,   159,   162,     1,     5,    47,     0,    47,    47,
      25,     0,    23,    24,    22,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   141,   150,     0,   151,     0,     0,     0,     0,
       0,   130,   141,     0,   124,     0,   121,   132,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,    51,     0,
     127,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    34,    34,     0,   140,    87,     0,     0,   155,
       0,     0,     2,    48,    47,     0,    31,    47,    76,    77,
      74,    75,    78,    79,    71,    82,    83,    84,    73,    72,
      81,    67,    68,    69,    70,   142,     0,   107,   152,    60,
      62,   154,     0,   112,   123,    46,     8,     9,    10,    11,
      15,    13,     0,     0,    14,    12,    18,   109,   116,    90,
       0,    92,     0,    94,     0,    96,     0,    88,    89,     0,
      48,   114,   165,   164,   161,     0,     0,   163,    47,     0,
      48,     0,   148,     0,   148,   141,    16,    17,     0,     0,
       0,     0,    47,    47,    49,    50,     0,     0,     0,    47,
       0,     0,    48,   143,    48,     0,    91,    93,    95,    97,
      33,     0,    28,     0,    47,    37,    47,   144,   138,   136,
     148,    32,    47,    48,    47,     0,   145,    48,    48,    47,
      46,    45,    38,     0,     0,    46,    41,   146,   137,   133,
      47,     0,    48,    47,    35,     0,     0,     0,    29,    47,
       0,    47,   147,    30,     0,    48,     0,    36,    47,    40,
      39
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -264,  -264,   106,  -264,  -250,  -264,     4,    39,  -264,  -264,
    -264,  -264,  -264,  -264,   180,  -264,   -75,   -79,  -229,  -264,
      -2,    -4,    68,   192,  -264,  -264,  -264,  -264,   -12,  -264,
    -264,  -264,  -264,  -264,  -264,  -264,  -264,  -155,  -264,  -263,
     193,   102,  -264,  -264,  -264,  -264,  -264,    40
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    41,    42,    43,    44,   129,   285,   259,    46,   314,
     322,   130,    47,    48,    49,   323,   152,   203,   268,    50,
      51,    52,    53,    54,    79,   111,   102,    55,    86,    56,
      75,    57,    87,   235,    76,    58,   112,   226,   328,   292,
      59,   159,    74,    60,   120,   121,   122,   123
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      64,   162,    66,    66,    45,    73,    71,   232,   124,   282,
     325,   294,   141,    85,   100,   272,    88,   274,   273,    61,
     273,    97,    99,    77,   103,   104,   105,   106,   107,   108,
     109,   283,   141,   113,   114,   115,   150,   151,   119,   172,
     306,   289,    78,   126,   287,   128,   132,   317,    95,   205,
     206,   173,    89,    90,   148,   149,   150,   151,   175,   154,
     326,   155,   288,   308,   155,   309,   227,   157,   155,     2,
     176,    66,   310,   166,     5,   273,    96,   164,    70,   116,
     101,   117,   242,   110,   319,   341,   153,   180,   329,   330,
     174,    91,    92,   265,   243,   266,   127,    93,   167,   125,
     229,   157,   165,   339,    94,     2,   230,   157,    67,    69,
       5,     8,   236,   157,    70,    10,   348,    11,   320,   321,
     295,   171,   160,   204,   163,   269,   177,   207,   271,    45,
     178,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   181,    66,
     194,   195,    40,   228,   118,   197,   231,   198,   200,   201,
     275,   234,   199,   225,   233,   238,   239,   156,   240,   241,
     158,   244,   245,   246,   248,   247,   261,   161,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,    40,   286,
     135,   136,   137,   138,   270,   262,   263,   141,   264,   291,
     142,   143,   144,   300,   301,   290,   293,   302,   303,   307,
     305,   311,   327,   336,   334,   342,   145,   146,   147,   148,
     149,   150,   151,   345,   347,   313,   338,   315,   343,   349,
     131,   202,   260,   318,   141,   324,   237,   142,   276,   277,
     331,   267,   133,   134,     0,   332,     0,     0,     0,     0,
     335,   337,     0,     0,   340,   147,   148,   149,   150,   151,
     344,     0,   346,     0,     0,     0,     0,     0,     0,   350,
      -5,     1,     0,     0,   296,   297,   298,   299,     0,     0,
       0,     0,     2,     0,   304,     3,     4,     5,     6,     7,
       8,     9,    10,     0,    11,     0,    12,    13,    14,   312,
       0,     0,    15,    16,    17,    18,     0,     0,    19,     0,
       0,     0,     0,    20,    21,    22,     0,     0,    23,   333,
       0,     0,    24,    25,    26,     0,    27,     0,     0,     0,
       0,     0,     0,    28,     0,     0,     0,     0,     0,     0,
      29,    30,    31,    32,    33,     0,     0,    34,    35,    36,
       0,     0,     0,     0,     0,     0,    37,    38,     0,     0,
       0,    -5,    39,     2,     0,    40,     3,     4,     5,     6,
       7,     8,     9,    10,     0,    11,     0,    12,    13,    14,
       0,     0,     0,    15,    16,    17,    18,     0,     0,    19,
       0,     0,     0,     0,    20,    21,    22,     0,     0,    23,
       0,     0,     0,    24,    25,    26,     0,    27,     0,     0,
       0,     0,     0,     0,    28,     0,     0,     0,     0,     0,
       0,    29,    30,    31,    32,    33,     0,     0,    34,    35,
      36,     0,     0,     0,     0,     0,     0,    37,    38,     0,
       0,     0,     0,    39,     2,     0,    40,     3,     4,     5,
       6,     7,     8,     9,    10,     0,    11,   316,    12,     0,
      14,     0,     0,     0,    15,    16,    17,    18,     0,     0,
       0,     0,     0,     0,     0,    20,    21,    22,     0,     0,
      23,     0,     0,     0,     0,    25,    26,     0,    27,     0,
       0,     0,     0,     0,     0,    28,     0,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,     0,    34,
      35,     0,     0,     0,     0,     0,     0,     0,    37,    38,
       0,     0,     0,   284,    39,     2,     0,    40,     3,     4,
       5,     6,     7,     8,     9,    10,     0,    11,     0,    12,
       0,    14,     0,     0,     0,    15,    16,    17,    18,     0,
       0,     0,     0,     0,     0,     0,    20,    21,    22,     0,
       0,    23,     0,     0,     0,     0,    25,    26,     0,    27,
       0,     0,     0,     0,     0,     0,    28,     0,     0,     0,
       0,     0,     0,    29,    30,    31,    32,    33,     0,     0,
      34,    35,     0,     0,     0,     0,     0,     0,     0,    37,
      38,     0,     0,     0,   284,    39,     2,     0,    40,     3,
       4,     5,     6,     7,     8,     9,    10,     0,    11,     0,
      12,     0,    14,     0,     0,     0,    15,    16,    17,    18,
       0,     0,     0,     0,     0,     0,     0,    20,     0,    22,
       0,     0,     0,     0,     0,     0,     0,    25,    26,     0,
      27,     0,     0,     0,     0,     0,     0,    28,     0,     0,
       0,     0,     0,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    35,     0,     0,     0,     0,     0,     0,     0,
      37,    38,     0,     0,     0,     0,    39,     2,     0,    40,
       3,     4,     5,     6,     7,     8,     9,    10,     0,    11,
       0,    12,     0,    14,     0,     0,     0,    15,    16,    17,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
      22,     0,     0,     0,     0,     0,     0,     0,    25,    26,
       0,    27,     0,     0,     0,     0,     0,     0,    28,     0,
       0,     0,     0,     0,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    35,     0,     0,     0,     0,     0,     0,
       0,    37,    38,     0,     0,     0,     0,    39,     2,     0,
      40,     3,     4,     5,     0,     7,     8,    62,    10,     0,
      11,     0,     0,     0,    14,     0,     0,     0,     0,     0,
       0,     0,     0,    80,     0,    81,    82,    83,    84,    63,
       0,    22,     0,     0,     2,     0,     0,     3,     4,     5,
       0,     7,     8,    62,    10,     0,    11,     0,     0,    28,
      14,     0,     0,     0,     0,     0,    29,    30,    31,    32,
      33,    81,     0,    83,     0,    63,     0,    22,     0,     0,
       0,     0,    37,    38,     0,     0,     0,    98,    39,     0,
       0,    40,     0,     0,     0,    28,     0,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    37,    38,
       0,     0,     0,     0,    39,     2,     0,    40,     3,     4,
       5,     0,     7,     8,    62,    10,     0,    11,     0,     0,
       0,    14,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    63,     0,    22,     2,
       0,     0,     3,     4,     5,     0,     7,     8,    62,    10,
       0,    11,     0,     0,     0,    14,    28,     0,     0,     0,
       0,     0,     0,    29,    30,    31,    32,    33,     0,     0,
      63,     0,    22,     0,     0,     0,     0,     0,     0,    37,
      38,     0,     0,     0,     0,    39,     0,     0,    40,     0,
      28,     0,     0,     0,     0,     0,     0,    29,    30,    31,
      32,    33,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    37,    38,     0,     0,     0,     0,    65,
       2,     0,    40,     3,     4,     5,     0,     7,     8,    62,
      10,     0,    11,     0,     0,     0,    14,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    63,     0,    22,     2,     0,     0,     3,     4,     5,
       0,     7,     8,    62,    10,     0,    11,     0,     0,     0,
      14,    28,     0,     0,     0,     0,     0,     0,    29,    30,
      31,    32,    33,     0,     0,    63,     0,    22,     0,     0,
       0,     0,     0,     0,    37,    38,     0,     0,     0,     0,
      68,     0,     0,    40,     0,    28,     0,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,   168,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    37,    38,
       0,     0,     0,    81,    72,    83,     0,    40,   135,   136,
     137,   138,   139,   140,     0,   141,   169,   170,   142,   143,
     144,   135,   136,   137,   138,   139,   140,     0,   141,     0,
       0,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,     0,     0,     0,     0,     0,     0,   145,   146,   147,
     148,   149,   150,   151,   182,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     135,   136,   137,   138,   139,   140,     0,   141,     0,     0,
     142,   143,   144,     0,     0,     0,     0,   184,     0,     0,
       0,     0,     0,   185,     0,     0,   145,   146,   147,   148,
     149,   150,   151,   135,   136,   137,   138,   139,   140,     0,
     141,     0,     0,   142,   143,   144,     0,     0,     0,     0,
     186,     0,     0,     0,     0,     0,   187,     0,     0,   145,
     146,   147,   148,   149,   150,   151,   135,   136,   137,   138,
     139,   140,     0,   141,     0,     0,   142,   143,   144,     0,
       0,     0,     0,   188,     0,     0,     0,     0,     0,   189,
       0,     0,   145,   146,   147,   148,   149,   150,   151,   135,
     136,   137,   138,   139,   140,     0,   141,     0,     0,   142,
     143,   144,     0,     0,     0,     0,   278,     0,     0,     0,
       0,     0,     0,     0,     0,   145,   146,   147,   148,   149,
     150,   151,   135,   136,   137,   138,   139,   140,     0,   141,
       0,     0,   142,   143,   144,     0,     0,     0,     0,   279,
       0,     0,     0,     0,     0,     0,     0,     0,   145,   146,
     147,   148,   149,   150,   151,   135,   136,   137,   138,   139,
     140,     0,   141,     0,     0,   142,   143,   144,     0,     0,
       0,     0,   280,     0,     0,     0,     0,     0,     0,     0,
       0,   145,   146,   147,   148,   149,   150,   151,   135,   136,
     137,   138,   139,   140,     0,   141,     0,     0,   142,   143,
     144,     0,     0,     0,     0,   281,     0,     0,     0,     0,
       0,     0,     0,     0,   145,   146,   147,   148,   149,   150,
     151,   135,   136,   137,   138,   139,   140,     0,   141,     0,
       0,   142,   143,   144,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   192,     0,     0,   145,   146,   147,
     148,   149,   150,   151,   135,   136,   137,   138,   139,   140,
       0,   141,     0,     0,   142,   143,   144,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     145,   146,   147,   148,   149,   150,   151,   179,   193,     0,
       0,     0,     0,     0,     0,   135,   136,   137,   138,   139,
     140,     0,   141,     0,     0,   142,   143,   144,   135,   136,
     137,   138,   139,   140,     0,   141,     0,     0,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,     0,     0,
       0,     0,     0,     0,   145,   146,   147,   148,   149,   150,
     151,     0,     0,     0,   196,   135,   136,   137,   138,   139,
     140,     0,   141,     0,     0,   142,   143,   144,     0,     0,
       0,     0,     0,   190,   191,     0,     0,     0,     0,     0,
       0,   145,   146,   147,   148,   149,   150,   151,   135,   136,
     137,   138,   139,   140,     0,   141,     0,     0,   142,   143,
     144,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   145,   146,   147,   148,   149,   150,
     151
};

static const yytype_int16 yycheck[] =
{
       4,    76,     6,     7,     0,     9,     8,   162,     0,   259,
      30,   274,    64,    17,    26,    91,    20,    91,    94,    91,
      94,    25,    26,    44,    28,    29,    30,    31,    32,    33,
      34,   260,    64,    37,    38,    39,    88,    89,    40,    47,
     290,   270,    21,    47,    29,    49,    50,   310,    21,   128,
     129,    59,    49,    50,    86,    87,    88,    89,    47,    63,
      80,    65,    47,   292,    68,   294,    93,    94,    72,    12,
      59,    75,    91,    85,    17,    94,    21,    81,    21,    22,
      21,    24,    47,    21,   313,   335,    92,    99,   317,   318,
      92,    15,    16,    22,    59,    24,    59,    21,    21,    91,
      93,    94,    39,   332,    28,    12,    93,    94,     6,     7,
      17,    20,    21,    94,    21,    22,   345,    24,    30,    31,
     275,    21,    94,   127,    94,   204,    47,   131,   207,   125,
      46,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,    94,   153,
      94,    21,    95,   157,    97,    97,   160,    97,    94,    94,
     235,   165,    96,    21,    21,   169,   170,    65,   172,   173,
      68,   175,   176,   177,    21,   179,    21,    75,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    95,   268,
      57,    58,    59,    60,    46,   197,   198,    64,   200,    94,
      67,    68,    69,   282,   283,    46,    21,    81,    47,    21,
     289,    82,    21,    94,    80,    21,    83,    84,    85,    86,
      87,    88,    89,    46,    80,   304,    81,   306,    81,    80,
      50,   125,   193,   312,    64,   314,   168,    67,   242,   243,
     319,   201,    50,    50,    -1,   320,    -1,    -1,    -1,    -1,
     325,   330,    -1,    -1,   333,    85,    86,    87,    88,    89,
     339,    -1,   341,    -1,    -1,    -1,    -1,    -1,    -1,   348,
       0,     1,    -1,    -1,   278,   279,   280,   281,    -1,    -1,
      -1,    -1,    12,    -1,   288,    15,    16,    17,    18,    19,
      20,    21,    22,    -1,    24,    -1,    26,    27,    28,   303,
      -1,    -1,    32,    33,    34,    35,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,   323,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    12,    -1,    95,    15,    16,    17,    18,
      19,    20,    21,    22,    -1,    24,    -1,    26,    27,    28,
      -1,    -1,    -1,    32,    33,    34,    35,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    48,
      -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    -1,
      -1,    -1,    -1,    92,    12,    -1,    95,    15,    16,    17,
      18,    19,    20,    21,    22,    -1,    24,    25,    26,    -1,
      28,    -1,    -1,    -1,    32,    33,    34,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
      48,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    12,    -1,    95,    15,    16,
      17,    18,    19,    20,    21,    22,    -1,    24,    -1,    26,
      -1,    28,    -1,    -1,    -1,    32,    33,    34,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      -1,    48,    -1,    -1,    -1,    -1,    53,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    12,    -1,    95,    15,
      16,    17,    18,    19,    20,    21,    22,    -1,    24,    -1,
      26,    -1,    28,    -1,    -1,    -1,    32,    33,    34,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    -1,    -1,    -1,    -1,    92,    12,    -1,    95,
      15,    16,    17,    18,    19,    20,    21,    22,    -1,    24,
      -1,    26,    -1,    28,    -1,    -1,    -1,    32,    33,    34,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    87,    -1,    -1,    -1,    -1,    92,    12,    -1,
      95,    15,    16,    17,    -1,    19,    20,    21,    22,    -1,
      24,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    -1,    39,    40,    41,    42,    43,
      -1,    45,    -1,    -1,    12,    -1,    -1,    15,    16,    17,
      -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,    63,
      28,    -1,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    39,    -1,    41,    -1,    43,    -1,    45,    -1,    -1,
      -1,    -1,    86,    87,    -1,    -1,    -1,    55,    92,    -1,
      -1,    95,    -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      -1,    -1,    -1,    -1,    92,    12,    -1,    95,    15,    16,
      17,    -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    45,    12,
      -1,    -1,    15,    16,    17,    -1,    19,    20,    21,    22,
      -1,    24,    -1,    -1,    -1,    28,    63,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      43,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,    -1,    -1,    -1,    -1,    92,    -1,    -1,    95,    -1,
      63,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    -1,    -1,    -1,    -1,    92,
      12,    -1,    95,    15,    16,    17,    -1,    19,    20,    21,
      22,    -1,    24,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    -1,    45,    12,    -1,    -1,    15,    16,    17,
      -1,    19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
      28,    63,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    43,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    -1,    -1,    -1,    -1,
      92,    -1,    -1,    95,    -1,    63,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      -1,    -1,    -1,    39,    92,    41,    -1,    95,    57,    58,
      59,    60,    61,    62,    -1,    64,    65,    66,    67,    68,
      69,    57,    58,    59,    60,    61,    62,    -1,    64,    -1,
      -1,    67,    68,    69,    83,    84,    85,    86,    87,    88,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    83,    84,    85,
      86,    87,    88,    89,    41,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    58,    59,    60,    61,    62,    -1,    64,    -1,    -1,
      67,    68,    69,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,    57,    58,    59,    60,    61,    62,    -1,
      64,    -1,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,
      41,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,    83,
      84,    85,    86,    87,    88,    89,    57,    58,    59,    60,
      61,    62,    -1,    64,    -1,    -1,    67,    68,    69,    -1,
      -1,    -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    83,    84,    85,    86,    87,    88,    89,    57,
      58,    59,    60,    61,    62,    -1,    64,    -1,    -1,    67,
      68,    69,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,
      88,    89,    57,    58,    59,    60,    61,    62,    -1,    64,
      -1,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,    84,
      85,    86,    87,    88,    89,    57,    58,    59,    60,    61,
      62,    -1,    64,    -1,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89,    57,    58,
      59,    60,    61,    62,    -1,    64,    -1,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,    88,
      89,    57,    58,    59,    60,    61,    62,    -1,    64,    -1,
      -1,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    83,    84,    85,
      86,    87,    88,    89,    57,    58,    59,    60,    61,    62,
      -1,    64,    -1,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      83,    84,    85,    86,    87,    88,    89,    49,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    58,    59,    60,    61,
      62,    -1,    64,    -1,    -1,    67,    68,    69,    57,    58,
      59,    60,    61,    62,    -1,    64,    -1,    -1,    67,    68,
      69,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,    88,
      89,    -1,    -1,    -1,    93,    57,    58,    59,    60,    61,
      62,    -1,    64,    -1,    -1,    67,    68,    69,    -1,    -1,
      -1,    -1,    -1,    75,    76,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89,    57,    58,
      59,    60,    61,    62,    -1,    64,    -1,    -1,    67,    68,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    83,    84,    85,    86,    87,    88,
      89
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    15,    16,    17,    18,    19,    20,    21,
      22,    24,    26,    27,    28,    32,    33,    34,    35,    38,
      43,    44,    45,    48,    52,    53,    54,    56,    63,    70,
      71,    72,    73,    74,    77,    78,    79,    86,    87,    92,
      95,    99,   100,   101,   102,   104,   106,   110,   111,   112,
     117,   118,   119,   120,   121,   125,   127,   129,   133,   138,
     141,    91,    21,    43,   119,    92,   119,   139,    92,   139,
      21,   118,    92,   119,   140,   128,   132,    44,    21,   122,
      37,    39,    40,    41,    42,   119,   126,   130,   119,    49,
      50,    15,    16,    21,    28,    21,    21,   119,    55,   119,
     126,    21,   124,   119,   119,   119,   119,   119,   119,   119,
      21,   123,   134,   119,   119,   119,    22,    24,    97,   118,
     142,   143,   144,   145,     0,    91,   119,    59,   119,   103,
     109,   112,   119,   121,   138,    57,    58,    59,    60,    61,
      62,    64,    67,    68,    69,    83,    84,    85,    86,    87,
      88,    89,   114,    92,   119,   119,   139,    94,   139,   139,
      94,   139,   114,    94,   119,    39,   126,    21,    36,    65,
      66,    21,    47,    59,   118,    47,    59,    47,    46,    49,
     126,    94,    41,    47,    41,    47,    41,    47,    41,    47,
      75,    76,    47,    91,    94,    21,    93,    97,    97,    96,
      94,    94,   100,   115,   119,   115,   115,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,    21,   135,    93,   119,    93,
      93,   119,   135,    21,   119,   131,    21,   120,   119,   119,
     119,   119,    47,    59,   119,   119,   119,   119,    21,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   105,
     105,    21,   118,   118,   118,    22,    24,   145,   116,   115,
      46,   115,    91,    94,    91,   114,   119,   119,    41,    41,
      41,    41,   102,   116,    91,   104,   115,    29,    47,   116,
      46,    94,   137,    21,   137,   135,   119,   119,   119,   119,
     115,   115,    81,    47,   119,   115,   102,    21,   116,   116,
      91,    82,   119,   115,   107,   115,    25,   137,   115,   116,
      30,    31,   108,   113,   115,    30,    80,    21,   136,   116,
     116,   115,   114,   119,    80,   114,    94,   115,    81,   116,
     115,   102,    21,    81,   115,    46,   115,    80,   116,    80,
     115
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    98,    99,    99,    99,   100,   100,   100,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   102,
     102,   102,   103,   103,   103,   103,   104,   104,   104,   104,
     104,   104,   104,   104,   105,   106,   106,   107,   107,   108,
     109,   109,   110,   111,   112,   113,   114,   115,   116,   116,
     116,   117,   118,   118,   118,   118,   118,   118,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   120,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   122,   122,   123,   123,   124,   124,   125,   125,   125,
     125,   125,   125,   126,   126,   127,   127,   127,   127,   128,
     127,   130,   131,   129,   129,   132,   129,   129,   129,   134,
     133,   135,   135,   135,   135,   136,   136,   136,   137,   138,
     139,   139,   139,   140,   140,   141,   142,   142,   142,   142,
     143,   143,   144,   144,   145,   145
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     1,     2,     0,     1,     1,     4,     4,
       4,     4,     4,     4,     4,     4,     5,     5,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     6,    10,
      11,     3,     7,     6,     0,     9,    12,     0,     2,     6,
      11,     7,     2,     3,     1,     1,     0,     0,     0,     2,
       2,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     2,     4,     1,     2,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     3,     3,     3,     3,     2,     2,     3,     4,     4,
       4,     6,     4,     6,     4,     6,     4,     6,     2,     2,
       1,     1,     2,     1,     2,     2,     2,     4,     2,     4,
       2,     1,     3,     1,     3,     1,     3,     2,     2,     2,
       2,     3,     2,     3,     2,     2,     2,     3,     2,     0,
       3,     0,     0,     9,     2,     0,     7,     8,     6,     0,
       3,     0,     1,     3,     4,     0,     1,     3,     0,     2,
       0,     1,     3,     1,     3,     3,     0,     1,     1,     1,
       1,     3,     1,     3,     3,     3
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
#line 1671 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 19: /* FBLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1677 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 20: /* RBLTIN  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1683 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 21: /* ID  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1689 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 22: /* STRING  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1695 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* HANDLER  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1701 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* SYMBOL  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1707 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* ENDCLAUSE  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1713 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* tPLAYACCEL  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1719 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* tMETHOD  */
#line 130 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1725 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* THEOBJECTFIELD  */
#line 131 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).objectfield).os; }
#line 1731 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 133: /* on  */
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
#line 192 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[-2].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[-2].objectfield).oe);
		delete (yyvsp[-2].objectfield).os;
		(yyval.code) = (yyvsp[0].code); }
#line 2117 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28:
#line 214 "engines/director/lingo/lingo-gr.y"
                                                                        {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[-3].code) - (yyvsp[-5].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 2] = end; }
#line 2128 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 225 "engines/director/lingo/lingo-gr.y"
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
#line 2145 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 242 "engines/director/lingo/lingo-gr.y"
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
#line 2162 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 254 "engines/director/lingo/lingo-gr.y"
                                      {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 259 "engines/director/lingo/lingo-gr.y"
                                                          {
		inst end;
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-3].code));
		(*g_lingo->_currentScript)[(yyvsp[-3].code) + 1] = end; }
#line 2181 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 263 "engines/director/lingo/lingo-gr.y"
                                                    {
		inst end;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 268 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); }
#line 2198 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
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
#line 2213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
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
#line 2228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 296 "engines/director/lingo/lingo-gr.y"
                                                        {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-3].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2239 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 303 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-8].code) - (yyvsp[-10].code));
		WRITE_UINT32(&else1, (yyvsp[-5].code) - (yyvsp[-10].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-10].code));
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-10].code), (yyvsp[-1].code) - (yyvsp[-10].code), (yyvsp[-3].code) - (yyvsp[-10].code)); }
#line 2254 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 313 "engines/director/lingo/lingo-gr.y"
                                                                   {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-4].code) - (yyvsp[-6].code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code));
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 2] = else1;/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code), (yyvsp[-1].code) - (yyvsp[-6].code)); }
#line 2269 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 324 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code3(LC::c_repeatwhilecode, STOP, STOP); }
#line 2275 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 326 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code3(LC::c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2285 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 332 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); }
#line 2295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 338 "engines/director/lingo/lingo-gr.y"
                                        {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); }
#line 2306 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2312 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 347 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2318 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 349 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2324 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 353 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 359 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2342 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 362 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2350 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 365 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2359 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 369 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2368 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 373 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2377 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 379 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2383 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 381 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2391 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 384 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2399 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 387 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2407 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 390 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2420 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 398 "engines/director/lingo/lingo-gr.y"
                               {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2431 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 404 "engines/director/lingo/lingo-gr.y"
                         {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[0].objectfield).os->c_str());
		g_lingo->codeInt((yyvsp[0].objectfield).oe);
		delete (yyvsp[0].objectfield).os; }
#line 2441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 410 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2447 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 411 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2453 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 412 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2459 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 413 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2465 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 414 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2471 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 415 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2477 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 416 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2483 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 417 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2489 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 418 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 419 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2501 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 420 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2507 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 421 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2513 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 422 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2519 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 423 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 424 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2531 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 425 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2537 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 426 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2543 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 427 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2549 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 428 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); }
#line 2555 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 429 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2561 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 430 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2567 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 431 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_intersects); }
#line 2573 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 432 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_within); }
#line 2579 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 433 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_charOf); }
#line 2585 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 434 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charToOf); }
#line 2591 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 435 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_itemOf); }
#line 2597 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 436 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemToOf); }
#line 2603 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 437 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_lineOf); }
#line 2609 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 438 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineToOf); }
#line 2615 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 439 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_wordOf); }
#line 2621 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 440 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordToOf); }
#line 2627 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 442 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2635 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 446 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_printtop); }
#line 2641 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 449 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_exitRepeat); }
#line 2647 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 450 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_procret); }
#line 2653 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 454 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2661 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 457 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2669 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 460 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_open); }
#line 2675 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 461 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code2(LC::c_voidpush, LC::c_open); }
#line 2681 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 463 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2690 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 467 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_global);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2699 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 472 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2708 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 476 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_property);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 481 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 485 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_instance);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 497 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 2741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 498 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 2747 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 499 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 2753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 500 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 2762 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 504 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 2771 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 508 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 2780 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 516 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_playdone); }
#line 2786 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 517 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 2795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 521 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 2804 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 525 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 2813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 529 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 2819 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 529 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 2828 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 559 "engines/director/lingo/lingo-gr.y"
             { g_lingo->_indef = kStateInArgs; }
#line 2834 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 559 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->_currentFactory.clear(); }
#line 2840 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 560 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete (yyvsp[-6].s); }
#line 2851 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 566 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 2857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 567 "engines/director/lingo/lingo-gr.y"
                  { g_lingo->_indef = kStateInArgs; }
#line 2863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 568 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		delete (yyvsp[-6].s); }
#line 2874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 574 "engines/director/lingo/lingo-gr.y"
                                                                     {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 2889 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 584 "engines/director/lingo/lingo-gr.y"
                                                 {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->_indef = kStateNone;
		g_lingo->clearArgStack();
		g_lingo->_ignoreMe = false;
		delete (yyvsp[-5].s); }
#line 2901 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 592 "engines/director/lingo/lingo-gr.y"
         { g_lingo->_indef = kStateInArgs; }
#line 2907 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 592 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.s) = (yyvsp[0].s); g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 2913 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 594 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 2919 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 595 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 2925 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 596 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 2931 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 597 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-3].narg) + 1; delete (yyvsp[0].s); }
#line 2937 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 600 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 2943 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 601 "engines/director/lingo/lingo-gr.y"
                                        { delete (yyvsp[0].s); }
#line 2949 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 603 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->codeArgStore(); g_lingo->_indef = kStateInDef; }
#line 2955 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 605 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 2967 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 613 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = 0; }
#line 2973 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 614 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 2979 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 615 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2985 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 617 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 2991 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 618 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2997 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 620 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = (yyvsp[-1].code); }
#line 3003 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 622 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3009 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 623 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3015 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 624 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3021 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 625 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3027 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 627 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3033 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 628 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3039 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 630 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3045 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 631 "engines/director/lingo/lingo-gr.y"
                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3051 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 633 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3060 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 637 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3069 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3073 "engines/director/lingo/lingo-gr.cpp"

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
#line 643 "engines/director/lingo/lingo-gr.y"

