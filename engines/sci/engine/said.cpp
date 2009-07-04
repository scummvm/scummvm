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
     WGROUP = 258,
     YY_COMMA = 259,
     YY_AMP = 260,
     YY_SLASH = 261,
     YY_PARENO = 262,
     YY_PARENC = 263,
     YY_BRACKETSO = 264,
     YY_BRACKETSC = 265,
     YY_HASH = 266,
     YY_LT = 267,
     YY_GT = 268,
     YY_BRACKETSO_LT = 269,
     YY_BRACKETSO_SLASH = 270,
     YY_LT_BRACKETSO = 271,
     YY_LT_PARENO = 272
   };
#endif
/* Tokens.  */
#define WGROUP 258
#define YY_COMMA 259
#define YY_AMP 260
#define YY_SLASH 261
#define YY_PARENO 262
#define YY_PARENC 263
#define YY_BRACKETSO 264
#define YY_BRACKETSC 265
#define YY_HASH 266
#define YY_LT 267
#define YY_GT 268
#define YY_BRACKETSO_LT 269
#define YY_BRACKETSO_SLASH 270
#define YY_LT_BRACKETSO 271
#define YY_LT_PARENO 272




/* Copy the first part of user declarations.  */
#line 1 "said.y"

/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/engine/state.h"


// Bison generates an empty switch statement that gives a warning in MSVC.
// This disables that warning.
#ifdef _MSC_VER
#pragma warning(disable:4065)
#endif


