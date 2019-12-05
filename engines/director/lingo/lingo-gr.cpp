/* A Bison parser, made by GNU Bison 3.4.2.  */

/* Skeleton implementation for Bison GLR parsers in C

   Copyright (C) 2002-2015, 2018-2019 Free Software Foundation, Inc.

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

/* C GLR parser skeleton written by Paul Hilfinger.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.4.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "glr.c"

/* Pure parsers.  */
#define YYPURE 0






/* First part of user prologue.  */
#line 50 "engines/director/lingo/lingo-gr.y"

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


#line 87 "engines/director/lingo/lingo-gr.cpp"


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

#include "lingo-gr.h"

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Default (constant) value used for initialization for null
   right-hand sides.  Unlike the standard yacc.c template, here we set
   the default value of $$ to a zeroed-out value.  Since the default
   value is undefined, this behavior is technically correct.  */
static YYSTYPE yyval_default;



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#ifndef YYFREE
# define YYFREE free
#endif
#ifndef YYMALLOC
# define YYMALLOC malloc
#endif
#ifndef YYREALLOC
# define YYREALLOC realloc
#endif

#define YYSIZEMAX ((size_t) -1)

#ifdef __cplusplus
  typedef bool yybool;
# define yytrue true
# define yyfalse false
#else
  /* When we move to stdbool, get rid of the various casts to yybool.  */
  typedef unsigned char yybool;
# define yytrue 1
# define yyfalse 0
#endif

#ifndef YYSETJMP
# include <setjmp.h>
# define YYJMP_BUF jmp_buf
# define YYSETJMP(Env) setjmp (Env)
/* Pacify Clang and ICC.  */
# define YYLONGJMP(Env, Val)                    \
 do {                                           \
   longjmp (Env, Val);                          \
   YYASSERT (0);                                \
 } while (yyfalse)
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

/* The _Noreturn keyword of C11.  */
#ifndef _Noreturn
# if (defined __cplusplus \
      && ((201103 <= __cplusplus && !(__GNUC__ == 4 && __GNUC_MINOR__ == 7)) \
          || (defined _MSC_VER && 1900 <= _MSC_VER)))
#  define _Noreturn [[noreturn]]
# elif ((!defined __cplusplus || defined __clang__) \
        && (201112 <= (defined __STDC_VERSION__ ? __STDC_VERSION__ : 0)  \
            || 4 < __GNUC__ + (7 <= __GNUC_MINOR__)))
   /* _Noreturn works as-is.  */
# elif 2 < __GNUC__ + (8 <= __GNUC_MINOR__) || 0x5110 <= __SUNPRO_C
#  define _Noreturn __attribute__ ((__noreturn__))
# elif 1200 <= (defined _MSC_VER ? _MSC_VER : 0)
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn
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


#ifndef YYASSERT
# define YYASSERT(Condition) ((void) ((Condition) || (abort (), 0)))
#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  126
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3379

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  107
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  41
/* YYNRULES -- Number of rules.  */
#define YYNRULES  163
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  358
/* YYMAXRHS -- Maximum number of symbols on right-hand side of rule.  */
#define YYMAXRHS 11
/* YYMAXLEFT -- Maximum number of symbols to the left of a handle
   accessed by $0, $-1, etc., in any rule.  */
#define YYMAXLEFT 0

/* YYMAXUTOK -- Last valid token number (for yychar).  */
#define YYMAXUTOK   347
/* YYUNDEFTOK -- Symbol number (for yytoken) that denotes an unknown
   token.  */
#define YYUNDEFTOK  2

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const unsigned char yytranslate[] =
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
/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   121,   121,   122,   123,   126,   127,   128,   131,   137,
     140,   141,   142,   148,   155,   161,   168,   174,   182,   183,
     184,   187,   188,   193,   206,   224,   238,   243,   246,   251,
     261,   271,   281,   293,   294,   297,   303,   311,   312,   318,
     322,   325,   329,   333,   336,   343,   350,   358,   361,   364,
     365,   366,   369,   375,   379,   382,   385,   388,   391,   397,
     398,   399,   402,   405,   406,   409,   417,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   459,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   475,   478,   481,   485,   486,   487,   488,   489,   490,
     493,   494,   497,   498,   501,   502,   513,   514,   515,   516,
     520,   524,   530,   531,   534,   535,   538,   539,   543,   547,
     551,   551,   581,   581,   586,   587,   587,   592,   599,   605,
     607,   608,   609,   610,   613,   614,   615,   618,   622,   630,
     631,   632,   635,   636
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
  "program", "programline", "asgn", "stmtoneliner", "stmt", "ifstmt",
  "elseifstmtlist", "elseifstmt", "endifstmt", "repeatwhile", "repeatwith",
  "if", "elseif", "begin", "end", "stmtlist", "when", "tell", "simpleexpr",
  "expr", "reference", "proc", "globallist", "propertylist",
  "instancelist", "gotofunc", "gotoframe", "gotomovie", "playfunc", "$@1",
  "defn", "$@2", "$@3", "on", "argdef", "endargdef", "argstore", "macro",
  "arglist", "nonemptyarglist", YY_NULLPTR
};
#endif

