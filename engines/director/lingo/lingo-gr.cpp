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
	warning("%s at line %d col %d", s, g_lingo->_linenumber, g_lingo->_colnumber);
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
    THEENTITY = 268,
    THEENTITYWITHID = 269,
    FLOAT = 270,
    BLTIN = 271,
    BLTINNOARGS = 272,
    BLTINNOARGSORONE = 273,
    BLTINONEARG = 274,
    BLTINARGLIST = 275,
    TWOWORDBUILTIN = 276,
    FBLTIN = 277,
    FBLTINNOARGS = 278,
    FBLTINONEARG = 279,
    FBLTINARGLIST = 280,
    RBLTIN = 281,
    RBLTINONEARG = 282,
    ID = 283,
    STRING = 284,
    HANDLER = 285,
    SYMBOL = 286,
    ENDCLAUSE = 287,
    tPLAYACCEL = 288,
    tDOWN = 289,
    tELSE = 290,
    tNLELSIF = 291,
    tEXIT = 292,
    tFRAME = 293,
    tGLOBAL = 294,
    tGO = 295,
    tIF = 296,
    tINTO = 297,
    tLOOP = 298,
    tMACRO = 299,
    tMOVIE = 300,
    tNEXT = 301,
    tOF = 302,
    tPREVIOUS = 303,
    tPUT = 304,
    tREPEAT = 305,
    tSET = 306,
    tTHEN = 307,
    tTHENNL = 308,
    tTO = 309,
    tWHEN = 310,
    tWITH = 311,
    tWHILE = 312,
    tNLELSE = 313,
    tFACTORY = 314,
    tMETHOD = 315,
    tOPEN = 316,
    tPLAY = 317,
    tDONE = 318,
    tINSTANCE = 319,
    tGE = 320,
    tLE = 321,
    tGT = 322,
    tLT = 323,
    tEQ = 324,
    tNEQ = 325,
    tAND = 326,
    tOR = 327,
    tNOT = 328,
    tMOD = 329,
    tAFTER = 330,
    tBEFORE = 331,
    tCONCAT = 332,
    tCONTAINS = 333,
    tSTARTS = 334,
    tCHAR = 335,
    tITEM = 336,
    tLINE = 337,
    tWORD = 338,
    tSPRITE = 339,
    tINTERSECTS = 340,
    tWITHIN = 341,
    tTELL = 342,
    tPROPERTY = 343,
    tON = 344,
    tME = 345
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

#line 244 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  124
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1792

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  43
/* YYNRULES -- Number of rules.  */
#define YYNRULES  164
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  360

#define YYUNDEFTOK  2
#define YYMAXUTOK   345

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      98,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    97,    92,     2,
      99,   100,    95,    93,   105,    94,     2,    96,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     102,    91,   101,     2,     2,     2,     2,     2,     2,     2,
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
static const yytype_uint16 yyrline[] =
{
       0,   119,   119,   120,   121,   124,   129,   130,   131,   132,
     135,   141,   144,   145,   146,   152,   159,   165,   171,   178,
     186,   187,   190,   191,   196,   209,   227,   241,   247,   250,
     255,   265,   277,   289,   299,   309,   319,   331,   332,   335,
     336,   339,   340,   343,   351,   352,   360,   361,   362,   365,
     368,   375,   382,   390,   393,   396,   397,   398,   401,   407,
     411,   414,   417,   420,   423,   426,   429,   430,   431,   434,
     438,   446,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   486,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   502,   505,   508,   512,   513,   514,
     515,   516,   517,   518,   521,   522,   525,   526,   529,   530,
     541,   542,   543,   544,   548,   552,   558,   559,   562,   563,
     566,   567,   571,   575,   579,   579,   609,   609,   614,   617,
     617,   622,   630,   637,   639,   640,   641,   642,   645,   649,
     657,   658,   659,   662,   663
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNARY", "CASTREF", "VOID", "VAR",
  "POINT", "RECT", "ARRAY", "OBJECT", "REFERENCE", "INT", "THEENTITY",
  "THEENTITYWITHID", "FLOAT", "BLTIN", "BLTINNOARGS", "BLTINNOARGSORONE",
  "BLTINONEARG", "BLTINARGLIST", "TWOWORDBUILTIN", "FBLTIN",
  "FBLTINNOARGS", "FBLTINONEARG", "FBLTINARGLIST", "RBLTIN",
  "RBLTINONEARG", "ID", "STRING", "HANDLER", "SYMBOL", "ENDCLAUSE",
  "tPLAYACCEL", "tDOWN", "tELSE", "tNLELSIF", "tEXIT", "tFRAME", "tGLOBAL",
  "tGO", "tIF", "tINTO", "tLOOP", "tMACRO", "tMOVIE", "tNEXT", "tOF",
  "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTHENNL", "tTO",
  "tWHEN", "tWITH", "tWHILE", "tNLELSE", "tFACTORY", "tMETHOD", "tOPEN",
  "tPLAY", "tDONE", "tINSTANCE", "tGE", "tLE", "tGT", "tLT", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tME", "'='",
  "'&'", "'+'", "'-'", "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "'>'",
  "'<'", "'['", "']'", "','", "$accept", "program", "nl", "programline",
  "asgn", "stmtoneliner", "stmt", "ifstmt", "elsestmtoneliner",
  "elseifstmt", "elseifstmtoneliner", "elseifstmtoneliner1", "elseifstmt1",
  "cond", "repeatwhile", "repeatwith", "if", "elseif", "begin", "end",
  "stmtlist", "when", "tell", "expr", "reference", "proc", "globallist",
  "propertylist", "instancelist", "gotofunc", "gotoframe", "gotomovie",
  "playfunc", "$@1", "defn", "$@2", "$@3", "on", "argdef", "argstore",
  "macro", "arglist", "nonemptyarglist", YY_NULLPTR
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
     345,    61,    38,    43,    45,    42,    47,    37,    10,    40,
      41,    62,    60,    91,    93,    44
};
# endif

