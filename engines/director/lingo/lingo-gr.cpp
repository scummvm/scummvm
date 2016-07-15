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
     CASTREF = 258,
     UNARY = 259,
     VOID = 260,
     VAR = 261,
     INT = 262,
     THEENTITY = 263,
     THEENTITYWITHID = 264,
     FLOAT = 265,
     BLTIN = 266,
     ID = 267,
     STRING = 268,
     HANDLER = 269,
     tDOWN = 270,
     tELSE = 271,
     tNLELSIF = 272,
     tEND = 273,
     tEXIT = 274,
     tFRAME = 275,
     tGLOBAL = 276,
     tGO = 277,
     tIF = 278,
     tINTO = 279,
     tLOOP = 280,
     tMACRO = 281,
     tMCI = 282,
     tMCIWAIT = 283,
     tMOVIE = 284,
     tNEXT = 285,
     tOF = 286,
     tPREVIOUS = 287,
     tPUT = 288,
     tREPEAT = 289,
     tSET = 290,
     tTHEN = 291,
     tTO = 292,
     tWITH = 293,
     tWHILE = 294,
     tNLELSE = 295,
     tFACTORY = 296,
     tMETHOD = 297,
     tGE = 298,
     tLE = 299,
     tGT = 300,
     tLT = 301,
     tEQ = 302,
     tNEQ = 303,
     tAND = 304,
     tOR = 305,
     tNOT = 306,
     tCONCAT = 307,
     tCONTAINS = 308,
     tSTARTS = 309
   };
