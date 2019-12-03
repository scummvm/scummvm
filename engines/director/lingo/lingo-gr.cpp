/* A Bison parser, made by GNU Bison 3.4.  */

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
#define YYBISON_VERSION "3.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 49 "engines/director/lingo/lingo-gr.y"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/hash-str.h"

#include "director/lingo/lingo.h"
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

	delete token;
}


#line 100 "engines/director/lingo/lingo-gr.cpp"

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
#ifndef YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_HPP_INCLUDED
# define YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_HPP_INCLUDED
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
    TWOWORDBUILTIN = 278,
    FBLTIN = 279,
    FBLTINNOARGS = 280,
    FBLTINONEARG = 281,
    FBLTINARGLIST = 282,
    RBLTIN = 283,
    RBLTINONEARG = 284,
    ID = 285,
    STRING = 286,
    HANDLER = 287,
    SYMBOL = 288,
    ENDCLAUSE = 289,
    tPLAYACCEL = 290,
    tDOWN = 291,
    tELSE = 292,
    tNLELSIF = 293,
    tEXIT = 294,
    tFRAME = 295,
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
    tTHENNL = 310,
    tTO = 311,
    tWHEN = 312,
    tWITH = 313,
    tWHILE = 314,
    tNLELSE = 315,
    tFACTORY = 316,
    tMETHOD = 317,
    tOPEN = 318,
    tPLAY = 319,
    tDONE = 320,
    tINSTANCE = 321,
    tGE = 322,
    tLE = 323,
    tGT = 324,
    tLT = 325,
    tEQ = 326,
    tNEQ = 327,
    tAND = 328,
    tOR = 329,
    tNOT = 330,
    tMOD = 331,
    tAFTER = 332,
    tBEFORE = 333,
    tCONCAT = 334,
    tCONTAINS = 335,
    tSTARTS = 336,
    tCHAR = 337,
    tITEM = 338,
    tLINE = 339,
    tWORD = 340,
    tSPRITE = 341,
    tINTERSECTS = 342,
    tWITHIN = 343,
    tTELL = 344,
    tPROPERTY = 345,
    tON = 346,
    tME = 347
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 79 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Common::Array<double> *arr;

#line 246 "engines/director/lingo/lingo-gr.cpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_HPP_INCLUDED  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
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
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  129
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1844

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  107
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  47
/* YYNRULES -- Number of rules.  */
#define YYNRULES  172
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  369

#define YYUNDEFTOK  2
#define YYMAXUTOK   347

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      99,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    98,    93,     2,
     100,   101,    96,    94,   106,    95,     2,    97,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     103,     2,   102,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   104,     2,   105,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   120,   120,   121,   122,   125,   130,   135,   140,   145,
     146,   147,   150,   156,   159,   160,   161,   167,   174,   180,
     187,   193,   201,   202,   203,   206,   207,   212,   225,   243,
     257,   262,   265,   270,   280,   292,   304,   314,   324,   334,
     346,   347,   350,   351,   354,   355,   358,   366,   367,   373,
     381,   384,   391,   398,   406,   409,   412,   413,   414,   417,
     423,   427,   430,   433,   436,   439,   445,   446,   447,   450,
     453,   454,   455,   458,   466,   472,   473,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   508,
     513,   514,   515,   516,   517,   518,   519,   520,   521,   524,
     527,   530,   534,   535,   536,   537,   538,   539,   540,   543,
     544,   547,   548,   551,   552,   563,   564,   565,   566,   570,
     574,   580,   581,   584,   585,   588,   589,   593,   597,   601,
     601,   631,   631,   636,   637,   637,   642,   649,   655,   657,
     658,   659,   660,   663,   664,   665,   668,   672,   680,   681,
     682,   685,   686
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
  "TWOWORDBUILTIN", "FBLTIN", "FBLTINNOARGS", "FBLTINONEARG",
  "FBLTINARGLIST", "RBLTIN", "RBLTINONEARG", "ID", "STRING", "HANDLER",
  "SYMBOL", "ENDCLAUSE", "tPLAYACCEL", "tDOWN", "tELSE", "tNLELSIF",
  "tEXIT", "tFRAME", "tGLOBAL", "tGO", "tIF", "tINTO", "tLOOP", "tMACRO",
  "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET",
  "tTHEN", "tTHENNL", "tTO", "tWHEN", "tWITH", "tWHILE", "tNLELSE",
  "tFACTORY", "tMETHOD", "tOPEN", "tPLAY", "tDONE", "tINSTANCE", "tGE",
  "tLE", "tGT", "tLT", "tEQ", "tNEQ", "tAND", "tOR", "tNOT", "tMOD",
  "tAFTER", "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR", "tITEM",
  "tLINE", "tWORD", "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL",
  "tPROPERTY", "tON", "tME", "'&'", "'+'", "'-'", "'*'", "'/'", "'%'",
  "'\\n'", "'('", "')'", "'>'", "'<'", "'['", "']'", "','", "$accept",
  "program", "nl", "thennl", "nlelse", "nlelsif", "programline", "asgn",
  "stmtoneliner", "stmt", "ifstmt", "elsestmtoneliner", "elseifstmt",
  "elseifstmtoneliner", "elseifstmtoneliner1", "elseifstmt1",
  "repeatwhile", "repeatwith", "if", "elseif", "begin", "end", "stmtlist",
  "when", "tell", "simpleexpr", "expr", "reference", "proc", "globallist",
  "propertylist", "instancelist", "gotofunc", "gotoframe", "gotomovie",
  "playfunc", "$@1", "defn", "$@2", "$@3", "on", "argdef", "endargdef",
  "argstore", "macro", "arglist", "nonemptyarglist", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
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
     345,   346,   347,    38,    43,    45,    42,    47,    37,    10,
      40,    41,    62,    60,    91,    93,    44
};
# endif