#define YYPACT_NINF -281

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-281)))

#define YYTABLE_NINF -56

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     348,   -80,  -281,  -281,  1005,  -281,  -281,  1005,  1005,  1045,
      22,  -281,  1005,  1118,  1158,  -281,  -281,  -281,   -22,    30,
     891,  -281,    33,  1005,    46,    35,    42,    48,    52,  1005,
     932,    54,  1005,  1005,  1005,  1005,  1005,  1005,  -281,    58,
      62,   -25,  1005,  1005,  1005,  1005,     3,  -281,  -281,  -281,
    -281,  -281,    -5,     9,  1231,   808,  1005,  1690,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,    -3,  1005,  1690,  1690,
    1690,  1005,  1690,     6,  1005,  1690,  1005,     6,  1005,     6,
    1005,  -281,  -281,    11,  1005,  -281,  1005,  -281,    81,  -281,
    1690,    26,  -281,  -281,  1261,   105,  -281,   -38,  1005,   -32,
     101,  -281,  -281,   151,  -281,    26,  -281,  -281,    49,   -50,
    1294,  1327,  1360,  1393,  1558,  -281,    51,  -281,   129,   -50,
     -50,  1624,  1690,    14,  -281,   440,  1231,  1005,  1231,    69,
    1657,  -281,   784,  1005,  1005,  1005,  1005,  1005,  1005,  1005,
    1005,  1005,  1005,  1005,  1005,  1005,  1005,  1005,  1005,   130,
    1005,  1261,  1624,   -16,  1005,    55,    -7,  1624,     4,    55,
     133,  1690,  1690,  1005,  -281,  -281,    96,  1005,  1005,  -281,
    1005,  1005,  1525,  1005,  1005,  -281,  -281,  1005,  -281,   134,
    1005,  1005,  1005,  1005,  1005,  1005,  1005,  1005,  1005,  1005,
     137,     5,  -281,  -281,  1005,  -281,    66,  1690,    67,  1591,
    -281,  -281,  1005,  -281,  1005,  -281,   950,   950,   950,   -50,
     -50,   -50,  1690,   950,   950,   231,  1063,  1063,   -50,   -50,
    1690,  1690,  -281,   -36,  -281,  1690,  -281,  -281,  -281,  1690,
     130,  1005,  -281,  -281,  1690,  1690,  1690,  1690,  1005,  1005,
    1690,  1690,   130,  1690,  -281,  1690,  1200,  1690,  1426,  1690,
    1459,  1690,  1492,  1690,  1690,  -281,  -281,  1005,  1690,  -281,
     -14,  -281,   808,   624,  1690,  1690,   624,   142,    68,   -36,
    1690,  1690,  1690,   -36,  1005,  1005,  1005,  1005,     7,   624,
     123,  1005,  -281,  -281,  -281,   -26,   147,  -281,   152,  -281,
      68,    68,  1690,  1690,  1690,  1690,  -281,   159,  1005,  1690,
     -29,  -281,    65,   158,  -281,  -281,   532,  -281,  -281,  -281,
    1690,  -281,  -281,  -281,   158,  -281,  1231,   808,   624,   158,
     158,  -281,  1231,  -281,   624,   624,  -281,   624,   808,  -281,
     138,   145,  -281,   168,  -281,   169,   150,   624,   174,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,   177,  -281,  -281,
     808,  -281,   716,   716,  -281,  -281,  -281,   624,  -281,  -281
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    60,    70,     0,    61,   113,   116,     0,     0,
       0,    64,     0,     0,    69,    63,    62,   144,   109,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,     0,
       0,     0,     0,     0,     0,   160,     0,     3,    72,    22,
       9,    23,     0,     0,     0,     0,     0,    20,    21,   106,
     107,     7,    53,     8,     5,     4,    69,     0,    71,   115,
     114,     0,   163,   117,   160,    65,     0,    66,   160,   159,
     160,   108,   124,   110,     0,   130,     0,   131,     0,   132,
     137,   133,   135,   146,   105,     0,    49,     0,     0,     0,
       0,   148,   149,   122,   140,   141,   143,   128,   112,    85,
       0,     0,     0,     0,     0,   126,   111,   153,     0,    90,
      91,     0,   161,     0,     1,     6,     0,     0,     0,     0,
      46,    54,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   154,
     160,     0,   163,     0,     0,   123,     0,   161,     0,   145,
       0,   136,   139,     0,   134,    53,     0,     0,     0,    50,
       0,     0,     0,     0,     0,    58,    53,     0,   142,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    92,    93,     0,     2,     0,    54,     0,     0,
      53,    55,     0,    27,     0,    55,    81,    82,    80,    83,
      84,    77,    87,    88,    89,    86,    73,    74,    75,    76,
      78,    79,   155,     0,   118,   164,    67,    68,   125,   138,
     154,     0,    10,    11,    12,    13,    18,    15,     0,     0,
      17,    14,   154,   121,   129,    96,     0,    98,     0,   100,
       0,   102,     0,    94,    95,   127,   119,   160,   162,    55,
       0,    48,     0,    54,    47,    29,    54,     0,   158,     0,
     104,    19,    16,     0,     0,     0,     0,     0,     0,    54,
       0,     0,    54,    56,    57,    53,     0,   156,     0,    55,
     158,   158,    97,    99,   101,   103,   120,     0,     0,    54,
      34,    30,    53,     0,    28,   157,   152,    55,    55,    24,
      54,    55,    52,    53,    54,    42,     0,     0,    54,    54,
      44,    40,     0,   151,   147,   150,    55,    54,     0,    41,
      37,     0,    54,     0,    39,     0,     0,    54,     0,    54,
      53,    54,    53,    33,    31,    32,    53,     0,    25,    35,
       0,    36,     0,    55,    26,    38,    54,    54,    43,    45
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -281,  -281,    18,    85,  -281,   -54,     0,  -281,  -281,  -281,
     -89,  -254,  -107,  -124,  -281,  -281,  -281,  -280,  -109,     2,
    -194,  -281,  -281,     1,  -281,  -281,  -281,  -281,  -281,  -281,
     183,   -13,  -281,  -281,  -281,  -281,  -281,  -281,  -188,  -160,
    -281,   -33,    12
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    46,   283,    47,    48,    49,   284,    51,   341,   319,
     320,   315,   321,   129,    52,    53,    54,   316,   149,   203,
     263,    55,    56,    57,   233,    58,    83,   116,   108,    59,
      91,    92,    60,    80,    61,   165,   176,    62,   223,   289,
      63,   158,    73
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      50,   131,   196,   124,   198,    68,   301,   312,    69,    70,
      72,   266,   123,    75,    72,    72,   170,   106,    64,    65,
     280,    90,   173,   322,    94,    77,    79,   139,    81,   313,
     103,    90,   302,   109,   110,   111,   112,   113,   114,   322,
     281,   155,   269,   119,   120,   121,   122,   159,    97,    98,
      74,   147,   148,   171,   273,   130,   230,   132,    82,   174,
     329,    93,    64,    99,   125,   279,   329,   242,   151,   267,
     100,    86,   152,    88,   118,   122,   101,   152,   164,   157,
     102,   122,   107,   153,   224,   161,   115,   162,   156,   154,
     117,   262,   178,   226,   126,   306,   150,   -55,   154,   172,
     127,    64,    95,    96,   227,   256,   -55,   296,   318,   194,
     257,   154,   194,   324,   325,   -55,   160,   327,   193,   194,
     -55,   200,   201,   231,   232,    50,   163,   130,   197,   199,
     307,   308,   337,   169,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     205,   122,   -55,   175,   179,   225,   190,   191,   222,   357,
     194,   228,   244,   -55,   229,   255,   259,   261,   234,   235,
     287,   236,   237,   288,   240,   241,   303,   298,   243,   304,
     305,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   309,   331,   317,   312,   258,   340,   342,   336,   260,
     344,   345,   346,   264,   328,   265,   348,   177,   282,   354,
     195,   314,   334,   105,     0,     0,   133,   134,     0,     0,
       0,   135,   136,   137,   278,   138,     0,     0,   139,   140,
     141,   350,   270,   352,     0,     0,     0,   353,     0,   271,
     272,   268,     0,   142,   143,   144,   145,   146,     0,     0,
       0,     0,   147,   148,     0,     0,     0,     0,   122,     0,
       0,     0,     0,   332,     0,   285,     0,     0,   286,     0,
       0,     0,     0,     0,   339,   292,   293,   294,   295,     0,
       0,   297,   299,     0,   300,     0,     0,   290,     0,     0,
       0,   291,     0,     0,     0,     0,   355,     0,     0,   310,
       0,   311,   136,   137,     0,   138,     0,     0,   139,     0,
       0,     0,   326,     0,     0,     0,   330,   130,     0,     0,
     333,   335,     0,   130,   143,   144,   145,   146,     0,   338,
       0,     0,   147,   148,   343,     0,     0,     0,     0,   347,
       0,   349,     0,   351,     0,     0,     0,     0,    -6,     1,
       0,     0,   356,   356,     0,     0,     0,     0,   358,   359,
       2,     3,     4,     5,     0,     6,     7,     8,     9,    10,
       0,    11,    12,    13,     0,     0,    14,    15,     0,    16,
       0,    17,     0,     0,     0,    18,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,     0,    23,    24,    25,
       0,     0,     0,    26,     0,     0,     0,    27,    28,    29,
      30,     0,    31,     0,     0,     0,     0,     0,     0,     0,
       0,    32,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     0,     0,    38,    39,    40,    41,     0,
       0,    42,    43,     0,     0,     0,    -6,    44,     0,     0,
       0,    45,     2,     3,     4,     5,     0,     6,     7,     8,
       9,    10,     0,    11,    12,    13,     0,     0,    14,    15,
       0,    16,     0,    17,     0,     0,     0,    18,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,     0,    23,
      24,    25,     0,     0,     0,    26,     0,     0,     0,    27,
      28,    29,    30,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,    32,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    36,    37,     0,     0,    38,    39,    40,
      41,     0,     0,    42,    43,     0,     0,     0,     0,    44,
       0,     0,     0,    45,     2,     3,     4,     5,     0,     6,
       7,     8,     9,    10,     0,    11,    12,    13,     0,     0,
      66,    15,     0,    16,   323,    17,     0,     0,     0,    18,
       0,    19,    20,    21,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,     0,     0,     0,    26,     0,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,    38,
      39,     0,    41,     0,     0,    42,    43,     0,     0,     0,
      64,    44,     0,     0,     0,    45,     2,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,     0,    66,    15,     0,    16,     0,    17,     0,     0,
       0,    18,     0,    19,    20,    21,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,     0,     0,     0,    26,
       0,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,     0,     0,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,    38,    39,     0,    41,     0,     0,    42,    43,     0,
       0,     0,    64,    44,     0,     0,     0,    45,     2,     3,
       4,     5,     0,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,    66,    15,     0,    16,     0,    17,
       0,     0,     0,    18,     0,    19,    20,    21,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,     0,     0,
       0,    26,     0,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,     0,     0,     0,     0,     0,    32,
       0,     0,     0,     0,     0,     0,    33,    34,    35,    36,
      37,     0,     0,    38,    39,     0,    41,     0,     0,    42,
      43,     0,     0,     0,     0,    44,     0,     0,     0,    45,
       2,     3,     4,     5,     0,     6,     7,     8,     9,    10,
       0,    11,    12,    13,     0,     0,    66,    15,   204,    16,
       0,    17,     0,     0,     0,    18,     0,    19,    20,   133,
     134,     0,     0,     0,   135,   136,   137,    23,   138,    25,
       0,   139,   140,   141,     0,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,   142,   143,   144,   145,
     146,    32,    64,     0,     0,   147,   148,     0,    33,    34,
      35,    36,    37,     0,     0,     0,    39,     0,    41,     0,
       0,    42,    43,     2,     3,     4,     5,    44,     0,     0,
       0,    45,     0,     0,    11,    12,    13,     0,     0,    66,
      15,     0,    16,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,    85,     0,    86,    87,    88,    89,
      67,     0,    25,     0,     2,     3,     4,     5,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
      66,    15,     0,    16,    32,     0,     0,     0,     0,     0,
      84,    33,    34,    35,    36,    37,     0,    86,     0,    88,
       0,    67,     0,    25,    42,    43,     0,     0,     0,     0,
      44,     0,     0,     0,    45,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     2,     3,     4,
       5,   136,   137,     0,   138,    42,    43,   139,    11,    12,
      13,    44,     0,    66,    15,    45,    16,     0,     0,     0,
       0,     0,   142,   143,   144,   145,   146,     0,     0,     0,
       0,   147,   148,     0,    67,     0,    25,     2,     3,     4,
       5,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,    66,    15,     0,    16,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,     0,     0,    67,     0,    25,     0,    42,    43,
       0,     0,     0,     0,    44,     0,     0,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    32,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       2,     3,     4,     5,   136,   137,     0,   138,    42,    43,
     139,    11,    12,    13,    71,     0,    66,    15,    45,    16,
       0,     0,     0,     0,     0,     0,     0,     0,   145,   146,
       0,     0,     0,     0,   147,   148,     0,    67,     0,    25,
       2,     3,     4,     5,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,    66,    15,     0,    16,
       0,    32,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     0,     0,     0,     0,    67,     0,    25,
       0,    42,    43,     0,     0,     0,     0,    76,     0,     0,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    32,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,    37,     2,     3,     4,     5,   274,     0,     0,
       0,    42,    43,     0,    11,    12,    13,    78,     0,    66,
      15,    45,    16,     0,     0,   133,   134,     0,     0,     0,
     135,   136,   137,     0,   138,     0,     0,   139,   140,   141,
      67,     0,    25,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   142,   143,   144,   145,   146,     0,     0,     0,
       0,   147,   148,   166,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,     0,     0,
       0,     0,     0,     0,    42,    43,   133,   134,     0,     0,
     128,   135,   136,   137,    45,   138,   167,   168,   139,   140,
     141,   180,     0,     0,     0,     0,     0,     0,   181,     0,
       0,     0,     0,   142,   143,   144,   145,   146,     0,   133,
     134,     0,   147,   148,   135,   136,   137,     0,   138,     0,
       0,   139,   140,   141,   182,     0,     0,     0,     0,     0,
       0,   183,     0,     0,     0,     0,   142,   143,   144,   145,
     146,     0,   133,   134,     0,   147,   148,   135,   136,   137,
       0,   138,     0,     0,   139,   140,   141,   184,     0,     0,
       0,     0,     0,     0,   185,     0,     0,     0,     0,   142,
     143,   144,   145,   146,     0,   133,   134,     0,   147,   148,
     135,   136,   137,     0,   138,     0,     0,   139,   140,   141,
     186,     0,     0,     0,     0,     0,     0,   187,     0,     0,
       0,     0,   142,   143,   144,   145,   146,     0,   133,   134,
       0,   147,   148,   135,   136,   137,     0,   138,     0,     0,
     139,   140,   141,   275,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   142,   143,   144,   145,   146,
       0,   133,   134,     0,   147,   148,   135,   136,   137,     0,
     138,     0,     0,   139,   140,   141,   276,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   142,   143,
     144,   145,   146,     0,   133,   134,     0,   147,   148,   135,
     136,   137,     0,   138,     0,     0,   139,   140,   141,   277,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   142,   143,   144,   145,   146,     0,   133,   134,     0,
     147,   148,   135,   136,   137,     0,   138,     0,     0,   139,
     140,   141,     0,     0,     0,     0,     0,     0,     0,   238,
       0,     0,     0,     0,   142,   143,   144,   145,   146,     0,
     133,   134,     0,   147,   148,   135,   136,   137,     0,   138,
       0,     0,   139,   140,   141,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   239,   142,   143,   144,
     145,   146,     0,   133,   134,     0,   147,   148,   135,   136,
     137,     0,   138,     0,     0,   139,   140,   141,     0,     0,
       0,     0,     0,   188,   189,     0,     0,     0,     0,     0,
     142,   143,   144,   145,   146,     0,   133,   134,     0,   147,
     148,   135,   136,   137,     0,   138,     0,     0,   139,   140,
     141,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   202,   142,   143,   144,   145,   146,     0,   133,
     134,   192,   147,   148,   135,   136,   137,     0,   138,     0,
       0,   139,   140,   141,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   142,   143,   144,   145,
     146,     0,   133,   134,   192,   147,   148,   135,   136,   137,
       0,   138,     0,     0,   139,   140,   141,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   202,   142,
     143,   144,   145,   146,     0,   133,   134,     0,   147,   148,
     135,   136,   137,     0,   138,     0,     0,   139,   140,   141,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   142,   143,   144,   145,   146,     0,     0,     0,
       0,   147,   148
};

