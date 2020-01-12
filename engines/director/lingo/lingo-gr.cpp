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
     tMETHOD = 291,
     THEOBJECTFIELD = 292,
     tDOWN = 293,
     tELSE = 294,
     tELSIF = 295,
     tEXIT = 296,
     tGLOBAL = 297,
     tGO = 298,
     tIF = 299,
     tINTO = 300,
     tLOOP = 301,
     tMACRO = 302,
     tMOVIE = 303,
     tNEXT = 304,
     tOF = 305,
     tPREVIOUS = 306,
     tPUT = 307,
     tREPEAT = 308,
     tSET = 309,
     tTHEN = 310,
     tTO = 311,
     tWHEN = 312,
     tWITH = 313,
     tWHILE = 314,
     tNLELSE = 315,
     tFACTORY = 316,
     tOPEN = 317,
     tPLAY = 318,
     tDONE = 319,
     tINSTANCE = 320,
     tGE = 321,
     tLE = 322,
     tEQ = 323,
     tNEQ = 324,
     tAND = 325,
     tOR = 326,
     tNOT = 327,
     tMOD = 328,
     tAFTER = 329,
     tBEFORE = 330,
     tCONCAT = 331,
     tCONTAINS = 332,
     tSTARTS = 333,
     tCHAR = 334,
     tITEM = 335,
     tLINE = 336,
     tWORD = 337,
     tSPRITE = 338,
     tINTERSECTS = 339,
     tWITHIN = 340,
     tTELL = 341,
     tPROPERTY = 342,
     tON = 343,
     tME = 344,
     tENDIF = 345,
     tENDREPEAT = 346,
     tENDTELL = 347
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
#define ARGC 268
#define ARGCNORET 269
#define THEENTITY 270
#define THEENTITYWITHID 271
#define FLOAT 272
#define BLTIN 273
#define BLTINNOARGS 274
#define BLTINNOARGSORONE 275
#define BLTINONEARG 276
#define BLTINARGLIST 277
#define TWOWORDBUILTIN 278
#define FBLTIN 279
#define FBLTINNOARGS 280
#define FBLTINONEARG 281
#define FBLTINARGLIST 282
#define RBLTIN 283
#define RBLTINONEARG 284
#define ID 285
#define STRING 286
#define HANDLER 287
#define SYMBOL 288
#define ENDCLAUSE 289
#define tPLAYACCEL 290
#define tMETHOD 291
#define THEOBJECTFIELD 292
#define tDOWN 293
#define tELSE 294
#define tELSIF 295
#define tEXIT 296
#define tGLOBAL 297
#define tGO 298
#define tIF 299
#define tINTO 300
#define tLOOP 301
#define tMACRO 302
#define tMOVIE 303
#define tNEXT 304
#define tOF 305
#define tPREVIOUS 306
#define tPUT 307
#define tREPEAT 308
#define tSET 309
#define tTHEN 310
#define tTO 311
#define tWHEN 312
#define tWITH 313
#define tWHILE 314
#define tNLELSE 315
#define tFACTORY 316
#define tOPEN 317
#define tPLAY 318
#define tDONE 319
#define tINSTANCE 320
#define tGE 321
#define tLE 322
#define tEQ 323
#define tNEQ 324
#define tAND 325
#define tOR 326
#define tNOT 327
#define tMOD 328
#define tAFTER 329
#define tBEFORE 330
#define tCONCAT 331
#define tCONTAINS 332
#define tSTARTS 333
#define tCHAR 334
#define tITEM 335
#define tLINE 336
#define tWORD 337
#define tSPRITE 338
#define tINTERSECTS 339
#define tWITHIN 340
#define tTELL 341
#define tPROPERTY 342
#define tON 343
#define tME 344
#define tENDIF 345
#define tENDREPEAT 346
#define tENDTELL 347




