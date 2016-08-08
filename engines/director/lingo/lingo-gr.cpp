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
     tDOWN = 278,
     tELSE = 279,
     tNLELSIF = 280,
     tEND = 281,
     tEXIT = 282,
     tFRAME = 283,
     tGLOBAL = 284,
     tGO = 285,
     tIF = 286,
     tINTO = 287,
     tLOOP = 288,
     tMACRO = 289,
     tMCI = 290,
     tMCIWAIT = 291,
     tMOVIE = 292,
     tNEXT = 293,
     tOF = 294,
     tPREVIOUS = 295,
     tPUT = 296,
     tREPEAT = 297,
     tSET = 298,
     tTHEN = 299,
     tTO = 300,
     tWHEN = 301,
     tWITH = 302,
     tWHILE = 303,
     tNLELSE = 304,
     tFACTORY = 305,
     tMETHOD = 306,
     tOPEN = 307,
     tPLAY = 308,
     tDONE = 309,
     tPLAYACCEL = 310,
     tGE = 311,
     tLE = 312,
     tGT = 313,
     tLT = 314,
     tEQ = 315,
     tNEQ = 316,
     tAND = 317,
     tOR = 318,
     tNOT = 319,
     tCONCAT = 320,
     tCONTAINS = 321,
     tSTARTS = 322,
     tSPRITE = 323,
     tINTERSECTS = 324,
     tWITHIN = 325
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
#define tDOWN 278
#define tELSE 279
#define tNLELSIF 280
#define tEND 281
#define tEXIT 282
#define tFRAME 283
#define tGLOBAL 284
#define tGO 285
#define tIF 286
#define tINTO 287
#define tLOOP 288
#define tMACRO 289
#define tMCI 290
#define tMCIWAIT 291
#define tMOVIE 292
#define tNEXT 293
#define tOF 294
#define tPREVIOUS 295
#define tPUT 296
#define tREPEAT 297
#define tSET 298
#define tTHEN 299
#define tTO 300
#define tWHEN 301
#define tWITH 302
#define tWHILE 303
#define tNLELSE 304
#define tFACTORY 305
#define tMETHOD 306
#define tOPEN 307
#define tPLAY 308
#define tDONE 309
#define tPLAYACCEL 310
#define tGE 311
#define tLE 312
#define tGT 313
#define tLT 314
#define tEQ 315
#define tNEQ 316
#define tAND 317
#define tOR 318
#define tNOT 319
#define tCONCAT 320
#define tCONTAINS 321
#define tSTARTS 322
#define tSPRITE 323
#define tINTERSECTS 324
#define tWITHIN 325




/* Copy the first part of user declarations.  */
#line 49 "engines/director/lingo/lingo-gr.y"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/hash-str.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"

extern int yylex();
extern int yyparse();