#define YYPACT_NINF -288

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-288)))

#define YYTABLE_NINF -10

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     382,   -77,  -288,  -288,  1014,  -288,  -288,  1014,  1014,  1054,
      26,  -288,  1014,  1135,  1014,  1175,  -288,  -288,  -288,   -26,
      30,   893,  -288,    44,  1014,   -34,    33,    53,    61,    64,
    1014,   933,    81,  1014,  1014,  1014,  1014,  1014,  1014,  -288,
      84,    89,   -22,  1014,  1014,  1014,  1014,     2,  -288,  -288,
    -288,  -288,  -288,  1014,    49,  1014,   812,  1014,  -288,  1741,
    -288,  -288,  -288,  -288,  -288,  -288,  -288,  -288,  -288,    21,
    1014,  -288,  1741,  1741,  1741,  1014,  1741,    16,  1014,  1741,
    1014,    16,  1741,  1014,    16,  1014,  -288,  -288,    17,  1014,
    -288,  1014,  -288,    77,  -288,  1741,    48,  -288,  -288,  1209,
      95,  -288,    -5,  1014,    -1,    72,  -288,  -288,  1638,  -288,
      48,  -288,  -288,    22,   -64,  1242,  1275,  1308,  1341,  1671,
    -288,    25,  -288,    97,   -64,   -64,  1704,  1741,   -37,  -288,
     468,  1741,  1014,  1506,  -288,  1605,  1014,  1014,  1014,  1014,
    1014,  1014,  1014,  1014,  1014,  1014,  1014,  1014,  1014,  1014,
    1014,  1014,  1014,   102,  1014,  1209,  1704,   -25,  1014,    46,
     -16,  1704,    -8,    46,   123,  1741,  1741,  1014,  -288,  -288,
      73,  1014,  1014,  -288,  1014,  1014,    83,   788,  1014,  1014,
    -288,  -288,  1014,  -288,   126,  1014,  1014,  1014,  1014,  1014,
    1014,  1014,  1014,  1014,  1014,   127,    11,  -288,  -288,  1014,
    -288,  -288,   640,  1741,  -288,  -288,  -288,  -288,  1014,  -288,
      13,    13,    13,    13,   283,   283,   -64,  1741,    13,    13,
     118,   130,   130,   -64,   -64,  1741,  1741,  -288,   -35,  -288,
    1741,  -288,  -288,  -288,  1741,   102,  -288,  -288,  1741,  1741,
    1741,    13,  1014,  1014,  1741,    13,   102,  1741,  -288,  1741,
    1374,  1741,  1407,  1741,  1440,  1741,  1473,  1741,  1741,  -288,
    -288,  1014,  1741,  -288,  -288,   124,     1,   812,   640,  1741,
     640,   129,    54,   -35,    13,  1741,   -35,  1014,  1014,  1014,
    1014,    12,  -288,   105,  1014,  -288,   -31,   128,  -288,   133,
    -288,    54,    54,  1741,  1741,  1741,  1741,  -288,  1014,  1741,
     -15,  -288,  -288,  -288,   132,  -288,  -288,   554,  -288,  -288,
    1741,  -288,  -288,  -288,  -288,   132,  -288,  1014,   812,   640,
     132,   132,  -288,  1014,   136,   640,   640,  -288,   640,   812,
    -288,   107,  1572,  -288,   139,  -288,   140,  1539,  -288,    75,
     640,   144,  -288,  -288,  -288,  -288,  -288,  -288,  -288,  -288,
    -288,   162,   161,  -288,  -288,   812,  -288,   726,   726,  -288,
    -288,  -288,  -288,  -288,   640,   640,  -288,  -288,  -288
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    61,    73,     0,    62,   118,   121,     0,     0,
       0,    68,     0,     0,     0,    65,    64,    63,   149,   114,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    60,
       0,     0,   108,     0,     0,     0,   168,     0,     3,    75,
      25,    11,    26,     0,     0,     0,     0,     0,    66,    23,
      67,    24,   111,   112,    10,    54,    22,     5,     4,    65,
       0,   108,    74,   120,   119,     0,   171,   122,   168,    69,
       0,    70,   109,   168,   167,   168,   113,   129,   115,     0,
     135,     0,   136,     0,   137,   142,   138,   140,   151,   110,
       0,    50,     0,     0,     0,     0,   153,   154,   127,   145,
     146,   148,   133,   117,    89,     0,     0,     0,     0,     0,
     131,   116,   158,     0,    94,    95,     0,   169,     0,     1,
       9,    54,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   159,   168,     0,   171,     0,     0,   128,
       0,   169,     0,   150,     0,   141,   144,     0,   139,    54,
       0,     0,     0,    51,     0,     0,    66,     0,     0,     0,
      59,    54,     0,   147,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    96,    97,     0,
       2,    56,    55,    55,    54,     6,    54,    30,     0,    54,
      85,    86,    83,    84,    87,    88,    80,    91,    92,    93,
      90,    76,    77,    78,    79,    81,    82,   160,     0,   123,
     172,    71,    72,   130,   143,   159,    12,    13,    14,    15,
      19,    17,     0,     0,    18,    16,   159,   126,   134,   100,
       0,   102,     0,   104,     0,   106,     0,    98,    99,   132,
     124,   168,   170,    57,    58,     0,     0,     0,    55,    32,
      55,     0,   166,     0,    21,    20,     0,     0,     0,     0,
       0,     0,    27,     0,     0,    55,    54,     0,   161,     0,
      54,   166,   166,   101,   103,   105,   107,   125,     0,    55,
      39,    33,     7,    54,     0,    31,   162,   157,    54,    54,
      55,    54,     8,    54,    53,    55,    45,     0,    56,    55,
      55,    47,    43,     0,   163,   152,   155,    54,    55,     0,
      44,    40,     0,    55,     0,    42,     0,     0,   164,   156,
      55,     0,    55,    54,    55,    54,    36,    34,    35,    54,
      54,     0,     0,    28,    38,     0,    37,     0,    54,    54,
     165,    29,    41,    55,    55,    55,    46,    48,    49
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -288,  -288,    15,  -135,  -287,  -288,    74,  -288,   -49,     0,
    -288,  -288,  -288,   -97,  -256,  -113,  -288,  -288,  -288,  -284,
      -4,    28,  -110,  -288,  -288,   108,    -3,    38,  -288,  -288,
    -288,  -288,  -288,   179,   -23,  -288,  -288,  -288,  -288,  -288,
    -288,  -188,  -288,  -187,  -288,   -32,     4
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    47,   263,   206,   303,   314,    48,    49,    50,   264,
      52,   344,   320,   321,   316,   322,    53,    54,    55,   317,
     201,   207,   202,    56,    57,    58,    59,    60,    61,    88,
     121,   113,    62,    96,    97,    63,    85,    64,   169,   181,
      65,   228,   339,   290,    66,   162,    77
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      51,    72,   129,   301,    73,    74,    76,   134,   111,    79,
      76,    82,    76,   313,   128,   143,    68,    81,    95,    84,
     323,    99,    67,   312,   100,   101,    86,   108,    95,   302,
     114,   115,   116,   117,   118,   119,   323,   283,   151,   152,
     124,   125,   126,   127,   343,   302,   159,   273,   102,   103,
     131,   174,   133,   163,   135,   178,    78,   284,   276,   330,
      87,   153,   130,   104,    67,   330,   175,   155,   198,   199,
     179,   271,   156,   168,    98,   127,   229,   156,   123,   157,
     161,   158,   127,   105,   160,   231,   165,   183,   166,   142,
     158,   106,   143,   232,   107,    91,   268,    93,   199,   270,
     177,    67,    14,   236,   308,   309,   146,   147,   148,   149,
     150,   112,   260,   297,   120,   151,   152,   261,   199,   122,
     132,   154,   158,   164,   167,   173,   180,   196,   184,   203,
      51,   195,   227,   210,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     209,   127,   199,   233,   242,   230,   248,   259,   282,   288,
     289,   298,   305,   306,   234,   235,   338,   302,   238,   239,
     312,   240,   241,   347,   348,   244,   245,   246,   353,   247,
     307,   351,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   360,   319,   142,   361,   262,   143,   325,   326,
     267,   328,   350,   315,   200,   269,   142,   335,   237,   143,
     110,   176,   147,   148,   149,   150,     0,   340,   285,     0,
     151,   152,     0,     0,     0,     0,   149,   150,     0,   281,
     265,   266,   151,   152,     0,     0,     0,     0,     0,   274,
     275,     0,     0,   272,     0,     0,     0,     0,   364,   365,
       0,     0,     0,     0,     0,     0,     0,     0,   127,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   333,
       0,     0,     0,     0,   293,   294,   295,   296,     0,     0,
     342,   299,   304,     0,     0,     0,     0,     0,   291,     0,
       0,   292,     0,     0,     0,   310,   286,     0,   287,   318,
       0,     0,     0,     0,     0,     0,   362,     0,     0,   329,
       0,     0,     0,   300,   332,     0,     0,     0,     0,     0,
     337,     0,     0,     0,     0,     0,     0,   311,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   327,   355,
       0,   357,     0,   331,     0,   358,   359,   334,   336,     0,
     136,   137,     0,     0,   138,   139,   341,   363,   363,   142,
       0,   346,   143,   144,   145,     0,     0,     0,   352,     0,
     354,     0,   356,     0,     0,     0,   146,   147,   148,   149,
     150,     0,    -9,     1,     0,   151,   152,     0,     0,     0,
       0,   366,   367,   368,     2,     0,     0,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,    14,    15,    16,     0,    17,     0,    18,     0,     0,
       0,    19,     0,    20,    21,    22,     0,     0,    23,     0,
       0,     0,     0,    24,    25,    26,     0,     0,     0,    27,
       0,     0,     0,    28,    29,    30,    31,     0,    32,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,     0,     0,    34,    35,    36,    37,    38,     0,
       0,    39,    40,    41,    42,     0,    43,    44,     0,     0,
       2,    -9,    45,     3,     4,     5,    46,     6,     7,     8,
       9,    10,     0,    11,    12,    13,     0,    14,    15,    16,
       0,    17,     0,    18,     0,     0,     0,    19,     0,    20,
      21,    22,     0,     0,    23,     0,     0,     0,     0,    24,
      25,    26,     0,     0,     0,    27,     0,     0,     0,    28,
      29,    30,    31,     0,    32,     0,     0,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,    39,    40,    41,
      42,     0,    43,    44,     0,     0,     2,     0,    45,     3,
       4,     5,    46,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,    14,    15,    16,     0,    17,   324,    18,
       0,     0,     0,    19,     0,    20,    21,    22,     0,     0,
       0,     0,     0,     0,     0,    24,    25,    26,     0,     0,
       0,    27,     0,     0,     0,     0,     0,    30,    31,     0,
      32,     0,     0,     0,     0,     0,     0,     0,     0,    33,
       0,     0,     0,     0,     0,     0,    34,    35,    36,    37,
      38,     0,     0,    39,    40,     0,    42,     0,    43,    44,
       0,     0,     2,    67,    45,     3,     4,     5,    46,     6,
       7,     8,     9,    10,     0,    11,    12,    13,     0,    14,
      15,    16,     0,    17,     0,    18,     0,     0,     0,    19,
       0,    20,    21,    22,     0,     0,     0,     0,     0,     0,
       0,    24,    25,    26,     0,     0,     0,    27,     0,     0,
       0,     0,     0,    30,    31,     0,    32,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,     0,    39,
      40,     0,    42,     0,    43,    44,     0,     0,     2,    67,
      45,     3,     4,     5,    46,     6,     7,     8,     9,    10,
       0,    11,    12,    13,     0,    14,    15,    16,     0,    17,
       0,    18,     0,     0,     0,    19,     0,    20,    21,    22,
       0,     0,     0,     0,     0,     0,     0,    24,    25,    26,
       0,     0,     0,    27,     0,     0,     0,     0,     0,    30,
      31,     0,    32,     0,     0,     0,     0,     0,     0,     0,
       0,    33,     0,     0,     0,     0,     0,     0,    34,    35,
      36,    37,    38,     0,     0,    39,    40,     0,    42,     0,
      43,    44,     0,     0,     2,     0,    45,     3,     4,     5,
      46,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,    14,    15,    16,   243,    17,     0,    18,     0,     0,
       0,    19,     0,    20,    21,   136,   137,     0,     0,   138,
     139,   140,   141,    24,   142,    26,     0,   143,   144,   145,
       0,     0,     0,     0,     0,    30,    31,     0,    32,     0,
       0,   146,   147,   148,   149,   150,     0,    33,     0,     0,
     151,   152,     0,     0,    34,    35,    36,    37,    38,     0,
       0,     0,    40,     0,    42,     2,    43,    44,     3,     4,
       5,     0,    45,     0,     0,     0,    46,     0,    11,    12,
      13,     0,    14,    69,    16,     0,    17,     0,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    90,     0,
      91,    92,    93,    94,    70,     2,    26,     0,     3,     4,
       5,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,    14,    69,    16,     0,    17,     0,    33,     0,
       0,     0,     0,    89,     0,    34,    35,    36,    37,    38,
      91,     0,    93,     0,    70,    71,    26,    43,    44,     0,
       0,     0,     0,    45,     0,     0,     0,    46,   109,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    33,     0,
       0,     0,     0,     0,     0,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    71,     2,    43,    44,     3,
       4,     5,     0,    45,     0,     0,     0,    46,     0,    11,
      12,    13,     0,    14,    69,    16,     0,    17,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    70,     2,    26,     0,     3,
       4,     5,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,    14,    69,    16,     0,    17,     0,    33,
       0,     0,     0,     0,     0,     0,    34,    35,    36,    37,
      38,     0,     0,     0,     0,    70,    71,    26,    43,    44,
       0,     0,     0,     0,    45,     0,     0,     0,    46,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    33,
       0,     0,     0,     0,     0,     0,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,    71,     2,    43,    44,
       3,     4,     5,     0,    75,     0,     0,     0,    46,     0,
      11,    12,    13,     0,    14,    69,    16,     0,    17,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    70,     2,    26,     0,
       3,     4,     5,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,    14,    69,    16,     0,    17,     0,
      33,     0,     0,     0,     0,     0,     0,    34,    35,    36,
      37,    38,     0,     0,     0,     0,    70,    71,    26,    43,
      44,     0,     0,     0,     0,    80,     0,     0,     0,    46,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      33,     0,     0,   170,     0,     0,     0,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    71,     0,    43,
      44,     0,     0,     0,     0,    83,   136,   137,     0,    46,
     138,   139,   140,   141,     0,   142,   171,   172,   143,   144,
     145,   185,     0,     0,     0,     0,     0,     0,   186,     0,
       0,     0,   146,   147,   148,   149,   150,     0,     0,   136,
     137,   151,   152,   138,   139,   140,   141,     0,   142,     0,
       0,   143,   144,   145,   187,     0,     0,     0,     0,     0,
       0,   188,     0,     0,     0,   146,   147,   148,   149,   150,
       0,     0,   136,   137,   151,   152,   138,   139,   140,   141,
       0,   142,     0,     0,   143,   144,   145,   189,     0,     0,
       0,     0,     0,     0,   190,     0,     0,     0,   146,   147,
     148,   149,   150,     0,     0,   136,   137,   151,   152,   138,
     139,   140,   141,     0,   142,     0,     0,   143,   144,   145,
     191,     0,     0,     0,     0,     0,     0,   192,     0,     0,
       0,   146,   147,   148,   149,   150,     0,     0,   136,   137,
     151,   152,   138,   139,   140,   141,     0,   142,     0,     0,
     143,   144,   145,   277,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   146,   147,   148,   149,   150,     0,
       0,   136,   137,   151,   152,   138,   139,   140,   141,     0,
     142,     0,     0,   143,   144,   145,   278,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   146,   147,   148,
     149,   150,     0,     0,   136,   137,   151,   152,   138,   139,
     140,   141,     0,   142,     0,     0,   143,   144,   145,   279,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     146,   147,   148,   149,   150,     0,     0,   136,   137,   151,
     152,   138,   139,   140,   141,     0,   142,     0,     0,   143,
     144,   145,   280,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   146,   147,   148,   149,   150,     0,     0,
     136,   137,   151,   152,   138,   139,   140,   141,     0,   142,
       0,     0,   143,   144,   145,     0,     0,     0,     0,     0,
     204,   205,     0,     0,     0,     0,   146,   147,   148,   149,
     150,     0,     0,   136,   137,   151,   152,   138,   139,   140,
     141,     0,   142,     0,     0,   143,   144,   145,     0,     0,
       0,     0,     0,   349,   205,     0,     0,     0,     0,   146,
     147,   148,   149,   150,     0,     0,   136,   137,   151,   152,
     138,   139,   140,   141,     0,   142,     0,     0,   143,   144,
     145,     0,     0,     0,     0,     0,   345,     0,     0,     0,
       0,     0,   146,   147,   148,   149,   150,     0,     0,   136,
     137,   151,   152,   138,   139,   140,   141,     0,   142,     0,
       0,   143,   144,   145,     0,     0,     0,     0,     0,     0,
       0,   208,     0,     0,     0,   146,   147,   148,   149,   150,
       0,     0,   136,   137,   151,   152,   138,   139,   140,   141,
       0,   142,     0,     0,   143,   144,   145,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   182,     0,   146,   147,
     148,   149,   150,     0,    67,   136,   137,   151,   152,   138,
     139,   140,   141,     0,   142,     0,     0,   143,   144,   145,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   146,   147,   148,   149,   150,     0,     0,   136,   137,
     151,   152,   138,   139,   140,   141,     0,   142,     0,     0,
     143,   144,   145,     0,     0,     0,     0,     0,   193,   194,
       0,     0,     0,     0,   146,   147,   148,   149,   150,     0,
       0,   136,   137,   151,   152,   138,   139,   140,   141,     0,
     142,     0,     0,   143,   144,   145,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   146,   147,   148,
     149,   150,     0,     0,     0,   197,   151,   152,   136,   137,
       0,     0,   138,   139,   140,   141,     0,   142,     0,     0,
     143,   144,   145,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   146,   147,   148,   149,   150,     0,
       0,     0,     0,   151,   152
};