/* Copy the first part of user declarations.  */
#line 52 "engines/director/lingo/lingo-gr.y"

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
#line 85 "engines/director/lingo/lingo-gr.y"
{
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
}
/* Line 193 of yacc.c.  */
#line 328 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 341 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  133
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1637

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  108
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  170
/* YYNRULES -- Number of states.  */
#define YYNSTATES  355

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   347

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     101,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   100,    95,     2,
     102,   103,    98,    96,   104,    97,     2,    99,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   107,     2,
      93,     2,    94,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   105,     2,   106,     2,     2,     2,     2,     2,     2,
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
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    13,    15,    17,    22,
      27,    32,    37,    42,    47,    52,    57,    63,    69,    74,
      76,    78,    80,    82,    84,    86,    88,    90,    92,    99,
     110,   122,   126,   133,   138,   148,   161,   162,   165,   172,
     184,   192,   195,   199,   201,   203,   204,   205,   206,   209,
     212,   216,   218,   220,   222,   224,   226,   228,   230,   232,
     234,   236,   239,   242,   247,   249,   252,   254,   256,   260,
     264,   268,   272,   276,   280,   284,   288,   292,   296,   300,
     304,   308,   311,   315,   319,   323,   327,   330,   333,   337,
     342,   347,   352,   359,   364,   371,   376,   383,   388,   395,
     400,   407,   409,   412,   415,   417,   419,   422,   424,   427,
     430,   433,   435,   438,   441,   443,   446,   451,   454,   458,
     460,   464,   466,   470,   472,   476,   479,   482,   485,   488,
     492,   495,   499,   502,   505,   508,   512,   515,   516,   520,
     521,   530,   533,   534,   542,   551,   558,   561,   562,   564,
     568,   573,   574,   576,   580,   581,   584,   585,   587,   591,
     593,   597,   601,   602,   604,   606,   608,   610,   614,   616,
     620
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     109,     0,    -1,   109,   101,   110,    -1,   110,    -1,     1,
     101,    -1,    -1,   139,    -1,   114,    -1,    52,   129,    45,
      30,    -1,    52,   129,    45,   130,    -1,    52,   129,    74,
     129,    -1,    52,   129,    75,   129,    -1,    54,    30,    68,
     129,    -1,    54,    15,    68,   129,    -1,    54,    30,    56,
     129,    -1,    54,    15,    56,   129,    -1,    54,    16,   128,
      56,   129,    -1,    54,    16,   128,    68,   129,    -1,    54,
      37,    56,   129,    -1,   146,    -1,   129,    -1,   131,    -1,
     146,    -1,   129,    -1,   131,    -1,   118,    -1,   112,    -1,
     115,    -1,   119,   129,   124,   125,   124,    91,    -1,   120,
      68,   129,   124,    56,   129,   124,   125,   124,    91,    -1,
     120,    68,   129,   124,    38,    56,   129,   124,   125,   124,
      91,    -1,   126,   113,   124,    -1,   127,   129,   101,   125,
     124,    92,    -1,   127,   129,    56,   129,    -1,   121,   129,
     124,    55,   125,   124,   116,   124,    90,    -1,   121,   129,
     124,    55,   125,   124,   116,    39,   123,   125,   124,    90,
      -1,    -1,   116,   117,    -1,   122,   129,   124,    55,   125,
     124,    -1,   121,   129,   124,    55,   112,   124,    39,   123,
     112,   124,    90,    -1,   121,   129,   124,    55,   112,   124,
      90,    -1,    53,    59,    -1,    53,    58,    30,    -1,    44,
      -1,    40,    -1,    -1,    -1,    -1,   125,   101,    -1,   125,
     114,    -1,    57,    30,    55,    -1,    86,    -1,    12,    -1,
      17,    -1,    33,    -1,    31,    -1,    30,    -1,   149,    -1,
     128,    -1,   130,    -1,    25,    -1,    26,   129,    -1,    27,
     147,    -1,    30,   102,   147,   103,    -1,    15,    -1,    16,
     128,    -1,    37,    -1,   111,    -1,   129,    96,   129,    -1,
     129,    97,   129,    -1,   129,    98,   129,    -1,   129,    99,
     129,    -1,   129,    73,   129,    -1,   129,    94,   129,    -1,
     129,    93,   129,    -1,   129,    68,   129,    -1,   129,    69,
     129,    -1,   129,    66,   129,    -1,   129,    67,   129,    -1,
     129,    70,   129,    -1,   129,    71,   129,    -1,    72,   129,
      -1,   129,    95,   129,    -1,   129,    76,   129,    -1,   129,
      77,   129,    -1,   129,    78,   129,    -1,    96,   129,    -1,
      97,   129,    -1,   102,   129,   103,    -1,    83,   129,    84,
     129,    -1,    83,   129,    85,   129,    -1,    79,   129,    50,
     129,    -1,    79,   129,    56,   129,    50,   129,    -1,    80,
     129,    50,   129,    -1,    80,   129,    56,   129,    50,   129,
      -1,    81,   129,    50,   129,    -1,    81,   129,    56,   129,
      50,   129,    -1,    82,   129,    50,   129,    -1,    82,   129,
      56,   129,    50,   129,    -1,    89,   102,    30,   103,    -1,
      89,   102,    30,   104,   147,   103,    -1,    89,    -1,    29,
     128,    -1,    52,   129,    -1,   135,    -1,   137,    -1,    41,
      53,    -1,    41,    -1,    42,   132,    -1,    87,   133,    -1,
      65,   134,    -1,    19,    -1,    21,   129,    -1,    20,   129,
      -1,    20,    -1,    22,   147,    -1,    62,   129,    58,   129,
      -1,    62,   129,    -1,    23,    30,   147,    -1,    30,    -1,
     132,   104,    30,    -1,    30,    -1,   133,   104,    30,    -1,
      30,    -1,   134,   104,    30,    -1,    43,    46,    -1,    43,
      49,    -1,    43,    51,    -1,    43,   129,    -1,    43,   129,
     136,    -1,    43,   136,    -1,    50,    48,   129,    -1,    48,
     129,    -1,    63,    64,    -1,    63,   129,    -1,    63,   129,
     136,    -1,    63,   136,    -1,    -1,    35,   138,   147,    -1,
      -1,    47,    30,   140,   123,   143,   101,   145,   125,    -1,
      61,    30,    -1,    -1,    36,   141,   123,   143,   101,   145,
     125,    -1,   142,   123,   143,   101,   145,   125,    34,   144,
      -1,   142,   123,   143,   101,   145,   125,    -1,    88,    30,
      -1,    -1,    30,    -1,   143,   104,    30,    -1,   143,   101,
     104,    30,    -1,    -1,    30,    -1,   144,   104,    30,    -1,
      -1,    30,   148,    -1,    -1,   129,    -1,   147,   104,   129,
      -1,   129,    -1,   148,   104,   129,    -1,   105,   150,   106,
      -1,    -1,   107,    -1,   151,    -1,   152,    -1,   128,    -1,
     151,   104,   128,    -1,   153,    -1,   152,   104,   153,    -1,
      33,   107,   128,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   132,   132,   133,   134,   137,   138,   139,   142,   148,
     151,   152,   153,   159,   166,   172,   179,   185,   191,   198,
     199,   200,   203,   204,   205,   206,   209,   210,   215,   226,
     243,   255,   260,   262,   267,   277,   289,   290,   293,   301,
     311,   324,   327,   334,   341,   349,   352,   355,   356,   357,
     360,   366,   370,   373,   376,   379,   382,   386,   389,   390,
     391,   394,   397,   398,   401,   409,   415,   419,   420,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   456,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   472,   475,   478,   482,   483,   484,   485,   488,
     489,   492,   493,   496,   497,   508,   509,   510,   511,   515,
     519,   525,   526,   529,   530,   534,   538,   542,   542,   572,
     572,   578,   579,   579,   585,   593,   600,   602,   603,   604,
     605,   608,   609,   610,   613,   616,   624,   625,   626,   629,
     630,   633,   636,   637,   638,   639,   642,   643,   646,   647,
     650
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
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
  "SYMBOL", "ENDCLAUSE", "tPLAYACCEL", "tMETHOD", "THEOBJECTFIELD",
  "tDOWN", "tELSE", "tELSIF", "tEXIT", "tGLOBAL", "tGO", "tIF", "tINTO",
  "tLOOP", "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT",
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tNLELSE",
  "tFACTORY", "tOPEN", "tPLAY", "tDONE", "tINSTANCE", "tGE", "tLE", "tEQ",
  "tNEQ", "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tME", "tENDIF",
  "tENDREPEAT", "tENDTELL", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'\\n'", "'('", "')'", "','", "'['", "']'", "':'",
  "$accept", "program", "programline", "asgn", "stmtoneliner",
  "stmtonelinerwithif", "stmt", "ifstmt", "elseifstmtlist", "elseifstmt",
  "ifoneliner", "repeatwhile", "repeatwith", "if", "elseif", "begin",
  "end", "stmtlist", "when", "tell", "simpleexpr", "expr", "reference",
  "proc", "globallist", "propertylist", "instancelist", "gotofunc",
  "gotomovie", "playfunc", "@1", "defn", "@2", "@3", "on", "argdef",
  "endargdef", "argstore", "macro", "arglist", "nonemptyarglist", "list",
  "valuelist", "linearlist", "proplist", "proppair", 0
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
     345,   346,   347,    60,    62,    38,    43,    45,    42,    47,
      37,    10,    40,    41,    44,    91,    93,    58
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   108,   109,   109,   109,   110,   110,   110,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   112,
     112,   112,   113,   113,   113,   113,   114,   114,   114,   114,
     114,   114,   114,   114,   115,   115,   116,   116,   117,   118,
     118,   119,   120,   121,   122,   123,   124,   125,   125,   125,
     126,   127,   128,   128,   128,   128,   128,   128,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   130,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   132,
     132,   133,   133,   134,   134,   135,   135,   135,   135,   135,
     135,   136,   136,   137,   137,   137,   137,   138,   137,   140,
     139,   139,   141,   139,   139,   139,   142,   143,   143,   143,
     143,   144,   144,   144,   145,   146,   147,   147,   147,   148,
     148,   149,   150,   150,   150,   150,   151,   151,   152,   152,
     153
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
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
       4,     4,     6,     4,     6,     4,     6,     4,     6,     4,
       6,     1,     2,     2,     1,     1,     2,     1,     2,     2,
       2,     1,     2,     2,     1,     2,     4,     2,     3,     1,
       3,     1,     3,     1,     3,     2,     2,     2,     2,     3,
       2,     3,     2,     2,     2,     3,     2,     0,     3,     0,
       8,     2,     0,     7,     8,     6,     2,     0,     1,     3,
       4,     0,     1,     3,     0,     2,     0,     1,     3,     1,
       3,     3,     0,     1,     1,     1,     1,     3,     1,     3,
       3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    52,    64,     0,    53,   111,   114,     0,   156,
       0,    60,     0,   156,     0,    56,    55,    54,   137,   142,
      66,   107,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,   101,     0,     0,     0,   162,     0,     3,
      67,    26,     7,    27,     0,     0,     0,     0,     0,    58,
      20,    59,    21,   104,   105,     6,    45,    19,    57,     4,
      56,    65,    56,     0,   113,   112,   157,   115,   156,    61,
      62,   102,   156,   159,   155,   156,    45,   106,   119,   108,
     125,     0,   126,     0,   127,   128,   130,   139,   103,     0,
      41,     0,     0,     0,     0,     0,   141,   117,   133,   134,
     136,   123,   110,    81,     0,     0,     0,     0,     0,   121,
     109,   146,     0,    86,    87,     0,    54,   163,   166,     0,
     164,   165,   168,     1,     5,    46,     0,    46,    46,    25,
       0,    23,    24,    22,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   147,   156,     0,     0,   118,   157,     0,     0,
     138,   147,     0,   132,     0,   129,    45,     0,     0,     0,
      42,     0,     0,     0,     0,     0,     0,    50,     0,   135,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    88,     0,   161,     0,     0,     2,    47,
      46,     0,    31,    46,     0,    47,    77,    78,    75,    76,
      79,    80,    72,    83,    84,    85,    74,    73,    82,    68,
      69,    70,    71,   148,     0,   158,    63,   160,     0,   120,
     131,   147,     8,     9,    10,    11,    15,    13,     0,     0,
      14,    12,    18,   116,   124,    91,     0,    93,     0,    95,
       0,    97,     0,    89,    90,   122,    99,   156,   170,   167,
       0,   169,    46,     0,    47,     0,    33,    46,   154,     0,
     154,     0,    16,    17,     0,     0,     0,     0,     0,    48,
      49,     0,     0,     0,    46,     0,     0,     0,    47,   149,
      47,   154,    92,    94,    96,    98,   100,    28,     0,    46,
      36,    46,    32,   150,   145,   143,    47,    46,    47,    46,
       0,   151,   140,    47,    46,    45,    44,    37,     0,     0,
      45,    40,   152,   144,    46,     0,    47,    46,    34,     0,
       0,     0,    29,    46,     0,    46,   153,    30,     0,    47,
       0,    35,    46,    39,    38
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    48,    49,    50,    51,   138,   290,    53,   319,   327,
     139,    54,    55,    56,   328,   162,   209,   272,    57,    58,
      59,    60,    61,    62,    89,   120,   112,    63,    96,    64,
      85,    65,   176,    86,    66,   234,   333,   298,    67,   168,
      84,    68,   129,   130,   131,   132
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -283
static const yytype_int16 yypact[] =
{
     336,   -86,  -283,  -283,    29,  -283,  -283,   996,   996,   996,
      15,  -283,   996,   996,    29,  1030,  -283,  -283,  -283,  -283,
    -283,   -17,    22,   874,  -283,    26,   996,    33,    18,    31,
      35,   996,   935,    37,   996,   996,   996,   996,   996,   996,
    -283,    51,    79,    16,   996,   996,   996,    52,     9,  -283,
    -283,  -283,  -283,  -283,   996,    53,   996,   700,   996,  -283,
    1525,  -283,  -283,  -283,  -283,  -283,  -283,  -283,  -283,  -283,
    -283,  -283,    17,   996,  1525,  1525,  1525,    19,   996,  1525,
      19,  -283,   996,  1525,    20,   996,  -283,  -283,  -283,    23,
    -283,   996,  -283,    72,  -283,  1083,  -283,  -283,  1070,    92,
    -283,   -14,    29,     2,    69,    71,  -283,  1419,  -283,  1083,
    -283,  -283,    24,  -283,  1133,  1167,  1201,  1235,  1491,  -283,
      27,  -283,   100,  -283,  -283,  1453,    25,  -283,  -283,    54,
      57,    59,  -283,  -283,   427,  1525,   996,  1525,  -283,  -283,
     996,  1525,  -283,  -283,  1405,   996,   996,   996,   996,   996,
     996,   996,   996,   996,   996,   996,   996,   996,   996,   996,
     996,   996,   107,   996,  1070,   996,    19,  1453,     1,   996,
      19,   107,   134,  1525,   996,  -283,  -283,    77,   996,   996,
    -283,   996,   996,    12,   996,   996,   996,  -283,   996,  -283,
     135,   996,   996,   996,   996,   996,   996,   996,   996,   996,
     996,   136,     8,  -283,    29,  -283,    29,   137,  -283,  -283,
    1525,   113,  -283,  1525,   996,  -283,     0,     0,     0,     0,
    1538,  1538,  -283,     4,     0,     0,     0,     0,     4,   -55,
     -55,  -283,  -283,  -283,   -78,  1525,  -283,  1525,   -15,  -283,
    1525,   107,  -283,  -283,  1525,  1525,  1525,     0,   996,   996,
    1525,     0,  1525,  1525,  -283,  1525,  1269,  1525,  1303,  1525,
    1337,  1525,  1371,  1525,  1525,  -283,  -283,   996,  -283,  -283,
      25,  -283,   609,   -16,  -283,   114,  1525,   609,    78,   143,
      78,   -11,  1525,     0,   996,   996,   996,   996,    10,  -283,
    -283,    89,   127,   996,   609,   791,   102,   165,  -283,  -283,
    -283,    78,  1525,  1525,  1525,  1525,  -283,  -283,   996,  1525,
    -283,  -283,  -283,  -283,   518,   609,  -283,  1525,  -283,    76,
     -27,   166,   609,  -283,   609,  -283,  -283,  -283,   996,   108,
    -283,  -283,  -283,    93,   609,   110,  -283,  1525,  -283,   791,
     169,   112,  -283,   609,   149,  -283,  -283,  -283,   115,  -283,
     116,  -283,   609,  -283,  -283
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -283,  -283,    75,  -283,  -282,  -283,    21,  -283,  -283,  -283,
    -283,  -283,  -283,   156,  -283,   -82,    -2,   -98,  -283,  -283,
       6,    -7,    38,   157,  -283,  -283,  -283,  -283,   -21,  -283,
    -283,  -283,  -283,  -283,  -283,  -154,  -283,  -266,   159,    -6,
    -283,  -283,  -283,  -283,  -283,    14
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -6
static const yytype_int16 yytable[] =
{
      74,    75,    76,    77,   171,    79,    76,    80,    83,   133,
      71,   110,   330,   311,   300,    69,    95,   238,   151,    98,
      81,    52,   292,   278,   107,   109,   279,   113,   114,   115,
     116,   117,   118,   101,   102,   316,    87,   123,   124,   125,
     293,     2,   181,   160,   161,    78,     5,   135,   103,   137,
     141,   144,    88,   128,   182,   104,    97,   345,   184,    70,
      16,   105,    17,   331,     2,   106,   164,   111,   248,     5,
     185,    76,   166,   151,   175,   167,   152,   151,    76,   170,
     249,   119,    70,    16,   173,   126,   280,   281,   189,   279,
     301,    99,   100,   279,   241,   157,   158,   159,   160,   161,
     158,   159,   160,   161,   236,   165,    14,   242,   183,   121,
     134,   266,   267,   306,   165,   325,   326,   277,   122,   163,
     174,   136,   180,   165,   169,   186,   187,   172,   190,   210,
     202,   201,   204,   213,    47,   211,   212,   233,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,    52,    76,    47,   235,   127,
     205,   206,   237,   207,   239,   254,   265,   240,   274,   295,
     270,   244,   245,   299,   246,   247,   294,   250,   251,   252,
     307,   253,   297,   308,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   312,   313,   332,   340,   338,   346,
     314,   342,   315,   347,   349,   351,   353,   276,   273,   208,
     268,   275,   269,   140,   142,   243,   143,     0,   322,     0,
     324,   271,     0,     0,     0,   334,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   343,     0,
       0,   282,   283,   336,     0,     0,     0,     0,   339,     0,
       0,   352,     0,     0,     0,     0,     0,     0,     0,     0,
      76,   288,     0,     0,     0,     0,     0,     0,     0,     0,
     291,     0,     0,     0,     0,   296,     0,   302,   303,   304,
     305,     0,     0,     0,     0,     0,   309,     0,     0,     0,
       0,     0,   310,     0,     0,     0,     0,     0,     0,     0,
       0,   317,     0,     0,     0,     0,     0,   318,     0,   320,
       0,     0,     0,     0,     0,   323,     0,   329,     0,     0,
       0,   337,   335,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   341,     0,     0,   344,    -5,     1,     0,     0,
       0,   348,     0,   350,     0,     0,     0,     0,     2,     0,
     354,     3,     4,     5,     0,     6,     7,     8,     9,    10,
       0,    11,    12,    13,     0,    14,    15,    16,     0,    17,
       0,    18,    19,    20,     0,     0,     0,    21,    22,    23,
      24,     0,     0,    25,     0,     0,     0,     0,    26,    27,
      28,     0,     0,    29,     0,     0,     0,    30,    31,    32,
       0,    33,     0,     0,     0,     0,     0,     0,    34,     0,
       0,     0,     0,     0,     0,    35,    36,    37,    38,    39,
       0,     0,    40,    41,    42,    43,     0,     0,     0,     0,
       0,     0,    44,    45,     0,     0,     0,    -5,    46,     2,
       0,    47,     3,     4,     5,     0,     6,     7,     8,     9,
      10,     0,    11,    12,    13,     0,    14,    15,    16,     0,
      17,     0,    18,    19,    20,     0,     0,     0,    21,    22,
      23,    24,     0,     0,    25,     0,     0,     0,     0,    26,
      27,    28,     0,     0,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,     0,     0,     0,     0,     0,    34,
       0,     0,     0,     0,     0,     0,    35,    36,    37,    38,
      39,     0,     0,    40,    41,    42,    43,     0,     0,     0,
       0,     0,     0,    44,    45,     0,     0,     0,     0,    46,
       2,     0,    47,     3,     4,     5,     0,     6,     7,     8,
       9,    10,     0,    11,    12,    13,     0,    14,    15,    16,
       0,    17,   321,    18,     0,    20,     0,     0,     0,    21,
      22,    23,    24,     0,     0,     0,     0,     0,     0,     0,
      26,    27,    28,     0,     0,    29,     0,     0,     0,     0,
      31,    32,     0,    33,     0,     0,     0,     0,     0,     0,
      34,     0,     0,     0,     0,     0,     0,    35,    36,    37,
      38,    39,     0,     0,    40,    41,     0,    43,     0,     0,
       0,     0,     0,     0,    44,    45,     0,     0,     0,   289,
      46,     2,     0,    47,     3,     4,     5,     0,     6,     7,
       8,     9,    10,     0,    11,    12,    13,     0,    14,    15,
      16,     0,    17,     0,    18,     0,    20,     0,     0,     0,
      21,    22,    23,    24,     0,     0,     0,     0,     0,     0,
       0,    26,    27,    28,     0,     0,    29,     0,     0,     0,
       0,    31,    32,     0,    33,     0,     0,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,     0,    35,    36,
      37,    38,    39,     0,     0,    40,    41,     0,    43,     0,
       0,     0,     0,     0,     0,    44,    45,     0,     0,     0,
     289,    46,     2,     0,    47,     3,     4,     5,     0,     6,
       7,     8,     9,    10,     0,    11,    12,    13,     0,    14,
      15,    16,     0,    17,     0,    18,     0,    20,     0,     0,
       0,    21,    22,    23,    24,     0,     0,     0,     0,     0,
       0,     0,    26,     0,    28,     0,     0,     0,     0,     0,
       0,     0,    31,    32,     0,    33,     0,     0,     0,     0,
       0,     0,    34,     0,     0,     0,     0,     0,     0,    35,
      36,    37,    38,    39,     0,     0,     0,    41,     0,    43,
       0,     0,     0,     0,     0,     0,    44,    45,     0,     0,
       0,     0,    46,     2,     0,    47,     3,     4,     5,     0,
       6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
      14,    15,    16,     0,    17,     0,    18,     0,    20,     0,
       0,     0,    21,    22,    23,     0,     0,     0,     0,     0,
       0,     0,     0,    26,     0,    28,     0,     0,     0,     0,
       0,     0,     0,    31,    32,     0,    33,     0,     0,     0,
       0,     0,     0,    34,     0,     0,     0,     0,     0,     0,
      35,    36,    37,    38,    39,     0,     0,     0,    41,     0,
      43,     0,     0,     0,     0,     0,     2,    44,    45,     3,
       4,     5,     0,    46,     0,     0,    47,     0,     0,    11,
      12,    13,     0,    14,    72,    16,     0,    17,     0,     0,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
      90,     0,    91,    92,    93,    94,    73,     0,    28,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    34,     2,     0,     0,
       3,     4,     5,    35,    36,    37,    38,    39,     0,     0,
      11,    12,    13,    43,    14,    72,    16,     0,    17,     0,
      44,    45,    20,     0,     0,     0,    46,     0,     0,    47,
       0,     0,     0,    91,     0,    93,     0,    73,     0,    28,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   108,
       0,     0,     0,     0,     0,     0,     0,    34,     2,     0,
       0,     3,     4,     5,    35,    36,    37,    38,    39,     0,
       0,    11,    12,    13,    43,    14,    72,    16,     0,    17,
       0,    44,    45,    20,     0,     0,     0,    46,     0,     0,
      47,     0,     2,     0,     0,     3,     4,     5,    73,     0,
      28,     0,     0,     0,     0,    11,    12,    13,     0,    14,
      72,    16,     0,    17,     0,     0,     0,    20,    34,     0,
       0,     0,     0,     0,     0,    35,    36,    37,    38,    39,
       0,     0,    73,     0,    28,    43,     0,     0,     0,     0,
       0,     0,    44,    45,     0,     0,     0,     0,    46,     0,
       0,    47,    34,     0,     0,     0,     0,     0,     0,    35,
      36,    37,    38,    39,     0,   177,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,    44,    45,     0,     0,
       0,    91,    82,    93,     0,    47,   145,   146,   147,   148,
     149,   150,     0,   151,   178,   179,   152,   153,   154,   145,
     146,   147,   148,   149,   150,     0,   151,     0,     0,   152,
     153,   154,     0,   155,   156,   157,   158,   159,   160,   161,
       0,     0,     0,     0,     0,     0,   155,   156,   157,   158,
     159,   160,   161,   191,     0,     0,     0,     0,     0,   192,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   145,
     146,   147,   148,   149,   150,     0,   151,     0,     0,   152,
     153,   154,     0,     0,     0,     0,     0,   193,     0,     0,
       0,     0,     0,   194,     0,     0,   155,   156,   157,   158,
     159,   160,   161,   145,   146,   147,   148,   149,   150,     0,
     151,     0,     0,   152,   153,   154,     0,     0,     0,     0,
       0,   195,     0,     0,     0,     0,     0,   196,     0,     0,
     155,   156,   157,   158,   159,   160,   161,   145,   146,   147,
     148,   149,   150,     0,   151,     0,     0,   152,   153,   154,
       0,     0,     0,     0,     0,   197,     0,     0,     0,     0,
       0,   198,     0,     0,   155,   156,   157,   158,   159,   160,
     161,   145,   146,   147,   148,   149,   150,     0,   151,     0,
       0,   152,   153,   154,     0,     0,     0,     0,     0,   284,
       0,     0,     0,     0,     0,     0,     0,     0,   155,   156,
     157,   158,   159,   160,   161,   145,   146,   147,   148,   149,
     150,     0,   151,     0,     0,   152,   153,   154,     0,     0,
       0,     0,     0,   285,     0,     0,     0,     0,     0,     0,
       0,     0,   155,   156,   157,   158,   159,   160,   161,   145,
     146,   147,   148,   149,   150,     0,   151,     0,     0,   152,
     153,   154,     0,     0,     0,     0,     0,   286,     0,     0,
       0,     0,     0,     0,     0,     0,   155,   156,   157,   158,
     159,   160,   161,   145,   146,   147,   148,   149,   150,     0,
     151,     0,     0,   152,   153,   154,     0,     0,     0,     0,
       0,   287,     0,     0,     0,     0,     0,     0,     0,     0,
     155,   156,   157,   158,   159,   160,   161,   145,   146,   147,
     148,   149,   150,     0,   151,     0,     0,   152,   153,   154,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   214,     0,     0,   155,   156,   157,   158,   159,   160,
     161,   145,   146,   147,   148,   149,   150,   188,   151,     0,
       0,   152,   153,   154,     0,   145,   146,   147,   148,   149,
     150,     0,   151,     0,     0,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,     0,   215,     0,     0,     0,
       0,     0,   155,   156,   157,   158,   159,   160,   161,   145,
     146,   147,   148,   149,   150,     0,   151,     0,     0,   152,
     153,   154,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   155,   156,   157,   158,
     159,   160,   161,     0,     0,     0,   203,   145,   146,   147,
     148,   149,   150,     0,   151,     0,     0,   152,   153,   154,
       0,     0,     0,     0,     0,   199,   200,     0,     0,     0,
       0,     0,     0,     0,   155,   156,   157,   158,   159,   160,
     161,   145,   146,   147,   148,   149,   150,     0,   151,     0,
       0,   152,   153,   154,   145,   146,   147,   148,     0,     0,
       0,   151,     0,     0,   152,   153,   154,     0,   155,   156,
     157,   158,   159,   160,   161,     0,     0,     0,     0,     0,
       0,   155,   156,   157,   158,   159,   160,   161
};

static const yytype_int16 yycheck[] =
{
       7,     8,     9,     9,    86,    12,    13,    13,    15,     0,
       4,    32,    39,   295,   280,   101,    23,   171,    73,    26,
      14,     0,    38,   101,    31,    32,   104,    34,    35,    36,
      37,    38,    39,    15,    16,   301,    53,    44,    45,    46,
      56,    12,    56,    98,    99,    30,    17,    54,    30,    56,
      57,    58,    30,    47,    68,    37,    30,   339,    56,    30,
      31,    30,    33,    90,    12,    30,    73,    30,    56,    17,
      68,    78,    78,    73,    95,    82,    76,    73,    85,    85,
      68,    30,    30,    31,    91,    33,   101,   241,   109,   104,
     101,    58,    59,   104,   176,    95,    96,    97,    98,    99,
      96,    97,    98,    99,   103,   104,    29,    30,   102,    30,
     101,   103,   104,   103,   104,    39,    40,   215,   102,   102,
      48,    68,    30,   104,   104,    56,    55,   104,   104,   136,
      30,   104,   107,   140,   105,   137,   138,    30,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   134,   163,   105,   165,   107,
     106,   104,   169,   104,    30,    30,    30,   174,    55,    55,
      33,   178,   179,    30,   181,   182,   274,   184,   185,   186,
      91,   188,   104,    56,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,    92,    30,    30,   104,    90,    30,
     298,    91,   300,    91,    55,    90,    90,   214,   210,   134,
     204,   213,   206,    57,    57,   177,    57,    -1,   316,    -1,
     318,   207,    -1,    -1,    -1,   323,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   336,    -1,
      -1,   248,   249,   325,    -1,    -1,    -1,    -1,   330,    -1,
      -1,   349,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     267,   267,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     272,    -1,    -1,    -1,    -1,   277,    -1,   284,   285,   286,
     287,    -1,    -1,    -1,    -1,    -1,   293,    -1,    -1,    -1,
      -1,    -1,   294,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   308,    -1,    -1,    -1,    -1,    -1,   309,    -1,   311,
      -1,    -1,    -1,    -1,    -1,   317,    -1,   319,    -1,    -1,
      -1,   328,   324,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   334,    -1,    -1,   337,     0,     1,    -1,    -1,
      -1,   343,    -1,   345,    -1,    -1,    -1,    -1,    12,    -1,
     352,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      -1,    25,    26,    27,    -1,    29,    30,    31,    -1,    33,
      -1,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      44,    -1,    -1,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    61,    62,    63,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      -1,    -1,    86,    87,    88,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    96,    97,    -1,    -1,    -1,   101,   102,    12,
      -1,   105,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    -1,    25,    26,    27,    -1,    29,    30,    31,    -1,
      33,    -1,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    44,    -1,    -1,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    61,    62,
      63,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    -1,    -1,    86,    87,    88,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    96,    97,    -1,    -1,    -1,    -1,   102,
      12,    -1,   105,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    -1,    25,    26,    27,    -1,    29,    30,    31,
      -1,    33,    34,    35,    -1,    37,    -1,    -1,    -1,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      72,    -1,    -1,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    -1,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    96,    97,    -1,    -1,    -1,   101,
     102,    12,    -1,   105,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    -1,    25,    26,    27,    -1,    29,    30,
      31,    -1,    33,    -1,    35,    -1,    37,    -1,    -1,    -1,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    -1,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    96,    97,    -1,    -1,    -1,
     101,   102,    12,    -1,   105,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    -1,    25,    26,    27,    -1,    29,
      30,    31,    -1,    33,    -1,    35,    -1,    37,    -1,    -1,
      -1,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    83,    -1,    -1,    -1,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    96,    97,    -1,    -1,
      -1,    -1,   102,    12,    -1,   105,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    -1,    25,    26,    27,    -1,
      29,    30,    31,    -1,    33,    -1,    35,    -1,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    -1,    -1,    -1,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    12,    96,    97,    15,
      16,    17,    -1,   102,    -1,    -1,   105,    -1,    -1,    25,
      26,    27,    -1,    29,    30,    31,    -1,    33,    -1,    -1,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    -1,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    72,    12,    -1,    -1,
      15,    16,    17,    79,    80,    81,    82,    83,    -1,    -1,
      25,    26,    27,    89,    29,    30,    31,    -1,    33,    -1,
      96,    97,    37,    -1,    -1,    -1,   102,    -1,    -1,   105,
      -1,    -1,    -1,    48,    -1,    50,    -1,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,    12,    -1,
      -1,    15,    16,    17,    79,    80,    81,    82,    83,    -1,
      -1,    25,    26,    27,    89,    29,    30,    31,    -1,    33,
      -1,    96,    97,    37,    -1,    -1,    -1,   102,    -1,    -1,
     105,    -1,    12,    -1,    -1,    15,    16,    17,    52,    -1,
      54,    -1,    -1,    -1,    -1,    25,    26,    27,    -1,    29,
      30,    31,    -1,    33,    -1,    -1,    -1,    37,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      -1,    -1,    52,    -1,    54,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    96,    97,    -1,    -1,    -1,    -1,   102,    -1,
      -1,   105,    72,    -1,    -1,    -1,    -1,    -1,    -1,    79,
      80,    81,    82,    83,    -1,    45,    -1,    -1,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    96,    97,    -1,    -1,
      -1,    48,   102,    50,    -1,   105,    66,    67,    68,    69,
      70,    71,    -1,    73,    74,    75,    76,    77,    78,    66,
      67,    68,    69,    70,    71,    -1,    73,    -1,    -1,    76,
      77,    78,    -1,    93,    94,    95,    96,    97,    98,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,    50,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      67,    68,    69,    70,    71,    -1,    73,    -1,    -1,    76,
      77,    78,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    56,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,    66,    67,    68,    69,    70,    71,    -1,
      73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,
      93,    94,    95,    96,    97,    98,    99,    66,    67,    68,
      69,    70,    71,    -1,    73,    -1,    -1,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    56,    -1,    -1,    93,    94,    95,    96,    97,    98,
      99,    66,    67,    68,    69,    70,    71,    -1,    73,    -1,
      -1,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,    66,    67,    68,    69,    70,
      71,    -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    98,    99,    66,
      67,    68,    69,    70,    71,    -1,    73,    -1,    -1,    76,
      77,    78,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,    66,    67,    68,    69,    70,    71,    -1,
      73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    98,    99,    66,    67,    68,
      69,    70,    71,    -1,    73,    -1,    -1,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    -1,    -1,    93,    94,    95,    96,    97,    98,
      99,    66,    67,    68,    69,    70,    71,    58,    73,    -1,
      -1,    76,    77,    78,    -1,    66,    67,    68,    69,    70,
      71,    -1,    73,    -1,    -1,    76,    77,    78,    93,    94,
      95,    96,    97,    98,    99,    -1,   101,    -1,    -1,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    98,    99,    66,
      67,    68,    69,    70,    71,    -1,    73,    -1,    -1,    76,
      77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,    -1,    -1,    -1,   103,    66,    67,    68,
      69,    70,    71,    -1,    73,    -1,    -1,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    84,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    98,
      99,    66,    67,    68,    69,    70,    71,    -1,    73,    -1,
      -1,    76,    77,    78,    66,    67,    68,    69,    -1,    -1,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    93,    94,
      95,    96,    97,    98,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    98,    99
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    15,    16,    17,    19,    20,    21,    22,
      23,    25,    26,    27,    29,    30,    31,    33,    35,    36,
      37,    41,    42,    43,    44,    47,    52,    53,    54,    57,
      61,    62,    63,    65,    72,    79,    80,    81,    82,    83,
      86,    87,    88,    89,    96,    97,   102,   105,   109,   110,
     111,   112,   114,   115,   119,   120,   121,   126,   127,   128,
     129,   130,   131,   135,   137,   139,   142,   146,   149,   101,
      30,   128,    30,    52,   129,   129,   129,   147,    30,   129,
     147,   128,   102,   129,   148,   138,   141,    53,    30,   132,
      46,    48,    49,    50,    51,   129,   136,    30,   129,    58,
      59,    15,    16,    30,    37,    30,    30,   129,    64,   129,
     136,    30,   134,   129,   129,   129,   129,   129,   129,    30,
     133,    30,   102,   129,   129,   129,    33,   107,   128,   150,
     151,   152,   153,     0,   101,   129,    68,   129,   113,   118,
     121,   129,   131,   146,   129,    66,    67,    68,    69,    70,
      71,    73,    76,    77,    78,    93,    94,    95,    96,    97,
      98,    99,   123,   102,   129,   104,   147,   129,   147,   104,
     147,   123,   104,   129,    48,   136,   140,    45,    74,    75,
      30,    56,    68,   128,    56,    68,    56,    55,    58,   136,
     104,    50,    56,    50,    56,    50,    56,    50,    56,    84,
      85,   104,    30,   103,   107,   106,   104,   104,   110,   124,
     129,   124,   124,   129,    56,   101,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,    30,   143,   129,   103,   129,   143,    30,
     129,   123,    30,   130,   129,   129,   129,   129,    56,    68,
     129,   129,   129,   129,    30,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,    30,   103,   104,   128,   128,
      33,   153,   125,   124,    55,   124,   129,   125,   101,   104,
     101,   143,   129,   129,    50,    50,    50,    50,   147,   101,
     114,   124,    38,    56,   125,    55,   124,   104,   145,    30,
     145,   101,   129,   129,   129,   129,   103,    91,    56,   129,
     124,   112,    92,    30,   125,   125,   145,   129,   124,   116,
     124,    34,   125,   124,   125,    39,    40,   117,   122,   124,
      39,    90,    30,   144,   125,   124,   123,   129,    90,   123,
     104,   124,    91,   125,   124,   112,    30,    91,   124,    55,
     124,    90,   125,    90,   124
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
#line 134 "engines/director/lingo/lingo-gr.y"
    { yyerrok; ;}
    break;

  case 8:
#line 142 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 9:
#line 148 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(LC::c_assign);
			(yyval.code) = (yyvsp[(2) - (4)].code); ;}
    break;

  case 10:
#line 151 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(LC::c_after); ;}
    break;

  case 11:
#line 152 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(LC::c_before); ;}
    break;

  case 12:
#line 153 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 13:
#line 159 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[1]);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 14:
#line 166 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 15:
#line 172 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[1]);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 16:
#line 179 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[1]);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 17:
#line 185 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[1]);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 18:
#line 191 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_objectfieldassign);
		g_lingo->codeString((yyvsp[(2) - (4)].objectfield).s->c_str());
		g_lingo->codeInt((yyvsp[(2) - (4)].objectfield).e);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 28:
#line 215 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(3) - (6)].code) - (yyvsp[(1) - (6)].code));
		WRITE_UINT32(&end, (yyvsp[(5) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 2] = end; ;}
    break;

  case 29:
#line 226 "engines/director/lingo/lingo-gr.y"
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
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 5] = end; ;}
    break;

  case 30:
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
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 5] = end; ;}
    break;

  case 31:
