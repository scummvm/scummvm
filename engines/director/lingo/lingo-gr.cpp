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
     FLOAT = 263,
     BLTIN = 264,
     ID = 265,
     STRING = 266,
     HANDLER = 267,
     tDOWN = 268,
     tELSE = 269,
     tNLELSIF = 270,
     tEND = 271,
     tEXIT = 272,
     tFRAME = 273,
     tGLOBAL = 274,
     tGO = 275,
     tIF = 276,
     tINTO = 277,
     tLOOP = 278,
     tMACRO = 279,
     tMCI = 280,
     tMCIWAIT = 281,
     tMOVIE = 282,
     tNEXT = 283,
     tOF = 284,
     tPREVIOUS = 285,
     tPUT = 286,
     tREPEAT = 287,
     tSET = 288,
     tTHEN = 289,
     tTO = 290,
     tWITH = 291,
     tWHILE = 292,
     tNLELSE = 293,
     tGE = 294,
     tLE = 295,
     tGT = 296,
     tLT = 297,
     tEQ = 298,
     tNEQ = 299
   };
#endif
/* Tokens.  */
#define CASTREF 258
#define UNARY 259
#define VOID 260
#define VAR 261
#define INT 262
#define FLOAT 263
#define BLTIN 264
#define ID 265
#define STRING 266
#define HANDLER 267
#define tDOWN 268
#define tELSE 269
#define tNLELSIF 270
#define tEND 271
#define tEXIT 272
#define tFRAME 273
#define tGLOBAL 274
#define tGO 275
#define tIF 276
#define tINTO 277
#define tLOOP 278
#define tMACRO 279
#define tMCI 280
#define tMCIWAIT 281
#define tMOVIE 282
#define tNEXT 283
#define tOF 284
#define tPREVIOUS 285
#define tPUT 286
#define tREPEAT 287
#define tSET 288
#define tTHEN 289
#define tTO 290
#define tWITH 291
#define tWHILE 292
#define tNLELSE 293
#define tGE 294
#define tLE 295
#define tGT 296
#define tLT 297
#define tEQ 298
#define tNEQ 299




/* Copy the first part of user declarations.  */
#line 49 "engines/director/lingo/lingo-gr.y"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/hash-str.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"

extern int yylex();
extern int yyparse();
void yyerror(char *s) { error("%s at line %d col %d", s, Director::g_lingo->_linenumber, Director::g_lingo->_colnumber); }