static const yytype_int16 yycheck[] =
{
       0,    55,   126,     0,   128,     4,    32,    36,     7,     8,
       9,   205,    45,    12,    13,    14,    54,    30,    98,     1,
      34,    20,    54,   303,    23,    13,    14,    77,    50,    58,
      29,    30,    58,    32,    33,    34,    35,    36,    37,   319,
      54,    74,   230,    42,    43,    44,    45,    80,    13,    14,
      28,   101,   102,    91,   242,    54,   165,    56,    28,    91,
     314,    28,    98,    28,    46,   259,   320,   176,    67,   105,
      28,    45,    71,    47,    99,    74,    28,    76,    91,    78,
      28,    80,    28,    71,   100,    84,    28,    86,    76,   105,
      28,   200,   105,   100,    99,   289,    99,    32,   105,    98,
      91,    98,    56,    57,   100,   100,    41,   100,   302,   105,
     105,   105,   105,   307,   308,    50,   105,   311,   104,   105,
      55,    52,    53,    27,    28,   125,    45,   126,   127,   128,
     290,   291,   326,    28,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     132,   150,    87,    52,   105,   154,   105,    28,    28,   353,
     105,    28,    28,    98,   163,    28,   100,   100,   167,   168,
      28,   170,   171,   105,   173,   174,   285,    54,   177,    32,
      28,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,    32,   316,   302,    36,   194,    58,    52,   322,   197,
      32,    32,    52,   202,   313,   204,    32,    56,   262,    32,
     125,   300,   319,    30,    -1,    -1,    65,    66,    -1,    -1,
      -1,    70,    71,    72,   257,    74,    -1,    -1,    77,    78,
      79,   340,   231,   342,    -1,    -1,    -1,   346,    -1,   238,
     239,   223,    -1,    92,    93,    94,    95,    96,    -1,    -1,
      -1,    -1,   101,   102,    -1,    -1,    -1,    -1,   257,    -1,
      -1,    -1,    -1,   317,    -1,   263,    -1,    -1,   266,    -1,
      -1,    -1,    -1,    -1,   328,   274,   275,   276,   277,    -1,
      -1,   279,   281,    -1,   282,    -1,    -1,   269,    -1,    -1,
      -1,   273,    -1,    -1,    -1,    -1,   350,    -1,    -1,   298,
      -1,   299,    71,    72,    -1,    74,    -1,    -1,    77,    -1,
      -1,    -1,   310,    -1,    -1,    -1,   314,   316,    -1,    -1,
     318,   319,    -1,   322,    93,    94,    95,    96,    -1,   327,
      -1,    -1,   101,   102,   332,    -1,    -1,    -1,    -1,   337,
      -1,   339,    -1,   341,    -1,    -1,    -1,    -1,     0,     1,
      -1,    -1,   352,   353,    -1,    -1,    -1,    -1,   356,   357,
      12,    13,    14,    15,    -1,    17,    18,    19,    20,    21,
      -1,    23,    24,    25,    -1,    -1,    28,    29,    -1,    31,
      -1,    33,    -1,    -1,    -1,    37,    -1,    39,    40,    41,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    49,    50,    51,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    59,    60,    61,
      62,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    -1,    -1,    87,    88,    89,    90,    -1,
      -1,    93,    94,    -1,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,    12,    13,    14,    15,    -1,    17,    18,    19,
      20,    21,    -1,    23,    24,    25,    -1,    -1,    28,    29,
      -1,    31,    -1,    33,    -1,    -1,    -1,    37,    -1,    39,
      40,    41,    -1,    -1,    44,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    55,    -1,    -1,    -1,    59,
      60,    61,    62,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    -1,    -1,    87,    88,    89,
      90,    -1,    -1,    93,    94,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,   103,    12,    13,    14,    15,    -1,    17,
      18,    19,    20,    21,    -1,    23,    24,    25,    -1,    -1,
      28,    29,    -1,    31,    32,    33,    -1,    -1,    -1,    37,
      -1,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    81,    82,    83,    84,    -1,    -1,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    -1,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,    12,    13,    14,    15,
      -1,    17,    18,    19,    20,    21,    -1,    23,    24,    25,
      -1,    -1,    28,    29,    -1,    31,    -1,    33,    -1,    -1,
      -1,    37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    61,    62,    -1,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,    -1,
      -1,    87,    88,    -1,    90,    -1,    -1,    93,    94,    -1,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,    12,    13,
      14,    15,    -1,    17,    18,    19,    20,    21,    -1,    23,
      24,    25,    -1,    -1,    28,    29,    -1,    31,    -1,    33,
      -1,    -1,    -1,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    61,    62,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    -1,    -1,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,   103,
      12,    13,    14,    15,    -1,    17,    18,    19,    20,    21,
      -1,    23,    24,    25,    -1,    -1,    28,    29,    54,    31,
      -1,    33,    -1,    -1,    -1,    37,    -1,    39,    40,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    49,    74,    51,
      -1,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    61,
      62,    -1,    64,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    73,    98,    -1,    -1,   101,   102,    -1,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    88,    -1,    90,    -1,
      -1,    93,    94,    12,    13,    14,    15,    99,    -1,    -1,
      -1,   103,    -1,    -1,    23,    24,    25,    -1,    -1,    28,
      29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    43,    -1,    45,    46,    47,    48,
      49,    -1,    51,    -1,    12,    13,    14,    15,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    23,    24,    25,    -1,    -1,
      28,    29,    -1,    31,    73,    -1,    -1,    -1,    -1,    -1,
      38,    80,    81,    82,    83,    84,    -1,    45,    -1,    47,
      -1,    49,    -1,    51,    93,    94,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,   103,    63,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    81,    82,    83,    84,    12,    13,    14,
      15,    71,    72,    -1,    74,    93,    94,    77,    23,    24,
      25,    99,    -1,    28,    29,   103,    31,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    -1,    -1,    -1,
      -1,   101,   102,    -1,    49,    -1,    51,    12,    13,    14,
      15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,    24,
      25,    -1,    -1,    28,    29,    -1,    31,    -1,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    49,    -1,    51,    -1,    93,    94,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      12,    13,    14,    15,    71,    72,    -1,    74,    93,    94,
      77,    23,    24,    25,    99,    -1,    28,    29,   103,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,
      -1,    -1,    -1,    -1,   101,   102,    -1,    49,    -1,    51,
      12,    13,    14,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    23,    24,    25,    -1,    -1,    28,    29,    -1,    31,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    49,    -1,    51,
      -1,    93,    94,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    12,    13,    14,    15,    47,    -1,    -1,
      -1,    93,    94,    -1,    23,    24,    25,    99,    -1,    28,
      29,   103,    31,    -1,    -1,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    -1,    74,    -1,    -1,    77,    78,    79,
      49,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    -1,    -1,    -1,
      -1,   101,   102,    42,    73,    -1,    -1,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    93,    94,    65,    66,    -1,    -1,
      99,    70,    71,    72,   103,    74,    75,    76,    77,    78,
      79,    47,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    -1,    65,
      66,    -1,   101,   102,    70,    71,    72,    -1,    74,    -1,
      -1,    77,    78,    79,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    -1,    65,    66,    -1,   101,   102,    70,    71,    72,
      -1,    74,    -1,    -1,    77,    78,    79,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    -1,    65,    66,    -1,   101,   102,
      70,    71,    72,    -1,    74,    -1,    -1,    77,    78,    79,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    -1,    65,    66,
      -1,   101,   102,    70,    71,    72,    -1,    74,    -1,    -1,
      77,    78,    79,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      -1,    65,    66,    -1,   101,   102,    70,    71,    72,    -1,
      74,    -1,    -1,    77,    78,    79,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    -1,    65,    66,    -1,   101,   102,    70,
      71,    72,    -1,    74,    -1,    -1,    77,    78,    79,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    -1,    65,    66,    -1,
     101,   102,    70,    71,    72,    -1,    74,    -1,    -1,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    -1,
      65,    66,    -1,   101,   102,    70,    71,    72,    -1,    74,
      -1,    -1,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    94,
      95,    96,    -1,    65,    66,    -1,   101,   102,    70,    71,
      72,    -1,    74,    -1,    -1,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    86,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    -1,    65,    66,    -1,   101,
     102,    70,    71,    72,    -1,    74,    -1,    -1,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    96,    -1,    65,
      66,   100,   101,   102,    70,    71,    72,    -1,    74,    -1,
      -1,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    -1,    65,    66,   100,   101,   102,    70,    71,    72,
      -1,    74,    -1,    -1,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    -1,    65,    66,    -1,   101,   102,
      70,    71,    72,    -1,    74,    -1,    -1,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    -1,    -1,    -1,
      -1,   101,   102
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    13,    14,    15,    17,    18,    19,    20,
      21,    23,    24,    25,    28,    29,    31,    33,    37,    39,
      40,    41,    44,    49,    50,    51,    55,    59,    60,    61,
      62,    64,    73,    80,    81,    82,    83,    84,    87,    88,
      89,    90,    93,    94,    99,   103,   107,   109,   110,   111,
     112,   113,   120,   121,   122,   127,   128,   129,   131,   135,
     138,   140,   143,   146,    98,   108,    28,    49,   129,   129,
     129,    99,   129,   148,    28,   129,    99,   148,    99,   148,
     139,    50,    28,   132,    38,    43,    45,    46,    47,    48,
     129,   136,   137,    28,   129,    56,    57,    13,    14,    28,
      28,    28,    28,   129,    63,   136,   137,    28,   134,   129,
     129,   129,   129,   129,   129,    28,   133,    28,    99,   129,
     129,   129,   129,   147,     0,   108,    99,    91,    99,   119,
     129,   111,   129,    65,    66,    70,    71,    72,    74,    77,
      78,    79,    92,    93,    94,    95,    96,   101,   102,   124,
      99,   129,   129,   148,   105,   147,   148,   129,   147,   147,
     105,   129,   129,    45,   137,   141,    42,    75,    76,    28,
      54,    91,   129,    54,    91,    52,   142,    56,   137,   105,
      47,    54,    47,    54,    47,    54,    47,    54,    85,    86,
     105,    28,   100,   104,   105,   109,   119,   129,   119,   129,
      52,    53,    91,   125,    54,   108,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,    28,   144,   100,   129,   100,   100,    28,   129,
     124,    27,    28,   130,   129,   129,   129,   129,    54,    91,
     129,   129,   124,   129,    28,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,    28,   100,   105,   129,   100,
     125,   100,   124,   126,   129,   129,   126,   105,   108,   144,
     129,   129,   129,   144,    47,    47,    47,    47,   147,   126,
      34,    54,   111,   108,   112,   125,   125,    28,   105,   145,
     108,   108,   129,   129,   129,   129,   100,   125,    54,   129,
     125,    32,    58,   124,    32,    28,   126,   145,   145,    32,
     129,   125,    36,    58,   116,   117,   123,   124,   126,   115,
     116,   118,   123,    32,   126,   126,   125,   126,   124,   117,
     125,   119,   111,   125,   118,   125,   119,   126,   125,   111,
      58,   114,    52,   125,    32,    32,    52,   125,    32,   125,
     124,   125,   124,   124,    32,   111,   112,   126,   125,   125
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   106,   107,   107,   107,   108,   109,   109,   109,   109,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     111,   111,   112,   112,   112,   112,   112,   112,   112,   112,
     113,   113,   113,   113,   113,   113,   113,   114,   114,   115,
     115,   116,   116,   117,   118,   118,   119,   119,   119,   120,
     121,   122,   123,   124,   125,   126,   126,   126,   127,   128,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   130,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   132,   132,   133,   133,   134,   134,
     135,   135,   135,   135,   135,   135,   136,   136,   137,   137,
     138,   138,   138,   138,   139,   138,   141,   140,   140,   142,
     140,   140,   140,   143,   144,   144,   144,   144,   145,   146,
     147,   147,   147,   148,   148
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     0,     1,     1,     1,
       4,     4,     4,     4,     4,     4,     5,     4,     4,     5,
       1,     1,     1,     1,     7,    10,    11,     3,     6,     4,
       6,     9,     9,     9,     6,    10,    10,     0,     3,     2,
       1,     2,     1,     6,     1,     6,     1,     3,     3,     2,
       3,     1,     1,     0,     0,     0,     2,     2,     3,     1,
       1,     1,     1,     1,     1,     2,     2,     4,     4,     1,
       1,     2,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     3,     3,     3,
       2,     2,     3,     3,     4,     4,     4,     6,     4,     6,
       4,     6,     4,     6,     2,     2,     1,     1,     2,     1,
       2,     2,     2,     1,     2,     2,     1,     2,     4,     4,
       6,     4,     2,     3,     1,     3,     1,     3,     1,     3,
       2,     2,     2,     2,     3,     2,     2,     1,     3,     2,
       2,     2,     3,     2,     0,     3,     0,     8,     2,     0,
       8,     7,     6,     2,     0,     1,     3,     4,     0,     2,
       0,     1,     3,     1,     3
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
#line 121 "engines/director/lingo/lingo-gr.y"
    { yyerrok; }
#line 1884 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5:
#line 124 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1;
	}