#endif
/* Tokens.  */
#define CASTREF 258
#define UNARY 259
#define VOID 260
#define VAR 261
#define INT 262
#define THEENTITY 263
#define THEENTITYWITHID 264
#define FLOAT 265
#define BLTIN 266
#define ID 267
#define STRING 268
#define HANDLER 269
#define tDOWN 270
#define tELSE 271
#define tNLELSIF 272
#define tEND 273
#define tEXIT 274
#define tFRAME 275
#define tGLOBAL 276
#define tGO 277
#define tIF 278
#define tINTO 279
#define tLOOP 280
#define tMACRO 281
#define tMCI 282
#define tMCIWAIT 283
#define tMOVIE 284
#define tNEXT 285
#define tOF 286
#define tPREVIOUS 287
#define tPUT 288
#define tREPEAT 289
#define tSET 290
#define tTHEN 291
#define tTO 292
#define tWITH 293
#define tWHILE 294
#define tNLELSE 295
#define tFACTORY 296
#define tMETHOD 297
#define tGE 298
#define tLE 299
#define tGT 300
#define tLT 301
#define tEQ 302
#define tNEQ 303
#define tAND 304
#define tOR 305
#define tNOT 306
#define tCONCAT 307
#define tCONTAINS 308
#define tSTARTS 309




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
	int	i;
	double f;
	int e[2];	// Entity + field
	int code;
	int	narg;	/* number of arguments */
}
/* Line 193 of yacc.c.  */
#line 233 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 246 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  74
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   626

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  68
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  34
/* YYNRULES -- Number of rules.  */
#define YYNRULES  115
/* YYNRULES -- Number of states.  */
#define YYNSTATES  247

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   309

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      61,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    60,    66,     2,
      62,    63,    58,    56,    67,    57,     2,    59,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      65,    55,    64,     2,     2,     2,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    14,    15,    17,    19,
      21,    23,    25,    30,    35,    40,    46,    51,    56,    62,
      64,    66,    68,    70,    79,    91,   104,   113,   125,   137,
     144,   155,   166,   167,   171,   174,   176,   179,   181,   188,
     190,   196,   198,   202,   206,   209,   213,   215,   217,   218,
     219,   220,   223,   226,   228,   230,   232,   237,   242,   244,
     246,   249,   251,   255,   259,   263,   267,   271,   275,   279,
     283,   287,   291,   295,   298,   302,   306,   310,   314,   317,
     320,   324,   327,   330,   333,   335,   337,   340,   342,   346,
     349,   352,   355,   358,   362,   365,   369,   372,   375,   377,
     381,   384,   388,   389,   398,   401,   402,   411,   412,   414,
     418,   423,   424,   428,   429,   431
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      69,     0,    -1,    69,    70,    71,    -1,    71,    -1,     1,
      61,    -1,    61,    -1,    -1,    95,    -1,    90,    -1,   100,
      -1,    72,    -1,    74,    -1,    33,    89,    24,    12,    -1,
      35,    12,    55,    89,    -1,    35,     8,    55,    89,    -1,
      35,     9,    89,    55,    89,    -1,    35,    12,    37,    89,
      -1,    35,     8,    37,    89,    -1,    35,     9,    89,    37,
      89,    -1,    89,    -1,    90,    -1,    73,    -1,    75,    -1,
      82,    62,    81,    63,    88,    87,    18,    34,    -1,    83,
      55,    89,    87,    37,    89,    87,    88,    87,    18,    34,
      -1,    83,    55,    89,    87,    15,    37,    89,    87,    88,
      87,    18,    34,    -1,    84,    81,    36,    70,    88,    87,
      18,    23,    -1,    84,    81,    36,    70,    88,    87,    40,
      88,    87,    18,    23,    -1,    84,    81,    36,    70,    88,
      87,    86,    77,    87,    18,    23,    -1,    84,    81,    36,
      86,    73,    87,    -1,    84,    81,    36,    86,    73,    87,
      40,    86,    73,    87,    -1,    84,    81,    36,    86,    73,
      87,    78,    87,    76,    87,    -1,    -1,    40,    86,    73,
      -1,    77,    80,    -1,    80,    -1,    78,    79,    -1,    79,
      -1,    85,    81,    36,    86,    74,    87,    -1,    78,    -1,
      85,    81,    36,    88,    87,    -1,    89,    -1,    89,    55,
      89,    -1,    62,    81,    63,    -1,    34,    39,    -1,    34,
      38,    12,    -1,    23,    -1,    17,    -1,    -1,    -1,    -1,
      88,    70,    -1,    88,    74,    -1,     7,    -1,    10,    -1,
      13,    -1,    11,    62,   101,    63,    -1,    12,    62,   101,
      63,    -1,    12,    -1,     8,    -1,     9,    89,    -1,    72,
      -1,    89,    56,    89,    -1,    89,    57,    89,    -1,    89,
      58,    89,    -1,    89,    59,    89,    -1,    89,    64,    89,
      -1,    89,    65,    89,    -1,    89,    48,    89,    -1,    89,
      43,    89,    -1,    89,    44,    89,    -1,    89,    49,    89,
      -1,    89,    50,    89,    -1,    51,    89,    -1,    89,    66,
      89,    -1,    89,    52,    89,    -1,    89,    53,    89,    -1,
      89,    54,    89,    -1,    56,    89,    -1,    57,    89,    -1,
      62,    89,    63,    -1,    27,    13,    -1,    28,    12,    -1,
      33,    89,    -1,    92,    -1,    19,    -1,    21,    91,    -1,
      12,    -1,    91,    67,    12,    -1,    22,    25,    -1,    22,
      30,    -1,    22,    32,    -1,    22,    93,    -1,    22,    93,
      94,    -1,    22,    94,    -1,    37,    20,    13,    -1,    20,
      13,    -1,    37,    13,    -1,    13,    -1,    31,    29,    13,
      -1,    29,    13,    -1,    37,    29,    13,    -1,    -1,    26,
      12,    96,    86,    98,    70,    99,    88,    -1,    41,    12,
      -1,    -1,    42,    12,    97,    86,    98,    70,    99,    88,
      -1,    -1,    12,    -1,    98,    67,    12,    -1,    98,    70,
      67,    12,    -1,    -1,    12,    86,   101,    -1,    -1,    89,
      -1,   101,    67,    89,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   100,   100,   101,   102,   105,   110,   111,   112,   113,
     114,   115,   118,   124,   130,   138,   146,   152,   160,   169,
     170,   172,   173,   178,   189,   205,   219,   226,   235,   244,
     254,   264,   275,   276,   279,   280,   283,   284,   287,   295,
     296,   304,   305,   306,   308,   310,   316,   322,   329,   331,
     333,   334,   335,   338,   343,   346,   349,   355,   363,   366,
     373,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   389,   390,   391,   392,   393,   394,   395,   396,   397,
     398,   401,   402,   403,   404,   405,   407,   410,   411,   422,
     423,   424,   425,   430,   436,   443,   444,   445,   446,   449,
     450,   451,   479,   479,   485,   488,   488,   494,   495,   496,
     497,   499,   503,   511,   512,   513
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CASTREF", "UNARY", "VOID", "VAR", "INT",
  "THEENTITY", "THEENTITYWITHID", "FLOAT", "BLTIN", "ID", "STRING",
  "HANDLER", "tDOWN", "tELSE", "tNLELSIF", "tEND", "tEXIT", "tFRAME",
  "tGLOBAL", "tGO", "tIF", "tINTO", "tLOOP", "tMACRO", "tMCI", "tMCIWAIT",
  "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET",
  "tTHEN", "tTO", "tWITH", "tWHILE", "tNLELSE", "tFACTORY", "tMETHOD",
  "tGE", "tLE", "tGT", "tLT", "tEQ", "tNEQ", "tAND", "tOR", "tNOT",
  "tCONCAT", "tCONTAINS", "tSTARTS", "'='", "'+'", "'-'", "'*'", "'/'",
  "'%'", "'\\n'", "'('", "')'", "'>'", "'<'", "'&'", "','", "$accept",
  "program", "nl", "programline", "asgn", "stmtoneliner", "stmt", "ifstmt",
  "elsestmtoneliner", "elseifstmt", "elseifstmtoneliner",
  "elseifstmtoneliner1", "elseifstmt1", "cond", "repeatwhile",
  "repeatwith", "if", "elseif", "begin", "end", "stmtlist", "expr", "func",
  "globallist", "gotofunc", "gotoframe", "gotomovie", "defn", "@1", "@2",
  "argdef", "argstore", "macro", "arglist", 0
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
     305,   306,   307,   308,   309,    61,    43,    45,    42,    47,
      37,    10,    40,    41,    62,    60,    38,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    68,    69,    69,    69,    70,    71,    71,    71,    71,
      71,    71,    72,    72,    72,    72,    72,    72,    72,    73,
      73,    74,    74,    74,    74,    74,    75,    75,    75,    75,
      75,    75,    76,    76,    77,    77,    78,    78,    79,    80,
      80,    81,    81,    81,    82,    83,    84,    85,    86,    87,
      88,    88,    88,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    90,    90,    90,    90,    90,    90,    91,    91,    92,
      92,    92,    92,    92,    92,    93,    93,    93,    93,    94,
      94,    94,    96,    95,    95,    97,    95,    98,    98,    98,
      98,    99,   100,   101,   101,   101
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     0,     1,     1,     1,
       1,     1,     4,     4,     4,     5,     4,     4,     5,     1,
       1,     1,     1,     8,    11,    12,     8,    11,    11,     6,
      10,    10,     0,     3,     2,     1,     2,     1,     6,     1,
       5,     1,     3,     3,     2,     3,     1,     1,     0,     0,
       0,     2,     2,     1,     1,     1,     4,     4,     1,     1,
       2,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     3,     3,     3,     3,     2,     2,
       3,     2,     2,     2,     1,     1,     2,     1,     3,     2,
       2,     2,     2,     3,     2,     3,     2,     2,     1,     3,
       2,     3,     0,     8,     2,     0,     8,     0,     1,     3,
       4,     0,     3,     0,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    53,    59,     0,    54,     0,    48,    55,    85,
       0,     0,    46,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,    61,    21,    11,
      22,     0,     0,     0,    19,     8,    84,     7,     9,     4,
      58,     0,    61,    60,   113,   113,   113,    87,    86,    98,
       0,    89,     0,    90,     0,    91,     0,    92,    94,   102,
      81,    82,    83,     0,    44,     0,     0,     0,   104,   105,
      73,    78,    79,     0,     1,     5,     6,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   114,     0,
       0,   112,     0,    96,   100,     0,    97,     0,     0,     0,
      93,    48,     0,    45,     0,     0,     0,     0,     0,    48,
      80,     2,     0,    49,     0,     0,    48,     0,    69,    70,
      68,    71,    72,    75,    76,    77,    62,    63,    64,    65,
      66,    67,    74,    56,     0,    57,    88,    99,    95,   101,
     107,    12,    17,    14,     0,     0,    16,    13,   107,    50,
       0,    43,    50,     0,    42,   115,   108,     0,    18,    15,
       0,    49,     0,     0,    49,    49,    20,     0,   111,   111,
      51,    52,     0,     0,    49,    48,    29,   109,     0,    50,
      50,     0,    49,    50,     0,    50,     0,    47,    48,    49,
      37,     0,   110,   103,   106,    23,    50,    49,    26,    49,
      49,    39,    35,     0,     0,    36,    32,     0,    49,     0,
       0,    34,     0,     0,    49,    48,    49,    48,     0,     0,
       0,     0,    48,    30,     0,    31,     0,     0,    24,    27,
      28,    49,    33,    49,    25,    40,    38
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    25,   180,    26,    42,    28,   181,    30,   226,   210,
     211,   200,   212,    80,    31,    32,    33,   201,   236,   160,
     171,    34,   176,    48,    36,    57,    58,    37,   111,   119,
     167,   189,    38,    99
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -178
static const yytype_int16 yypact[] =
{
     199,   -40,  -178,  -178,     2,  -178,   -21,   517,  -178,  -178,
      32,   128,  -178,    38,    42,    49,     2,    43,    98,    58,
      86,     2,     2,     2,     2,     4,  -178,     7,  -178,  -178,
    -178,    47,    56,   236,   498,  -178,  -178,  -178,  -178,  -178,
      52,     2,  -178,   498,     2,     2,     2,  -178,    57,  -178,
     109,  -178,   110,  -178,    96,  -178,    23,    40,  -178,  -178,
    -178,  -178,   388,   114,  -178,   -17,     2,    -7,  -178,  -178,
     560,   560,   560,   455,  -178,  -178,   255,   236,     2,   236,
      91,   479,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   388,   498,    45,
      54,    62,   119,  -178,  -178,   121,  -178,   122,   125,   103,
    -178,  -178,   127,  -178,     2,     2,   412,     2,     2,  -178,
    -178,  -178,    77,   498,    80,   431,    88,     2,   498,   498,
     498,   498,   498,   498,   498,   498,   541,   541,   560,   560,
     498,   498,   498,  -178,     2,  -178,  -178,  -178,  -178,  -178,
     138,  -178,   498,   498,     2,     2,   498,   498,   138,  -178,
       3,  -178,  -178,   373,   498,   498,  -178,     5,   498,   498,
       5,   311,   115,     2,   311,  -178,  -178,   139,    95,    95,
    -178,  -178,   145,     2,   498,     9,    -1,  -178,   152,  -178,
    -178,   132,   498,  -178,   144,  -178,   151,  -178,  -178,   151,
    -178,   236,  -178,   311,   311,  -178,  -178,   311,  -178,   311,
     151,   151,  -178,   236,   373,  -178,   130,   137,   311,   157,
     158,  -178,   159,   142,  -178,  -178,  -178,  -178,   163,   148,
     160,   162,    14,  -178,   373,  -178,   342,   153,  -178,  -178,
    -178,   311,  -178,  -178,  -178,  -178,  -178
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -178,  -178,   -23,   113,    26,  -160,     0,  -178,  -178,  -178,
       8,  -148,   -20,   -71,  -178,  -178,  -178,  -177,    -6,   -38,
    -133,     1,    28,  -178,  -178,  -178,   134,  -178,  -178,  -178,
      35,    16,  -178,    55
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -59
static const yytype_int16 yytable[] =
{
      29,    46,    76,   175,    74,    43,   122,   -10,   124,     2,
       3,     4,     5,     6,    40,     8,   197,    62,   172,   213,
     114,    39,    70,    71,    72,    73,    27,   194,    35,   174,
     117,   -50,   -50,   213,    81,    41,   106,    18,   115,   198,
     173,    44,    97,   107,    47,    98,    98,    98,   118,   195,
      59,   215,   108,    21,   224,    60,   203,   204,    22,    23,
     207,    61,   209,   215,    24,    75,    75,   116,   -10,    52,
      68,    54,   177,   218,   242,   -50,    29,   109,    81,   123,
     125,    63,    64,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,    69,   241,
     100,   101,    27,   162,    35,   150,    65,    66,   143,    77,
      67,    78,   144,   158,    45,   152,   153,   145,   156,   157,
     163,   144,   103,   104,   102,   105,   113,   126,   164,   144,
     217,   146,   108,   182,   147,   148,   185,   186,   149,   151,
     159,    49,   223,   161,   178,   165,   193,   179,    50,    75,
     166,   187,   183,    51,   206,   168,   169,    52,    53,    54,
      55,   216,   188,   191,   202,    56,   205,   208,   197,   219,
     225,   220,   222,   227,   184,   229,   230,   231,   232,   196,
     228,   237,   238,   239,   192,   240,   233,   244,   235,   121,
     221,   110,   214,   170,   199,   190,     0,     0,     0,    -6,
       1,     0,    81,   245,     0,   246,     2,     3,     4,     5,
       6,     7,     8,     0,    81,     0,     0,     0,     9,   234,
      10,    11,    12,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,    17,    18,     0,   243,     0,     0,     0,
      19,    20,     0,     2,     3,     4,     5,     6,    40,     8,
      21,     0,     0,     0,     0,    22,    23,     0,     0,     0,
      -6,    24,     2,     3,     4,     5,     6,     7,     8,    41,
       0,    18,     0,     0,     9,     0,    10,    11,    12,     0,
       0,    13,    14,    15,     0,     0,     0,    21,    16,    17,
      18,     0,    22,    23,     0,     0,    19,    20,    79,     0,
       0,     0,     0,     0,     0,     0,    21,     0,     0,     0,
       0,    22,    23,     0,     0,     0,     0,    24,     2,     3,
       4,     5,     6,    40,     8,     0,     0,     0,     0,     0,
       9,     0,    10,    11,    12,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,    17,    18,     0,     0,     2,
       3,     4,     5,     6,    40,     8,     0,     0,     0,     0,
       0,     9,    21,    10,    11,    12,     0,    22,    23,    14,
      15,     0,    75,    24,     0,    16,    17,    18,     0,     0,
       2,     3,     4,     5,     6,    40,     8,     0,     0,     0,
       0,     0,     9,    21,    10,    11,     0,     0,    22,    23,
      14,    15,     0,     0,    24,     0,    16,     0,    18,     0,
       0,     0,   112,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,    22,
      23,    82,    83,     0,     0,    24,    84,    85,    86,     0,
      87,    88,    89,     0,    90,    91,    92,    93,     0,   154,
       0,     0,    94,    95,    96,    82,    83,     0,     0,     0,
      84,    85,    86,     0,    87,    88,    89,   155,    90,    91,
      92,    93,     0,     0,    82,    83,    94,    95,    96,    84,
      85,    86,     0,    87,    88,    89,   127,    90,    91,    92,
      93,     0,     0,     0,   120,    94,    95,    96,    82,    83,
       0,     0,     0,    84,    85,    86,     0,    87,    88,    89,
       0,    90,    91,    92,    93,     0,     0,     0,   120,    94,
      95,    96,    82,    83,     0,     0,     0,    84,    85,    86,
       0,    87,    88,    89,   127,    90,    91,    92,    93,     0,
       0,    82,    83,    94,    95,    96,    84,    85,    86,     0,
      87,    88,    89,     0,    90,    91,    92,    93,     0,     0,
     -58,   -58,    94,    95,    96,   -58,   -58,   -58,     0,   -58,
     -58,   -58,     0,     0,     0,   -58,   -58,     0,     0,    45,
       0,   -58,   -58,   -58,    82,    83,     0,     0,     0,    84,
      85,    86,     0,    87,    88,    89,     0,     0,     0,    92,
      93,     0,     0,    82,    83,    94,    95,    96,    84,    85,
      86,     0,    87,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    94,    95,    96
};

static const yytype_int16 yycheck[] =
{
       0,     7,    25,   163,     0,     4,    77,     0,    79,     7,
       8,     9,    10,    11,    12,    13,    17,    16,    15,   196,
      37,    61,    21,    22,    23,    24,     0,    18,     0,   162,
      37,    17,    18,   210,    33,    33,    13,    35,    55,    40,
      37,    62,    41,    20,    12,    44,    45,    46,    55,    40,
      12,   199,    29,    51,   214,    13,   189,   190,    56,    57,
     193,    12,   195,   211,    62,    61,    61,    66,    61,    29,
      12,    31,    67,   206,   234,    61,    76,    37,    77,    78,
      79,    38,    39,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    12,   232,
      45,    46,    76,   126,    76,   111,     8,     9,    63,    62,
      12,    55,    67,   119,    62,   114,   115,    63,   117,   118,
     126,    67,    13,    13,    67,    29,    12,    36,   127,    67,
     201,    12,    29,   171,    13,    13,   174,   175,    13,    12,
      63,    13,   213,    63,   167,   144,   184,   170,    20,    61,
      12,    12,    37,    25,   192,   154,   155,    29,    30,    31,
      32,   199,    67,    18,    12,    37,    34,    23,    17,   207,
      40,   209,   210,    36,   173,    18,    18,    18,    36,   185,
     218,    18,    34,    23,   183,    23,   224,    34,   226,    76,
     210,    57,   198,   158,   186,   179,    -1,    -1,    -1,     0,
       1,    -1,   201,   241,    -1,   243,     7,     8,     9,    10,
      11,    12,    13,    -1,   213,    -1,    -1,    -1,    19,   225,
      21,    22,    23,    -1,    -1,    26,    27,    28,    -1,    -1,
      -1,    -1,    33,    34,    35,    -1,   236,    -1,    -1,    -1,
      41,    42,    -1,     7,     8,     9,    10,    11,    12,    13,
      51,    -1,    -1,    -1,    -1,    56,    57,    -1,    -1,    -1,
      61,    62,     7,     8,     9,    10,    11,    12,    13,    33,
      -1,    35,    -1,    -1,    19,    -1,    21,    22,    23,    -1,
      -1,    26,    27,    28,    -1,    -1,    -1,    51,    33,    34,
      35,    -1,    56,    57,    -1,    -1,    41,    42,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    56,    57,    -1,    -1,    -1,    -1,    62,     7,     8,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      19,    -1,    21,    22,    23,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    33,    34,    35,    -1,    -1,     7,
       8,     9,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    19,    51,    21,    22,    23,    -1,    56,    57,    27,
      28,    -1,    61,    62,    -1,    33,    34,    35,    -1,    -1,
       7,     8,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    19,    51,    21,    22,    -1,    -1,    56,    57,
      27,    28,    -1,    -1,    62,    -1,    33,    -1,    35,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    56,
      57,    43,    44,    -1,    -1,    62,    48,    49,    50,    -1,
      52,    53,    54,    -1,    56,    57,    58,    59,    -1,    37,
      -1,    -1,    64,    65,    66,    43,    44,    -1,    -1,    -1,
      48,    49,    50,    -1,    52,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,    43,    44,    64,    65,    66,    48,
      49,    50,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    -1,    -1,    -1,    63,    64,    65,    66,    43,    44,
      -1,    -1,    -1,    48,    49,    50,    -1,    52,    53,    54,
      -1,    56,    57,    58,    59,    -1,    -1,    -1,    63,    64,
      65,    66,    43,    44,    -1,    -1,    -1,    48,    49,    50,
      -1,    52,    53,    54,    55,    56,    57,    58,    59,    -1,
      -1,    43,    44,    64,    65,    66,    48,    49,    50,    -1,
      52,    53,    54,    -1,    56,    57,    58,    59,    -1,    -1,
      43,    44,    64,    65,    66,    48,    49,    50,    -1,    52,
      53,    54,    -1,    -1,    -1,    58,    59,    -1,    -1,    62,
      -1,    64,    65,    66,    43,    44,    -1,    -1,    -1,    48,
      49,    50,    -1,    52,    53,    54,    -1,    -1,    -1,    58,
      59,    -1,    -1,    43,    44,    64,    65,    66,    48,    49,
      50,    -1,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    65,    66
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     7,     8,     9,    10,    11,    12,    13,    19,
      21,    22,    23,    26,    27,    28,    33,    34,    35,    41,
      42,    51,    56,    57,    62,    69,    71,    72,    73,    74,
      75,    82,    83,    84,    89,    90,    92,    95,   100,    61,
      12,    33,    72,    89,    62,    62,    86,    12,    91,    13,
      20,    25,    29,    30,    31,    32,    37,    93,    94,    12,
      13,    12,    89,    38,    39,     8,     9,    12,    12,    12,
      89,    89,    89,    89,     0,    61,    70,    62,    55,    62,
      81,    89,    43,    44,    48,    49,    50,    52,    53,    54,
      56,    57,    58,    59,    64,    65,    66,    89,    89,   101,
     101,   101,    67,    13,    13,    29,    13,    20,    29,    37,
      94,    96,    24,    12,    37,    55,    89,    37,    55,    97,
      63,    71,    81,    89,    81,    89,    36,    55,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    63,    67,    63,    12,    13,    13,    13,
      86,    12,    89,    89,    37,    55,    89,    89,    86,    63,
      87,    63,    70,    86,    89,    89,    12,    98,    89,    89,
      98,    88,    15,    37,    88,    73,    90,    67,    70,    70,
      70,    74,    87,    37,    89,    87,    87,    12,    67,    99,
      99,    18,    89,    87,    18,    40,    86,    17,    40,    78,
      79,    85,    12,    88,    88,    34,    87,    88,    23,    88,
      77,    78,    80,    85,    86,    79,    87,    81,    88,    87,
      87,    80,    87,    81,    73,    40,    76,    36,    87,    18,
      18,    18,    36,    87,    86,    87,    86,    18,    34,    23,
      23,    88,    73,    74,    34,    87,    87
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
#line 102 "engines/director/lingo/lingo-gr.y"
    { yyerrok; ;}
    break;

  case 5:
#line 105 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1;
	;}
    break;

  case 10:
#line 114 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 12:
#line 118 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 13:
#line 124 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 14:
#line 130 "engines/director/lingo/lingo-gr.y"
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
#line 138 "engines/director/lingo/lingo-gr.y"
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
#line 146 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 17:
#line 152 "engines/director/lingo/lingo-gr.y"
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
#line 160 "engines/director/lingo/lingo-gr.y"
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
#line 169 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 23:
#line 178 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 2] = end; ;}
    break;

  case 24:
#line 189 "engines/director/lingo/lingo-gr.y"
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
#line 205 "engines/director/lingo/lingo-gr.y"
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
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 3] = end;	/* end, if cond fails */
		g_lingo->processIf(0, 0); ;}
    break;

  case 27:
#line 226 "engines/director/lingo/lingo-gr.y"
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

  case 28:
#line 235 "engines/director/lingo/lingo-gr.y"
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

  case 29:
#line 244 "engines/director/lingo/lingo-gr.y"
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

  case 30:
#line 254 "engines/director/lingo/lingo-gr.y"
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

  case 31:
#line 264 "engines/director/lingo/lingo-gr.y"
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

  case 32:
#line 275 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 33:
#line 276 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 38:
#line 287 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 40:
#line 296 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (5)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (5)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (5)].code)); ;}
    break;

  case 41:
#line 304 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 42:
#line 305 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 44:
#line 308 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 45:
#line 310 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 46:
#line 316 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 47:
#line 322 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 48:
#line 329 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 49:
#line 331 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 50:
#line 333 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 53:
#line 338 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_constpush);
		inst i = 0;
		WRITE_UINT32(&i, (yyvsp[(1) - (1)].i));
		g_lingo->code1(i); ;}
    break;

  case 54:
#line 343 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_fconstpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 55:
#line 346 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 56:
#line 349 "engines/director/lingo/lingo-gr.y"
    {
		if ((yyvsp[(3) - (4)].narg) != g_lingo->_builtins[*(yyvsp[(1) - (4)].s)]->nargs)
			error("Built-in function %s expects %d arguments but got %d", (yyvsp[(1) - (4)].s)->c_str(), g_lingo->_builtins[*(yyvsp[(1) - (4)].s)]->nargs, (yyvsp[(3) - (4)].narg));

		(yyval.code) = g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (4)].s)]->func);
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 57:
#line 355 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (4)].s)->c_str());

		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(3) - (4)].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 58:
#line 363 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeId(*(yyvsp[(1) - (1)].s));
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 59:
#line 366 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code2(g_lingo->c_constpush, 0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 60:
#line 373 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 62:
#line 380 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 63:
#line 381 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 64:
#line 382 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 65:
#line 383 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 66:
#line 384 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 67:
#line 385 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 68:
#line 386 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 69:
#line 387 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 70:
#line 388 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 71:
#line 389 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 72:
#line 390 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 73:
#line 391 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 74:
#line 392 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 75:
#line 393 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 76:
#line 394 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 77:
#line 395 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 78:
#line 396 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 79:
#line 397 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 80:
#line 398 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 81:
#line 401 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mci); g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str()); delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 82:
#line 402 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mciwait); g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str()); delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 83:
#line 403 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 85:
#line 405 "engines/director/lingo/lingo-gr.y"
    { 	g_lingo->code2(g_lingo->c_constpush, (inst)0); // Push fake value on stack
								g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 87:
#line 410 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 88:
#line 411 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 89:
#line 422 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 90:
#line 423 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 91:
#line 424 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 92:
#line 425 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_goto);
		g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str());
		g_lingo->codeString("");
		delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 93:
#line 430 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_goto);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s);
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 94:
#line 436 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_goto);
		g_lingo->codeString("");
		g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str());
		delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 95:
#line 443 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(3) - (3)].s); ;}
    break;

  case 96:
#line 444 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); ;}
    break;

  case 97:
#line 445 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); ;}
    break;

  case 98:
#line 446 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); ;}
    break;

  case 99:
#line 449 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(3) - (3)].s); ;}
    break;

  case 100:
#line 450 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); ;}
    break;

  case 101:
#line 451 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(3) - (3)].s); ;}
    break;

  case 102:
#line 479 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 103:
#line 480 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code2(g_lingo->c_constpush, (inst)0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 104:
#line 485 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeFactory(*(yyvsp[(2) - (2)].s));
		;}
    break;

  case 105:
#line 488 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 106:
#line 489 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code2(g_lingo->c_constpush, (inst)0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg), &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 107:
#line 494 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 108:
#line 495 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 109:
#line 496 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 110:
#line 497 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 111:
#line 499 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 112:
#line 503 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (3)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(3) - (3)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 113:
#line 511 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 114:
#line 512 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 115:
#line 513 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2410 "engines/director/lingo/lingo-gr.cpp"
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


#line 516 "engines/director/lingo/lingo-gr.y"