using namespace Director;
void yyerror(char *s) {
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
#line 266 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 279 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  91
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   939

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  84
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  126
/* YYNRULES -- Number of states.  */
#define YYNSTATES  263

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   325

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      77,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    76,    82,     2,
      78,    79,    74,    72,    83,    73,     2,    75,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      81,    71,    80,     2,     2,     2,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    14,    15,    17,    19,
      21,    23,    25,    30,    35,    40,    46,    51,    56,    62,
      64,    66,    68,    70,    79,    91,   104,   109,   118,   130,
     142,   149,   160,   171,   172,   176,   179,   181,   184,   186,
     193,   195,   201,   203,   207,   211,   214,   218,   220,   222,
     223,   224,   225,   228,   231,   233,   235,   237,   239,   244,
     246,   248,   251,   253,   257,   261,   265,   269,   273,   277,
     281,   285,   289,   293,   297,   300,   304,   308,   312,   316,
     319,   322,   326,   331,   336,   339,   342,   345,   347,   349,
     351,   354,   357,   360,   362,   365,   370,   373,   375,   379,
     382,   385,   388,   391,   395,   398,   401,   403,   407,   410,
     413,   416,   420,   423,   424,   433,   436,   437,   446,   447,
     449,   453,   458,   459,   463,   464,   466
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      85,     0,    -1,    85,    86,    87,    -1,    87,    -1,     1,
      86,    -1,    77,    -1,    -1,   112,    -1,   106,    -1,   117,
      -1,    88,    -1,    90,    -1,    41,   105,    32,    20,    -1,
      43,    20,    71,   105,    -1,    43,    12,    71,   105,    -1,
      43,    13,   105,    71,   105,    -1,    43,    20,    45,   105,
      -1,    43,    12,    45,   105,    -1,    43,    13,   105,    45,
     105,    -1,   105,    -1,   106,    -1,    89,    -1,    91,    -1,
      98,    78,    97,    79,   104,   103,    26,    42,    -1,    99,
      71,   105,   103,    45,   105,   103,   104,   103,    26,    42,
      -1,    99,    71,   105,   103,    23,    45,   105,   103,   104,
     103,    26,    42,    -1,    46,    20,    44,   105,    -1,   100,
      97,    44,    86,   104,   103,    26,    31,    -1,   100,    97,
      44,    86,   104,   103,    49,   104,   103,    26,    31,    -1,
     100,    97,    44,    86,   104,   103,   102,    93,   103,    26,
      31,    -1,   100,    97,    44,   102,    89,   103,    -1,   100,
      97,    44,   102,    89,   103,    49,   102,    89,   103,    -1,
     100,    97,    44,   102,    89,   103,    94,   103,    92,   103,
      -1,    -1,    49,   102,    89,    -1,    93,    96,    -1,    96,
      -1,    94,    95,    -1,    95,    -1,   101,    97,    44,   102,
      90,   103,    -1,    94,    -1,   101,    97,    44,   104,   103,
      -1,   105,    -1,   105,    71,   105,    -1,    78,    97,    79,
      -1,    42,    48,    -1,    42,    47,    20,    -1,    31,    -1,
      25,    -1,    -1,    -1,    -1,   104,    86,    -1,   104,    90,
      -1,    11,    -1,    14,    -1,    21,    -1,    16,    -1,    20,
      78,   118,    79,    -1,    20,    -1,    12,    -1,    13,   105,
      -1,    88,    -1,   105,    72,   105,    -1,   105,    73,   105,
      -1,   105,    74,   105,    -1,   105,    75,   105,    -1,   105,
      80,   105,    -1,   105,    81,   105,    -1,   105,    61,   105,
      -1,   105,    56,   105,    -1,   105,    57,   105,    -1,   105,
      62,   105,    -1,   105,    63,   105,    -1,    64,   105,    -1,
     105,    82,   105,    -1,   105,    65,   105,    -1,   105,    66,
     105,    -1,   105,    67,   105,    -1,    72,   105,    -1,    73,
     105,    -1,    78,   105,    79,    -1,    68,   105,    69,   105,
      -1,    68,   105,    70,   105,    -1,    35,    21,    -1,    36,
      20,    -1,    41,   105,    -1,   108,    -1,   111,    -1,    27,
      -1,    29,   107,    -1,    18,   105,    -1,    17,   105,    -1,
      17,    -1,    19,   118,    -1,    52,   105,    47,   105,    -1,
      52,   105,    -1,    20,    -1,   107,    83,    20,    -1,    30,
      33,    -1,    30,    38,    -1,    30,    40,    -1,    30,   109,
      -1,    30,   109,   110,    -1,    30,   110,    -1,    28,   105,
      -1,   105,    -1,    39,    37,   105,    -1,    37,   105,    -1,
      53,    54,    -1,    53,   109,    -1,    53,   109,   110,    -1,
      53,   110,    -1,    -1,    34,    20,   113,   102,   115,    86,
     116,   104,    -1,    50,    20,    -1,    -1,    51,    20,   114,
     102,   115,    86,   116,   104,    -1,    -1,    20,    -1,   115,
      83,    20,    -1,   115,    86,    83,    20,    -1,    -1,    20,
     102,   118,    -1,    -1,   105,    -1,   118,    83,   105,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   102,   102,   103,   104,   107,   112,   113,   114,   115,
     116,   117,   120,   126,   132,   140,   148,   154,   162,   171,
     172,   174,   175,   180,   191,   207,   219,   224,   231,   240,
     249,   259,   269,   280,   281,   284,   285,   288,   289,   292,
     300,   301,   309,   310,   311,   313,   315,   321,   327,   334,
     336,   338,   339,   340,   343,   344,   347,   350,   354,   357,
     361,   368,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   389,   390,   391,
     392,   393,   394,   395,   398,   399,   400,   401,   402,   403,
     405,   406,   409,   412,   415,   416,   417,   420,   421,   432,
     433,   434,   435,   438,   441,   446,   447,   450,   451,   454,
     455,   458,   461,   491,   491,   497,   500,   500,   506,   507,
     508,   509,   511,   515,   523,   524,   525
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNARY", "CASTREF", "VOID", "VAR",
  "POINT", "RECT", "ARRAY", "SYMBOL", "INT", "THEENTITY",
  "THEENTITYWITHID", "FLOAT", "BLTIN", "BLTINNOARGS", "BLTINNOARGSORONE",
  "BLTINONEARG", "BLTINARGLIST", "ID", "STRING", "HANDLER", "tDOWN",
  "tELSE", "tNLELSIF", "tEND", "tEXIT", "tFRAME", "tGLOBAL", "tGO", "tIF",
  "tINTO", "tLOOP", "tMACRO", "tMCI", "tMCIWAIT", "tMOVIE", "tNEXT", "tOF",
  "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH",
  "tWHILE", "tNLELSE", "tFACTORY", "tMETHOD", "tOPEN", "tPLAY", "tDONE",
  "tPLAYACCEL", "tGE", "tLE", "tGT", "tLT", "tEQ", "tNEQ", "tAND", "tOR",
  "tNOT", "tCONCAT", "tCONTAINS", "tSTARTS", "tSPRITE", "tINTERSECTS",
  "tWITHIN", "'='", "'+'", "'-'", "'*'", "'/'", "'%'", "'\\n'", "'('",
  "')'", "'>'", "'<'", "'&'", "','", "$accept", "program", "nl",
  "programline", "asgn", "stmtoneliner", "stmt", "ifstmt",
  "elsestmtoneliner", "elseifstmt", "elseifstmtoneliner",
  "elseifstmtoneliner1", "elseifstmt1", "cond", "repeatwhile",
  "repeatwith", "if", "elseif", "begin", "end", "stmtlist", "expr", "func",
  "globallist", "gotofunc", "gotoframe", "gotomovie", "playfunc", "defn",
  "@1", "@2", "argdef", "argstore", "macro", "arglist", 0
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
     325,    61,    43,    45,    42,    47,    37,    10,    40,    41,
      62,    60,    38,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    84,    85,    85,    85,    86,    87,    87,    87,    87,
      87,    87,    88,    88,    88,    88,    88,    88,    88,    89,
      89,    90,    90,    90,    90,    90,    90,    91,    91,    91,
      91,    91,    91,    92,    92,    93,    93,    94,    94,    95,
      96,    96,    97,    97,    97,    98,    99,   100,   101,   102,
     103,   104,   104,   104,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   107,   107,   108,
     108,   108,   108,   108,   108,   109,   109,   110,   110,   111,
     111,   111,   111,   113,   112,   112,   114,   112,   115,   115,
     115,   115,   116,   117,   118,   118,   118
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     0,     1,     1,     1,
       1,     1,     4,     4,     4,     5,     4,     4,     5,     1,
       1,     1,     1,     8,    11,    12,     4,     8,    11,    11,
       6,    10,    10,     0,     3,     2,     1,     2,     1,     6,
       1,     5,     1,     3,     3,     2,     3,     1,     1,     0,
       0,     0,     2,     2,     1,     1,     1,     1,     4,     1,
       1,     2,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     3,     3,     3,     3,     2,
       2,     3,     4,     4,     2,     2,     2,     1,     1,     1,
       2,     2,     2,     1,     2,     4,     2,     1,     3,     2,
       2,     2,     2,     3,     2,     2,     1,     3,     2,     2,
       2,     3,     2,     0,     8,     2,     0,     8,     0,     1,
       3,     4,     0,     3,     0,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    54,    60,     0,    55,    57,    93,     0,   124,
      49,    56,    89,     0,     0,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     3,    62,    21,    11,    22,     0,     0,
       0,    19,     8,    87,    88,     7,     9,     5,     4,    59,
       0,    62,    61,    92,    91,   125,    94,   124,   124,    97,
      90,     0,    99,     0,   100,     0,   101,   106,   102,   104,
     113,    84,    85,    86,     0,    45,     0,     0,     0,     0,
     115,   116,    96,   109,   110,   112,    74,     0,    79,    80,
       0,     1,     6,     0,     0,     0,     0,    42,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   123,     0,   105,   108,
       0,   103,    49,     0,    46,     0,     0,     0,     0,     0,
       0,    49,     0,   111,     0,     0,    81,     2,     0,    50,
       0,     0,    49,     0,    70,    71,    69,    72,    73,    76,
      77,    78,    63,    64,    65,    66,    67,    68,    75,   126,
      58,    98,   107,   118,    12,    17,    14,     0,     0,    16,
      13,    26,   118,    95,    82,    83,    51,     0,    44,    51,
       0,    43,   119,     0,    18,    15,     0,    50,     0,     0,
      50,    50,    20,     0,   122,   122,    52,    53,     0,     0,
      50,    49,    30,   120,     0,    51,    51,     0,    50,    51,
       0,    51,     0,    48,    49,    50,    38,     0,   121,   114,
     117,    23,    51,    50,    27,    50,    50,    40,    36,     0,
       0,    37,    33,     0,    50,     0,     0,    35,     0,     0,
      50,    49,    50,    49,     0,     0,     0,     0,    49,    31,
       0,    32,     0,     0,    24,    28,    29,    50,    34,    50,
      25,    41,    39
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    32,   196,    33,    51,    35,   197,    37,   242,   226,
     227,   216,   228,    96,    38,    39,    40,   217,   252,   177,
     187,    41,   192,    60,    43,    68,    69,    44,    45,   122,
     131,   183,   205,    46,    56
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -199
static const yytype_int16 yypact[] =
{
     248,   -50,  -199,  -199,   169,  -199,  -199,   169,   169,   169,
      49,  -199,  -199,     9,   588,  -199,    13,    26,    18,   169,
       4,    11,    34,    36,    39,   169,   626,   169,   169,   169,
     169,   169,     6,  -199,     7,  -199,  -199,  -199,   -17,    -7,
     394,   813,  -199,  -199,  -199,  -199,  -199,  -199,  -199,   -16,
     169,  -199,   813,   813,   813,   813,   -18,   169,   169,  -199,
      -6,   169,  -199,   169,  -199,    43,  -199,   813,    -5,  -199,
    -199,  -199,  -199,   640,    56,  -199,   -36,   169,   -28,    38,
    -199,  -199,   693,  -199,    -5,  -199,   857,   715,   857,   857,
     764,  -199,   348,   394,   169,   394,    42,   791,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   640,   169,   -58,   -18,    65,   813,   813,
     169,  -199,  -199,    67,  -199,   169,   169,   671,   169,   169,
     169,  -199,   169,  -199,   169,   169,  -199,  -199,    10,   813,
      14,   737,   -50,   169,   813,   813,   813,   813,   813,   813,
     813,   813,   835,   835,   857,   857,   813,   813,   813,   813,
    -199,  -199,   813,    68,  -199,   813,   813,   169,   169,   813,
     813,   813,    68,   813,   813,   813,  -199,    -4,  -199,  -199,
     542,   813,  -199,   -57,   813,   813,   -57,   411,    45,   169,
     411,  -199,  -199,    74,    20,    20,  -199,  -199,    70,   169,
     813,   -10,    -3,  -199,    80,  -199,  -199,    59,   813,  -199,
      76,  -199,    83,  -199,  -199,    83,  -199,   394,  -199,   411,
     411,  -199,  -199,   411,  -199,   411,    83,    83,  -199,   394,
     542,  -199,    60,    73,   411,    93,    94,  -199,    95,    81,
    -199,  -199,  -199,  -199,   102,    90,   106,   125,   -14,  -199,
     542,  -199,   479,   116,  -199,  -199,  -199,   411,  -199,  -199,
    -199,  -199,  -199
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -199,  -199,    12,    71,     3,  -172,     0,  -199,  -199,  -199,
     -45,  -197,   -67,   -91,  -199,  -199,  -199,  -198,    -9,   115,
    -169,    41,     5,  -199,  -199,   134,   -11,  -199,  -199,  -199,
    -199,    -8,   -33,  -199,    17
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -60
static const yytype_int16 yytable[] =
{
      36,    58,   138,    34,   140,    42,    91,   -10,   191,   125,
     190,   -51,   -51,    48,   229,    85,   210,   128,   231,   188,
      47,   160,   213,    76,    77,   114,   193,    47,   229,    59,
     231,    78,    63,    70,    65,   126,   219,   220,    72,   211,
     223,   189,   225,   129,    92,    52,   214,    71,    53,    54,
      55,    74,    75,   234,    79,    67,    80,   121,   240,    81,
      73,    93,    57,   -51,    94,   114,    82,    67,    86,    87,
      88,    89,    90,   133,   115,   116,   124,   117,   258,   257,
     120,    97,   130,    47,   -10,   161,   142,   164,   182,   176,
     199,   113,    36,   178,   203,    34,   207,    42,    55,    55,
     218,   221,   118,   204,   119,   -59,   -59,   224,   213,   241,
     -59,   -59,   -59,   163,   -59,   -59,   -59,   243,   127,   245,
     246,   247,   172,   -59,   -59,   248,   233,    57,   253,   -59,
     -59,   -59,   254,   180,    97,   139,   141,   255,   239,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   179,   159,   256,   215,   260,   237,
      84,   162,   206,   137,   186,     0,   165,   166,     0,   169,
     170,   171,     0,   173,     0,   174,   175,     0,     0,     0,
       2,     3,     4,     5,   181,     6,     0,     0,     0,    49,
      11,     0,   212,     0,     0,   194,     0,     0,   195,     0,
       0,     0,     0,     0,     0,   230,     0,     0,   184,   185,
      50,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     200,     0,   250,    27,     0,     0,     0,    28,     0,     0,
     208,    29,    30,     0,     0,     0,     0,    31,    -6,     1,
       0,     0,   259,     0,     0,     0,     0,     0,    97,     2,
       3,     4,     5,     0,     6,     7,     8,     9,    10,    11,
      97,     0,     0,     0,     0,    12,     0,    13,    14,    15,
       0,     0,    16,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,    23,    24,
      25,    26,   198,     0,     0,   201,   202,     0,     0,     0,
       0,     0,    27,     0,     0,   209,    28,     0,     0,     0,
      29,    30,     0,   222,     0,    -6,    31,     0,     0,     0,
     232,     0,     0,     0,     0,     0,     0,     0,   235,     0,
     236,   238,     0,     0,     0,     0,     0,     0,     0,   244,
       0,     0,     0,     0,     0,   249,     0,   251,     0,     2,
       3,     4,     5,     0,     6,     7,     8,     9,    10,    11,
       0,     0,   261,     0,   262,    12,     0,    13,    14,    15,
       0,     0,    16,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,    23,    24,
      25,    26,     0,     0,     0,     2,     3,     4,     5,     0,
       6,     0,    27,     0,    49,    11,    28,     0,     0,     0,
      29,    30,     2,     3,     4,     5,    31,     6,     7,     8,
       9,    49,    11,     0,     0,    50,     0,    21,    12,     0,
      13,    14,    15,     0,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,    27,     0,
       0,     0,    28,    25,    26,     0,    29,    30,     0,     0,
       0,     0,    95,     0,     0,    27,     0,     0,     0,    28,
       0,     0,     0,    29,    30,     0,     0,     0,    47,    31,
       2,     3,     4,     5,     0,     6,     7,     8,     9,    49,
      11,     0,     0,     0,     0,     0,    12,     0,    13,    14,
      15,     0,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,     0,
       0,    25,    26,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,    28,     0,     0,
       0,    29,    30,     2,     3,     4,     5,    31,     6,     7,
       8,     9,    49,    11,     0,     0,     0,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    17,    18,     0,
       0,     0,     0,    19,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     2,
       3,     4,     5,     0,     6,     0,    27,     0,    49,    11,
      28,     0,     0,     0,    29,    30,    61,     0,     0,     0,
      31,    62,     0,     0,     0,    63,    64,    65,    66,    50,
       0,    21,     0,     0,     0,     0,     0,     2,     3,     4,
       5,     0,     6,     0,     0,     0,    49,    11,     0,     0,
       0,     0,    27,     0,    61,     0,    28,     0,     0,     0,
      29,    30,     0,    63,     0,    65,    31,    50,     0,    21,
       0,     0,   123,     0,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,    28,     0,    98,    99,    29,    30,
       0,   100,   101,   102,    31,   103,   104,   105,     0,     0,
       0,     0,   106,   107,   108,   109,   167,     0,     0,     0,
     110,   111,   112,     0,     0,     0,     0,    98,    99,     0,
       0,     0,   100,   101,   102,     0,   103,   104,   105,     0,
     132,     0,   168,   106,   107,   108,   109,     0,     0,    98,
      99,   110,   111,   112,   100,   101,   102,     0,   103,   104,
     105,     0,     0,     0,     0,   106,   107,   108,   109,     0,
       0,    98,    99,   110,   111,   112,   100,   101,   102,     0,
     103,   104,   105,     0,   134,   135,     0,   106,   107,   108,
     109,     0,     0,    98,    99,   110,   111,   112,   100,   101,
     102,     0,   103,   104,   105,     0,     0,     0,   143,   106,
     107,   108,   109,     0,     0,     0,   136,   110,   111,   112,
      98,    99,     0,     0,     0,   100,   101,   102,     0,   103,
     104,   105,     0,     0,     0,     0,   106,   107,   108,   109,
       0,     0,     0,   136,   110,   111,   112,    98,    99,     0,
       0,     0,   100,   101,   102,     0,   103,   104,   105,     0,
       0,     0,   143,   106,   107,   108,   109,     0,     0,    98,
      99,   110,   111,   112,   100,   101,   102,     0,   103,   104,
     105,     0,     0,     0,     0,   106,   107,   108,   109,     0,
       0,    98,    99,   110,   111,   112,   100,   101,   102,     0,
     103,   104,   105,     0,     0,     0,     0,     0,     0,   108,
     109,     0,     0,    98,    99,   110,   111,   112,   100,   101,
     102,     0,   103,   104,   105,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   110,   111,   112
};

static const yytype_int16 yycheck[] =
{
       0,    10,    93,     0,    95,     0,     0,     0,   180,    45,
     179,    25,    26,     1,   212,    26,    26,    45,   215,    23,
      77,    79,    25,    12,    13,    83,    83,    77,   226,    20,
     227,    20,    37,    20,    39,    71,   205,   206,    20,    49,
     209,    45,   211,    71,    32,     4,    49,    21,     7,     8,
       9,    47,    48,   222,    20,    14,    20,    68,   230,    20,
      19,    78,    78,    77,    71,    83,    25,    26,    27,    28,
      29,    30,    31,    84,    57,    58,    20,    83,   250,   248,
      37,    40,    44,    77,    77,    20,    44,    20,    20,    79,
      45,    50,    92,    79,    20,    92,    26,    92,    57,    58,
      20,    42,    61,    83,    63,    56,    57,    31,    25,    49,
      61,    62,    63,   122,    65,    66,    67,    44,    77,    26,
      26,    26,   131,    74,    75,    44,   217,    78,    26,    80,
      81,    82,    42,   142,    93,    94,    95,    31,   229,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   142,   114,    31,   202,    42,   226,
      26,   120,   195,    92,   172,    -1,   125,   126,    -1,   128,
     129,   130,    -1,   132,    -1,   134,   135,    -1,    -1,    -1,
      11,    12,    13,    14,   143,    16,    -1,    -1,    -1,    20,
      21,    -1,   201,    -1,    -1,   183,    -1,    -1,   186,    -1,
      -1,    -1,    -1,    -1,    -1,   214,    -1,    -1,   167,   168,
      41,    -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     189,    -1,   241,    64,    -1,    -1,    -1,    68,    -1,    -1,
     199,    72,    73,    -1,    -1,    -1,    -1,    78,     0,     1,
      -1,    -1,   252,    -1,    -1,    -1,    -1,    -1,   217,    11,
      12,    13,    14,    -1,    16,    17,    18,    19,    20,    21,
     229,    -1,    -1,    -1,    -1,    27,    -1,    29,    30,    31,
      -1,    -1,    34,    35,    36,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    46,    -1,    -1,    -1,    50,    51,
      52,    53,   187,    -1,    -1,   190,   191,    -1,    -1,    -1,
      -1,    -1,    64,    -1,    -1,   200,    68,    -1,    -1,    -1,
      72,    73,    -1,   208,    -1,    77,    78,    -1,    -1,    -1,
     215,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,
     225,   226,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   234,
      -1,    -1,    -1,    -1,    -1,   240,    -1,   242,    -1,    11,
      12,    13,    14,    -1,    16,    17,    18,    19,    20,    21,
      -1,    -1,   257,    -1,   259,    27,    -1,    29,    30,    31,
      -1,    -1,    34,    35,    36,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    46,    -1,    -1,    -1,    50,    51,
      52,    53,    -1,    -1,    -1,    11,    12,    13,    14,    -1,
      16,    -1,    64,    -1,    20,    21,    68,    -1,    -1,    -1,
      72,    73,    11,    12,    13,    14,    78,    16,    17,    18,
      19,    20,    21,    -1,    -1,    41,    -1,    43,    27,    -1,
      29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    46,    64,    -1,
      -1,    -1,    68,    52,    53,    -1,    72,    73,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    64,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    72,    73,    -1,    -1,    -1,    77,    78,
      11,    12,    13,    14,    -1,    16,    17,    18,    19,    20,
      21,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    30,
      31,    -1,    -1,    -1,    35,    36,    -1,    -1,    -1,    -1,
      41,    42,    43,    -1,    -1,    46,    -1,    -1,    -1,    -1,
      -1,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    72,    73,    11,    12,    13,    14,    78,    16,    17,
      18,    19,    20,    21,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    -1,    41,    -1,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    53,    -1,    -1,    -1,    11,
      12,    13,    14,    -1,    16,    -1,    64,    -1,    20,    21,
      68,    -1,    -1,    -1,    72,    73,    28,    -1,    -1,    -1,
      78,    33,    -1,    -1,    -1,    37,    38,    39,    40,    41,
      -1,    43,    -1,    -1,    -1,    -1,    -1,    11,    12,    13,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    -1,    -1,
      -1,    -1,    64,    -1,    28,    -1,    68,    -1,    -1,    -1,
      72,    73,    -1,    37,    -1,    39,    78,    41,    -1,    43,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      64,    -1,    -1,    -1,    68,    -1,    56,    57,    72,    73,
      -1,    61,    62,    63,    78,    65,    66,    67,    -1,    -1,
      -1,    -1,    72,    73,    74,    75,    45,    -1,    -1,    -1,
      80,    81,    82,    -1,    -1,    -1,    -1,    56,    57,    -1,
      -1,    -1,    61,    62,    63,    -1,    65,    66,    67,    -1,
      47,    -1,    71,    72,    73,    74,    75,    -1,    -1,    56,
      57,    80,    81,    82,    61,    62,    63,    -1,    65,    66,
      67,    -1,    -1,    -1,    -1,    72,    73,    74,    75,    -1,
      -1,    56,    57,    80,    81,    82,    61,    62,    63,    -1,
      65,    66,    67,    -1,    69,    70,    -1,    72,    73,    74,
      75,    -1,    -1,    56,    57,    80,    81,    82,    61,    62,
      63,    -1,    65,    66,    67,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      56,    57,    -1,    -1,    -1,    61,    62,    63,    -1,    65,
      66,    67,    -1,    -1,    -1,    -1,    72,    73,    74,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    56,    57,    -1,
      -1,    -1,    61,    62,    63,    -1,    65,    66,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    -1,    -1,    56,
      57,    80,    81,    82,    61,    62,    63,    -1,    65,    66,
      67,    -1,    -1,    -1,    -1,    72,    73,    74,    75,    -1,
      -1,    56,    57,    80,    81,    82,    61,    62,    63,    -1,
      65,    66,    67,    -1,    -1,    -1,    -1,    -1,    -1,    74,
      75,    -1,    -1,    56,    57,    80,    81,    82,    61,    62,
      63,    -1,    65,    66,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    11,    12,    13,    14,    16,    17,    18,    19,
      20,    21,    27,    29,    30,    31,    34,    35,    36,    41,
      42,    43,    46,    50,    51,    52,    53,    64,    68,    72,
      73,    78,    85,    87,    88,    89,    90,    91,    98,    99,
     100,   105,   106,   108,   111,   112,   117,    77,    86,    20,
      41,    88,   105,   105,   105,   105,   118,    78,   102,    20,
     107,    28,    33,    37,    38,    39,    40,   105,   109,   110,
      20,    21,    20,   105,    47,    48,    12,    13,    20,    20,
      20,    20,   105,    54,   109,   110,   105,   105,   105,   105,
     105,     0,    86,    78,    71,    78,    97,   105,    56,    57,
      61,    62,    63,    65,    66,    67,    72,    73,    74,    75,
      80,    81,    82,   105,    83,   118,   118,    83,   105,   105,
      37,   110,   113,    32,    20,    45,    71,   105,    45,    71,
      44,   114,    47,   110,    69,    70,    79,    87,    97,   105,
      97,   105,    44,    71,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
      79,    20,   105,   102,    20,   105,   105,    45,    71,   105,
     105,   105,   102,   105,   105,   105,    79,   103,    79,    86,
     102,   105,    20,   115,   105,   105,   115,   104,    23,    45,
     104,    89,   106,    83,    86,    86,    86,    90,   103,    45,
     105,   103,   103,    20,    83,   116,   116,    26,   105,   103,
      26,    49,   102,    25,    49,    94,    95,   101,    20,   104,
     104,    42,   103,   104,    31,   104,    93,    94,    96,   101,
     102,    95,   103,    97,   104,   103,   103,    96,   103,    97,
      89,    49,    92,    44,   103,    26,    26,    26,    44,   103,
     102,   103,   102,    26,    42,    31,    31,   104,    89,    90,
      42,   103,   103
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
#line 104 "engines/director/lingo/lingo-gr.y"
    { yyerrok; ;}
    break;

  case 5:
#line 107 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1;
	;}
    break;

  case 10:
#line 116 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 12:
#line 120 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 13:
#line 126 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 14:
#line 132 "engines/director/lingo/lingo-gr.y"
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
#line 140 "engines/director/lingo/lingo-gr.y"
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
#line 148 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 17:
#line 154 "engines/director/lingo/lingo-gr.y"
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
#line 162 "engines/director/lingo/lingo-gr.y"
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
#line 171 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 23:
#line 180 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 2] = end; ;}
    break;

  case 24:
#line 191 "engines/director/lingo/lingo-gr.y"
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
#line 207 "engines/director/lingo/lingo-gr.y"
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
#line 219 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_ifcode);
		;}
    break;

  case 27:
#line 224 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 3] = end;	/* end, if cond fails */
		g_lingo->processIf(0, 0); ;}
    break;

  case 28:
#line 231 "engines/director/lingo/lingo-gr.y"
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
#line 240 "engines/director/lingo/lingo-gr.y"
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
#line 249 "engines/director/lingo/lingo-gr.y"
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
#line 259 "engines/director/lingo/lingo-gr.y"
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
#line 269 "engines/director/lingo/lingo-gr.y"
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
#line 280 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 34:
#line 281 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 39:
#line 292 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 41:
#line 301 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (5)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (5)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (5)].code)); ;}
    break;

  case 42:
#line 309 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 43:
#line 310 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 45:
#line 313 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 46:
#line 315 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 47:
#line 321 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 48:
#line 327 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 49:
#line 334 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 50:
#line 336 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 51:
#line 338 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 54:
#line 343 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->codeConst((yyvsp[(1) - (1)].i)); ;}
    break;

  case 55:
#line 344 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_fconstpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 56:
#line 347 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 57:
#line 350 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->_handlers[*(yyvsp[(1) - (1)].s)]->u.func);
		g_lingo->codeConst(0); // Put dummy value
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 58:
#line 354 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 59:
#line 357 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 60:
#line 361 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 61:
#line 368 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 63:
#line 375 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 64:
#line 376 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 65:
#line 377 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 66:
#line 378 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 67:
#line 379 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 68:
#line 380 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 69:
#line 381 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 70:
#line 382 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 71:
#line 383 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 72:
#line 384 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 73:
#line 385 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 74:
#line 386 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 75:
#line 387 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 76:
#line 388 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 77:
#line 389 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 78:
#line 390 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 79:
#line 391 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 80:
#line 392 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 81:
#line 393 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 82:
#line 394 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 83:
#line 395 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 84:
#line 398 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mci); g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str()); delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 85:
#line 399 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mciwait); g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str()); delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 86:
#line 400 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 89:
#line 403 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeConst(0); // Push fake value on stack
							  g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 91:
#line 406 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_handlers[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 92:
#line 409 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_handlers[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 93:
#line 412 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code2(g_lingo->c_voidpush, g_lingo->_handlers[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 94:
#line 415 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 95:
#line 416 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 96:
#line 417 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 97:
#line 420 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 98:
#line 421 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 99:
#line 432 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 100:
#line 433 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 101:
#line 434 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 102:
#line 435 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 103:
#line 438 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 104:
#line 441 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 109:
#line 454 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 110:
#line 455 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 111:
#line 458 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 112:
#line 461 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 113:
#line 491 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 114:
#line 492 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeConst(0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 115:
#line 497 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeFactory(*(yyvsp[(2) - (2)].s));
		;}
    break;

  case 116:
#line 500 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 117:
#line 501 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeConst(0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg), &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 118:
#line 506 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 119:
#line 507 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 120:
#line 508 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 121:
#line 509 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 122:
#line 511 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 123:
#line 515 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (3)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(3) - (3)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 124:
#line 523 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 125:
#line 524 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 126:
#line 525 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2552 "engines/director/lingo/lingo-gr.cpp"
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


#line 528 "engines/director/lingo/lingo-gr.y"