#define YYPACT_NINF -290
#define YYTABLE_NINF -31

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const short yypact[] =
{
     341,   -83,  -290,  -290,  1231,  -290,  -290,  1271,  1231,  1231,
      16,  -290,  1231,  1231,  1231,  1352,  -290,  -290,  -290,    21,
      22,  1110,  -290,    53,  1231,    94,    -2,    67,    90,   126,
    1231,  1150,   130,  1231,  1231,  1231,  1231,  1231,  1231,  -290,
     136,   137,    63,  1231,  1231,  1231,  1231,     6,  -290,  -290,
    -290,  -290,  -290,  1231,    99,  1231,   857,  1231,  -290,  1945,
    -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,    71,  1231,
    -290,  1978,  2011,  2044,  2077,    82,  1392,  2110,    86,  2143,
    1231,    82,  1473,  -290,  -290,    87,  1231,  -290,  1231,  -290,
     152,  -290,  2176,    77,  -290,  -290,  1507,   143,  -290,    -8,
    1231,    10,   148,  -290,  -290,   919,  -290,    77,  -290,  -290,
      98,   -43,    19,  1578,  1611,  1644,  1875,  -290,   101,  -290,
     179,   -41,     0,  1005,  2209,     4,  -290,   427,  2242,  1231,
     146,  -290,  1842,  1231,  1231,  1231,  1231,  1231,  1231,  1231,
    1231,  1231,  1231,  1231,  1231,  1231,  1231,  1231,  1231,  1231,
     180,  1231,  1545,  1231,   109,  1908,   -82,   109,   191,  2275,
    2308,  1231,  -290,  -290,   125,  1231,  1231,  -290,  1231,  1231,
     153,   833,  1231,  1231,  -290,  -290,  1231,  -290,   193,  1231,
    1231,  1231,  1231,  1231,  1231,  1231,  1231,  1231,  1231,   200,
     -51,  -290,  -290,  1231,  -290,   599,  2341,  -290,  -290,  -290,
    1231,  -290,   158,   202,  1173,  1294,  3199,  3232,    15,  2374,
    1467,  3243,   230,   312,   398,    48,    95,  2407,  2440,  -290,
     -35,  2473,  -290,  -290,  2506,   180,  -290,  -290,  2539,  2572,
    2605,  3254,  1231,  1231,  2638,  3265,   180,  2671,  -290,  2704,
    1442,  2737,  1677,  2770,  1710,  2803,  1743,  2836,  2869,  -290,
    -290,  1231,  2902,  -290,  -290,   199,   -11,   857,   599,  2935,
     599,   129,   208,   -29,  3276,  2968,     2,  1231,  1231,  1231,
    1231,     5,  -290,   188,  1231,  -290,    51,   211,   216,  -290,
    -290,   129,   129,  3001,  3034,  3067,  3100,  -290,  1231,  3133,
     -16,  -290,  -290,  -290,   212,  1231,  -290,  -290,   513,  -290,
    -290,  3166,  -290,  -290,    20,  -290,  1776,   226,   685,   771,
    -290,   599,    20,  -290,  -290,  -290,  -290,  1231,  -290,  -290,
    -290,   156,   599,   224,  -290,   943,  1029,  -290,  1809,   857,
    -290,   237,   234,  -290,  -290,  -290,   599,  -290,   599,  -290,
    -290,   599,  -290,  -290,  -290,   235,  -290,   239,   857,  -290,
    -290,  -290,  -290,  -290,   240,   857,  -290,  -290
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,    54,    65,     0,    55,   110,   113,     0,     0,
       0,    61,     0,     0,     0,    58,    57,    56,   140,   106,
       0,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,   100,     0,     0,     0,   159,     0,     3,    67,
      21,     7,    22,     0,     0,     0,     0,     0,    59,    19,
      60,    20,   103,   104,     6,    47,    18,     4,    58,     0,
     100,    66,   112,   111,   162,   114,   159,    62,    63,   101,
     159,   158,   159,   105,   120,   107,     0,   126,     0,   127,
       0,   128,   133,   129,   131,   142,   102,     0,    43,     0,
       0,     0,     0,   144,   145,   118,   136,   137,   139,   124,
     109,    81,     0,     0,     0,     0,     0,   122,   108,   149,
       0,    86,    87,     0,   160,     0,     1,     5,    49,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     150,   159,     0,     0,   119,   160,     0,   141,     0,   132,
     135,     0,   130,    47,     0,     0,     0,    44,     0,     0,
      59,     0,     0,     0,    52,    47,     0,   138,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    88,    89,     0,     2,    48,    48,    47,    49,    26,
       0,    49,    77,    78,    75,    76,    79,    80,    72,    83,
      84,    85,    82,    68,    69,    70,    71,    73,    74,   151,
       0,   163,    64,   121,   134,   150,     8,     9,    10,    11,
      15,    13,     0,     0,    14,    12,   150,   117,   125,    92,
       0,    94,     0,    96,     0,    98,     0,    90,    91,   123,
     115,   159,   161,    50,    51,     0,     0,     0,    48,    28,
      48,   157,     0,     0,    17,    16,     0,     0,     0,     0,
       0,     0,    23,     0,     0,    48,    33,     0,     0,    49,
     152,   157,   157,    93,    95,    97,    99,   116,     0,    48,
      30,    32,    46,    48,    33,     0,    27,   153,   148,    49,
      49,    48,    49,    48,     0,    34,     0,   154,   143,   146,
      49,    48,     0,    37,    47,    47,    48,     0,    47,    47,
     155,   147,    48,     0,    48,    49,    49,    31,     0,     0,
      49,     0,     0,    24,    29,    48,    48,    48,    48,    47,
      48,    48,   156,    25,    40,     0,    42,     0,     0,    35,
      36,    39,    41,    48,     0,     0,    48,    38
};

  /* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -290,  -290,   149,  -290,   -54,     1,  -290,  -171,  -290,   -33,
    -290,  -290,  -290,  -289,  -128,    -1,   -94,  -290,  -290,   182,
      -4,   116,  -290,  -290,  -290,  -290,  -290,   252,   -19,  -290,
    -290,  -290,  -290,  -290,  -290,  -169,  -290,  -123,  -290,   -39,
     133
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const short yydefgoto[] =
{
      -1,    47,    48,    49,    50,   254,    52,   293,   294,   316,
      53,    54,    55,   295,   150,   199,   195,    56,    57,    58,
      59,    60,    61,    85,   118,   110,    62,    93,    94,    63,
      82,    64,   163,   175,    65,   220,   321,   279,    66,   156,
      75
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const short yytable[] =
{
      71,    51,   131,    72,    73,    74,   126,   125,    77,    74,
      79,    74,   108,    99,   100,   317,    67,    92,   -30,   222,
      96,   -30,   292,   317,   193,   273,   105,    92,   101,   111,
     112,   113,   114,   115,   116,   225,   140,   154,   140,   121,
     122,   123,   124,   157,   -30,   274,    76,   236,   168,   128,
     250,   130,    84,   132,   313,   251,   263,   314,   292,   148,
     149,   148,   149,   169,   261,   152,   172,   266,   179,   257,
     281,   262,   124,    83,   162,   180,   155,   262,   124,   140,
     315,   173,   159,    95,   160,   291,   133,   134,   177,   292,
     135,   136,   137,   138,   140,   139,   171,   102,   140,   141,
     142,   282,   148,   149,   258,   127,   287,   260,   262,   192,
     193,   193,   143,   144,   145,   146,   147,   148,   149,   303,
     103,   148,   149,   305,    88,   196,    90,   140,    51,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,    78,   124,    81,   221,
     148,   149,    97,    98,    14,   226,   104,   224,   299,   300,
     109,   228,   229,   120,   230,   231,   117,   119,   234,   235,
     129,   151,   237,   167,   140,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   298,   325,   326,   153,   252,
     329,   330,   153,   158,   255,   256,   259,   148,   149,   161,
     197,   198,   174,   275,   178,   308,   309,   189,   311,   190,
     219,   348,   271,   133,   134,   193,   322,   135,   136,   137,
     138,   223,   139,   238,   232,   140,   141,   142,   264,   265,
     249,   336,   338,   272,   139,   278,   341,   140,   280,   143,
     144,   145,   146,   147,   288,   296,   297,   124,   148,   149,
     292,   143,   144,   145,   146,   147,   320,   276,   333,   277,
     148,   149,   331,   283,   284,   285,   286,   342,   343,   351,
     289,   335,   337,   352,   290,   340,   194,   355,   139,   324,
     227,   140,   170,   107,   301,     0,     0,     0,   302,     0,
       0,   306,   304,     0,   353,   143,   144,   145,   146,   147,
     310,   356,   312,     0,   148,   149,   139,     0,     0,   140,
     323,     0,     0,   328,     0,   327,     0,     0,     0,     0,
       0,   332,     0,   334,   144,   145,   146,   147,     0,     0,
       0,     0,   148,   149,   344,   345,   346,   347,     0,   349,
     350,    -5,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   354,     2,     0,   357,     3,     4,     5,     0,
       6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
      14,    15,    16,     0,    17,     0,    18,     0,     0,     0,
      19,     0,    20,    21,    22,     0,     0,    23,   139,     0,
       0,   140,    24,    25,    26,     0,     0,     0,    27,     0,
       0,     0,    28,    29,    30,    31,     0,    32,   146,   147,
       0,     0,     0,     0,   148,   149,    33,     0,     0,     0,
       0,     0,     0,    34,    35,    36,    37,    38,     0,     0,
      39,    40,    41,    42,     0,    43,    44,     0,     0,     2,
      -5,    45,     3,     4,     5,    46,     6,     7,     8,     9,
      10,     0,    11,    12,    13,     0,    14,    15,    16,     0,
      17,     0,    18,     0,     0,     0,    19,     0,    20,    21,
      22,     0,     0,    23,   139,     0,     0,   140,    24,    25,
      26,     0,     0,     0,    27,     0,     0,     0,    28,    29,
      30,    31,     0,    32,   146,   147,     0,     0,     0,     0,
     148,   149,    33,     0,     0,     0,     0,     0,     0,    34,
      35,    36,    37,    38,     0,     0,    39,    40,    41,    42,
       0,    43,    44,     0,     0,     2,     0,    45,     3,     4,
       5,    46,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,    14,    15,    16,     0,    17,   307,    18,     0,
       0,     0,    19,     0,    20,    21,    22,     0,     0,     0,
       0,     0,     0,     0,    24,    25,    26,     0,     0,     0,
      27,     0,     0,     0,     0,     0,    30,    31,     0,    32,
       0,     0,     0,     0,     0,     0,     0,     0,    33,     0,
       0,     0,     0,     0,     0,    34,    35,    36,    37,    38,
       0,     0,    39,    40,     0,    42,     0,    43,    44,     0,
       0,     2,   253,    45,     3,     4,     5,    46,     6,     7,
       8,     9,    10,     0,    11,    12,    13,     0,    14,    15,
      16,     0,    17,     0,    18,     0,     0,     0,    19,     0,
      20,    21,    22,     0,     0,     0,     0,     0,     0,     0,
      24,    25,    26,     0,     0,     0,    27,     0,     0,     0,
       0,     0,    30,    31,     0,    32,     0,     0,     0,     0,
       0,     0,     0,     0,    33,     0,     0,     0,     0,     0,
       0,    34,    35,    36,    37,    38,     0,     0,    39,    40,
       0,    42,     0,    43,    44,     0,     0,     2,   253,    45,
       3,     4,     5,    46,     6,     7,     8,     9,    10,     0,
      11,    12,    13,     0,    14,    15,    16,     0,    17,     0,
      18,     0,     0,     0,    19,     0,    20,    21,    22,     0,
       0,     0,     0,     0,     0,     0,    24,    25,    26,     0,
       0,     0,    27,     0,     0,     0,     0,     0,    30,    31,
       0,    32,     0,     0,     0,     0,     0,     0,     0,     0,
      33,     0,     0,     0,     0,     0,     0,    34,    35,    36,
      37,    38,     0,     0,    39,    40,     0,    42,     0,    43,
      44,     0,     0,     2,   253,    45,     3,     4,     5,    46,
       6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
      14,    15,    16,     0,    17,     0,    18,     0,     0,     0,
      19,     0,    20,    21,    22,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,     0,     0,     0,    27,     0,
       0,     0,     0,     0,    30,    31,     0,    32,     0,     0,
       0,     0,     0,     0,     0,     0,    33,     0,     0,     0,
       0,     0,     0,    34,    35,    36,    37,    38,     0,     0,
      39,    40,     0,    42,     0,    43,    44,     0,     0,     2,
     253,    45,     3,     4,     5,    46,     6,     7,     8,     9,
      10,     0,    11,    12,    13,     0,    14,    15,    16,   233,
      17,     0,    18,     0,     0,     0,    19,     0,    20,    21,
     133,   134,     0,     0,   135,   136,   137,   138,    24,   139,
      26,     0,   140,   141,   142,     0,     0,     0,     0,     0,
      30,    31,     0,    32,     0,     0,   143,   144,   145,   146,
     147,     0,    33,     0,     0,   148,   149,     0,     0,    34,
      35,    36,    37,    38,     0,     0,     0,    40,     0,    42,
       0,    43,    44,     0,     0,     2,     0,    45,     3,     4,
       5,    46,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,    14,    15,    16,     0,    17,   176,    18,     0,
       0,     0,    19,     0,    20,    21,   133,   134,     0,     0,
     135,   136,   137,   138,    24,   139,    26,     0,   140,   141,
     142,     0,     0,     0,     0,     0,    30,    31,     0,    32,
       0,     0,   143,   144,   145,   146,   147,     0,    33,     0,
       0,   148,   149,     0,     0,    34,    35,    36,    37,    38,
       0,     0,     0,    40,     0,    42,     0,    43,    44,     0,
       0,     2,     0,    45,     3,     4,     5,    46,     6,     7,
       8,     9,    10,     0,    11,    12,    13,     0,    14,    15,
      16,     0,    17,     0,    18,     0,     0,     0,    19,     0,
      20,    21,   133,   134,     0,     0,   135,   136,   137,   138,
      24,   139,    26,     0,   140,   141,   142,     0,     0,     0,
       0,     0,    30,    31,     0,    32,     0,     0,   143,   144,
     145,   146,   147,     0,    33,     0,   191,   148,   149,     0,
       0,    34,    35,    36,    37,    38,     0,     0,     0,    40,
       0,    42,     2,    43,    44,     3,     4,     5,     0,    45,
       0,     0,     0,    46,     0,    11,    12,    13,     0,    14,
      68,    16,     0,    17,     0,     0,     0,     0,     0,     0,
      86,     0,     0,     0,     0,    87,     0,    88,    89,    90,
      91,    69,     2,    26,     0,     3,     4,     5,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,    14,
      68,    16,     0,    17,     0,    33,     0,     0,     0,     0,
      86,     0,    34,    35,    36,    37,    38,    88,     0,    90,
       0,    69,    70,    26,    43,    44,     0,     0,     0,     0,
      45,     0,     0,     0,    46,   106,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    70,     2,    43,    44,     3,     4,     5,   139,
      45,     0,   140,     0,    46,     0,    11,    12,    13,     0,
      14,    68,    16,     0,    17,     0,   143,   144,   145,   146,
     147,     0,     0,     0,     0,   148,   149,     0,     0,     0,
       0,     0,    69,     2,    26,     0,     3,     4,     5,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
      14,    68,    16,     0,    17,     0,    33,     0,     0,     0,
       0,     0,     0,    34,    35,    36,    37,    38,     0,     0,
       0,     0,    69,    70,    26,    43,    44,     0,     0,     0,
       0,    45,     0,     0,     0,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    33,     0,     0,     0,
       0,     0,     0,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    70,     2,    43,    44,     3,     4,     5,
     139,    45,     0,   140,     0,    46,     0,    11,    12,    13,
       0,    14,    68,    16,     0,    17,     0,   143,   144,   145,
     146,   147,     0,     0,     0,     0,   148,   149,     0,     0,
       0,     0,     0,    69,     2,    26,     0,     3,     4,     5,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,    14,    68,    16,     0,    17,     0,    33,     0,     0,
       0,     0,     0,     0,    34,    35,    36,    37,    38,     0,
       0,     0,     0,    69,    70,    26,    43,    44,     0,     0,
       0,     0,    80,     0,     0,     0,    46,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,     0,     0,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    70,     2,    43,    44,     3,     4,
       5,   267,    45,     0,     0,     0,    46,     0,    11,    12,
      13,     0,    14,    68,    16,     0,    17,     0,     0,   133,
     134,     0,     0,   135,   136,   137,   138,     0,   139,     0,
       0,   140,   141,   142,    69,     0,    26,     0,     0,     0,
       0,     0,     0,     0,     0,   143,   144,   145,   146,   147,
       0,     0,     0,   139,   148,   149,   140,     0,    33,     0,
       0,   164,     0,     0,     0,    34,    35,    36,    37,    38,
     143,   144,   145,   146,   147,    70,     0,    43,    44,   148,
     149,     0,     0,    45,   133,   134,     0,    46,   135,   136,
     137,   138,     0,   139,   165,   166,   140,   141,   142,   164,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     143,   144,   145,   146,   147,     0,     0,     0,     0,   148,
     149,     0,   133,   134,     0,     0,   135,   136,   137,   138,
       0,   139,   165,   166,   140,   141,   142,   181,     0,     0,
       0,     0,     0,     0,   182,     0,     0,     0,   143,   144,
     145,   146,   147,     0,     0,   133,   134,   148,   149,   135,
     136,   137,   138,     0,   139,     0,     0,   140,   141,   142,
     183,     0,     0,     0,     0,     0,     0,   184,     0,     0,
       0,   143,   144,   145,   146,   147,     0,     0,   133,   134,
     148,   149,   135,   136,   137,   138,     0,   139,     0,     0,
     140,   141,   142,   185,     0,     0,     0,     0,     0,     0,
     186,     0,     0,     0,   143,   144,   145,   146,   147,     0,
       0,   133,   134,   148,   149,   135,   136,   137,   138,     0,
     139,     0,     0,   140,   141,   142,   268,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   143,   144,   145,
     146,   147,     0,     0,   133,   134,   148,   149,   135,   136,
     137,   138,     0,   139,     0,     0,   140,   141,   142,   269,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     143,   144,   145,   146,   147,     0,     0,   133,   134,   148,
     149,   135,   136,   137,   138,     0,   139,     0,     0,   140,
     141,   142,   270,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   143,   144,   145,   146,   147,     0,     0,
     133,   134,   148,   149,   135,   136,   137,   138,     0,   139,
       0,     0,   140,   141,   142,     0,     0,     0,     0,     0,
     318,   319,     0,     0,     0,     0,   143,   144,   145,   146,
     147,     0,     0,   133,   134,   148,   149,   135,   136,   137,
     138,     0,   139,     0,     0,   140,   141,   142,     0,     0,
       0,     0,     0,   339,     0,     0,     0,     0,     0,   143,
     144,   145,   146,   147,     0,     0,   133,   134,   148,   149,
     135,   136,   137,   138,     0,   139,     0,     0,   140,   141,
     142,     0,     0,     0,     0,     0,     0,     0,   200,     0,
       0,     0,   143,   144,   145,   146,   147,     0,     0,   133,
     134,   148,   149,   135,   136,   137,   138,     0,   139,     0,
       0,   140,   141,   142,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   143,   144,   145,   146,   147,
       0,   201,   133,   134,   148,   149,   135,   136,   137,   138,
       0,   139,     0,     0,   140,   141,   142,     0,     0,     0,
       0,     0,   187,   188,     0,     0,     0,     0,   143,   144,
     145,   146,   147,     0,     0,   133,   134,   148,   149,   135,
     136,   137,   138,     0,   139,     0,     0,   140,   141,   142,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   143,   144,   145,   146,   147,     0,     0,     0,   191,
     148,   149,   133,   134,     0,     0,   135,   136,   137,   138,
       0,   139,     0,     0,   140,   141,   142,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   144,
     145,   146,   147,     0,     0,   133,   134,   148,   149,   135,
     136,   137,   138,     0,   139,     0,     0,   140,   141,   142,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   143,   144,   145,   146,   147,     0,     0,   133,   134,
     148,   149,   135,   136,   137,   138,     0,   139,     0,     0,
     140,   141,   142,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,   144,   145,   146,   147,     0,
       0,   133,   134,   148,   149,   135,   136,   137,   138,     0,
     139,     0,     0,   140,   141,   142,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   143,   144,   145,
     146,   147,     0,     0,   133,   134,   148,   149,   135,   136,
     137,   138,     0,   139,     0,     0,   140,   141,   142,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     143,   144,   145,   146,   147,     0,     0,   133,   134,   148,
     149,   135,   136,   137,   138,     0,   139,     0,     0,   140,
     141,   142,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   143,   144,   145,   146,   147,     0,     0,
     133,   134,   148,   149,   135,   136,   137,   138,     0,   139,
       0,     0,   140,   141,   142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   143,   144,   145,   146,
     147,     0,     0,   133,   134,   148,   149,   135,   136,   137,
     138,     0,   139,     0,     0,   140,   141,   142,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   143,
     144,   145,   146,   147,     0,     0,   133,   134,   148,   149,
     135,   136,   137,   138,     0,   139,     0,     0,   140,   141,
     142,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   143,   144,   145,   146,   147,     0,     0,   133,
     134,   148,   149,   135,   136,   137,   138,     0,   139,     0,
       0,   140,   141,   142,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   143,   144,   145,   146,   147,
       0,     0,   133,   134,   148,   149,   135,   136,   137,   138,
       0,   139,     0,     0,   140,   141,   142,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   144,
     145,   146,   147,     0,     0,   133,   134,   148,   149,   135,
     136,   137,   138,     0,   139,     0,     0,   140,   141,   142,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   143,   144,   145,   146,   147,     0,     0,   133,   134,
     148,   149,   135,   136,   137,   138,     0,   139,     0,     0,
     140,   141,   142,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,   144,   145,   146,   147,     0,
       0,   133,   134,   148,   149,   135,   136,   137,   138,     0,
     139,     0,     0,   140,   141,   142,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   143,   144,   145,
     146,   147,     0,     0,   133,   134,   148,   149,   135,   136,
     137,   138,     0,   139,     0,     0,   140,   141,   142,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     143,   144,   145,   146,   147,     0,     0,   133,   134,   148,
     149,   135,   136,   137,   138,     0,   139,     0,     0,   140,
     141,   142,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   143,   144,   145,   146,   147,     0,     0,
     133,   134,   148,   149,   135,   136,   137,   138,     0,   139,
       0,     0,   140,   141,   142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   143,   144,   145,   146,
     147,     0,     0,   133,   134,   148,   149,   135,   136,   137,
     138,     0,   139,     0,     0,   140,   141,   142,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   143,
     144,   145,   146,   147,     0,     0,   133,   134,   148,   149,
     135,   136,   137,   138,     0,   139,     0,     0,   140,   141,
     142,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   143,   144,   145,   146,   147,     0,     0,   133,
     134,   148,   149,   135,   136,   137,   138,     0,   139,     0,
       0,   140,   141,   142,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   143,   144,   145,   146,   147,
       0,     0,   133,   134,   148,   149,   135,   136,   137,   138,
       0,   139,     0,     0,   140,   141,   142,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   144,
     145,   146,   147,     0,     0,   133,   134,   148,   149,   135,
     136,   137,   138,     0,   139,     0,     0,   140,   141,   142,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   143,   144,   145,   146,   147,     0,     0,   133,   134,
     148,   149,   135,   136,   137,   138,     0,   139,     0,     0,
     140,   141,   142,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,   144,   145,   146,   147,     0,
       0,   133,   134,   148,   149,   135,   136,   137,   138,     0,
     139,     0,     0,   140,   141,   142,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   143,   144,   145,
     146,   147,     0,     0,   133,   134,   148,   149,   135,   136,
     137,   138,     0,   139,     0,     0,   140,   141,   142,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     143,   144,   145,   146,   147,     0,     0,   133,   134,   148,
     149,   135,   136,   137,   138,     0,   139,     0,     0,   140,
     141,   142,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   143,   144,   145,   146,   147,     0,     0,
     133,   134,   148,   149,   135,   136,   137,   138,     0,   139,
       0,     0,   140,   141,   142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   143,   144,   145,   146,
     147,     0,     0,   133,   134,   148,   149,   135,   136,   137,
     138,     0,   139,     0,     0,   140,   141,   142,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   143,
     144,   145,   146,   147,     0,     0,   133,   134,   148,   149,
     135,   136,   137,   138,     0,   139,     0,     0,   140,   141,
     142,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   143,   144,   145,   146,   147,     0,     0,   133,
     134,   148,   149,   135,   136,   137,   138,     0,   139,     0,
       0,   140,   141,   142,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   143,   144,   145,   146,   147,
       0,     0,   133,   134,   148,   149,   135,   136,   137,   138,
       0,   139,     0,     0,   140,   141,   142,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   144,
     145,   146,   147,     0,     0,   133,   134,   148,   149,   135,
     136,   137,   138,     0,   139,     0,     0,   140,   141,   142,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   143,   144,   145,   146,   147,     0,     0,   133,   134,
     148,   149,   135,   136,   137,   138,     0,   139,     0,     0,
     140,   141,   142,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,   144,   145,   146,   147,     0,
       0,   133,   134,   148,   149,   135,   136,   137,   138,     0,
     139,     0,     0,   140,   141,   142,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   143,   144,   145,
     146,   147,     0,     0,   133,   134,   148,   149,   135,   136,
     137,   138,     0,   139,     0,     0,   140,   141,   142,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     143,   144,   145,   146,   147,     0,     0,   133,   134,   148,
     149,   135,   136,   137,   138,     0,   139,     0,     0,   140,
     141,   142,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   143,   144,   145,   146,   147,     0,     0,
     133,   134,   148,   149,   135,   136,   137,   138,     0,   139,
       0,     0,   140,   141,   142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   143,   144,   145,   146,
     147,     0,     0,   133,   134,   148,   149,   135,   136,   137,
     138,     0,   139,     0,     0,   140,   141,   142,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   143,
     144,   145,   146,   147,     0,     0,   133,   134,   148,   149,
     135,   136,     0,     0,     0,   139,     0,     0,   140,   141,
     142,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   143,   144,   145,   146,   147,     0,     0,   133,
     134,   148,   149,   135,   136,     0,     0,     0,   139,     0,
       0,   140,   141,   142,     0,     0,     0,     0,     0,   139,
       0,     0,   140,     0,     0,   143,   144,   145,   146,   147,
     139,     0,     0,   140,   148,   149,   143,   144,   145,   146,
     147,   139,     0,     0,   140,   148,   149,   143,   144,   145,
     146,   147,   139,     0,     0,   140,   148,   149,   143,   144,
     145,   146,   147,     0,     0,     0,     0,   148,   149,   143,
     144,   145,   146,   147,     0,     0,     0,     0,   148,   149
};

static const short yycheck[] =
{
       4,     0,    56,     7,     8,     9,     0,    46,    12,    13,
      14,    15,    31,    15,    16,   304,    99,    21,    34,   101,
      24,    37,    38,   312,   106,    36,    30,    31,    30,    33,
      34,    35,    36,    37,    38,   163,    79,    76,    79,    43,
      44,    45,    46,    82,    60,    56,    30,   175,    56,    53,
     101,    55,    30,    57,    34,   106,   225,    37,    38,   102,
     103,   102,   103,    71,    99,    69,    56,   236,    49,   197,
      99,   106,    76,    52,    93,    56,    80,   106,    82,    79,
      60,    71,    86,    30,    88,    34,    67,    68,   107,    38,
      71,    72,    73,    74,    79,    76,   100,    30,    79,    80,
      81,    99,   102,   103,   198,    99,   101,   201,   106,   105,
     106,   106,    93,    94,    95,    96,    97,   102,   103,   290,
      30,   102,   103,   294,    47,   129,    49,    79,   127,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,    13,   151,    15,   153,
     102,   103,    58,    59,    29,    30,    30,   161,   281,   282,
      30,   165,   166,   100,   168,   169,    30,    30,   172,   173,
      71,   100,   176,    30,    79,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   279,   314,   315,   106,   193,
     318,   319,   106,   106,   195,   196,   200,   102,   103,    47,
      54,    55,    54,   257,   106,   299,   300,   106,   302,    30,
      30,   339,   251,    67,    68,   106,   310,    71,    72,    73,
      74,    30,    76,    30,    71,    79,    80,    81,   232,   233,
      30,   325,   326,    34,    76,   106,   330,    79,    30,    93,
      94,    95,    96,    97,    56,    34,    30,   251,   102,   103,
      38,    93,    94,    95,    96,    97,    30,   258,    34,   260,
     102,   103,   106,   267,   268,   269,   270,    30,    34,    34,
     274,   325,   326,    34,   275,   329,   127,    37,    76,   312,
     164,    79,   100,    31,   288,    -1,    -1,    -1,   289,    -1,
      -1,   295,   293,    -1,   348,    93,    94,    95,    96,    97,
     301,   355,   303,    -1,   102,   103,    76,    -1,    -1,    79,
     311,    -1,    -1,   317,    -1,   316,    -1,    -1,    -1,    -1,
      -1,   322,    -1,   324,    94,    95,    96,    97,    -1,    -1,
      -1,    -1,   102,   103,   335,   336,   337,   338,    -1,   340,
     341,     0,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   353,    12,    -1,   356,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    -1,    25,    26,    27,    -1,
      29,    30,    31,    -1,    33,    -1,    35,    -1,    -1,    -1,
      39,    -1,    41,    42,    43,    -1,    -1,    46,    76,    -1,
      -1,    79,    51,    52,    53,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    61,    62,    63,    64,    -1,    66,    96,    97,
      -1,    -1,    -1,    -1,   102,   103,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    -1,    -1,
      89,    90,    91,    92,    -1,    94,    95,    -1,    -1,    12,
      99,   100,    15,    16,    17,   104,    19,    20,    21,    22,
      23,    -1,    25,    26,    27,    -1,    29,    30,    31,    -1,
      33,    -1,    35,    -1,    -1,    -1,    39,    -1,    41,    42,
      43,    -1,    -1,    46,    76,    -1,    -1,    79,    51,    52,
      53,    -1,    -1,    -1,    57,    -1,    -1,    -1,    61,    62,
      63,    64,    -1,    66,    96,    97,    -1,    -1,    -1,    -1,
     102,   103,    75,    -1,    -1,    -1,    -1,    -1,    -1,    82,
      83,    84,    85,    86,    -1,    -1,    89,    90,    91,    92,
      -1,    94,    95,    -1,    -1,    12,    -1,   100,    15,    16,
      17,   104,    19,    20,    21,    22,    23,    -1,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    34,    35,    -1,
      -1,    -1,    39,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    52,    53,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    63,    64,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      -1,    -1,    89,    90,    -1,    92,    -1,    94,    95,    -1,
      -1,    12,    99,   100,    15,    16,    17,   104,    19,    20,
      21,    22,    23,    -1,    25,    26,    27,    -1,    29,    30,
      31,    -1,    33,    -1,    35,    -1,    -1,    -1,    39,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    52,    53,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    63,    64,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    -1,    -1,    89,    90,
      -1,    92,    -1,    94,    95,    -1,    -1,    12,    99,   100,
      15,    16,    17,   104,    19,    20,    21,    22,    23,    -1,
      25,    26,    27,    -1,    29,    30,    31,    -1,    33,    -1,
      35,    -1,    -1,    -1,    39,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    63,    64,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    -1,    -1,    89,    90,    -1,    92,    -1,    94,
      95,    -1,    -1,    12,    99,   100,    15,    16,    17,   104,
      19,    20,    21,    22,    23,    -1,    25,    26,    27,    -1,
      29,    30,    31,    -1,    33,    -1,    35,    -1,    -1,    -1,
      39,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    52,    53,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    63,    64,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    -1,    -1,
      89,    90,    -1,    92,    -1,    94,    95,    -1,    -1,    12,
      99,   100,    15,    16,    17,   104,    19,    20,    21,    22,
      23,    -1,    25,    26,    27,    -1,    29,    30,    31,    56,
      33,    -1,    35,    -1,    -1,    -1,    39,    -1,    41,    42,
      67,    68,    -1,    -1,    71,    72,    73,    74,    51,    76,
      53,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      63,    64,    -1,    66,    -1,    -1,    93,    94,    95,    96,
      97,    -1,    75,    -1,    -1,   102,   103,    -1,    -1,    82,
      83,    84,    85,    86,    -1,    -1,    -1,    90,    -1,    92,
      -1,    94,    95,    -1,    -1,    12,    -1,   100,    15,    16,
      17,   104,    19,    20,    21,    22,    23,    -1,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    58,    35,    -1,
      -1,    -1,    39,    -1,    41,    42,    67,    68,    -1,    -1,
      71,    72,    73,    74,    51,    76,    53,    -1,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    63,    64,    -1,    66,
      -1,    -1,    93,    94,    95,    96,    97,    -1,    75,    -1,
      -1,   102,   103,    -1,    -1,    82,    83,    84,    85,    86,
      -1,    -1,    -1,    90,    -1,    92,    -1,    94,    95,    -1,
      -1,    12,    -1,   100,    15,    16,    17,   104,    19,    20,
      21,    22,    23,    -1,    25,    26,    27,    -1,    29,    30,
      31,    -1,    33,    -1,    35,    -1,    -1,    -1,    39,    -1,
      41,    42,    67,    68,    -1,    -1,    71,    72,    73,    74,
      51,    76,    53,    -1,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    63,    64,    -1,    66,    -1,    -1,    93,    94,
      95,    96,    97,    -1,    75,    -1,   101,   102,   103,    -1,
      -1,    82,    83,    84,    85,    86,    -1,    -1,    -1,    90,
      -1,    92,    12,    94,    95,    15,    16,    17,    -1,   100,
      -1,    -1,    -1,   104,    -1,    25,    26,    27,    -1,    29,
      30,    31,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    45,    -1,    47,    48,    49,
      50,    51,    12,    53,    -1,    15,    16,    17,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    26,    27,    -1,    29,
      30,    31,    -1,    33,    -1,    75,    -1,    -1,    -1,    -1,
      40,    -1,    82,    83,    84,    85,    86,    47,    -1,    49,
      -1,    51,    92,    53,    94,    95,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,   104,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    82,    83,    84,    85,    86,    -1,    -1,    -1,
      -1,    -1,    92,    12,    94,    95,    15,    16,    17,    76,
     100,    -1,    79,    -1,   104,    -1,    25,    26,    27,    -1,
      29,    30,    31,    -1,    33,    -1,    93,    94,    95,    96,
      97,    -1,    -1,    -1,    -1,   102,   103,    -1,    -1,    -1,
      -1,    -1,    51,    12,    53,    -1,    15,    16,    17,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    27,    -1,
      29,    30,    31,    -1,    33,    -1,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    -1,    51,    92,    53,    94,    95,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    -1,    -1,
      -1,    -1,    -1,    92,    12,    94,    95,    15,    16,    17,
      76,   100,    -1,    79,    -1,   104,    -1,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    -1,    93,    94,    95,
      96,    97,    -1,    -1,    -1,    -1,   102,   103,    -1,    -1,
      -1,    -1,    -1,    51,    12,    53,    -1,    15,    16,    17,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    -1,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    -1,    51,    92,    53,    94,    95,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    82,    83,    84,    85,    86,    -1,
      -1,    -1,    -1,    -1,    92,    12,    94,    95,    15,    16,
      17,    49,   100,    -1,    -1,    -1,   104,    -1,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    -1,    -1,    67,
      68,    -1,    -1,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    79,    80,    81,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      -1,    -1,    -1,    76,   102,   103,    79,    -1,    75,    -1,
      -1,    44,    -1,    -1,    -1,    82,    83,    84,    85,    86,
      93,    94,    95,    96,    97,    92,    -1,    94,    95,   102,
     103,    -1,    -1,   100,    67,    68,    -1,   104,    71,    72,
      73,    74,    -1,    76,    77,    78,    79,    80,    81,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    -1,    -1,    -1,    -1,   102,
     103,    -1,    67,    68,    -1,    -1,    71,    72,    73,    74,
      -1,    76,    77,    78,    79,    80,    81,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    -1,    -1,    67,    68,   102,   103,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    79,    80,    81,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    -1,    -1,    67,    68,
     102,   103,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      79,    80,    81,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,
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
      -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    -1,    -1,    67,    68,   102,   103,
      71,    72,    73,    74,    -1,    76,    -1,    -1,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,    67,
      68,   102,   103,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      -1,    99,    67,    68,   102,   103,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    87,    88,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    -1,    -1,    67,    68,   102,   103,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    -1,    -1,    -1,   101,
     102,   103,    67,    68,    -1,    -1,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    -1,    -1,    67,    68,   102,   103,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    -1,    -1,    67,    68,
     102,   103,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,
      -1,    67,    68,   102,   103,    71,    72,    73,    74,    -1,
      76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    -1,    -1,    67,    68,   102,   103,    71,    72,
      73,    74,    -1,    76,    -1,    -1,    79,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    -1,    -1,    67,    68,   102,
     103,    71,    72,    73,    74,    -1,    76,    -1,    -1,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,
      67,    68,   102,   103,    71,    72,    73,    74,    -1,    76,
      -1,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    -1,    -1,    67,    68,   102,   103,    71,    72,    73,
      74,    -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    -1,    -1,    67,    68,   102,   103,
      71,    72,    73,    74,    -1,    76,    -1,    -1,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,    67,
      68,   102,   103,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      -1,    -1,    67,    68,   102,   103,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    -1,    -1,    67,    68,   102,   103,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    -1,    -1,    67,    68,
     102,   103,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,
      -1,    67,    68,   102,   103,    71,    72,    73,    74,    -1,
      76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    -1,    -1,    67,    68,   102,   103,    71,    72,
      73,    74,    -1,    76,    -1,    -1,    79,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    -1,    -1,    67,    68,   102,
     103,    71,    72,    73,    74,    -1,    76,    -1,    -1,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,
      67,    68,   102,   103,    71,    72,    73,    74,    -1,    76,
      -1,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    -1,    -1,    67,    68,   102,   103,    71,    72,    73,
      74,    -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    -1,    -1,    67,    68,   102,   103,
      71,    72,    73,    74,    -1,    76,    -1,    -1,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,    67,
      68,   102,   103,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      -1,    -1,    67,    68,   102,   103,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    -1,    -1,    67,    68,   102,   103,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    -1,    -1,    67,    68,
     102,   103,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,
      -1,    67,    68,   102,   103,    71,    72,    73,    74,    -1,
      76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    -1,    -1,    67,    68,   102,   103,    71,    72,
      73,    74,    -1,    76,    -1,    -1,    79,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    -1,    -1,    67,    68,   102,
     103,    71,    72,    73,    74,    -1,    76,    -1,    -1,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,
      67,    68,   102,   103,    71,    72,    73,    74,    -1,    76,
      -1,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    -1,    -1,    67,    68,   102,   103,    71,    72,    73,
      74,    -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    -1,    -1,    67,    68,   102,   103,
      71,    72,    73,    74,    -1,    76,    -1,    -1,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,    67,
      68,   102,   103,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      -1,    -1,    67,    68,   102,   103,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    -1,    -1,    67,    68,   102,   103,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    79,    80,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    -1,    -1,    67,    68,
     102,   103,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,
      -1,    67,    68,   102,   103,    71,    72,    73,    74,    -1,
      76,    -1,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    -1,    -1,    67,    68,   102,   103,    71,    72,
      73,    74,    -1,    76,    -1,    -1,    79,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    -1,    -1,    67,    68,   102,
     103,    71,    72,    73,    74,    -1,    76,    -1,    -1,    79,
      80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,
      67,    68,   102,   103,    71,    72,    73,    74,    -1,    76,
      -1,    -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    -1,    -1,    67,    68,   102,   103,    71,    72,    73,
      74,    -1,    76,    -1,    -1,    79,    80,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    -1,    -1,    67,    68,   102,   103,
      71,    72,    -1,    -1,    -1,    76,    -1,    -1,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    -1,    -1,    67,
      68,   102,   103,    71,    72,    -1,    -1,    -1,    76,    -1,
      -1,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,    76,
      -1,    -1,    79,    -1,    -1,    93,    94,    95,    96,    97,
      76,    -1,    -1,    79,   102,   103,    93,    94,    95,    96,
      97,    76,    -1,    -1,    79,   102,   103,    93,    94,    95,
      96,    97,    76,    -1,    -1,    79,   102,   103,    93,    94,
      95,    96,    97,    -1,    -1,    -1,    -1,   102,   103,    93,
      94,    95,    96,    97,    -1,    -1,    -1,    -1,   102,   103
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     1,    12,    15,    16,    17,    19,    20,    21,    22,
      23,    25,    26,    27,    29,    30,    31,    33,    35,    39,
      41,    42,    43,    46,    51,    52,    53,    57,    61,    62,
      63,    64,    66,    75,    82,    83,    84,    85,    86,    89,
      90,    91,    92,    94,    95,   100,   104,   108,   109,   110,
     111,   112,   113,   117,   118,   119,   124,   125,   126,   127,
     128,   129,   133,   136,   138,   141,   145,    99,    30,    51,
      92,   127,   127,   127,   127,   147,    30,   127,   147,   127,
     100,   147,   137,    52,    30,   130,    40,    45,    47,    48,
      49,    50,   127,   134,   135,    30,   127,    58,    59,    15,
      16,    30,    30,    30,    30,   127,    65,   134,   135,    30,
     132,   127,   127,   127,   127,   127,   127,    30,   131,    30,
     100,   127,   127,   127,   127,   146,     0,    99,   127,    71,
     127,   111,   127,    67,    68,    71,    72,    73,    74,    76,
      79,    80,    81,    93,    94,    95,    96,    97,   102,   103,
     121,   100,   127,   106,   146,   127,   146,   146,   106,   127,
     127,    47,   135,   139,    44,    77,    78,    30,    56,    71,
     126,   127,    56,    71,    54,   140,    58,   135,   106,    49,
      56,    49,    56,    49,    56,    49,    56,    87,    88,   106,
      30,   101,   105,   106,   109,   123,   127,    54,    55,   122,
      56,    99,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,    30,
     142,   127,   101,    30,   127,   121,    30,   128,   127,   127,
     127,   127,    71,    56,   127,   127,   121,   127,    30,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,    30,
     101,   106,   127,    99,   112,   122,   122,   121,   123,   127,
     123,    99,   106,   142,   127,   127,   142,    49,    49,    49,
      49,   146,    34,    36,    56,   111,   122,   122,   106,   144,
      30,    99,    99,   127,   127,   127,   127,   101,    56,   127,
     122,    34,    38,   114,   115,   120,    34,    30,   123,   144,
     144,   127,   122,   114,   122,   114,   127,    34,   123,   123,
     122,   123,   122,    34,    37,    60,   116,   120,    54,    55,
      30,   143,   123,   122,   116,   121,   121,   122,   127,   121,
     121,   106,   122,    34,   122,   111,   123,   111,   123,    54,
     111,   123,    30,    34,   122,   122,   122,   122,   121,   122,
     122,    34,    34,   111,   122,    37,   111,   122
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   107,   108,   108,   108,   109,   109,   109,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   111,   111,
     111,   112,   112,   112,   112,   112,   112,   112,   112,   113,
     113,   113,   113,   114,   114,   115,   115,   116,   116,   116,
     116,   116,   116,   117,   118,   119,   120,   121,   122,   123,
     123,   123,   124,   125,   126,   126,   126,   126,   126,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   128,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     130,   130,   131,   131,   132,   132,   133,   133,   133,   133,
     133,   133,   134,   134,   135,   135,   136,   136,   136,   136,
     137,   136,   139,   138,   138,   140,   138,   138,   138,   141,
     142,   142,   142,   142,   143,   143,   143,   144,   145,   146,
     146,   146,   147,   147
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     3,     1,     2,     0,     1,     1,     4,     4,
       4,     4,     4,     4,     4,     4,     5,     5,     1,     1,
       1,     1,     1,     5,    10,    11,     3,     6,     4,    10,
       6,     9,     6,     0,     2,     6,     6,     1,     9,     5,
       4,     5,     4,     2,     3,     1,     1,     0,     0,     0,
       2,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     4,     1,     2,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     3,     3,     3,     2,     2,     3,     3,
       4,     4,     4,     6,     4,     6,     4,     6,     4,     6,
       1,     2,     2,     1,     1,     2,     1,     2,     2,     2,
       1,     2,     2,     1,     2,     4,     6,     4,     2,     3,
       1,     3,     1,     3,     1,     3,     2,     2,     2,     2,
       3,     2,     2,     1,     3,     2,     2,     2,     3,     2,
       0,     3,     0,     8,     2,     0,     8,     8,     6,     2,
       0,     1,     3,     4,     0,     1,     3,     0,     2,     0,
       1,     3,     1,     3
};


/* YYDPREC[RULE-NUM] -- Dynamic precedence of rule #RULE-NUM (0 if none).  */
static const unsigned char yydprec[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0
};

