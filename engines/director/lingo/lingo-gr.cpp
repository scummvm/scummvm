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
     vCASTREF = 259,
     vVOID = 260,
     vVAR = 261,
     vPOINT = 262,
     vRECT = 263,
     vARRAY = 264,
     vSYMBOL = 265,
     vINT = 266,
     vTHEENTITY = 267,
     vTHEENTITYWITHID = 268,
     vFLOAT = 269,
     vBLTIN = 270,
     vBLTINNOARGS = 271,
     vSTRING = 272,
     vHANDLER = 273,
     ID = 274,
     tDOWN = 275,
     tELSE = 276,
     tNLELSIF = 277,
     tEND = 278,
     tEXIT = 279,
     tFRAME = 280,
     tGLOBAL = 281,
     tGO = 282,
     tIF = 283,
     tINTO = 284,
     tLOOP = 285,
     tMACRO = 286,
     tMCI = 287,
     tMCIWAIT = 288,
     tMOVIE = 289,
     tNEXT = 290,
     tOF = 291,
     tPREVIOUS = 292,
     tPUT = 293,
     tREPEAT = 294,
     tSET = 295,
     tTHEN = 296,
     tTO = 297,
     tWHEN = 298,
     tWITH = 299,
     tWHILE = 300,
     tNLELSE = 301,
     tFACTORY = 302,
     tMETHOD = 303,
     tALERT = 304,
     tBEEP = 305,
     tCLOSERESFILE = 306,
     tCLOSEXLIB = 307,
     tGE = 308,
     tLE = 309,
     tGT = 310,
     tLT = 311,
     tEQ = 312,
     tNEQ = 313,
     tAND = 314,
     tOR = 315,
     tNOT = 316,
     tCONCAT = 317,
     tCONTAINS = 318,
     tSTARTS = 319,
     tSPRITE = 320,
     tINTERSECTS = 321,
     tWITHIN = 322
   };