static const yytype_int16 yycheck[] =
{
       0,     4,     0,    34,     7,     8,     9,    56,    31,    12,
      13,    14,    15,   300,    46,    79,     1,    13,    21,    15,
     304,    24,    99,    38,    58,    59,    52,    30,    31,    60,
      33,    34,    35,    36,    37,    38,   320,    36,   102,   103,
      43,    44,    45,    46,   331,    60,    78,   235,    15,    16,
      53,    56,    55,    85,    57,    56,    30,    56,   246,   315,
      30,    65,    47,    30,    99,   321,    71,    70,   105,   106,
      71,   106,    75,    96,    30,    78,   101,    80,   100,    75,
      83,   106,    85,    30,    80,   101,    89,   110,    91,    76,
     106,    30,    79,   101,    30,    47,   206,    49,   106,   209,
     103,    99,    29,    30,   291,   292,    93,    94,    95,    96,
      97,    30,   101,   101,    30,   102,   103,   106,   106,    30,
      71,   100,   106,   106,    47,    30,    54,    30,   106,   132,
     130,   106,    30,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     135,   154,   106,    30,    71,   158,    30,    30,    34,    30,
     106,    56,    34,    30,   167,   169,    30,    60,   171,   172,
      38,   174,   175,    34,    34,   178,   179,   181,    34,   182,
     290,   106,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    30,   303,    76,    34,   199,    79,   308,   309,
     204,   311,   337,   300,   130,   208,    76,   320,   170,    79,
      31,   103,    94,    95,    96,    97,    -1,   327,   267,    -1,
     102,   103,    -1,    -1,    -1,    -1,    96,    97,    -1,   261,
     202,   203,   102,   103,    -1,    -1,    -1,    -1,    -1,   242,
     243,    -1,    -1,   228,    -1,    -1,    -1,    -1,   358,   359,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   261,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   318,
      -1,    -1,    -1,    -1,   277,   278,   279,   280,    -1,    -1,
     329,   284,   286,    -1,    -1,    -1,    -1,    -1,   273,    -1,
      -1,   276,    -1,    -1,    -1,   298,   268,    -1,   270,   303,
      -1,    -1,    -1,    -1,    -1,    -1,   355,    -1,    -1,   313,
      -1,    -1,    -1,   285,   317,    -1,    -1,    -1,    -1,    -1,
     323,    -1,    -1,    -1,    -1,    -1,    -1,   299,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   310,   343,
      -1,   345,    -1,   315,    -1,   349,   350,   319,   320,    -1,
      67,    68,    -1,    -1,    71,    72,   328,   357,   358,    76,
      -1,   333,    79,    80,    81,    -1,    -1,    -1,   340,    -1,
     342,    -1,   344,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    -1,     0,     1,    -1,   102,   103,    -1,    -1,    -1,
      -1,   363,   364,   365,    12,    -1,    -1,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    -1,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    -1,    35,    -1,    -1,
      -1,    39,    -1,    41,    42,    43,    -1,    -1,    46,    -1,
      -1,    -1,    -1,    51,    52,    53,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    61,    62,    63,    64,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    -1,
      -1,    89,    90,    91,    92,    -1,    94,    95,    -1,    -1,
      12,    99,   100,    15,    16,    17,   104,    19,    20,    21,
      22,    23,    -1,    25,    26,    27,    -1,    29,    30,    31,
      -1,    33,    -1,    35,    -1,    -1,    -1,    39,    -1,    41,
      42,    43,    -1,    -1,    46,    -1,    -1,    -1,    -1,    51,
      52,    53,    -1,    -1,    -1,    57,    -1,    -1,    -1,    61,
      62,    63,    64,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,
      82,    83,    84,    85,    86,    -1,    -1,    89,    90,    91,
      92,    -1,    94,    95,    -1,    -1,    12,    -1,   100,    15,
      16,    17,   104,    19,    20,    21,    22,    23,    -1,    25,
      26,    27,    -1,    29,    30,    31,    -1,    33,    34,    35,
      -1,    -1,    -1,    39,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    52,    53,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    63,    64,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    -1,    -1,    89,    90,    -1,    92,    -1,    94,    95,
      -1,    -1,    12,    99,   100,    15,    16,    17,   104,    19,
      20,    21,    22,    23,    -1,    25,    26,    27,    -1,    29,
      30,    31,    -1,    33,    -1,    35,    -1,    -1,    -1,    39,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    52,    53,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    63,    64,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    -1,    -1,    89,
      90,    -1,    92,    -1,    94,    95,    -1,    -1,    12,    99,
     100,    15,    16,    17,   104,    19,    20,    21,    22,    23,
      -1,    25,    26,    27,    -1,    29,    30,    31,    -1,    33,
      -1,    35,    -1,    -1,    -1,    39,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    63,
      64,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    -1,    -1,    89,    90,    -1,    92,    -1,
      94,    95,    -1,    -1,    12,    -1,   100,    15,    16,    17,
     104,    19,    20,    21,    22,    23,    -1,    25,    26,    27,
      -1,    29,    30,    31,    56,    33,    -1,    35,    -1,    -1,
      -1,    39,    -1,    41,    42,    67,    68,    -1,    -1,    71,
      72,    73,    74,    51,    76,    53,    -1,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    63,    64,    -1,    66,    -1,
      -1,    93,    94,    95,    96,    97,    -1,    75,    -1,    -1,
     102,   103,    -1,    -1,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    90,    -1,    92,    12,    94,    95,    15,    16,
      17,    -1,   100,    -1,    -1,    -1,   104,    -1,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    45,    -1,
      47,    48,    49,    50,    51,    12,    53,    -1,    15,    16,
      17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    -1,    75,    -1,
      -1,    -1,    -1,    40,    -1,    82,    83,    84,    85,    86,
      47,    -1,    49,    -1,    51,    92,    53,    94,    95,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,   104,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    -1,    -1,    92,    12,    94,    95,    15,
      16,    17,    -1,   100,    -1,    -1,    -1,   104,    -1,    25,
      26,    27,    -1,    29,    30,    31,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    12,    53,    -1,    15,
      16,    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      26,    27,    -1,    29,    30,    31,    -1,    33,    -1,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    -1,    51,    92,    53,    94,    95,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,    85,
      86,    -1,    -1,    -1,    -1,    -1,    92,    12,    94,    95,
      15,    16,    17,    -1,   100,    -1,    -1,    -1,   104,    -1,
      25,    26,    27,    -1,    29,    30,    31,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    12,    53,    -1,
      15,    16,    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    26,    27,    -1,    29,    30,    31,    -1,    33,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    -1,    51,    92,    53,    94,
      95,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    -1,    44,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    -1,    -1,    92,    -1,    94,
      95,    -1,    -1,    -1,    -1,   100,    67,    68,    -1,   104,
      71,    72,    73,    74,    -1,    76,    77,    78,    79,    80,
      81,    49,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,    67,
      68,   102,   103,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    79,    80,    81,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      -1,    -1,    67,    68,   102,   103,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    79,    80,    81,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    -1,    -1,    67,    68,   102,   103,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    79,    80,    81,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    -1,    -1,    67,    68,
     102,   103,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      79,    80,    81,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,
      -1,    67,    68,   102,   103,    71,    72,    73,    74,    -1,
      76,    -1,    -1,    79,    80,    81,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    -1,    -1,    67,    68,   102,   103,    71,    72,
      73,    74,    -1,    76,    -1,    -1,    79,    80,    81,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    -1,    -1,    67,    68,   102,
     103,    71,    72,    73,    74,    -1,    76,    -1,    -1,    79,
      80,    81,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,
      67,    68,   102,   103,    71,    72,    73,    74,    -1,    76,
      -1,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      54,    55,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    -1,    -1,    67,    68,   102,   103,    71,    72,    73,
      74,    -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    54,    55,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    -1,    -1,    67,    68,   102,   103,
      71,    72,    73,    74,    -1,    76,    -1,    -1,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,    67,
      68,   102,   103,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      -1,    -1,    67,    68,   102,   103,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    -1,    93,    94,
      95,    96,    97,    -1,    99,    67,    68,   102,   103,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    -1,    -1,    67,    68,
     102,   103,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    87,    88,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,
      -1,    67,    68,   102,   103,    71,    72,    73,    74,    -1,
      76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    -1,    -1,    -1,   101,   102,   103,    67,    68,
      -1,    -1,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,
      -1,    -1,    -1,   102,   103
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    15,    16,    17,    19,    20,    21,    22,
      23,    25,    26,    27,    29,    30,    31,    33,    35,    39,
      41,    42,    43,    46,    51,    52,    53,    57,    61,    62,
      63,    64,    66,    75,    82,    83,    84,    85,    86,    89,
      90,    91,    92,    94,    95,   100,   104,   108,   113,   114,
     115,   116,   117,   123,   124,   125,   130,   131,   132,   133,
     134,   135,   139,   142,   144,   147,   151,    99,   109,    30,
      51,    92,   133,   133,   133,   100,   133,   153,    30,   133,
     100,   153,   133,   100,   153,   143,    52,    30,   136,    40,
      45,    47,    48,    49,    50,   133,   140,   141,    30,   133,
      58,    59,    15,    16,    30,    30,    30,    30,   133,    65,
     140,   141,    30,   138,   133,   133,   133,   133,   133,   133,
      30,   137,    30,   100,   133,   133,   133,   133,   152,     0,
     109,   133,    71,   133,   115,   133,    67,    68,    71,    72,
      73,    74,    76,    79,    80,    81,    93,    94,    95,    96,
      97,   102,   103,   127,   100,   133,   133,   153,   106,   152,
     153,   133,   152,   152,   106,   133,   133,    47,   141,   145,
      44,    77,    78,    30,    56,    71,   132,   133,    56,    71,
      54,   146,    58,   141,   106,    49,    56,    49,    56,    49,
      56,    49,    56,    87,    88,   106,    30,   101,   105,   106,
     113,   127,   129,   133,    54,    55,   110,   128,    56,   109,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,    30,   148,   101,
     133,   101,   101,    30,   133,   127,    30,   134,   133,   133,
     133,   133,    71,    56,   133,   133,   127,   133,    30,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,    30,
     101,   106,   133,   109,   116,   128,   128,   127,   129,   133,
     129,   106,   109,   148,   133,   133,   148,    49,    49,    49,
      49,   152,    34,    36,    56,   115,   128,   128,    30,   106,
     150,   109,   109,   133,   133,   133,   133,   101,    56,   133,
     128,    34,    60,   111,   127,    34,    30,   129,   150,   150,
     133,   128,    38,   111,   112,   120,   121,   126,   127,   129,
     119,   120,   122,   126,    34,   129,   129,   128,   129,   127,
     121,   128,   133,   115,   128,   122,   128,   133,    30,   149,
     129,   128,   115,   111,   118,    54,   128,    34,    34,    54,
     110,   106,   128,    34,   128,   127,   128,   127,   127,   127,
      30,    34,   115,   116,   129,   129,   128,   128,   128
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   107,   108,   108,   108,   109,   110,   111,   112,   113,
     113,   113,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   115,   115,   115,   116,   116,   116,   116,   116,
     116,   116,   116,   117,   117,   117,   117,   117,   117,   117,
     118,   118,   119,   119,   120,   120,   121,   122,   122,   122,
     123,   124,   125,   126,   127,   128,   129,   129,   129,   130,
     131,   132,   132,   132,   132,   132,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   134,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   136,
     136,   137,   137,   138,   138,   139,   139,   139,   139,   139,
     139,   140,   140,   141,   141,   142,   142,   142,   142,   143,
     142,   145,   144,   144,   146,   144,   144,   144,   147,   148,
     148,   148,   148,   149,   149,   149,   150,   151,   152,   152,
     152,   153,   153
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     1,     1,     1,     0,
       1,     1,     4,     4,     4,     4,     4,     4,     4,     4,
       5,     5,     1,     1,     1,     1,     1,     5,    10,    11,
       3,     6,     4,     6,     9,     9,     9,    10,    10,     6,
       0,     3,     2,     1,     2,     1,     6,     1,     6,     6,
       2,     3,     1,     1,     0,     0,     1,     2,     2,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     4,     4,     1,     2,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     2,     2,     3,     3,     4,     4,
       4,     6,     4,     6,     4,     6,     4,     6,     1,     2,
       2,     1,     1,     2,     1,     2,     2,     2,     1,     2,
       2,     1,     2,     4,     4,     6,     4,     2,     3,     1,
       3,     1,     3,     1,     3,     2,     2,     2,     2,     3,
       2,     2,     1,     3,     2,     2,     2,     3,     2,     0,
       3,     0,     8,     2,     0,     8,     8,     6,     2,     0,
       1,     3,     4,     0,     1,     3,     0,     2,     0,     1,
       3,     1,     3
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
  YYUSE (yytype);
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
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
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
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
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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

  if (! yyres)
    return yystrlen (yystr);

  return (YYSIZE_T) (yystpcpy (yyres, yystr) - yyres);
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
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
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
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
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
          yyp++;
          yyformat++;
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
  YYUSE (yytype);
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
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

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
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
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
| yynewstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  *yyssp = (yytype_int16) yystate;

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = (YYSIZE_T) (yyssp - yyss + 1);

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
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
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
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

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
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
#line 122 "engines/director/lingo/lingo-gr.y"
    { yyerrok; }
#line 1903 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5:
#line 125 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1; }
#line 1911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 6:
#line 130 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1; }
#line 1919 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 7:
#line 135 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 5; }
#line 1927 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 140 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 8; }
#line 1935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 150 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 1946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 156 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_assign);
			(yyval.code) = (yyvsp[-2].code); }
