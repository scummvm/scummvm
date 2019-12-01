/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
/* Tokens.  */
#define UNARY 258
#define CASTREF 259
#define VOID 260
#define VAR 261
#define POINT 262
#define RECT 263
#define ARRAY 264
#define OBJECT 265
#define REFERENCE 266
#define INT 267
#define THEENTITY 268
#define THEENTITYWITHID 269
#define FLOAT 270
#define BLTIN 271
#define BLTINNOARGS 272
#define BLTINNOARGSORONE 273
#define BLTINONEARG 274
#define BLTINARGLIST 275
#define TWOWORDBUILTIN 276
#define FBLTIN 277
#define FBLTINNOARGS 278
#define FBLTINONEARG 279
#define FBLTINARGLIST 280
#define RBLTIN 281
#define RBLTINONEARG 282
#define ID 283
#define STRING 284
#define HANDLER 285
#define SYMBOL 286
#define ENDCLAUSE 287
#define tPLAYACCEL 288
#define tDOWN 289
#define tELSE 290
#define tNLELSIF 291
#define tEXIT 292
#define tFRAME 293
#define tGLOBAL 294
#define tGO 295
#define tIF 296
#define tINTO 297
#define tLOOP 298
#define tMACRO 299
#define tMOVIE 300
#define tNEXT 301
#define tOF 302
#define tPREVIOUS 303
#define tPUT 304
#define tREPEAT 305
#define tSET 306
#define tTHEN 307
#define tTHENNL 308
#define tTO 309
#define tWHEN 310
#define tWITH 311
#define tWHILE 312
#define tNLELSE 313
#define tFACTORY 314
#define tMETHOD 315
#define tOPEN 316
#define tPLAY 317
#define tDONE 318
#define tINSTANCE 319
#define tGE 320
#define tLE 321
#define tGT 322
#define tLT 323
#define tEQ 324
#define tNEQ 325
#define tAND 326
#define tOR 327
#define tNOT 328
#define tMOD 329
#define tAFTER 330
#define tBEFORE 331
#define tCONCAT 332
#define tCONTAINS 333
#define tSTARTS 334
#define tCHAR 335
#define tITEM 336
#define tLINE 337
#define tWORD 338
#define tSPRITE 339
#define tINTERSECTS 340
#define tWITHIN 341
#define tTELL 342
#define tPROPERTY 343
#define tON 344
#define tME 345