/* YYMERGER[RULE-NUM] -- Index of merging function for rule #RULE-NUM.  */
static const unsigned char yymerger[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0
};

/* YYIMMEDIATE[RULE-NUM] -- True iff rule #RULE-NUM is not to be deferred, as
   in the case of predicates.  */
static const yybool yyimmediate[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0
};

/* YYCONFLP[YYPACT[STATE-NUM]] -- Pointer into YYCONFL of start of
   list of conflicting reductions corresponding to action entry for
   state STATE-NUM in yytable.  0 means no conflicts.  The list in
   yyconfl is terminated by a rule number of 0.  */
static const unsigned short yyconflp[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1171,     0,
       0,  1173,  1175,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   361,     0,   367,     0,
       0,     0,     0,     0,  1178,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   363,
     365,   369,   371,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,     0,   373,
       0,     0,     0,     0,     0,  1027,     0,     0,     0,  1029,
       0,     0,     0,     0,   435,     0,     0,     0,     0,     0,
       0,     0,   375,   377,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   437,   439,     0,
       0,     0,     0,     0,     0,     0,     0,   505,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     507,   509,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,     0,     0,
       0,   105,     0,     0,   511,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   265,     0,     0,     0,     0,   513,   515,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   397,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   399,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1180,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     401,   403,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   405,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   407,   409,     0,     0,     0,   487,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   489,   491,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   493,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   495,   497,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   499,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     501,   503,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1182,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1188,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1190,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1192,     0,     0,  1194,  1196,
    1198,     0,  1200,  1202,  1204,  1206,  1208,     0,  1210,  1212,
    1214,     0,  1216,  1218,  1220,     0,  1222,     0,  1224,     0,
       0,     0,  1226,     0,  1228,  1230,     0,     0,     0,     0,
       0,     0,     0,     0,  1232,     0,  1234,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1236,  1238,     0,  1240,
       0,     0,     0,   357,   359,     0,     0,     0,  1242,     0,
       0,     0,     0,     0,     0,  1244,  1246,  1248,  1250,  1252,
       0,     0,     0,  1254,     0,  1256,     0,  1258,  1260,     0,
       0,  1266,     0,  1262,  1268,  1270,  1272,  1264,  1274,  1276,
    1278,  1280,  1282,     0,  1284,  1286,  1288,     0,  1290,  1292,
    1294,     0,  1296,     0,  1298,     0,     0,     0,  1300,     0,
    1302,  1304,     0,     0,     0,     0,     0,     0,     0,     0,
    1306,     0,  1308,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1310,  1312,     0,  1314,     0,     0,     0,     0,
       0,     0,     0,     0,  1316,     0,     0,     0,     0,     0,
       0,  1318,  1320,  1322,  1324,  1326,     0,     0,     0,  1328,
       0,  1330,     0,  1332,  1334,     0,     0,     0,     0,  1336,
       0,     0,     0,  1338,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   411,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   413,   415,     0,     0,     0,
       0,     0,     0,     1,     0,     0,     3,     5,     7,     0,
       0,     0,     0,     0,     0,     0,     9,    11,    13,     0,
      15,    17,    19,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,     0,    25,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     0,     0,    29,    31,    33,    35,    37,     0,     0,
       0,     0,     0,    39,    49,    41,    43,    51,    53,    55,
       0,    45,     0,   417,     0,    47,     0,    57,    59,    61,
       0,    63,    65,    67,     0,    69,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   419,   421,     0,     0,
       0,     0,     0,    71,   183,    73,     0,   185,   187,   189,
       0,     0,     0,     0,     0,     0,     0,   191,   193,   195,
       0,   197,   199,   201,     0,   203,     0,    75,     0,     0,
       0,     0,     0,     0,    77,    79,    81,    83,    85,     0,
       0,     0,     0,   205,    87,   207,    89,    91,     0,     0,
       0,     0,    93,     0,     0,     0,    95,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,     0,     0,
       0,     0,     0,     0,   211,   213,   215,   217,   219,     0,
       0,     0,     0,     0,   221,   301,   223,   225,   303,   305,
     307,     0,   227,     0,     0,     0,   229,     0,   309,   311,
     313,     0,   315,   317,   319,     0,   321,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   323,     0,   325,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   475,     0,   327,     0,
       0,     0,     0,     0,     0,   329,   331,   333,   335,   337,
       0,     0,     0,     0,     0,   339,     0,   341,   343,   477,
     479,     0,     0,   345,     0,     0,     0,   347,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   353,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   387,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   101,
     103,     0,     0,     0,     0,   107,   109,     0,     0,   111,
     113,   115,   117,     0,   119,     0,     0,   121,   123,   125,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   127,   129,   131,   133,   135,     0,     0,     0,     0,
     137,   139,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   141,   143,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   145,   147,
       0,     0,     0,     0,   149,   151,     0,     0,   153,   155,
     157,   159,     0,   161,     0,     0,   163,   165,   167,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     169,   171,   173,   175,   177,     0,     0,   231,   233,   179,
     181,   235,   237,   239,   241,     0,   243,     0,     0,   245,
     247,   249,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   251,   253,   255,   257,   259,     0,     0,
     267,   269,   261,   263,   271,   273,   275,   277,     0,   279,
       0,     0,   281,   283,   285,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   287,   289,   291,   293,
     295,     0,     0,     0,     0,   297,   299,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     349,   351,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   379,   381,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   383,   385,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   389,
     391,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   393,   395,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   441,   443,     0,     0,   445,   447,   449,   451,     0,
     453,     0,     0,   455,   457,   459,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   461,   463,   465,
     467,   469,     0,     0,   517,   519,   471,   473,   521,   523,
     525,   527,     0,   529,     0,     0,   531,   533,   535,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     537,   539,   541,   543,   545,     0,     0,   551,   553,   547,
     549,   555,   557,   559,   561,     0,   563,     0,     0,   565,
     567,   569,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   571,   573,   575,   577,   579,     0,     0,
     585,   587,   581,   583,   589,   591,   593,   595,     0,   597,
       0,     0,   599,   601,   603,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   605,   607,   609,   611,
     613,     0,     0,     0,     0,   615,   617,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     619,   621,     0,     0,     0,     0,   623,   625,     0,     0,
     627,   629,   631,   633,     0,   635,     0,     0,   637,   639,
     641,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   643,   645,   647,   649,   651,     0,     0,   657,
     659,   653,   655,   661,   663,   665,   667,     0,   669,     0,
       0,   671,   673,   675,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   677,   679,   681,   683,   685,
       0,     0,   691,   693,   687,   689,   695,   697,   699,   701,
       0,   703,     0,     0,   705,   707,   709,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   711,   713,
     715,   717,   719,     0,     0,   731,   733,   721,   723,   735,
     737,   739,   741,     0,   743,     0,     0,   745,   747,   749,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   751,   753,   755,   757,   759,     0,     0,     0,     0,
     761,   763,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   771,   773,     0,     0,     0,
       0,   775,   777,     0,     0,   779,   781,   783,   785,     0,
     787,     0,     0,   789,   791,   793,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   795,   797,   799,
     801,   803,     0,     0,   809,   811,   805,   807,   813,   815,
     817,   819,     0,   821,     0,     0,   823,   825,   827,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     829,   831,   833,   835,   837,     0,     0,   843,   845,   839,
     841,   847,   849,   851,   853,     0,   855,     0,     0,   857,
     859,   861,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   863,   865,   867,   869,   871,     0,     0,
     877,   879,   873,   875,   881,   883,   885,   887,     0,   889,
       0,     0,   891,   893,   895,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   897,   899,   901,   903,
     905,     0,     0,   911,   913,   907,   909,   915,   917,   919,
     921,     0,   923,     0,     0,   925,   927,   929,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   931,
     933,   935,   937,   939,     0,     0,   945,   947,   941,   943,
     949,   951,   953,   955,     0,   957,     0,     0,   959,   961,
     963,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   965,   967,   969,   971,   973,     0,     0,     0,
       0,   975,   977,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   979,   981,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   983,
     985,     0,     0,     0,     0,   993,   995,     0,     0,   997,
     999,  1001,  1003,     0,  1005,     0,     0,  1007,  1009,  1011,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1013,  1015,  1017,  1019,  1021,     0,     0,  1031,  1033,
    1023,  1025,  1035,  1037,  1039,  1041,     0,  1043,     0,     0,
    1045,  1047,  1049,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1051,  1053,  1055,  1057,  1059,     0,
       0,  1065,  1067,  1061,  1063,  1069,  1071,  1073,  1075,     0,
    1077,     0,     0,  1079,  1081,  1083,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1085,  1087,  1089,
    1091,  1093,     0,     0,  1099,  1101,  1095,  1097,  1103,  1105,
    1107,  1109,     0,  1111,     0,     0,  1113,  1115,  1117,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1119,  1121,  1123,  1125,  1127,     0,     0,  1133,  1135,  1129,
    1131,  1137,  1139,  1141,  1143,     0,  1145,     0,     0,  1147,
    1149,  1151,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1153,  1155,  1157,  1159,  1161,     0,     0,
       0,     0,  1163,  1165,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1167,  1169,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1184,  1186,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   423,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   425,   427,     0,     0,     0,     0,     0,     0,     0,
       0,   429,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   481,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   725,   431,   433,     0,     0,     0,     0,
       0,     0,     0,     0,   765,   483,   485,     0,     0,     0,
       0,     0,     0,     0,     0,   987,   727,   729,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   767,   769,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   989,   991
};