#line 1893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 135 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 1904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 141 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_assign);
			(yyval.code) = (yyvsp[-2].code); }
#line 1912 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 144 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_after); }
#line 1918 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 145 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_before); }
#line 1924 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 146 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 1935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 152 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 1947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 159 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 1958 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 165 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 1969 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 171 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 1981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 178 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 1992 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24:
#line 196 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[-2].code) - (yyvsp[-6].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-6].code));
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[-6].code) + 2] = end;	/* end, if cond fails */

		checkEnd((yyvsp[0].s), "repeat", true); }
#line 2005 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25:
#line 209 "engines/director/lingo/lingo-gr.y"
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
#line 2024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26:
#line 227 "engines/director/lingo/lingo-gr.y"
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
#line 2043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27:
#line 241 "engines/director/lingo/lingo-gr.y"
    {
			inst end = 0;
			WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-2].code));
			g_lingo->code1(STOP);
			(*g_lingo->_currentScript)[(yyvsp[-2].code) + 1] = end;
		}
#line 2054 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28:
#line 247 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
			checkEnd((yyvsp[0].s), "tell", true); }
#line 2062 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 250 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		}
#line 2070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 255 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-2].code) - (yyvsp[-5].code));
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[0].s), "if", true);

		g_lingo->processIf(0, 0); }
