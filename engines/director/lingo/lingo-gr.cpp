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
     INT = 266,
     THEENTITY = 267,
     THEENTITYWITHID = 268,
     FLOAT = 269,
     BLTIN = 270,
     BLTINNOARGS = 271,
     BLTINNOARGSORONE = 272,
     BLTINONEARG = 273,
     BLTINARGLIST = 274,
     ID = 275,
     STRING = 276,
     HANDLER = 277,
     SYMBOL = 278,
     tDOWN = 279,
     tELSE = 280,
     tNLELSIF = 281,
     tEND = 282,
     tEXIT = 283,
     tFRAME = 284,
     tGLOBAL = 285,
     tGO = 286,
     tIF = 287,
     tINTO = 288,
     tLOOP = 289,
     tMACRO = 290,
     tMOVIE = 291,
     tNEXT = 292,
     tOF = 293,
     tPREVIOUS = 294,
     tPUT = 295,
     tREPEAT = 296,
     tSET = 297,
     tTHEN = 298,
     tTO = 299,
     tWHEN = 300,
     tWITH = 301,
     tWHILE = 302,
     tNLELSE = 303,
     tFACTORY = 304,
     tMETHOD = 305,
     tOPEN = 306,
     tPLAY = 307,
     tDONE = 308,
     tPLAYACCEL = 309,
     tINSTANCE = 310,
     tGE = 311,
     tLE = 312,
     tGT = 313,
     tLT = 314,
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
     tON = 333,
     tSOUND = 334
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
#define INT 266
#define THEENTITY 267
#define THEENTITYWITHID 268
#define FLOAT 269
#define BLTIN 270
#define BLTINNOARGS 271
#define BLTINNOARGSORONE 272
#define BLTINONEARG 273
#define BLTINARGLIST 274
#define ID 275
#define STRING 276
#define HANDLER 277
#define SYMBOL 278
#define tDOWN 279
#define tELSE 280
#define tNLELSIF 281
#define tEND 282
#define tEXIT 283
#define tFRAME 284
#define tGLOBAL 285
#define tGO 286
#define tIF 287
#define tINTO 288
#define tLOOP 289
#define tMACRO 290
#define tMOVIE 291
#define tNEXT 292
#define tOF 293
#define tPREVIOUS 294
#define tPUT 295
#define tREPEAT 296
#define tSET 297
#define tTHEN 298
#define tTO 299
#define tWHEN 300
#define tWITH 301
#define tWHILE 302
#define tNLELSE 303
#define tFACTORY 304
#define tMETHOD 305
#define tOPEN 306
#define tPLAY 307
#define tDONE 308
#define tPLAYACCEL 309
#define tINSTANCE 310
#define tGE 311
#define tLE 312
#define tGT 313
#define tLT 314
#define tEQ 315
#define tNEQ 316
#define tAND 317
#define tOR 318
#define tNOT 319
#define tMOD 320
#define tAFTER 321
#define tBEFORE 322
#define tCONCAT 323
#define tCONTAINS 324
#define tSTARTS 325
#define tCHAR 326
#define tITEM 327
#define tLINE 328
#define tWORD 329
#define tSPRITE 330
#define tINTERSECTS 331
#define tWITHIN 332
#define tON 333
#define tSOUND 334




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
#line 69 "engines/director/lingo/lingo-gr.y"
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
#line 284 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 297 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  105
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1550

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  93
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  39
/* YYNRULES -- Number of rules.  */
#define YYNRULES  146
/* YYNRULES -- Number of states.  */
#define YYNSTATES  321

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   334

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      87,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    86,    81,     2,
      88,    89,    84,    82,    92,    83,     2,    85,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      91,    80,    90,     2,     2,     2,     2,     2,     2,     2,
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
      75,    76,    77,    78,    79
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    14,    15,    17,    19,
      21,    23,    25,    30,    35,    40,    45,    50,    56,    61,
      66,    72,    74,    76,    78,    80,    89,   101,   114,   118,
     127,   139,   151,   158,   169,   180,   181,   185,   188,   190,
     193,   195,   202,   204,   210,   212,   216,   220,   223,   227,
     229,   231,   232,   233,   234,   237,   240,   244,   246,   248,
     250,   252,   254,   259,   261,   263,   266,   268,   272,   276,
     280,   284,   288,   292,   296,   300,   304,   308,   312,   316,
     319,   323,   327,   331,   335,   339,   342,   345,   349,   354,
     359,   364,   371,   376,   383,   388,   395,   400,   407,   410,
     412,   414,   417,   419,   422,   425,   428,   431,   433,   436,
     441,   444,   448,   450,   454,   456,   460,   463,   466,   469,
     472,   476,   479,   482,   484,   488,   491,   494,   497,   501,
     504,   505,   514,   517,   518,   527,   528,   529,   540,   541,
     543,   547,   552,   553,   557,   558,   560
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      94,     0,    -1,    94,    95,    96,    -1,    96,    -1,     1,
      95,    -1,    87,    -1,    -1,   123,    -1,   116,    -1,   130,
      -1,    97,    -1,    99,    -1,    40,   115,    33,    20,    -1,
      40,   115,    66,   115,    -1,    40,   115,    67,   115,    -1,
      42,    20,    80,   115,    -1,    42,    12,    80,   115,    -1,
      42,    13,   115,    80,   115,    -1,    42,    20,    44,   115,
      -1,    42,    12,    44,   115,    -1,    42,    13,   115,    44,
     115,    -1,   115,    -1,   116,    -1,    98,    -1,   100,    -1,
     107,    88,   106,    89,   113,   112,    27,    41,    -1,   108,
      80,   115,   112,    44,   115,   112,   113,   112,    27,    41,
      -1,   108,    80,   115,   112,    24,    44,   115,   112,   113,
     112,    27,    41,    -1,   114,   115,   112,    -1,   109,   106,
      43,    95,   113,   112,    27,    32,    -1,   109,   106,    43,
      95,   113,   112,    48,   113,   112,    27,    32,    -1,   109,
     106,    43,    95,   113,   112,   111,   102,   112,    27,    32,
      -1,   109,   106,    43,   111,    98,   112,    -1,   109,   106,
      43,   111,    98,   112,    48,   111,    98,   112,    -1,   109,
     106,    43,   111,    98,   112,   103,   112,   101,   112,    -1,
      -1,    48,   111,    98,    -1,   102,   105,    -1,   105,    -1,
     103,   104,    -1,   104,    -1,   110,   106,    43,   111,    99,
     112,    -1,   103,    -1,   110,   106,    43,   113,   112,    -1,
     115,    -1,   115,    80,   115,    -1,    88,   106,    89,    -1,
      41,    47,    -1,    41,    46,    20,    -1,    32,    -1,    26,
      -1,    -1,    -1,    -1,   113,    95,    -1,   113,    99,    -1,
      45,    20,    43,    -1,    11,    -1,    14,    -1,    23,    -1,
      21,    -1,    16,    -1,    20,    88,   131,    89,    -1,    20,
      -1,    12,    -1,    13,   115,    -1,    97,    -1,   115,    82,
     115,    -1,   115,    83,   115,    -1,   115,    84,   115,    -1,
     115,    85,   115,    -1,   115,    65,   115,    -1,   115,    90,
     115,    -1,   115,    91,   115,    -1,   115,    61,   115,    -1,
     115,    56,   115,    -1,   115,    57,   115,    -1,   115,    62,
     115,    -1,   115,    63,   115,    -1,    64,   115,    -1,   115,
      81,   115,    -1,   115,    66,   115,    -1,   115,    68,   115,
      -1,   115,    69,   115,    -1,   115,    70,   115,    -1,    82,
     115,    -1,    83,   115,    -1,    88,   115,    89,    -1,    75,
     115,    76,   115,    -1,    75,   115,    77,   115,    -1,    71,
     115,    38,   115,    -1,    71,   115,    44,   115,    38,   115,
      -1,    72,   115,    38,   115,    -1,    72,   115,    44,   115,
      38,   115,    -1,    73,   115,    38,   115,    -1,    73,   115,
      44,   115,    38,   115,    -1,    74,   115,    38,   115,    -1,
      74,   115,    44,   115,    38,   115,    -1,    40,   115,    -1,
     119,    -1,   122,    -1,    28,    41,    -1,    28,    -1,    30,
     117,    -1,    55,   118,    -1,    18,   115,    -1,    17,   115,
      -1,    17,    -1,    19,   131,    -1,    51,   115,    46,   115,
      -1,    51,   115,    -1,    79,    20,   131,    -1,    20,    -1,
     117,    92,    20,    -1,    20,    -1,   118,    92,    20,    -1,
      31,    34,    -1,    31,    37,    -1,    31,    39,    -1,    31,
     120,    -1,    31,   120,   121,    -1,    31,   121,    -1,    29,
     115,    -1,   115,    -1,    38,    36,   115,    -1,    36,   115,
      -1,    52,    53,    -1,    52,   120,    -1,    52,   120,   121,
      -1,    52,   121,    -1,    -1,    35,    20,   124,   111,   128,
      95,   129,   113,    -1,    49,    20,    -1,    -1,    50,    20,
     125,   111,   128,    95,   129,   113,    -1,    -1,    -1,    78,
      20,   126,   111,   127,   128,    95,   129,   113,    27,    -1,
      -1,    20,    -1,   128,    92,    20,    -1,   128,    95,    92,
      20,    -1,    -1,    20,   111,   131,    -1,    -1,   115,    -1,
     131,    92,   115,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   106,   106,   107,   108,   111,   116,   117,   118,   119,
     120,   121,   124,   130,   131,   132,   138,   146,   154,   160,
     168,   177,   178,   180,   181,   186,   197,   213,   225,   233,
     240,   249,   258,   268,   278,   289,   290,   293,   294,   297,
     298,   301,   309,   310,   318,   319,   320,   322,   324,   330,
     336,   343,   345,   347,   348,   349,   352,   358,   359,   362,
     365,   368,   371,   374,   378,   385,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   425,   426,
     427,   428,   429,   431,   432,   433,   436,   439,   442,   443,
     444,   445,   448,   449,   452,   453,   464,   465,   466,   467,
     470,   473,   478,   479,   482,   483,   486,   487,   490,   493,
     523,   523,   529,   532,   532,   537,   538,   537,   550,   551,
     552,   553,   555,   559,   567,   568,   569
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNARY", "CASTREF", "VOID", "VAR",
  "POINT", "RECT", "ARRAY", "OBJECT", "INT", "THEENTITY",
  "THEENTITYWITHID", "FLOAT", "BLTIN", "BLTINNOARGS", "BLTINNOARGSORONE",
  "BLTINONEARG", "BLTINARGLIST", "ID", "STRING", "HANDLER", "SYMBOL",
  "tDOWN", "tELSE", "tNLELSIF", "tEND", "tEXIT", "tFRAME", "tGLOBAL",
  "tGO", "tIF", "tINTO", "tLOOP", "tMACRO", "tMOVIE", "tNEXT", "tOF",
  "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH",
  "tWHILE", "tNLELSE", "tFACTORY", "tMETHOD", "tOPEN", "tPLAY", "tDONE",
  "tPLAYACCEL", "tINSTANCE", "tGE", "tLE", "tGT", "tLT", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tON", "tSOUND", "'='", "'&'", "'+'", "'-'",
  "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "'>'", "'<'", "','",
  "$accept", "program", "nl", "programline", "asgn", "stmtoneliner",
  "stmt", "ifstmt", "elsestmtoneliner", "elseifstmt", "elseifstmtoneliner",
  "elseifstmtoneliner1", "elseifstmt1", "cond", "repeatwhile",
  "repeatwith", "if", "elseif", "begin", "end", "stmtlist", "when", "expr",
  "func", "globallist", "instancelist", "gotofunc", "gotoframe",
  "gotomovie", "playfunc", "defn", "@1", "@2", "@3", "@4", "argdef",
  "argstore", "macro", "arglist", 0
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
      61,    38,    43,    45,    42,    47,    37,    10,    40,    41,
      62,    60,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    93,    94,    94,    94,    95,    96,    96,    96,    96,
      96,    96,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    98,    98,    99,    99,    99,    99,    99,    99,   100,
     100,   100,   100,   100,   100,   101,   101,   102,   102,   103,
     103,   104,   105,   105,   106,   106,   106,   107,   108,   109,
     110,   111,   112,   113,   113,   113,   114,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   117,   117,   118,   118,   119,   119,   119,   119,
     119,   119,   120,   120,   121,   121,   122,   122,   122,   122,
     124,   123,   123,   125,   123,   126,   127,   123,   128,   128,
     128,   128,   129,   130,   131,   131,   131
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     0,     1,     1,     1,
       1,     1,     4,     4,     4,     4,     4,     5,     4,     4,
       5,     1,     1,     1,     1,     8,    11,    12,     3,     8,
      11,    11,     6,    10,    10,     0,     3,     2,     1,     2,
       1,     6,     1,     5,     1,     3,     3,     2,     3,     1,
       1,     0,     0,     0,     2,     2,     3,     1,     1,     1,
       1,     1,     4,     1,     1,     2,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     3,     2,     2,     3,     4,     4,
       4,     6,     4,     6,     4,     6,     4,     6,     2,     1,
       1,     2,     1,     2,     2,     2,     2,     1,     2,     4,
       2,     3,     1,     3,     1,     3,     2,     2,     2,     2,
       3,     2,     2,     1,     3,     2,     2,     2,     3,     2,
       0,     8,     2,     0,     8,     0,     0,    10,     0,     1,
       3,     4,     0,     3,     0,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    57,    64,     0,    58,    61,   107,     0,   144,
      51,    60,    59,   102,     0,     0,    49,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     3,
      66,    23,    11,    24,     0,     0,     0,     0,    21,     8,
      99,   100,     7,     9,     5,     4,    63,     0,    66,    65,
     106,   105,   145,   108,   144,   144,   101,   112,   103,     0,
     116,     0,   117,     0,   118,   123,   119,   121,   130,    98,
       0,    47,     0,     0,     0,     0,   132,   133,   110,   126,
     127,   129,   114,   104,    79,     0,     0,     0,     0,     0,
     135,   144,    85,    86,     0,     1,     6,     0,     0,     0,
       0,    44,    52,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   143,     0,   122,   125,     0,   120,    51,
       0,     0,     0,    48,     0,     0,     0,     0,     0,    56,
      51,     0,   128,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,   111,    87,     2,     0,    52,
       0,     0,    51,     0,    28,    75,    76,    74,    77,    78,
      71,    81,    82,    83,    84,    80,    67,    68,    69,    70,
      72,    73,   146,    62,   113,   124,   138,    12,    13,    14,
      19,    16,     0,     0,    18,    15,   138,   109,   115,    90,
       0,    92,     0,    94,     0,    96,     0,    88,    89,   136,
      53,     0,    46,    53,     0,    45,   139,     0,    20,    17,
       0,     0,     0,     0,     0,   138,    52,     0,     0,    52,
      52,    22,     0,   142,   142,    91,    93,    95,    97,     0,
      54,    55,     0,     0,    52,    51,    32,   140,     0,    53,
      53,   142,     0,    52,    53,     0,    53,     0,    50,    51,
      52,    40,     0,   141,   131,   134,    53,    25,    53,    52,
      29,    52,    52,    42,    38,     0,     0,    39,    35,     0,
       0,    52,     0,     0,    37,     0,     0,    52,    51,    52,
      51,   137,     0,     0,     0,     0,    51,    33,     0,    34,
       0,     0,    26,    30,    31,    52,    36,    52,    27,    43,
      41
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    38,   250,    39,    58,    41,   251,    43,   299,   282,
     283,   271,   284,   110,    44,    45,    46,   272,   310,   174,
     236,    47,    48,   241,    68,    93,    50,    76,    77,    51,
      52,   139,   150,   164,   235,   227,   259,    53,    63
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -244
static const yytype_int16 yypact[] =
{
     300,   -52,  -244,  -244,   862,  -244,  -244,   862,   862,   862,
    1459,  -244,  -244,    -5,    17,   753,  -244,    34,   862,   -17,
      13,    38,    46,    54,   862,   826,    55,   862,   862,   862,
     862,   862,   862,    58,    65,   862,   862,   862,     3,  -244,
       5,  -244,  -244,  -244,   -16,     6,   899,   862,  1428,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,     7,   862,  -244,  1428,
    1428,  1428,  1428,     8,   862,   862,  -244,  -244,     9,   862,
    -244,   862,  -244,    60,  -244,  1428,    26,  -244,  -244,   923,
      74,  -244,   -30,   862,   -28,    59,  -244,  -244,  1273,  -244,
      26,  -244,  -244,    11,   -49,   959,   995,  1031,  1067,  1304,
    -244,   862,   -49,   -49,  1366,  -244,   378,   899,   862,   899,
      61,  1397,  1428,   862,   862,   862,   862,   862,   862,   862,
     862,   862,   862,   862,   862,   862,   862,   862,   862,   862,
     923,   862,   -58,     8,    77,  1428,  1428,   862,  -244,  -244,
      85,   862,   862,  -244,   862,   862,  1242,   862,   862,  -244,
    -244,   862,  -244,    87,   862,   862,   862,   862,   862,   862,
     862,   862,   862,   862,  -244,     8,  -244,  -244,    22,  1428,
      23,  1335,   -52,   862,  -244,   166,   166,   166,   -49,   -49,
     -49,  1428,  1428,   166,   166,   715,   203,   203,   -49,   -49,
    1428,  1428,  1428,  -244,  -244,  1428,    93,  -244,  1428,  1428,
    1428,  1428,   862,   862,  1428,  1428,    93,  1428,  -244,  1428,
    1103,  1428,  1139,  1428,  1175,  1428,  1211,  1428,  1428,  -244,
    -244,    -1,  -244,  -244,   680,  1428,  -244,   -60,  1428,  1428,
     -60,   862,   862,   862,   862,    93,   529,    70,   862,   529,
    -244,  -244,    95,    27,    27,  1428,  1428,  1428,  1428,   -60,
    -244,  -244,    91,   862,  1428,     1,    -8,  -244,   100,  -244,
    -244,    27,    81,  1428,  -244,    92,  -244,    99,  -244,  -244,
      99,  -244,   899,  -244,   529,   529,  -244,  -244,  -244,   529,
    -244,   529,    99,    99,  -244,   899,   680,  -244,    78,    84,
     451,   529,   101,   102,  -244,   104,    89,  -244,  -244,  -244,
    -244,  -244,   106,    96,   107,   108,   -14,  -244,   680,  -244,
     607,    97,  -244,  -244,  -244,   529,  -244,  -244,  -244,  -244,
    -244
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -244,  -244,    10,    28,     2,  -217,     0,  -244,  -244,  -244,
    -120,  -215,  -140,   -87,  -244,  -244,  -244,  -243,    -9,   -18,
    -213,  -244,    52,     4,  -244,  -244,  -244,   118,   -19,  -244,
    -244,  -244,  -244,  -244,  -244,  -191,  -223,  -244,   -56
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -64
static const yytype_int16 yytable[] =
{
      42,    65,    40,   105,    49,   -10,    91,   240,   132,   133,
     239,    55,   -53,   -53,   144,   230,   147,   119,   268,   120,
     168,   260,   170,   237,   285,    82,    83,    54,   265,    80,
      81,   193,   242,    84,   131,    54,    66,    67,   276,   285,
     269,   128,   129,   238,   249,   165,   274,   275,   106,   266,
     145,   279,   148,   281,    78,   287,    59,   138,    85,    60,
      61,    62,    71,   290,    73,   291,    86,    75,   287,   297,
      79,   152,   107,   -53,    87,    92,    88,    75,   100,    94,
      95,    96,    97,    98,    99,   101,   108,   102,   103,   104,
      54,   316,   -10,   315,   143,    64,   137,   194,   111,   112,
     131,   134,   149,   153,   172,   197,    42,   208,    40,   130,
      49,   220,   222,   226,   253,   257,    62,    62,   262,   258,
     273,   135,   277,   136,   280,   268,   298,   300,   303,   304,
     196,   305,   306,   311,   167,   146,   270,   312,   318,   313,
     314,   206,   294,    90,     0,     0,     0,     0,     0,     0,
       0,   221,     0,    62,     0,   219,     0,     0,     0,   111,
     169,   171,     0,   224,     0,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   223,   192,     0,   289,     0,     0,     0,   195,
       0,     0,     0,   198,   199,     0,   200,   201,   296,   204,
     205,     0,     0,   207,     0,     0,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,     0,     0,   252,     0,
       0,   255,   256,     0,     0,   225,     0,     0,   116,   117,
       0,   118,   119,     0,   120,     0,   264,   243,     0,     0,
     244,     0,     0,     0,     0,   278,   267,   123,   124,   125,
     126,   127,   288,     0,   228,   229,   128,   129,     0,   261,
     286,   292,     0,   293,   295,   116,   117,     0,   118,   119,
       0,   120,     0,   302,     0,     0,     0,     0,     0,   307,
       0,   309,     0,   245,   246,   247,   248,   126,   127,   308,
     254,     0,     0,   128,   129,     0,     0,   319,     0,   320,
      -6,     1,     0,     0,     0,   263,     0,     0,     0,     0,
     317,     2,     3,     4,     5,     0,     6,     7,     8,     9,
      10,    11,     0,    12,   111,     0,     0,     0,    13,     0,
      14,    15,    16,     0,     0,    17,     0,   111,     0,     0,
      18,    19,    20,     0,     0,    21,     0,     0,     0,    22,
      23,    24,    25,     0,     0,    26,     0,     0,     0,     0,
       0,     0,     0,     0,    27,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,     0,     0,    33,    34,
       0,     0,    35,    36,     0,     0,     0,    -6,    37,     2,
       3,     4,     5,     0,     6,     7,     8,     9,    10,    11,
       0,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,     0,     0,    17,     0,     0,     0,     0,    18,    19,
      20,     0,     0,    21,     0,     0,     0,    22,    23,    24,
      25,     0,     0,    26,     0,     0,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,    28,
      29,    30,    31,    32,     0,     0,    33,    34,     0,     0,
      35,    36,     2,     3,     4,     5,    37,     6,     7,     8,
       9,    56,    11,     0,    12,     0,     0,     0,   301,    13,
       0,    14,    15,    16,     0,     0,     0,     0,     0,     0,
       0,    18,    19,    20,     0,     0,    21,     0,     0,     0,
       0,     0,    24,    25,     0,     0,    26,     0,     0,     0,
       0,     0,     0,     0,     0,    27,     0,     0,     0,     0,
       0,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      34,     0,     0,    35,    36,     0,     0,     0,    54,    37,
       2,     3,     4,     5,     0,     6,     7,     8,     9,    56,
      11,     0,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,     0,     0,     0,     0,     0,     0,     0,    18,
      19,    20,     0,     0,    21,     0,     0,     0,     0,     0,
      24,    25,     0,     0,    26,     0,     0,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    34,     0,
       0,    35,    36,     0,     0,     0,    54,    37,     2,     3,
       4,     5,     0,     6,     7,     8,     9,    56,    11,     0,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    20,
       0,     0,    21,     0,     0,     0,     0,     0,    24,    25,
       0,     0,    26,     0,     0,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    34,     0,     0,    35,
      36,     2,     3,     4,     5,    37,     6,     7,     8,     9,
      56,    11,     0,    12,     0,     0,     0,     0,    13,     0,
      14,    15,     0,     0,     0,     0,     0,     0,     0,     0,
      18,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,    24,    25,     0,     0,    26,     0,     0,     0,     0,
       0,     0,     0,     0,    27,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    34,
       0,     0,    35,    36,     2,     3,     4,     5,    37,     6,
       0,     0,     0,    56,    11,     0,    12,   116,   117,     0,
     118,   119,    69,   120,     0,     0,     0,    70,     0,    71,
      72,    73,    74,    57,     0,    20,     0,   124,   125,   126,
     127,     0,     0,     0,     0,   128,   129,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,    28,    29,    30,    31,    32,     0,
       0,     0,     0,     0,     0,    35,    36,     2,     3,     4,
       5,    37,     6,     0,     0,     0,    56,    11,     0,    12,
       0,     0,     0,     0,     0,    69,     0,     0,     0,     0,
       0,     0,    71,     0,    73,     0,    57,     0,    20,     0,
       0,     0,     0,     2,     3,     4,     5,     0,     6,    89,
       0,     0,    56,    11,     0,    12,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,    28,    29,    30,
      31,    32,    57,     0,    20,     0,     0,     0,    35,    36,
       2,     3,     4,     5,    37,     6,     0,     0,     0,    56,
      11,     0,    12,     0,     0,     0,    27,     0,     0,     0,
       0,     0,     0,    28,    29,    30,    31,    32,     0,    57,
       0,    20,     0,     0,    35,    36,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,   140,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
      28,    29,    30,    31,    32,     0,     0,     0,     0,   113,
     114,    35,    36,     0,   115,   116,   117,   109,   118,   141,
     142,   120,   121,   122,     0,     0,     0,   154,     0,     0,
       0,     0,     0,   155,   123,   124,   125,   126,   127,     0,
       0,     0,     0,   128,   129,   113,   114,     0,     0,     0,
     115,   116,   117,     0,   118,   119,     0,   120,   121,   122,
       0,     0,     0,   156,     0,     0,     0,     0,     0,   157,
     123,   124,   125,   126,   127,     0,     0,     0,     0,   128,
     129,   113,   114,     0,     0,     0,   115,   116,   117,     0,
     118,   119,     0,   120,   121,   122,     0,     0,     0,   158,
       0,     0,     0,     0,     0,   159,   123,   124,   125,   126,
     127,     0,     0,     0,     0,   128,   129,   113,   114,     0,
       0,     0,   115,   116,   117,     0,   118,   119,     0,   120,
     121,   122,     0,     0,     0,   160,     0,     0,     0,     0,
       0,   161,   123,   124,   125,   126,   127,     0,     0,     0,
       0,   128,   129,   113,   114,     0,     0,     0,   115,   116,
     117,     0,   118,   119,     0,   120,   121,   122,     0,     0,
       0,   231,     0,     0,     0,     0,     0,     0,   123,   124,
     125,   126,   127,     0,     0,     0,     0,   128,   129,   113,
     114,     0,     0,     0,   115,   116,   117,     0,   118,   119,
       0,   120,   121,   122,     0,     0,     0,   232,     0,     0,
       0,     0,     0,     0,   123,   124,   125,   126,   127,     0,
       0,     0,     0,   128,   129,   113,   114,     0,     0,     0,
     115,   116,   117,     0,   118,   119,     0,   120,   121,   122,
       0,     0,     0,   233,     0,     0,     0,     0,     0,     0,
     123,   124,   125,   126,   127,     0,     0,     0,     0,   128,
     129,   113,   114,     0,     0,     0,   115,   116,   117,     0,
     118,   119,     0,   120,   121,   122,     0,     0,     0,   234,
       0,     0,     0,     0,     0,     0,   123,   124,   125,   126,
     127,     0,     0,     0,     0,   128,   129,   113,   114,     0,
       0,     0,   115,   116,   117,     0,   118,   119,     0,   120,
     121,   122,     0,     0,     0,     0,   202,     0,     0,     0,
       0,     0,   123,   124,   125,   126,   127,     0,   113,   114,
       0,   128,   129,   115,   116,   117,     0,   118,   119,     0,
     120,   121,   122,     0,     0,     0,     0,     0,     0,   151,
       0,     0,   203,   123,   124,   125,   126,   127,     0,   113,
     114,     0,   128,   129,   115,   116,   117,     0,   118,   119,
       0,   120,   121,   122,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   123,   124,   125,   126,   127,     0,
     113,   114,     0,   128,   129,   115,   116,   117,     0,   118,
     119,     0,   120,   121,   122,     0,     0,     0,     0,     0,
     162,   163,     0,     0,     0,   123,   124,   125,   126,   127,
       0,   113,   114,     0,   128,   129,   115,   116,   117,     0,
     118,   119,     0,   120,   121,   122,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   173,   123,   124,   125,   126,
     127,     0,   113,   114,   166,   128,   129,   115,   116,   117,
       0,   118,   119,     0,   120,   121,   122,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   123,   124,   125,
     126,   127,     0,   113,   114,   166,   128,   129,   115,   116,
     117,     0,   118,   119,     0,   120,   121,   122,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   173,   123,   124,
     125,   126,   127,     0,   113,   114,     0,   128,   129,   115,
     116,   117,     0,   118,   119,     0,   120,   121,   122,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   123,
     124,   125,   126,   127,     0,   -63,   -63,     0,   128,   129,
     -63,   -63,   -63,     0,   -63,   -63,     0,   -63,   -63,   -63,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     -63,     0,     0,   -63,   -63,     0,     0,    64,     0,   -63,
     -63
};

static const yytype_int16 yycheck[] =
{
       0,    10,     0,     0,     0,     0,    25,   224,    64,    65,
     223,     1,    26,    27,    44,   206,    44,    66,    26,    68,
     107,   244,   109,    24,   267,    12,    13,    87,    27,    46,
      47,    89,    92,    20,    92,    87,    41,    20,   261,   282,
      48,    90,    91,    44,   235,   101,   259,   260,    38,    48,
      80,   264,    80,   266,    20,   270,     4,    76,    20,     7,
       8,     9,    36,   276,    38,   278,    20,    15,   283,   286,
      18,    90,    88,    87,    20,    20,    24,    25,    20,    27,
      28,    29,    30,    31,    32,    20,    80,    35,    36,    37,
      87,   308,    87,   306,    20,    88,    36,    20,    46,    47,
      92,    92,    43,    92,    43,    20,   106,    20,   106,    57,
     106,    89,    89,    20,    44,    20,    64,    65,    27,    92,
      20,    69,    41,    71,    32,    26,    48,    43,    27,    27,
     139,    27,    43,    27,   106,    83,   256,    41,    41,    32,
      32,   150,   282,    25,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   169,    -1,   101,    -1,   164,    -1,    -1,    -1,   107,
     108,   109,    -1,   172,    -1,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   172,   131,    -1,   272,    -1,    -1,    -1,   137,
      -1,    -1,    -1,   141,   142,    -1,   144,   145,   285,   147,
     148,    -1,    -1,   151,    -1,    -1,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,    -1,    -1,   236,    -1,
      -1,   239,   240,    -1,    -1,   173,    -1,    -1,    62,    63,
      -1,    65,    66,    -1,    68,    -1,   254,   227,    -1,    -1,
     230,    -1,    -1,    -1,    -1,   263,   255,    81,    82,    83,
      84,    85,   270,    -1,   202,   203,    90,    91,    -1,   249,
     269,   279,    -1,   281,   282,    62,    63,    -1,    65,    66,
      -1,    68,    -1,   291,    -1,    -1,    -1,    -1,    -1,   297,
      -1,   299,    -1,   231,   232,   233,   234,    84,    85,   298,
     238,    -1,    -1,    90,    91,    -1,    -1,   315,    -1,   317,
       0,     1,    -1,    -1,    -1,   253,    -1,    -1,    -1,    -1,
     310,    11,    12,    13,    14,    -1,    16,    17,    18,    19,
      20,    21,    -1,    23,   272,    -1,    -1,    -1,    28,    -1,
      30,    31,    32,    -1,    -1,    35,    -1,   285,    -1,    -1,
      40,    41,    42,    -1,    -1,    45,    -1,    -1,    -1,    49,
      50,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    -1,    78,    79,
      -1,    -1,    82,    83,    -1,    -1,    -1,    87,    88,    11,
      12,    13,    14,    -1,    16,    17,    18,    19,    20,    21,
      -1,    23,    -1,    -1,    -1,    -1,    28,    -1,    30,    31,
      32,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,    41,
      42,    -1,    -1,    45,    -1,    -1,    -1,    49,    50,    51,
      52,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    -1,    -1,    78,    79,    -1,    -1,
      82,    83,    11,    12,    13,    14,    88,    16,    17,    18,
      19,    20,    21,    -1,    23,    -1,    -1,    -1,    27,    28,
      -1,    30,    31,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    41,    42,    -1,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      79,    -1,    -1,    82,    83,    -1,    -1,    -1,    87,    88,
      11,    12,    13,    14,    -1,    16,    17,    18,    19,    20,
      21,    -1,    23,    -1,    -1,    -1,    -1,    28,    -1,    30,
      31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,
      41,    42,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      51,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    79,    -1,
      -1,    82,    83,    -1,    -1,    -1,    87,    88,    11,    12,
      13,    14,    -1,    16,    17,    18,    19,    20,    21,    -1,
      23,    -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    41,    42,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    51,    52,
      -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    79,    -1,    -1,    82,
      83,    11,    12,    13,    14,    88,    16,    17,    18,    19,
      20,    21,    -1,    23,    -1,    -1,    -1,    -1,    28,    -1,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    -1,    -1,    79,
      -1,    -1,    82,    83,    11,    12,    13,    14,    88,    16,
      -1,    -1,    -1,    20,    21,    -1,    23,    62,    63,    -1,
      65,    66,    29,    68,    -1,    -1,    -1,    34,    -1,    36,
      37,    38,    39,    40,    -1,    42,    -1,    82,    83,    84,
      85,    -1,    -1,    -1,    -1,    90,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    82,    83,    11,    12,    13,
      14,    88,    16,    -1,    -1,    -1,    20,    21,    -1,    23,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    -1,    38,    -1,    40,    -1,    42,    -1,
      -1,    -1,    -1,    11,    12,    13,    14,    -1,    16,    53,
      -1,    -1,    20,    21,    -1,    23,    -1,    -1,    -1,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    40,    -1,    42,    -1,    -1,    -1,    82,    83,
      11,    12,    13,    14,    88,    16,    -1,    -1,    -1,    20,
      21,    -1,    23,    -1,    -1,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,    40,
      -1,    42,    -1,    -1,    82,    83,    -1,    -1,    -1,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,    56,
      57,    82,    83,    -1,    61,    62,    63,    88,    65,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    44,    81,    82,    83,    84,    85,    -1,
      -1,    -1,    -1,    90,    91,    56,    57,    -1,    -1,    -1,
      61,    62,    63,    -1,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    44,
      81,    82,    83,    84,    85,    -1,    -1,    -1,    -1,    90,
      91,    56,    57,    -1,    -1,    -1,    61,    62,    63,    -1,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    44,    81,    82,    83,    84,
      85,    -1,    -1,    -1,    -1,    90,    91,    56,    57,    -1,
      -1,    -1,    61,    62,    63,    -1,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    44,    81,    82,    83,    84,    85,    -1,    -1,    -1,
      -1,    90,    91,    56,    57,    -1,    -1,    -1,    61,    62,
      63,    -1,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    -1,    -1,    -1,    -1,    90,    91,    56,
      57,    -1,    -1,    -1,    61,    62,    63,    -1,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    82,    83,    84,    85,    -1,
      -1,    -1,    -1,    90,    91,    56,    57,    -1,    -1,    -1,
      61,    62,    63,    -1,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    -1,    -1,    -1,    -1,    90,
      91,    56,    57,    -1,    -1,    -1,    61,    62,    63,    -1,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    82,    83,    84,
      85,    -1,    -1,    -1,    -1,    90,    91,    56,    57,    -1,
      -1,    -1,    61,    62,    63,    -1,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,
      -1,    -1,    81,    82,    83,    84,    85,    -1,    56,    57,
      -1,    90,    91,    61,    62,    63,    -1,    65,    66,    -1,
      68,    69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      -1,    -1,    80,    81,    82,    83,    84,    85,    -1,    56,
      57,    -1,    90,    91,    61,    62,    63,    -1,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    82,    83,    84,    85,    -1,
      56,    57,    -1,    90,    91,    61,    62,    63,    -1,    65,
      66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,
      76,    77,    -1,    -1,    -1,    81,    82,    83,    84,    85,
      -1,    56,    57,    -1,    90,    91,    61,    62,    63,    -1,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      85,    -1,    56,    57,    89,    90,    91,    61,    62,    63,
      -1,    65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    82,    83,
      84,    85,    -1,    56,    57,    89,    90,    91,    61,    62,
      63,    -1,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    -1,    56,    57,    -1,    90,    91,    61,
      62,    63,    -1,    65,    66,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      82,    83,    84,    85,    -1,    56,    57,    -1,    90,    91,
      61,    62,    63,    -1,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    84,    85,    -1,    -1,    88,    -1,    90,
      91
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    11,    12,    13,    14,    16,    17,    18,    19,
      20,    21,    23,    28,    30,    31,    32,    35,    40,    41,
      42,    45,    49,    50,    51,    52,    55,    64,    71,    72,
      73,    74,    75,    78,    79,    82,    83,    88,    94,    96,
      97,    98,    99,   100,   107,   108,   109,   114,   115,   116,
     119,   122,   123,   130,    87,    95,    20,    40,    97,   115,
     115,   115,   115,   131,    88,   111,    41,    20,   117,    29,
      34,    36,    37,    38,    39,   115,   120,   121,    20,   115,
      46,    47,    12,    13,    20,    20,    20,    20,   115,    53,
     120,   121,    20,   118,   115,   115,   115,   115,   115,   115,
      20,    20,   115,   115,   115,     0,    95,    88,    80,    88,
     106,   115,   115,    56,    57,    61,    62,    63,    65,    66,
      68,    69,    70,    81,    82,    83,    84,    85,    90,    91,
     115,    92,   131,   131,    92,   115,   115,    36,   121,   124,
      33,    66,    67,    20,    44,    80,   115,    44,    80,    43,
     125,    46,   121,    92,    38,    44,    38,    44,    38,    44,
      38,    44,    76,    77,   126,   131,    89,    96,   106,   115,
     106,   115,    43,    80,   112,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,    89,    20,   115,   111,    20,   115,   115,
     115,   115,    44,    80,   115,   115,   111,   115,    20,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   111,
      89,   112,    89,    95,   111,   115,    20,   128,   115,   115,
     128,    38,    38,    38,    38,   127,   113,    24,    44,   113,
      98,   116,    92,    95,    95,   115,   115,   115,   115,   128,
      95,    99,   112,    44,   115,   112,   112,    20,    92,   129,
     129,    95,    27,   115,   112,    27,    48,   111,    26,    48,
     103,   104,   110,    20,   113,   113,   129,    41,   112,   113,
      32,   113,   102,   103,   105,   110,   111,   104,   112,   106,
     113,   113,   112,   112,   105,   112,   106,    98,    48,   101,
      43,    27,   112,    27,    27,    27,    43,   112,   111,   112,
     111,    27,    41,    32,    32,   113,    98,    99,    41,   112,
     112
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
#line 108 "engines/director/lingo/lingo-gr.y"
    { yyerrok; ;}
    break;

  case 5:
#line 111 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1;
	;}
    break;

  case 10:
#line 120 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 12:
#line 124 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 13:
#line 130 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_after); ;}
    break;

  case 14:
#line 131 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_before); ;}
    break;

  case 15:
#line 132 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 16:
#line 138 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (4)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (4)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 17:
#line 146 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (5)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (5)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 18:
#line 154 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 19:
#line 160 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (4)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (4)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 20:
#line 168 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (5)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (5)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 21:
#line 177 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 25:
#line 186 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 2] = end; ;}
    break;

  case 26:
#line 197 "engines/director/lingo/lingo-gr.y"
    {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[(3) - (11)].code));
		WRITE_UINT32(&finish, (yyvsp[(6) - (11)].code));
		WRITE_UINT32(&body, (yyvsp[(8) - (11)].code));
		WRITE_UINT32(&end, (yyvsp[(9) - (11)].code));
		WRITE_UINT32(&inc, 1);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 1] = init;	/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 2] = finish;/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 3] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 4] = inc;	/* increment */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 5] = end; ;}
    break;

  case 27:
#line 213 "engines/director/lingo/lingo-gr.y"
    {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[(3) - (12)].code));
		WRITE_UINT32(&finish, (yyvsp[(7) - (12)].code));
		WRITE_UINT32(&body, (yyvsp[(9) - (12)].code));
		WRITE_UINT32(&end, (yyvsp[(10) - (12)].code));
		WRITE_UINT32(&inc, -1);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 1] = init;	/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 2] = finish;/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 3] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 4] = inc;	/* increment */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 5] = end; ;}
    break;

  case 28:
#line 225 "engines/director/lingo/lingo-gr.y"
    {
			inst end = 0;
			WRITE_UINT32(&end, (yyvsp[(3) - (3)].code));
			g_lingo->code1(STOP);
			(*g_lingo->_currentScript)[(yyvsp[(1) - (3)].code) + 1] = end;
		;}
    break;

  case 29:
#line 233 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 3] = end;	/* end, if cond fails */
		g_lingo->processIf(0, 0); ;}
    break;

  case 30:
#line 240 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (11)].code));
		WRITE_UINT32(&else1, (yyvsp[(8) - (11)].code));
		WRITE_UINT32(&end, (yyvsp[(9) - (11)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 3] = end;	/* end, if cond fails */
		g_lingo->processIf(0, 0); ;}
    break;

  case 31:
#line 249 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (11)].code));
		WRITE_UINT32(&else1, (yyvsp[(7) - (11)].code));
		WRITE_UINT32(&end, (yyvsp[(9) - (11)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 3] = end;	/* end, if cond fails */
		g_lingo->processIf(0, (yyvsp[(9) - (11)].code)); ;}
    break;

  case 32:
#line 258 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code));
		WRITE_UINT32(&else1, 0);
		WRITE_UINT32(&end, (yyvsp[(6) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, 0); ;}
    break;

  case 33:
#line 268 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (10)].code));
		WRITE_UINT32(&else1, (yyvsp[(8) - (10)].code));
		WRITE_UINT32(&end, (yyvsp[(10) - (10)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, 0); ;}
    break;

  case 34:
#line 278 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (10)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (10)].code));
		WRITE_UINT32(&end, (yyvsp[(10) - (10)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 3] = end; 	/* end, if cond fails */

		g_lingo->processIf(0, (yyvsp[(10) - (10)].code)); ;}
    break;

  case 35:
#line 289 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 36:
#line 290 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 41:
#line 301 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 43:
#line 310 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (5)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (5)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (5)].code)); ;}
    break;

  case 44:
#line 318 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 45:
#line 319 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 47:
#line 322 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 48:
#line 324 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 49:
#line 330 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 50:
#line 336 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 51:
#line 343 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 52:
#line 345 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 53:
#line 347 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 56:
#line 352 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 57:
#line 358 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->codeConst((yyvsp[(1) - (1)].i)); ;}
    break;

  case 58:
#line 359 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_fconstpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 59:
#line 362 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 60:
#line 365 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 61:
#line 368 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->_handlers[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 62:
#line 371 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 63:
#line 374 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 64:
#line 378 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 65:
#line 385 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 67:
#line 392 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 68:
#line 393 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 69:
#line 394 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 70:
#line 395 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 71:
#line 396 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); ;}
    break;

  case 72:
#line 397 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 73:
#line 398 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 74:
#line 399 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 75:
#line 400 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 76:
#line 401 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 77:
#line 402 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 78:
#line 403 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 79:
#line 404 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 80:
#line 405 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 81:
#line 406 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_after); ;}
    break;

  case 82:
#line 407 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 83:
#line 408 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 84:
#line 409 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 85:
#line 410 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 86:
#line 411 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 87:
#line 412 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 88:
#line 413 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 89:
#line 414 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 90:
#line 415 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); ;}
    break;

  case 91:
#line 416 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); ;}
    break;

  case 92:
#line 417 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); ;}
    break;

  case 93:
#line 418 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); ;}
    break;

  case 94:
#line 419 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); ;}
    break;

  case 95:
#line 420 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); ;}
    break;

  case 96:
#line 421 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); ;}
    break;

  case 97:
#line 422 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); ;}
    break;

  case 98:
#line 425 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 101:
#line 428 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); ;}
    break;

  case 102:
#line 429 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeConst(0); // Push fake value on stack
							  g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 105:
#line 433 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_handlers[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 106:
#line 436 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_handlers[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 107:
#line 439 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code2(g_lingo->c_voidpush, g_lingo->_handlers[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 108:
#line 442 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 109:
#line 443 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 110:
#line 444 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 111:
#line 445 "engines/director/lingo/lingo-gr.y"
    { Common::String s("sound-"); s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 112:
#line 448 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 113:
#line 449 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 114:
#line 452 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 115:
#line 453 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 116:
#line 464 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 117:
#line 465 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 118:
#line 466 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 119:
#line 467 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 120:
#line 470 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 121:
#line 473 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 126:
#line 486 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 127:
#line 487 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 128:
#line 490 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 129:
#line 493 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 130:
#line 523 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 131:
#line 524 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeConst(0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 132:
#line 529 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeFactory(*(yyvsp[(2) - (2)].s));
		;}
    break;

  case 133:
#line 532 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 134:
#line 533 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 135:
#line 537 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 136:
#line 538 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_ignoreMe = true; ;}
    break;

  case 137:
#line 538 "engines/director/lingo/lingo-gr.y"
    {
				g_lingo->codeConst(0); // Push fake value on stack
				g_lingo->code1(g_lingo->c_procret);
				g_lingo->define(*(yyvsp[(2) - (10)].s), (yyvsp[(4) - (10)].code), (yyvsp[(6) - (10)].narg));
				g_lingo->_indef = false;
				g_lingo->_ignoreMe = false;

				//if (*$2 != *$11) {
				//	yyerror("on vs end handler mismatch");
				//}
			;}
    break;

  case 138:
#line 550 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 139:
#line 551 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 140:
#line 552 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 141:
#line 553 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 142:
#line 555 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 143:
#line 559 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (3)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(3) - (3)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 144:
#line 567 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 145:
#line 568 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 146:
#line 569 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2846 "engines/director/lingo/lingo-gr.cpp"
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


#line 572 "engines/director/lingo/lingo-gr.y"