/* YYCONFL[I] -- lists of conflicting rule numbers, each terminated by
   0, pointed into by YYCONFLP.  */
static const short yyconfl[] =
{
       0,   113,     0,   113,     0,   113,     0,   113,     0,   113,
       0,   113,     0,   113,     0,   113,     0,   113,     0,   113,
       0,   113,     0,   113,     0,   113,     0,   113,     0,   113,
       0,   113,     0,   113,     0,   113,     0,   113,     0,   113,
       0,   113,     0,   113,     0,   113,     0,   113,     0,    58,
       0,    58,     0,    58,     0,    58,     0,    58,     0,    58,
       0,    58,     0,    58,     0,    58,     0,    58,     0,    58,
       0,    58,     0,    58,     0,    58,     0,    58,     0,    58,
       0,    58,     0,    58,     0,    58,     0,    58,     0,    58,
       0,    58,     0,    58,     0,    58,     0,   106,     0,   100,
       0,    19,     0,    19,     0,    58,     0,    66,     0,    66,
       0,    66,     0,    66,     0,    66,     0,    66,     0,    66,
       0,    66,     0,    66,     0,    66,     0,    66,     0,    66,
       0,    66,     0,    66,     0,    66,     0,    66,     0,    66,
       0,   112,     0,   112,     0,   111,     0,   111,     0,   162,
       0,   162,     0,   162,     0,   162,     0,   162,     0,   162,
       0,   162,     0,   162,     0,   162,     0,   162,     0,   162,
       0,   162,     0,   162,     0,   162,     0,   162,     0,   162,
       0,   162,     0,   159,     0,   159,     0,   159,     0,   159,
       0,   159,     0,   159,     0,   159,     0,   159,     0,   159,
       0,   159,     0,   159,     0,   159,     0,   159,     0,   159,
       0,   159,     0,   159,     0,   159,     0,   159,     0,   159,
       0,   159,     0,   159,     0,   159,     0,   159,     0,   159,
       0,    62,     0,    62,     0,    62,     0,    62,     0,    62,
       0,    62,     0,    62,     0,    62,     0,    62,     0,    62,
       0,    62,     0,    62,     0,    62,     0,    62,     0,    62,
       0,    62,     0,    62,     0,    63,     0,   101,     0,   101,
       0,   101,     0,   101,     0,   101,     0,   101,     0,   101,
       0,   101,     0,   101,     0,   101,     0,   101,     0,   101,
       0,   101,     0,   101,     0,   101,     0,   101,     0,   101,
       0,   159,     0,   159,     0,   159,     0,   159,     0,   159,
       0,   159,     0,   159,     0,   159,     0,   159,     0,   159,
       0,   159,     0,   159,     0,   159,     0,   159,     0,   159,
       0,   159,     0,   159,     0,   159,     0,   159,     0,   159,
       0,   159,     0,   159,     0,   159,     0,   159,     0,   133,
       0,   133,     0,   102,     0,   102,     0,   118,     0,   118,
       0,    81,     0,    81,     0,    81,     0,    86,     0,    86,
       0,    86,     0,    87,     0,    87,     0,    87,     0,   160,
       0,   160,     0,    49,     0,    49,     0,   160,     0,   132,
       0,   132,     0,   135,     0,   135,     0,    59,     0,    77,
       0,    77,     0,    77,     0,    78,     0,    78,     0,    78,
       0,    75,     0,    75,     0,    75,     0,    76,     0,    76,
       0,    76,     0,    79,     0,    79,     0,    79,     0,    80,
       0,    80,     0,    80,     0,    72,     0,    72,     0,    72,
       0,    83,     0,    83,     0,    83,     0,    83,     0,    83,
       0,    83,     0,    83,     0,    83,     0,    83,     0,    83,
       0,    83,     0,    83,     0,    83,     0,    83,     0,    83,
       0,    83,     0,    83,     0,    84,     0,    84,     0,    84,
       0,    85,     0,    85,     0,    85,     0,    82,     0,    82,
       0,    82,     0,    68,     0,    68,     0,    68,     0,    69,
       0,    69,     0,    69,     0,    70,     0,    70,     0,    70,
       0,    71,     0,    71,     0,    71,     0,    73,     0,    73,
       0,    73,     0,    73,     0,    73,     0,    73,     0,    73,
       0,    73,     0,    73,     0,    73,     0,    73,     0,    73,
       0,    73,     0,    73,     0,    73,     0,    73,     0,    73,
       0,    74,     0,    74,     0,    74,     0,    74,     0,    74,
       0,    74,     0,    74,     0,    74,     0,    74,     0,    74,
       0,    74,     0,    74,     0,    74,     0,    74,     0,    74,
       0,    74,     0,    74,     0,   163,     0,   163,     0,   163,
       0,   163,     0,   163,     0,   163,     0,   163,     0,   163,
       0,   163,     0,   163,     0,   163,     0,   163,     0,   163,
       0,   163,     0,   163,     0,   163,     0,   163,     0,   134,
       0,   134,     0,    10,     0,    10,     0,    10,     0,    10,
       0,    10,     0,    10,     0,    10,     0,    10,     0,    10,
       0,    10,     0,    10,     0,    10,     0,    10,     0,    10,
       0,    10,     0,    10,     0,    10,     0,    11,     0,    11,
       0,    11,     0,    11,     0,    11,     0,    11,     0,    11,
       0,    11,     0,    11,     0,    11,     0,    11,     0,    11,
       0,    11,     0,    11,     0,    11,     0,    11,     0,    11,
       0,    15,     0,    15,     0,    15,     0,    15,     0,    15,
       0,    15,     0,    15,     0,    15,     0,    15,     0,    15,
       0,    15,     0,    15,     0,    15,     0,    15,     0,    15,
       0,    15,     0,    15,     0,    13,     0,    13,     0,    13,
       0,    14,     0,    14,     0,    14,     0,    14,     0,    14,
       0,    14,     0,    14,     0,    14,     0,    14,     0,    14,
       0,    14,     0,    14,     0,    14,     0,    14,     0,    14,
       0,    14,     0,    14,     0,    12,     0,    12,     0,    12,
       0,   117,     0,   117,     0,    92,     0,    92,     0,    92,
       0,    92,     0,    92,     0,    92,     0,    92,     0,    92,
       0,    92,     0,    92,     0,    92,     0,    92,     0,    92,
       0,    92,     0,    92,     0,    92,     0,    92,     0,    94,
       0,    94,     0,    94,     0,    94,     0,    94,     0,    94,
       0,    94,     0,    94,     0,    94,     0,    94,     0,    94,
       0,    94,     0,    94,     0,    94,     0,    94,     0,    94,
       0,    94,     0,    96,     0,    96,     0,    96,     0,    96,
       0,    96,     0,    96,     0,    96,     0,    96,     0,    96,
       0,    96,     0,    96,     0,    96,     0,    96,     0,    96,
       0,    96,     0,    96,     0,    96,     0,    98,     0,    98,
       0,    98,     0,    98,     0,    98,     0,    98,     0,    98,
       0,    98,     0,    98,     0,    98,     0,    98,     0,    98,
       0,    98,     0,    98,     0,    98,     0,    98,     0,    98,
       0,    90,     0,    90,     0,    90,     0,    90,     0,    90,
       0,    90,     0,    90,     0,    90,     0,    90,     0,    90,
       0,    90,     0,    90,     0,    90,     0,    90,     0,    90,
       0,    90,     0,    90,     0,    91,     0,    91,     0,    91,
       0,    91,     0,    91,     0,    91,     0,    91,     0,    91,
       0,    91,     0,    91,     0,    91,     0,    91,     0,    91,
       0,    91,     0,    91,     0,    91,     0,    91,     0,   161,
       0,   161,     0,    28,     0,    28,     0,    17,     0,    17,
       0,    17,     0,    16,     0,    16,     0,    16,     0,    16,
       0,    16,     0,    16,     0,    16,     0,    16,     0,    16,
       0,    16,     0,    16,     0,    16,     0,    16,     0,    16,
       0,    16,     0,    16,     0,    16,     0,    33,     0,    33,
       0,    93,     0,    93,     0,    93,     0,    93,     0,    93,
       0,    93,     0,    93,     0,    93,     0,    93,     0,    93,
       0,    93,     0,    93,     0,    93,     0,    93,     0,    93,
       0,    93,     0,    93,     0,    95,     0,    95,     0,    95,
       0,    95,     0,    95,     0,    95,     0,    95,     0,    95,
       0,    95,     0,    95,     0,    95,     0,    95,     0,    95,
       0,    95,     0,    95,     0,    95,     0,    95,     0,    97,
       0,    97,     0,    97,     0,    97,     0,    97,     0,    97,
       0,    97,     0,    97,     0,    97,     0,    97,     0,    97,
       0,    97,     0,    97,     0,    97,     0,    97,     0,    97,
       0,    97,     0,    99,     0,    99,     0,    99,     0,    99,
       0,    99,     0,    99,     0,    99,     0,    99,     0,    99,
       0,    99,     0,    99,     0,    99,     0,    99,     0,    99,
       0,    99,     0,    99,     0,    99,     0,    48,     0,    48,
       0,    33,     0,    33,     0,    30,    33,     0,    33,     0,
      33,     0,   148,     0,    48,     0,    48,     0,   143,     0,
     146,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0,
      49,     0,    49,     0,    49,     0,    49,     0,    49,     0
};

/* Error token number */
#define YYTERROR 1



YYSTYPE yylval;

int yynerrs;
int yychar;

static const int YYEOF = 0;
static const int YYEMPTY = -2;

typedef enum { yyok, yyaccept, yyabort, yyerr } YYRESULTTAG;

#define YYCHK(YYE)                              \
  do {                                          \
    YYRESULTTAG yychk_flag = YYE;               \
    if (yychk_flag != yyok)                     \
      return yychk_flag;                        \
  } while (0)

#if YYDEBUG

# ifndef YYFPRINTF
#  define YYFPRINTF fprintf
# endif

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YYDPRINTF(Args)                        \
  do {                                          \
    if (yydebug)                                \
      YYFPRINTF Args;                           \
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

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                  \
  do {                                                                  \
    if (yydebug)                                                        \
      {                                                                 \
        YYFPRINTF (stderr, "%s ", Title);                               \
        yy_symbol_print (stderr, Type, Value);        \
        YYFPRINTF (stderr, "\n");                                       \
      }                                                                 \
  } while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;

struct yyGLRStack;
static void yypstack (struct yyGLRStack* yystackp, size_t yyk)
  YY_ATTRIBUTE_UNUSED;
static void yypdumpstack (struct yyGLRStack* yystackp)
  YY_ATTRIBUTE_UNUSED;

#else /* !YYDEBUG */

# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)

#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYMAXDEPTH * sizeof (GLRStackItem)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

/* Minimum number of free items on the stack allowed after an
   allocation.  This is to allow allocation and initialization
   to be completed by functions that call yyexpandGLRStack before the
   stack is expanded, thus insuring that all necessary pointers get
   properly redirected to new data.  */
#define YYHEADROOM 2

#ifndef YYSTACKEXPANDABLE
#  define YYSTACKEXPANDABLE 1
#endif

#if YYSTACKEXPANDABLE
# define YY_RESERVE_GLRSTACK(Yystack)                   \
  do {                                                  \
    if (Yystack->yyspaceLeft < YYHEADROOM)              \
      yyexpandGLRStack (Yystack);                       \
  } while (0)
#else
# define YY_RESERVE_GLRSTACK(Yystack)                   \
  do {                                                  \
    if (Yystack->yyspaceLeft < YYHEADROOM)              \
      yyMemoryExhausted (Yystack);                      \
  } while (0)
#endif


#if YYERROR_VERBOSE

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
static size_t
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      size_t yyn = 0;
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
    return strlen (yystr);

  return (size_t) (yystpcpy (yyres, yystr) - yyres);
}
# endif

#endif /* !YYERROR_VERBOSE */

/** State numbers, as in LALR(1) machine */
typedef int yyStateNum;

/** Rule numbers, as in LALR(1) machine */
typedef int yyRuleNum;

/** Grammar symbol */
typedef int yySymbol;

/** Item references, as in LALR(1) machine */
typedef short yyItemNum;

typedef struct yyGLRState yyGLRState;
typedef struct yyGLRStateSet yyGLRStateSet;
typedef struct yySemanticOption yySemanticOption;
typedef union yyGLRStackItem yyGLRStackItem;
typedef struct yyGLRStack yyGLRStack;

struct yyGLRState {
  /** Type tag: always true.  */
  yybool yyisState;
  /** Type tag for yysemantics.  If true, yysval applies, otherwise
   *  yyfirstVal applies.  */
  yybool yyresolved;
  /** Number of corresponding LALR(1) machine state.  */
  yyStateNum yylrState;
  /** Preceding state in this stack */
  yyGLRState* yypred;
  /** Source position of the last token produced by my symbol */
  size_t yyposn;
  union {
    /** First in a chain of alternative reductions producing the
     *  nonterminal corresponding to this state, threaded through
     *  yynext.  */
    yySemanticOption* yyfirstVal;
    /** Semantic value for this state.  */
    YYSTYPE yysval;
  } yysemantics;
};