#line 255 "engines/director/lingo/lingo-gr.y"
    {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[(3) - (3)].code) - (yyvsp[(1) - (3)].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (3)].code) + 1] = end; ;}
    break;

  case 32:
#line 260 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented"); ;}
    break;

  case 33:
#line 262 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		;}
    break;

  case 34:
#line 267 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&end, (yyvsp[(8) - (9)].code) - (yyvsp[(1) - (9)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[(1) - (9)].code), (yyvsp[(8) - (9)].code) - (yyvsp[(1) - (9)].code), 0); ;}
    break;

  case 35:
#line 277 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (12)].code) - (yyvsp[(1) - (12)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (12)].code) - (yyvsp[(1) - (12)].code));
		WRITE_UINT32(&end, (yyvsp[(11) - (12)].code) - (yyvsp[(1) - (12)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[(1) - (12)].code), (yyvsp[(11) - (12)].code) - (yyvsp[(1) - (12)].code), (yyvsp[(9) - (12)].code) - (yyvsp[(1) - (12)].code)); ;}
    break;

  case 38:
#line 293 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 39:
#line 301 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (11)].code) - (yyvsp[(1) - (11)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (11)].code) - (yyvsp[(1) - (11)].code));
		WRITE_UINT32(&end, (yyvsp[(10) - (11)].code) - (yyvsp[(1) - (11)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[(1) - (11)].code), (yyvsp[(10) - (11)].code) - (yyvsp[(1) - (11)].code), (yyvsp[(8) - (11)].code) - (yyvsp[(1) - (11)].code)); ;}
    break;

  case 40:
#line 311 "engines/director/lingo/lingo-gr.y"
    {
			inst then = 0, else1 = 0, end = 0;
			WRITE_UINT32(&then, (yyvsp[(3) - (7)].code) - (yyvsp[(1) - (7)].code));
			WRITE_UINT32(&else1, 0);
			WRITE_UINT32(&end, (yyvsp[(6) - (7)].code) - (yyvsp[(1) - (7)].code));
			(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 1] = then;	/* thenpart */
			(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 2] = else1;	/* elsepart */
			(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 3] = end;	/* end, if cond fails */

			g_lingo->processIf((yyvsp[(1) - (7)].code), (yyvsp[(6) - (7)].code) - (yyvsp[(1) - (7)].code), (yyvsp[(6) - (7)].code) - (yyvsp[(1) - (7)].code)); ;}
    break;

  case 41:
#line 324 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(LC::c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 42:
#line 327 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(LC::c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 43:
#line 334 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 44:
#line 341 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(LC::c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 45:
#line 349 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 46:
#line 352 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 47:
#line 355 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 50:
#line 360 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(LC::c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 51:
#line 366 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(LC::c_tellcode);
		g_lingo->code1(STOP); ;}
    break;

  case 52:
#line 370 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[(1) - (1)].i)); ;}
    break;

  case 53:
#line 373 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 54:
#line 376 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 55:
#line 379 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 56:
#line 382 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 58:
#line 389 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(1) - (1)].code); ;}
    break;

  case 60:
#line 391 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 61:
#line 394 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 62:
#line 397 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 63:
#line 398 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 64:
#line 401 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 65:
#line 409 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 66:
#line 415 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_objectfieldpush);
		g_lingo->codeString((yyvsp[(1) - (1)].objectfield).s->c_str());
		g_lingo->codeInt((yyvsp[(1) - (1)].objectfield).e); ;}
    break;

  case 68:
#line 420 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_add); ;}
    break;

  case 69:
#line 421 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_sub); ;}
    break;

  case 70:
#line 422 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_mul); ;}
    break;

  case 71:
#line 423 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_div); ;}
    break;

  case 72:
#line 424 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_mod); ;}
    break;

  case 73:
#line 425 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_gt); ;}
    break;

  case 74:
#line 426 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_lt); ;}
    break;

  case 75:
#line 427 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_eq); ;}
    break;

  case 76:
#line 428 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_neq); ;}
    break;

  case 77:
#line 429 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_ge); ;}
    break;

  case 78:
#line 430 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_le); ;}
    break;

  case 79:
#line 431 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_and); ;}
    break;

  case 80:
#line 432 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_or); ;}
    break;

  case 81:
#line 433 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_not); ;}
    break;

  case 82:
#line 434 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_ampersand); ;}
    break;

  case 83:
#line 435 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_concat); ;}
    break;

  case 84:
#line 436 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_contains); ;}
    break;

  case 85:
#line 437 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_starts); ;}
    break;

  case 86:
#line 438 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 87:
#line 439 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(LC::c_negate); ;}
    break;

  case 88:
#line 440 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 89:
#line 441 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_intersects); ;}
    break;

  case 90:
#line 442 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_within); ;}
    break;

  case 91:
#line 443 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_charOf); ;}
    break;

  case 92:
#line 444 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_charToOf); ;}
    break;

  case 93:
#line 445 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_itemOf); ;}
    break;

  case 94:
#line 446 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_itemToOf); ;}
    break;

  case 95:
#line 447 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_lineOf); ;}
    break;

  case 96:
#line 448 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_lineToOf); ;}
    break;

  case 97:
#line 449 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_wordOf); ;}
    break;

  case 98:
#line 450 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_wordToOf); ;}
    break;

  case 99:
#line 451 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 100:
#line 452 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].narg)); ;}
    break;

  case 101:
#line 453 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe(nullptr, 0); ;}
    break;

  case 102:
#line 456 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 103:
#line 461 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_printtop); ;}
    break;

  case 106:
#line 464 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_exitRepeat); ;}
    break;

  case 107:
#line 465 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_procret); ;}
    break;

  case 111:
#line 469 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 112:
#line 472 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 113:
#line 475 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 114:
#line 478 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_voidpush);
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 1);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 115:
#line 482 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 116:
#line 483 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_open); ;}
    break;

  case 117:
#line 484 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(LC::c_voidpush, LC::c_open); ;}
    break;

  case 118:
#line 485 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[(1) - (3)].s)); s += '-'; s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 119:
#line 488 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 120:
#line 489 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 121:
#line 492 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_property); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 122:
#line 493 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_property); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 123:
#line 496 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 124:
#line 497 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 125:
#line 508 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_gotoloop); ;}
    break;

  case 126:
#line 509 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_gotonext); ;}
    break;

  case 127:
#line 510 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_gotoprevious); ;}
    break;

  case 128:
#line 511 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); ;}
    break;

  case 129:
#line 515 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); ;}
    break;

  case 130:
#line 519 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); ;}
    break;

  case 133:
#line 529 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(LC::c_playdone); ;}
    break;

  case 134:
#line 530 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); ;}
    break;

  case 135:
#line 534 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); ;}
    break;

  case 136:
#line 538 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); ;}
    break;

  case 137:
#line 542 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); ;}
    break;

  case 138:
#line 542 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[(1) - (3)].s), (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 139:
#line 572 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = kStateInArgs; g_lingo->_currentFactory.clear(); ;}
    break;

  case 140:
#line 573 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(LC::c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->clearArgStack();
			g_lingo->_indef = kStateNone; ;}
    break;

  case 141:
#line 578 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFactory(*(yyvsp[(2) - (2)].s)); ;}
    break;

  case 142:
#line 579 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = kStateInArgs; ;}
    break;

  case 143:
#line 580 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(LC::c_procret);
			g_lingo->define(*(yyvsp[(1) - (7)].s), (yyvsp[(3) - (7)].code), (yyvsp[(4) - (7)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->clearArgStack();
			g_lingo->_indef = kStateNone; ;}
    break;

  case 144:
#line 585 "engines/director/lingo/lingo-gr.y"
    {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[(1) - (8)].s), (yyvsp[(2) - (8)].code), (yyvsp[(3) - (8)].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[(7) - (8)].s), (yyvsp[(1) - (8)].s)->c_str(), false); ;}
    break;

  case 145:
#line 593 "engines/director/lingo/lingo-gr.y"
    {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->define(*(yyvsp[(1) - (6)].s), (yyvsp[(2) - (6)].code), (yyvsp[(3) - (6)].narg));
		g_lingo->_indef = kStateNone;
		g_lingo->clearArgStack();
		g_lingo->_ignoreMe = false; ;}
    break;

  case 146:
#line 600 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); g_lingo->_indef = kStateInArgs; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; ;}
    break;

  case 147:
#line 602 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 148:
#line 603 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 149:
#line 604 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 150:
#line 605 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 154:
#line 613 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); g_lingo->_indef = kStateInDef; ;}
    break;

  case 155:
#line 616 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[(1) - (2)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(2) - (2)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 156:
#line 624 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 157:
#line 625 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 158:
#line 626 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 159:
#line 629 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 160:
#line 630 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 161:
#line 633 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 162:
#line 636 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); ;}
    break;

  case 163:
#line 637 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); ;}
    break;

  case 164:
#line 638 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[(1) - (1)].narg)); ;}
    break;

  case 165:
#line 639 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[(1) - (1)].narg)); ;}
    break;

  case 166:
#line 642 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 167:
#line 643 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 168:
#line 646 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 169:
#line 647 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 170:
#line 650 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (3)].s)->c_str()); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3047 "engines/director/lingo/lingo-gr.cpp"
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


#line 656 "engines/director/lingo/lingo-gr.y"