#endif
/* Tokens.  */
#define UNARY 258
#define vCASTREF 259
#define vVOID 260
#define vVAR 261
#define vPOINT 262
#define vRECT 263
#define vARRAY 264
#define vSYMBOL 265
#define vINT 266
#define vTHEENTITY 267
#define vTHEENTITYWITHID 268
#define vFLOAT 269
#define vBLTIN 270
#define vBLTINNOARGS 271
#define vSTRING 272
#define vHANDLER 273
#define ID 274
#define tDOWN 275
#define tELSE 276
#define tNLELSIF 277
#define tEND 278
#define tEXIT 279
#define tFRAME 280
#define tGLOBAL 281
#define tGO 282
#define tIF 283
#define tINTO 284
#define tLOOP 285
#define tMACRO 286
#define tMCI 287
#define tMCIWAIT 288
#define tMOVIE 289
#define tNEXT 290
#define tOF 291
#define tPREVIOUS 292
#define tPUT 293
#define tREPEAT 294
#define tSET 295
#define tTHEN 296
#define tTO 297
#define tWHEN 298
#define tWITH 299
#define tWHILE 300
#define tNLELSE 301
#define tFACTORY 302
#define tMETHOD 303
#define tALERT 304
#define tBEEP 305
#define tCLOSERESFILE 306
#define tCLOSEXLIB 307
#define tGE 308
#define tLE 309
#define tGT 310
#define tLT 311
#define tEQ 312
#define tNEQ 313
#define tAND 314
#define tOR 315
#define tNOT 316
#define tCONCAT 317
#define tCONTAINS 318
#define tSTARTS 319
#define tSPRITE 320
#define tINTERSECTS 321
#define tWITHIN 322




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
#line 260 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 273 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  85
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   801

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  81
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  34
/* YYNRULES -- Number of rules.  */
#define YYNRULES  125
/* YYNRULES -- Number of states.  */
#define YYNSTATES  262

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      74,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    73,    79,     2,
      75,    76,    71,    69,    80,    70,     2,    72,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      78,    68,    77,     2,     2,     2,     2,     2,     2,     2,
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
      65,    66,    67
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
     352,   355,   358,   360,   363,   365,   368,   370,   372,   376,
     379,   382,   385,   388,   392,   395,   399,   402,   405,   407,
     411,   414,   418,   419,   428,   431,   432,   441,   442,   444,
     448,   453,   454,   458,   459,   461
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      82,     0,    -1,    82,    83,    84,    -1,    84,    -1,     1,
      74,    -1,    74,    -1,    -1,   108,    -1,   103,    -1,   113,
      -1,    85,    -1,    87,    -1,    38,   102,    29,    19,    -1,
      40,    19,    68,   102,    -1,    40,    12,    68,   102,    -1,
      40,    13,   102,    68,   102,    -1,    40,    19,    42,   102,
      -1,    40,    12,    42,   102,    -1,    40,    13,   102,    42,
     102,    -1,   102,    -1,   103,    -1,    86,    -1,    88,    -1,
      95,    75,    94,    76,   101,   100,    23,    39,    -1,    96,
      68,   102,   100,    42,   102,   100,   101,   100,    23,    39,
      -1,    96,    68,   102,   100,    20,    42,   102,   100,   101,
     100,    23,    39,    -1,    43,    19,    41,   102,    -1,    97,
      94,    41,    83,   101,   100,    23,    28,    -1,    97,    94,
      41,    83,   101,   100,    46,   101,   100,    23,    28,    -1,
      97,    94,    41,    83,   101,   100,    99,    90,   100,    23,
      28,    -1,    97,    94,    41,    99,    86,   100,    -1,    97,
      94,    41,    99,    86,   100,    46,    99,    86,   100,    -1,
      97,    94,    41,    99,    86,   100,    91,   100,    89,   100,
      -1,    -1,    46,    99,    86,    -1,    90,    93,    -1,    93,
      -1,    91,    92,    -1,    92,    -1,    98,    94,    41,    99,
      87,   100,    -1,    91,    -1,    98,    94,    41,   101,   100,
      -1,   102,    -1,   102,    68,   102,    -1,    75,    94,    76,
      -1,    39,    45,    -1,    39,    44,    19,    -1,    28,    -1,
      22,    -1,    -1,    -1,    -1,   101,    83,    -1,   101,    87,
      -1,    11,    -1,    14,    -1,    17,    -1,    16,    -1,    19,
      75,   114,    76,    -1,    19,    -1,    12,    -1,    13,   102,
      -1,    85,    -1,   102,    69,   102,    -1,   102,    70,   102,
      -1,   102,    71,   102,    -1,   102,    72,   102,    -1,   102,
      77,   102,    -1,   102,    78,   102,    -1,   102,    58,   102,
      -1,   102,    53,   102,    -1,   102,    54,   102,    -1,   102,
      59,   102,    -1,   102,    60,   102,    -1,    61,   102,    -1,
     102,    79,   102,    -1,   102,    62,   102,    -1,   102,    63,
     102,    -1,   102,    64,   102,    -1,    69,   102,    -1,    70,
     102,    -1,    75,   102,    76,    -1,    65,   102,    66,   102,
      -1,    65,   102,    67,   102,    -1,    32,    17,    -1,    33,
      19,    -1,    38,   102,    -1,   105,    -1,    24,    -1,    26,
     104,    -1,    49,   102,    -1,    50,    11,    -1,    50,    -1,
      51,   102,    -1,    51,    -1,    52,   102,    -1,    52,    -1,
      19,    -1,   104,    80,    19,    -1,    27,    30,    -1,    27,
      35,    -1,    27,    37,    -1,    27,   106,    -1,    27,   106,
     107,    -1,    27,   107,    -1,    42,    25,    17,    -1,    25,
      17,    -1,    42,    17,    -1,    17,    -1,    36,    34,    17,
      -1,    34,    17,    -1,    42,    34,    17,    -1,    -1,    31,
      19,   109,    99,   111,    83,   112,   101,    -1,    47,    19,
      -1,    -1,    48,    19,   110,    99,   111,    83,   112,   101,
      -1,    -1,    19,    -1,   111,    80,    19,    -1,   111,    83,
      80,    19,    -1,    -1,    19,    99,   114,    -1,    -1,   102,
      -1,   114,    80,   102,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   104,   104,   105,   106,   109,   114,   115,   116,   117,
     118,   119,   122,   128,   134,   142,   150,   156,   164,   173,
     174,   176,   177,   182,   193,   209,   221,   226,   233,   242,
     251,   261,   271,   282,   283,   286,   287,   290,   291,   294,
     302,   303,   311,   312,   313,   315,   317,   323,   329,   336,
     338,   340,   341,   342,   345,   350,   353,   356,   360,   363,
     367,   374,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   389,   390,   391,   392,   393,   394,   395,   396,   397,
     398,   399,   400,   401,   404,   405,   406,   407,   408,   410,
     411,   412,   418,   424,   425,   431,   432,   440,   441,   452,
     453,   454,   455,   460,   466,   473,   474,   475,   476,   479,
     480,   481,   509,   509,   515,   518,   518,   524,   525,   526,
     527,   529,   533,   541,   542,   543
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNARY", "vCASTREF", "vVOID", "vVAR",
  "vPOINT", "vRECT", "vARRAY", "vSYMBOL", "vINT", "vTHEENTITY",
  "vTHEENTITYWITHID", "vFLOAT", "vBLTIN", "vBLTINNOARGS", "vSTRING",
  "vHANDLER", "ID", "tDOWN", "tELSE", "tNLELSIF", "tEND", "tEXIT",
  "tFRAME", "tGLOBAL", "tGO", "tIF", "tINTO", "tLOOP", "tMACRO", "tMCI",
  "tMCIWAIT", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT",
  "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tNLELSE",
  "tFACTORY", "tMETHOD", "tALERT", "tBEEP", "tCLOSERESFILE", "tCLOSEXLIB",
  "tGE", "tLE", "tGT", "tLT", "tEQ", "tNEQ", "tAND", "tOR", "tNOT",
  "tCONCAT", "tCONTAINS", "tSTARTS", "tSPRITE", "tINTERSECTS", "tWITHIN",
  "'='", "'+'", "'-'", "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "'>'",
  "'<'", "'&'", "','", "$accept", "program", "nl", "programline", "asgn",
  "stmtoneliner", "stmt", "ifstmt", "elsestmtoneliner", "elseifstmt",
  "elseifstmtoneliner", "elseifstmtoneliner1", "elseifstmt1", "cond",
  "repeatwhile", "repeatwith", "if", "elseif", "begin", "end", "stmtlist",
  "expr", "func", "globallist", "gotofunc", "gotoframe", "gotomovie",
  "defn", "@1", "@2", "argdef", "argstore", "macro", "arglist", 0
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
     315,   316,   317,   318,   319,   320,   321,   322,    61,    43,
      45,    42,    47,    37,    10,    40,    41,    62,    60,    38,
      44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    81,    82,    82,    82,    83,    84,    84,    84,    84,
      84,    84,    85,    85,    85,    85,    85,    85,    85,    86,
      86,    87,    87,    87,    87,    87,    87,    88,    88,    88,
      88,    88,    88,    89,    89,    90,    90,    91,    91,    92,
      93,    93,    94,    94,    94,    95,    96,    97,    98,    99,
     100,   101,   101,   101,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   104,   104,   105,
     105,   105,   105,   105,   105,   106,   106,   106,   106,   107,
     107,   107,   109,   108,   108,   110,   108,   111,   111,   111,
     111,   112,   113,   114,   114,   114
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
       2,     3,     4,     4,     2,     2,     2,     1,     1,     2,
       2,     2,     1,     2,     1,     2,     1,     1,     3,     2,
       2,     2,     2,     3,     2,     3,     2,     2,     1,     3,
       2,     3,     0,     8,     2,     0,     8,     0,     1,     3,
       4,     0,     3,     0,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    54,    60,     0,    55,    57,    56,    49,    88,
       0,     0,    47,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    92,    94,    96,     0,     0,     0,     0,
       0,     0,     3,    62,    21,    11,    22,     0,     0,     0,
      19,     8,    87,     7,     9,     4,    59,     0,    62,    61,
     123,   123,    97,    89,   108,     0,    99,     0,   100,     0,
     101,     0,   102,   104,   112,    84,    85,    86,     0,    45,
       0,     0,     0,     0,   114,   115,    90,    91,    93,    95,
      74,     0,    79,    80,     0,     1,     5,     6,     0,     0,
       0,     0,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   124,
       0,   122,     0,   106,   110,     0,   107,     0,     0,     0,
     103,    49,     0,    46,     0,     0,     0,     0,     0,     0,
      49,     0,     0,    81,     2,     0,    50,     0,     0,    49,
       0,    70,    71,    69,    72,    73,    76,    77,    78,    63,
      64,    65,    66,    67,    68,    75,    58,     0,    98,   109,
     105,   111,   117,    12,    17,    14,     0,     0,    16,    13,
      26,   117,    82,    83,    51,     0,    44,    51,     0,    43,
     125,   118,     0,    18,    15,     0,    50,     0,     0,    50,
      50,    20,     0,   121,   121,    52,    53,     0,     0,    50,
      49,    30,   119,     0,    51,    51,     0,    50,    51,     0,
      51,     0,    48,    49,    50,    38,     0,   120,   113,   116,
      23,    51,    50,    27,    50,    50,    40,    36,     0,     0,
      37,    33,     0,    50,     0,     0,    35,     0,     0,    50,
      49,    50,    49,     0,     0,     0,     0,    49,    31,     0,
      32,     0,     0,    24,    28,    29,    50,    34,    50,    25,
      41,    39
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    31,   195,    32,    48,    34,   196,    36,   241,   225,
     226,   215,   227,    91,    37,    38,    39,   216,   251,   175,
     186,    40,   191,    53,    42,    62,    63,    43,   121,   130,
     182,   204,    44,   110
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -192
static const yytype_int16 yypact[] =
{
     196,   -57,  -192,  -192,   105,  -192,  -192,  -192,   695,  -192,
      26,   147,  -192,    28,     5,    31,   105,    14,    70,    49,
      56,    60,   105,     9,   105,   105,   105,   105,   105,   105,
     105,     3,  -192,     4,  -192,  -192,  -192,    13,    43,   530,
     226,  -192,  -192,  -192,  -192,  -192,    18,   105,  -192,   226,
     105,   105,  -192,    35,  -192,    77,  -192,    96,  -192,    91,
    -192,    29,    38,  -192,  -192,  -192,  -192,   544,   109,  -192,
     -34,   105,   -32,    95,  -192,  -192,   226,  -192,   226,   226,
       7,   597,     7,     7,   646,  -192,  -192,   302,   530,   105,
     530,    97,   673,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   544,   226,
     -61,    57,   120,  -192,  -192,   123,  -192,   124,   127,   112,
    -192,  -192,   129,  -192,   105,   105,   575,   105,   105,   105,
    -192,   105,   105,  -192,  -192,    73,   226,    74,   619,    78,
     105,   226,   226,   226,   226,   226,   226,   226,   226,   722,
     722,     7,     7,   226,   226,   226,  -192,   105,  -192,  -192,
    -192,  -192,   132,  -192,   226,   226,   105,   105,   226,   226,
     226,   132,   226,   226,  -192,     1,  -192,  -192,   487,   226,
     226,  -192,   -23,   226,   226,   -23,   362,   113,   105,   362,
    -192,  -192,   135,    80,    80,  -192,  -192,   134,   105,   226,
      -9,    -6,  -192,   139,  -192,  -192,   122,   226,  -192,   137,
    -192,   140,  -192,  -192,   140,  -192,   530,  -192,   362,   362,
    -192,  -192,   362,  -192,   362,   140,   140,  -192,   530,   487,
    -192,   117,   126,   362,   148,   150,  -192,   153,   138,  -192,
    -192,  -192,  -192,   155,   146,   158,   159,   -10,  -192,   487,
    -192,   427,   149,  -192,  -192,  -192,   362,  -192,  -192,  -192,
    -192,  -192
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -192,  -192,   -29,   104,    25,  -173,     0,  -192,  -192,  -192,
      -3,  -191,   -33,   -81,  -192,  -192,  -192,  -163,    -7,    69,
    -166,     2,    33,  -192,  -192,  -192,   133,  -192,  -192,  -192,
      23,     8,  -192,   152
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -60
static const yytype_int16 yytable[] =
{
      35,    51,    87,    85,   -10,   190,    49,   135,   124,   137,
     127,   189,   -51,   -51,   209,   156,   212,    45,    67,   157,
      77,   187,    65,   230,    76,    33,    78,    79,    80,    81,
      82,    83,    84,    41,   125,   230,   128,   210,   218,   219,
     213,    92,   222,   188,   224,    52,   116,    64,   228,   108,
      66,    86,   109,   109,   117,   233,   239,   192,    68,    69,
      93,    94,   228,   118,   -51,    95,    96,    97,    73,    98,
      99,   100,    57,   126,    59,    74,   257,    86,   -10,    75,
     119,   256,    70,    71,   105,   106,   107,    35,    88,    72,
      92,   136,   138,    50,   113,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     177,    89,    33,   114,   162,   112,     2,     3,     4,     5,
      41,     6,     7,   171,    46,   115,   164,   165,   123,   168,
     169,   170,   178,   172,   173,   232,   129,   157,   139,   158,
     159,   160,   179,    47,   161,    18,   118,   238,   163,   174,
     176,   181,    86,   193,   202,   198,   194,   206,   217,   180,
     203,   220,   212,   240,    54,   223,    26,   242,   183,   184,
      27,   244,    55,   245,    28,    29,   246,    56,   252,   247,
      30,    57,    58,    59,    60,   253,   254,   255,   259,    61,
     199,   134,   236,   211,   185,   120,    -6,     1,   214,     0,
     207,     0,   205,   111,     0,     0,   229,     2,     3,     4,
       5,     0,     6,     7,     0,     8,     0,     0,    92,     0,
       9,     0,    10,    11,    12,     0,     0,    13,    14,    15,
      92,     0,     0,   249,    16,    17,    18,     0,     0,    19,
       0,     0,     0,    20,    21,    22,    23,    24,    25,     0,
       0,   258,     0,     0,     0,   197,     0,    26,   200,   201,
       0,    27,     0,     0,     0,    28,    29,     0,   208,     0,
      -6,    30,     0,     0,     0,     0,   221,     0,     0,    93,
      94,     0,     0,   231,    95,    96,    97,     0,    98,    99,
     100,   234,     0,   235,   237,   101,   102,   103,   104,     0,
       0,     0,   243,   105,   106,   107,     0,     0,   248,     0,
     250,     0,     0,     2,     3,     4,     5,     0,     6,     7,
       0,     8,     0,     0,     0,   260,     9,   261,    10,    11,
      12,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,    17,    18,     0,     0,    19,     0,     0,     0,    20,
      21,    22,    23,    24,    25,     0,     0,     0,     0,     0,
       0,     0,     0,    26,     0,     0,     0,    27,     0,     0,
       0,    28,    29,     2,     3,     4,     5,    30,     6,     7,
       0,    46,     0,     0,     0,     0,     9,     0,    10,    11,
      12,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,    17,    18,     0,     0,    19,     0,     0,     0,     0,
       0,    22,    23,    24,    25,     0,     0,     0,     0,     0,
       0,     0,     0,    26,     0,     0,     0,    27,     0,     0,
       0,    28,    29,     0,     0,     0,    86,    30,     2,     3,
       4,     5,     0,     6,     7,     0,    46,     0,     0,     0,
       0,     9,     0,    10,    11,    12,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,    17,    18,     0,     0,
      19,     0,     0,     0,     0,     0,    22,    23,    24,    25,
       0,     0,     0,     0,     0,     0,     0,     0,    26,     0,
       0,     0,    27,     0,     0,     0,    28,    29,     2,     3,
       4,     5,    30,     6,     7,     0,    46,     0,     0,     0,
       0,     9,     0,    10,    11,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,    22,    23,    24,    25,
       0,     2,     3,     4,     5,     0,     6,     7,    26,    46,
       0,     0,    27,     0,     0,     0,    28,    29,     0,     0,
       0,     0,    30,     0,     0,     0,     0,     0,    47,     0,
      18,     0,     0,   122,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    26,     0,     0,     0,    27,     0,    93,    94,    28,
      29,     0,    95,    96,    97,    90,    98,    99,   100,     0,
       0,     0,     0,   101,   102,   103,   104,   166,     0,     0,
       0,   105,   106,   107,     0,     0,     0,     0,    93,    94,
       0,     0,     0,    95,    96,    97,     0,    98,    99,   100,
       0,     0,     0,   167,   101,   102,   103,   104,     0,     0,
      93,    94,   105,   106,   107,    95,    96,    97,     0,    98,
      99,   100,     0,   131,   132,     0,   101,   102,   103,   104,
       0,     0,    93,    94,   105,   106,   107,    95,    96,    97,
       0,    98,    99,   100,     0,     0,     0,   140,   101,   102,
     103,   104,     0,     0,     0,   133,   105,   106,   107,    93,
      94,     0,     0,     0,    95,    96,    97,     0,    98,    99,
     100,     0,     0,     0,     0,   101,   102,   103,   104,     0,
       0,     0,   133,   105,   106,   107,    93,    94,     0,     0,
       0,    95,    96,    97,     0,    98,    99,   100,     0,     0,
       0,   140,   101,   102,   103,   104,     0,     0,   -59,   -59,
     105,   106,   107,   -59,   -59,   -59,     0,   -59,   -59,   -59,
       0,     0,     0,     0,     0,     0,   -59,   -59,     0,     0,
      50,     0,   -59,   -59,   -59,    93,    94,     0,     0,     0,
      95,    96,    97,     0,    98,    99,   100,     0,     0,     0,
       0,     0,     0,   103,   104,     0,     0,     0,     0,   105,
     106,   107
};

static const yytype_int16 yycheck[] =
{
       0,     8,    31,     0,     0,   178,     4,    88,    42,    90,
      42,   177,    22,    23,    23,    76,    22,    74,    16,    80,
      11,    20,    17,   214,    22,     0,    24,    25,    26,    27,
      28,    29,    30,     0,    68,   226,    68,    46,   204,   205,
      46,    39,   208,    42,   210,    19,    17,    19,   211,    47,
      19,    74,    50,    51,    25,   221,   229,    80,    44,    45,
      53,    54,   225,    34,    74,    58,    59,    60,    19,    62,
      63,    64,    34,    71,    36,    19,   249,    74,    74,    19,
      42,   247,    12,    13,    77,    78,    79,    87,    75,    19,
      88,    89,    90,    75,    17,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     139,    68,    87,    17,   121,    80,    11,    12,    13,    14,
      87,    16,    17,   130,    19,    34,   124,   125,    19,   127,
     128,   129,   139,   131,   132,   216,    41,    80,    41,    19,
      17,    17,   140,    38,    17,    40,    34,   228,    19,    76,
      76,    19,    74,   182,    19,    42,   185,    23,    19,   157,
      80,    39,    22,    46,    17,    28,    61,    41,   166,   167,
      65,    23,    25,    23,    69,    70,    23,    30,    23,    41,
      75,    34,    35,    36,    37,    39,    28,    28,    39,    42,
     188,    87,   225,   200,   171,    62,     0,     1,   201,    -1,
     198,    -1,   194,    51,    -1,    -1,   213,    11,    12,    13,
      14,    -1,    16,    17,    -1,    19,    -1,    -1,   216,    -1,
      24,    -1,    26,    27,    28,    -1,    -1,    31,    32,    33,
     228,    -1,    -1,   240,    38,    39,    40,    -1,    -1,    43,
      -1,    -1,    -1,    47,    48,    49,    50,    51,    52,    -1,
      -1,   251,    -1,    -1,    -1,   186,    -1,    61,   189,   190,
      -1,    65,    -1,    -1,    -1,    69,    70,    -1,   199,    -1,
      74,    75,    -1,    -1,    -1,    -1,   207,    -1,    -1,    53,
      54,    -1,    -1,   214,    58,    59,    60,    -1,    62,    63,
      64,   222,    -1,   224,   225,    69,    70,    71,    72,    -1,
      -1,    -1,   233,    77,    78,    79,    -1,    -1,   239,    -1,
     241,    -1,    -1,    11,    12,    13,    14,    -1,    16,    17,
      -1,    19,    -1,    -1,    -1,   256,    24,   258,    26,    27,
      28,    -1,    -1,    31,    32,    33,    -1,    -1,    -1,    -1,
      38,    39,    40,    -1,    -1,    43,    -1,    -1,    -1,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    69,    70,    11,    12,    13,    14,    75,    16,    17,
      -1,    19,    -1,    -1,    -1,    -1,    24,    -1,    26,    27,
      28,    -1,    -1,    -1,    32,    33,    -1,    -1,    -1,    -1,
      38,    39,    40,    -1,    -1,    43,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    69,    70,    -1,    -1,    -1,    74,    75,    11,    12,
      13,    14,    -1,    16,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    -1,    26,    27,    28,    -1,    -1,    -1,    32,
      33,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,
      43,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    69,    70,    11,    12,
      13,    14,    75,    16,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    -1,    26,    27,    -1,    -1,    -1,    -1,    32,
      33,    -1,    -1,    -1,    -1,    38,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,    52,
      -1,    11,    12,    13,    14,    -1,    16,    17,    61,    19,
      -1,    -1,    65,    -1,    -1,    -1,    69,    70,    -1,    -1,
      -1,    -1,    75,    -1,    -1,    -1,    -1,    -1,    38,    -1,
      40,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    -1,    -1,    -1,    65,    -1,    53,    54,    69,
      70,    -1,    58,    59,    60,    75,    62,    63,    64,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    42,    -1,    -1,
      -1,    77,    78,    79,    -1,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    -1,    62,    63,    64,
      -1,    -1,    -1,    68,    69,    70,    71,    72,    -1,    -1,
      53,    54,    77,    78,    79,    58,    59,    60,    -1,    62,
      63,    64,    -1,    66,    67,    -1,    69,    70,    71,    72,
      -1,    -1,    53,    54,    77,    78,    79,    58,    59,    60,
      -1,    62,    63,    64,    -1,    -1,    -1,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    76,    77,    78,    79,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    -1,    62,    63,
      64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    -1,
      -1,    -1,    76,    77,    78,    79,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    -1,    62,    63,    64,    -1,    -1,
      -1,    68,    69,    70,    71,    72,    -1,    -1,    53,    54,
      77,    78,    79,    58,    59,    60,    -1,    62,    63,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    -1,    -1,
      75,    -1,    77,    78,    79,    53,    54,    -1,    -1,    -1,
      58,    59,    60,    -1,    62,    63,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    72,    -1,    -1,    -1,    -1,    77,
      78,    79
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    11,    12,    13,    14,    16,    17,    19,    24,
      26,    27,    28,    31,    32,    33,    38,    39,    40,    43,
      47,    48,    49,    50,    51,    52,    61,    65,    69,    70,
      75,    82,    84,    85,    86,    87,    88,    95,    96,    97,
     102,   103,   105,   108,   113,    74,    19,    38,    85,   102,
      75,    99,    19,   104,    17,    25,    30,    34,    35,    36,
      37,    42,   106,   107,    19,    17,    19,   102,    44,    45,
      12,    13,    19,    19,    19,    19,   102,    11,   102,   102,
     102,   102,   102,   102,   102,     0,    74,    83,    75,    68,
      75,    94,   102,    53,    54,    58,    59,    60,    62,    63,
      64,    69,    70,    71,    72,    77,    78,    79,   102,   102,
     114,   114,    80,    17,    17,    34,    17,    25,    34,    42,
     107,   109,    29,    19,    42,    68,   102,    42,    68,    41,
     110,    66,    67,    76,    84,    94,   102,    94,   102,    41,
      68,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,    76,    80,    19,    17,
      17,    17,    99,    19,   102,   102,    42,    68,   102,   102,
     102,    99,   102,   102,    76,   100,    76,    83,    99,   102,
     102,    19,   111,   102,   102,   111,   101,    20,    42,   101,
      86,   103,    80,    83,    83,    83,    87,   100,    42,   102,
     100,   100,    19,    80,   112,   112,    23,   102,   100,    23,
      46,    99,    22,    46,    91,    92,    98,    19,   101,   101,
      39,   100,   101,    28,   101,    90,    91,    93,    98,    99,
      92,   100,    94,   101,   100,   100,    93,   100,    94,    86,
      46,    89,    41,   100,    23,    23,    23,    41,   100,    99,
     100,    99,    23,    39,    28,    28,   101,    86,    87,    39,
     100,   100
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
#line 106 "engines/director/lingo/lingo-gr.y"
    { yyerrok; ;}
    break;

  case 5:
#line 109 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1;
	;}
    break;

  case 10:
#line 118 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 12:
#line 122 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 13:
#line 128 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 14:
#line 134 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code2(g_lingo->c_constpush, 0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (4)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (4)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 15:
#line 142 "engines/director/lingo/lingo-gr.y"
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
#line 150 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 17:
#line 156 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code2(g_lingo->c_constpush, 0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (4)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (4)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 18:
#line 164 "engines/director/lingo/lingo-gr.y"
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
#line 173 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 23:
#line 182 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 2] = end; ;}
    break;

  case 24:
#line 193 "engines/director/lingo/lingo-gr.y"
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
#line 209 "engines/director/lingo/lingo-gr.y"
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
#line 221 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_ifcode);
		;}
    break;

  case 27:
#line 226 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 3] = end;	/* end, if cond fails */
		g_lingo->processIf(0, 0); ;}
    break;

  case 28:
#line 233 "engines/director/lingo/lingo-gr.y"
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
#line 242 "engines/director/lingo/lingo-gr.y"
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
#line 251 "engines/director/lingo/lingo-gr.y"
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
#line 261 "engines/director/lingo/lingo-gr.y"
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
#line 271 "engines/director/lingo/lingo-gr.y"
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
#line 282 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 34:
#line 283 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 39:
#line 294 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 41:
#line 303 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (5)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (5)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (5)].code)); ;}
    break;

  case 42:
#line 311 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 43:
#line 312 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 45:
#line 315 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 46:
#line 317 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 47:
#line 323 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 48:
#line 329 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 49:
#line 336 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 50:
#line 338 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 51:
#line 340 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 54:
#line 345 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_constpush);
		inst i = 0;
		WRITE_UINT32(&i, (yyvsp[(1) - (1)].i));
		g_lingo->code1(i); ;}
    break;

  case 55:
#line 350 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_fconstpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 56:
#line 353 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 57:
#line 356 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->_handlers[*(yyvsp[(1) - (1)].s)]->u.func);
		(yyval.code) = g_lingo->code2(g_lingo->c_constpush, 0); // Put dummy value
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 58:
#line 360 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 59:
#line 363 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 60:
#line 367 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code2(g_lingo->c_constpush, 0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 61:
#line 374 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 63:
#line 381 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 64:
#line 382 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 65:
#line 383 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 66:
#line 384 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 67:
#line 385 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 68:
#line 386 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 69:
#line 387 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 70:
#line 388 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 71:
#line 389 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 72:
#line 390 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 73:
#line 391 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 74:
#line 392 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 75:
#line 393 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 76:
#line 394 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 77:
#line 395 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 78:
#line 396 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 79:
#line 397 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 80:
#line 398 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 81:
#line 399 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 82:
#line 400 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 83:
#line 401 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 84:
#line 404 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mci); g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str()); delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 85:
#line 405 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mciwait); g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str()); delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 86:
#line 406 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 88:
#line 408 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_constpush, (inst)0); // Push fake value on stack
							  g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 90:
#line 411 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_alert); ;}
    break;

  case 91:
#line 412 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_constpush);
		inst i = 0;
		WRITE_UINT32(&i, (yyvsp[(2) - (2)].i));
		g_lingo->code1(i);
		g_lingo->code1(g_lingo->c_beep); ;}
    break;

  case 92:
#line 418 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_constpush);
		inst i = 0;
		WRITE_UINT32(&i, 0);
		g_lingo->code1(i);
		g_lingo->code1(g_lingo->c_beep); ;}
    break;

  case 93:
#line 424 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_closeResFile); ;}
    break;

  case 94:
#line 425 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_constpush);
		inst i = 0;
		WRITE_UINT32(&i, 0);
		g_lingo->code1(i);
		g_lingo->code1(g_lingo->c_closeResFile); ;}
    break;

  case 95:
#line 431 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_closeXlib); ;}
    break;

  case 96:
#line 432 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_constpush);
		inst i = 0;
		WRITE_UINT32(&i, 0);
		g_lingo->code1(i);
		g_lingo->code1(g_lingo->c_closeXlib); ;}
    break;

  case 97:
#line 440 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 98:
#line 441 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 99:
#line 452 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 100:
#line 453 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 101:
#line 454 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 102:
#line 455 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_goto);
		g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str());
		g_lingo->codeString("");
		delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 103:
#line 460 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_goto);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s);
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 104:
#line 466 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_goto);
		g_lingo->codeString("");
		g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str());
		delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 105:
#line 473 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(3) - (3)].s); ;}
    break;

  case 106:
#line 474 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); ;}
    break;

  case 107:
#line 475 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); ;}
    break;

  case 108:
#line 476 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); ;}
    break;

  case 109:
#line 479 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(3) - (3)].s); ;}
    break;

  case 110:
#line 480 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); ;}
    break;

  case 111:
#line 481 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(3) - (3)].s); ;}
    break;

  case 112:
#line 509 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 113:
#line 510 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code2(g_lingo->c_constpush, (inst)0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 114:
#line 515 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeFactory(*(yyvsp[(2) - (2)].s));
		;}
    break;

  case 115:
#line 518 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 116:
#line 519 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code2(g_lingo->c_constpush, (inst)0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg), &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 117:
#line 524 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 118:
#line 525 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 119:
#line 526 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 120:
#line 527 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 121:
#line 529 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 122:
#line 533 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (3)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(3) - (3)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 123:
#line 541 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 124:
#line 542 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 125:
#line 543 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2558 "engines/director/lingo/lingo-gr.cpp"
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


#line 546 "engines/director/lingo/lingo-gr.y"