struct yyGLRStateSet {
  yyGLRState** yystates;
  /** During nondeterministic operation, yylookaheadNeeds tracks which
   *  stacks have actually needed the current lookahead.  During deterministic
   *  operation, yylookaheadNeeds[0] is not maintained since it would merely
   *  duplicate yychar != YYEMPTY.  */
  yybool* yylookaheadNeeds;
  size_t yysize, yycapacity;
};

struct yySemanticOption {
  /** Type tag: always false.  */
  yybool yyisState;
  /** Rule number for this reduction */
  yyRuleNum yyrule;
  /** The last RHS state in the list of states to be reduced.  */
  yyGLRState* yystate;
  /** The lookahead for this reduction.  */
  int yyrawchar;
  YYSTYPE yyval;
  /** Next sibling in chain of options.  To facilitate merging,
   *  options are chained in decreasing order by address.  */
  yySemanticOption* yynext;
};

/** Type of the items in the GLR stack.  The yyisState field
 *  indicates which item of the union is valid.  */
union yyGLRStackItem {
  yyGLRState yystate;
  yySemanticOption yyoption;
};

struct yyGLRStack {
  int yyerrState;


  YYJMP_BUF yyexception_buffer;
  yyGLRStackItem* yyitems;
  yyGLRStackItem* yynextFree;
  size_t yyspaceLeft;
  yyGLRState* yysplitPoint;
  yyGLRState* yylastDeleted;
  yyGLRStateSet yytops;
};

#if YYSTACKEXPANDABLE
static void yyexpandGLRStack (yyGLRStack* yystackp);
#endif

_Noreturn static void
yyFail (yyGLRStack* yystackp, const char* yymsg)
{
  if (yymsg != YY_NULLPTR)
    yyerror (yymsg);
  YYLONGJMP (yystackp->yyexception_buffer, 1);
}

_Noreturn static void
yyMemoryExhausted (yyGLRStack* yystackp)
{
  YYLONGJMP (yystackp->yyexception_buffer, 2);
}

#if YYDEBUG || YYERROR_VERBOSE
/** A printable representation of TOKEN.  */
static inline const char*
yytokenName (yySymbol yytoken)
{
  if (yytoken == YYEMPTY)
    return "";

  return yytname[yytoken];
}
#endif

/** Fill in YYVSP[YYLOW1 .. YYLOW0-1] from the chain of states starting
 *  at YYVSP[YYLOW0].yystate.yypred.  Leaves YYVSP[YYLOW1].yystate.yypred
 *  containing the pointer to the next state in the chain.  */
static void yyfillin (yyGLRStackItem *, int, int) YY_ATTRIBUTE_UNUSED;
static void
yyfillin (yyGLRStackItem *yyvsp, int yylow0, int yylow1)
{
  int i;
  yyGLRState *s = yyvsp[yylow0].yystate.yypred;
  for (i = yylow0-1; i >= yylow1; i -= 1)
    {
#if YYDEBUG
      yyvsp[i].yystate.yylrState = s->yylrState;
#endif
      yyvsp[i].yystate.yyresolved = s->yyresolved;
      if (s->yyresolved)
        yyvsp[i].yystate.yysemantics.yysval = s->yysemantics.yysval;
      else
        /* The effect of using yysval or yyloc (in an immediate rule) is
         * undefined.  */
        yyvsp[i].yystate.yysemantics.yyfirstVal = YY_NULLPTR;
      s = yyvsp[i].yystate.yypred = s->yypred;
    }
}


/** If yychar is empty, fetch the next token.  */
static inline yySymbol
yygetToken (int *yycharp)
{
  yySymbol yytoken;
  if (*yycharp == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      *yycharp = yylex ();
    }
  if (*yycharp <= YYEOF)
    {
      *yycharp = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (*yycharp);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }
  return yytoken;
}

/* Do nothing if YYNORMAL or if *YYLOW <= YYLOW1.  Otherwise, fill in
 * YYVSP[YYLOW1 .. *YYLOW-1] as in yyfillin and set *YYLOW = YYLOW1.
 * For convenience, always return YYLOW1.  */
static inline int yyfill (yyGLRStackItem *, int *, int, yybool)
     YY_ATTRIBUTE_UNUSED;
static inline int
yyfill (yyGLRStackItem *yyvsp, int *yylow, int yylow1, yybool yynormal)
{
  if (!yynormal && yylow1 < *yylow)
    {
      yyfillin (yyvsp, *yylow, yylow1);
      *yylow = yylow1;
    }
  return yylow1;
}

/** Perform user action for rule number YYN, with RHS length YYRHSLEN,
 *  and top stack item YYVSP.  YYLVALP points to place to put semantic
 *  value ($$), and yylocp points to place for location information
 *  (@$).  Returns yyok for normal return, yyaccept for YYACCEPT,
 *  yyerr for YYERROR, yyabort for YYABORT.  */
static YYRESULTTAG
yyuserAction (yyRuleNum yyn, int yyrhslen, yyGLRStackItem* yyvsp,
              yyGLRStack* yystackp,
              YYSTYPE* yyvalp)
{
  yybool yynormal YY_ATTRIBUTE_UNUSED = (yybool) (yystackp->yysplitPoint == YY_NULLPTR);
  int yylow;
  YYUSE (yyvalp);
  YYUSE (yyrhslen);
# undef yyerrok
# define yyerrok (yystackp->yyerrState = 0)
# undef YYACCEPT
# define YYACCEPT return yyaccept
# undef YYABORT
# define YYABORT return yyabort
# undef YYERROR
# define YYERROR return yyerrok, yyerr
# undef YYRECOVERING
# define YYRECOVERING() (yystackp->yyerrState != 0)
# undef yyclearin
# define yyclearin (yychar = YYEMPTY)
# undef YYFILL
# define YYFILL(N) yyfill (yyvsp, &yylow, (N), yynormal)
# undef YYBACKUP
# define YYBACKUP(Token, Value)                                              \
  return yyerror (YY_("syntax error: cannot back up")),     \
         yyerrok, yyerr

  yylow = 1;
  if (yyrhslen == 0)
    *yyvalp = yyval_default;
  else
    *yyvalp = yyvsp[YYFILL (1-yyrhslen)].yystate.yysemantics.yysval;
  switch (yyn)
    {
  case 4:
#line 123 "engines/director/lingo/lingo-gr.y"
    { yyerrok; }
#line 2239 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 131 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code);
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 2250 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 137 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_assign);
			((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code); }
#line 2258 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 140 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).code) = g_lingo->code1(g_lingo->c_after); }
#line 2264 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 141 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).code) = g_lingo->code1(g_lingo->c_before); }
#line 2270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 142 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code);
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.s); }
#line 2281 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 148 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.e)[0]);
		g_lingo->codeInt((((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.e)[1]);
		((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code); }
#line 2293 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 155 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code);
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.s); }
#line 2304 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 161 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.e)[0]);
		g_lingo->codeInt((((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.e)[1]);
		((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code); }
#line 2316 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 168 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.e)[0]);
		g_lingo->codeInt((((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.e)[1]);
		((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code); }
#line 2327 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 174 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.e)[0]);
		g_lingo->codeInt((((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.e)[1]);
		((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code); }
#line 2338 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23:
#line 193 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&end, (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.code));
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.code) + 2] = end;	/* end, if cond fails */

		checkEnd((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s), "repeat", true); }
#line 2351 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24:
#line 206 "engines/director/lingo/lingo-gr.y"
    {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (((yyGLRStackItem const *)yyvsp)[YYFILL (-7)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&finish, (((yyGLRStackItem const *)yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&body, (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&end, (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&inc, 1);
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code) + 1] = init;	/* initial count value */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code) + 2] = finish;/* final count value */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code) + 3] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code) + 4] = inc;	/* increment */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code) + 5] = end;	/* end, if cond fails */

		checkEnd((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s), "repeat", true); }
#line 2370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25:
#line 224 "engines/director/lingo/lingo-gr.y"
    {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (((yyGLRStackItem const *)yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&finish, (((yyGLRStackItem const *)yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&body, (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&end, (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&inc, (uint32)-1);
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.code) + 1] = init;	/* initial count value */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.code) + 2] = finish;/* final count value */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.code) + 3] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.code) + 4] = inc;	/* increment */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.code) + 5] = end;	/* end, if cond fails */

		checkEnd((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s), "repeat", true); }
#line 2389 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26:
#line 238 "engines/director/lingo/lingo-gr.y"
    {
		inst end = 0;
		WRITE_UINT32(&end, (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code) + 1] = end; }
#line 2399 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27:
#line 243 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
			checkEnd((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s), "tell", true); }
#line 2407 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28:
#line 246 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		}
#line 2415 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 251 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (((yyGLRStackItem const *)yyvsp)[YYFILL (-6)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&else1, (((yyGLRStackItem const *)yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&end, (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code));
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf(0, (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-9)].yystate.yysemantics.yysval.code)); }
#line 2430 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 261 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code));
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, 0); }
#line 2445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 271 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&else1, (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&end, (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.code));
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf(0, 0); }
#line 2460 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 281 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code));
		WRITE_UINT32(&end, (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code));
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code) + 3] = end;	/* end, if cond fails */

		checkEnd((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s), "if", true);

		g_lingo->processIf(0, 0); }
#line 2475 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 297 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code));
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code)); }
#line 2486 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 303 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code));
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code)); }
#line 2497 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 312 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.code) - (((yyGLRStackItem const *)yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.code));
		(*g_lingo->_currentScript)[(((yyGLRStackItem const *)yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((((yyGLRStackItem const *)yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.code)); }
#line 2508 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 318 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeLabel((((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.code));

			checkEnd((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s), "if", true); }
#line 2517 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 322 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeLabel((((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code)); }
#line 2524 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 325 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeLabel((((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.code));

			checkEnd((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s), "if", true); }
#line 2533 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 329 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeLabel((((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.code)); }
#line 2540 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 333 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); }
#line 2546 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 336 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str());
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 2556 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 343 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); }
#line 2566 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 350 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); }
#line 2577 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 358 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).code) = g_lingo->_currentScript->size(); }
#line 2583 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 361 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ((*yyvalp).code) = g_lingo->_currentScript->size(); }
#line 2589 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 364 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).code) = g_lingo->_currentScript->size(); }
#line 2595 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 369 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s)->c_str());
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s); }
#line 2605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 375 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_tellcode);
		g_lingo->code1(STOP); }
#line 2613 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 379 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.i)); }
#line 2621 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 382 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_floatpush);
		g_lingo->codeFloat((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.f)); }
#line 2629 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 385 "engines/director/lingo/lingo-gr.y"
    {											// D3
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str()); }
#line 2637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 388 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str()); }
#line 2645 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 391 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str());
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 2654 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 397 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code); }
#line 2660 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 399 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s), 0);
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 2668 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 402 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s), 1);
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s); }
#line 2676 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 405 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s), (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.narg)); }
#line 2682 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 406 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.s), (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.narg));
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.s); }
#line 2690 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 409 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.e)[0]);
		WRITE_UINT32(&f, (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.e)[1]);
		g_lingo->code2(e, f); }
#line 2703 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 417 "engines/director/lingo/lingo-gr.y"
    {
		((*yyvalp).code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.e)[0]);
		WRITE_UINT32(&f, (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.e)[1]);
		g_lingo->code2(e, f); }
#line 2714 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 424 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); }
#line 2720 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 425 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); }
#line 2726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 426 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); }
#line 2732 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 427 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); }
#line 2738 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 428 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); }
#line 2744 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 429 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); }
#line 2750 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 430 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); }
#line 2756 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 431 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_eq); }
#line 2762 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 432 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); }
#line 2768 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 433 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); }
#line 2774 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 434 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); }
#line 2780 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 435 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); }
#line 2786 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 436 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); }
#line 2792 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 437 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); }
#line 2798 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 438 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); }
#line 2804 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 439 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); }
#line 2810 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 440 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); }
#line 2816 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 441 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); }
#line 2822 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 442 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code); }
#line 2828 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 443 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.code); g_lingo->code1(g_lingo->c_negate); }
#line 2834 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 444 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).code) = (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.code); }
#line 2840 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 445 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).code) = g_lingo->code1(g_lingo->c_arraypush); g_lingo->codeArray((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.narg)); }
#line 2846 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 446 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); }
#line 2852 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 447 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); }
#line 2858 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 448 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); }
#line 2864 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 449 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); }
#line 2870 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 450 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); }
#line 2876 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 451 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); }
#line 2882 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 452 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); }
#line 2888 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 453 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); }
#line 2894 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 454 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); }
#line 2900 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 455 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); }
#line 2906 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 456 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe(nullptr, 0); }
#line 2912 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 459 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s), 1);
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s); }
#line 2920 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 464 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); }
#line 2926 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 467 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); }
#line 2932 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 468 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_procret); }
#line 2938 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 472 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s), 0);
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 2946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 475 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s), 1);
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s); }
#line 2954 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 478 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s), 1);
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s); }
#line 2962 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 481 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_voidpush);
		g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s), 1);
		delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 2971 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 485 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s), (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.narg)); }
#line 2977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 486 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s), 0); }
#line 2983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116:
#line 487 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.s), (((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.narg)); }
#line 2989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 488 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); }
#line 2995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 489 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); }
#line 3001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 490 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.s)); s += '-'; s += *(((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s); g_lingo->codeFunc(&s, (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.narg)); }
#line 3007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 493 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str()); delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 3013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 494 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str()); delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 3019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 497 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str()); delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 3025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 498 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str()); delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 3031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 501 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str()); delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 3037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 502 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)->c_str()); delete (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); }
#line 3043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 513 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); }
#line 3049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 514 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); }
#line 3055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 515 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); }
#line 3061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 516 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_goto); }
#line 3070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 520 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_goto); }
#line 3079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 524 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_goto); }
#line 3088 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 538 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); }
#line 3094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 539 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_play); }
#line 3103 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 543 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_play); }
#line 3112 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 547 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_play); }
#line 3121 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 551 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); }
#line 3127 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 551 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.s), (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.narg)); }
#line 3135 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 581 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); }
#line 3141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 582 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(((yyGLRStackItem const *)yyvsp)[YYFILL (-6)].yystate.yysemantics.yysval.s), (((yyGLRStackItem const *)yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.code), (((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.narg));
			g_lingo->_indef = false; }
#line 3150 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 586 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFactory(*(((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)); }
#line 3156 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 587 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; }
#line 3162 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 588 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(((yyGLRStackItem const *)yyvsp)[YYFILL (-6)].yystate.yysemantics.yysval.s), (((yyGLRStackItem const *)yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.code), (((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; }
#line 3171 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 592 "engines/director/lingo/lingo-gr.y"
    {	// D3
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(((yyGLRStackItem const *)yyvsp)[YYFILL (-7)].yystate.yysemantics.yysval.s), (((yyGLRStackItem const *)yyvsp)[YYFILL (-6)].yystate.yysemantics.yysval.code), (((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.narg));
		g_lingo->_indef = false;
		g_lingo->_ignoreMe = false;

		checkEnd((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s), (((yyGLRStackItem const *)yyvsp)[YYFILL (-7)].yystate.yysemantics.yysval.s)->c_str(), false); }
#line 3183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 599 "engines/director/lingo/lingo-gr.y"
    {	// D4. No 'end' clause
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(((yyGLRStackItem const *)yyvsp)[YYFILL (-5)].yystate.yysemantics.yysval.s), (((yyGLRStackItem const *)yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.code), (((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.narg));
		g_lingo->_indef = false;
		g_lingo->_ignoreMe = false; }
#line 3193 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 605 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).s) = (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s); g_lingo->_indef = true; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; }
#line 3199 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 607 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).narg) = 0; }
#line 3205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 608 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)); ((*yyvalp).narg) = 1; }
#line 3211 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 609 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)); ((*yyvalp).narg) = (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.narg) + 1; }
#line 3217 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 610 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.s)); ((*yyvalp).narg) = (((yyGLRStackItem const *)yyvsp)[YYFILL (-3)].yystate.yysemantics.yysval.narg) + 1; }
#line 3223 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 618 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); }
#line 3229 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 622 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((((yyGLRStackItem const *)yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (((yyGLRStackItem const *)yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.narg));
		g_lingo->code1(numpar); }
#line 3240 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 630 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).narg) = 0; }
#line 3246 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 631 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).narg) = 1; }
#line 3252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 632 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).narg) = (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.narg) + 1; }
#line 3258 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 635 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).narg) = 1; }
#line 3264 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 636 "engines/director/lingo/lingo-gr.y"
    { ((*yyvalp).narg) = (((yyGLRStackItem const *)yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.narg) + 1; }
#line 3270 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3274 "engines/director/lingo/lingo-gr.cpp"

      default: break;
    }

  return yyok;
# undef yyerrok
# undef YYABORT
# undef YYACCEPT
# undef YYERROR
# undef YYBACKUP
# undef yyclearin
# undef YYRECOVERING
}


static void
yyuserMerge (int yyn, YYSTYPE* yy0, YYSTYPE* yy1)
{
  YYUSE (yy0);
  YYUSE (yy1);

  switch (yyn)
    {

      default: break;
    }
}

                              /* Bison grammar-table manipulation.  */

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

/** Number of symbols composing the right hand side of rule #RULE.  */
static inline int
yyrhsLength (yyRuleNum yyrule)
{
  return yyr2[yyrule];
}

static void
yydestroyGLRState (char const *yymsg, yyGLRState *yys)
{
  if (yys->yyresolved)
    yydestruct (yymsg, yystos[yys->yylrState],
                &yys->yysemantics.yysval);
  else
    {
#if YYDEBUG
      if (yydebug)
        {
          if (yys->yysemantics.yyfirstVal)
            YYFPRINTF (stderr, "%s unresolved", yymsg);
          else
            YYFPRINTF (stderr, "%s incomplete", yymsg);
          YY_SYMBOL_PRINT ("", yystos[yys->yylrState], YY_NULLPTR, &yys->yyloc);
        }
#endif

      if (yys->yysemantics.yyfirstVal)
        {
          yySemanticOption *yyoption = yys->yysemantics.yyfirstVal;
          yyGLRState *yyrh;
          int yyn;
          for (yyrh = yyoption->yystate, yyn = yyrhsLength (yyoption->yyrule);
               yyn > 0;
               yyrh = yyrh->yypred, yyn -= 1)
            yydestroyGLRState (yymsg, yyrh);
        }
    }
}

/** Left-hand-side symbol for rule #YYRULE.  */
static inline yySymbol
yylhsNonterm (yyRuleNum yyrule)
{
  return yyr1[yyrule];
}

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-290)))