namespace Sci {

#define SAID_BRANCH_NULL 0

#define MAX_SAID_TOKENS 128

// Maximum number of words to be expected in a parsed sentence
#define AUGMENT_MAX_WORDS 64


#define ANYWORD 0xfff

#define WORD_TYPE_BASE 0x141
#define WORD_TYPE_REF 0x144
#define WORD_TYPE_SYNTACTIC_SUGAR 0x145

#define AUGMENT_SENTENCE_PART_BRACKETS 0x152

// Minor numbers
#define AUGMENT_SENTENCE_MINOR_MATCH_PHRASE 0x14c
#define AUGMENT_SENTENCE_MINOR_MATCH_WORD 0x153
#define AUGMENT_SENTENCE_MINOR_RECURSE 0x144
#define AUGMENT_SENTENCE_MINOR_PARENTHESES 0x14f


#undef YYDEBUG /*1*/
//#define SAID_DEBUG*/
//#define SCI_DEBUG_PARSE_TREE_AUGMENTATION // uncomment to debug parse tree augmentation


#ifdef SCI_DEBUG_PARSE_TREE_AUGMENTATION
#define scidprintf sciprintf
#else
#define scidprintf if (0) sciprintf
#endif


static char *said_parse_error;

static int said_token;
static int said_tokens_nr;
static int said_tokens[MAX_SAID_TOKENS];
static int said_blessed;  // increminated by said_top_branch

static int said_tree_pos; // Set to 0 if we're out of space
#define SAID_TREE_START 4; // Reserve space for the 4 top nodes

#define VALUE_IGNORE -424242

static parse_tree_node_t said_tree[VOCAB_TREE_NODES];

typedef int wgroup_t;
typedef int tree_t;
typedef int said_spec_t;

static tree_t said_aug_branch(int, int, tree_t, tree_t);
static tree_t said_attach_branch(tree_t, tree_t);
/*
static tree_t said_wgroup_branch(wgroup_t);
*/
static said_spec_t said_top_branch(tree_t);
static tree_t said_paren(tree_t, tree_t);
static tree_t said_value(int, tree_t);
static tree_t said_terminal(int);

static int yylex(void);

static int yyerror(const char *s) {
	said_parse_error = strdup(s);
	return 1; /* Abort */
}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
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
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 250 "said.tab.c"

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
# if YYENABLE_NLS
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
#define YYFINAL  23
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   80

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  18
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  13
/* YYNRULES -- Number of rules.  */
#define YYNRULES  35
/* YYNRULES -- Number of states.  */
#define YYNSTATES  69

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   272

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     6,    10,    15,    16,    18,    19,    21,
      24,    29,    31,    34,    39,    41,    43,    45,    49,    51,
      55,    59,    64,    70,    73,    75,    77,    79,    83,    88,
      92,    97,   100,   105,   109,   112
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      19,     0,    -1,    21,    20,    -1,    21,    22,    20,    -1,
      21,    22,    23,    20,    -1,    -1,    13,    -1,    -1,    27,
      -1,     6,    27,    -1,    15,     6,    27,    10,    -1,     6,
      -1,     6,    27,    -1,    15,     6,    27,    10,    -1,     6,
      -1,     3,    -1,    26,    -1,     9,    26,    10,    -1,    24,
      -1,     7,    27,     8,    -1,    26,     4,    26,    -1,    26,
      14,    29,    10,    -1,    26,     4,     9,    26,    10,    -1,
      25,    28,    -1,    25,    -1,    28,    -1,    29,    -1,    14,
      29,    10,    -1,    29,    14,    29,    10,    -1,    12,    24,
      30,    -1,    17,     7,    27,     8,    -1,    12,    26,    -1,
      16,     9,    26,    10,    -1,    12,    26,    30,    -1,    12,
      26,    -1,    17,     7,    27,     8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   129,   129,   131,   133,   139,   140,   147,   148,   154,
     156,   158,   164,   166,   168,   173,   178,   180,   185,   187,
     189,   191,   193,   198,   200,   202,   207,   209,   211,   216,
     218,   220,   222,   227,   229,   231
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "WGROUP", "YY_COMMA", "YY_AMP",
  "YY_SLASH", "YY_PARENO", "YY_PARENC", "YY_BRACKETSO", "YY_BRACKETSC",
  "YY_HASH", "YY_LT", "YY_GT", "YY_BRACKETSO_LT", "YY_BRACKETSO_SLASH",
  "YY_LT_BRACKETSO", "YY_LT_PARENO", "$accept", "saidspec", "optcont",
  "leftspec", "midspec", "rightspec", "word", "cwordset", "wordset",
  "expr", "cwordrefset", "wordrefset", "recref", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    18,    19,    19,    19,    20,    20,    21,    21,    22,
      22,    22,    23,    23,    23,    24,    25,    25,    26,    26,
      26,    26,    26,    27,    27,    27,    28,    28,    28,    29,
      29,    29,    29,    30,    30,    30
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     4,     0,     1,     0,     1,     2,
       4,     1,     2,     4,     1,     1,     1,     3,     1,     3,
       3,     4,     5,     2,     1,     1,     1,     3,     4,     3,
       4,     2,     4,     3,     2,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       7,    15,     0,     0,     0,     0,     0,     0,     0,     5,
      18,    24,    16,     8,    25,    26,     0,     0,    18,    31,
       0,     0,     0,     1,    11,     6,     0,     2,     5,    23,
       0,     0,     0,    19,    17,     0,     0,    29,    27,     0,
       0,     9,     0,    14,     0,     3,     5,     0,    20,     0,
       0,    34,     0,    32,    30,     0,    12,     0,     4,     0,
      21,    28,    33,     0,    10,     0,    22,    35,    13
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     8,    27,     9,    28,    46,    10,    11,    12,    13,
      14,    15,    37
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -24
static const yytype_int8 yypact[] =
{
      -1,   -24,    -1,    62,    62,    54,     1,     5,    18,    38,
     -24,    47,     3,   -24,   -24,    12,    23,    15,    -3,     3,
      28,    62,    -1,   -24,    -1,   -24,    42,   -24,    39,   -24,
      53,    54,    54,   -24,   -24,    62,    50,   -24,   -24,    29,
      41,   -24,    -1,    -1,    52,   -24,    55,    62,     3,    57,
      63,    20,    -1,   -24,   -24,    64,   -24,    -1,   -24,    32,
     -24,   -24,   -24,    67,   -24,    66,   -24,   -24,   -24
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -24,   -24,   -23,   -24,   -24,   -24,    68,   -24,     0,    -2,
      69,    -4,    26
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      16,    20,     1,    17,    19,    45,     2,    30,     3,    35,
      21,     4,    22,     5,    36,     6,     7,    31,    23,    30,
      40,    39,    41,    58,    30,    34,    32,    49,    50,    31,
      48,    33,    35,    30,    31,    51,    30,    36,    38,    53,
      55,    56,    66,    31,    24,    43,    31,    59,    42,    54,
      63,    25,    25,    26,    44,    65,     1,    52,    57,     4,
       2,     5,    47,     6,     7,     1,     4,    60,    25,     2,
       6,     7,    18,    61,    64,    67,    68,    62,     0,     0,
      29
};

static const yytype_int8 yycheck[] =
{
       2,     5,     3,     3,     4,    28,     7,     4,     9,    12,
       9,    12,     7,    14,    17,    16,    17,    14,     0,     4,
      22,    21,    24,    46,     4,    10,    14,    31,    32,    14,
      30,     8,    12,     4,    14,    35,     4,    17,    10,    10,
      42,    43,    10,    14,     6,     6,    14,    47,     6,     8,
      52,    13,    13,    15,    15,    57,     3,     7,     6,    12,
       7,    14,     9,    16,    17,     3,    12,    10,    13,     7,
      16,    17,     4,    10,    10,     8,    10,    51,    -1,    -1,
      11
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     7,     9,    12,    14,    16,    17,    19,    21,
      24,    25,    26,    27,    28,    29,    27,    26,    24,    26,
      29,     9,     7,     0,     6,    13,    15,    20,    22,    28,
       4,    14,    14,     8,    10,    12,    17,    30,    10,    26,
      27,    27,     6,     6,    15,    20,    23,     9,    26,    29,
      29,    26,     7,    10,     8,    27,    27,     6,    20,    26,
      10,    10,    30,    27,    10,    27,    10,     8,    10
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
# if YYLTYPE_IS_TRIVIAL
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
        case 2:
#line 130 "said.y"
    { (yyval) = said_top_branch(said_attach_branch((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]))); ;}
    break;