/* Copy the first part of user declarations.  */
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
	warning("LINGO: %s at line %d col %d", s, g_lingo->_linenumber, g_lingo->_colnumber);
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



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 79 "engines/director/lingo/lingo-gr.y"
{
	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Common::Array<double> *arr;
}
/* Line 193 of yacc.c.  */
#line 316 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 329 "engines/director/lingo/lingo-gr.cpp"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  127
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1804

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  169
/* YYNRULES -- Number of states.  */
#define YYNSTATES  369

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   345

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
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
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    14,    16,    18,    20,
      21,    23,    25,    27,    32,    37,    42,    47,    52,    57,
      63,    68,    73,    79,    81,    83,    85,    87,    95,   106,
     118,   122,   129,   134,   141,   151,   161,   171,   178,   189,
     200,   201,   205,   208,   210,   213,   215,   222,   224,   231,
     238,   240,   244,   248,   251,   255,   257,   259,   260,   261,
     263,   266,   269,   273,   275,   277,   279,   281,   283,   285,
     287,   290,   293,   298,   303,   305,   307,   310,   312,   316,
     320,   324,   328,   332,   336,   340,   344,   348,   352,   356,
     360,   363,   367,   371,   375,   379,   382,   385,   389,   393,
     398,   403,   408,   415,   420,   427,   432,   439,   444,   451,
     454,   457,   459,   461,   464,   466,   469,   472,   475,   477,
     480,   483,   485,   488,   493,   498,   505,   510,   513,   517,
     519,   523,   525,   529,   531,   535,   538,   541,   544,   547,
     551,   554,   557,   559,   563,   566,   569,   572,   576,   579,
     580,   584,   585,   594,   597,   598,   607,   615,   622,   625,
     626,   628,   632,   637,   638,   641,   642,   644,   648,   650
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     107,     0,    -1,   107,   108,   112,    -1,   112,    -1,     1,
     108,    -1,    98,    -1,    53,    -1,    58,    -1,    36,    -1,
      -1,   143,    -1,   149,    -1,   115,    -1,    49,   132,    42,
      28,    -1,    49,   132,    42,   133,    -1,    49,   132,    75,
     132,    -1,    49,   132,    76,   132,    -1,    51,    28,    91,
     132,    -1,    51,    13,    91,   132,    -1,    51,    14,   132,
      91,   132,    -1,    51,    28,    54,   132,    -1,    51,    13,
      54,   132,    -1,    51,    14,   132,    54,   132,    -1,   132,
      -1,   134,    -1,   114,    -1,   116,    -1,   123,    99,   122,
     100,   129,   128,    32,    -1,   124,    91,   132,   128,    54,
     132,   128,   129,   128,    32,    -1,   124,    91,   132,   128,
      34,    54,   132,   128,   129,   128,    32,    -1,   130,   114,
     128,    -1,   131,   132,   108,   129,   128,    32,    -1,   131,
     132,    54,   132,    -1,   125,   122,   109,   129,   128,    32,
      -1,   125,   122,   109,   129,   128,   110,   129,   128,    32,
      -1,   125,   122,   109,   129,   128,   127,   118,   128,    32,
      -1,   125,   122,   109,   129,   128,   110,   127,   114,   128,
      -1,   125,   122,    52,   127,   114,   128,    -1,   125,   122,
      52,   127,   114,   128,   110,   127,   114,   128,    -1,   125,
     122,    52,   127,   114,   128,   119,   128,   117,   128,    -1,
      -1,   110,   127,   114,    -1,   118,   121,    -1,   121,    -1,
     119,   120,    -1,   120,    -1,   126,   122,    52,   127,   115,
     128,    -1,   119,    -1,   126,   122,    52,   127,   129,   128,
      -1,   126,   122,    53,   127,   129,   128,    -1,   132,    -1,
     132,    91,   132,    -1,    99,   122,   100,    -1,    50,    57,
      -1,    50,    56,    28,    -1,    41,    -1,   111,    -1,    -1,
      -1,   127,    -1,   129,   108,    -1,   129,   115,    -1,    55,
      28,    52,    -1,    87,    -1,    12,    -1,    15,    -1,    31,
      -1,    29,    -1,   133,    -1,    23,    -1,    24,   132,    -1,
      25,   151,    -1,    25,    99,   151,   100,    -1,    28,    99,
     150,   100,    -1,    28,    -1,    13,    -1,    14,   132,    -1,
     113,    -1,   132,    93,   132,    -1,   132,    94,   132,    -1,
     132,    95,   132,    -1,   132,    96,   132,    -1,   132,    74,
     132,    -1,   132,   101,   132,    -1,   132,   102,   132,    -1,
     132,    70,   132,    -1,   132,    65,   132,    -1,   132,    66,
     132,    -1,   132,    71,   132,    -1,   132,    72,   132,    -1,
      73,   132,    -1,   132,    92,   132,    -1,   132,    77,   132,
      -1,   132,    78,   132,    -1,   132,    79,   132,    -1,    93,
     132,    -1,    94,   132,    -1,    99,   132,   100,    -1,   103,
     150,   104,    -1,    84,   132,    85,   132,    -1,    84,   132,
      86,   132,    -1,    80,   132,    47,   132,    -1,    80,   132,
      54,   132,    47,   132,    -1,    81,   132,    47,   132,    -1,
      81,   132,    54,   132,    47,   132,    -1,    82,   132,    47,
     132,    -1,    82,   132,    54,   132,    47,   132,    -1,    83,
     132,    47,   132,    -1,    83,   132,    54,   132,    47,   132,
      -1,    27,   132,    -1,    49,   132,    -1,   138,    -1,   141,
      -1,    37,    50,    -1,    37,    -1,    39,   135,    -1,    88,
     136,    -1,    64,   137,    -1,    17,    -1,    19,   132,    -1,
      18,   132,    -1,    18,    -1,    20,   151,    -1,    20,    99,
     151,   100,    -1,    90,    99,    28,   100,    -1,    90,    99,
      28,   105,   150,   100,    -1,    61,   132,    56,   132,    -1,
      61,   132,    -1,    21,    28,   150,    -1,    28,    -1,   135,
     105,    28,    -1,    28,    -1,   136,   105,    28,    -1,    28,
      -1,   137,   105,    28,    -1,    40,    43,    -1,    40,    46,
      -1,    40,    48,    -1,    40,   139,    -1,    40,   139,   140,
      -1,    40,   140,    -1,    38,   132,    -1,   132,    -1,    47,
      45,   132,    -1,    45,   132,    -1,    62,    63,    -1,    62,
     139,    -1,    62,   139,   140,    -1,    62,   140,    -1,    -1,
      33,   142,   150,    -1,    -1,    44,    28,   144,   127,   147,
     108,   148,   129,    -1,    59,    28,    -1,    -1,    60,    28,
     145,   127,   147,   108,   148,   129,    -1,   146,   127,   147,
     108,   148,   129,    32,    -1,   146,   127,   147,   108,   148,
     129,    -1,    89,    28,    -1,    -1,    28,    -1,   147,   105,
      28,    -1,   147,   108,   105,    28,    -1,    -1,    28,   151,
      -1,    -1,   132,    -1,   150,   105,   132,    -1,   132,    -1,
     151,   105,   132,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   120,   120,   121,   122,   125,   130,   135,   140,   145,
     146,   147,   148,   151,   157,   160,   161,   162,   168,   175,
     181,   187,   194,   202,   203,   206,   207,   212,   225,   243,
     257,   262,   265,   270,   280,   292,   304,   314,   324,   334,
     346,   347,   350,   351,   354,   355,   358,   366,   367,   373,
     381,   382,   383,   386,   389,   396,   403,   411,   414,   417,
     418,   419,   422,   428,   432,   435,   438,   441,   444,   445,
     448,   451,   452,   453,   456,   460,   468,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   508,
     513,   514,   515,   516,   517,   518,   519,   520,   521,   524,
     527,   530,   534,   535,   536,   537,   538,   539,   540,   543,
     544,   547,   548,   551,   552,   563,   564,   565,   566,   570,
     574,   580,   581,   584,   585,   588,   589,   593,   597,   601,
     601,   631,   631,   636,   637,   637,   642,   649,   655,   657,
     658,   659,   660,   663,   667,   675,   676,   677,   680,   681
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
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
  "'<'", "'['", "']'", "','", "$accept", "program", "nl", "thennl",
  "nlelse", "nlelsif", "programline", "asgn", "stmtoneliner", "stmt",
  "ifstmt", "elsestmtoneliner", "elseifstmt", "elseifstmtoneliner",
  "elseifstmtoneliner1", "elseifstmt1", "cond", "repeatwhile",
  "repeatwith", "if", "elseif", "begin", "end", "stmtlist", "when", "tell",
  "expr", "reference", "proc", "globallist", "propertylist",
  "instancelist", "gotofunc", "gotoframe", "gotomovie", "playfunc", "@1",
  "defn", "@2", "@3", "on", "argdef", "argstore", "macro", "arglist",
  "nonemptyarglist", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
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

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   106,   107,   107,   107,   108,   109,   110,   111,   112,
     112,   112,   112,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   114,   114,   115,   115,   115,   115,   115,
     115,   115,   115,   116,   116,   116,   116,   116,   116,   116,
     117,   117,   118,   118,   119,   119,   120,   121,   121,   121,
     122,   122,   122,   123,   124,   125,   126,   127,   128,   129,
     129,   129,   130,   131,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   133,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   135,
     135,   136,   136,   137,   137,   138,   138,   138,   138,   138,
     138,   139,   139,   140,   140,   141,   141,   141,   141,   142,
     141,   144,   143,   143,   145,   143,   143,   143,   146,   147,
     147,   147,   147,   148,   149,   150,   150,   150,   151,   151
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     1,     1,     1,     0,
       1,     1,     1,     4,     4,     4,     4,     4,     4,     5,
       4,     4,     5,     1,     1,     1,     1,     7,    10,    11,
       3,     6,     4,     6,     9,     9,     9,     6,    10,    10,
       0,     3,     2,     1,     2,     1,     6,     1,     6,     6,
       1,     3,     3,     2,     3,     1,     1,     0,     0,     1,
       2,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     4,     4,     1,     1,     2,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     3,     3,     3,     3,     2,     2,     3,     3,     4,
       4,     4,     6,     4,     6,     4,     6,     4,     6,     2,
       2,     1,     1,     2,     1,     2,     2,     2,     1,     2,
       2,     1,     2,     4,     4,     6,     4,     2,     3,     1,
       3,     1,     3,     1,     3,     2,     2,     2,     2,     3,
       2,     2,     1,     3,     2,     2,     2,     3,     2,     0,
       3,     0,     8,     2,     0,     8,     7,     6,     2,     0,
       1,     3,     4,     0,     2,     0,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    64,    75,     0,    65,   118,   121,     0,     0,
       0,    69,     0,     0,     0,    74,    67,    66,   149,   114,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    63,
       0,     0,     0,     0,     0,     0,   165,     0,     3,    77,
      25,    12,    26,     0,     0,     0,     0,     0,    23,    68,
      24,   111,   112,    10,    57,    11,     5,     4,    74,     0,
      76,   120,   119,     0,   168,   122,   165,    70,     0,    71,
     109,   165,   164,   165,   113,   129,   115,     0,   135,     0,
     136,     0,   137,   142,   138,   140,   151,   110,     0,    53,
       0,     0,     0,     0,   153,   154,   127,   145,   146,   148,
     133,   117,    90,     0,     0,     0,     0,     0,   131,   116,
     158,     0,    95,    96,     0,   166,     0,     1,     9,     0,
       0,     0,     0,    50,    58,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   159,   165,     0,   168,     0,     0,   128,     0,
     166,     0,   150,     0,   141,   144,     0,   139,    57,     0,
       0,     0,    54,     0,     0,     0,     0,     0,    62,    57,
       0,   147,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,    98,     0,     2,     0,
      58,     0,     0,    57,     6,    57,     0,    30,     0,    57,
      86,    87,    85,    88,    89,    82,    92,    93,    94,    91,
      78,    79,    80,    81,    83,    84,   160,     0,   123,   169,
      72,    73,   130,   143,   159,    13,    14,    15,    16,    21,
      18,     0,     0,    20,    17,   159,   126,   134,   101,     0,
     103,     0,   105,     0,   107,     0,    99,   100,   132,   124,
     165,   167,    57,     0,    52,     0,    59,    58,    51,    32,
      58,     0,   163,     0,    22,    19,     0,     0,     0,     0,
       0,     0,    58,     0,     0,    58,    60,    61,    57,     0,
     161,     0,    57,   163,   163,   102,   104,   106,   108,   125,
       0,     0,    58,    37,    33,     7,    57,     0,    31,   162,
     157,    57,    57,    27,    58,    57,     8,    57,    56,    58,
      45,     0,    59,    58,    58,    47,    43,     0,   156,   152,
     155,    57,    58,     0,    44,    40,     0,    58,     0,    42,
       0,     0,    58,     0,    58,    57,    58,    57,    36,    34,
      35,    57,    57,     0,    28,    38,     0,    39,     0,    57,
      57,    29,    41,    58,    58,    58,    46,    48,    49
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    47,   286,   205,   306,   318,    48,    49,    50,   287,
      52,   346,   324,   325,   320,   326,   132,    53,    54,    55,
     321,   266,   207,   267,    56,    57,    58,    59,    60,    86,
     119,   111,    61,    94,    95,    62,    83,    63,   168,   179,
      64,   227,   292,    65,   161,    75
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -277
static const yytype_int16 yypact[] =
{
     382,   -97,  -277,  -277,  1039,  -277,  -277,  1039,  1039,  1079,
      14,  -277,  1039,  1152,  1039,  1192,  -277,  -277,  -277,    13,
      43,   925,  -277,    69,  1039,    12,    48,    72,    75,    90,
    1039,   966,    95,  1039,  1039,  1039,  1039,  1039,  1039,  -277,
      99,   101,    35,  1039,  1039,  1039,  1039,     7,  -277,  -277,
    -277,  -277,  -277,    37,    64,  1265,   842,  1039,  1702,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,    58,  1039,
    1702,  1702,  1702,  1039,  1702,    51,  1039,  1702,  1039,    51,
    1702,  1039,    51,  1039,  -277,  -277,    55,  1039,  -277,  1039,
    -277,   118,  -277,  1702,    46,  -277,  -277,   155,   136,  -277,
     -37,  1039,   -36,   113,  -277,  -277,   261,  -277,    46,  -277,
    -277,    61,   -49,  1306,  1339,  1372,  1405,  1570,  -277,    65,
    -277,   140,   -49,   -49,  1636,  1702,     2,  -277,   474,  1265,
    1039,  1265,    57,  1669,  -277,   818,  1039,  1039,  1039,  1039,
    1039,  1039,  1039,  1039,  1039,  1039,  1039,  1039,  1039,  1039,
    1039,  1039,   141,  1039,   155,  1636,   -33,  1039,    68,   -22,
    1636,   -20,    68,   148,  1702,  1702,  1039,  -277,  -277,    84,
    1039,  1039,  -277,  1039,  1039,  1537,  1039,  1039,  -277,  -277,
    1039,  -277,   151,  1039,  1039,  1039,  1039,  1039,  1039,  1039,
    1039,  1039,  1039,   152,   -13,  -277,  -277,  1039,  -277,    82,
    1702,    83,  1603,  -277,  -277,  -277,  1039,  -277,  1039,  -277,
     984,   984,   984,   -49,   -49,   -49,  1702,   984,   984,  1097,
      24,    24,   -49,   -49,  1702,  1702,  -277,   -32,  -277,  1702,
    -277,  -277,  -277,  1702,   141,  -277,  -277,  1702,  1702,  1702,
    1702,  1039,  1039,  1702,  1702,   141,  1702,  -277,  1702,  1234,
    1702,  1438,  1702,  1471,  1702,  1504,  1702,  1702,  -277,  -277,
    1039,  1702,  -277,    -5,  -277,   842,  -277,   658,  1702,  1702,
     658,   167,    91,   -32,  1702,  1702,   -32,  1039,  1039,  1039,
    1039,    -1,   658,   145,  1039,  -277,  -277,  -277,    -8,   169,
    -277,   174,  -277,    91,    91,  1702,  1702,  1702,  1702,  -277,
     171,  1039,  1702,   -10,  -277,  -277,  -277,   168,  -277,  -277,
     566,  -277,  -277,  -277,  1702,  -277,  -277,  -277,  -277,   168,
    -277,  1265,   842,   658,   168,   168,  -277,  1265,  -277,   658,
     658,  -277,   658,   842,  -277,   147,   154,  -277,   176,  -277,
     178,    63,   658,   179,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,   180,  -277,  -277,   842,  -277,   750,   750,
    -277,  -277,  -277,  -277,   658,   658,  -277,  -277,  -277
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -277,  -277,    18,  -277,  -276,  -277,    85,  -277,   -50,     0,
    -277,  -277,  -277,   -89,  -268,  -108,  -127,  -277,  -277,  -277,
    -264,   -44,    28,  -198,  -277,  -277,     1,    49,  -277,  -277,
    -277,  -277,  -277,   188,   -19,  -277,  -277,  -277,  -277,  -277,
    -277,  -204,  -172,  -277,   -43,     8
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -10
static const yytype_int16 yytable[] =
{
      51,    66,   199,   126,   201,    70,   134,   127,    71,    72,
      74,   270,   109,    77,    74,    80,    74,   173,   176,    67,
     152,    79,    93,    82,   304,    97,   316,   317,   142,   283,
     273,   106,    93,   158,   112,   113,   114,   115,   116,   117,
     162,   276,    76,   327,   122,   123,   124,   125,   305,   284,
     305,   334,   150,   151,   174,   177,   133,   334,   135,   345,
     327,   100,   101,    84,   282,   128,    66,   228,    98,    99,
     154,    85,   157,   271,   155,   167,   102,   125,   230,   155,
     231,   156,   160,   157,   125,   197,   159,   259,   164,   181,
     165,    89,   260,    91,   310,   139,   140,    96,   141,   299,
     103,   142,   175,   104,   197,    66,   196,   197,   323,   203,
     204,    14,   235,   329,   330,   351,   352,   332,   105,   148,
     149,   311,   312,   110,   234,   150,   151,   118,    51,   120,
     133,   200,   202,   342,   121,   245,   129,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   209,   125,   130,   157,   153,   229,   265,
     163,   364,   365,   166,   172,   178,   182,   233,   194,   226,
     193,   237,   238,   197,   239,   240,   232,   243,   244,   247,
     258,   246,   262,   264,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   336,   290,   291,   169,   261,   301,
     341,   308,   309,   313,   316,   305,   347,   268,   349,   269,
     350,   354,   361,   198,   319,   285,   339,   281,   236,   108,
     136,   137,     0,     0,     0,   138,   139,   140,   263,   141,
     170,   171,   142,   143,   144,     0,     0,     0,     0,     0,
       0,     0,   274,   275,   307,   272,     0,   145,   146,   147,
     148,   149,     0,     0,     0,     0,   150,   151,     0,     0,
       0,   125,   322,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   337,   333,     0,     0,     0,     0,   295,   296,
     297,   298,     0,   344,     0,   302,     0,     0,     0,     0,
       0,   293,     0,     0,   294,   288,     0,     0,   289,     0,
       0,   356,   314,   358,     0,     0,   362,   359,   360,     0,
     300,     0,     0,   303,     0,     0,     0,   180,     0,     0,
       0,     0,   133,     0,     0,     0,   136,   137,   133,     0,
     315,   138,   139,   140,     0,   141,     0,     0,   142,   143,
     144,     0,   331,     0,     0,     0,     0,   335,     0,     0,
       0,   338,   340,   145,   146,   147,   148,   149,   363,   363,
     343,     0,   150,   151,     0,   348,     0,     0,     0,     0,
     353,     0,   355,     0,   357,     0,     0,     0,     0,     0,
       0,     0,    -9,     1,     0,     0,     0,     0,     0,     0,
       0,   366,   367,   368,     2,     3,     4,     5,     0,     6,
       7,     8,     9,    10,     0,    11,    12,    13,     0,    14,
      15,    16,     0,    17,     0,    18,     0,     0,     0,    19,
       0,    20,    21,    22,     0,     0,    23,     0,     0,     0,
       0,    24,    25,    26,     0,     0,     0,    27,     0,     0,
       0,    28,    29,    30,    31,     0,    32,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,     0,    39,
      40,    41,    42,     0,     0,    43,    44,     0,     0,     0,
      -9,    45,     0,     0,     0,    46,     2,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,    14,    15,    16,     0,    17,     0,    18,     0,     0,
       0,    19,     0,    20,    21,    22,     0,     0,    23,     0,
       0,     0,     0,    24,    25,    26,     0,     0,     0,    27,
       0,     0,     0,    28,    29,    30,    31,     0,    32,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,     0,     0,    34,    35,    36,    37,    38,     0,
       0,    39,    40,    41,    42,     0,     0,    43,    44,     0,
       0,     0,     0,    45,     0,     0,     0,    46,     2,     3,
       4,     5,     0,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,    14,    68,    16,     0,    17,   328,    18,
       0,     0,     0,    19,     0,    20,    21,    22,     0,     0,
       0,     0,     0,     0,     0,    24,    25,    26,     0,     0,
       0,    27,     0,     0,     0,     0,     0,    30,    31,     0,
      32,     0,     0,     0,     0,     0,     0,     0,     0,    33,
       0,     0,     0,     0,     0,     0,    34,    35,    36,    37,
      38,     0,     0,    39,    40,     0,    42,     0,     0,    43,
      44,     0,     0,     0,    66,    45,     0,     0,     0,    46,
       2,     3,     4,     5,     0,     6,     7,     8,     9,    10,
       0,    11,    12,    13,     0,    14,    68,    16,     0,    17,
       0,    18,     0,     0,     0,    19,     0,    20,    21,    22,
       0,     0,     0,     0,     0,     0,     0,    24,    25,    26,
       0,     0,     0,    27,     0,     0,     0,     0,     0,    30,
      31,     0,    32,     0,     0,     0,     0,     0,     0,     0,
       0,    33,     0,     0,     0,     0,     0,     0,    34,    35,
      36,    37,    38,     0,     0,    39,    40,     0,    42,     0,
       0,    43,    44,     0,     0,     0,    66,    45,     0,     0,
       0,    46,     2,     3,     4,     5,     0,     6,     7,     8,
       9,    10,     0,    11,    12,    13,     0,    14,    68,    16,
       0,    17,     0,    18,     0,     0,     0,    19,     0,    20,
      21,    22,     0,     0,     0,     0,     0,     0,     0,    24,
      25,    26,     0,     0,     0,    27,     0,     0,     0,     0,
       0,    30,    31,     0,    32,     0,     0,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,    39,    40,     0,
      42,     0,     0,    43,    44,     0,     0,     0,     0,    45,
       0,     0,     0,    46,     2,     3,     4,     5,     0,     6,
       7,     8,     9,    10,     0,    11,    12,    13,     0,    14,
      68,    16,   208,    17,     0,    18,     0,     0,     0,    19,
       0,    20,    21,   136,   137,     0,     0,     0,   138,   139,
     140,    24,   141,    26,     0,   142,   143,   144,     0,     0,
       0,     0,     0,    30,    31,     0,    32,     0,     0,     0,
     145,   146,   147,   148,   149,    33,    66,     0,     0,   150,
     151,     0,    34,    35,    36,    37,    38,     0,     0,     0,
      40,     0,    42,     0,     0,    43,    44,     2,     3,     4,
       5,    45,     0,     0,     0,    46,     0,     0,    11,    12,
      13,     0,    14,    68,    16,     0,    17,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,    88,     0,
      89,    90,    91,    92,    69,     0,    26,     0,     2,     3,
       4,     5,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,    14,    68,    16,     0,    17,    33,     0,
       0,     0,     0,     0,    87,    34,    35,    36,    37,    38,
       0,    89,     0,    91,     0,    69,     0,    26,    43,    44,
       0,     0,     0,     0,    45,     0,     0,     0,    46,   107,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    33,
       0,     0,     0,     0,     0,     0,    34,    35,    36,    37,
      38,     2,     3,     4,     5,   139,   140,     0,   141,    43,
      44,   142,    11,    12,    13,    45,    14,    68,    16,    46,
      17,     0,     0,     0,     0,     0,   145,   146,   147,   148,
     149,     0,     0,     0,     0,   150,   151,     0,    69,     0,
      26,     2,     3,     4,     5,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,    14,    68,    16,     0,
      17,     0,    33,     0,     0,     0,     0,     0,     0,    34,
      35,    36,    37,    38,     0,     0,     0,     0,    69,     0,
      26,     0,    43,    44,     0,     0,     0,     0,    45,     0,
       0,     0,    46,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    33,     0,     0,     0,     0,     0,     0,    34,
      35,    36,    37,    38,     2,     3,     4,     5,   139,   140,
       0,   141,    43,    44,   142,    11,    12,    13,    73,    14,
      68,    16,    46,    17,     0,     0,     0,     0,     0,     0,
     146,   147,   148,   149,     0,     0,     0,     0,   150,   151,
       0,    69,     0,    26,     2,     3,     4,     5,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,    14,
      68,    16,     0,    17,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,     0,     0,
       0,    69,     0,    26,     0,    43,    44,     0,     0,     0,
       0,    78,     0,     0,     0,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     2,     3,     4,
       5,   277,     0,     0,     0,    43,    44,     0,    11,    12,
      13,    81,    14,    68,    16,    46,    17,     0,     0,   136,
     137,     0,     0,     0,   138,   139,   140,     0,   141,     0,
       0,   142,   143,   144,    69,     0,    26,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   145,   146,   147,   148,
     149,     0,     0,     0,     0,   150,   151,     0,    33,     0,
       0,     0,     0,     0,     0,    34,    35,    36,    37,    38,
       0,     0,     0,   183,     0,     0,     0,     0,    43,    44,
     184,     0,     0,     0,   131,     0,     0,     0,    46,     0,
       0,   136,   137,     0,     0,     0,   138,   139,   140,     0,
     141,     0,     0,   142,   143,   144,   185,     0,     0,     0,
       0,     0,     0,   186,     0,     0,     0,     0,   145,   146,
     147,   148,   149,     0,   136,   137,     0,   150,   151,   138,
     139,   140,     0,   141,     0,     0,   142,   143,   144,   187,
       0,     0,     0,     0,     0,     0,   188,     0,     0,     0,
       0,   145,   146,   147,   148,   149,     0,   136,   137,     0,
     150,   151,   138,   139,   140,     0,   141,     0,     0,   142,
     143,   144,   189,     0,     0,     0,     0,     0,     0,   190,
       0,     0,     0,     0,   145,   146,   147,   148,   149,     0,
     136,   137,     0,   150,   151,   138,   139,   140,     0,   141,
       0,     0,   142,   143,   144,   278,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   145,   146,   147,
     148,   149,     0,   136,   137,     0,   150,   151,   138,   139,
     140,     0,   141,     0,     0,   142,   143,   144,   279,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     145,   146,   147,   148,   149,     0,   136,   137,     0,   150,
     151,   138,   139,   140,     0,   141,     0,     0,   142,   143,
     144,   280,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   145,   146,   147,   148,   149,     0,   136,
     137,     0,   150,   151,   138,   139,   140,     0,   141,     0,
       0,   142,   143,   144,     0,     0,     0,     0,     0,     0,
       0,   241,     0,     0,     0,     0,   145,   146,   147,   148,
     149,     0,   136,   137,     0,   150,   151,   138,   139,   140,
       0,   141,     0,     0,   142,   143,   144,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   242,   145,
     146,   147,   148,   149,     0,   136,   137,     0,   150,   151,
     138,   139,   140,     0,   141,     0,     0,   142,   143,   144,
       0,     0,     0,     0,     0,   191,   192,     0,     0,     0,
       0,     0,   145,   146,   147,   148,   149,     0,   136,   137,
       0,   150,   151,   138,   139,   140,     0,   141,     0,     0,
     142,   143,   144,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   206,   145,   146,   147,   148,   149,
       0,   136,   137,   195,   150,   151,   138,   139,   140,     0,
     141,     0,     0,   142,   143,   144,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   145,   146,
     147,   148,   149,     0,   136,   137,   195,   150,   151,   138,
     139,   140,     0,   141,     0,     0,   142,   143,   144,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     206,   145,   146,   147,   148,   149,     0,   136,   137,     0,
     150,   151,   138,   139,   140,     0,   141,     0,     0,   142,
     143,   144,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   145,   146,   147,   148,   149,     0,
       0,     0,     0,   150,   151
};

static const yytype_int16 yycheck[] =
{
       0,    98,   129,    46,   131,     4,    56,     0,     7,     8,
       9,   209,    31,    12,    13,    14,    15,    54,    54,     1,
      64,    13,    21,    15,    32,    24,    36,   303,    77,    34,
     234,    30,    31,    76,    33,    34,    35,    36,    37,    38,
      83,   245,    28,   307,    43,    44,    45,    46,    58,    54,
      58,   319,   101,   102,    91,    91,    55,   325,    57,   335,
     324,    13,    14,    50,   262,    47,    98,   100,    56,    57,
      69,    28,   105,   105,    73,    94,    28,    76,   100,    78,
     100,    73,    81,   105,    83,   105,    78,   100,    87,   108,
      89,    45,   105,    47,   292,    71,    72,    28,    74,   100,
      28,    77,   101,    28,   105,    98,   104,   105,   306,    52,
      53,    27,    28,   311,   312,    52,    53,   315,    28,    95,
      96,   293,   294,    28,   168,   101,   102,    28,   128,    28,
     129,   130,   131,   331,    99,   179,    99,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   135,   153,    91,   105,    99,   157,   203,
     105,   359,   360,    45,    28,    52,   105,   166,    28,    28,
     105,   170,   171,   105,   173,   174,    28,   176,   177,    28,
      28,   180,   100,   100,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   321,    28,   105,    42,   197,    54,
     327,    32,    28,    32,    36,    58,    52,   206,    32,   208,
      32,    32,    32,   128,   303,   265,   324,   260,   169,    31,
      65,    66,    -1,    -1,    -1,    70,    71,    72,   200,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   241,   242,   288,   227,    -1,    92,    93,    94,
      95,    96,    -1,    -1,    -1,    -1,   101,   102,    -1,    -1,
      -1,   260,   306,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   322,   317,    -1,    -1,    -1,    -1,   277,   278,
     279,   280,    -1,   333,    -1,   284,    -1,    -1,    -1,    -1,
      -1,   273,    -1,    -1,   276,   267,    -1,    -1,   270,    -1,
      -1,   345,   301,   347,    -1,    -1,   356,   351,   352,    -1,
     282,    -1,    -1,   285,    -1,    -1,    -1,    56,    -1,    -1,
      -1,    -1,   321,    -1,    -1,    -1,    65,    66,   327,    -1,
     302,    70,    71,    72,    -1,    74,    -1,    -1,    77,    78,
      79,    -1,   314,    -1,    -1,    -1,    -1,   319,    -1,    -1,
      -1,   323,   324,    92,    93,    94,    95,    96,   358,   359,
     332,    -1,   101,   102,    -1,   337,    -1,    -1,    -1,    -1,
     342,    -1,   344,    -1,   346,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     0,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   363,   364,   365,    12,    13,    14,    15,    -1,    17,
      18,    19,    20,    21,    -1,    23,    24,    25,    -1,    27,
      28,    29,    -1,    31,    -1,    33,    -1,    -1,    -1,    37,
      -1,    39,    40,    41,    -1,    -1,    44,    -1,    -1,    -1,
      -1,    49,    50,    51,    -1,    -1,    -1,    55,    -1,    -1,
      -1,    59,    60,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    81,    82,    83,    84,    -1,    -1,    87,
      88,    89,    90,    -1,    -1,    93,    94,    -1,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,    12,    13,    14,    15,
      -1,    17,    18,    19,    20,    21,    -1,    23,    24,    25,
      -1,    27,    28,    29,    -1,    31,    -1,    33,    -1,    -1,
      -1,    37,    -1,    39,    40,    41,    -1,    -1,    44,    -1,
      -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    59,    60,    61,    62,    -1,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,    -1,
      -1,    87,    88,    89,    90,    -1,    -1,    93,    94,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,   103,    12,    13,
      14,    15,    -1,    17,    18,    19,    20,    21,    -1,    23,
      24,    25,    -1,    27,    28,    29,    -1,    31,    32,    33,
      -1,    -1,    -1,    37,    -1,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    61,    62,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    -1,    -1,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    -1,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
      12,    13,    14,    15,    -1,    17,    18,    19,    20,    21,
      -1,    23,    24,    25,    -1,    27,    28,    29,    -1,    31,
      -1,    33,    -1,    -1,    -1,    37,    -1,    39,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    61,
      62,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    -1,    -1,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    -1,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,    12,    13,    14,    15,    -1,    17,    18,    19,
      20,    21,    -1,    23,    24,    25,    -1,    27,    28,    29,
      -1,    31,    -1,    33,    -1,    -1,    -1,    37,    -1,    39,
      40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    61,    62,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    -1,    -1,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,   103,    12,    13,    14,    15,    -1,    17,
      18,    19,    20,    21,    -1,    23,    24,    25,    -1,    27,
      28,    29,    54,    31,    -1,    33,    -1,    -1,    -1,    37,
      -1,    39,    40,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    49,    74,    51,    -1,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    61,    62,    -1,    64,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    73,    98,    -1,    -1,   101,
     102,    -1,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      88,    -1,    90,    -1,    -1,    93,    94,    12,    13,    14,
      15,    99,    -1,    -1,    -1,   103,    -1,    -1,    23,    24,
      25,    -1,    27,    28,    29,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    43,    -1,
      45,    46,    47,    48,    49,    -1,    51,    -1,    12,    13,
      14,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,
      24,    25,    -1,    27,    28,    29,    -1,    31,    73,    -1,
      -1,    -1,    -1,    -1,    38,    80,    81,    82,    83,    84,
      -1,    45,    -1,    47,    -1,    49,    -1,    51,    93,    94,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,   103,    63,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    12,    13,    14,    15,    71,    72,    -1,    74,    93,
      94,    77,    23,    24,    25,    99,    27,    28,    29,   103,
      31,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    -1,    -1,    -1,    -1,   101,   102,    -1,    49,    -1,
      51,    12,    13,    14,    15,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    24,    25,    -1,    27,    28,    29,    -1,
      31,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    49,    -1,
      51,    -1,    93,    94,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      81,    82,    83,    84,    12,    13,    14,    15,    71,    72,
      -1,    74,    93,    94,    77,    23,    24,    25,    99,    27,
      28,    29,   103,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    -1,    -1,    -1,    -1,   101,   102,
      -1,    49,    -1,    51,    12,    13,    14,    15,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    23,    24,    25,    -1,    27,
      28,    29,    -1,    31,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    49,    -1,    51,    -1,    93,    94,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    81,    82,    83,    84,    12,    13,    14,
      15,    47,    -1,    -1,    -1,    93,    94,    -1,    23,    24,
      25,    99,    27,    28,    29,   103,    31,    -1,    -1,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    -1,    74,    -1,
      -1,    77,    78,    79,    49,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    -1,    -1,    -1,    -1,   101,   102,    -1,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    47,    -1,    -1,    -1,    -1,    93,    94,
      54,    -1,    -1,    -1,    99,    -1,    -1,    -1,   103,    -1,
      -1,    65,    66,    -1,    -1,    -1,    70,    71,    72,    -1,
      74,    -1,    -1,    77,    78,    79,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    -1,    65,    66,    -1,   101,   102,    70,
      71,    72,    -1,    74,    -1,    -1,    77,    78,    79,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    -1,    65,    66,    -1,
     101,   102,    70,    71,    72,    -1,    74,    -1,    -1,    77,
      78,    79,    47,    -1,    -1,    -1,    -1,    -1,    -1,    54,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    -1,
      65,    66,    -1,   101,   102,    70,    71,    72,    -1,    74,
      -1,    -1,    77,    78,    79,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    -1,    65,    66,    -1,   101,   102,    70,    71,
      72,    -1,    74,    -1,    -1,    77,    78,    79,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    -1,    65,    66,    -1,   101,
     102,    70,    71,    72,    -1,    74,    -1,    -1,    77,    78,
      79,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    -1,    65,
      66,    -1,   101,   102,    70,    71,    72,    -1,    74,    -1,
      -1,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    -1,    65,    66,    -1,   101,   102,    70,    71,    72,
      -1,    74,    -1,    -1,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    -1,    65,    66,    -1,   101,   102,
      70,    71,    72,    -1,    74,    -1,    -1,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    85,    86,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    -1,    65,    66,
      -1,   101,   102,    70,    71,    72,    -1,    74,    -1,    -1,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    93,    94,    95,    96,
      -1,    65,    66,   100,   101,   102,    70,    71,    72,    -1,
      74,    -1,    -1,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    -1,    65,    66,   100,   101,   102,    70,
      71,    72,    -1,    74,    -1,    -1,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    93,    94,    95,    96,    -1,    65,    66,    -1,
     101,   102,    70,    71,    72,    -1,    74,    -1,    -1,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    -1,
      -1,    -1,    -1,   101,   102
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    13,    14,    15,    17,    18,    19,    20,
      21,    23,    24,    25,    27,    28,    29,    31,    33,    37,
      39,    40,    41,    44,    49,    50,    51,    55,    59,    60,
      61,    62,    64,    73,    80,    81,    82,    83,    84,    87,
      88,    89,    90,    93,    94,    99,   103,   107,   112,   113,
     114,   115,   116,   123,   124,   125,   130,   131,   132,   133,
     134,   138,   141,   143,   146,   149,    98,   108,    28,    49,
     132,   132,   132,    99,   132,   151,    28,   132,    99,   151,
     132,    99,   151,   142,    50,    28,   135,    38,    43,    45,
      46,    47,    48,   132,   139,   140,    28,   132,    56,    57,
      13,    14,    28,    28,    28,    28,   132,    63,   139,   140,
      28,   137,   132,   132,   132,   132,   132,   132,    28,   136,
      28,    99,   132,   132,   132,   132,   150,     0,   108,    99,
      91,    99,   122,   132,   114,   132,    65,    66,    70,    71,
      72,    74,    77,    78,    79,    92,    93,    94,    95,    96,
     101,   102,   127,    99,   132,   132,   151,   105,   150,   151,
     132,   150,   150,   105,   132,   132,    45,   140,   144,    42,
      75,    76,    28,    54,    91,   132,    54,    91,    52,   145,
      56,   140,   105,    47,    54,    47,    54,    47,    54,    47,
      54,    85,    86,   105,    28,   100,   104,   105,   112,   122,
     132,   122,   132,    52,    53,   109,    91,   128,    54,   108,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,    28,   147,   100,   132,
     100,   100,    28,   132,   127,    28,   133,   132,   132,   132,
     132,    54,    91,   132,   132,   127,   132,    28,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,    28,   100,
     105,   132,   100,   128,   100,   127,   127,   129,   132,   132,
     129,   105,   108,   147,   132,   132,   147,    47,    47,    47,
      47,   150,   129,    34,    54,   114,   108,   115,   128,   128,
      28,   105,   148,   108,   108,   132,   132,   132,   132,   100,
     128,    54,   132,   128,    32,    58,   110,   127,    32,    28,
     129,   148,   148,    32,   132,   128,    36,   110,   111,   119,
     120,   126,   127,   129,   118,   119,   121,   126,    32,   129,
     129,   128,   129,   127,   120,   128,   122,   114,   128,   121,
     128,   122,   129,   128,   114,   110,   117,    52,   128,    32,
      32,    52,    53,   128,    32,   128,   127,   128,   127,   127,
     127,    32,   114,   115,   129,   129,   128,   128,   128
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
	    /* Fall through.  */
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

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
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
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
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
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

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
    { yyerrok; ;}
    break;

  case 5:
#line 125 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1;
	;}
    break;

  case 6:
#line 130 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1;
	;}
    break;

  case 7:
#line 135 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 5;
	;}
    break;

  case 8:
#line 140 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 8;
	;}
    break;

  case 13:
#line 151 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 14:
#line 157 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_assign);
			(yyval.code) = (yyvsp[(2) - (4)].code); ;}
    break;

  case 15:
#line 160 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_after); ;}
    break;

  case 16:
#line 161 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_before); ;}
    break;

  case 17:
#line 162 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 18:
#line 168 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[1]);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 19:
#line 175 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[1]);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 20:
#line 181 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 21:
#line 187 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[1]);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 22:
#line 194 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[1]);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 27:
#line 212 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(5) - (7)].code) - (yyvsp[(1) - (7)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (7)].code) - (yyvsp[(1) - (7)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 2] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(7) - (7)].s), "repeat", true); ;}
    break;

  case 28:
#line 225 "engines/director/lingo/lingo-gr.y"
    {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[(3) - (10)].code) - (yyvsp[(1) - (10)].code));
		WRITE_UINT32(&finish, (yyvsp[(6) - (10)].code) - (yyvsp[(1) - (10)].code));
		WRITE_UINT32(&body, (yyvsp[(8) - (10)].code) - (yyvsp[(1) - (10)].code));
		WRITE_UINT32(&end, (yyvsp[(9) - (10)].code) - (yyvsp[(1) - (10)].code));
		WRITE_UINT32(&inc, 1);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 1] = init;	/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 2] = finish;/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 3] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 4] = inc;	/* increment */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 5] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(10) - (10)].s), "repeat", true); ;}
    break;

  case 29:
#line 243 "engines/director/lingo/lingo-gr.y"
    {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[(3) - (11)].code) - (yyvsp[(1) - (11)].code));
		WRITE_UINT32(&finish, (yyvsp[(7) - (11)].code) - (yyvsp[(1) - (11)].code));
		WRITE_UINT32(&body, (yyvsp[(9) - (11)].code) - (yyvsp[(1) - (11)].code));
		WRITE_UINT32(&end, (yyvsp[(10) - (11)].code) - (yyvsp[(1) - (11)].code));
		WRITE_UINT32(&inc, (uint32)-1);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 1] = init;	/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 2] = finish;/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 3] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 4] = inc;	/* increment */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 5] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(11) - (11)].s), "repeat", true); ;}
    break;

  case 30:
#line 257 "engines/director/lingo/lingo-gr.y"
    {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[(3) - (3)].code) - (yyvsp[(1) - (3)].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (3)].code) + 1] = end; ;}
    break;

  case 31:
#line 262 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
			checkEnd((yyvsp[(6) - (6)].s), "tell", true); ;}
    break;

  case 32:
#line 265 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		;}
    break;

  case 33:
#line 270 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code) - (yyvsp[(1) - (6)].code));
		WRITE_UINT32(&end, (yyvsp[(5) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(6) - (6)].s), "if", true);

		g_lingo->processIf(0, 0); ;}
    break;

  case 34:
#line 280 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&else1, (yyvsp[(7) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&end, (yyvsp[(8) - (9)].code) - (yyvsp[(1) - (9)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(9) - (9)].s), "if", true);

		g_lingo->processIf(0, 0); ;}
    break;

  case 35:
#line 292 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&end, (yyvsp[(8) - (9)].code) - (yyvsp[(1) - (9)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(9) - (9)].s), "if", true);

		g_lingo->processIf(0, (yyvsp[(8) - (9)].code) - (yyvsp[(1) - (9)].code)); ;}
    break;

  case 36:
#line 304 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&else1, (yyvsp[(7) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&end, (yyvsp[(9) - (9)].code) - (yyvsp[(1) - (9)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf(0, (yyvsp[(9) - (9)].code) - (yyvsp[(1) - (9)].code)); ;}
    break;

  case 37:
#line 314 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code) - (yyvsp[(1) - (6)].code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (yyvsp[(6) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, 0); ;}
    break;

  case 38:
#line 324 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (10)].code) - (yyvsp[(1) - (10)].code));
		WRITE_UINT32(&else1, (yyvsp[(8) - (10)].code) - (yyvsp[(1) - (10)].code));
		WRITE_UINT32(&end, (yyvsp[(10) - (10)].code) - (yyvsp[(1) - (10)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, 0); ;}
    break;

  case 39:
#line 334 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (10)].code) - (yyvsp[(1) - (10)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (10)].code) - (yyvsp[(1) - (10)].code));
		WRITE_UINT32(&end, (yyvsp[(10) - (10)].code) - (yyvsp[(1) - (10)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, (yyvsp[(10) - (10)].code) - (yyvsp[(1) - (10)].code)); ;}
    break;

  case 40:
#line 346 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 41:
#line 347 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 46:
#line 358 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 48:
#line 367 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 49:
#line 373 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 50:
#line 381 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 51:
#line 382 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 53:
#line 386 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 54:
#line 389 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 55:
#line 396 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 56:
#line 403 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 57:
#line 411 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 58:
#line 414 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 59:
#line 417 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 62:
#line 422 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 63:
#line 428 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_tellcode);
		g_lingo->code1(STOP); ;}
    break;

  case 64:
#line 432 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt((yyvsp[(1) - (1)].i)); ;}
    break;

  case 65:
#line 435 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_floatpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 66:
#line 438 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 67:
#line 441 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 69:
#line 445 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 70:
#line 448 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 71:
#line 451 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 72:
#line 452 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg)); ;}
    break;

  case 73:
#line 453 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 74:
#line 456 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 75:
#line 460 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 76:
#line 468 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 78:
#line 475 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 79:
#line 476 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 80:
#line 477 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 81:
#line 478 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 82:
#line 479 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); ;}
    break;

  case 83:
#line 480 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 84:
#line 481 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 85:
#line 482 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 86:
#line 483 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 87:
#line 484 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 88:
#line 485 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 89:
#line 486 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 90:
#line 487 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 91:
#line 488 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 92:
#line 489 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 93:
#line 490 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 94:
#line 491 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 95:
#line 492 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 96:
#line 493 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 97:
#line 494 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 98:
#line 495 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_arraypush); g_lingo->codeArray((yyvsp[(2) - (3)].narg)); ;}
    break;

  case 99:
#line 496 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 100:
#line 497 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 101:
#line 498 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); ;}
    break;

  case 102:
#line 499 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); ;}
    break;

  case 103:
#line 500 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); ;}
    break;

  case 104:
#line 501 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); ;}
    break;

  case 105:
#line 502 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); ;}
    break;

  case 106:
#line 503 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); ;}
    break;

  case 107:
#line 504 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); ;}
    break;

  case 108:
#line 505 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); ;}
    break;

  case 109:
#line 508 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 110:
#line 513 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 113:
#line 516 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); ;}
    break;

  case 114:
#line 517 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 118:
#line 521 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 119:
#line 524 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 120:
#line 527 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 121:
#line 530 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_voidpush);
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 1);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 122:
#line 534 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 123:
#line 535 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg)); ;}
    break;

  case 124:
#line 536 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 125:
#line 537 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].narg)); ;}
    break;

  case 126:
#line 538 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 127:
#line 539 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 128:
#line 540 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[(1) - (3)].s)); s += '-'; s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 129:
#line 543 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 130:
#line 544 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 131:
#line 547 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 132:
#line 548 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 133:
#line 551 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 134:
#line 552 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 135:
#line 563 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 136:
#line 564 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 137:
#line 565 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 138:
#line 566 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 139:
#line 570 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 140:
#line 574 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 145:
#line 588 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 146:
#line 589 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 147:
#line 593 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 148:
#line 597 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 149:
#line 601 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); ;}
    break;

  case 150:
#line 601 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[(1) - (3)].s), (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 151:
#line 631 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 152:
#line 632 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 153:
#line 636 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFactory(*(yyvsp[(2) - (2)].s)); ;}
    break;

  case 154:
#line 637 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 155:
#line 638 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 156:
#line 642 "engines/director/lingo/lingo-gr.y"
    {	// D3
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(yyvsp[(1) - (7)].s), (yyvsp[(2) - (7)].code), (yyvsp[(3) - (7)].narg));
		g_lingo->_indef = false;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[(7) - (7)].s), (yyvsp[(1) - (7)].s)->c_str(), false); ;}
    break;

  case 157:
#line 649 "engines/director/lingo/lingo-gr.y"
    {	// D4. No 'end' clause
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(yyvsp[(1) - (6)].s), (yyvsp[(2) - (6)].code), (yyvsp[(3) - (6)].narg));
		g_lingo->_indef = false;
		g_lingo->_ignoreMe = false; ;}
    break;

  case 158:
#line 655 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); g_lingo->_indef = true; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; ;}
    break;

  case 159:
#line 657 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 160:
#line 658 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 161:
#line 659 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 162:
#line 660 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 163:
#line 663 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 164:
#line 667 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (2)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(2) - (2)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 165:
#line 675 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 166:
#line 676 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 167:
#line 677 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 168:
#line 680 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 169:
#line 681 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3123 "engines/director/lingo/lingo-gr.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
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

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
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

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 684 "engines/director/lingo/lingo-gr.y"