#line 2085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 265 "engines/director/lingo/lingo-gr.y"
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
#line 2102 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 277 "engines/director/lingo/lingo-gr.y"
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
#line 2119 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 289 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-5].code) - (yyvsp[-8].code));
		WRITE_UINT32(&else1, (yyvsp[-2].code) - (yyvsp[-8].code));
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-8].code));
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-8].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf(0, (yyvsp[0].code) - (yyvsp[-8].code)); }
#line 2134 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 299 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-2].code) - (yyvsp[-5].code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, 0); }
#line 2149 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 309 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-6].code) - (yyvsp[-9].code));
		WRITE_UINT32(&else1, (yyvsp[-2].code) - (yyvsp[-9].code));
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-9].code));
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, 0); }
#line 2164 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 319 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[-6].code) - (yyvsp[-9].code));
		WRITE_UINT32(&else1, (yyvsp[-4].code) - (yyvsp[-9].code));
		WRITE_UINT32(&end, (yyvsp[0].code) - (yyvsp[-9].code));
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[-9].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, (yyvsp[0].code) - (yyvsp[-9].code)); }
#line 2179 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 331 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; }
#line 2185 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 332 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[-1].code); }
#line 2191 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 343 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-2].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2202 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 352 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[-1].code) - (yyvsp[-5].code));
		(*g_lingo->_currentScript)[(yyvsp[-5].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[-5].code)); }