#line 1954 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 159 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_after); }
#line 1960 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 160 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_before); }
#line 1966 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 161 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 1977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 167 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 1989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 174 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2000 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 180 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2012 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20:
#line 187 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2023 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21:
#line 193 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2034 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27:
#line 212 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[-2].code) - (yyvsp[-4].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-4].code));
		(*g_lingo->_currentScript)[(yyvsp[-4].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-4].code) + 2] = end;	/* end, if cond fails */

		checkEnd((yyvsp[0].s), "repeat", true); }
#line 2047 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28:
#line 225 "engines/director/lingo/lingo-gr.y"
    {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[-7].code) - (yyvsp[-9].code));
		WRITE_UINT32(&finish, (yyvsp[-4].code) - (yyvsp[-9].code));
		WRITE_UINT32(&body, (yyvsp[-2].code) - (yyvsp[-9].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-9].code));
		WRITE_UINT32(&inc, 1);
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 1] = init;	/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 2] = finish;/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 3] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 4] = inc;	/* increment */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 5] = end;	/* end, if cond fails */

		checkEnd((yyvsp[0].s), "repeat", true); }
#line 2066 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 243 "engines/director/lingo/lingo-gr.y"
    {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[-8].code) - (yyvsp[-10].code));
		WRITE_UINT32(&finish, (yyvsp[-4].code) - (yyvsp[-10].code));
		WRITE_UINT32(&body, (yyvsp[-2].code) - (yyvsp[-10].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-10].code));
		WRITE_UINT32(&inc, (uint32)-1);
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 1] = init;	/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 2] = finish;/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 3] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 4] = inc;	/* increment */
		(*g_lingo->_currentScript)[(yyvsp[-10].code) + 5] = end;	/* end, if cond fails */

		checkEnd((yyvsp[0].s), "repeat", true); }