/** True iff LR state YYSTATE has only a default reduction (regardless
 *  of token).  */
static inline yybool
yyisDefaultedState (yyStateNum yystate)
{
  return (yybool) yypact_value_is_default (yypact[yystate]);
}

/** The default reduction for YYSTATE, assuming it has one.  */
static inline yyRuleNum
yydefaultAction (yyStateNum yystate)
{
  return yydefact[yystate];
}

#define yytable_value_is_error(Yytable_value) \
  0

/** The action to take in YYSTATE on seeing YYTOKEN.
 *  Result R means
 *    R < 0:  Reduce on rule -R.
 *    R = 0:  Error.
 *    R > 0:  Shift to state R.
 *  Set *YYCONFLICTS to a pointer into yyconfl to a 0-terminated list
 *  of conflicting reductions.
 */
static inline int
yygetLRActions (yyStateNum yystate, yySymbol yytoken, const short** yyconflicts)
{
  int yyindex = yypact[yystate] + yytoken;
  if (yyisDefaultedState (yystate)
      || yyindex < 0 || YYLAST < yyindex || yycheck[yyindex] != yytoken)
    {
      *yyconflicts = yyconfl;
      return -yydefact[yystate];
    }
  else if (! yytable_value_is_error (yytable[yyindex]))
    {
      *yyconflicts = yyconfl + yyconflp[yyindex];
      return yytable[yyindex];
    }
  else
    {
      *yyconflicts = yyconfl + yyconflp[yyindex];
      return 0;
    }
}

/** Compute post-reduction state.
 * \param yystate   the current state
 * \param yysym     the nonterminal to push on the stack
 */
static inline yyStateNum
yyLRgotoState (yyStateNum yystate, yySymbol yysym)
{
  int yyr = yypgoto[yysym - YYNTOKENS] + yystate;
  if (0 <= yyr && yyr <= YYLAST && yycheck[yyr] == yystate)
    return yytable[yyr];
  else
    return yydefgoto[yysym - YYNTOKENS];
}

static inline yybool
yyisShiftAction (int yyaction)
{
  return (yybool) (0 < yyaction);
}

static inline yybool
yyisErrorAction (int yyaction)
{
  return (yybool) (yyaction == 0);
}

                                /* GLRStates */

/** Return a fresh GLRStackItem in YYSTACKP.  The item is an LR state
 *  if YYISSTATE, and otherwise a semantic option.  Callers should call
 *  YY_RESERVE_GLRSTACK afterwards to make sure there is sufficient
 *  headroom.  */

static inline yyGLRStackItem*
yynewGLRStackItem (yyGLRStack* yystackp, yybool yyisState)
{
  yyGLRStackItem* yynewItem = yystackp->yynextFree;
  yystackp->yyspaceLeft -= 1;
  yystackp->yynextFree += 1;
  yynewItem->yystate.yyisState = yyisState;
  return yynewItem;
}

/** Add a new semantic action that will execute the action for rule
 *  YYRULE on the semantic values in YYRHS to the list of
 *  alternative actions for YYSTATE.  Assumes that YYRHS comes from
 *  stack #YYK of *YYSTACKP. */
static void
yyaddDeferredAction (yyGLRStack* yystackp, size_t yyk, yyGLRState* yystate,
                     yyGLRState* yyrhs, yyRuleNum yyrule)
{
  yySemanticOption* yynewOption =
    &yynewGLRStackItem (yystackp, yyfalse)->yyoption;
  YYASSERT (!yynewOption->yyisState);
  yynewOption->yystate = yyrhs;
  yynewOption->yyrule = yyrule;
  if (yystackp->yytops.yylookaheadNeeds[yyk])
    {
      yynewOption->yyrawchar = yychar;
      yynewOption->yyval = yylval;
    }
  else
    yynewOption->yyrawchar = YYEMPTY;
  yynewOption->yynext = yystate->yysemantics.yyfirstVal;
  yystate->yysemantics.yyfirstVal = yynewOption;

  YY_RESERVE_GLRSTACK (yystackp);
}

                                /* GLRStacks */

/** Initialize YYSET to a singleton set containing an empty stack.  */
static yybool
yyinitStateSet (yyGLRStateSet* yyset)
{
  yyset->yysize = 1;
  yyset->yycapacity = 16;
  yyset->yystates
    = (yyGLRState**) YYMALLOC (yyset->yycapacity * sizeof yyset->yystates[0]);
  if (! yyset->yystates)
    return yyfalse;
  yyset->yystates[0] = YY_NULLPTR;
  yyset->yylookaheadNeeds
    = (yybool*) YYMALLOC (yyset->yycapacity * sizeof yyset->yylookaheadNeeds[0]);
  if (! yyset->yylookaheadNeeds)
    {
      YYFREE (yyset->yystates);
      return yyfalse;
    }
  memset (yyset->yylookaheadNeeds,
          0, yyset->yycapacity * sizeof yyset->yylookaheadNeeds[0]);
  return yytrue;
}

static void yyfreeStateSet (yyGLRStateSet* yyset)
{
  YYFREE (yyset->yystates);
  YYFREE (yyset->yylookaheadNeeds);
}

/** Initialize *YYSTACKP to a single empty stack, with total maximum
 *  capacity for all stacks of YYSIZE.  */
static yybool
yyinitGLRStack (yyGLRStack* yystackp, size_t yysize)
{
  yystackp->yyerrState = 0;
  yynerrs = 0;
  yystackp->yyspaceLeft = yysize;
  yystackp->yyitems =
    (yyGLRStackItem*) YYMALLOC (yysize * sizeof yystackp->yynextFree[0]);
  if (!yystackp->yyitems)
    return yyfalse;
  yystackp->yynextFree = yystackp->yyitems;
  yystackp->yysplitPoint = YY_NULLPTR;
  yystackp->yylastDeleted = YY_NULLPTR;
  return yyinitStateSet (&yystackp->yytops);
}


#if YYSTACKEXPANDABLE
# define YYRELOC(YYFROMITEMS,YYTOITEMS,YYX,YYTYPE) \
  &((YYTOITEMS) - ((YYFROMITEMS) - (yyGLRStackItem*) (YYX)))->YYTYPE

/** If *YYSTACKP is expandable, extend it.  WARNING: Pointers into the
    stack from outside should be considered invalid after this call.
    We always expand when there are 1 or fewer items left AFTER an
    allocation, so that we can avoid having external pointers exist
    across an allocation.  */
static void
yyexpandGLRStack (yyGLRStack* yystackp)
{
  yyGLRStackItem* yynewItems;
  yyGLRStackItem* yyp0, *yyp1;
  size_t yynewSize;
  size_t yyn;
  size_t yysize = (size_t) (yystackp->yynextFree - yystackp->yyitems);
  if (YYMAXDEPTH - YYHEADROOM < yysize)
    yyMemoryExhausted (yystackp);
  yynewSize = 2*yysize;
  if (YYMAXDEPTH < yynewSize)
    yynewSize = YYMAXDEPTH;
  yynewItems = (yyGLRStackItem*) YYMALLOC (yynewSize * sizeof yynewItems[0]);
  if (! yynewItems)
    yyMemoryExhausted (yystackp);
  for (yyp0 = yystackp->yyitems, yyp1 = yynewItems, yyn = yysize;
       0 < yyn;
       yyn -= 1, yyp0 += 1, yyp1 += 1)
    {
      *yyp1 = *yyp0;
      if (*(yybool *) yyp0)
        {
          yyGLRState* yys0 = &yyp0->yystate;
          yyGLRState* yys1 = &yyp1->yystate;
          if (yys0->yypred != YY_NULLPTR)
            yys1->yypred =
              YYRELOC (yyp0, yyp1, yys0->yypred, yystate);
          if (! yys0->yyresolved && yys0->yysemantics.yyfirstVal != YY_NULLPTR)
            yys1->yysemantics.yyfirstVal =
              YYRELOC (yyp0, yyp1, yys0->yysemantics.yyfirstVal, yyoption);
        }
      else
        {
          yySemanticOption* yyv0 = &yyp0->yyoption;
          yySemanticOption* yyv1 = &yyp1->yyoption;
          if (yyv0->yystate != YY_NULLPTR)
            yyv1->yystate = YYRELOC (yyp0, yyp1, yyv0->yystate, yystate);
          if (yyv0->yynext != YY_NULLPTR)
            yyv1->yynext = YYRELOC (yyp0, yyp1, yyv0->yynext, yyoption);
        }
    }
  if (yystackp->yysplitPoint != YY_NULLPTR)
    yystackp->yysplitPoint = YYRELOC (yystackp->yyitems, yynewItems,
                                      yystackp->yysplitPoint, yystate);

  for (yyn = 0; yyn < yystackp->yytops.yysize; yyn += 1)
    if (yystackp->yytops.yystates[yyn] != YY_NULLPTR)
      yystackp->yytops.yystates[yyn] =
        YYRELOC (yystackp->yyitems, yynewItems,
                 yystackp->yytops.yystates[yyn], yystate);
  YYFREE (yystackp->yyitems);
  yystackp->yyitems = yynewItems;
  yystackp->yynextFree = yynewItems + yysize;
  yystackp->yyspaceLeft = yynewSize - yysize;
}
#endif

static void
yyfreeGLRStack (yyGLRStack* yystackp)
{
  YYFREE (yystackp->yyitems);
  yyfreeStateSet (&yystackp->yytops);
}

/** Assuming that YYS is a GLRState somewhere on *YYSTACKP, update the
 *  splitpoint of *YYSTACKP, if needed, so that it is at least as deep as
 *  YYS.  */
static inline void
yyupdateSplit (yyGLRStack* yystackp, yyGLRState* yys)
{
  if (yystackp->yysplitPoint != YY_NULLPTR && yystackp->yysplitPoint > yys)
    yystackp->yysplitPoint = yys;
}

/** Invalidate stack #YYK in *YYSTACKP.  */
static inline void
yymarkStackDeleted (yyGLRStack* yystackp, size_t yyk)
{
  if (yystackp->yytops.yystates[yyk] != YY_NULLPTR)
    yystackp->yylastDeleted = yystackp->yytops.yystates[yyk];
  yystackp->yytops.yystates[yyk] = YY_NULLPTR;
}

/** Undelete the last stack in *YYSTACKP that was marked as deleted.  Can
    only be done once after a deletion, and only when all other stacks have
    been deleted.  */
static void
yyundeleteLastStack (yyGLRStack* yystackp)
{
  if (yystackp->yylastDeleted == YY_NULLPTR || yystackp->yytops.yysize != 0)
    return;
  yystackp->yytops.yystates[0] = yystackp->yylastDeleted;
  yystackp->yytops.yysize = 1;
  YYDPRINTF ((stderr, "Restoring last deleted stack as stack #0.\n"));
  yystackp->yylastDeleted = YY_NULLPTR;
}

static inline void
yyremoveDeletes (yyGLRStack* yystackp)
{
  size_t yyi, yyj;
  yyi = yyj = 0;
  while (yyj < yystackp->yytops.yysize)
    {
      if (yystackp->yytops.yystates[yyi] == YY_NULLPTR)
        {
          if (yyi == yyj)
            {
              YYDPRINTF ((stderr, "Removing dead stacks.\n"));
            }
          yystackp->yytops.yysize -= 1;
        }
      else
        {
          yystackp->yytops.yystates[yyj] = yystackp->yytops.yystates[yyi];
          /* In the current implementation, it's unnecessary to copy
             yystackp->yytops.yylookaheadNeeds[yyi] since, after
             yyremoveDeletes returns, the parser immediately either enters
             deterministic operation or shifts a token.  However, it doesn't
             hurt, and the code might evolve to need it.  */
          yystackp->yytops.yylookaheadNeeds[yyj] =
            yystackp->yytops.yylookaheadNeeds[yyi];
          if (yyj != yyi)
            {
              YYDPRINTF ((stderr, "Rename stack %lu -> %lu.\n",
                          (unsigned long) yyi, (unsigned long) yyj));
            }
          yyj += 1;
        }
      yyi += 1;
    }
}

/** Shift to a new state on stack #YYK of *YYSTACKP, corresponding to LR
 * state YYLRSTATE, at input position YYPOSN, with (resolved) semantic
 * value *YYVALP and source location *YYLOCP.  */
static inline void
yyglrShift (yyGLRStack* yystackp, size_t yyk, yyStateNum yylrState,
            size_t yyposn,
            YYSTYPE* yyvalp)
{
  yyGLRState* yynewState = &yynewGLRStackItem (yystackp, yytrue)->yystate;

  yynewState->yylrState = yylrState;
  yynewState->yyposn = yyposn;
  yynewState->yyresolved = yytrue;
  yynewState->yypred = yystackp->yytops.yystates[yyk];
  yynewState->yysemantics.yysval = *yyvalp;
  yystackp->yytops.yystates[yyk] = yynewState;

  YY_RESERVE_GLRSTACK (yystackp);
}

/** Shift stack #YYK of *YYSTACKP, to a new state corresponding to LR
 *  state YYLRSTATE, at input position YYPOSN, with the (unresolved)
 *  semantic value of YYRHS under the action for YYRULE.  */
static inline void
yyglrShiftDefer (yyGLRStack* yystackp, size_t yyk, yyStateNum yylrState,
                 size_t yyposn, yyGLRState* yyrhs, yyRuleNum yyrule)
{
  yyGLRState* yynewState = &yynewGLRStackItem (yystackp, yytrue)->yystate;
  YYASSERT (yynewState->yyisState);

  yynewState->yylrState = yylrState;
  yynewState->yyposn = yyposn;
  yynewState->yyresolved = yyfalse;
  yynewState->yypred = yystackp->yytops.yystates[yyk];
  yynewState->yysemantics.yyfirstVal = YY_NULLPTR;
  yystackp->yytops.yystates[yyk] = yynewState;

  /* Invokes YY_RESERVE_GLRSTACK.  */
  yyaddDeferredAction (yystackp, yyk, yynewState, yyrhs, yyrule);
}

#if !YYDEBUG
# define YY_REDUCE_PRINT(Args)
#else
# define YY_REDUCE_PRINT(Args)          \
  do {                                  \
    if (yydebug)                        \
      yy_reduce_print Args;             \
  } while (0)

/*----------------------------------------------------------------------.
| Report that stack #YYK of *YYSTACKP is going to be reduced by YYRULE. |
`----------------------------------------------------------------------*/

static inline void
yy_reduce_print (yybool yynormal, yyGLRStackItem* yyvsp, size_t yyk,
                 yyRuleNum yyrule)
{
  int yynrhs = yyrhsLength (yyrule);
  int yyi;
  YYFPRINTF (stderr, "Reducing stack %lu by rule %d (line %lu):\n",
             (unsigned long) yyk, yyrule - 1,
             (unsigned long) yyrline[yyrule]);
  if (! yynormal)
    yyfillin (yyvsp, 1, -yynrhs);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyvsp[yyi - yynrhs + 1].yystate.yylrState],
                       &yyvsp[yyi - yynrhs + 1].yystate.yysemantics.yysval                       );
      if (!yyvsp[yyi - yynrhs + 1].yystate.yyresolved)
        YYFPRINTF (stderr, " (unresolved)");
      YYFPRINTF (stderr, "\n");
    }
}
#endif

/** Pop the symbols consumed by reduction #YYRULE from the top of stack
 *  #YYK of *YYSTACKP, and perform the appropriate semantic action on their
 *  semantic values.  Assumes that all ambiguities in semantic values
 *  have been previously resolved.  Set *YYVALP to the resulting value,
 *  and *YYLOCP to the computed location (if any).  Return value is as
 *  for userAction.  */
static inline YYRESULTTAG
yydoAction (yyGLRStack* yystackp, size_t yyk, yyRuleNum yyrule,
            YYSTYPE* yyvalp)
{
  int yynrhs = yyrhsLength (yyrule);

  if (yystackp->yysplitPoint == YY_NULLPTR)
    {
      /* Standard special case: single stack.  */
      yyGLRStackItem* yyrhs = (yyGLRStackItem*) yystackp->yytops.yystates[yyk];
      YYASSERT (yyk == 0);
      yystackp->yynextFree -= yynrhs;
      yystackp->yyspaceLeft += (size_t) yynrhs;
      yystackp->yytops.yystates[0] = & yystackp->yynextFree[-1].yystate;
      YY_REDUCE_PRINT ((yytrue, yyrhs, yyk, yyrule));
      return yyuserAction (yyrule, yynrhs, yyrhs, yystackp,
                           yyvalp);
    }
  else
    {
      int yyi;
      yyGLRState* yys;
      yyGLRStackItem yyrhsVals[YYMAXRHS + YYMAXLEFT + 1];
      yys = yyrhsVals[YYMAXRHS + YYMAXLEFT].yystate.yypred
        = yystackp->yytops.yystates[yyk];
      for (yyi = 0; yyi < yynrhs; yyi += 1)
        {
          yys = yys->yypred;
          YYASSERT (yys);
        }
      yyupdateSplit (yystackp, yys);
      yystackp->yytops.yystates[yyk] = yys;
      YY_REDUCE_PRINT ((yyfalse, yyrhsVals + YYMAXRHS + YYMAXLEFT - 1, yyk, yyrule));
      return yyuserAction (yyrule, yynrhs, yyrhsVals + YYMAXRHS + YYMAXLEFT - 1,
                           yystackp, yyvalp);
    }
}