using namespace Director;



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
#line 65 "engines/director/lingo/lingo-gr.y"
{
	Common::String *s;
	int	i;
	float f;
	int code;
	int	narg;	/* number of arguments */
}
/* Line 193 of yacc.c.  */
#line 208 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 221 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  61
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   384

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  57
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  32
/* YYNRULES -- Number of rules.  */
#define YYNRULES  96
/* YYNRULES -- Number of states.  */
#define YYNSTATES  205

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   299

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      51,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    50,     2,     2,
      52,    53,    48,    46,    56,    47,     2,    49,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      55,    45,    54,     2,     2,     2,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    11,    12,    14,    16,    18,
      20,    22,    24,    29,    34,    39,    41,    43,    45,    47,
      56,    68,    81,    90,   102,   114,   121,   132,   143,   144,
     148,   151,   153,   160,   162,   168,   170,   174,   178,   181,
     185,   187,   189,   190,   191,   192,   195,   198,   200,   202,
     207,   212,   214,   216,   220,   224,   228,   232,   236,   240,
     244,   248,   252,   255,   258,   262,   265,   268,   271,   273,
     275,   278,   280,   284,   287,   290,   293,   296,   300,   303,
     307,   310,   313,   315,   319,   322,   326,   327,   336,   337,
     339,   343,   348,   349,   353,   354,   356
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      58,     0,    -1,    58,    59,    60,    -1,    60,    -1,    51,
      -1,    -1,    83,    -1,    78,    -1,    87,    -1,    61,    -1,
      63,    -1,     1,    -1,    31,    77,    22,    10,    -1,    33,
      10,    45,    77,    -1,    33,    10,    35,    77,    -1,    77,
      -1,    78,    -1,    62,    -1,    64,    -1,    70,    52,    69,
      53,    76,    75,    16,    32,    -1,    71,    45,    77,    75,
      35,    77,    75,    76,    75,    16,    32,    -1,    71,    45,
      77,    75,    13,    35,    77,    75,    76,    75,    16,    32,
      -1,    72,    69,    34,    59,    76,    75,    16,    21,    -1,
      72,    69,    34,    59,    76,    75,    14,    76,    75,    16,
      21,    -1,    72,    69,    34,    59,    76,    75,    74,    66,
      75,    16,    21,    -1,    72,    69,    34,    74,    62,    75,
      -1,    72,    69,    34,    74,    62,    75,    38,    74,    62,
      75,    -1,    72,    69,    34,    74,    62,    75,    67,    75,
      65,    75,    -1,    -1,    38,    74,    62,    -1,    66,    68,
      -1,    68,    -1,    73,    69,    34,    74,    63,    75,    -1,
      67,    -1,    73,    69,    34,    76,    75,    -1,    77,    -1,
      77,    45,    77,    -1,    52,    69,    53,    -1,    32,    37,
      -1,    32,    36,    10,    -1,    21,    -1,    15,    -1,    -1,
      -1,    -1,    76,    59,    -1,    76,    63,    -1,     7,    -1,
       8,    -1,     9,    52,    88,    53,    -1,    10,    52,    88,
      53,    -1,    10,    -1,    61,    -1,    77,    46,    77,    -1,
      77,    47,    77,    -1,    77,    48,    77,    -1,    77,    49,
      77,    -1,    77,    54,    77,    -1,    77,    55,    77,    -1,
      77,    44,    77,    -1,    77,    39,    77,    -1,    77,    40,
      77,    -1,    46,    77,    -1,    47,    77,    -1,    52,    77,
      53,    -1,    25,    11,    -1,    26,    10,    -1,    31,    77,
      -1,    80,    -1,    17,    -1,    19,    79,    -1,    10,    -1,
      79,    56,    10,    -1,    20,    23,    -1,    20,    28,    -1,
      20,    30,    -1,    20,    81,    -1,    20,    81,    82,    -1,
      20,    82,    -1,    35,    18,    11,    -1,    18,    11,    -1,
      35,    11,    -1,    11,    -1,    29,    27,    11,    -1,    27,
      11,    -1,    35,    27,    11,    -1,    -1,    24,    10,    84,
      74,    85,    59,    86,    76,    -1,    -1,    10,    -1,    85,
      56,    10,    -1,    85,    59,    56,    10,    -1,    -1,    10,
      74,    88,    -1,    -1,    77,    -1,    88,    56,    77,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    93,    93,    94,    97,   102,   103,   104,   105,   106,
     107,   108,   111,   117,   123,   130,   131,   133,   134,   139,
     150,   166,   180,   187,   196,   205,   215,   225,   236,   237,
     240,   241,   244,   251,   252,   260,   261,   262,   264,   266,
     272,   278,   285,   287,   289,   290,   291,   294,   299,   302,
     308,   316,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   334,   335,   336,   337,   338,
     340,   343,   344,   355,   356,   357,   358,   363,   369,   376,
     377,   378,   379,   382,   383,   384,   412,   412,   419,   420,
     421,   422,   424,   427,   435,   436,   437
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CASTREF", "UNARY", "VOID", "VAR", "INT",
  "FLOAT", "BLTIN", "ID", "STRING", "HANDLER", "tDOWN", "tELSE",
  "tNLELSIF", "tEND", "tEXIT", "tFRAME", "tGLOBAL", "tGO", "tIF", "tINTO",
  "tLOOP", "tMACRO", "tMCI", "tMCIWAIT", "tMOVIE", "tNEXT", "tOF",
  "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO", "tWITH",
  "tWHILE", "tNLELSE", "tGE", "tLE", "tGT", "tLT", "tEQ", "tNEQ", "'='",
  "'+'", "'-'", "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "'>'", "'<'",
  "','", "$accept", "program", "nl", "programline", "asgn", "stmtoneliner",
  "stmt", "ifstmt", "elsestmtoneliner", "elseifstmt", "elseifstmtoneliner",
  "elseifstmt1", "cond", "repeatwhile", "repeatwith", "if", "elseif",
  "begin", "end", "stmtlist", "expr", "func", "globallist", "gotofunc",
  "gotoframe", "gotomovie", "defn", "@1", "argdef", "argstore", "macro",
  "arglist", 0
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
     295,   296,   297,   298,   299,    61,    43,    45,    42,    47,
      37,    10,    40,    41,    62,    60,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    57,    58,    58,    59,    60,    60,    60,    60,    60,
      60,    60,    61,    61,    61,    62,    62,    63,    63,    63,
      63,    63,    64,    64,    64,    64,    64,    64,    65,    65,
      66,    66,    67,    68,    68,    69,    69,    69,    70,    71,
      72,    73,    74,    75,    76,    76,    76,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    78,    78,    78,    78,    78,
      78,    79,    79,    80,    80,    80,    80,    80,    80,    81,
      81,    81,    81,    82,    82,    82,    84,    83,    85,    85,
      85,    85,    86,    87,    88,    88,    88
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     1,     0,     1,     1,     1,     1,
       1,     1,     4,     4,     4,     1,     1,     1,     1,     8,
      11,    12,     8,    11,    11,     6,    10,    10,     0,     3,
       2,     1,     6,     1,     5,     1,     3,     3,     2,     3,
       1,     1,     0,     0,     0,     2,     2,     1,     1,     4,
       4,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     3,     2,     2,     2,     1,     1,
       2,     1,     3,     2,     2,     2,     2,     3,     2,     3,
       2,     2,     1,     3,     2,     3,     0,     8,     0,     1,
       3,     4,     0,     3,     0,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    11,    47,    48,     0,    42,    69,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       3,    52,    17,    10,    18,     0,     0,     0,    15,     7,
      68,     6,     8,    94,    94,    94,    71,    70,    82,     0,
      73,     0,    74,     0,    75,     0,    76,    78,    86,    65,
      66,    51,     0,    52,    67,     0,    38,     0,    62,    63,
       0,     1,     4,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    95,     0,
       0,    93,     0,    80,    84,     0,    81,     0,     0,     0,
      77,    42,     0,     0,    39,     0,     0,    64,     2,     0,
      43,     0,     0,    42,     0,    60,    61,    59,    53,    54,
      55,    56,    57,    58,    49,     0,    50,    72,    83,    79,
      85,    88,    12,    14,    13,    44,     0,    37,    44,     0,
      36,    96,    89,     0,    43,     0,     0,    43,    43,    16,
       0,    92,    45,    46,     0,     0,    43,    42,    25,    90,
       0,    44,     0,    43,    44,    44,     0,     0,    41,    42,
      43,     0,    91,    87,    19,    44,    43,    43,    22,    43,
      33,    31,     0,     0,    28,     0,    43,     0,     0,    30,
       0,     0,    43,    42,    43,    42,     0,     0,     0,     0,
      42,    26,     0,    27,     0,     0,    20,    23,    24,    43,
      29,    43,    21,    34,    32
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    19,   142,    20,    53,    22,   143,    24,   184,   169,
     170,   171,    67,    25,    26,    27,   172,   194,   126,   134,
      28,   139,    37,    30,    46,    47,    31,    91,   133,   151,
      32,    79
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -124
static const yytype_int16 yypact[] =
{
     144,  -124,  -124,  -124,   -40,   329,  -124,    23,   288,  -124,
      28,    29,    37,   173,     6,    42,   173,   173,   173,     7,
    -124,     9,  -124,  -124,  -124,     5,    17,   219,   317,  -124,
    -124,  -124,  -124,   173,   173,   173,  -124,     8,  -124,    50,
    -124,    57,  -124,    53,  -124,    12,    -3,  -124,  -124,  -124,
    -124,    38,   173,  -124,    87,    71,  -124,   -14,   148,   148,
     293,  -124,  -124,   144,   219,   173,   219,    55,   305,   173,
     173,   173,   173,   173,   173,   173,   173,   173,   317,   -12,
      -2,    35,    82,  -124,  -124,    84,  -124,    91,    92,    66,
    -124,  -124,    87,    88,  -124,   173,   173,  -124,  -124,    51,
     317,    54,   281,    59,   173,   317,   317,   317,   241,   241,
     148,   148,   317,   317,  -124,   173,  -124,  -124,  -124,  -124,
    -124,    96,  -124,   317,   317,  -124,     2,  -124,  -124,   267,
     317,   317,  -124,   -43,   190,    77,   173,   190,  -124,  -124,
     103,    63,  -124,  -124,   104,   173,   317,    13,   -13,  -124,
     111,  -124,    90,   317,  -124,  -124,   102,   110,  -124,  -124,
    -124,   219,  -124,   190,  -124,  -124,   190,   190,  -124,   110,
    -124,  -124,   219,   267,   100,    94,   190,   113,   123,  -124,
     124,   114,  -124,  -124,  -124,  -124,   134,   125,   135,   137,
      -5,  -124,   267,  -124,   236,   127,  -124,  -124,  -124,   190,
    -124,  -124,  -124,  -124,  -124
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -124,  -124,   -15,    97,    16,  -123,     0,  -124,  -124,  -124,
      18,     3,   -61,  -124,  -124,  -124,    19,    -4,   -52,  -106,
       1,    20,  -124,  -124,  -124,   128,  -124,  -124,  -124,  -124,
    -124,    21
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -52
static const yytype_int16 yytable[] =
{
      23,    35,   158,    99,    63,   101,   138,    61,    62,    -9,
     -44,   -44,    33,   140,    54,   135,    21,    58,    59,    60,
      29,    95,   137,    86,    41,   159,    43,   155,    68,   156,
      87,    96,    89,    36,    78,    78,    78,   136,    48,    88,
      49,   114,    55,    56,   115,   163,   -44,    50,   166,   167,
     182,   116,    57,    92,   115,    80,    81,    64,    62,   176,
      -9,    83,    65,    23,    82,    68,   100,   102,    84,   200,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    21,
      85,    94,   144,    29,   199,   147,   148,   121,   128,   103,
      34,   115,   117,    88,   154,   118,   123,   124,   122,   129,
     175,   165,   119,   120,   125,   130,   132,   127,   174,    93,
      62,   181,   145,   149,   177,   178,   131,   180,   141,   150,
     152,   162,   164,   168,   186,   158,    69,    70,   185,   187,
     191,    71,   193,    72,    73,    74,    75,   146,   183,   188,
     189,    76,    77,   157,    -5,     1,   153,   203,   190,   204,
     195,     2,     3,     4,     5,   173,   197,   196,   198,   202,
      98,     6,    68,     7,     8,     9,   160,   161,    10,    11,
      12,     0,   179,    68,    90,    13,    14,    15,     0,   192,
       2,     3,     4,    51,     0,     0,     0,    69,    70,     0,
      16,    17,    71,     0,   201,    -5,    18,     2,     3,     4,
      51,     0,    76,    77,    52,     0,    15,     6,     0,     7,
       8,     9,     0,     0,     0,    11,    12,     0,     0,    16,
      17,    13,    14,    15,     0,    18,     2,     3,     4,    51,
       0,     0,     0,     0,     0,     0,    16,    17,     0,     0,
       0,    62,    18,     2,     3,     4,    51,     0,     0,     0,
      52,     0,    15,     6,     0,     7,     8,     9,     0,     0,
       0,    11,    12,     0,     0,    16,    17,    13,    14,    15,
       0,    66,     0,     0,     2,     3,     4,    51,     0,     0,
      69,    70,    16,    17,     6,    71,     7,     8,    18,    74,
      75,     0,    11,    12,     0,    76,    77,     0,    13,    38,
      15,     0,     0,     0,     0,     0,    39,     0,     0,     0,
       0,    40,     0,    16,    17,    41,    42,    43,    44,    18,
      69,    70,     0,    45,     0,    71,   104,    72,    73,    74,
      75,     0,    69,    70,    97,    76,    77,    71,     0,    72,
      73,    74,    75,     0,    69,    70,    97,    76,    77,    71,
     104,    72,    73,    74,    75,     0,    69,    70,     0,    76,
      77,    71,     0,    72,    73,    74,    75,     0,   -51,   -51,
       0,    76,    77,   -51,     0,     0,     0,   -51,   -51,     0,
       0,    34,     0,   -51,   -51
};

static const yytype_int16 yycheck[] =
{
       0,     5,    15,    64,    19,    66,   129,     0,    51,     0,
      15,    16,    52,    56,    13,    13,     0,    16,    17,    18,
       0,    35,   128,    11,    27,    38,    29,    14,    27,    16,
      18,    45,    35,    10,    33,    34,    35,    35,    10,    27,
      11,    53,    36,    37,    56,   151,    51,    10,   154,   155,
     173,    53,    10,    52,    56,    34,    35,    52,    51,   165,
      51,    11,    45,    63,    56,    64,    65,    66,    11,   192,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    63,
      27,    10,   134,    63,   190,   137,   138,    91,   103,    34,
      52,    56,    10,    27,   146,    11,    95,    96,    10,   103,
     161,   153,    11,    11,    53,   104,    10,    53,   160,    22,
      51,   172,    35,    10,   166,   167,   115,   169,   133,    56,
      16,    10,    32,    21,   176,    15,    39,    40,    34,    16,
     182,    44,   184,    46,    47,    48,    49,   136,    38,    16,
      16,    54,    55,   147,     0,     1,   145,   199,    34,   201,
      16,     7,     8,     9,    10,   159,    21,    32,    21,    32,
      63,    17,   161,    19,    20,    21,   148,   148,    24,    25,
      26,    -1,   169,   172,    46,    31,    32,    33,    -1,   183,
       7,     8,     9,    10,    -1,    -1,    -1,    39,    40,    -1,
      46,    47,    44,    -1,   194,    51,    52,     7,     8,     9,
      10,    -1,    54,    55,    31,    -1,    33,    17,    -1,    19,
      20,    21,    -1,    -1,    -1,    25,    26,    -1,    -1,    46,
      47,    31,    32,    33,    -1,    52,     7,     8,     9,    10,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    51,    52,     7,     8,     9,    10,    -1,    -1,    -1,
      31,    -1,    33,    17,    -1,    19,    20,    21,    -1,    -1,
      -1,    25,    26,    -1,    -1,    46,    47,    31,    32,    33,
      -1,    52,    -1,    -1,     7,     8,     9,    10,    -1,    -1,
      39,    40,    46,    47,    17,    44,    19,    20,    52,    48,
      49,    -1,    25,    26,    -1,    54,    55,    -1,    31,    11,
      33,    -1,    -1,    -1,    -1,    -1,    18,    -1,    -1,    -1,
      -1,    23,    -1,    46,    47,    27,    28,    29,    30,    52,
      39,    40,    -1,    35,    -1,    44,    45,    46,    47,    48,
      49,    -1,    39,    40,    53,    54,    55,    44,    -1,    46,
      47,    48,    49,    -1,    39,    40,    53,    54,    55,    44,
      45,    46,    47,    48,    49,    -1,    39,    40,    -1,    54,
      55,    44,    -1,    46,    47,    48,    49,    -1,    39,    40,
      -1,    54,    55,    44,    -1,    -1,    -1,    48,    49,    -1,
      -1,    52,    -1,    54,    55
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     7,     8,     9,    10,    17,    19,    20,    21,
      24,    25,    26,    31,    32,    33,    46,    47,    52,    58,
      60,    61,    62,    63,    64,    70,    71,    72,    77,    78,
      80,    83,    87,    52,    52,    74,    10,    79,    11,    18,
      23,    27,    28,    29,    30,    35,    81,    82,    10,    11,
      10,    10,    31,    61,    77,    36,    37,    10,    77,    77,
      77,     0,    51,    59,    52,    45,    52,    69,    77,    39,
      40,    44,    46,    47,    48,    49,    54,    55,    77,    88,
      88,    88,    56,    11,    11,    27,    11,    18,    27,    35,
      82,    84,    77,    22,    10,    35,    45,    53,    60,    69,
      77,    69,    77,    34,    45,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    53,    56,    53,    10,    11,    11,
      11,    74,    10,    77,    77,    53,    75,    53,    59,    74,
      77,    77,    10,    85,    76,    13,    35,    76,    62,    78,
      56,    59,    59,    63,    75,    35,    77,    75,    75,    10,
      56,    86,    16,    77,    75,    14,    16,    74,    15,    38,
      67,    73,    10,    76,    32,    75,    76,    76,    21,    66,
      67,    68,    73,    74,    75,    69,    76,    75,    75,    68,
      75,    69,    62,    38,    65,    34,    75,    16,    16,    16,
      34,    75,    74,    75,    74,    16,    32,    21,    21,    76,
      62,    63,    32,    75,    75
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
#line 97 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1;
	;}
    break;

  case 9:
#line 106 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 11:
#line 108 "engines/director/lingo/lingo-gr.y"
    { yyerrok; ;}
    break;

  case 12:
#line 111 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 13:
#line 117 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 14:
#line 123 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 15:
#line 130 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 19:
#line 139 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 2] = end; ;}
    break;

  case 20:
#line 150 "engines/director/lingo/lingo-gr.y"
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

  case 21:
#line 166 "engines/director/lingo/lingo-gr.y"
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

  case 22:
#line 180 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (8)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (8)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (8)].code) + 3] = end;	/* end, if cond fails */
		g_lingo->processIf(0, 0); ;}
    break;

  case 23:
#line 187 "engines/director/lingo/lingo-gr.y"
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

  case 24:
#line 196 "engines/director/lingo/lingo-gr.y"
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

  case 25:
#line 205 "engines/director/lingo/lingo-gr.y"
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

  case 26:
#line 215 "engines/director/lingo/lingo-gr.y"
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

  case 27:
#line 225 "engines/director/lingo/lingo-gr.y"
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

  case 28:
#line 236 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 29:
#line 237 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 32:
#line 244 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 34:
#line 252 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (5)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (5)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (5)].code)); ;}
    break;

  case 35:
#line 260 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 36:
#line 261 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 38:
#line 264 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 39:
#line 266 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 40:
#line 272 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 41:
#line 278 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 42:
#line 285 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 43:
#line 287 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 44:
#line 289 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 47:
#line 294 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_constpush);
		inst i = 0;
		WRITE_UINT32(&i, (yyvsp[(1) - (1)].i));
		g_lingo->code1(i); ;}
    break;

  case 48:
#line 299 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_fconstpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 49:
#line 302 "engines/director/lingo/lingo-gr.y"
    {
		if ((yyvsp[(3) - (4)].narg) != g_lingo->_builtins[*(yyvsp[(1) - (4)].s)]->nargs)
			error("Built-in function %s expects %d arguments but got %d", (yyvsp[(1) - (4)].s)->c_str(), g_lingo->_builtins[*(yyvsp[(1) - (4)].s)]->nargs, (yyvsp[(3) - (4)].narg));

		(yyval.code) = g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (4)].s)]->func);
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 50:
#line 308 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (4)].s)->c_str());

		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(3) - (4)].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 51:
#line 316 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeId(*(yyvsp[(1) - (1)].s));
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 53:
#line 320 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 54:
#line 321 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 55:
#line 322 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 56:
#line 323 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 57:
#line 324 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 58:
#line 325 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 59:
#line 326 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 60:
#line 327 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 61:
#line 328 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 62:
#line 329 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 63:
#line 330 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 64:
#line 331 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 65:
#line 334 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mci); g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str()); delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 66:
#line 335 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mciwait); g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str()); delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 67:
#line 336 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 69:
#line 338 "engines/director/lingo/lingo-gr.y"
    { 	g_lingo->code2(g_lingo->c_constpush, (inst)0); // Push fake value on stack
								g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 71:
#line 343 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 72:
#line 344 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 73:
#line 355 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 74:
#line 356 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 75:
#line 357 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 76:
#line 358 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_goto);
		g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str());
		g_lingo->codeString("");
		delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 77:
#line 363 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_goto);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s);
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 78:
#line 369 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_goto);
		g_lingo->codeString("");
		g_lingo->codeString((yyvsp[(2) - (2)].s)->c_str());
		delete (yyvsp[(2) - (2)].s); ;}
    break;

  case 79:
#line 376 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(3) - (3)].s); ;}
    break;

  case 80:
#line 377 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); ;}
    break;

  case 81:
#line 378 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); ;}
    break;

  case 82:
#line 379 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(1) - (1)].s); ;}
    break;

  case 83:
#line 382 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(3) - (3)].s); ;}
    break;

  case 84:
#line 383 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); ;}
    break;

  case 85:
#line 384 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(3) - (3)].s); ;}
    break;

  case 86:
#line 412 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 87:
#line 413 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code2(g_lingo->c_constpush, (inst)0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 88:
#line 419 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 89:
#line 420 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 90:
#line 421 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 91:
#line 422 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 92:
#line 424 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 93:
#line 427 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (3)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(3) - (3)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 94:
#line 435 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 95:
#line 436 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 96:
#line 437 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2173 "engines/director/lingo/lingo-gr.cpp"
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


#line 440 "engines/director/lingo/lingo-gr.y"