#line 2085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 257 "engines/director/lingo/lingo-gr.y"
    {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end; }
#line 2095 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 262 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
			checkEnd((yyvsp[0].s), "tell", true); }
#line 2103 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 265 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		}
#line 2111 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 270 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-2].code) - (yyvsp[-5].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[0].s), "if", true);

		g_lingo->processIf(0, 0); }
#line 2126 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 280 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-5].code) - (yyvsp[-8].code));
		WRITE_UINT32(&else1, (yyvsp[-2].code) - (yyvsp[-8].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-8].code));
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[0].s), "if", true);

		g_lingo->processIf(0, 0); }
#line 2143 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 292 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-5].code) - (yyvsp[-8].code));
		WRITE_UINT32(&else1, (yyvsp[-3].code) - (yyvsp[-8].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-8].code));
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[0].s), "if", true);

		g_lingo->processIf(0, (yyvsp[-1].code) - (yyvsp[-8].code)); }
#line 2160 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 304 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-5].code) - (yyvsp[-8].code));
		WRITE_UINT32(&else1, (yyvsp[-2].code) - (yyvsp[-8].code));
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-8].code));
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf(0, (yyvsp[0].code) - (yyvsp[-8].code)); }
#line 2175 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 314 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-6].code) - (yyvsp[-9].code));
		WRITE_UINT32(&else1, (yyvsp[-4].code) - (yyvsp[-9].code));
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-9].code));
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, (yyvsp[0].code) - (yyvsp[-9].code)); }
#line 2190 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 324 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-6].code) - (yyvsp[-9].code));
		WRITE_UINT32(&else1, (yyvsp[-2].code) - (yyvsp[-9].code));
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-9].code));
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, 0); }
#line 2205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 334 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-2].code) - (yyvsp[-5].code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, 0); }
#line 2220 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 346 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; }
#line 2226 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 347 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[-1].code); }
#line 2232 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 358 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-2].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2243 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 367 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2254 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 373 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 381 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); }
#line 2271 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 384 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2281 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 391 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); }
#line 2291 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 398 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); }
#line 2302 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 406 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2308 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 409 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2314 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 412 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2320 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 417 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2330 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 423 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_tellcode);
		g_lingo->code1(STOP); }