  case 3:
#line 132 "said.y"
    { (yyval) = said_top_branch(said_attach_branch((yyvsp[(1) - (3)]), said_attach_branch((yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])))); ;}
    break;

  case 4:
#line 134 "said.y"
    { (yyval) = said_top_branch(said_attach_branch((yyvsp[(1) - (4)]), said_attach_branch((yyvsp[(2) - (4)]), said_attach_branch((yyvsp[(3) - (4)]), (yyvsp[(4) - (4)]))))); ;}
    break;

  case 5:
#line 139 "said.y"
    { (yyval) = SAID_BRANCH_NULL; ;}
    break;

  case 6:
#line 141 "said.y"
    { (yyval) = said_paren(said_value(0x14b, said_value(0xf900, said_terminal(0xf900))), SAID_BRANCH_NULL); ;}
    break;

  case 7:
#line 147 "said.y"
    { (yyval) = SAID_BRANCH_NULL; ;}
    break;

  case 8:
#line 149 "said.y"
    { (yyval) = said_paren(said_value(0x141, said_value(0x149, (yyvsp[(1) - (1)]))), SAID_BRANCH_NULL); ;}
    break;

  case 9:
#line 155 "said.y"
    { (yyval) = said_aug_branch(0x142, 0x14a, (yyvsp[(2) - (2)]), SAID_BRANCH_NULL); ;}
    break;

  case 10:
#line 157 "said.y"
    { (yyval) = said_aug_branch(0x152, 0x142, said_aug_branch(0x142, 0x14a, (yyvsp[(3) - (4)]), SAID_BRANCH_NULL), SAID_BRANCH_NULL); ;}
    break;

  case 11:
#line 159 "said.y"
    { (yyval) = SAID_BRANCH_NULL; ;}
    break;

  case 12:
#line 165 "said.y"
    { (yyval) = said_aug_branch(0x143, 0x14a, (yyvsp[(2) - (2)]), SAID_BRANCH_NULL); ;}
    break;

  case 13:
#line 167 "said.y"
    { (yyval) = said_aug_branch(0x152, 0x143, said_aug_branch(0x143, 0x14a, (yyvsp[(3) - (4)]), SAID_BRANCH_NULL), SAID_BRANCH_NULL); ;}
    break;

  case 14:
#line 169 "said.y"
    { (yyval) = SAID_BRANCH_NULL; ;}
    break;

  case 15:
#line 174 "said.y"
    { (yyval) = said_paren(said_value(0x141, said_value(0x153, said_terminal((yyvsp[(1) - (1)])))), SAID_BRANCH_NULL); ;}
    break;

  case 16:
#line 179 "said.y"
    { (yyval) = said_aug_branch(0x141, 0x14f, (yyvsp[(1) - (1)]), SAID_BRANCH_NULL); ;}
    break;

  case 17:
#line 181 "said.y"
    { (yyval) = said_aug_branch(0x141, 0x14f, said_aug_branch(0x152, 0x14c, said_aug_branch(0x141, 0x14f, (yyvsp[(2) - (3)]), SAID_BRANCH_NULL), SAID_BRANCH_NULL), SAID_BRANCH_NULL); ;}
    break;

  case 18:
#line 186 "said.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 19:
#line 188 "said.y"
    { (yyval) = said_aug_branch(0x141, 0x14c, (yyvsp[(2) - (3)]), SAID_BRANCH_NULL); ;}
    break;

  case 20:
#line 190 "said.y"
    { (yyval) = said_attach_branch((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 21:
#line 192 "said.y"
    { (yyval) = said_attach_branch((yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 22:
#line 194 "said.y"
    { (yyval) = said_attach_branch((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)])); ;}
    break;

  case 23:
#line 199 "said.y"
    { (yyval) = said_attach_branch((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 24:
#line 201 "said.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 25:
#line 203 "said.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 26:
#line 208 "said.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 27:
#line 210 "said.y"
    { (yyval) = said_aug_branch(0x152, 0x144, (yyvsp[(2) - (3)]), SAID_BRANCH_NULL); ;}
    break;

  case 28:
#line 212 "said.y"
    { (yyval) = said_attach_branch((yyvsp[(1) - (4)]), said_aug_branch(0x152, 0x144, (yyvsp[(3) - (4)]), SAID_BRANCH_NULL)); ;}
    break;

  case 29:
#line 217 "said.y"
    { (yyval) = said_aug_branch(0x144, 0x14f, (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 30:
#line 219 "said.y"
    { (yyval) = said_aug_branch(0x144, 0x14f, said_aug_branch(0x141, 0x144, (yyvsp[(2) - (4)]), SAID_BRANCH_NULL), SAID_BRANCH_NULL); ;}
    break;

  case 31:
#line 221 "said.y"
    { (yyval) = said_aug_branch(0x144, 0x14f, (yyvsp[(2) - (2)]), SAID_BRANCH_NULL); ;}
    break;

  case 32:
#line 223 "said.y"
    { (yyval) = said_aug_branch(0x152, 0x144, said_aug_branch(0x144, 0x14f, (yyvsp[(3) - (4)]), SAID_BRANCH_NULL), SAID_BRANCH_NULL); ;}
    break;

  case 33:
#line 228 "said.y"
    { (yyval) = said_aug_branch(0x141, 0x144, said_aug_branch(0x144, 0x14f, (yyvsp[(2) - (3)]), SAID_BRANCH_NULL), (yyvsp[(3) - (3)])); ;}
    break;

  case 34:
#line 230 "said.y"
    { (yyval) = said_aug_branch(0x141, 0x144, said_aug_branch(0x144, 0x14f, (yyvsp[(2) - (2)]), SAID_BRANCH_NULL), SAID_BRANCH_NULL); ;}
    break;

  case 35:
#line 232 "said.y"
    { (yyval) = said_aug_branch(0x141, 0x14c, (yyvsp[(2) - (4)]), SAID_BRANCH_NULL); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 1665 "said.tab.c"
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


#line 235 "said.y"


int parse_yy_token_lookup[] = {YY_COMMA, YY_AMP, YY_SLASH, YY_PARENO, YY_PARENC, YY_BRACKETSO, YY_BRACKETSC, YY_HASH, YY_LT, YY_GT};

static int yylex(void) {
	int retval = said_tokens[said_token++];

	if (retval < SAID_LONG(SAID_FIRST)) {
		yylval = retval;
		retval = WGROUP;
	} else {
		retval >>= 8;

		if (retval == SAID_TERM)
			retval = 0;
		else {
			assert(retval >= SAID_FIRST);
			retval = parse_yy_token_lookup[retval - SAID_FIRST];
			if (retval == YY_BRACKETSO) {
				if ((said_tokens[said_token] >> 8) == SAID_LT)
					retval = YY_BRACKETSO_LT;
				else
					if ((said_tokens[said_token] >> 8) == SAID_SLASH)
						retval = YY_BRACKETSO_SLASH;
			} else if (retval == YY_LT && (said_tokens[said_token] >> 8) == SAID_BRACKO) {
				retval = YY_LT_BRACKETSO;
			} else if (retval == YY_LT && (said_tokens[said_token] >> 8) == SAID_PARENO) {
				retval = YY_LT_PARENO;
			}
		}
	}

	return retval;
}

static int said_next_node() {
	return ((said_tree_pos == 0) || (said_tree_pos >= VOCAB_TREE_NODES)) ? said_tree_pos = 0 : said_tree_pos++;
}

#define SAID_NEXT_NODE said_next_node()

static int said_leaf_node(tree_t pos, int value) {
	said_tree[pos].type = kParseTreeLeafNode;

	if (value != VALUE_IGNORE)
		said_tree[pos].content.value = value;

	return pos;
}

static int said_branch_node(tree_t pos, int left, int right) {
	said_tree[pos].type = kParseTreeBranchNode;

	if (left != VALUE_IGNORE)
		said_tree[pos].content.branches[0] = left;

	if (right != VALUE_IGNORE)
		said_tree[pos].content.branches[1] = right;

	return pos;
}

static tree_t said_paren(tree_t t1, tree_t t2) {
	if (t1)
		return said_branch_node(SAID_NEXT_NODE, t1, t2);
	else
		return t2;
}

static tree_t said_value(int val, tree_t t) {
	return said_branch_node(SAID_NEXT_NODE, said_leaf_node(SAID_NEXT_NODE, val), t);

}

static tree_t said_terminal(int val) {
	return said_leaf_node(SAID_NEXT_NODE, val);
}

static tree_t said_aug_branch(int n1, int n2, tree_t t1, tree_t t2) {
	int retval;

	retval = said_branch_node(SAID_NEXT_NODE,
				said_branch_node(SAID_NEXT_NODE,
					said_leaf_node(SAID_NEXT_NODE, n1),
						said_branch_node(SAID_NEXT_NODE,
							said_leaf_node(SAID_NEXT_NODE, n2),
						t1)
					),
				t2);

#ifdef SAID_DEBUG
	fprintf(stderr, "AUG(0x%x, 0x%x, [%04x], [%04x]) = [%04x]\n", n1, n2, t1, t2, retval);
#endif

	return retval;
}

static tree_t said_attach_branch(tree_t base, tree_t attacheant) {
#ifdef SAID_DEBUG
	fprintf(stderr, "ATT2([%04x], [%04x]) = [%04x]\n", base, attacheant, base);
#endif

	if (!attacheant)
		return base;
	if (!base)
		return attacheant;

	if (!base)
		return 0; // Happens if we're out of space

	said_branch_node(base, VALUE_IGNORE, attacheant);

	return base;
}

static said_spec_t said_top_branch(tree_t first) {
#ifdef SAID_DEBUG
	fprintf(stderr, "TOP([%04x])\n", first);
#endif
	said_branch_node(0, 1, 2);
	said_leaf_node(1, 0x141); // Magic number #1
	said_branch_node(2, 3, first);
	said_leaf_node(3, 0x13f); // Magic number #2

	++said_blessed;

	return 0;
}

static int said_parse_spec(EngineState *s, byte *spec) {
	int nextitem;

	said_parse_error = NULL;
	said_token = 0;
	said_tokens_nr = 0;
	said_blessed = 0;

	said_tree_pos = SAID_TREE_START;

	do {
		nextitem = *spec++;
		if (nextitem < SAID_FIRST)
			said_tokens[said_tokens_nr++] = nextitem << 8 | *spec++;
		else
			said_tokens[said_tokens_nr++] = SAID_LONG(nextitem);

	} while ((nextitem != SAID_TERM) && (said_tokens_nr < MAX_SAID_TOKENS));

	if (nextitem == SAID_TERM)
		yyparse();
	else {
		sciprintf("Error: SAID spec is too long\n");
		return 1;
	}

	if (said_parse_error) {
		sciprintf("Error while parsing SAID spec: %s\n", said_parse_error);
		free(said_parse_error);
		return 1;
	}

	if (said_tree_pos == 0) {
		sciprintf("Error: Out of tree space while parsing SAID spec\n");
		return 1;
	}

	if (said_blessed != 1) {
		sciprintf("Error: Found multiple top branches\n");
		return 1;
	}

	return 0;
}

/**********************/
/**** Augmentation ****/
/**********************/

// primitive functions

#define AUG_READ_BRANCH(a, br, p) \
	if (tree[p].type != kParseTreeBranchNode) \
		return 0; \
	a = tree[p].content.branches[br];

#define AUG_READ_VALUE(a, p) \
	if (tree[p].type != kParseTreeLeafNode) \
		return 0; \
	a = tree[p].content.value;

#define AUG_ASSERT(i) \
	if (!i) return 0;

static int aug_get_next_sibling(parse_tree_node_t *tree, int pos, int *first, int *second) {
	// Returns the next sibling relative to the specified position in 'tree',
	// sets *first and *second to its augment node values, returns the new position
	// or 0 if there was no next sibling
	int seek, valpos;

	AUG_READ_BRANCH(pos, 1, pos);
	AUG_ASSERT(pos);
	AUG_READ_BRANCH(seek, 0, pos);
	AUG_ASSERT(seek);

	// Now retrieve first value
	AUG_READ_BRANCH(valpos, 0, seek);
	AUG_ASSERT(valpos);
	AUG_READ_VALUE(*first, valpos);

	// Get second value
	AUG_READ_BRANCH(seek, 1, seek);
	AUG_ASSERT(seek);
	AUG_READ_BRANCH(valpos, 0, seek);
	AUG_ASSERT(valpos);
	AUG_READ_VALUE(*second, valpos);

	return pos;
}

static int aug_get_wgroup(parse_tree_node_t *tree, int pos) {
	// Returns 0 if pos in tree is not the root of a 3-element list, otherwise
	// it returns the last element (which, in practice, is the word group
	int val;

	AUG_READ_BRANCH(pos, 0, pos);
	AUG_ASSERT(pos);
	AUG_READ_BRANCH(pos, 1, pos);
	AUG_ASSERT(pos);
	AUG_READ_BRANCH(pos, 1, pos);
	AUG_ASSERT(pos);
	AUG_READ_VALUE(val, pos);

	return val;
}

static int aug_get_base_node(parse_tree_node_t *tree) {
	int startpos = 0;
	AUG_READ_BRANCH(startpos, 1, startpos);

	return startpos;
}

// semi-primitive functions

static int aug_get_first_child(parse_tree_node_t *tree, int pos, int *first, int *second) {
	// like aug_get_next_sibling, except that it recurses into the tree and
	// finds the first child (usually *not* Ayanami Rei) of the current branch
	// rather than its next sibling.
	AUG_READ_BRANCH(pos, 0, pos);
	AUG_ASSERT(pos);
	AUG_READ_BRANCH(pos, 1, pos);
	AUG_ASSERT(pos);

	return aug_get_next_sibling(tree, pos, first, second);
}

static void aug_find_words_recursively(parse_tree_node_t *tree, int startpos, int *base_words, int *base_words_nr,
			   int *ref_words, int *ref_words_nr, int maxwords, int refbranch) {
	// Finds and lists all base (141) and reference (144) words */
	int major, minor;
	int word;
	int pos = aug_get_first_child(tree, startpos, &major, &minor);

	//if (major == WORD_TYPE_REF)
	//	refbranch = 1;

	while (pos) {
		if ((word = aug_get_wgroup(tree, pos))) { // found a word
			if (!refbranch && major == WORD_TYPE_BASE) {
				if ((*base_words_nr) == maxwords) {
					sciprintf("Out of regular words\n");
					return; // return gracefully
				}

				base_words[*base_words_nr] = word; // register word
				++(*base_words_nr);

			}
			if (major == WORD_TYPE_REF || refbranch) {
				if ((*ref_words_nr) == maxwords) {
					sciprintf("Out of reference words\n");
					return; // return gracefully
				}

				ref_words[*ref_words_nr] = word; // register word
				++(*ref_words_nr);

			}
			if (major != WORD_TYPE_SYNTACTIC_SUGAR && major != WORD_TYPE_BASE && major != WORD_TYPE_REF)
				sciprintf("aug_find_words_recursively(): Unknown word type %03x\n", major);

		} else // Did NOT find a word group: Attempt to recurse
			aug_find_words_recursively(tree, pos, base_words, base_words_nr,
						   ref_words, ref_words_nr, maxwords, refbranch || major == WORD_TYPE_REF);

		pos = aug_get_next_sibling(tree, pos, &major, &minor);
	}
}


static void aug_find_words(parse_tree_node_t *tree, int startpos, int *base_words, int *base_words_nr,
				int *ref_words, int *ref_words_nr, int maxwords) {
	// initializing wrapper for aug_find_words_recursively()
	*base_words_nr = 0;
	*ref_words_nr = 0;

	aug_find_words_recursively(tree, startpos, base_words, base_words_nr, ref_words, ref_words_nr, maxwords, 0);
}


static int aug_contains_word(int *list, int length, int word) {
	int i;

	if (word == ANYWORD)
		return (length);

	for (i = 0; i < length; i++)
		if (list[i] == word)
			return 1;

	return 0;
}


static int augment_sentence_expression(parse_tree_node_t *saidt, int augment_pos, parse_tree_node_t *parset,
					int parse_branch, int major, int minor, int *base_words, int base_words_nr,
					int *ref_words, int ref_words_nr);

static int augment_match_expression_p(parse_tree_node_t *saidt, int augment_pos, parse_tree_node_t *parset,
					int parse_basepos, int major, int minor,
			   int *base_words, int base_words_nr, int *ref_words, int ref_words_nr) {
	int cmajor, cminor, cpos;
	cpos = aug_get_first_child(saidt, augment_pos, &cmajor, &cminor);
	if (!cpos) {
		sciprintf("augment_match_expression_p(): Empty condition\n");
		return 1;
	}

	scidprintf("Attempting to match (%03x %03x (%03x %03x\n", major, minor, cmajor, cminor);

	if ((major == WORD_TYPE_BASE) && (minor == AUGMENT_SENTENCE_MINOR_RECURSE))
		return augment_match_expression_p(saidt, cpos, parset, parse_basepos, cmajor, cminor,
						  base_words, base_words_nr, ref_words, ref_words_nr);

	switch (major) {

	case WORD_TYPE_BASE:
		while (cpos) {
			if (cminor == AUGMENT_SENTENCE_MINOR_MATCH_WORD) {
				int word = aug_get_wgroup(saidt, cpos);
				scidprintf("Looking for word %03x\n", word);

				if (aug_contains_word(base_words, base_words_nr, word))
					return 1;
			} else if (cminor == AUGMENT_SENTENCE_MINOR_MATCH_PHRASE) {
				if (augment_sentence_expression(saidt, cpos, parset, parse_basepos, cmajor, cminor,
								base_words, base_words_nr, ref_words, ref_words_nr))
					return 1;
			} else if (cminor == AUGMENT_SENTENCE_MINOR_PARENTHESES) {
				int gc_major, gc_minor;
				int gchild = aug_get_first_child(saidt, cpos, &gc_major, &gc_minor);

				while (gchild) {
					if (augment_match_expression_p(saidt, cpos, parset, parse_basepos, major,
									minor, base_words, base_words_nr,
								       ref_words, ref_words_nr))
						return 1;
					gchild = aug_get_next_sibling(saidt, gchild, &gc_major, &gc_minor);
				}
			} else
				sciprintf("augment_match_expression_p(): Unknown type 141 minor number %3x\n", cminor);

			cpos = aug_get_next_sibling(saidt, cpos, &cmajor, &cminor);

		}
		break;

	case WORD_TYPE_REF:
		while (cpos) {
			if (cminor == AUGMENT_SENTENCE_MINOR_MATCH_WORD) {
				int word = aug_get_wgroup(saidt, cpos);
				scidprintf("Looking for refword %03x\n", word);

				if (aug_contains_word(ref_words, ref_words_nr, word))
					return 1;
			} else if (cminor == AUGMENT_SENTENCE_MINOR_MATCH_PHRASE) {
				if (augment_match_expression_p(saidt, cpos, parset, parse_basepos, cmajor, cminor,
							       base_words, base_words_nr, ref_words, ref_words_nr))
					return 1;
			} else if (cminor == AUGMENT_SENTENCE_MINOR_PARENTHESES) {
				int gc_major, gc_minor;
				int gchild = aug_get_first_child(saidt, cpos, &gc_major, &gc_minor);

				while (gchild) {
					if (augment_match_expression_p(saidt, cpos, parset, parse_basepos, major,
									minor, base_words, base_words_nr,
									ref_words, ref_words_nr))
						return 1;
					gchild = aug_get_next_sibling(saidt, gchild, &gc_major, &gc_minor);
				}
			} else
				sciprintf("augment_match_expression_p(): Unknown type 144 minor number %3x\n", cminor);

			cpos = aug_get_next_sibling(saidt, cpos, &cmajor, &cminor);

		}
		break;

	case AUGMENT_SENTENCE_PART_BRACKETS:
		if (augment_match_expression_p(saidt, cpos, parset, parse_basepos, cmajor, cminor,
					       base_words, base_words_nr, ref_words, ref_words_nr))
			return 1;

		scidprintf("Didn't match subexpression; checking sub-bracked predicate %03x\n", cmajor);

		switch (cmajor) {
		case WORD_TYPE_BASE:
			if (!base_words_nr)
				return 1;
			break;

		case WORD_TYPE_REF:
			if (!ref_words_nr)
				return 1;
			break;

		default:
			sciprintf("augment_match_expression_p(): (subp1) Unkonwn sub-bracket predicate %03x\n", cmajor);
		}

		break;

	default:
		sciprintf("augment_match_expression_p(): Unknown predicate %03x\n", major);

	}

	scidprintf("Generic failure\n");

	return 0;
}

static int augment_sentence_expression(parse_tree_node_t *saidt, int augment_pos, parse_tree_node_t *parset,
					int parse_branch, int major, int minor, int *base_words, int base_words_nr,
					int *ref_words, int ref_words_nr) {
	int check_major, check_minor;
	int check_pos = aug_get_first_child(saidt, augment_pos, &check_major, &check_minor);
	do {
		if (!(augment_match_expression_p(saidt, check_pos, parset, parse_branch, check_major, check_minor,
						base_words, base_words_nr, ref_words, ref_words_nr)))
			return 0;
	} while ((check_pos = aug_get_next_sibling(saidt, check_pos, &check_major, &check_minor)));

	return 1;
}

static int augment_sentence_part(parse_tree_node_t *saidt, int augment_pos, parse_tree_node_t *parset, int parse_basepos, int major, int minor) {
	int pmajor, pminor;
	int parse_branch = parse_basepos;
	int optional = 0;
	int foundwords = 0;

	scidprintf("Augmenting (%03x %03x\n", major, minor);

	if (major == AUGMENT_SENTENCE_PART_BRACKETS) { // '[/ foo]' is true if '/foo' or if there
						       // exists no x for which '/x' is true
		if ((augment_pos = aug_get_first_child(saidt, augment_pos, &major, &minor))) {
			scidprintf("Optional part: Now augmenting (%03x %03x\n", major, minor);
			optional = 1;
		} else {
			scidprintf("Matched empty optional expression\n");
			return 1;
		}
	}

	if ((major < 0x141) || (major > 0x143)) {
		scidprintf("augment_sentence_part(): Unexpected sentence part major number %03x\n", major);
		return 0;
	}

	while ((parse_branch = aug_get_next_sibling(parset, parse_branch, &pmajor, &pminor))) {
		if (pmajor == major) { // found matching sentence part
			int success;
			int base_words_nr;
			int ref_words_nr;
			int base_words[AUGMENT_MAX_WORDS];
			int ref_words[AUGMENT_MAX_WORDS];
#ifdef SCI_DEBUG_PARSE_TREE_AUGMENTATION
			int i;
#endif

			scidprintf("Found match with pminor = %03x\n", pminor);
			aug_find_words(parset, parse_branch, base_words, &base_words_nr, ref_words, &ref_words_nr, AUGMENT_MAX_WORDS);
			foundwords |= (ref_words_nr | base_words_nr);
#ifdef SCI_DEBUG_PARSE_TREE_AUGMENTATION
			sciprintf("%d base words:", base_words_nr);
			for (i = 0; i < base_words_nr; i++)
				sciprintf(" %03x", base_words[i]);
			sciprintf("\n%d reference words:", ref_words_nr);
			for (i = 0; i < ref_words_nr; i++)
				sciprintf(" %03x", ref_words[i]);
			sciprintf("\n");
#endif

			success = augment_sentence_expression(saidt, augment_pos, parset, parse_basepos, major, minor,
							      base_words, base_words_nr, ref_words, ref_words_nr);

			if (success) {
				scidprintf("SUCCESS on augmenting (%03x %03x\n", major, minor);
				return 1;
			}
		}
	}

	if (optional && (foundwords == 0)) {
		scidprintf("Found no words and optional branch => SUCCESS on augmenting (%03x %03x\n", major, minor);
		return 1;
	}
	scidprintf("FAILURE on augmenting (%03x %03x\n", major, minor);

	return 0;
}

static int augment_parse_nodes(parse_tree_node_t *parset, parse_tree_node_t *saidt) {
	int augment_basepos = 0;
	int parse_basepos;
	int major, minor;
	int dontclaim = 0;

	parse_basepos = aug_get_base_node(parset);
	if (!parse_basepos) {
		sciprintf("augment_parse_nodes(): Parse tree is corrupt\n");
		return 0;
	}

	augment_basepos = aug_get_base_node(saidt);
	if (!augment_basepos) {
		sciprintf("augment_parse_nodes(): Said tree is corrupt\n");
		return 0;
	}

	while ((augment_basepos = aug_get_next_sibling(saidt, augment_basepos, &major, &minor))) {
		if ((major == 0x14b) && (minor == SAID_LONG(SAID_GT)))
			dontclaim = 1; // special case
		else // normal sentence part
			if (!(augment_sentence_part(saidt, augment_basepos, parset, parse_basepos, major, minor))) {
				scidprintf("Returning failure\n");
				return 0; // fail
			}
	}

	scidprintf("Returning success with dontclaim=%d\n", dontclaim);

	if (dontclaim)
		return SAID_PARTIAL_MATCH;
	else
		return 1; // full match
}


/*******************/
/**** Main code ****/
/*******************/

int said(EngineState *s, byte *spec, int verbose) {
	int retval;

	parse_tree_node_t *parse_tree_ptr = s->parser_nodes;

	if (s->parser_valid) {
		if (said_parse_spec(s, spec)) {
			sciprintf("Offending spec was: ");
			s->_vocabulary->decipherSaidBlock(spec);
			return SAID_NO_MATCH;
		}

		if (verbose)
			vocab_dump_parse_tree("Said-tree", said_tree); // Nothing better to do yet
		retval = augment_parse_nodes(parse_tree_ptr, &(said_tree[0]));

		if (!retval)
			return SAID_NO_MATCH;
		else if (retval != SAID_PARTIAL_MATCH)
			return SAID_FULL_MATCH;
		else
			return SAID_PARTIAL_MATCH;
	}

	return SAID_NO_MATCH;
}


#ifdef SAID_DEBUG_PROGRAM
int main (int argc, char *argv) {
	byte block[] = {0x01, 0x00, 0xf8, 0xf5, 0x02, 0x01, 0xf6, 0xf2, 0x02, 0x01, 0xf2, 0x01, 0x03, 0xff};
	EngineState s;

	s.parser_valid = 1;
	said(&s, block);
}
#endif

} // End of namespace Sci