/** Pop items off stack #YYK of *YYSTACKP according to grammar rule YYRULE,
 *  and push back on the resulting nonterminal symbol.  Perform the
 *  semantic action associated with YYRULE and store its value with the
 *  newly pushed state, if YYFORCEEVAL or if *YYSTACKP is currently
 *  unambiguous.  Otherwise, store the deferred semantic action with
 *  the new state.  If the new state would have an identical input
 *  position, LR state, and predecessor to an existing state on the stack,
 *  it is identified with that existing state, eliminating stack #YYK from
 *  *YYSTACKP.  In this case, the semantic value is
 *  added to the options for the existing state's semantic value.
 */
static inline YYRESULTTAG
yyglrReduce (yyGLRStack* yystackp, size_t yyk, yyRuleNum yyrule,
             yybool yyforceEval)
{
  size_t yyposn = yystackp->yytops.yystates[yyk]->yyposn;

  if (yyforceEval || yystackp->yysplitPoint == YY_NULLPTR)
    {
      YYSTYPE yysval;

      YYRESULTTAG yyflag = yydoAction (yystackp, yyk, yyrule, &yysval);
      if (yyflag == yyerr && yystackp->yysplitPoint != YY_NULLPTR)
        {
          YYDPRINTF ((stderr, "Parse on stack %lu rejected by rule #%d.\n",
                     (unsigned long) yyk, yyrule - 1));
        }
      if (yyflag != yyok)
        return yyflag;
      YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyrule], &yysval, &yyloc);
      yyglrShift (yystackp, yyk,
                  yyLRgotoState (yystackp->yytops.yystates[yyk]->yylrState,
                                 yylhsNonterm (yyrule)),
                  yyposn, &yysval);
    }
  else
    {
      size_t yyi;
      int yyn;
      yyGLRState* yys, *yys0 = yystackp->yytops.yystates[yyk];
      yyStateNum yynewLRState;

      for (yys = yystackp->yytops.yystates[yyk], yyn = yyrhsLength (yyrule);
           0 < yyn; yyn -= 1)
        {
          yys = yys->yypred;
          YYASSERT (yys);
        }
      yyupdateSplit (yystackp, yys);
      yynewLRState = yyLRgotoState (yys->yylrState, yylhsNonterm (yyrule));
      YYDPRINTF ((stderr,
                  "Reduced stack %lu by rule #%d; action deferred.  "
                  "Now in state %d.\n",
                  (unsigned long) yyk, yyrule - 1, yynewLRState));
      for (yyi = 0; yyi < yystackp->yytops.yysize; yyi += 1)
        if (yyi != yyk && yystackp->yytops.yystates[yyi] != YY_NULLPTR)
          {
            yyGLRState *yysplit = yystackp->yysplitPoint;
            yyGLRState *yyp = yystackp->yytops.yystates[yyi];
            while (yyp != yys && yyp != yysplit && yyp->yyposn >= yyposn)
              {
                if (yyp->yylrState == yynewLRState && yyp->yypred == yys)
                  {
                    yyaddDeferredAction (yystackp, yyk, yyp, yys0, yyrule);
                    yymarkStackDeleted (yystackp, yyk);
                    YYDPRINTF ((stderr, "Merging stack %lu into stack %lu.\n",
                                (unsigned long) yyk,
                                (unsigned long) yyi));
                    return yyok;
                  }
                yyp = yyp->yypred;
              }
          }
      yystackp->yytops.yystates[yyk] = yys;
      yyglrShiftDefer (yystackp, yyk, yynewLRState, yyposn, yys0, yyrule);
    }
  return yyok;
}

static size_t
yysplitStack (yyGLRStack* yystackp, size_t yyk)
{
  if (yystackp->yysplitPoint == YY_NULLPTR)
    {
      YYASSERT (yyk == 0);
      yystackp->yysplitPoint = yystackp->yytops.yystates[yyk];
    }
  if (yystackp->yytops.yysize >= yystackp->yytops.yycapacity)
    {
      yyGLRState** yynewStates = YY_NULLPTR;
      yybool* yynewLookaheadNeeds;

      if (yystackp->yytops.yycapacity
          > (YYSIZEMAX / (2 * sizeof yynewStates[0])))
        yyMemoryExhausted (yystackp);
      yystackp->yytops.yycapacity *= 2;

      yynewStates =
        (yyGLRState**) YYREALLOC (yystackp->yytops.yystates,
                                  (yystackp->yytops.yycapacity
                                   * sizeof yynewStates[0]));
      if (yynewStates == YY_NULLPTR)
        yyMemoryExhausted (yystackp);
      yystackp->yytops.yystates = yynewStates;

      yynewLookaheadNeeds =
        (yybool*) YYREALLOC (yystackp->yytops.yylookaheadNeeds,
                             (yystackp->yytops.yycapacity
                              * sizeof yynewLookaheadNeeds[0]));
      if (yynewLookaheadNeeds == YY_NULLPTR)
        yyMemoryExhausted (yystackp);
      yystackp->yytops.yylookaheadNeeds = yynewLookaheadNeeds;
    }
  yystackp->yytops.yystates[yystackp->yytops.yysize]
    = yystackp->yytops.yystates[yyk];
  yystackp->yytops.yylookaheadNeeds[yystackp->yytops.yysize]
    = yystackp->yytops.yylookaheadNeeds[yyk];
  yystackp->yytops.yysize += 1;
  return yystackp->yytops.yysize-1;
}

/** True iff YYY0 and YYY1 represent identical options at the top level.
 *  That is, they represent the same rule applied to RHS symbols
 *  that produce the same terminal symbols.  */
static yybool
yyidenticalOptions (yySemanticOption* yyy0, yySemanticOption* yyy1)
{
  if (yyy0->yyrule == yyy1->yyrule)
    {
      yyGLRState *yys0, *yys1;
      int yyn;
      for (yys0 = yyy0->yystate, yys1 = yyy1->yystate,
           yyn = yyrhsLength (yyy0->yyrule);
           yyn > 0;
           yys0 = yys0->yypred, yys1 = yys1->yypred, yyn -= 1)
        if (yys0->yyposn != yys1->yyposn)
          return yyfalse;
      return yytrue;
    }
  else
    return yyfalse;
}

/** Assuming identicalOptions (YYY0,YYY1), destructively merge the
 *  alternative semantic values for the RHS-symbols of YYY1 and YYY0.  */
static void
yymergeOptionSets (yySemanticOption* yyy0, yySemanticOption* yyy1)
{
  yyGLRState *yys0, *yys1;
  int yyn;
  for (yys0 = yyy0->yystate, yys1 = yyy1->yystate,
       yyn = yyrhsLength (yyy0->yyrule);
       yyn > 0;
       yys0 = yys0->yypred, yys1 = yys1->yypred, yyn -= 1)
    {
      if (yys0 == yys1)
        break;
      else if (yys0->yyresolved)
        {
          yys1->yyresolved = yytrue;
          yys1->yysemantics.yysval = yys0->yysemantics.yysval;
        }
      else if (yys1->yyresolved)
        {
          yys0->yyresolved = yytrue;
          yys0->yysemantics.yysval = yys1->yysemantics.yysval;
        }
      else
        {
          yySemanticOption** yyz0p = &yys0->yysemantics.yyfirstVal;
          yySemanticOption* yyz1 = yys1->yysemantics.yyfirstVal;
          while (yytrue)
            {
              if (yyz1 == *yyz0p || yyz1 == YY_NULLPTR)
                break;
              else if (*yyz0p == YY_NULLPTR)
                {
                  *yyz0p = yyz1;
                  break;
                }
              else if (*yyz0p < yyz1)
                {
                  yySemanticOption* yyz = *yyz0p;
                  *yyz0p = yyz1;
                  yyz1 = yyz1->yynext;
                  (*yyz0p)->yynext = yyz;
                }
              yyz0p = &(*yyz0p)->yynext;
            }
          yys1->yysemantics.yyfirstVal = yys0->yysemantics.yyfirstVal;
        }
    }
}

/** Y0 and Y1 represent two possible actions to take in a given
 *  parsing state; return 0 if no combination is possible,
 *  1 if user-mergeable, 2 if Y0 is preferred, 3 if Y1 is preferred.  */
static int
yypreference (yySemanticOption* y0, yySemanticOption* y1)
{
  yyRuleNum r0 = y0->yyrule, r1 = y1->yyrule;
  int p0 = yydprec[r0], p1 = yydprec[r1];

  if (p0 == p1)
    {
      if (yymerger[r0] == 0 || yymerger[r0] != yymerger[r1])
        return 0;
      else
        return 1;
    }
  if (p0 == 0 || p1 == 0)
    return 0;
  if (p0 < p1)
    return 3;
  if (p1 < p0)
    return 2;
  return 0;
}

static YYRESULTTAG yyresolveValue (yyGLRState* yys,
                                   yyGLRStack* yystackp);


/** Resolve the previous YYN states starting at and including state YYS
 *  on *YYSTACKP. If result != yyok, some states may have been left
 *  unresolved possibly with empty semantic option chains.  Regardless
 *  of whether result = yyok, each state has been left with consistent
 *  data so that yydestroyGLRState can be invoked if necessary.  */
static YYRESULTTAG
yyresolveStates (yyGLRState* yys, int yyn,
                 yyGLRStack* yystackp)
{
  if (0 < yyn)
    {
      YYASSERT (yys->yypred);
      YYCHK (yyresolveStates (yys->yypred, yyn-1, yystackp));
      if (! yys->yyresolved)
        YYCHK (yyresolveValue (yys, yystackp));
    }
  return yyok;
}

/** Resolve the states for the RHS of YYOPT on *YYSTACKP, perform its
 *  user action, and return the semantic value and location in *YYVALP
 *  and *YYLOCP.  Regardless of whether result = yyok, all RHS states
 *  have been destroyed (assuming the user action destroys all RHS
 *  semantic values if invoked).  */
static YYRESULTTAG
yyresolveAction (yySemanticOption* yyopt, yyGLRStack* yystackp,
                 YYSTYPE* yyvalp)
{
  yyGLRStackItem yyrhsVals[YYMAXRHS + YYMAXLEFT + 1];
  int yynrhs = yyrhsLength (yyopt->yyrule);
  YYRESULTTAG yyflag =
    yyresolveStates (yyopt->yystate, yynrhs, yystackp);
  if (yyflag != yyok)
    {
      yyGLRState *yys;
      for (yys = yyopt->yystate; yynrhs > 0; yys = yys->yypred, yynrhs -= 1)
        yydestroyGLRState ("Cleanup: popping", yys);
      return yyflag;
    }

  yyrhsVals[YYMAXRHS + YYMAXLEFT].yystate.yypred = yyopt->yystate;
  {
    int yychar_current = yychar;
    YYSTYPE yylval_current = yylval;
    yychar = yyopt->yyrawchar;
    yylval = yyopt->yyval;
    yyflag = yyuserAction (yyopt->yyrule, yynrhs,
                           yyrhsVals + YYMAXRHS + YYMAXLEFT - 1,
                           yystackp, yyvalp);
    yychar = yychar_current;
    yylval = yylval_current;
  }
  return yyflag;
}

#if YYDEBUG
static void
yyreportTree (yySemanticOption* yyx, int yyindent)
{
  int yynrhs = yyrhsLength (yyx->yyrule);
  int yyi;
  yyGLRState* yys;
  yyGLRState* yystates[1 + YYMAXRHS];
  yyGLRState yyleftmost_state;

  for (yyi = yynrhs, yys = yyx->yystate; 0 < yyi; yyi -= 1, yys = yys->yypred)
    yystates[yyi] = yys;
  if (yys == YY_NULLPTR)
    {
      yyleftmost_state.yyposn = 0;
      yystates[0] = &yyleftmost_state;
    }
  else
    yystates[0] = yys;

  if (yyx->yystate->yyposn < yys->yyposn + 1)
    YYFPRINTF (stderr, "%*s%s -> <Rule %d, empty>\n",
               yyindent, "", yytokenName (yylhsNonterm (yyx->yyrule)),
               yyx->yyrule - 1);
  else
    YYFPRINTF (stderr, "%*s%s -> <Rule %d, tokens %lu .. %lu>\n",
               yyindent, "", yytokenName (yylhsNonterm (yyx->yyrule)),
               yyx->yyrule - 1, (unsigned long) (yys->yyposn + 1),
               (unsigned long) yyx->yystate->yyposn);
  for (yyi = 1; yyi <= yynrhs; yyi += 1)
    {
      if (yystates[yyi]->yyresolved)
        {
          if (yystates[yyi-1]->yyposn+1 > yystates[yyi]->yyposn)
            YYFPRINTF (stderr, "%*s%s <empty>\n", yyindent+2, "",
                       yytokenName (yystos[yystates[yyi]->yylrState]));
          else
            YYFPRINTF (stderr, "%*s%s <tokens %lu .. %lu>\n", yyindent+2, "",
                       yytokenName (yystos[yystates[yyi]->yylrState]),
                       (unsigned long) (yystates[yyi-1]->yyposn + 1),
                       (unsigned long) yystates[yyi]->yyposn);
        }
      else
        yyreportTree (yystates[yyi]->yysemantics.yyfirstVal, yyindent+2);
    }
}
#endif

static YYRESULTTAG
yyreportAmbiguity (yySemanticOption* yyx0,
                   yySemanticOption* yyx1)
{
  YYUSE (yyx0);
  YYUSE (yyx1);

#if YYDEBUG
  YYFPRINTF (stderr, "Ambiguity detected.\n");
  YYFPRINTF (stderr, "Option 1,\n");
  yyreportTree (yyx0, 2);
  YYFPRINTF (stderr, "\nOption 2,\n");
  yyreportTree (yyx1, 2);
  YYFPRINTF (stderr, "\n");
#endif

  yyerror (YY_("syntax is ambiguous"));
  return yyabort;
}

/** Resolve the ambiguity represented in state YYS in *YYSTACKP,
 *  perform the indicated actions, and set the semantic value of YYS.
 *  If result != yyok, the chain of semantic options in YYS has been
 *  cleared instead or it has been left unmodified except that
 *  redundant options may have been removed.  Regardless of whether
 *  result = yyok, YYS has been left with consistent data so that
 *  yydestroyGLRState can be invoked if necessary.  */
static YYRESULTTAG
yyresolveValue (yyGLRState* yys, yyGLRStack* yystackp)
{
  yySemanticOption* yyoptionList = yys->yysemantics.yyfirstVal;
  yySemanticOption* yybest = yyoptionList;
  yySemanticOption** yypp;
  yybool yymerge = yyfalse;
  YYSTYPE yysval;
  YYRESULTTAG yyflag;

  for (yypp = &yyoptionList->yynext; *yypp != YY_NULLPTR; )
    {
      yySemanticOption* yyp = *yypp;

      if (yyidenticalOptions (yybest, yyp))
        {
          yymergeOptionSets (yybest, yyp);
          *yypp = yyp->yynext;
        }
      else
        {
          switch (yypreference (yybest, yyp))
            {
            case 0:
              return yyreportAmbiguity (yybest, yyp);
              break;
            case 1:
              yymerge = yytrue;
              break;
            case 2:
              break;
            case 3:
              yybest = yyp;
              yymerge = yyfalse;
              break;
            default:
              /* This cannot happen so it is not worth a YYASSERT (yyfalse),
                 but some compilers complain if the default case is
                 omitted.  */
              break;
            }
          yypp = &yyp->yynext;
        }
    }

  if (yymerge)
    {
      yySemanticOption* yyp;
      int yyprec = yydprec[yybest->yyrule];
      yyflag = yyresolveAction (yybest, yystackp, &yysval);
      if (yyflag == yyok)
        for (yyp = yybest->yynext; yyp != YY_NULLPTR; yyp = yyp->yynext)
          {
            if (yyprec == yydprec[yyp->yyrule])
              {
                YYSTYPE yysval_other;
                yyflag = yyresolveAction (yyp, yystackp, &yysval_other);
                if (yyflag != yyok)
                  {
                    yydestruct ("Cleanup: discarding incompletely merged value for",
                                yystos[yys->yylrState],
                                &yysval);
                    break;
                  }
                yyuserMerge (yymerger[yyp->yyrule], &yysval, &yysval_other);
              }
          }
    }
  else
    yyflag = yyresolveAction (yybest, yystackp, &yysval);

  if (yyflag == yyok)
    {
      yys->yyresolved = yytrue;
      yys->yysemantics.yysval = yysval;
    }
  else
    yys->yysemantics.yyfirstVal = YY_NULLPTR;
  return yyflag;
}

static YYRESULTTAG
yyresolveStack (yyGLRStack* yystackp)
{
  if (yystackp->yysplitPoint != YY_NULLPTR)
    {
      yyGLRState* yys;
      int yyn;

      for (yyn = 0, yys = yystackp->yytops.yystates[0];
           yys != yystackp->yysplitPoint;
           yys = yys->yypred, yyn += 1)
        continue;
      YYCHK (yyresolveStates (yystackp->yytops.yystates[0], yyn, yystackp
                             ));
    }
  return yyok;
}

static void
yycompressStack (yyGLRStack* yystackp)
{
  yyGLRState* yyp, *yyq, *yyr;

  if (yystackp->yytops.yysize != 1 || yystackp->yysplitPoint == YY_NULLPTR)
    return;

  for (yyp = yystackp->yytops.yystates[0], yyq = yyp->yypred, yyr = YY_NULLPTR;
       yyp != yystackp->yysplitPoint;
       yyr = yyp, yyp = yyq, yyq = yyp->yypred)
    yyp->yypred = yyr;

  yystackp->yyspaceLeft += (size_t) (yystackp->yynextFree - yystackp->yyitems);
  yystackp->yynextFree = ((yyGLRStackItem*) yystackp->yysplitPoint) + 1;
  yystackp->yyspaceLeft -= (size_t) (yystackp->yynextFree - yystackp->yyitems);
  yystackp->yysplitPoint = YY_NULLPTR;
  yystackp->yylastDeleted = YY_NULLPTR;

  while (yyr != YY_NULLPTR)
    {
      yystackp->yynextFree->yystate = *yyr;
      yyr = yyr->yypred;
      yystackp->yynextFree->yystate.yypred = &yystackp->yynextFree[-1].yystate;
      yystackp->yytops.yystates[0] = &yystackp->yynextFree->yystate;
      yystackp->yynextFree += 1;
      yystackp->yyspaceLeft -= 1;
    }
}