#line 2338 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 427 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2346 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 430 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2354 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 433 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str()); }
#line 2362 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 436 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str()); }
#line 2370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 439 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 445 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[0].code); }
#line 2385 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 447 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[0].s), 0);
		delete (yyvsp[0].s); }
#line 2393 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 450 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2401 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 453 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg)); }
#line 2407 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 454 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg)); }
#line 2413 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 455 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2421 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 458 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2434 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 466 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 473 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); }
#line 2451 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 474 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); }
#line 2457 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 475 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); }
#line 2463 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 476 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); }
#line 2469 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 477 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); }
#line 2475 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 478 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); }
#line 2481 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 479 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); }
#line 2487 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 480 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_eq); }
#line 2493 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 481 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); }
#line 2499 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 482 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); }
#line 2505 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 483 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); }
#line 2511 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 484 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); }
#line 2517 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 485 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); }
#line 2523 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 486 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); }
#line 2529 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 487 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); }
#line 2535 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 488 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); }
#line 2541 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 489 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); }
#line 2547 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 490 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); }
#line 2553 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 491 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[0].code); }
#line 2559 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 492 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[0].code); g_lingo->code1(g_lingo->c_negate); }
#line 2565 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 493 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[-1].code); }
#line 2571 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 494 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_arraypush); g_lingo->codeArray((yyvsp[-1].narg)); }
#line 2577 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 495 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); }
#line 2583 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 496 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); }
#line 2589 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 497 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); }
#line 2595 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 498 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); }
#line 2601 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 499 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); }
#line 2607 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 500 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); }
#line 2613 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 501 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); }
#line 2619 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 502 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); }
#line 2625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 503 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); }
#line 2631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 504 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); }
#line 2637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 505 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe(nullptr, 0); }
#line 2643 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 508 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2651 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 513 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); }
#line 2657 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 516 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); }
#line 2663 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 517 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_procret); }
#line 2669 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 521 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[0].s), 0);
		delete (yyvsp[0].s); }