#line 2213 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 360 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); }
#line 2219 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 361 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); }
#line 2225 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 365 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); }
#line 2231 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 368 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2241 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 375 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); }
#line 2251 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 382 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); }
#line 2262 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 390 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2268 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 393 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); }
#line 2274 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 396 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); }
#line 2280 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 401 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		delete (yyvsp[-1].s); }
#line 2290 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 407 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_tellcode);
		g_lingo->code1(STOP); }
#line 2298 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 411 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2306 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 414 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2314 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 417 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str()); }
#line 2322 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 420 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str()); }
#line 2330 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 423 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[0].s), 0);
		delete (yyvsp[0].s); }
#line 2338 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 426 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2346 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 429 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg)); }
#line 2352 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 430 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg)); }
#line 2358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 431 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 434 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2375 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 438 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 446 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2399 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 453 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); }
#line 2405 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 454 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); }
#line 2411 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 455 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); }
#line 2417 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 456 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); }
#line 2423 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 457 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); }
#line 2429 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 458 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); }
#line 2435 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 459 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); }
#line 2441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 460 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); }
#line 2447 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 461 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); }
#line 2453 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 462 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); }
#line 2459 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 463 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); }
#line 2465 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 464 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); }
#line 2471 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 465 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); }
#line 2477 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 466 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); }
#line 2483 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 467 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); }
#line 2489 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 468 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); }
#line 2495 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 469 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); }
#line 2501 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 470 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[0].code); }
#line 2507 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 471 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[0].code); g_lingo->code1(g_lingo->c_negate); }
#line 2513 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 472 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[-1].code); }
#line 2519 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 473 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_arraypush); g_lingo->codeArray((yyvsp[-1].narg)); }
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 474 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); }
#line 2531 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 475 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); }
#line 2537 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 476 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); }
#line 2543 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 477 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); }
#line 2549 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 478 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); }
#line 2555 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 479 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); }
#line 2561 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 480 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); }
#line 2567 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 481 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); }
#line 2573 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 482 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); }
#line 2579 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 483 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); }
#line 2585 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 486 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2593 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 491 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); }
#line 2599 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 494 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); }
#line 2605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 495 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_procret); }
#line 2611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 499 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[0].s), 0);
		delete (yyvsp[0].s); }
