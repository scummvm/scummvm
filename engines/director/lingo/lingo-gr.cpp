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
     SYMBOL = 265,
     OBJECT = 266,
     INT = 267,
     THEENTITY = 268,
     THEENTITYWITHID = 269,
     FLOAT = 270,
     BLTIN = 271,
     BLTINNOARGS = 272,
     BLTINNOARGSORONE = 273,
     BLTINONEARG = 274,
     BLTINARGLIST = 275,
     ID = 276,
     STRING = 277,
     HANDLER = 278,
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
     tCONCAT = 321,
     tCONTAINS = 322,
     tSTARTS = 323,
     tSPRITE = 324,
     tINTERSECTS = 325,
     tWITHIN = 326,
     tON = 327,
     tME = 328,
     tSOUND = 329
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
#define SYMBOL 265
#define OBJECT 266
#define INT 267
#define THEENTITY 268
#define THEENTITYWITHID 269
#define FLOAT 270
#define BLTIN 271
#define BLTINNOARGS 272
#define BLTINNOARGSORONE 273
#define BLTINONEARG 274
#define BLTINARGLIST 275
#define ID 276
#define STRING 277
#define HANDLER 278
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
#define tCONCAT 321
#define tCONTAINS 322
#define tSTARTS 323
#define tSPRITE 324
#define tINTERSECTS 325
#define tWITHIN 326
#define tON 327
#define tME 328
#define tSOUND 329




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
#line 274 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 287 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  96
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   990

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  88
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  38
/* YYNRULES -- Number of rules.  */
#define YYNRULES  133
/* YYNRULES -- Number of states.  */
#define YYNSTATES  282

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   329

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      82,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    81,    76,     2,
      83,    84,    79,    77,    87,    78,     2,    80,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      86,    75,    85,     2,     2,     2,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    14,    15,    17,    19,
      21,    23,    25,    30,    35,    40,    46,    51,    56,    62,
      64,    66,    68,    70,    79,    91,   104,   108,   117,   129,
     141,   148,   159,   170,   171,   175,   178,   180,   183,   185,
     192,   194,   200,   202,   206,   210,   213,   217,   219,   221,
     222,   223,   224,   227,   230,   234,   236,   238,   240,   242,
     247,   249,   251,   254,   256,   260,   264,   268,   272,   276,
     280,   284,   288,   292,   296,   300,   304,   307,   311,   315,
     319,   323,   326,   329,   333,   338,   343,   346,   348,   350,
     353,   355,   358,   361,   364,   367,   369,   372,   377,   380,
     384,   386,   390,   392,   396,   399,   402,   405,   408,   412,
     415,   418,   420,   424,   427,   430,   433,   437,   440,   441,
     450,   453,   454,   463,   464,   475,   476,   478,   482,   487,
     488,   492,   493,   495
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      89,     0,    -1,    89,    90,    91,    -1,    91,    -1,     1,
      90,    -1,    82,    -1,    -1,   118,    -1,   111,    -1,   124,
      -1,    92,    -1,    94,    -1,    40,   110,    33,    21,    -1,
      42,    21,    75,   110,    -1,    42,    13,    75,   110,    -1,
      42,    14,   110,    75,   110,    -1,    42,    21,    44,   110,
      -1,    42,    13,    44,   110,    -1,    42,    14,   110,    44,
     110,    -1,   110,    -1,   111,    -1,    93,    -1,    95,    -1,
     102,    83,   101,    84,   108,   107,    27,    41,    -1,   103,
      75,   110,   107,    44,   110,   107,   108,   107,    27,    41,
      -1,   103,    75,   110,   107,    24,    44,   110,   107,   108,
     107,    27,    41,    -1,   109,   110,   107,    -1,   104,   101,
      43,    90,   108,   107,    27,    32,    -1,   104,   101,    43,
      90,   108,   107,    48,   108,   107,    27,    32,    -1,   104,
     101,    43,    90,   108,   107,   106,    97,   107,    27,    32,
      -1,   104,   101,    43,   106,    93,   107,    -1,   104,   101,
      43,   106,    93,   107,    48,   106,    93,   107,    -1,   104,
     101,    43,   106,    93,   107,    98,   107,    96,   107,    -1,
      -1,    48,   106,    93,    -1,    97,   100,    -1,   100,    -1,
      98,    99,    -1,    99,    -1,   105,   101,    43,   106,    94,
     107,    -1,    98,    -1,   105,   101,    43,   108,   107,    -1,
     110,    -1,   110,    75,   110,    -1,    83,   101,    84,    -1,
      41,    47,    -1,    41,    46,    21,    -1,    32,    -1,    26,
      -1,    -1,    -1,    -1,   108,    90,    -1,   108,    94,    -1,
      45,    21,    43,    -1,    12,    -1,    15,    -1,    22,    -1,
      17,    -1,    21,    83,   125,    84,    -1,    21,    -1,    13,
      -1,    14,   110,    -1,    92,    -1,   110,    77,   110,    -1,
     110,    78,   110,    -1,   110,    79,   110,    -1,   110,    80,
     110,    -1,   110,    65,   110,    -1,   110,    85,   110,    -1,
     110,    86,   110,    -1,   110,    61,   110,    -1,   110,    56,
     110,    -1,   110,    57,   110,    -1,   110,    62,   110,    -1,
     110,    63,   110,    -1,    64,   110,    -1,   110,    76,   110,
      -1,   110,    66,   110,    -1,   110,    67,   110,    -1,   110,
      68,   110,    -1,    77,   110,    -1,    78,   110,    -1,    83,
     110,    84,    -1,    69,   110,    70,   110,    -1,    69,   110,
      71,   110,    -1,    40,   110,    -1,   114,    -1,   117,    -1,
      28,    41,    -1,    28,    -1,    30,   112,    -1,    55,   113,
      -1,    19,   110,    -1,    18,   110,    -1,    18,    -1,    20,
     125,    -1,    51,   110,    46,   110,    -1,    51,   110,    -1,
      74,    21,   125,    -1,    21,    -1,   112,    87,    21,    -1,
      21,    -1,   113,    87,    21,    -1,    31,    34,    -1,    31,
      37,    -1,    31,    39,    -1,    31,   115,    -1,    31,   115,
     116,    -1,    31,   116,    -1,    29,   110,    -1,   110,    -1,
      38,    36,   110,    -1,    36,   110,    -1,    52,    53,    -1,
      52,   115,    -1,    52,   115,   116,    -1,    52,   116,    -1,
      -1,    35,    21,   119,   106,   122,    90,   123,   108,    -1,
      49,    21,    -1,    -1,    50,    21,   120,   106,   122,    90,
     123,   108,    -1,    -1,    72,    21,   121,   106,   122,    90,
     123,   108,    27,    21,    -1,    -1,    21,    -1,   122,    87,
      21,    -1,   122,    90,    87,    21,    -1,    -1,    21,   106,
     125,    -1,    -1,   110,    -1,   125,    87,   110,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   106,   106,   107,   108,   111,   116,   117,   118,   119,
     120,   121,   124,   130,   136,   144,   152,   158,   166,   175,
     176,   178,   179,   184,   195,   211,   223,   231,   238,   247,
     256,   266,   276,   287,   288,   291,   292,   295,   296,   299,
     307,   308,   316,   317,   318,   320,   322,   328,   334,   341,
     343,   345,   346,   347,   350,   356,   357,   360,   363,   366,
     369,   373,   380,   386,   387,   388,   389,   390,   391,   392,
     393,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   411,   412,   413,   414,
     415,   417,   418,   419,   422,   425,   428,   429,   430,   431,
     434,   435,   438,   439,   450,   451,   452,   453,   456,   459,
     464,   465,   468,   469,   472,   473,   476,   479,   509,   509,
     515,   518,   518,   523,   523,   535,   536,   537,   538,   540,
     544,   552,   553,   554
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNARY", "CASTREF", "VOID", "VAR",
  "POINT", "RECT", "ARRAY", "SYMBOL", "OBJECT", "INT", "THEENTITY",
  "THEENTITYWITHID", "FLOAT", "BLTIN", "BLTINNOARGS", "BLTINNOARGSORONE",
  "BLTINONEARG", "BLTINARGLIST", "ID", "STRING", "HANDLER", "tDOWN",
  "tELSE", "tNLELSIF", "tEND", "tEXIT", "tFRAME", "tGLOBAL", "tGO", "tIF",
  "tINTO", "tLOOP", "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS",
  "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE",
  "tNLELSE", "tFACTORY", "tMETHOD", "tOPEN", "tPLAY", "tDONE",
  "tPLAYACCEL", "tINSTANCE", "tGE", "tLE", "tGT", "tLT", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tCONCAT", "tCONTAINS", "tSTARTS",
  "tSPRITE", "tINTERSECTS", "tWITHIN", "tON", "tME", "tSOUND", "'='",
  "'&'", "'+'", "'-'", "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "'>'",
  "'<'", "','", "$accept", "program", "nl", "programline", "asgn",
  "stmtoneliner", "stmt", "ifstmt", "elsestmtoneliner", "elseifstmt",
  "elseifstmtoneliner", "elseifstmtoneliner1", "elseifstmt1", "cond",
  "repeatwhile", "repeatwith", "if", "elseif", "begin", "end", "stmtlist",
  "when", "expr", "func", "globallist", "instancelist", "gotofunc",
  "gotoframe", "gotomovie", "playfunc", "defn", "@1", "@2", "@3", "argdef",
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
     325,   326,   327,   328,   329,    61,    38,    43,    45,    42,
      47,    37,    10,    40,    41,    62,    60,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    88,    89,    89,    89,    90,    91,    91,    91,    91,
      91,    91,    92,    92,    92,    92,    92,    92,    92,    93,
      93,    94,    94,    94,    94,    94,    94,    95,    95,    95,
      95,    95,    95,    96,    96,    97,    97,    98,    98,    99,
     100,   100,   101,   101,   101,   102,   103,   104,   105,   106,
     107,   108,   108,   108,   109,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     112,   112,   113,   113,   114,   114,   114,   114,   114,   114,
     115,   115,   116,   116,   117,   117,   117,   117,   119,   118,
     118,   120,   118,   121,   118,   122,   122,   122,   122,   123,
     124,   125,   125,   125
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     0,     1,     1,     1,
       1,     1,     4,     4,     4,     5,     4,     4,     5,     1,
       1,     1,     1,     8,    11,    12,     3,     8,    11,    11,
       6,    10,    10,     0,     3,     2,     1,     2,     1,     6,
       1,     5,     1,     3,     3,     2,     3,     1,     1,     0,
       0,     0,     2,     2,     3,     1,     1,     1,     1,     4,
       1,     1,     2,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     3,     3,
       3,     2,     2,     3,     4,     4,     2,     1,     1,     2,
       1,     2,     2,     2,     2,     1,     2,     4,     2,     3,
       1,     3,     1,     3,     2,     2,     2,     2,     3,     2,
       2,     1,     3,     2,     2,     2,     3,     2,     0,     8,
       2,     0,     8,     0,    10,     0,     1,     3,     4,     0,
       3,     0,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    55,    61,     0,    56,    58,    95,     0,   131,
      49,    57,    90,     0,     0,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     3,    63,    21,    11,    22,     0,
       0,     0,     0,    19,     8,    87,    88,     7,     9,     5,
       4,    60,     0,    63,    62,    94,    93,   132,    96,   131,
     131,    89,   100,    91,     0,   104,     0,   105,     0,   106,
     111,   107,   109,   118,    86,     0,    45,     0,     0,     0,
       0,   120,   121,    98,   114,   115,   117,   102,    92,    76,
       0,   123,   131,    81,    82,     0,     1,     6,     0,     0,
       0,     0,    42,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   130,     0,   110,   113,     0,   108,    49,
       0,    46,     0,     0,     0,     0,     0,    54,    49,     0,
     116,     0,     0,     0,    49,    99,    83,     2,     0,    50,
       0,     0,    49,     0,    26,    72,    73,    71,    74,    75,
      68,    78,    79,    80,    77,    64,    65,    66,    67,    69,
      70,   133,    59,   101,   112,   125,    12,    17,    14,     0,
       0,    16,    13,   125,    97,   103,    84,    85,   125,    51,
       0,    44,    51,     0,    43,   126,     0,    18,    15,     0,
       0,    50,     0,     0,    50,    50,    20,     0,   129,   129,
     129,    52,    53,     0,     0,    50,    49,    30,   127,     0,
      51,    51,    51,     0,    50,    51,     0,    51,     0,    48,
      49,    50,    38,     0,   128,   119,   122,     0,    23,    51,
      50,    27,    50,    50,    40,    36,     0,     0,    37,    33,
       0,     0,    50,     0,     0,    35,     0,     0,    50,    49,
      50,    49,   124,     0,     0,     0,     0,    49,    31,     0,
      32,     0,     0,    24,    28,    29,    50,    34,    50,    25,
      41,    39
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    33,   211,    34,    53,    36,   212,    38,   260,   243,
     244,   232,   245,   101,    39,    40,    41,   233,   271,   154,
     201,    42,    43,   206,    63,    88,    45,    71,    72,    46,
      47,   129,   138,   144,   196,   220,    48,    58
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -214
static const yytype_int16 yypact[] =
{
     263,   -63,  -214,  -214,    11,  -214,  -214,    11,    11,    11,
     172,  -214,   -14,    15,   671,  -214,    31,    11,    61,    77,
      47,    82,    84,    11,   703,    88,    11,    11,    89,    90,
      11,    11,    11,     2,  -214,     3,  -214,  -214,  -214,    29,
      38,   659,    11,   904,  -214,  -214,  -214,  -214,  -214,  -214,
    -214,    32,    11,  -214,   904,   904,   904,   904,    27,    11,
      11,  -214,  -214,    30,    11,  -214,    11,  -214,    80,  -214,
     904,    24,  -214,  -214,   722,    98,  -214,   -38,    11,   -36,
      78,  -214,  -214,   774,  -214,    24,  -214,  -214,    35,    -8,
     800,  -214,    11,    -8,    -8,   852,  -214,   335,   659,    11,
     659,    83,   878,   904,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
     722,    11,     9,    27,   106,   904,   904,    11,  -214,  -214,
     110,  -214,    11,    11,   748,    11,    11,  -214,  -214,    11,
    -214,   113,    11,    11,  -214,    27,  -214,  -214,    52,   904,
      53,   826,   -63,    11,  -214,   244,   244,   244,    -8,    -8,
      -8,   904,   244,   244,   316,   -45,   -45,    -8,    -8,   904,
     904,   904,  -214,  -214,   904,   120,  -214,   904,   904,    11,
      11,   904,   904,   120,   904,  -214,   904,   904,   120,  -214,
      -2,  -214,  -214,   613,   904,  -214,   -37,   904,   904,   -37,
     -37,   474,   116,    11,   474,  -214,  -214,   142,    79,    79,
      79,  -214,  -214,   138,    11,   904,    28,   -10,  -214,   147,
    -214,  -214,  -214,   128,   904,  -214,   139,  -214,   148,  -214,
    -214,   148,  -214,   659,  -214,   474,   474,   402,  -214,  -214,
     474,  -214,   474,   148,   148,  -214,   659,   613,  -214,   130,
     137,   160,   474,   157,   159,  -214,   163,   144,  -214,  -214,
    -214,  -214,  -214,   164,   151,   162,   165,   -17,  -214,   613,
    -214,   546,   154,  -214,  -214,  -214,   474,  -214,  -214,  -214,
    -214,  -214
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -214,  -214,    10,    99,     4,  -186,     0,  -214,  -214,  -214,
     -19,  -175,   -43,   -69,  -214,  -214,  -214,  -213,    -9,   -16,
     -97,  -214,    40,     5,  -214,  -214,  -214,   177,   -12,  -214,
    -214,  -214,  -214,  -214,  -109,  -123,  -214,   -46
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -61
static const yytype_int16 yytable[] =
{
      37,    60,    96,   -10,    35,    44,   132,   205,   135,   -51,
     -51,    50,    86,   122,   123,   246,   229,   107,   108,    49,
     109,   110,   202,     2,     3,     4,     5,    61,     6,   148,
     246,   150,    51,    11,   116,   117,    62,   133,   230,   136,
     118,   119,   203,    97,    54,    49,   145,    55,    56,    57,
     207,    52,    73,    19,    70,   226,   248,    74,   110,   128,
      66,   258,    68,    83,    70,   -51,    89,    90,    80,   248,
      93,    94,    95,   140,   199,    26,   227,   118,   119,   200,
      27,   102,   103,   277,    49,   -10,   221,   222,    30,    31,
      77,    78,   120,   172,    32,   204,   121,    37,    79,    57,
      57,    35,    44,    81,   125,    82,   126,    75,    76,    87,
      91,    92,    98,    99,   121,    59,   127,   124,   134,   131,
     175,   137,   141,   235,   236,   237,   152,   173,   240,   183,
     242,   176,    57,   190,   185,   188,   189,   191,   102,   149,
     151,   195,   252,   193,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     214,   171,   192,   218,   250,   223,   219,   174,   234,   238,
     276,   241,   177,   178,   229,   181,   182,   257,   259,   184,
     261,   262,   186,   187,   264,   213,   265,   267,   216,   217,
     266,   272,   273,   194,   274,   279,   147,   275,   231,   225,
     255,    85,     0,     0,     0,     0,   208,   228,   239,   209,
     210,     0,     0,     0,     0,   249,     0,     0,     0,   197,
     198,   247,     0,     0,   253,     0,   254,   256,   -60,   -60,
       0,     0,     0,   -60,   -60,   -60,   263,   -60,   -60,   -60,
     -60,     0,   268,   215,   270,     0,     0,     0,   -60,     0,
     269,   -60,   -60,     0,   224,    59,     0,   -60,   -60,     0,
     280,     0,   281,    -6,     1,     0,     0,     0,     0,     0,
       0,   278,     0,   102,     0,     2,     3,     4,     5,     0,
       6,     7,     8,     9,    10,    11,   102,     0,     0,     0,
       0,    12,     0,    13,    14,    15,     0,     0,    16,     0,
       0,     0,     0,    17,    18,    19,   107,   108,    20,   109,
     110,     0,    21,    22,    23,    24,     0,     0,    25,     0,
     113,   114,   115,   116,   117,     0,     0,    26,     0,   118,
     119,     0,    27,     0,     0,    28,     0,    29,     0,     0,
      30,    31,     0,     0,     0,    -6,    32,     2,     3,     4,
       5,     0,     6,     7,     8,     9,    10,    11,     0,     0,
       0,     0,     0,    12,     0,    13,    14,    15,     0,     0,
      16,     0,     0,     0,     0,    17,    18,    19,   107,   108,
      20,   109,   110,     0,    21,    22,    23,    24,     0,     0,
      25,     0,     0,   114,   115,   116,   117,     0,     0,    26,
       0,   118,   119,     0,    27,     0,     0,    28,     0,    29,
       0,     0,    30,    31,     2,     3,     4,     5,    32,     6,
       7,     8,     9,    51,    11,     0,     0,     0,     0,   251,
      12,     0,    13,    14,    15,     0,     0,     0,     0,     0,
       0,     0,    17,    18,    19,     0,     0,    20,     0,     0,
       0,     0,     0,    23,    24,     0,     0,    25,     0,     0,
       0,     0,     0,     0,     0,     0,    26,     0,     0,     0,
       0,    27,     0,     0,     0,     0,    29,     0,     0,    30,
      31,     0,     0,     0,    49,    32,     2,     3,     4,     5,
       0,     6,     7,     8,     9,    51,    11,     0,     0,     0,
       0,     0,    12,     0,    13,    14,    15,     0,     0,     0,
       0,     0,     0,     0,    17,    18,    19,     0,     0,    20,
       0,     0,     0,     0,     0,    23,    24,     0,     0,    25,
       0,     0,     0,     0,     0,     0,     0,     0,    26,     0,
       0,     0,     0,    27,     0,     0,     0,     0,    29,     0,
       0,    30,    31,     0,     0,     0,    49,    32,     2,     3,
       4,     5,     0,     6,     7,     8,     9,    51,    11,     0,
       0,     0,     0,     0,    12,     0,    13,    14,    15,     0,
       0,     0,     0,     0,     0,     0,    17,    18,    19,     0,
       0,    20,     0,     0,     0,     0,     0,    23,    24,     0,
       0,    25,     0,     0,     0,     0,     0,     0,     0,     0,
      26,     0,     0,     0,     0,    27,     0,     0,     0,     0,
      29,     0,     0,    30,    31,     2,     3,     4,     5,    32,
       6,     7,     8,     9,    51,    11,     0,     0,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,     0,
       0,     0,     0,    17,     0,    19,     0,     0,     0,     0,
       0,     0,     0,     0,    23,    24,     0,     0,    25,     0,
       0,     2,     3,     4,     5,     0,     6,    26,     0,     0,
      51,    11,    27,     2,     3,     4,     5,    29,     6,     0,
      30,    31,    51,    11,     0,     0,    32,     0,     0,    52,
      64,    19,     0,     0,     0,    65,     0,    66,    67,    68,
      69,    52,     0,    19,     0,     2,     3,     4,     5,     0,
       6,     0,     0,    26,    51,    11,     0,     0,    27,     0,
       0,     0,    64,     0,     0,    26,    30,    31,     0,    66,
      27,    68,   100,    52,     0,    19,     0,     0,    30,    31,
       0,     0,     0,     0,    32,   130,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    26,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,   104,   105,
      30,    31,     0,   106,   107,   108,    32,   109,   110,   111,
     112,     0,   179,     0,     0,     0,     0,     0,   113,   114,
     115,   116,   117,     0,   104,   105,     0,   118,   119,   106,
     107,   108,     0,   109,   110,   111,   112,     0,     0,     0,
     139,     0,     0,   180,   113,   114,   115,   116,   117,     0,
     104,   105,     0,   118,   119,   106,   107,   108,     0,   109,
     110,   111,   112,     0,     0,     0,     0,     0,     0,     0,
     113,   114,   115,   116,   117,     0,   104,   105,     0,   118,
     119,   106,   107,   108,     0,   109,   110,   111,   112,     0,
     142,   143,     0,     0,     0,     0,   113,   114,   115,   116,
     117,     0,   104,   105,     0,   118,   119,   106,   107,   108,
       0,   109,   110,   111,   112,     0,     0,     0,     0,     0,
       0,   153,   113,   114,   115,   116,   117,     0,   104,   105,
     146,   118,   119,   106,   107,   108,     0,   109,   110,   111,
     112,     0,     0,     0,     0,     0,     0,     0,   113,   114,
     115,   116,   117,     0,   104,   105,   146,   118,   119,   106,
     107,   108,     0,   109,   110,   111,   112,     0,     0,     0,
       0,     0,     0,   153,   113,   114,   115,   116,   117,     0,
     104,   105,     0,   118,   119,   106,   107,   108,     0,   109,
     110,   111,   112,     0,     0,     0,     0,     0,     0,     0,
     113,   114,   115,   116,   117,     0,     0,     0,     0,   118,
     119
};

static const yytype_int16 yycheck[] =
{
       0,    10,     0,     0,     0,     0,    44,   193,    44,    26,
      27,     1,    24,    59,    60,   228,    26,    62,    63,    82,
      65,    66,    24,    12,    13,    14,    15,    41,    17,    98,
     243,   100,    21,    22,    79,    80,    21,    75,    48,    75,
      85,    86,    44,    33,     4,    82,    92,     7,     8,     9,
      87,    40,    21,    42,    14,    27,   231,    17,    66,    71,
      36,   247,    38,    23,    24,    82,    26,    27,    21,   244,
      30,    31,    32,    85,   183,    64,    48,    85,    86,   188,
      69,    41,    42,   269,    82,    82,   209,   210,    77,    78,
      13,    14,    52,    84,    83,   192,    87,    97,    21,    59,
      60,    97,    97,    21,    64,    21,    66,    46,    47,    21,
      21,    21,    83,    75,    87,    83,    36,    87,    78,    21,
     129,    43,    87,   220,   221,   222,    43,    21,   225,   138,
     227,    21,    92,   149,    21,   144,    84,    84,    98,    99,
     100,    21,   239,   152,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
      44,   121,   152,    21,   233,    27,    87,   127,    21,    41,
     267,    32,   132,   133,    26,   135,   136,   246,    48,   139,
      43,    21,   142,   143,    27,   201,    27,    43,   204,   205,
      27,    27,    41,   153,    32,    41,    97,    32,   217,   215,
     243,    24,    -1,    -1,    -1,    -1,   196,   216,   224,   199,
     200,    -1,    -1,    -1,    -1,   231,    -1,    -1,    -1,   179,
     180,   230,    -1,    -1,   240,    -1,   242,   243,    56,    57,
      -1,    -1,    -1,    61,    62,    63,   252,    65,    66,    67,
      68,    -1,   258,   203,   260,    -1,    -1,    -1,    76,    -1,
     259,    79,    80,    -1,   214,    83,    -1,    85,    86,    -1,
     276,    -1,   278,     0,     1,    -1,    -1,    -1,    -1,    -1,
      -1,   271,    -1,   233,    -1,    12,    13,    14,    15,    -1,
      17,    18,    19,    20,    21,    22,   246,    -1,    -1,    -1,
      -1,    28,    -1,    30,    31,    32,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    40,    41,    42,    62,    63,    45,    65,
      66,    -1,    49,    50,    51,    52,    -1,    -1,    55,    -1,
      76,    77,    78,    79,    80,    -1,    -1,    64,    -1,    85,
      86,    -1,    69,    -1,    -1,    72,    -1,    74,    -1,    -1,
      77,    78,    -1,    -1,    -1,    82,    83,    12,    13,    14,
      15,    -1,    17,    18,    19,    20,    21,    22,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    30,    31,    32,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    40,    41,    42,    62,    63,
      45,    65,    66,    -1,    49,    50,    51,    52,    -1,    -1,
      55,    -1,    -1,    77,    78,    79,    80,    -1,    -1,    64,
      -1,    85,    86,    -1,    69,    -1,    -1,    72,    -1,    74,
      -1,    -1,    77,    78,    12,    13,    14,    15,    83,    17,
      18,    19,    20,    21,    22,    -1,    -1,    -1,    -1,    27,
      28,    -1,    30,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    41,    42,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    51,    52,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    74,    -1,    -1,    77,
      78,    -1,    -1,    -1,    82,    83,    12,    13,    14,    15,
      -1,    17,    18,    19,    20,    21,    22,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    30,    31,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40,    41,    42,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    77,    78,    -1,    -1,    -1,    82,    83,    12,    13,
      14,    15,    -1,    17,    18,    19,    20,    21,    22,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    40,    41,    42,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      64,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    77,    78,    12,    13,    14,    15,    83,
      17,    18,    19,    20,    21,    22,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    30,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    55,    -1,
      -1,    12,    13,    14,    15,    -1,    17,    64,    -1,    -1,
      21,    22,    69,    12,    13,    14,    15,    74,    17,    -1,
      77,    78,    21,    22,    -1,    -1,    83,    -1,    -1,    40,
      29,    42,    -1,    -1,    -1,    34,    -1,    36,    37,    38,
      39,    40,    -1,    42,    -1,    12,    13,    14,    15,    -1,
      17,    -1,    -1,    64,    21,    22,    -1,    -1,    69,    -1,
      -1,    -1,    29,    -1,    -1,    64,    77,    78,    -1,    36,
      69,    38,    83,    40,    -1,    42,    -1,    -1,    77,    78,
      -1,    -1,    -1,    -1,    83,    33,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    56,    57,
      77,    78,    -1,    61,    62,    63,    83,    65,    66,    67,
      68,    -1,    44,    -1,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    -1,    56,    57,    -1,    85,    86,    61,
      62,    63,    -1,    65,    66,    67,    68,    -1,    -1,    -1,
      46,    -1,    -1,    75,    76,    77,    78,    79,    80,    -1,
      56,    57,    -1,    85,    86,    61,    62,    63,    -1,    65,
      66,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    -1,    56,    57,    -1,    85,
      86,    61,    62,    63,    -1,    65,    66,    67,    68,    -1,
      70,    71,    -1,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    -1,    56,    57,    -1,    85,    86,    61,    62,    63,
      -1,    65,    66,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    80,    -1,    56,    57,
      84,    85,    86,    61,    62,    63,    -1,    65,    66,    67,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    80,    -1,    56,    57,    84,    85,    86,    61,
      62,    63,    -1,    65,    66,    67,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    -1,
      56,    57,    -1,    85,    86,    61,    62,    63,    -1,    65,
      66,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,    85,
      86
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    13,    14,    15,    17,    18,    19,    20,
      21,    22,    28,    30,    31,    32,    35,    40,    41,    42,
      45,    49,    50,    51,    52,    55,    64,    69,    72,    74,
      77,    78,    83,    89,    91,    92,    93,    94,    95,   102,
     103,   104,   109,   110,   111,   114,   117,   118,   124,    82,
      90,    21,    40,    92,   110,   110,   110,   110,   125,    83,
     106,    41,    21,   112,    29,    34,    36,    37,    38,    39,
     110,   115,   116,    21,   110,    46,    47,    13,    14,    21,
      21,    21,    21,   110,    53,   115,   116,    21,   113,   110,
     110,    21,    21,   110,   110,   110,     0,    90,    83,    75,
      83,   101,   110,   110,    56,    57,    61,    62,    63,    65,
      66,    67,    68,    76,    77,    78,    79,    80,    85,    86,
     110,    87,   125,   125,    87,   110,   110,    36,   116,   119,
      33,    21,    44,    75,   110,    44,    75,    43,   120,    46,
     116,    87,    70,    71,   121,   125,    84,    91,   101,   110,
     101,   110,    43,    75,   107,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,    84,    21,   110,   106,    21,   110,   110,    44,
      75,   110,   110,   106,   110,    21,   110,   110,   106,    84,
     107,    84,    90,   106,   110,    21,   122,   110,   110,   122,
     122,   108,    24,    44,   108,    93,   111,    87,    90,    90,
      90,    90,    94,   107,    44,   110,   107,   107,    21,    87,
     123,   123,   123,    27,   110,   107,    27,    48,   106,    26,
      48,    98,    99,   105,    21,   108,   108,   108,    41,   107,
     108,    32,   108,    97,    98,   100,   105,   106,    99,   107,
     101,    27,   108,   107,   107,   100,   107,   101,    93,    48,
      96,    43,    21,   107,    27,    27,    27,    43,   107,   106,
     107,   106,    27,    41,    32,    32,   108,    93,    94,    41,
     107,   107
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
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 14:
#line 136 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (4)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (4)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 15:
#line 144 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (5)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (5)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 16:
#line 152 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 17:
#line 158 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (4)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (4)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 18:
#line 166 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (5)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (5)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 19:
#line 175 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 23:
#line 184 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 2] = end; ;}
    break;

  case 24:
#line 195 "engines/director/lingo/lingo-gr.y"
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

  case 25:
#line 211 "engines/director/lingo/lingo-gr.y"
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

  case 26:
#line 223 "engines/director/lingo/lingo-gr.y"
    {
			inst end = 0;
			WRITE_UINT32(&end, (yyvsp[(3) - (3)].code));
			g_lingo->code1(STOP);
			(*g_lingo->_currentScript)[(yyvsp[(1) - (3)].code) + 1] = end;
		;}
    break;

  case 27:
#line 231 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 3] = end;	/* end, if cond fails */
		g_lingo->processIf(0, 0); ;}
    break;

  case 28:
#line 238 "engines/director/lingo/lingo-gr.y"
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

  case 29:
#line 247 "engines/director/lingo/lingo-gr.y"
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

  case 30:
#line 256 "engines/director/lingo/lingo-gr.y"
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

  case 31:
#line 266 "engines/director/lingo/lingo-gr.y"
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

  case 32:
#line 276 "engines/director/lingo/lingo-gr.y"
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

  case 33:
#line 287 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 34:
#line 288 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 39:
#line 299 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 41:
#line 308 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (5)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (5)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (5)].code)); ;}
    break;

  case 42:
#line 316 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 43:
#line 317 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 45:
#line 320 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 46:
#line 322 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 47:
#line 328 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 48:
#line 334 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 49:
#line 341 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 50:
#line 343 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 51:
#line 345 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 54:
#line 350 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 55:
#line 356 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->codeConst((yyvsp[(1) - (1)].i)); ;}
    break;

  case 56:
#line 357 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_fconstpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 57:
#line 360 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 58:
#line 363 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->_handlers[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 59:
#line 366 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 60:
#line 369 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 61:
#line 373 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 62:
#line 380 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 64:
#line 387 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 65:
#line 388 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 66:
#line 389 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 67:
#line 390 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 68:
#line 391 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); ;}
    break;

  case 69:
#line 392 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 70:
#line 393 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 71:
#line 394 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 72:
#line 395 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 73:
#line 396 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 74:
#line 397 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 75:
#line 398 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 76:
#line 399 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 77:
#line 400 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 78:
#line 401 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 79:
#line 402 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 80:
#line 403 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 81:
#line 404 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 82:
#line 405 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 83:
#line 406 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 84:
#line 407 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 85:
#line 408 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 86:
#line 411 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 89:
#line 414 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); ;}
    break;

  case 90:
#line 415 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeConst(0); // Push fake value on stack
							  g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 93:
#line 419 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_handlers[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 94:
#line 422 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_handlers[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 95:
#line 425 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code2(g_lingo->c_voidpush, g_lingo->_handlers[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 96:
#line 428 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 97:
#line 429 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 98:
#line 430 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 99:
#line 431 "engines/director/lingo/lingo-gr.y"
    { Common::String s("sound-"); s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 100:
#line 434 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 101:
#line 435 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 102:
#line 438 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 103:
#line 439 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 104:
#line 450 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 105:
#line 451 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 106:
#line 452 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 107:
#line 453 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 108:
#line 456 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 109:
#line 459 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 114:
#line 472 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 115:
#line 473 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 116:
#line 476 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 117:
#line 479 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 118:
#line 509 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 119:
#line 510 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeConst(0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 120:
#line 515 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeFactory(*(yyvsp[(2) - (2)].s));
		;}
    break;

  case 121:
#line 518 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 122:
#line 519 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 123:
#line 523 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 124:
#line 524 "engines/director/lingo/lingo-gr.y"
    {
				g_lingo->codeConst(0); // Push fake value on stack
				g_lingo->code1(g_lingo->c_procret);
				g_lingo->define(*(yyvsp[(2) - (10)].s), (yyvsp[(4) - (10)].code), (yyvsp[(5) - (10)].narg));
				g_lingo->_indef = false;

				if (*(yyvsp[(2) - (10)].s) != *(yyvsp[(10) - (10)].s)) {
					yyerror("on vs end handler mismatch");
				}
			;}
    break;

  case 125:
#line 535 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 126:
#line 536 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 127:
#line 537 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 128:
#line 538 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 129:
#line 540 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 130:
#line 544 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (3)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(3) - (3)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 131:
#line 552 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 132:
#line 553 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 133:
#line 554 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2630 "engines/director/lingo/lingo-gr.cpp"
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


#line 557 "engines/director/lingo/lingo-gr.y"


