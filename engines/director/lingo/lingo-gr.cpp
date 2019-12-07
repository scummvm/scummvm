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
     tDOWN = 291,
     tELSE = 292,
     tELSIF = 293,
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
     tTO = 310,
     tWHEN = 311,
     tWITH = 312,
     tWHILE = 313,
     tNLELSE = 314,
     tFACTORY = 315,
     tMETHOD = 316,
     tOPEN = 317,
     tPLAY = 318,
     tDONE = 319,
     tINSTANCE = 320,
     tGE = 321,
     tLE = 322,
     tGT = 323,
     tLT = 324,
     tEQ = 325,
     tNEQ = 326,
     tAND = 327,
     tOR = 328,
     tNOT = 329,
     tMOD = 330,
     tAFTER = 331,
     tBEFORE = 332,
     tCONCAT = 333,
     tCONTAINS = 334,
     tSTARTS = 335,
     tCHAR = 336,
     tITEM = 337,
     tLINE = 338,
     tWORD = 339,
     tSPRITE = 340,
     tINTERSECTS = 341,
     tWITHIN = 342,
     tTELL = 343,
     tPROPERTY = 344,
     tON = 345,
     tME = 346,
     tENDIF = 347,
     tENDREPEAT = 348,
     tENDTELL = 349
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
#define tDOWN 291
#define tELSE 292
#define tELSIF 293
#define tEXIT 294
#define tFRAME 295
#define tGLOBAL 296
#define tGO 297
#define tIF 298
#define tINTO 299
#define tLOOP 300
#define tMACRO 301
#define tMOVIE 302
#define tNEXT 303
#define tOF 304
#define tPREVIOUS 305
#define tPUT 306
#define tREPEAT 307
#define tSET 308
#define tTHEN 309
#define tTO 310
#define tWHEN 311
#define tWITH 312
#define tWHILE 313
#define tNLELSE 314
#define tFACTORY 315
#define tMETHOD 316
#define tOPEN 317
#define tPLAY 318
#define tDONE 319
#define tINSTANCE 320
#define tGE 321
#define tLE 322
#define tGT 323
#define tLT 324
#define tEQ 325
#define tNEQ 326
#define tAND 327
#define tOR 328
#define tNOT 329
#define tMOD 330
#define tAFTER 331
#define tBEFORE 332
#define tCONCAT 333
#define tCONTAINS 334
#define tSTARTS 335
#define tCHAR 336
#define tITEM 337
#define tLINE 338
#define tWORD 339
#define tSPRITE 340
#define tINTERSECTS 341
#define tWITHIN 342
#define tTELL 343
#define tPROPERTY 344
#define tON 345
#define tME 346
#define tENDIF 347
#define tENDREPEAT 348
#define tENDTELL 349




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
#line 324 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 337 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  126
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1554

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  109
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  40
/* YYNRULES -- Number of rules.  */
#define YYNRULES  154
/* YYNRULES -- Number of states.  */
#define YYNSTATES  326

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   349

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     101,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   100,    95,     2,
     102,   103,    98,    96,   108,    97,     2,    99,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     105,     2,   104,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   106,     2,   107,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    13,    15,    17,    22,
      27,    32,    37,    42,    47,    52,    57,    63,    69,    71,
      73,    75,    77,    79,    86,    97,   109,   113,   120,   125,
     135,   148,   149,   152,   159,   162,   166,   168,   170,   171,
     172,   173,   176,   179,   183,   185,   187,   189,   191,   193,
     195,   197,   199,   201,   204,   207,   212,   214,   217,   219,
     223,   227,   231,   235,   239,   243,   247,   251,   255,   259,
     263,   267,   271,   274,   278,   282,   286,   290,   293,   296,
     300,   304,   309,   314,   319,   326,   331,   338,   343,   350,
     355,   362,   364,   367,   370,   372,   374,   377,   379,   382,
     385,   388,   390,   393,   396,   398,   401,   406,   413,   418,
     421,   425,   427,   431,   433,   437,   439,   443,   446,   449,
     452,   455,   459,   462,   465,   467,   471,   474,   477,   480,
     484,   487,   488,   492,   493,   502,   505,   506,   515,   524,
     531,   534,   535,   537,   541,   546,   547,   549,   553,   554,
     557,   558,   560,   564,   566
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     110,     0,    -1,   110,   101,   111,    -1,   111,    -1,     1,
     101,    -1,    -1,   139,    -1,   114,    -1,    51,   128,    44,
      30,    -1,    51,   128,    44,   129,    -1,    51,   128,    76,
     128,    -1,    51,   128,    77,   128,    -1,    53,    30,    70,
     128,    -1,    53,    15,    70,   128,    -1,    53,    30,    55,
     128,    -1,    53,    15,    55,   128,    -1,    53,    16,   128,
      55,   128,    -1,    53,    16,   127,    70,   128,    -1,   146,
      -1,   128,    -1,   130,    -1,   113,    -1,   115,    -1,   118,
     128,   123,   124,   123,    93,    -1,   119,    70,   128,   123,
      55,   128,   123,   124,   123,    93,    -1,   119,    70,   128,
     123,    36,    55,   128,   123,   124,   123,    93,    -1,   125,
     113,   123,    -1,   126,   128,   101,   124,   123,    94,    -1,
     126,   128,    55,   128,    -1,   120,   128,   123,    54,   124,
     123,   116,   123,    92,    -1,   120,   128,   123,    54,   124,
     123,   116,   123,    37,   124,   123,    92,    -1,    -1,   117,
     116,    -1,   121,   128,   123,    54,   124,   123,    -1,    52,
      58,    -1,    52,    57,    30,    -1,    43,    -1,    38,    -1,
      -1,    -1,    -1,   124,   101,    -1,   124,   114,    -1,    56,
      30,    54,    -1,    88,    -1,    12,    -1,    17,    -1,    33,
      -1,    31,    -1,    30,    -1,   127,    -1,   129,    -1,    25,
      -1,    26,   128,    -1,    27,   148,    -1,    30,   102,   147,
     103,    -1,    15,    -1,    16,   128,    -1,   112,    -1,   128,
      96,   128,    -1,   128,    97,   128,    -1,   128,    98,   128,
      -1,   128,    99,   128,    -1,   128,    75,   128,    -1,   128,
     104,   128,    -1,   128,   105,   128,    -1,   128,    70,   128,
      -1,   128,    71,   128,    -1,   128,    66,   128,    -1,   128,
      67,   128,    -1,   128,    72,   128,    -1,   128,    73,   128,
      -1,    74,   128,    -1,   128,    95,   128,    -1,   128,    78,
     128,    -1,   128,    79,   128,    -1,   128,    80,   128,    -1,
      96,   128,    -1,    97,   128,    -1,   102,   128,   103,    -1,
     106,   147,   107,    -1,    85,   128,    86,   128,    -1,    85,
     128,    87,   128,    -1,    81,   128,    49,   128,    -1,    81,
     128,    55,   128,    49,   128,    -1,    82,   128,    49,   128,
      -1,    82,   128,    55,   128,    49,   128,    -1,    83,   128,
      49,   128,    -1,    83,   128,    55,   128,    49,   128,    -1,
      84,   128,    49,   128,    -1,    84,   128,    55,   128,    49,
     128,    -1,    91,    -1,    29,   128,    -1,    51,   128,    -1,
     134,    -1,   137,    -1,    39,    52,    -1,    39,    -1,    41,
     131,    -1,    89,   132,    -1,    65,   133,    -1,    19,    -1,
      21,   128,    -1,    20,   128,    -1,    20,    -1,    22,   148,
      -1,    91,   102,    30,   103,    -1,    91,   102,    30,   108,
     147,   103,    -1,    62,   128,    57,   128,    -1,    62,   128,
      -1,    23,    30,   147,    -1,    30,    -1,   131,   108,    30,
      -1,    30,    -1,   132,   108,    30,    -1,    30,    -1,   133,
     108,    30,    -1,    42,    45,    -1,    42,    48,    -1,    42,
      50,    -1,    42,   135,    -1,    42,   135,   136,    -1,    42,
     136,    -1,    40,   128,    -1,   128,    -1,    49,    47,   128,
      -1,    47,   128,    -1,    63,    64,    -1,    63,   135,    -1,
      63,   135,   136,    -1,    63,   136,    -1,    -1,    35,   138,
     147,    -1,    -1,    46,    30,   140,   122,   143,   101,   145,
     124,    -1,    60,    30,    -1,    -1,    61,    30,   141,   122,
     143,   101,   145,   124,    -1,   142,   122,   143,   101,   145,
     124,    34,   144,    -1,   142,   122,   143,   101,   145,   124,
      -1,    90,    30,    -1,    -1,    30,    -1,   143,   108,    30,
      -1,   143,   101,   108,    30,    -1,    -1,    30,    -1,   144,
     108,    30,    -1,    -1,    30,   148,    -1,    -1,   128,    -1,
     147,   108,   128,    -1,   128,    -1,   148,   108,   128,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   120,   120,   121,   122,   125,   126,   127,   130,   136,
     139,   140,   141,   147,   154,   160,   167,   173,   181,   182,
     183,   186,   187,   192,   203,   220,   232,   237,   239,   244,
     254,   268,   269,   272,   280,   283,   290,   297,   305,   308,
     311,   312,   313,   316,   322,   326,   329,   332,   335,   338,
     344,   345,   346,   349,   352,   353,   356,   364,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   389,   390,   391,
     392,   393,   394,   395,   396,   397,   398,   399,   400,   401,
     402,   403,   406,   411,   412,   413,   414,   415,   416,   417,
     418,   419,   422,   425,   428,   432,   433,   434,   435,   436,
     437,   440,   441,   444,   445,   448,   449,   460,   461,   462,
     463,   467,   471,   477,   478,   481,   482,   485,   486,   490,
     494,   498,   498,   528,   528,   533,   534,   534,   539,   546,
     552,   554,   555,   556,   557,   560,   561,   562,   565,   569,
     577,   578,   579,   582,   583
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
  "SYMBOL", "ENDCLAUSE", "tPLAYACCEL", "tDOWN", "tELSE", "tELSIF", "tEXIT",
  "tFRAME", "tGLOBAL", "tGO", "tIF", "tINTO", "tLOOP", "tMACRO", "tMOVIE",
  "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO",
  "tWHEN", "tWITH", "tWHILE", "tNLELSE", "tFACTORY", "tMETHOD", "tOPEN",
  "tPLAY", "tDONE", "tINSTANCE", "tGE", "tLE", "tGT", "tLT", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tME", "tENDIF",
  "tENDREPEAT", "tENDTELL", "'&'", "'+'", "'-'", "'*'", "'/'", "'%'",
  "'\\n'", "'('", "')'", "'>'", "'<'", "'['", "']'", "','", "$accept",
  "program", "programline", "asgn", "stmtoneliner", "stmt", "ifstmt",
  "elseifstmtlist", "elseifstmt", "repeatwhile", "repeatwith", "if",
  "elseif", "begin", "end", "stmtlist", "when", "tell", "simpleexpr",
  "expr", "reference", "proc", "globallist", "propertylist",
  "instancelist", "gotofunc", "gotoframe", "gotomovie", "playfunc", "@1",
  "defn", "@2", "@3", "on", "argdef", "endargdef", "argstore", "macro",
  "arglist", "nonemptyarglist", 0
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
     345,   346,   347,   348,   349,    38,    43,    45,    42,    47,
      37,    10,    40,    41,    62,    60,    91,    93,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   109,   110,   110,   110,   111,   111,   111,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   113,   113,
     113,   114,   114,   114,   114,   114,   114,   114,   114,   115,
     115,   116,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   124,   124,   125,   126,   127,   127,   127,   127,   127,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   129,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   131,   131,   132,   132,   133,   133,   134,   134,   134,
     134,   134,   134,   135,   135,   136,   136,   137,   137,   137,
     137,   138,   137,   140,   139,   139,   141,   139,   139,   139,
     142,   143,   143,   143,   143,   144,   144,   144,   145,   146,
     147,   147,   147,   148,   148
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     0,     1,     1,     4,     4,
       4,     4,     4,     4,     4,     4,     5,     5,     1,     1,
       1,     1,     1,     6,    10,    11,     3,     6,     4,     9,
      12,     0,     2,     6,     2,     3,     1,     1,     0,     0,
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     4,     1,     2,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     3,     3,     3,     3,     2,     2,     3,
       3,     4,     4,     4,     6,     4,     6,     4,     6,     4,
       6,     1,     2,     2,     1,     1,     2,     1,     2,     2,
       2,     1,     2,     2,     1,     2,     4,     6,     4,     2,
       3,     1,     3,     1,     3,     1,     3,     2,     2,     2,
       2,     3,     2,     2,     1,     3,     2,     2,     2,     3,
       2,     0,     3,     0,     8,     2,     0,     8,     8,     6,
       2,     0,     1,     3,     4,     0,     1,     3,     0,     2,
       0,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    45,    56,     0,    46,   101,   104,     0,     0,
       0,    52,     0,     0,     0,    49,    48,    47,   131,    97,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    44,
       0,     0,    91,     0,     0,     0,   150,     0,     3,    58,
      21,     7,    22,     0,     0,     0,     0,     0,    50,    19,
      51,    20,    94,    95,     6,    38,    18,     4,    49,     0,
      91,    57,   103,   102,   153,   105,   150,    53,    54,    92,
     150,   149,   150,    96,   111,    98,     0,   117,     0,   118,
       0,   119,   124,   120,   122,   133,    93,     0,    34,     0,
       0,     0,     0,   135,   136,   109,   127,   128,   130,   115,
     100,    72,     0,     0,     0,     0,     0,   113,    99,   140,
       0,    77,    78,     0,   151,     0,     1,     5,    39,     0,
      39,    39,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     141,   150,     0,     0,   110,   151,     0,   132,     0,   123,
     126,     0,   121,    38,     0,     0,     0,    35,     0,     0,
      50,     0,     0,     0,    43,    38,     0,   129,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    79,    80,     0,     2,    40,    39,     0,    26,     0,
      40,    68,    69,    66,    67,    70,    71,    63,    74,    75,
      76,    73,    59,    60,    61,    62,    64,    65,   142,     0,
     154,    55,   112,   125,   141,     8,     9,    10,    11,    15,
      13,     0,     0,    14,    12,   141,   108,   116,    83,     0,
      85,     0,    87,     0,    89,     0,    81,    82,   114,   106,
     150,   152,    39,     0,    40,    28,    39,   148,     0,     0,
      17,    16,     0,     0,     0,     0,     0,     0,    41,    42,
       0,     0,     0,    39,     0,     0,    40,   143,   148,   148,
      84,    86,    88,    90,   107,    23,     0,    39,    31,    27,
     144,   139,    40,    40,    39,    40,    37,    39,    31,     0,
     145,   134,   137,    40,    39,     0,    32,    39,   146,   138,
      39,     0,    40,    29,     0,     0,     0,    24,    39,    40,
     147,    25,     0,    39,    30,    33
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    47,    48,    49,    50,   269,    52,   297,   298,    53,
      54,    55,   299,   150,   195,   252,    56,    57,    58,    59,
      60,    61,    85,   118,   110,    62,    93,    94,    63,    82,
      64,   163,   175,    65,   219,   309,   276,    66,   156,    75
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -173
static const yytype_int16 yypact[] =
{
     271,   -82,  -173,  -173,   821,  -173,  -173,   821,   821,   821,
      28,  -173,   821,   821,   821,   899,  -173,  -173,  -173,    12,
      51,   701,  -173,    55,   821,    11,    -2,    57,    59,    65,
     821,   779,    67,   821,   821,   821,   821,   821,   821,  -173,
      80,    84,    13,   821,   821,   821,   821,     2,  -173,  -173,
    -173,  -173,  -173,   821,    46,   821,   623,   821,  -173,  1413,
    -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,    15,   821,
    -173,  1413,  1413,  1413,  1413,    10,   821,  1413,    10,  1413,
     821,    10,   821,  -173,  -173,    14,   821,  -173,   821,  -173,
      73,  -173,  1413,     7,  -173,  -173,   941,    89,  -173,   -32,
     821,   -11,    69,  -173,  -173,  1301,  -173,     7,  -173,  -173,
      16,   -57,   977,  1013,  1049,  1085,  1337,  -173,    18,  -173,
      91,   -57,   -57,  1373,  1413,   -28,  -173,   359,  1413,   821,
    1413,  -173,   867,   821,   821,   821,   821,   821,   821,   821,
     821,   821,   821,   821,   821,   821,   821,   821,   821,   821,
      97,   821,   941,   821,    38,  1373,   -48,    38,   118,  1413,
    1413,   821,  -173,  -173,    72,   821,   821,  -173,   821,   821,
      81,  1265,   821,   821,  -173,  -173,   821,  -173,   120,   821,
     821,   821,   821,   821,   821,   821,   821,   821,   821,   123,
     -41,  -173,  -173,   821,  -173,  -173,  1413,   100,  -173,   821,
    -173,    -5,    -5,    -5,    -5,  1449,  1449,   -57,  1413,    -5,
      -5,     8,   125,   125,   -57,   -57,  1413,  1413,  -173,   -86,
    1413,  -173,  -173,  1413,    97,  -173,  -173,  1413,  1413,  1413,
      -5,   821,   821,  1413,    -5,    97,  1413,  -173,  1413,  1121,
    1413,  1157,  1413,  1193,  1413,  1229,  1413,  1413,  -173,  -173,
     821,  1413,   535,   -20,  -173,  1413,   535,    47,   126,   -83,
      -5,  1413,   -51,   821,   821,   821,   821,   -37,  -173,  -173,
      66,   103,   821,   535,    76,   130,  -173,  -173,    47,    47,
    1413,  1413,  1413,  1413,  -173,  -173,   821,  1413,   128,  -173,
    -173,   447,  -173,  -173,  1413,  -173,  -173,  -173,   128,   821,
     133,   535,   535,  -173,   535,   -31,  -173,  1413,  -173,    77,
     535,    93,  -173,  -173,   119,   157,    99,  -173,   535,  -173,
    -173,  -173,   102,   535,  -173,  -173
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -173,  -173,    70,  -173,   140,     1,  -173,  -100,  -173,  -173,
    -173,  -173,  -173,  -139,   -85,  -102,  -173,  -173,   104,    -4,
      35,  -173,  -173,  -173,  -173,  -173,   174,   -19,  -173,  -173,
    -173,  -173,  -173,  -173,  -172,  -173,  -170,  -173,   -39,    62
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -6
static const yytype_int16 yytable[] =
{
      71,    51,   126,    72,    73,    74,   312,   125,    77,    74,
      79,    74,   108,    99,   100,   257,   271,    92,   278,    67,
      96,   140,   258,   168,   224,   258,   105,    92,   101,   111,
     112,   113,   114,   115,   116,   272,   235,   154,   169,   121,
     122,   123,   124,   157,   172,   197,   198,   148,   149,   128,
     279,   130,   259,   132,    88,   221,    90,   258,    76,   173,
     193,   313,   249,   262,    83,   152,   284,   250,    97,    98,
     139,   193,   124,   140,   162,    78,   155,    81,   124,   192,
     193,    84,   159,   139,   160,    95,   140,   102,   177,   103,
     143,   144,   145,   146,   147,   104,   171,   109,   256,   148,
     149,    14,   225,   127,   144,   145,   146,   147,   292,   293,
     117,   253,   148,   149,   119,   120,   129,   151,   153,   167,
     161,   190,   158,   174,   178,   196,   189,   218,    51,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   193,   124,   222,   220,
     237,   231,   273,   248,   254,   275,   277,   223,   286,   285,
     290,   227,   228,   308,   229,   230,   296,   270,   233,   234,
     289,   274,   236,   319,   291,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   315,   317,   320,   288,   251,
     301,   302,   321,   304,   324,   255,   131,   194,   306,   226,
     139,   310,   295,   140,   170,   107,     0,     0,     0,   303,
     318,   267,   305,     0,     0,     0,     0,   323,     0,   311,
       0,     0,   314,   146,   147,   316,     0,   260,   261,   148,
     149,     0,     0,   322,     0,     0,     0,     0,   325,     0,
       0,     0,     0,     0,     0,     0,   124,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   280,
     281,   282,   283,     0,     0,     0,     0,     0,   287,     0,
       0,    -5,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   294,     2,     0,     0,     3,     4,     5,     0,
       6,     7,     8,     9,    10,   307,    11,    12,    13,     0,
      14,    15,    16,     0,    17,     0,    18,     0,     0,     0,
      19,     0,    20,    21,    22,     0,     0,    23,     0,     0,
       0,     0,    24,    25,    26,     0,     0,    27,     0,     0,
       0,    28,    29,    30,    31,     0,    32,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,     0,    39,
      40,    41,    42,     0,     0,     0,     0,    43,    44,     0,
       0,     2,    -5,    45,     3,     4,     5,    46,     6,     7,
       8,     9,    10,     0,    11,    12,    13,     0,    14,    15,
      16,     0,    17,     0,    18,     0,     0,     0,    19,     0,
      20,    21,    22,     0,     0,    23,     0,     0,     0,     0,
      24,    25,    26,     0,     0,    27,     0,     0,     0,    28,
      29,    30,    31,     0,    32,     0,     0,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,    39,    40,    41,
      42,     0,     0,     0,     0,    43,    44,     0,     0,     2,
       0,    45,     3,     4,     5,    46,     6,     7,     8,     9,
      10,     0,    11,    12,    13,     0,    14,    15,    16,     0,
      17,   300,    18,     0,     0,     0,    19,     0,    20,    21,
      22,     0,     0,     0,     0,     0,     0,     0,    24,    25,
      26,     0,     0,    27,     0,     0,     0,     0,     0,    30,
      31,     0,    32,     0,     0,     0,     0,     0,     0,     0,
       0,    33,     0,     0,     0,     0,     0,     0,    34,    35,
      36,    37,    38,     0,     0,    39,    40,     0,    42,     0,
       0,     0,     0,    43,    44,     0,     0,     2,   268,    45,
       3,     4,     5,    46,     6,     7,     8,     9,    10,     0,
      11,    12,    13,     0,    14,    15,    16,     0,    17,     0,
      18,     0,     0,     0,    19,     0,    20,    21,    22,     0,
       0,     0,     0,     0,     0,     0,    24,    25,    26,     0,
       0,    27,     0,     0,     0,     0,     0,    30,    31,     0,
      32,     0,     0,     0,     0,     0,     0,     0,     0,    33,
       0,     0,     0,     0,     0,     0,    34,    35,    36,    37,
      38,     0,     0,    39,    40,     0,    42,     0,     0,     0,
       0,    43,    44,     0,     0,     2,   268,    45,     3,     4,
       5,    46,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,    14,    15,    16,     0,    17,     0,    18,     0,
       0,     0,    19,     0,    20,    21,     0,     0,     0,     0,
       0,     0,     0,     0,    24,     0,    26,     0,     0,     0,
       0,     0,     0,     0,     0,    30,    31,     0,    32,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,     0,     0,    34,    35,    36,    37,    38,     0,
       0,     0,    40,     2,    42,     0,     3,     4,     5,    43,
      44,     0,     0,     0,     0,    45,    11,    12,    13,    46,
      14,    68,    16,     0,    17,     0,     0,     0,     0,     0,
       0,    86,     0,     0,     0,     0,    87,     0,    88,    89,
      90,    91,    69,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     2,    70,     0,     3,     4,     5,    43,    44,     0,
       0,     0,     0,    45,    11,    12,    13,    46,    14,    68,
      16,     0,    17,     0,     0,     0,     0,     0,     0,    86,
       0,     0,     0,     0,     0,     0,    88,     0,    90,     0,
      69,     0,    26,     2,     0,     0,     3,     4,     5,     0,
       0,     0,     0,   106,     0,     0,    11,    12,    13,     0,
      14,    68,    16,    33,    17,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      70,     0,    69,     0,    26,    43,    44,     0,     0,     0,
       0,    45,     0,     0,     0,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     2,    70,     0,     3,     4,     5,    43,    44,     0,
       0,     0,   199,    45,    11,    12,    13,    46,    14,    68,
      16,     0,    17,   133,   134,     0,     0,   135,   136,   137,
     138,     0,   139,     0,     0,   140,   141,   142,     0,     0,
      69,     0,    26,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   143,   144,   145,   146,   147,     0,   200,     0,
       0,   148,   149,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,   164,     0,     0,     0,     0,
      70,     0,     0,     0,     0,    43,    44,     0,     0,     0,
       0,    80,     0,     0,     0,    46,     0,   133,   134,     0,
       0,   135,   136,   137,   138,     0,   139,   165,   166,   140,
     141,   142,     0,     0,     0,     0,   179,     0,     0,     0,
       0,     0,   180,     0,     0,     0,   143,   144,   145,   146,
     147,     0,     0,   133,   134,   148,   149,   135,   136,   137,
     138,     0,   139,     0,     0,   140,   141,   142,     0,     0,
       0,     0,   181,     0,     0,     0,     0,     0,   182,     0,
       0,     0,   143,   144,   145,   146,   147,     0,     0,   133,
     134,   148,   149,   135,   136,   137,   138,     0,   139,     0,
       0,   140,   141,   142,     0,     0,     0,     0,   183,     0,
       0,     0,     0,     0,   184,     0,     0,     0,   143,   144,
     145,   146,   147,     0,     0,   133,   134,   148,   149,   135,
     136,   137,   138,     0,   139,     0,     0,   140,   141,   142,
       0,     0,     0,     0,   185,     0,     0,     0,     0,     0,
     186,     0,     0,     0,   143,   144,   145,   146,   147,     0,
       0,   133,   134,   148,   149,   135,   136,   137,   138,     0,
     139,     0,     0,   140,   141,   142,     0,     0,     0,     0,
     263,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     143,   144,   145,   146,   147,     0,     0,   133,   134,   148,
     149,   135,   136,   137,   138,     0,   139,     0,     0,   140,
     141,   142,     0,     0,     0,     0,   264,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   143,   144,   145,   146,
     147,     0,     0,   133,   134,   148,   149,   135,   136,   137,
     138,     0,   139,     0,     0,   140,   141,   142,     0,     0,
       0,     0,   265,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   143,   144,   145,   146,   147,     0,     0,   133,
     134,   148,   149,   135,   136,   137,   138,     0,   139,     0,
       0,   140,   141,   142,     0,     0,     0,     0,   266,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   144,
     145,   146,   147,     0,     0,   133,   134,   148,   149,   135,
     136,   137,   138,     0,   139,     0,     0,   140,   141,   142,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     232,     0,     0,     0,   143,   144,   145,   146,   147,     0,
       0,   133,   134,   148,   149,   135,   136,   137,   138,     0,
     139,     0,     0,   140,   141,   142,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   176,     0,
     143,   144,   145,   146,   147,     0,     0,   133,   134,   148,
     149,   135,   136,   137,   138,     0,   139,     0,     0,   140,
     141,   142,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   143,   144,   145,   146,
     147,     0,     0,   133,   134,   148,   149,   135,   136,   137,
     138,     0,   139,     0,     0,   140,   141,   142,     0,     0,
       0,     0,     0,   187,   188,     0,     0,     0,     0,     0,
       0,     0,   143,   144,   145,   146,   147,     0,     0,   133,
     134,   148,   149,   135,   136,   137,   138,     0,   139,     0,
       0,   140,   141,   142,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   144,
     145,   146,   147,     0,     0,     0,   191,   148,   149,   133,
     134,     0,     0,   135,   136,   137,   138,     0,   139,     0,
       0,   140,   141,   142,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   144,
     145,   146,   147,     0,     0,   133,   134,   148,   149,   135,
     136,     0,     0,     0,   139,     0,     0,   140,   141,   142,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,   144,   145,   146,   147,     0,
       0,     0,     0,   148,   149
};

static const yytype_int16 yycheck[] =
{
       4,     0,     0,     7,     8,     9,    37,    46,    12,    13,
      14,    15,    31,    15,    16,   101,    36,    21,   101,   101,
      24,    78,   108,    55,   163,   108,    30,    31,    30,    33,
      34,    35,    36,    37,    38,    55,   175,    76,    70,    43,
      44,    45,    46,    82,    55,   130,   131,   104,   105,    53,
     101,    55,   224,    57,    47,   103,    49,   108,    30,    70,
     108,    92,   103,   235,    52,    69,   103,   108,    57,    58,
      75,   108,    76,    78,    93,    13,    80,    15,    82,   107,
     108,    30,    86,    75,    88,    30,    78,    30,   107,    30,
      95,    96,    97,    98,    99,    30,   100,    30,   200,   104,
     105,    29,    30,   101,    96,    97,    98,    99,   278,   279,
      30,   196,   104,   105,    30,   102,    70,   102,   108,    30,
      47,    30,   108,    54,   108,   129,   108,    30,   127,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   108,   151,    30,   153,
      30,    70,   254,    30,    54,   108,    30,   161,    55,    93,
      30,   165,   166,    30,   168,   169,    38,   252,   172,   173,
      94,   256,   176,    54,   276,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   108,    93,    30,   273,   193,
     292,   293,    93,   295,    92,   199,    56,   127,   298,   164,
      75,   303,   287,    78,   100,    31,    -1,    -1,    -1,   294,
     312,   250,   297,    -1,    -1,    -1,    -1,   319,    -1,   304,
      -1,    -1,   307,    98,    99,   310,    -1,   231,   232,   104,
     105,    -1,    -1,   318,    -1,    -1,    -1,    -1,   323,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   250,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   263,
     264,   265,   266,    -1,    -1,    -1,    -1,    -1,   272,    -1,
      -1,     0,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   286,    12,    -1,    -1,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,   299,    25,    26,    27,    -1,
      29,    30,    31,    -1,    33,    -1,    35,    -1,    -1,    -1,
      39,    -1,    41,    42,    43,    -1,    -1,    46,    -1,    -1,
      -1,    -1,    51,    52,    53,    -1,    -1,    56,    -1,    -1,
      -1,    60,    61,    62,    63,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    82,    83,    84,    85,    -1,    -1,    88,
      89,    90,    91,    -1,    -1,    -1,    -1,    96,    97,    -1,
      -1,    12,   101,   102,    15,    16,    17,   106,    19,    20,
      21,    22,    23,    -1,    25,    26,    27,    -1,    29,    30,
      31,    -1,    33,    -1,    35,    -1,    -1,    -1,    39,    -1,
      41,    42,    43,    -1,    -1,    46,    -1,    -1,    -1,    -1,
      51,    52,    53,    -1,    -1,    56,    -1,    -1,    -1,    60,
      61,    62,    63,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    -1,    -1,    88,    89,    90,
      91,    -1,    -1,    -1,    -1,    96,    97,    -1,    -1,    12,
      -1,   102,    15,    16,    17,   106,    19,    20,    21,    22,
      23,    -1,    25,    26,    27,    -1,    29,    30,    31,    -1,
      33,    34,    35,    -1,    -1,    -1,    39,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,
      53,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    -1,    -1,    88,    89,    -1,    91,    -1,
      -1,    -1,    -1,    96,    97,    -1,    -1,    12,   101,   102,
      15,    16,    17,   106,    19,    20,    21,    22,    23,    -1,
      25,    26,    27,    -1,    29,    30,    31,    -1,    33,    -1,
      35,    -1,    -1,    -1,    39,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,    -1,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    82,    83,    84,
      85,    -1,    -1,    88,    89,    -1,    91,    -1,    -1,    -1,
      -1,    96,    97,    -1,    -1,    12,   101,   102,    15,    16,
      17,   106,    19,    20,    21,    22,    23,    -1,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    -1,    35,    -1,
      -1,    -1,    39,    -1,    41,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    82,    83,    84,    85,    -1,
      -1,    -1,    89,    12,    91,    -1,    15,    16,    17,    96,
      97,    -1,    -1,    -1,    -1,   102,    25,    26,    27,   106,
      29,    30,    31,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    45,    -1,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    82,    83,    84,    85,    -1,    -1,    -1,
      -1,    12,    91,    -1,    15,    16,    17,    96,    97,    -1,
      -1,    -1,    -1,   102,    25,    26,    27,   106,    29,    30,
      31,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    49,    -1,
      51,    -1,    53,    12,    -1,    -1,    15,    16,    17,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    25,    26,    27,    -1,
      29,    30,    31,    74,    33,    -1,    -1,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    51,    -1,    53,    96,    97,    -1,    -1,    -1,
      -1,   102,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    82,    83,    84,    85,    -1,    -1,    -1,
      -1,    12,    91,    -1,    15,    16,    17,    96,    97,    -1,
      -1,    -1,    55,   102,    25,    26,    27,   106,    29,    30,
      31,    -1,    33,    66,    67,    -1,    -1,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    78,    79,    80,    -1,    -1,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    95,    96,    97,    98,    99,    -1,   101,    -1,
      -1,   104,   105,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    44,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    96,    97,    -1,    -1,    -1,
      -1,   102,    -1,    -1,    -1,   106,    -1,    66,    67,    -1,
      -1,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    80,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    -1,    -1,    95,    96,    97,    98,
      99,    -1,    -1,    66,    67,   104,   105,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    78,    79,    80,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    95,    96,    97,    98,    99,    -1,    -1,    66,
      67,   104,   105,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    78,    79,    80,    -1,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    95,    96,
      97,    98,    99,    -1,    -1,    66,    67,   104,   105,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    78,    79,    80,
      -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    95,    96,    97,    98,    99,    -1,
      -1,    66,    67,   104,   105,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    78,    79,    80,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      95,    96,    97,    98,    99,    -1,    -1,    66,    67,   104,
     105,    70,    71,    72,    73,    -1,    75,    -1,    -1,    78,
      79,    80,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    97,    98,
      99,    -1,    -1,    66,    67,   104,   105,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    78,    79,    80,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    95,    96,    97,    98,    99,    -1,    -1,    66,
      67,   104,   105,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    78,    79,    80,    -1,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,
      97,    98,    99,    -1,    -1,    66,    67,   104,   105,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    95,    96,    97,    98,    99,    -1,
      -1,    66,    67,   104,   105,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    78,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      95,    96,    97,    98,    99,    -1,    -1,    66,    67,   104,
     105,    70,    71,    72,    73,    -1,    75,    -1,    -1,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    97,    98,
      99,    -1,    -1,    66,    67,   104,   105,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    78,    79,    80,    -1,    -1,
      -1,    -1,    -1,    86,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    95,    96,    97,    98,    99,    -1,    -1,    66,
      67,   104,   105,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,
      97,    98,    99,    -1,    -1,    -1,   103,   104,   105,    66,
      67,    -1,    -1,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,
      97,    98,    99,    -1,    -1,    66,    67,   104,   105,    70,
      71,    -1,    -1,    -1,    75,    -1,    -1,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    95,    96,    97,    98,    99,    -1,
      -1,    -1,    -1,   104,   105
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    15,    16,    17,    19,    20,    21,    22,
      23,    25,    26,    27,    29,    30,    31,    33,    35,    39,
      41,    42,    43,    46,    51,    52,    53,    56,    60,    61,
      62,    63,    65,    74,    81,    82,    83,    84,    85,    88,
      89,    90,    91,    96,    97,   102,   106,   110,   111,   112,
     113,   114,   115,   118,   119,   120,   125,   126,   127,   128,
     129,   130,   134,   137,   139,   142,   146,   101,    30,    51,
      91,   128,   128,   128,   128,   148,    30,   128,   148,   128,
     102,   148,   138,    52,    30,   131,    40,    45,    47,    48,
      49,    50,   128,   135,   136,    30,   128,    57,    58,    15,
      16,    30,    30,    30,    30,   128,    64,   135,   136,    30,
     133,   128,   128,   128,   128,   128,   128,    30,   132,    30,
     102,   128,   128,   128,   128,   147,     0,   101,   128,    70,
     128,   113,   128,    66,    67,    70,    71,    72,    73,    75,
      78,    79,    80,    95,    96,    97,    98,    99,   104,   105,
     122,   102,   128,   108,   147,   128,   147,   147,   108,   128,
     128,    47,   136,   140,    44,    76,    77,    30,    55,    70,
     127,   128,    55,    70,    54,   141,    57,   136,   108,    49,
      55,    49,    55,    49,    55,    49,    55,    86,    87,   108,
      30,   103,   107,   108,   111,   123,   128,   123,   123,    55,
     101,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,    30,   143,
     128,   103,    30,   128,   122,    30,   129,   128,   128,   128,
     128,    70,    55,   128,   128,   122,   128,    30,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,    30,   103,
     108,   128,   124,   123,    54,   128,   124,   101,   108,   143,
     128,   128,   143,    49,    49,    49,    49,   147,   101,   114,
     123,    36,    55,   124,   123,   108,   145,    30,   101,   101,
     128,   128,   128,   128,   103,    93,    55,   128,   123,    94,
      30,   124,   145,   145,   128,   123,    38,   116,   117,   121,
      34,   124,   124,   123,   124,   123,   116,   128,    30,   144,
     124,   123,    37,    92,   123,   108,   123,    93,   124,    54,
      30,    93,   123,   124,    92,   123
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

  case 8:
#line 130 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 9:
#line 136 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_assign);
			(yyval.code) = (yyvsp[(2) - (4)].code); ;}
    break;

  case 10:
#line 139 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_after); ;}
    break;

  case 11:
#line 140 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_before); ;}
    break;

  case 12:
#line 141 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 13:
#line 147 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[1]);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 14:
#line 154 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 15:
#line 160 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[1]);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 16:
#line 167 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[1]);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 17:
#line 173 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[1]);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 23:
#line 192 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(3) - (6)].code) - (yyvsp[(1) - (6)].code));
		WRITE_UINT32(&end, (yyvsp[(5) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 2] = end; ;}
    break;

  case 24:
#line 203 "engines/director/lingo/lingo-gr.y"
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

  case 25:
#line 220 "engines/director/lingo/lingo-gr.y"
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

  case 26:
#line 232 "engines/director/lingo/lingo-gr.y"
    {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[(3) - (3)].code) - (yyvsp[(1) - (3)].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (3)].code) + 1] = end; ;}
    break;

  case 27:
#line 237 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented"); ;}
    break;

  case 28:
#line 239 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		;}
    break;

  case 29:
#line 244 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&end, (yyvsp[(8) - (9)].code) - (yyvsp[(1) - (9)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf(0, (yyvsp[(8) - (9)].code) - (yyvsp[(1) - (9)].code)); ;}
    break;

  case 30:
#line 254 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (12)].code) - (yyvsp[(1) - (12)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (12)].code) - (yyvsp[(1) - (12)].code));
		WRITE_UINT32(&end, (yyvsp[(11) - (12)].code) - (yyvsp[(1) - (12)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 3] = end;	/* end, if cond fails */

		g_lingo->codeLabel((yyvsp[(8) - (12)].code));

		g_lingo->processIf(0, (yyvsp[(11) - (12)].code) - (yyvsp[(1) - (12)].code)); ;}
    break;

  case 33:
#line 272 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 34:
#line 280 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 35:
#line 283 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 36:
#line 290 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 37:
#line 297 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 38:
#line 305 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 39:
#line 308 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 40:
#line 311 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 43:
#line 316 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 44:
#line 322 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_tellcode);
		g_lingo->code1(STOP); ;}
    break;

  case 45:
#line 326 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt((yyvsp[(1) - (1)].i)); ;}
    break;

  case 46:
#line 329 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_floatpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 47:
#line 332 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 48:
#line 335 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 49:
#line 338 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 50:
#line 344 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(1) - (1)].code); ;}
    break;

  case 52:
#line 346 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 53:
#line 349 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 54:
#line 352 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 55:
#line 353 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 56:
#line 356 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 57:
#line 364 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 59:
#line 371 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 60:
#line 372 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 61:
#line 373 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 62:
#line 374 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 63:
#line 375 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); ;}
    break;

  case 64:
#line 376 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 65:
#line 377 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 66:
#line 378 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_eq); ;}
    break;

  case 67:
#line 379 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 68:
#line 380 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 69:
#line 381 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 70:
#line 382 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 71:
#line 383 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 72:
#line 384 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 73:
#line 385 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 74:
#line 386 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 75:
#line 387 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 76:
#line 388 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 77:
#line 389 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 78:
#line 390 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 79:
#line 391 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 80:
#line 392 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_arraypush); g_lingo->codeArray((yyvsp[(2) - (3)].narg)); ;}
    break;

  case 81:
#line 393 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 82:
#line 394 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 83:
#line 395 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); ;}
    break;

  case 84:
#line 396 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); ;}
    break;

  case 85:
#line 397 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); ;}
    break;

  case 86:
#line 398 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); ;}
    break;

  case 87:
#line 399 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); ;}
    break;

  case 88:
#line 400 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); ;}
    break;

  case 89:
#line 401 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); ;}
    break;

  case 90:
#line 402 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); ;}
    break;

  case 91:
#line 403 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe(nullptr, 0); ;}
    break;

  case 92:
#line 406 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 93:
#line 411 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 96:
#line 414 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); ;}
    break;

  case 97:
#line 415 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 101:
#line 419 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 102:
#line 422 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 103:
#line 425 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 104:
#line 428 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_voidpush);
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 1);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 105:
#line 432 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 106:
#line 433 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 107:
#line 434 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].narg)); ;}
    break;

  case 108:
#line 435 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 109:
#line 436 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 110:
#line 437 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[(1) - (3)].s)); s += '-'; s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 111:
#line 440 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 112:
#line 441 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 113:
#line 444 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 114:
#line 445 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 115:
#line 448 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 116:
#line 449 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 117:
#line 460 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 118:
#line 461 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 119:
#line 462 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 120:
#line 463 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 121:
#line 467 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 122:
#line 471 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 127:
#line 485 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 128:
#line 486 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 129:
#line 490 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 130:
#line 494 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 131:
#line 498 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); ;}
    break;

  case 132:
#line 498 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[(1) - (3)].s), (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 133:
#line 528 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 134:
#line 529 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 135:
#line 533 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFactory(*(yyvsp[(2) - (2)].s)); ;}
    break;

  case 136:
#line 534 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 137:
#line 535 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 138:
#line 539 "engines/director/lingo/lingo-gr.y"
    {	// D3
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(yyvsp[(1) - (8)].s), (yyvsp[(2) - (8)].code), (yyvsp[(3) - (8)].narg));
		g_lingo->_indef = false;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[(7) - (8)].s), (yyvsp[(1) - (8)].s)->c_str(), false); ;}
    break;

  case 139:
#line 546 "engines/director/lingo/lingo-gr.y"
    {	// D4. No 'end' clause
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(yyvsp[(1) - (6)].s), (yyvsp[(2) - (6)].code), (yyvsp[(3) - (6)].narg));
		g_lingo->_indef = false;
		g_lingo->_ignoreMe = false; ;}
    break;

  case 140:
#line 552 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); g_lingo->_indef = true; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; ;}
    break;

  case 141:
#line 554 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 142:
#line 555 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 143:
#line 556 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 144:
#line 557 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 148:
#line 565 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 149:
#line 569 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (2)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(2) - (2)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 150:
#line 577 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 151:
#line 578 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 152:
#line 579 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 153:
#line 582 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 154:
#line 583 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2907 "engines/director/lingo/lingo-gr.cpp"
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


#line 586 "engines/director/lingo/lingo-gr.y"