#line 2619 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 502 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2627 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 505 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2635 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 508 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_voidpush);
		g_lingo->codeFunc((yyvsp[0].s), 1);
		delete (yyvsp[0].s); }
#line 2644 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 512 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg)); }
#line 2650 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 513 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg)); }
#line 2656 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 514 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[-1].s), 0); }
#line 2662 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 515 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[-3].s), (yyvsp[-1].narg)); }
#line 2668 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 516 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); }
#line 2674 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 517 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); }
#line 2680 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 518 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[-2].s)); s += '-'; s += *(yyvsp[-1].s); g_lingo->codeFunc(&s, (yyvsp[0].narg)); }
#line 2686 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 521 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2692 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 522 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2698 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 525 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2704 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 526 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2710 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 529 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2716 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 530 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[0].s)->c_str()); delete (yyvsp[0].s); }
#line 2722 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 541 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 542 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); }
#line 2734 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 543 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); }
#line 2740 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 544 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_goto); }
#line 2749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 548 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_goto); }
#line 2758 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 552 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_goto); }
#line 2767 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 566 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); }
#line 2773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 567 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_play); }
#line 2782 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 571 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_play); }
#line 2791 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 575 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_play); }
#line 2800 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 579 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); }
#line 2806 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 579 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg)); }
#line 2814 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 609 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); }
#line 2820 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 610 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg));
			g_lingo->_indef = false; }
