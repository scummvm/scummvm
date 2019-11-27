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
#define YYLAST   1834

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  168
/* YYNRULES -- Number of states.  */
#define YYNSTATES  365

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
     233,   237,   241,   244,   248,   250,   252,   253,   254,   256,
     259,   262,   266,   268,   270,   272,   274,   276,   278,   280,
     283,   286,   291,   296,   298,   300,   303,   305,   309,   313,
     317,   321,   325,   329,   333,   337,   341,   345,   349,   353,
     356,   360,   364,   368,   372,   375,   378,   382,   386,   391,
     396,   401,   408,   413,   420,   425,   432,   437,   444,   447,
     450,   452,   454,   457,   459,   462,   465,   468,   470,   473,
     476,   478,   481,   486,   491,   498,   503,   506,   510,   512,
     516,   518,   522,   524,   528,   531,   534,   537,   540,   544,
     547,   550,   552,   556,   559,   562,   565,   569,   572,   573,
     577,   578,   587,   590,   591,   600,   608,   615,   618,   619,
     621,   625,   630,   631,   634,   635,   637,   641,   643
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
      -1,   132,    -1,   132,    91,   132,    -1,    99,   122,   100,
      -1,    50,    57,    -1,    50,    56,    28,    -1,    41,    -1,
     111,    -1,    -1,    -1,   127,    -1,   129,   108,    -1,   129,
     115,    -1,    55,    28,    52,    -1,    87,    -1,    12,    -1,
      15,    -1,    31,    -1,    29,    -1,   133,    -1,    23,    -1,
      24,   132,    -1,    25,   151,    -1,    25,    99,   151,   100,
      -1,    28,    99,   150,   100,    -1,    28,    -1,    13,    -1,
      14,   132,    -1,   113,    -1,   132,    93,   132,    -1,   132,
      94,   132,    -1,   132,    95,   132,    -1,   132,    96,   132,
      -1,   132,    74,   132,    -1,   132,   101,   132,    -1,   132,
     102,   132,    -1,   132,    70,   132,    -1,   132,    65,   132,
      -1,   132,    66,   132,    -1,   132,    71,   132,    -1,   132,
      72,   132,    -1,    73,   132,    -1,   132,    92,   132,    -1,
     132,    77,   132,    -1,   132,    78,   132,    -1,   132,    79,
     132,    -1,    93,   132,    -1,    94,   132,    -1,    99,   132,
     100,    -1,   103,   150,   104,    -1,    84,   132,    85,   132,
      -1,    84,   132,    86,   132,    -1,    80,   132,    47,   132,
      -1,    80,   132,    54,   132,    47,   132,    -1,    81,   132,
      47,   132,    -1,    81,   132,    54,   132,    47,   132,    -1,
      82,   132,    47,   132,    -1,    82,   132,    54,   132,    47,
     132,    -1,    83,   132,    47,   132,    -1,    83,   132,    54,
     132,    47,   132,    -1,    27,   132,    -1,    49,   132,    -1,
     138,    -1,   141,    -1,    37,    50,    -1,    37,    -1,    39,
     135,    -1,    88,   136,    -1,    64,   137,    -1,    17,    -1,
      19,   132,    -1,    18,   132,    -1,    18,    -1,    20,   151,
      -1,    20,    99,   151,   100,    -1,    90,    99,    28,   100,
      -1,    90,    99,    28,   105,   150,   100,    -1,    61,   132,
      56,   132,    -1,    61,   132,    -1,    21,    28,   150,    -1,
      28,    -1,   135,   105,    28,    -1,    28,    -1,   136,   105,
      28,    -1,    28,    -1,   137,   105,    28,    -1,    40,    43,
      -1,    40,    46,    -1,    40,    48,    -1,    40,   139,    -1,
      40,   139,   140,    -1,    40,   140,    -1,    38,   132,    -1,
     132,    -1,    47,    45,   132,    -1,    45,   132,    -1,    62,
      63,    -1,    62,   139,    -1,    62,   139,   140,    -1,    62,
     140,    -1,    -1,    33,   142,   150,    -1,    -1,    44,    28,
     144,   127,   147,   108,   148,   129,    -1,    59,    28,    -1,
      -1,    60,    28,   145,   127,   147,   108,   148,   129,    -1,
     146,   127,   147,   108,   148,   129,    32,    -1,   146,   127,
     147,   108,   148,   129,    -1,    89,    28,    -1,    -1,    28,
      -1,   147,   105,    28,    -1,   147,   108,   105,    28,    -1,
      -1,    28,   151,    -1,    -1,   132,    -1,   150,   105,   132,
      -1,   132,    -1,   151,   105,   132,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   120,   120,   121,   122,   125,   130,   135,   140,   145,
     146,   147,   148,   151,   157,   160,   161,   162,   168,   175,
     181,   187,   194,   202,   203,   206,   207,   212,   225,   243,
     257,   263,   266,   271,   281,   293,   305,   315,   325,   335,
     347,   348,   351,   352,   355,   356,   359,   367,   368,   376,
     377,   378,   381,   384,   391,   398,   406,   409,   412,   413,
     414,   417,   423,   427,   430,   433,   436,   439,   440,   443,
     446,   447,   448,   451,   455,   463,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   503,   508,
     509,   510,   511,   512,   513,   514,   515,   516,   519,   522,
     525,   529,   530,   531,   532,   533,   534,   535,   538,   539,
     542,   543,   546,   547,   558,   559,   560,   561,   565,   569,
     575,   576,   579,   580,   583,   584,   588,   592,   596,   596,
     626,   626,   631,   634,   634,   639,   647,   654,   656,   657,
     658,   659,   662,   666,   674,   675,   676,   679,   680
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
     117,   117,   118,   118,   119,   119,   120,   121,   121,   122,
     122,   122,   123,   124,   125,   126,   127,   128,   129,   129,
     129,   130,   131,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   133,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   135,   135,
     136,   136,   137,   137,   138,   138,   138,   138,   138,   138,
     139,   139,   140,   140,   141,   141,   141,   141,   142,   141,
     144,   143,   143,   145,   143,   143,   143,   146,   147,   147,
     147,   147,   148,   149,   150,   150,   150,   151,   151
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     1,     1,     1,     0,
       1,     1,     1,     4,     4,     4,     4,     4,     4,     5,
       4,     4,     5,     1,     1,     1,     1,     7,    10,    11,
       3,     6,     4,     6,     9,     9,     9,     6,    10,    10,
       0,     3,     2,     1,     2,     1,     6,     1,     6,     1,
       3,     3,     2,     3,     1,     1,     0,     0,     1,     2,
       2,     3,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     4,     4,     1,     1,     2,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     2,     2,     3,     3,     4,     4,
       4,     6,     4,     6,     4,     6,     4,     6,     2,     2,
       1,     1,     2,     1,     2,     2,     2,     1,     2,     2,
       1,     2,     4,     4,     6,     4,     2,     3,     1,     3,
       1,     3,     1,     3,     2,     2,     2,     2,     3,     2,
       2,     1,     3,     2,     2,     2,     3,     2,     0,     3,
       0,     8,     2,     0,     8,     7,     6,     2,     0,     1,
       3,     4,     0,     2,     0,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    63,    74,     0,    64,   117,   120,     0,     0,
       0,    68,     0,     0,     0,    73,    66,    65,   148,   113,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    62,
       0,     0,     0,     0,     0,     0,   164,     0,     3,    76,
      25,    12,    26,     0,     0,     0,     0,     0,    23,    67,
      24,   110,   111,    10,    56,    11,     5,     4,    73,     0,
      75,   119,   118,     0,   167,   121,   164,    69,     0,    70,
     108,   164,   163,   164,   112,   128,   114,     0,   134,     0,
     135,     0,   136,   141,   137,   139,   150,   109,     0,    52,
       0,     0,     0,     0,   152,   153,   126,   144,   145,   147,
     132,   116,    89,     0,     0,     0,     0,     0,   130,   115,
     157,     0,    94,    95,     0,   165,     0,     1,     9,     0,
       0,     0,     0,    49,    57,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   158,   164,     0,   167,     0,     0,   127,     0,
     165,     0,   149,     0,   140,   143,     0,   138,    56,     0,
       0,     0,    53,     0,     0,     0,     0,     0,    61,    56,
       0,   146,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    96,    97,     0,     2,     0,
      57,     0,     0,    56,     6,    56,     0,    30,     0,    56,
      85,    86,    84,    87,    88,    81,    91,    92,    93,    90,
      77,    78,    79,    80,    82,    83,   159,     0,   122,   168,
      71,    72,   129,   142,   158,    13,    14,    15,    16,    21,
      18,     0,     0,    20,    17,   158,   125,   133,   100,     0,
     102,     0,   104,     0,   106,     0,    98,    99,   131,   123,
     164,   166,    56,     0,    51,     0,    58,    57,    50,    32,
      57,     0,   162,     0,    22,    19,     0,     0,     0,     0,
       0,     0,    57,     0,     0,    57,    59,    60,    56,     0,
     160,     0,    56,   162,   162,   101,   103,   105,   107,   124,
       0,     0,    57,    37,    33,     7,    56,     0,    31,   161,
     156,    56,    56,    27,    57,    56,     8,    56,    55,    57,
      45,     0,    58,    57,    57,    47,    43,     0,   155,   151,
     154,    56,    57,     0,    44,    40,     0,    57,     0,    42,
       0,     0,    57,     0,    57,    56,    57,    56,    36,    34,
      35,    56,     0,    28,    38,     0,    39,     0,    56,    29,
      41,    57,    57,    46,    48
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
     379,   -97,  -277,  -277,  1036,  -277,  -277,  1036,  1036,  1076,
      14,  -277,  1036,  1149,  1036,  1189,  -277,  -277,  -277,    13,
      43,   922,  -277,    69,  1036,    12,    48,    72,    75,    90,
    1036,   963,    93,  1036,  1036,  1036,  1036,  1036,  1036,  -277,
      94,    95,    30,  1036,  1036,  1036,  1036,     7,  -277,  -277,
    -277,  -277,  -277,    35,    36,  1262,   839,  1036,  1732,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,    37,  1036,
    1732,  1732,  1732,  1036,  1732,    50,  1036,  1732,  1036,    50,
    1732,  1036,    50,  1036,  -277,  -277,    51,  1036,  -277,  1036,
    -277,   112,  -277,  1732,    46,  -277,  -277,   155,   133,  -277,
     -37,  1036,   -36,   110,  -277,  -277,  1567,  -277,    46,  -277,
    -277,    58,   -49,  1303,  1336,  1369,  1402,  1600,  -277,    59,
    -277,   137,   -49,   -49,  1666,  1732,     2,  -277,   471,  1262,
    1036,  1262,    57,  1699,  -277,   815,  1036,  1036,  1036,  1036,
    1036,  1036,  1036,  1036,  1036,  1036,  1036,  1036,  1036,  1036,
    1036,  1036,   138,  1036,   155,  1666,   -33,  1036,    63,   -22,
    1666,   -20,    63,   141,  1732,  1732,  1036,  -277,  -277,    84,
    1036,  1036,  -277,  1036,  1036,  1534,  1036,  1036,  -277,  -277,
    1036,  -277,   142,  1036,  1036,  1036,  1036,  1036,  1036,  1036,
    1036,  1036,  1036,   145,   -13,  -277,  -277,  1036,  -277,    76,
    1732,    79,  1633,  -277,  -277,  -277,  1036,  -277,  1036,  -277,
     243,   243,   243,   -49,   -49,   -49,  1732,   243,   243,   282,
      24,    24,   -49,   -49,  1732,  1732,  -277,   -32,  -277,  1732,
    -277,  -277,  -277,  1732,   138,  -277,  -277,  1732,  1732,  1732,
    1732,  1036,  1036,  1732,  1732,   138,  1732,  -277,  1732,  1231,
    1732,  1435,  1732,  1468,  1732,  1501,  1732,  1732,  -277,  -277,
    1036,  1732,  -277,    -5,  -277,   839,  -277,   655,  1732,  1732,
     655,   152,    77,   -32,  1732,  1732,   -32,  1036,  1036,  1036,
    1036,    -1,   655,   129,  1036,  -277,  -277,  -277,    -8,   163,
    -277,   168,  -277,    77,    77,  1732,  1732,  1732,  1732,  -277,
     167,  1036,  1732,   -10,  -277,  -277,  -277,   165,  -277,  -277,
     563,  -277,  -277,  -277,  1732,  -277,  -277,  -277,  -277,   165,
    -277,  1262,   839,   655,   165,   165,  -277,  1262,  -277,   655,
     655,  -277,   655,   839,  -277,   144,   151,  -277,   172,  -277,
     173,   154,   655,   176,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,   178,  -277,  -277,   839,  -277,   747,   747,  -277,
    -277,  -277,   655,  -277,  -277
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -277,  -277,    18,  -277,  -276,  -277,    83,  -277,   -50,     0,
    -277,  -277,  -277,   -91,  -268,  -111,  -127,  -277,  -277,  -277,
    -264,   -44,    28,  -198,  -277,  -277,     1,    45,  -277,  -277,
    -277,  -277,  -277,   185,   -19,  -277,  -277,  -277,  -277,  -277,
    -277,  -204,  -178,  -277,   -43,     8
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
     204,    14,   235,   329,   330,   311,   312,   332,   105,   148,
     149,   110,   118,   120,   234,   150,   151,   130,    51,   121,
     133,   200,   202,   342,   129,   245,   153,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   209,   125,   157,   163,   166,   229,   265,
     362,   172,   178,   182,   193,   194,   226,   233,   197,   232,
     247,   237,   238,   258,   239,   240,   262,   243,   244,   264,
     290,   246,   291,   301,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   336,   308,   309,   169,   261,   313,
     341,   316,   305,   347,   349,   350,   351,   268,   353,   269,
     359,   198,   319,   339,   236,   285,   108,   281,     0,     0,
     136,   137,     0,     0,     0,   138,   139,   140,   263,   141,
     170,   171,   142,   143,   144,     0,     0,     0,     0,     0,
       0,     0,   274,   275,   307,   272,     0,   145,   146,   147,
     148,   149,     0,     0,     0,     0,   150,   151,     0,     0,
       0,   125,   322,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   337,   333,     0,     0,     0,     0,   295,   296,
     297,   298,     0,   344,     0,   302,     0,     0,     0,     0,
       0,   293,     0,     0,   294,   288,     0,     0,   289,     0,
       0,   355,   314,   357,     0,   360,     0,   358,     0,     0,
     300,     0,     0,   303,   139,   140,     0,   141,     0,     0,
     142,     0,   133,     0,     0,     0,     0,     0,   133,     0,
     315,     0,     0,     0,     0,   145,   146,   147,   148,   149,
       0,     0,   331,     0,   150,   151,     0,   335,     0,     0,
       0,   338,   340,   139,   140,     0,   141,   361,   361,   142,
     343,     0,     0,     0,     0,   348,     0,     0,     0,     0,
     352,     0,   354,     0,   356,   146,   147,   148,   149,    -9,
       1,     0,     0,   150,   151,     0,     0,     0,     0,   363,
     364,     2,     3,     4,     5,     0,     6,     7,     8,     9,
      10,     0,    11,    12,    13,     0,    14,    15,    16,     0,
      17,     0,    18,     0,     0,     0,    19,     0,    20,    21,
      22,     0,     0,    23,     0,     0,     0,     0,    24,    25,
      26,     0,     0,     0,    27,     0,     0,     0,    28,    29,
      30,    31,     0,    32,     0,     0,     0,     0,     0,     0,
       0,     0,    33,     0,     0,     0,     0,     0,     0,    34,
      35,    36,    37,    38,     0,     0,    39,    40,    41,    42,
       0,     0,    43,    44,     0,     0,     0,    -9,    45,     0,
       0,     0,    46,     2,     3,     4,     5,     0,     6,     7,
       8,     9,    10,     0,    11,    12,    13,     0,    14,    15,
      16,     0,    17,     0,    18,     0,     0,     0,    19,     0,
      20,    21,    22,     0,     0,    23,     0,     0,     0,     0,
      24,    25,    26,     0,     0,     0,    27,     0,     0,     0,
      28,    29,    30,    31,     0,    32,     0,     0,     0,     0,
       0,     0,     0,     0,    33,     0,     0,     0,     0,     0,
       0,    34,    35,    36,    37,    38,     0,     0,    39,    40,
      41,    42,     0,     0,    43,    44,     0,     0,     0,     0,
      45,     0,     0,     0,    46,     2,     3,     4,     5,     0,
       6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
      14,    68,    16,     0,    17,   328,    18,     0,     0,     0,
      19,     0,    20,    21,    22,     0,     0,     0,     0,     0,
       0,     0,    24,    25,    26,     0,     0,     0,    27,     0,
       0,     0,     0,     0,    30,    31,     0,    32,     0,     0,
       0,     0,     0,     0,     0,     0,    33,     0,     0,     0,
       0,     0,     0,    34,    35,    36,    37,    38,     0,     0,
      39,    40,     0,    42,     0,     0,    43,    44,     0,     0,
       0,    66,    45,     0,     0,     0,    46,     2,     3,     4,
       5,     0,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,    14,    68,    16,     0,    17,     0,    18,     0,
       0,     0,    19,     0,    20,    21,    22,     0,     0,     0,
       0,     0,     0,     0,    24,    25,    26,     0,     0,     0,
      27,     0,     0,     0,     0,     0,    30,    31,     0,    32,
       0,     0,     0,     0,     0,     0,     0,     0,    33,     0,
       0,     0,     0,     0,     0,    34,    35,    36,    37,    38,
       0,     0,    39,    40,     0,    42,     0,     0,    43,    44,
       0,     0,     0,    66,    45,     0,     0,     0,    46,     2,
       3,     4,     5,     0,     6,     7,     8,     9,    10,     0,
      11,    12,    13,     0,    14,    68,    16,     0,    17,     0,
      18,     0,     0,     0,    19,     0,    20,    21,    22,     0,
       0,     0,     0,     0,     0,     0,    24,    25,    26,     0,
       0,     0,    27,     0,     0,     0,     0,     0,    30,    31,
       0,    32,     0,     0,     0,     0,     0,     0,     0,     0,
      33,     0,     0,     0,     0,     0,     0,    34,    35,    36,
      37,    38,     0,     0,    39,    40,     0,    42,     0,     0,
      43,    44,     0,     0,     0,     0,    45,     0,     0,     0,
      46,     2,     3,     4,     5,     0,     6,     7,     8,     9,
      10,     0,    11,    12,    13,     0,    14,    68,    16,   208,
      17,     0,    18,     0,     0,     0,    19,     0,    20,    21,
     136,   137,     0,     0,     0,   138,   139,   140,    24,   141,
      26,     0,   142,   143,   144,     0,     0,     0,     0,     0,
      30,    31,     0,    32,     0,     0,     0,   145,   146,   147,
     148,   149,    33,    66,     0,     0,   150,   151,     0,    34,
      35,    36,    37,    38,     0,     0,     0,    40,     0,    42,
       0,     0,    43,    44,     2,     3,     4,     5,    45,     0,
       0,     0,    46,     0,     0,    11,    12,    13,     0,    14,
      68,    16,     0,    17,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,    88,     0,    89,    90,    91,
      92,    69,     0,    26,     0,     2,     3,     4,     5,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
      14,    68,    16,     0,    17,    33,     0,     0,     0,     0,
       0,    87,    34,    35,    36,    37,    38,     0,    89,     0,
      91,     0,    69,     0,    26,    43,    44,     0,     0,     0,
       0,    45,     0,     0,     0,    46,   107,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    33,     0,     0,     0,
       0,     0,     0,    34,    35,    36,    37,    38,     2,     3,
       4,     5,     0,     0,     0,     0,    43,    44,     0,    11,
      12,    13,    45,    14,    68,    16,    46,    17,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    69,     0,    26,     2,     3,
       4,     5,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,    14,    68,    16,     0,    17,     0,    33,
       0,     0,     0,     0,     0,     0,    34,    35,    36,    37,
      38,     0,     0,     0,     0,    69,     0,    26,     0,    43,
      44,     0,     0,     0,     0,    45,     0,     0,     0,    46,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    33,
       0,     0,     0,     0,     0,     0,    34,    35,    36,    37,
      38,     2,     3,     4,     5,     0,     0,     0,     0,    43,
      44,     0,    11,    12,    13,    73,    14,    68,    16,    46,
      17,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,     0,
      26,     2,     3,     4,     5,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,    14,    68,    16,     0,
      17,     0,    33,     0,     0,     0,     0,     0,     0,    34,
      35,    36,    37,    38,     0,     0,     0,     0,    69,     0,
      26,     0,    43,    44,     0,     0,     0,     0,    78,     0,
       0,     0,    46,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    33,     0,     0,     0,     0,     0,     0,    34,
      35,    36,    37,    38,     2,     3,     4,     5,   277,     0,
       0,     0,    43,    44,     0,    11,    12,    13,    81,    14,
      68,    16,    46,    17,     0,     0,   136,   137,     0,     0,
       0,   138,   139,   140,     0,   141,     0,     0,   142,   143,
     144,    69,     0,    26,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   145,   146,   147,   148,   149,     0,     0,
       0,     0,   150,   151,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,     0,     0,
     183,     0,     0,     0,     0,    43,    44,   184,     0,     0,
       0,   131,     0,     0,     0,    46,     0,     0,   136,   137,
       0,     0,     0,   138,   139,   140,     0,   141,     0,     0,
     142,   143,   144,   185,     0,     0,     0,     0,     0,     0,
     186,     0,     0,     0,     0,   145,   146,   147,   148,   149,
       0,   136,   137,     0,   150,   151,   138,   139,   140,     0,
     141,     0,     0,   142,   143,   144,   187,     0,     0,     0,
       0,     0,     0,   188,     0,     0,     0,     0,   145,   146,
     147,   148,   149,     0,   136,   137,     0,   150,   151,   138,
     139,   140,     0,   141,     0,     0,   142,   143,   144,   189,
       0,     0,     0,     0,     0,     0,   190,     0,     0,     0,
       0,   145,   146,   147,   148,   149,     0,   136,   137,     0,
     150,   151,   138,   139,   140,     0,   141,     0,     0,   142,
     143,   144,   278,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   145,   146,   147,   148,   149,     0,
     136,   137,     0,   150,   151,   138,   139,   140,     0,   141,
       0,     0,   142,   143,   144,   279,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   145,   146,   147,
     148,   149,     0,   136,   137,     0,   150,   151,   138,   139,
     140,     0,   141,     0,     0,   142,   143,   144,   280,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     145,   146,   147,   148,   149,     0,   136,   137,     0,   150,
     151,   138,   139,   140,     0,   141,     0,     0,   142,   143,
     144,     0,     0,     0,     0,     0,     0,     0,   241,     0,
       0,     0,     0,   145,   146,   147,   148,   149,     0,   136,
     137,     0,   150,   151,   138,   139,   140,     0,   141,     0,
       0,   142,   143,   144,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   180,     0,   242,   145,   146,   147,   148,
     149,     0,   136,   137,     0,   150,   151,   138,   139,   140,
       0,   141,     0,     0,   142,   143,   144,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   145,
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
      53,    27,    28,   311,   312,   293,   294,   315,    28,    95,
      96,    28,    28,    28,   168,   101,   102,    91,   128,    99,
     129,   130,   131,   331,    99,   179,    99,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   135,   153,   105,   105,    45,   157,   203,
     358,    28,    52,   105,   105,    28,    28,   166,   105,    28,
      28,   170,   171,    28,   173,   174,   100,   176,   177,   100,
      28,   180,   105,    54,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   321,    32,    28,    42,   197,    32,
     327,    36,    58,    52,    32,    32,    52,   206,    32,   208,
      32,   128,   303,   324,   169,   265,    31,   260,    -1,    -1,
      65,    66,    -1,    -1,    -1,    70,    71,    72,   200,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   241,   242,   288,   227,    -1,    92,    93,    94,
      95,    96,    -1,    -1,    -1,    -1,   101,   102,    -1,    -1,
      -1,   260,   306,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   322,   317,    -1,    -1,    -1,    -1,   277,   278,
     279,   280,    -1,   333,    -1,   284,    -1,    -1,    -1,    -1,
      -1,   273,    -1,    -1,   276,   267,    -1,    -1,   270,    -1,
      -1,   345,   301,   347,    -1,   355,    -1,   351,    -1,    -1,
     282,    -1,    -1,   285,    71,    72,    -1,    74,    -1,    -1,
      77,    -1,   321,    -1,    -1,    -1,    -1,    -1,   327,    -1,
     302,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      -1,    -1,   314,    -1,   101,   102,    -1,   319,    -1,    -1,
      -1,   323,   324,    71,    72,    -1,    74,   357,   358,    77,
     332,    -1,    -1,    -1,    -1,   337,    -1,    -1,    -1,    -1,
     342,    -1,   344,    -1,   346,    93,    94,    95,    96,     0,
       1,    -1,    -1,   101,   102,    -1,    -1,    -1,    -1,   361,
     362,    12,    13,    14,    15,    -1,    17,    18,    19,    20,
      21,    -1,    23,    24,    25,    -1,    27,    28,    29,    -1,
      31,    -1,    33,    -1,    -1,    -1,    37,    -1,    39,    40,
      41,    -1,    -1,    44,    -1,    -1,    -1,    -1,    49,    50,
      51,    -1,    -1,    -1,    55,    -1,    -1,    -1,    59,    60,
      61,    62,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      81,    82,    83,    84,    -1,    -1,    87,    88,    89,    90,
      -1,    -1,    93,    94,    -1,    -1,    -1,    98,    99,    -1,
      -1,    -1,   103,    12,    13,    14,    15,    -1,    17,    18,
      19,    20,    21,    -1,    23,    24,    25,    -1,    27,    28,
      29,    -1,    31,    -1,    33,    -1,    -1,    -1,    37,    -1,
      39,    40,    41,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      49,    50,    51,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      59,    60,    61,    62,    -1,    64,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    -1,    -1,    87,    88,
      89,    90,    -1,    -1,    93,    94,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,   103,    12,    13,    14,    15,    -1,
      17,    18,    19,    20,    21,    -1,    23,    24,    25,    -1,
      27,    28,    29,    -1,    31,    32,    33,    -1,    -1,    -1,
      37,    -1,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    50,    51,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    61,    62,    -1,    64,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    -1,    -1,
      87,    88,    -1,    90,    -1,    -1,    93,    94,    -1,    -1,
      -1,    98,    99,    -1,    -1,    -1,   103,    12,    13,    14,
      15,    -1,    17,    18,    19,    20,    21,    -1,    23,    24,
      25,    -1,    27,    28,    29,    -1,    31,    -1,    33,    -1,
      -1,    -1,    37,    -1,    39,    40,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    49,    50,    51,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    61,    62,    -1,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      -1,    -1,    87,    88,    -1,    90,    -1,    -1,    93,    94,
      -1,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,    12,
      13,    14,    15,    -1,    17,    18,    19,    20,    21,    -1,
      23,    24,    25,    -1,    27,    28,    29,    -1,    31,    -1,
      33,    -1,    -1,    -1,    37,    -1,    39,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,    -1,
      -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    61,    62,
      -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    -1,    -1,    87,    88,    -1,    90,    -1,    -1,
      93,    94,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
     103,    12,    13,    14,    15,    -1,    17,    18,    19,    20,
      21,    -1,    23,    24,    25,    -1,    27,    28,    29,    54,
      31,    -1,    33,    -1,    -1,    -1,    37,    -1,    39,    40,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    49,    74,
      51,    -1,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      61,    62,    -1,    64,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    73,    98,    -1,    -1,   101,   102,    -1,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    88,    -1,    90,
      -1,    -1,    93,    94,    12,    13,    14,    15,    99,    -1,
      -1,    -1,   103,    -1,    -1,    23,    24,    25,    -1,    27,
      28,    29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    43,    -1,    45,    46,    47,
      48,    49,    -1,    51,    -1,    12,    13,    14,    15,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    23,    24,    25,    -1,
      27,    28,    29,    -1,    31,    73,    -1,    -1,    -1,    -1,
      -1,    38,    80,    81,    82,    83,    84,    -1,    45,    -1,
      47,    -1,    49,    -1,    51,    93,    94,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,   103,    63,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    12,    13,
      14,    15,    -1,    -1,    -1,    -1,    93,    94,    -1,    23,
      24,    25,    99,    27,    28,    29,   103,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    -1,    51,    12,    13,
      14,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,
      24,    25,    -1,    27,    28,    29,    -1,    31,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    49,    -1,    51,    -1,    93,
      94,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    12,    13,    14,    15,    -1,    -1,    -1,    -1,    93,
      94,    -1,    23,    24,    25,    99,    27,    28,    29,   103,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,
      51,    12,    13,    14,    15,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    24,    25,    -1,    27,    28,    29,    -1,
      31,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    49,    -1,
      51,    -1,    93,    94,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      81,    82,    83,    84,    12,    13,    14,    15,    47,    -1,
      -1,    -1,    93,    94,    -1,    23,    24,    25,    99,    27,
      28,    29,   103,    31,    -1,    -1,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    -1,    74,    -1,    -1,    77,    78,
      79,    49,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    -1,    -1,
      -1,    -1,   101,   102,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    -1,    93,    94,    54,    -1,    -1,
      -1,    99,    -1,    -1,    -1,   103,    -1,    -1,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    -1,    74,    -1,    -1,
      77,    78,    79,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      -1,    65,    66,    -1,   101,   102,    70,    71,    72,    -1,
      74,    -1,    -1,    77,    78,    79,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    -1,    65,    66,    -1,   101,   102,    70,
      71,    72,    -1,    74,    -1,    -1,    77,    78,    79,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    -1,    65,    66,    -1,
     101,   102,    70,    71,    72,    -1,    74,    -1,    -1,    77,
      78,    79,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    -1,
      65,    66,    -1,   101,   102,    70,    71,    72,    -1,    74,
      -1,    -1,    77,    78,    79,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    -1,    65,    66,    -1,   101,   102,    70,    71,
      72,    -1,    74,    -1,    -1,    77,    78,    79,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    -1,    65,    66,    -1,   101,
     102,    70,    71,    72,    -1,    74,    -1,    -1,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    -1,    65,
      66,    -1,   101,   102,    70,    71,    72,    -1,    74,    -1,
      -1,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    -1,    91,    92,    93,    94,    95,
      96,    -1,    65,    66,    -1,   101,   102,    70,    71,    72,
      -1,    74,    -1,    -1,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
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
      32,    52,   128,    32,   128,   127,   128,   127,   127,    32,
     114,   115,   129,   128,   128
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
			(*g_lingo->_currentScript)[(yyvsp[(1) - (3)].code) + 1] = end;
		;}
    break;

  case 31:
#line 263 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
			checkEnd((yyvsp[(6) - (6)].s), "tell", true); ;}
    break;

  case 32:
#line 266 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		;}
    break;

  case 33:
#line 271 "engines/director/lingo/lingo-gr.y"
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
#line 281 "engines/director/lingo/lingo-gr.y"
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
#line 293 "engines/director/lingo/lingo-gr.y"
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
#line 305 "engines/director/lingo/lingo-gr.y"
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
#line 315 "engines/director/lingo/lingo-gr.y"
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
#line 325 "engines/director/lingo/lingo-gr.y"
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
#line 335 "engines/director/lingo/lingo-gr.y"
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
#line 347 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 41:
#line 348 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 46:
#line 359 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 48:
#line 368 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 49:
#line 376 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 50:
#line 377 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 52:
#line 381 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 53:
#line 384 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 54:
#line 391 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 55:
#line 398 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 56:
#line 406 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 57:
#line 409 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 58:
#line 412 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 61:
#line 417 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 62:
#line 423 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_tellcode);
		g_lingo->code1(STOP); ;}
    break;

  case 63:
#line 427 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt((yyvsp[(1) - (1)].i)); ;}
    break;

  case 64:
#line 430 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_floatpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 65:
#line 433 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 66:
#line 436 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 68:
#line 440 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 69:
#line 443 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 70:
#line 446 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 71:
#line 447 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg)); ;}
    break;

  case 72:
#line 448 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 73:
#line 451 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 74:
#line 455 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 75:
#line 463 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 77:
#line 470 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 78:
#line 471 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 79:
#line 472 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 80:
#line 473 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 81:
#line 474 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); ;}
    break;

  case 82:
#line 475 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 83:
#line 476 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 84:
#line 477 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 85:
#line 478 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 86:
#line 479 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 87:
#line 480 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 88:
#line 481 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 89:
#line 482 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 90:
#line 483 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 91:
#line 484 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 92:
#line 485 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 93:
#line 486 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 94:
#line 487 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 95:
#line 488 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 96:
#line 489 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 97:
#line 490 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_arraypush); g_lingo->codeArray((yyvsp[(2) - (3)].narg)); ;}
    break;

  case 98:
#line 491 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 99:
#line 492 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 100:
#line 493 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); ;}
    break;

  case 101:
#line 494 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); ;}
    break;

  case 102:
#line 495 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); ;}
    break;

  case 103:
#line 496 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); ;}
    break;

  case 104:
#line 497 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); ;}
    break;

  case 105:
#line 498 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); ;}
    break;

  case 106:
#line 499 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); ;}
    break;

  case 107:
#line 500 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); ;}
    break;

  case 108:
#line 503 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 109:
#line 508 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 112:
#line 511 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); ;}
    break;

  case 113:
#line 512 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 117:
#line 516 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 118:
#line 519 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 119:
#line 522 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 120:
#line 525 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_voidpush);
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 1);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 121:
#line 529 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 122:
#line 530 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg)); ;}
    break;

  case 123:
#line 531 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 124:
#line 532 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].narg)); ;}
    break;

  case 125:
#line 533 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 126:
#line 534 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 127:
#line 535 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[(1) - (3)].s)); s += '-'; s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 128:
#line 538 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 129:
#line 539 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 130:
#line 542 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 131:
#line 543 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 132:
#line 546 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 133:
#line 547 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 134:
#line 558 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 135:
#line 559 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 136:
#line 560 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 137:
#line 561 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 138:
#line 565 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 139:
#line 569 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 144:
#line 583 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 145:
#line 584 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 146:
#line 588 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 147:
#line 592 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 148:
#line 596 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); ;}
    break;

  case 149:
#line 596 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[(1) - (3)].s), (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 150:
#line 626 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 151:
#line 627 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 152:
#line 631 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeFactory(*(yyvsp[(2) - (2)].s));
		;}
    break;

  case 153:
#line 634 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 154:
#line 635 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 155:
#line 639 "engines/director/lingo/lingo-gr.y"
    {	// D3
				g_lingo->code1(g_lingo->c_procret);
				g_lingo->define(*(yyvsp[(1) - (7)].s), (yyvsp[(2) - (7)].code), (yyvsp[(3) - (7)].narg));
				g_lingo->_indef = false;
				g_lingo->_ignoreMe = false;

				checkEnd((yyvsp[(7) - (7)].s), (yyvsp[(1) - (7)].s)->c_str(), false);
			;}
    break;

  case 156:
#line 647 "engines/director/lingo/lingo-gr.y"
    {	// D4. No 'end' clause
				g_lingo->code1(g_lingo->c_procret);
				g_lingo->define(*(yyvsp[(1) - (6)].s), (yyvsp[(2) - (6)].code), (yyvsp[(3) - (6)].narg));
				g_lingo->_indef = false;
				g_lingo->_ignoreMe = false;
			;}
    break;

  case 157:
#line 654 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); g_lingo->_indef = true; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; ;}
    break;

  case 158:
#line 656 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 159:
#line 657 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 160:
#line 658 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 161:
#line 659 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 162:
#line 662 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 163:
#line 666 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (2)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(2) - (2)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 164:
#line 674 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 165:
#line 675 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 166:
#line 676 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 167:
#line 679 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 168:
#line 680 "engines/director/lingo/lingo-gr.y"
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


#line 683 "engines/director/lingo/lingo-gr.y"