#line 2677 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 524 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2685 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 527 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2693 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 530 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_voidpush);
		g_lingo->codeFunc((yyvsp[0].s), 1);
		delete (yyvsp[0].s); }
#line 2702 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 534 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg)); }
#line 2708 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 535 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg)); }
#line 2714 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 536 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[-1].s), 0); }
#line 2720 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 537 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[-3].s), (yyvsp[-1].narg)); }
#line 2726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 538 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); }
#line 2732 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 539 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); }
#line 2738 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 540 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[-2].s)); s += '-'; s += *(yyvsp[-1].s); g_lingo->codeFunc(&s, (yyvsp[0].narg)); }
#line 2744 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 543 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2750 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 544 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2756 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 547 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2762 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 548 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2768 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 551 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2774 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 552 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2780 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 563 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); }
#line 2786 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 564 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); }
#line 2792 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 565 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); }
#line 2798 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 566 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_goto); }
#line 2807 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 570 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_goto); }
#line 2816 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 574 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_goto); }
#line 2825 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 588 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); }
#line 2831 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 589 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_play); }
#line 2840 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 593 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_play); }
#line 2849 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 597 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_play); }
#line 2858 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 601 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); }
#line 2864 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 601 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg)); }
#line 2872 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 631 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); }
#line 2878 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 632 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
			g_lingo->_indef = false; }
#line 2887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 636 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFactory(*(yyvsp[0].s)); }
#line 2893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 637 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; }
#line 2899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 638 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; }
#line 2908 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 642 "engines/director/lingo/lingo-gr.y"
    {	// D3
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		g_lingo->_indef = false;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false); }
#line 2920 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 649 "engines/director/lingo/lingo-gr.y"
    {	// D4. No 'end' clause
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		g_lingo->_indef = false;
		g_lingo->_ignoreMe = false; }
#line 2930 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 655 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[0].s); g_lingo->_indef = true; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 2936 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 657 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; }
#line 2942 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 658 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = 1; }
#line 2948 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 659 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2954 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 660 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-3].narg) + 1; }
#line 2960 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 668 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); }
#line 2966 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 672 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar); }
#line 2977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 680 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; }
#line 2983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 681 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; }
#line 2989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 682 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 685 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; }
#line 3001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172:
#line 686 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3007 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3011 "engines/director/lingo/lingo-gr.cpp"

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
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
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
#line 689 "engines/director/lingo/lingo-gr.y"