#line 2829 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 614 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeFactory(*(yyvsp[0].s));
		}
#line 2837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 617 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; }
#line 2843 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 618 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; }
#line 2852 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 622 "engines/director/lingo/lingo-gr.y"
    {	// D3
				g_lingo->code1(g_lingo->c_procret);
				g_lingo->define(*(yyvsp[-6].s), (yyvsp[-5].code), (yyvsp[-4].narg));
				g_lingo->_indef = false;
				g_lingo->_ignoreMe = false;

				checkEnd((yyvsp[0].s), (yyvsp[-6].s)->c_str(), false);
			}
#line 2865 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 630 "engines/director/lingo/lingo-gr.y"
    {	// D4. No 'end' clause
				g_lingo->code1(g_lingo->c_procret);
				g_lingo->define(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
				g_lingo->_indef = false;
				g_lingo->_ignoreMe = false;
			}
#line 2876 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 637 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[0].s); g_lingo->_indef = true; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 2882 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 639 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; }
#line 2888 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 640 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = 1; }
#line 2894 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 641 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2900 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 642 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[0].s)); (yyval.narg) = (yyvsp[-3].narg) + 1; }
#line 2906 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 645 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); }
#line 2912 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 649 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar); }
#line 2923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 657 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; }
#line 2929 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 658 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; }
#line 2935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 659 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2941 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 662 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; }
#line 2947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 663 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 2953 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 2957 "engines/director/lingo/lingo-gr.cpp"

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
#line 666 "engines/director/lingo/lingo-gr.y"