static YYRESULTTAG
yyprocessOneStack (yyGLRStack* yystackp, size_t yyk,
                   size_t yyposn)
{
  while (yystackp->yytops.yystates[yyk] != YY_NULLPTR)
    {
      yyStateNum yystate = yystackp->yytops.yystates[yyk]->yylrState;
      YYDPRINTF ((stderr, "Stack %lu Entering state %d\n",
                  (unsigned long) yyk, yystate));

      YYASSERT (yystate != YYFINAL);

      if (yyisDefaultedState (yystate))
        {
          YYRESULTTAG yyflag;
          yyRuleNum yyrule = yydefaultAction (yystate);
          if (yyrule == 0)
            {
              YYDPRINTF ((stderr, "Stack %lu dies.\n",
                          (unsigned long) yyk));
              yymarkStackDeleted (yystackp, yyk);
              return yyok;
            }
          yyflag = yyglrReduce (yystackp, yyk, yyrule, yyimmediate[yyrule]);
          if (yyflag == yyerr)
            {
              YYDPRINTF ((stderr,
                          "Stack %lu dies "
                          "(predicate failure or explicit user error).\n",
                          (unsigned long) yyk));
              yymarkStackDeleted (yystackp, yyk);
              return yyok;
            }
          if (yyflag != yyok)
            return yyflag;
        }
      else
        {
          yySymbol yytoken;
          int yyaction;
          const short* yyconflicts;

          yystackp->yytops.yylookaheadNeeds[yyk] = yytrue;
          yytoken = yygetToken (&yychar);
          yyaction = yygetLRActions (yystate, yytoken, &yyconflicts);

          while (*yyconflicts != 0)
            {
              YYRESULTTAG yyflag;
              size_t yynewStack = yysplitStack (yystackp, yyk);
              YYDPRINTF ((stderr, "Splitting off stack %lu from %lu.\n",
                          (unsigned long) yynewStack,
                          (unsigned long) yyk));
              yyflag = yyglrReduce (yystackp, yynewStack,
                                    *yyconflicts,
                                    yyimmediate[*yyconflicts]);
              if (yyflag == yyok)
                YYCHK (yyprocessOneStack (yystackp, yynewStack,
                                          yyposn));
              else if (yyflag == yyerr)
                {
                  YYDPRINTF ((stderr, "Stack %lu dies.\n",
                              (unsigned long) yynewStack));
                  yymarkStackDeleted (yystackp, yynewStack);
                }
              else
                return yyflag;
              yyconflicts += 1;
            }

          if (yyisShiftAction (yyaction))
            break;
          else if (yyisErrorAction (yyaction))
            {
              YYDPRINTF ((stderr, "Stack %lu dies.\n",
                          (unsigned long) yyk));
              yymarkStackDeleted (yystackp, yyk);
              break;
            }
          else
            {
              YYRESULTTAG yyflag = yyglrReduce (yystackp, yyk, -yyaction,
                                                yyimmediate[-yyaction]);
              if (yyflag == yyerr)
                {
                  YYDPRINTF ((stderr,
                              "Stack %lu dies "
                              "(predicate failure or explicit user error).\n",
                              (unsigned long) yyk));
                  yymarkStackDeleted (yystackp, yyk);
                  break;
                }
              else if (yyflag != yyok)
                return yyflag;
            }
        }
    }
  return yyok;
}

static void
yyreportSyntaxError (yyGLRStack* yystackp)
{
  if (yystackp->yyerrState != 0)
    return;
#if ! YYERROR_VERBOSE
  yyerror (YY_("syntax error"));
#else
  {
  yySymbol yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);
  size_t yysize0 = yytnamerr (YY_NULLPTR, yytokenName (yytoken));
  size_t yysize = yysize0;
  yybool yysize_overflow = yyfalse;
  char* yymsg = YY_NULLPTR;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected").  */
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
      int yyn = yypact[yystackp->yytops.yystates[0]->yylrState];
      yyarg[yycount++] = yytokenName (yytoken);
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for this
             state because they are default actions.  */
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
                yyarg[yycount++] = yytokenName (yyx);
                {
                  size_t yysz = yysize + yytnamerr (YY_NULLPTR, yytokenName (yyx));
                  if (yysz < yysize)
                    yysize_overflow = yytrue;
                  yysize = yysz;
                }
              }
        }
    }

  switch (yycount)
    {
#define YYCASE_(N, S)                   \
      case N:                           \
        yyformat = S;                   \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  {
    size_t yysz = yysize + strlen (yyformat);
    if (yysz < yysize)
      yysize_overflow = yytrue;
    yysize = yysz;
  }

  if (!yysize_overflow)
    yymsg = (char *) YYMALLOC (yysize);

  if (yymsg)
    {
      char *yyp = yymsg;
      int yyi = 0;
      while ((*yyp = *yyformat))
        {
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
      yyerror (yymsg);
      YYFREE (yymsg);
    }
  else
    {
      yyerror (YY_("syntax error"));
      yyMemoryExhausted (yystackp);
    }
  }
#endif /* YYERROR_VERBOSE */
  yynerrs += 1;
}

/* Recover from a syntax error on *YYSTACKP, assuming that *YYSTACKP->YYTOKENP,
   yylval, and yylloc are the syntactic category, semantic value, and location
   of the lookahead.  */
static void
yyrecoverSyntaxError (yyGLRStack* yystackp)
{
  if (yystackp->yyerrState == 3)
    /* We just shifted the error token and (perhaps) took some
       reductions.  Skip tokens until we can proceed.  */
    while (yytrue)
      {
        yySymbol yytoken;
        int yyj;
        if (yychar == YYEOF)
          yyFail (yystackp, YY_NULLPTR);
        if (yychar != YYEMPTY)
          {
            yytoken = YYTRANSLATE (yychar);
            yydestruct ("Error: discarding",
                        yytoken, &yylval);
            yychar = YYEMPTY;
          }
        yytoken = yygetToken (&yychar);
        yyj = yypact[yystackp->yytops.yystates[0]->yylrState];
        if (yypact_value_is_default (yyj))
          return;
        yyj += yytoken;
        if (yyj < 0 || YYLAST < yyj || yycheck[yyj] != yytoken)
          {
            if (yydefact[yystackp->yytops.yystates[0]->yylrState] != 0)
              return;
          }
        else if (! yytable_value_is_error (yytable[yyj]))
          return;
      }

  /* Reduce to one stack.  */
  {
    size_t yyk;
    for (yyk = 0; yyk < yystackp->yytops.yysize; yyk += 1)
      if (yystackp->yytops.yystates[yyk] != YY_NULLPTR)
        break;
    if (yyk >= yystackp->yytops.yysize)
      yyFail (yystackp, YY_NULLPTR);
    for (yyk += 1; yyk < yystackp->yytops.yysize; yyk += 1)
      yymarkStackDeleted (yystackp, yyk);
    yyremoveDeletes (yystackp);
    yycompressStack (yystackp);
  }

  /* Now pop stack until we find a state that shifts the error token.  */
  yystackp->yyerrState = 3;
  while (yystackp->yytops.yystates[0] != YY_NULLPTR)
    {
      yyGLRState *yys = yystackp->yytops.yystates[0];
      int yyj = yypact[yys->yylrState];
      if (! yypact_value_is_default (yyj))
        {
          yyj += YYTERROR;
          if (0 <= yyj && yyj <= YYLAST && yycheck[yyj] == YYTERROR
              && yyisShiftAction (yytable[yyj]))
            {
              /* Shift the error token.  */
              YY_SYMBOL_PRINT ("Shifting", yystos[yytable[yyj]],
                               &yylval, &yyerrloc);
              yyglrShift (yystackp, 0, yytable[yyj],
                          yys->yyposn, &yylval);
              yys = yystackp->yytops.yystates[0];
              break;
            }
        }
      if (yys->yypred != YY_NULLPTR)
        yydestroyGLRState ("Error: popping", yys);
      yystackp->yytops.yystates[0] = yys->yypred;
      yystackp->yynextFree -= 1;
      yystackp->yyspaceLeft += 1;
    }
  if (yystackp->yytops.yystates[0] == YY_NULLPTR)
    yyFail (yystackp, YY_NULLPTR);
}

#define YYCHK1(YYE)                                                          \
  do {                                                                       \
    switch (YYE) {                                                           \
    case yyok:                                                               \
      break;                                                                 \
    case yyabort:                                                            \
      goto yyabortlab;                                                       \
    case yyaccept:                                                           \
      goto yyacceptlab;                                                      \
    case yyerr:                                                              \
      goto yyuser_error;                                                     \
    default:                                                                 \
      goto yybuglab;                                                         \
    }                                                                        \
  } while (0)

/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
  int yyresult;
  yyGLRStack yystack;
  yyGLRStack* const yystackp = &yystack;
  size_t yyposn;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY;
  yylval = yyval_default;

  if (! yyinitGLRStack (yystackp, YYINITDEPTH))
    goto yyexhaustedlab;
  switch (YYSETJMP (yystack.yyexception_buffer))
    {
    case 0: break;
    case 1: goto yyabortlab;
    case 2: goto yyexhaustedlab;
    default: goto yybuglab;
    }
  yyglrShift (&yystack, 0, 0, 0, &yylval);
  yyposn = 0;

  while (yytrue)
    {
      /* For efficiency, we have two loops, the first of which is
         specialized to deterministic operation (single stack, no
         potential ambiguity).  */
      /* Standard mode */
      while (yytrue)
        {
          yyStateNum yystate = yystack.yytops.yystates[0]->yylrState;
          YYDPRINTF ((stderr, "Entering state %d\n", yystate));
          if (yystate == YYFINAL)
            goto yyacceptlab;
          if (yyisDefaultedState (yystate))
            {
              yyRuleNum yyrule = yydefaultAction (yystate);
              if (yyrule == 0)
                {
                  yyreportSyntaxError (&yystack);
                  goto yyuser_error;
                }
              YYCHK1 (yyglrReduce (&yystack, 0, yyrule, yytrue));
            }
          else
            {
              yySymbol yytoken = yygetToken (&yychar);
              const short* yyconflicts;
              int yyaction = yygetLRActions (yystate, yytoken, &yyconflicts);
              if (*yyconflicts != 0)
                break;
              if (yyisShiftAction (yyaction))
                {
                  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
                  yychar = YYEMPTY;
                  yyposn += 1;
                  yyglrShift (&yystack, 0, yyaction, yyposn, &yylval);
                  if (0 < yystack.yyerrState)
                    yystack.yyerrState -= 1;
                }
              else if (yyisErrorAction (yyaction))
                {                  yyreportSyntaxError (&yystack);
                  goto yyuser_error;
                }
              else
                YYCHK1 (yyglrReduce (&yystack, 0, -yyaction, yytrue));
            }
        }

      while (yytrue)
        {
          yySymbol yytoken_to_shift;
          size_t yys;

          for (yys = 0; yys < yystack.yytops.yysize; yys += 1)
            yystackp->yytops.yylookaheadNeeds[yys] = (yybool) (yychar != YYEMPTY);

          /* yyprocessOneStack returns one of three things:

              - An error flag.  If the caller is yyprocessOneStack, it
                immediately returns as well.  When the caller is finally
                yyparse, it jumps to an error label via YYCHK1.

              - yyok, but yyprocessOneStack has invoked yymarkStackDeleted
                (&yystack, yys), which sets the top state of yys to NULL.  Thus,
                yyparse's following invocation of yyremoveDeletes will remove
                the stack.

              - yyok, when ready to shift a token.

             Except in the first case, yyparse will invoke yyremoveDeletes and
             then shift the next token onto all remaining stacks.  This
             synchronization of the shift (that is, after all preceding
             reductions on all stacks) helps prevent double destructor calls
             on yylval in the event of memory exhaustion.  */

          for (yys = 0; yys < yystack.yytops.yysize; yys += 1)
            YYCHK1 (yyprocessOneStack (&yystack, yys, yyposn));
          yyremoveDeletes (&yystack);
          if (yystack.yytops.yysize == 0)
            {
              yyundeleteLastStack (&yystack);
              if (yystack.yytops.yysize == 0)
                yyFail (&yystack, YY_("syntax error"));
              YYCHK1 (yyresolveStack (&yystack));
              YYDPRINTF ((stderr, "Returning to deterministic operation.\n"));
              yyreportSyntaxError (&yystack);
              goto yyuser_error;
            }

          /* If any yyglrShift call fails, it will fail after shifting.  Thus,
             a copy of yylval will already be on stack 0 in the event of a
             failure in the following loop.  Thus, yychar is set to YYEMPTY
             before the loop to make sure the user destructor for yylval isn't
             called twice.  */
          yytoken_to_shift = YYTRANSLATE (yychar);
          yychar = YYEMPTY;
          yyposn += 1;
          for (yys = 0; yys < yystack.yytops.yysize; yys += 1)
            {
              yyStateNum yystate = yystack.yytops.yystates[yys]->yylrState;
              const short* yyconflicts;
              int yyaction = yygetLRActions (yystate, yytoken_to_shift,
                              &yyconflicts);
              /* Note that yyconflicts were handled by yyprocessOneStack.  */
              YYDPRINTF ((stderr, "On stack %lu, ", (unsigned long) yys));
              YY_SYMBOL_PRINT ("shifting", yytoken_to_shift, &yylval, &yylloc);
              yyglrShift (&yystack, yys, yyaction, yyposn,
                          &yylval);
              YYDPRINTF ((stderr, "Stack %lu now in state #%d\n",
                          (unsigned long) yys,
                          yystack.yytops.yystates[yys]->yylrState));
            }

          if (yystack.yytops.yysize == 1)
            {
              YYCHK1 (yyresolveStack (&yystack));
              YYDPRINTF ((stderr, "Returning to deterministic operation.\n"));
              yycompressStack (&yystack);
              break;
            }
        }
      continue;
    yyuser_error:
      yyrecoverSyntaxError (&yystack);
      yyposn = yystack.yytops.yystates[0]->yyposn;
    }

 yyacceptlab:
  yyresult = 0;
  goto yyreturn;

 yybuglab:
  YYASSERT (yyfalse);
  goto yyabortlab;

 yyabortlab:
  yyresult = 1;
  goto yyreturn;

 yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;

 yyreturn:
  if (yychar != YYEMPTY)
    yydestruct ("Cleanup: discarding lookahead",
                YYTRANSLATE (yychar), &yylval);

  /* If the stack is well-formed, pop the stack until it is empty,
     destroying its entries as we go.  But free the stack regardless
     of whether it is well-formed.  */
  if (yystack.yyitems)
    {
      yyGLRState** yystates = yystack.yytops.yystates;
      if (yystates)
        {
          size_t yysize = yystack.yytops.yysize;
          size_t yyk;
          for (yyk = 0; yyk < yysize; yyk += 1)
            if (yystates[yyk])
              {
                while (yystates[yyk])
                  {
                    yyGLRState *yys = yystates[yyk];
                    if (yys->yypred != YY_NULLPTR)
                      yydestroyGLRState ("Cleanup: popping", yys);
                    yystates[yyk] = yys->yypred;
                    yystack.yynextFree -= 1;
                    yystack.yyspaceLeft += 1;
                  }
                break;
              }
        }
      yyfreeGLRStack (&yystack);
    }

  return yyresult;
}

/* DEBUGGING ONLY */
#if YYDEBUG
static void
yy_yypstack (yyGLRState* yys)
{
  if (yys->yypred)
    {
      yy_yypstack (yys->yypred);
      YYFPRINTF (stderr, " -> ");
    }
  YYFPRINTF (stderr, "%d@%lu", yys->yylrState,
             (unsigned long) yys->yyposn);
}

static void
yypstates (yyGLRState* yyst)
{
  if (yyst == YY_NULLPTR)
    YYFPRINTF (stderr, "<null>");
  else
    yy_yypstack (yyst);
  YYFPRINTF (stderr, "\n");
}

static void
yypstack (yyGLRStack* yystackp, size_t yyk)
{
  yypstates (yystackp->yytops.yystates[yyk]);
}

#define YYINDEX(YYX)                                                         \
    ((YYX) == YY_NULLPTR ? -1 : (yyGLRStackItem*) (YYX) - yystackp->yyitems)


static void
yypdumpstack (yyGLRStack* yystackp)
{
  yyGLRStackItem* yyp;
  size_t yyi;
  for (yyp = yystackp->yyitems; yyp < yystackp->yynextFree; yyp += 1)
    {
      YYFPRINTF (stderr, "%3lu. ",
                 (unsigned long) (yyp - yystackp->yyitems));
      if (*(yybool *) yyp)
        {
          YYASSERT (yyp->yystate.yyisState);
          YYASSERT (yyp->yyoption.yyisState);
          YYFPRINTF (stderr, "Res: %d, LR State: %d, posn: %lu, pred: %ld",
                     yyp->yystate.yyresolved, yyp->yystate.yylrState,
                     (unsigned long) yyp->yystate.yyposn,
                     (long) YYINDEX (yyp->yystate.yypred));
          if (! yyp->yystate.yyresolved)
            YYFPRINTF (stderr, ", firstVal: %ld",
                       (long) YYINDEX (yyp->yystate
                                             .yysemantics.yyfirstVal));
        }
      else
        {
          YYASSERT (!yyp->yystate.yyisState);
          YYASSERT (!yyp->yyoption.yyisState);
          YYFPRINTF (stderr, "Option. rule: %d, state: %ld, next: %ld",
                     yyp->yyoption.yyrule - 1,
                     (long) YYINDEX (yyp->yyoption.yystate),
                     (long) YYINDEX (yyp->yyoption.yynext));
        }
      YYFPRINTF (stderr, "\n");
    }
  YYFPRINTF (stderr, "Tops:");
  for (yyi = 0; yyi < yystackp->yytops.yysize; yyi += 1)
    YYFPRINTF (stderr, "%lu: %ld; ", (unsigned long) yyi,
               (long) YYINDEX (yystackp->yytops.yystates[yyi]));
  YYFPRINTF (stderr, "\n");
}
#endif

#undef yylval
#undef yychar
#undef yynerrs



#line 639 "engines/director/lingo/lingo-gr.y"
