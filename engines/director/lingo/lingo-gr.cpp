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
     TWOWORDBUILTIN = 275,
     FBLTIN = 276,
     FBLTINNOARGS = 277,
     FBLTINONEARG = 278,
     ID = 279,
     STRING = 280,
     HANDLER = 281,
     SYMBOL = 282,
     ENDCLAUSE = 283,
     tPLAYACCEL = 284,
     tDOWN = 285,
     tELSE = 286,
     tNLELSIF = 287,
     tEXIT = 288,
     tFRAME = 289,
     tGLOBAL = 290,
     tGO = 291,
     tIF = 292,
     tINTO = 293,
     tLOOP = 294,
     tMACRO = 295,
     tMOVIE = 296,
     tNEXT = 297,
     tOF = 298,
     tPREVIOUS = 299,
     tPUT = 300,
     tREPEAT = 301,
     tSET = 302,
     tTHEN = 303,
     tTO = 304,
     tWHEN = 305,
     tWITH = 306,
     tWHILE = 307,
     tNLELSE = 308,
     tFACTORY = 309,
     tMETHOD = 310,
     tOPEN = 311,
     tPLAY = 312,
     tDONE = 313,
     tINSTANCE = 314,
     tGE = 315,
     tLE = 316,
     tGT = 317,
     tLT = 318,
     tEQ = 319,
     tNEQ = 320,
     tAND = 321,
     tOR = 322,
     tNOT = 323,
     tMOD = 324,
     tAFTER = 325,
     tBEFORE = 326,
     tCONCAT = 327,
     tCONTAINS = 328,
     tSTARTS = 329,
     tCHAR = 330,
     tITEM = 331,
     tLINE = 332,
     tWORD = 333,
     tSPRITE = 334,
     tINTERSECTS = 335,
     tWITHIN = 336,
     tTELL = 337,
     tON = 338,
     tME = 339
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
#define TWOWORDBUILTIN 275
#define FBLTIN 276
#define FBLTINNOARGS 277
#define FBLTINONEARG 278
#define ID 279
#define STRING 280
#define HANDLER 281
#define SYMBOL 282
#define ENDCLAUSE 283
#define tPLAYACCEL 284
#define tDOWN 285
#define tELSE 286
#define tNLELSIF 287
#define tEXIT 288
#define tFRAME 289
#define tGLOBAL 290
#define tGO 291
#define tIF 292
#define tINTO 293
#define tLOOP 294
#define tMACRO 295
#define tMOVIE 296
#define tNEXT 297
#define tOF 298
#define tPREVIOUS 299
#define tPUT 300
#define tREPEAT 301
#define tSET 302
#define tTHEN 303
#define tTO 304
#define tWHEN 305
#define tWITH 306
#define tWHILE 307
#define tNLELSE 308
#define tFACTORY 309
#define tMETHOD 310
#define tOPEN 311
#define tPLAY 312
#define tDONE 313
#define tINSTANCE 314
#define tGE 315
#define tLE 316
#define tGT 317
#define tLT 318
#define tEQ 319
#define tNEQ 320
#define tAND 321
#define tOR 322
#define tNOT 323
#define tMOD 324
#define tAFTER 325
#define tBEFORE 326
#define tCONCAT 327
#define tCONTAINS 328
#define tSTARTS 329
#define tCHAR 330
#define tITEM 331
#define tLINE 332
#define tWORD 333
#define tSPRITE 334
#define tINTERSECTS 335
#define tWITHIN 336
#define tTELL 337
#define tON 338
#define tME 339




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
#line 304 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 317 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  116
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1602

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  100
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  42
/* YYNRULES -- Number of rules.  */
#define YYNRULES  155
/* YYNRULES -- Number of states.  */
#define YYNSTATES  341

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   339

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      92,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    91,    86,     2,
      93,    94,    89,    87,    99,    88,     2,    90,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      96,    85,    95,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    97,     2,    98,     2,     2,     2,     2,     2,     2,
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
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    14,    15,    17,    19,
      21,    26,    31,    36,    41,    46,    52,    57,    62,    68,
      70,    72,    74,    76,    84,    95,   107,   111,   118,   123,
     131,   142,   153,   160,   171,   182,   183,   187,   190,   192,
     195,   197,   204,   206,   213,   215,   219,   223,   226,   230,
     232,   234,   235,   236,   237,   240,   243,   247,   249,   251,
     253,   255,   257,   259,   262,   267,   269,   271,   274,   276,
     280,   284,   288,   292,   296,   300,   304,   308,   312,   316,
     320,   324,   327,   331,   335,   339,   343,   346,   349,   353,
     357,   362,   367,   372,   379,   384,   391,   396,   403,   408,
     415,   418,   420,   422,   425,   427,   430,   433,   435,   438,
     441,   443,   446,   451,   458,   463,   466,   470,   472,   476,
     478,   482,   485,   488,   491,   494,   498,   501,   504,   506,
     510,   513,   516,   519,   523,   526,   527,   531,   532,   541,
     544,   545,   554,   555,   556,   567,   568,   570,   574,   579,
     580,   583,   584,   586,   590,   592
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     101,     0,    -1,   101,   102,   103,    -1,   103,    -1,     1,
     102,    -1,    92,    -1,    -1,   132,    -1,   139,    -1,   106,
      -1,    45,   123,    38,    24,    -1,    45,   123,    70,   123,
      -1,    45,   123,    71,   123,    -1,    47,    24,    85,   123,
      -1,    47,    12,    85,   123,    -1,    47,    13,   123,    85,
     123,    -1,    47,    24,    49,   123,    -1,    47,    12,    49,
     123,    -1,    47,    13,   123,    49,   123,    -1,   123,    -1,
     124,    -1,   105,    -1,   107,    -1,   114,    93,   113,    94,
     120,   119,    28,    -1,   115,    85,   123,   119,    49,   123,
     119,   120,   119,    28,    -1,   115,    85,   123,   119,    30,
      49,   123,   119,   120,   119,    28,    -1,   121,   105,   119,
      -1,   122,   123,   102,   120,   119,    28,    -1,   122,   123,
      49,   123,    -1,   116,   113,    48,   102,   120,   119,    28,
      -1,   116,   113,    48,   102,   120,   119,    53,   120,   119,
      28,    -1,   116,   113,    48,   102,   120,   119,   118,   109,
     119,    28,    -1,   116,   113,    48,   118,   105,   119,    -1,
     116,   113,    48,   118,   105,   119,    53,   118,   105,   119,
      -1,   116,   113,    48,   118,   105,   119,   110,   119,   108,
     119,    -1,    -1,    53,   118,   105,    -1,   109,   112,    -1,
     112,    -1,   110,   111,    -1,   111,    -1,   117,   113,    48,
     118,   106,   119,    -1,   110,    -1,   117,   113,    48,   118,
     120,   119,    -1,   123,    -1,   123,    85,   123,    -1,    93,
     113,    94,    -1,    46,    52,    -1,    46,    51,    24,    -1,
      37,    -1,    32,    -1,    -1,    -1,    -1,   120,   102,    -1,
     120,   106,    -1,    50,    24,    48,    -1,    82,    -1,    11,
      -1,    14,    -1,    27,    -1,    25,    -1,    22,    -1,    23,
     123,    -1,    24,    93,   140,    94,    -1,    24,    -1,    12,
      -1,    13,   123,    -1,   104,    -1,   123,    87,   123,    -1,
     123,    88,   123,    -1,   123,    89,   123,    -1,   123,    90,
     123,    -1,   123,    69,   123,    -1,   123,    95,   123,    -1,
     123,    96,   123,    -1,   123,    65,   123,    -1,   123,    60,
     123,    -1,   123,    61,   123,    -1,   123,    66,   123,    -1,
     123,    67,   123,    -1,    68,   123,    -1,   123,    86,   123,
      -1,   123,    72,   123,    -1,   123,    73,   123,    -1,   123,
      74,   123,    -1,    87,   123,    -1,    88,   123,    -1,    93,
     123,    94,    -1,    97,   140,    98,    -1,    79,   123,    80,
     123,    -1,    79,   123,    81,   123,    -1,    75,   123,    43,
     123,    -1,    75,   123,    49,   123,    43,   123,    -1,    76,
     123,    43,   123,    -1,    76,   123,    49,   123,    43,   123,
      -1,    77,   123,    43,   123,    -1,    77,   123,    49,   123,
      43,   123,    -1,    78,   123,    43,   123,    -1,    78,   123,
      49,   123,    43,   123,    -1,    45,   123,    -1,   127,    -1,
     130,    -1,    33,    46,    -1,    33,    -1,    35,   125,    -1,
      59,   126,    -1,    16,    -1,    18,   123,    -1,    17,   123,
      -1,    17,    -1,    19,   141,    -1,    84,    93,    24,    94,
      -1,    84,    93,    24,    99,   140,    94,    -1,    56,   123,
      51,   123,    -1,    56,   123,    -1,    20,    24,   140,    -1,
      24,    -1,   125,    99,    24,    -1,    24,    -1,   126,    99,
      24,    -1,    36,    39,    -1,    36,    42,    -1,    36,    44,
      -1,    36,   128,    -1,    36,   128,   129,    -1,    36,   129,
      -1,    34,   123,    -1,   123,    -1,    43,    41,   123,    -1,
      41,   123,    -1,    57,    58,    -1,    57,   128,    -1,    57,
     128,   129,    -1,    57,   129,    -1,    -1,    29,   131,   140,
      -1,    -1,    40,    24,   133,   118,   137,   102,   138,   120,
      -1,    54,    24,    -1,    -1,    55,    24,   134,   118,   137,
     102,   138,   120,    -1,    -1,    -1,    83,    24,   135,   118,
     136,   137,   102,   138,   120,    28,    -1,    -1,    24,    -1,
     137,    99,    24,    -1,   137,   102,    99,    24,    -1,    -1,
      24,   141,    -1,    -1,   123,    -1,   140,    99,   123,    -1,
     123,    -1,   141,    99,   123,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   118,   118,   119,   120,   123,   128,   129,   130,   131,
     134,   140,   141,   142,   148,   156,   164,   170,   178,   188,
     189,   192,   193,   198,   211,   229,   243,   249,   252,   257,
     267,   279,   291,   301,   311,   323,   324,   327,   328,   331,
     332,   335,   343,   344,   352,   353,   354,   357,   360,   367,
     374,   382,   385,   388,   389,   390,   393,   399,   403,   404,
     407,   410,   413,   416,   419,   422,   426,   433,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     473,   474,   475,   476,   477,   479,   480,   481,   484,   487,
     490,   493,   494,   495,   496,   497,   498,   501,   502,   505,
     506,   517,   518,   519,   520,   523,   526,   531,   532,   535,
     536,   539,   540,   543,   546,   549,   549,   579,   579,   585,
     588,   588,   593,   594,   593,   604,   605,   606,   607,   610,
     614,   622,   623,   624,   627,   628
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
  "BLTINONEARG", "BLTINARGLIST", "TWOWORDBUILTIN", "FBLTIN",
  "FBLTINNOARGS", "FBLTINONEARG", "ID", "STRING", "HANDLER", "SYMBOL",
  "ENDCLAUSE", "tPLAYACCEL", "tDOWN", "tELSE", "tNLELSIF", "tEXIT",
  "tFRAME", "tGLOBAL", "tGO", "tIF", "tINTO", "tLOOP", "tMACRO", "tMOVIE",
  "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO",
  "tWHEN", "tWITH", "tWHILE", "tNLELSE", "tFACTORY", "tMETHOD", "tOPEN",
  "tPLAY", "tDONE", "tINSTANCE", "tGE", "tLE", "tGT", "tLT", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tON", "tME", "'='", "'&'", "'+'",
  "'-'", "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "'>'", "'<'", "'['",
  "']'", "','", "$accept", "program", "nl", "programline", "asgn",
  "stmtoneliner", "stmt", "ifstmt", "elsestmtoneliner", "elseifstmt",
  "elseifstmtoneliner", "elseifstmtoneliner1", "elseifstmt1", "cond",
  "repeatwhile", "repeatwith", "if", "elseif", "begin", "end", "stmtlist",
  "when", "tell", "expr", "proc", "globallist", "instancelist", "gotofunc",
  "gotoframe", "gotomovie", "playfunc", "@1", "defn", "@2", "@3", "@4",
  "@5", "argdef", "argstore", "macro", "arglist", "nonemptyarglist", 0
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
     335,   336,   337,   338,   339,    61,    38,    43,    45,    42,
      47,    37,    10,    40,    41,    62,    60,    91,    93,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   100,   101,   101,   101,   102,   103,   103,   103,   103,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   105,
     105,   106,   106,   106,   106,   106,   106,   106,   106,   107,
     107,   107,   107,   107,   107,   108,   108,   109,   109,   110,
     110,   111,   112,   112,   113,   113,   113,   114,   115,   116,
     117,   118,   119,   120,   120,   120,   121,   122,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   125,   125,   126,
     126,   127,   127,   127,   127,   127,   127,   128,   128,   129,
     129,   130,   130,   130,   130,   131,   130,   133,   132,   132,
     134,   132,   135,   136,   132,   137,   137,   137,   137,   138,
     139,   140,   140,   140,   141,   141
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     0,     1,     1,     1,
       4,     4,     4,     4,     4,     5,     4,     4,     5,     1,
       1,     1,     1,     7,    10,    11,     3,     6,     4,     7,
      10,    10,     6,    10,    10,     0,     3,     2,     1,     2,
       1,     6,     1,     6,     1,     3,     3,     2,     3,     1,
       1,     0,     0,     0,     2,     2,     3,     1,     1,     1,
       1,     1,     1,     2,     4,     1,     1,     2,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     3,     3,     3,     2,     2,     3,     3,
       4,     4,     4,     6,     4,     6,     4,     6,     4,     6,
       2,     1,     1,     2,     1,     2,     2,     1,     2,     2,
       1,     2,     4,     6,     4,     2,     3,     1,     3,     1,
       3,     2,     2,     2,     2,     3,     2,     2,     1,     3,
       2,     2,     2,     3,     2,     0,     3,     0,     8,     2,
       0,     8,     0,     0,    10,     0,     1,     3,     4,     0,
       2,     0,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    58,    66,     0,    59,   107,   110,     0,     0,
       0,    62,     0,    65,    61,    60,   135,   104,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    57,     0,     0,
       0,     0,     0,   151,     0,     3,    68,    21,     9,    22,
       0,     0,     0,     0,     0,    19,    20,   101,   102,     7,
       8,     5,     4,    65,     0,    67,   109,   108,   154,   111,
     151,    63,   151,   150,   151,   103,   117,   105,     0,   121,
       0,   122,     0,   123,   128,   124,   126,   137,   100,     0,
      47,     0,     0,     0,     0,   139,   140,   115,   131,   132,
     134,   119,   106,    81,     0,     0,     0,     0,     0,   142,
       0,    86,    87,     0,   152,     0,     1,     6,     0,     0,
       0,     0,    44,    52,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   151,     0,     0,   116,   152,     0,   136,     0,   127,
     130,     0,   125,    51,     0,     0,     0,    48,     0,     0,
       0,     0,     0,    56,    51,     0,   133,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
      88,    89,     0,     2,     0,    52,     0,     0,    51,     0,
      26,     0,    53,    77,    78,    76,    79,    80,    73,    83,
      84,    85,    82,    69,    70,    71,    72,    74,    75,   155,
      64,   118,   129,   145,    10,    11,    12,    17,    14,     0,
       0,    16,    13,   145,   114,   120,    92,     0,    94,     0,
      96,     0,    98,     0,    90,    91,   143,   112,   151,   153,
      53,     0,    46,    53,     0,    45,    28,    52,   146,     0,
      18,    15,     0,     0,     0,     0,     0,   145,     0,    52,
       0,     0,    52,    52,    54,    55,     0,     0,   149,   149,
      93,    95,    97,    99,     0,   113,     0,     0,    52,    51,
      32,    27,   147,     0,    53,    53,   149,    23,    52,    53,
      29,    53,     0,    50,    51,    52,    40,     0,   148,   138,
     141,    53,    53,    52,    52,    52,    42,    38,     0,     0,
      39,    35,     0,     0,    52,     0,     0,    37,     0,     0,
      52,    51,    52,    51,   144,     0,    24,    30,    31,    51,
      33,     0,    34,     0,    25,    53,    36,    52,    52,    41,
      43
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    44,   264,    45,    46,    47,   265,    49,   322,   305,
     306,   296,   307,   121,    50,    51,    52,   297,   213,   190,
     247,    53,    54,    55,    56,    77,   102,    57,    85,    86,
      58,    74,    59,   153,   164,   178,   257,   249,   284,    60,
     146,    69
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -278
static const yytype_int16 yypact[] =
{
     294,   -36,  -278,  -278,   902,  -278,  -278,   902,   902,   902,
       7,  -278,   902,   960,  -278,  -278,  -278,    18,    46,   807,
    -278,    54,   902,    52,    48,    63,    66,    68,   902,   865,
      70,   902,   902,   902,   902,   902,   902,  -278,    73,   -17,
     902,   902,   902,   902,     6,  -278,  -278,  -278,  -278,  -278,
       8,    14,   997,   729,   902,  1506,  -278,  -278,  -278,  -278,
    -278,  -278,  -278,    29,   902,  1506,  1506,  1506,  1506,    17,
     902,  1506,   902,    17,   902,  -278,  -278,    24,   902,  -278,
     902,  -278,    77,  -278,  1506,    25,  -278,  -278,  1026,   101,
    -278,   -45,   902,   -38,    95,  -278,  -278,  1346,  -278,    25,
    -278,  -278,    49,   -13,  1058,  1090,  1122,  1154,  1378,  -278,
     121,   -13,   -13,  1442,  1506,    12,  -278,   381,   997,   902,
     997,   102,  1474,  -278,   706,   902,   902,   902,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,   902,   902,
     902,   902,  1026,   902,    50,  1442,   -78,    50,   127,  1506,
    1506,   902,  -278,  -278,   130,   902,   902,  -278,   902,   902,
    1314,   902,   902,  -278,  -278,   902,  -278,   131,   902,   902,
     902,   902,   902,   902,   902,   902,   902,   902,  -278,   -77,
    -278,  -278,   902,  -278,    64,  1506,    67,  1410,   -36,   902,
    -278,   902,  -278,    19,    19,    19,   -13,   -13,   -13,  1506,
      19,    19,   197,   140,   140,   -13,   -13,  1506,  1506,  1506,
    -278,  -278,  1506,   141,  -278,  1506,  1506,  1506,  1506,   902,
     902,  1506,  1506,   141,  1506,  -278,  1506,  1186,  1506,  1218,
    1506,  1250,  1506,  1282,  1506,  1506,  -278,  -278,   902,  1506,
    -278,     9,  -278,  -278,   729,  1506,  1506,   555,  -278,   -73,
    1506,  1506,   -73,   902,   902,   902,   902,   141,   -10,   555,
     115,   902,   555,  -278,  -278,  -278,   139,   155,    82,    82,
    1506,  1506,  1506,  1506,   -73,  -278,   154,   902,  1506,    -3,
      -5,  -278,  -278,   160,  -278,  -278,    82,  -278,  1506,  -278,
    -278,  -278,   153,  -278,  -278,   153,  -278,   997,  -278,   555,
     555,  -278,  -278,   555,   555,   153,   153,  -278,   997,   729,
    -278,   134,   146,   468,   555,   161,   168,  -278,   171,   152,
    -278,  -278,  -278,  -278,  -278,   176,  -278,  -278,  -278,  -278,
    -278,   729,  -278,   642,  -278,   642,  -278,  -278,   555,  -278,
    -278
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -278,  -278,    23,    71,  -278,   -51,     0,  -278,  -278,  -278,
     -75,  -226,   -97,  -117,  -278,  -278,  -278,  -277,  -126,   -61,
    -189,  -278,  -278,     1,  -278,  -278,  -278,  -278,   181,   -22,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,  -211,  -212,  -278,
     -25,   201
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -7
static const yytype_int16 yytable[] =
{
      48,   184,   123,   186,   158,    65,   116,   100,    66,    67,
      68,   161,   252,    71,    68,   308,   210,   237,   115,    61,
      84,   182,   238,    88,    62,   290,   267,   293,   308,    97,
      84,    70,   103,   104,   105,   106,   107,   108,   223,   260,
     159,   111,   112,   113,   114,   144,   274,   162,   294,   147,
     291,   259,   236,   122,   262,   124,    61,   285,   261,   131,
      91,    92,   244,   152,    75,   142,    80,   117,    82,   310,
      76,   114,    93,   145,   301,   114,   110,   166,    87,   149,
     310,   150,   139,   140,   275,   128,   129,    94,   130,   182,
      95,   131,    96,   160,   101,   299,   300,   109,    61,   119,
     303,   118,   304,    89,    90,   134,   135,   136,   137,   138,
     181,   182,   313,   314,   139,   140,   143,    48,   151,   122,
     185,   187,   141,   148,   241,   157,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   114,   163,   209,   179,   338,   192,   167,   182,
     188,   211,   212,   292,   214,   225,   215,   216,   240,   217,
     218,   242,   221,   222,   277,   248,   224,   281,   309,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   282,
     312,   283,   287,   239,   298,   293,   266,   321,   183,   326,
     245,   319,   246,   263,   323,   331,   327,   333,   276,   328,
     329,   279,   280,   335,   334,   295,   128,   129,   317,   130,
      99,   243,   131,   258,    73,     0,     0,   289,     0,     0,
     250,   251,     0,     0,     0,     0,     0,   302,     0,   137,
     138,     0,     0,     0,   311,   139,   140,     0,     0,   114,
       0,     0,   315,   316,   318,     0,     0,     0,     0,     0,
       0,     0,     0,   325,   270,   271,   272,   273,   320,   330,
       0,   332,   278,   128,   129,     0,   130,     0,     0,   131,
       0,     0,   268,     0,     0,   269,   339,   340,   288,     0,
     336,     0,     0,     0,   135,   136,   137,   138,     0,     0,
       0,     0,   139,   140,    -6,     1,     0,   286,   122,     0,
       0,     0,     0,     0,     0,     2,     3,     4,     5,   122,
       6,     7,     8,     9,    10,     0,    11,    12,    13,    14,
       0,    15,     0,    16,     0,     0,     0,    17,     0,    18,
      19,    20,     0,   337,    21,   337,     0,     0,     0,    22,
      23,    24,     0,     0,    25,     0,     0,     0,    26,    27,
      28,    29,     0,    30,     0,     0,     0,     0,     0,     0,
       0,     0,    31,     0,     0,     0,     0,     0,     0,    32,
      33,    34,    35,    36,     0,     0,    37,    38,    39,     0,
       0,    40,    41,     0,     0,     0,    -6,    42,     0,     0,
       0,    43,     2,     3,     4,     5,     0,     6,     7,     8,
       9,    10,     0,    11,    12,    13,    14,     0,    15,     0,
      16,     0,     0,     0,    17,     0,    18,    19,    20,     0,
       0,    21,     0,     0,     0,     0,    22,    23,    24,     0,
       0,    25,     0,     0,     0,    26,    27,    28,    29,     0,
      30,     0,     0,     0,     0,     0,     0,     0,     0,    31,
       0,     0,     0,     0,     0,     0,    32,    33,    34,    35,
      36,     0,     0,    37,    38,    39,     0,     0,    40,    41,
       0,     0,     0,     0,    42,     0,     0,     0,    43,     2,
       3,     4,     5,     0,     6,     7,     8,     9,    10,     0,
      11,    12,    63,    14,     0,    15,   324,    16,     0,     0,
       0,    17,     0,    18,    19,    20,     0,     0,     0,     0,
       0,     0,     0,    22,    23,    24,     0,     0,    25,     0,
       0,     0,     0,     0,    28,    29,     0,    30,     0,     0,
       0,     0,     0,     0,     0,     0,    31,     0,     0,     0,
       0,     0,     0,    32,    33,    34,    35,    36,     0,     0,
      37,     0,    39,     0,     0,    40,    41,     0,     0,     0,
      61,    42,     0,     0,     0,    43,     2,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    63,
      14,     0,    15,     0,    16,     0,     0,     0,    17,     0,
      18,    19,    20,     0,     0,     0,     0,     0,     0,     0,
      22,    23,    24,     0,     0,    25,     0,     0,     0,     0,
       0,    28,    29,     0,    30,     0,     0,     0,     0,     0,
       0,     0,     0,    31,     0,     0,     0,     0,     0,     0,
      32,    33,    34,    35,    36,     0,     0,    37,     0,    39,
       0,     0,    40,    41,     0,     0,     0,    61,    42,     0,
       0,     0,    43,     2,     3,     4,     5,     0,     6,     7,
       8,     9,    10,     0,    11,    12,    63,    14,     0,    15,
       0,    16,     0,     0,     0,    17,     0,    18,    19,    20,
       0,     0,     0,     0,     0,     0,     0,    22,    23,    24,
       0,     0,    25,     0,     0,     0,     0,     0,    28,    29,
       0,    30,     0,     0,     0,     0,     0,     0,     0,     0,
      31,     0,     0,     0,     0,     0,     0,    32,    33,    34,
      35,    36,     0,     0,    37,     0,    39,     0,     0,    40,
      41,     0,     0,     0,     0,    42,     0,     0,     0,    43,
       2,     3,     4,     5,     0,     6,     7,     8,     9,    10,
       0,    11,    12,    63,    14,   191,    15,     0,    16,     0,
       0,     0,    17,     0,    18,    19,   125,   126,     0,     0,
       0,   127,   128,   129,    22,   130,    24,     0,   131,   132,
     133,     0,     0,     0,     0,    28,    29,     0,    30,     0,
       0,     0,   134,   135,   136,   137,   138,    31,    61,     0,
       0,   139,   140,     0,    32,    33,    34,    35,    36,     0,
       0,     0,     0,    39,     0,     0,    40,    41,     2,     3,
       4,     5,    42,     0,     0,     0,    43,     0,     0,    11,
      12,    63,    14,     0,    15,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,    79,     0,    80,    81,
      82,    83,    64,     0,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    31,     2,     3,     4,     5,
       0,     0,    32,    33,    34,    35,    36,    11,    12,    63,
      14,     0,    15,     0,    40,    41,     0,     0,     0,    78,
      42,     0,     0,     0,    43,     0,    80,     0,    82,     0,
      64,     0,    24,     2,     3,     4,     5,     0,     0,     0,
       0,     0,     0,    98,    11,    12,    63,    14,     0,    15,
       0,     0,     0,    31,     0,     0,     0,     0,     0,     0,
      32,    33,    34,    35,    36,     0,     0,    64,     0,    24,
       0,     0,    40,    41,     0,     0,     0,     0,    42,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
      31,     2,     3,     4,     5,     0,     0,    32,    33,    34,
      35,    36,    11,    12,    63,    14,     0,    15,     0,    40,
      41,     0,     0,     0,     0,    42,     0,     0,     0,    43,
       0,     0,     0,     0,     0,    64,     0,    24,     2,     3,
       4,     5,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    63,    14,     0,    15,     0,     0,     0,    31,     0,
       0,     0,     0,     0,     0,    32,    33,    34,    35,    36,
       0,     0,    64,     0,    24,     0,     0,    40,    41,     0,
       0,     0,     0,    72,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,   154,    31,     0,     0,     0,     0,
       0,     0,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,     0,     0,    40,    41,   125,   126,     0,     0,
     120,   127,   128,   129,    43,   130,   155,   156,   131,   132,
     133,   168,     0,     0,     0,     0,     0,   169,     0,     0,
       0,     0,   134,   135,   136,   137,   138,     0,   125,   126,
       0,   139,   140,   127,   128,   129,     0,   130,     0,     0,
     131,   132,   133,   170,     0,     0,     0,     0,     0,   171,
       0,     0,     0,     0,   134,   135,   136,   137,   138,     0,
     125,   126,     0,   139,   140,   127,   128,   129,     0,   130,
       0,     0,   131,   132,   133,   172,     0,     0,     0,     0,
       0,   173,     0,     0,     0,     0,   134,   135,   136,   137,
     138,     0,   125,   126,     0,   139,   140,   127,   128,   129,
       0,   130,     0,     0,   131,   132,   133,   174,     0,     0,
       0,     0,     0,   175,     0,     0,     0,     0,   134,   135,
     136,   137,   138,     0,   125,   126,     0,   139,   140,   127,
     128,   129,     0,   130,     0,     0,   131,   132,   133,   253,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     134,   135,   136,   137,   138,     0,   125,   126,     0,   139,
     140,   127,   128,   129,     0,   130,     0,     0,   131,   132,
     133,   254,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   134,   135,   136,   137,   138,     0,   125,   126,
       0,   139,   140,   127,   128,   129,     0,   130,     0,     0,
     131,   132,   133,   255,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   134,   135,   136,   137,   138,     0,
     125,   126,     0,   139,   140,   127,   128,   129,     0,   130,
       0,     0,   131,   132,   133,   256,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   134,   135,   136,   137,
     138,     0,   125,   126,     0,   139,   140,   127,   128,   129,
       0,   130,     0,     0,   131,   132,   133,     0,     0,     0,
       0,     0,     0,   219,     0,     0,     0,     0,   134,   135,
     136,   137,   138,     0,   125,   126,     0,   139,   140,   127,
     128,   129,     0,   130,     0,     0,   131,   132,   133,     0,
       0,     0,     0,     0,     0,     0,     0,   165,     0,   220,
     134,   135,   136,   137,   138,     0,   125,   126,     0,   139,
     140,   127,   128,   129,     0,   130,     0,     0,   131,   132,
     133,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   134,   135,   136,   137,   138,     0,   125,   126,
       0,   139,   140,   127,   128,   129,     0,   130,     0,     0,
     131,   132,   133,     0,     0,     0,     0,     0,   176,   177,
       0,     0,     0,     0,   134,   135,   136,   137,   138,     0,
     125,   126,     0,   139,   140,   127,   128,   129,     0,   130,
       0,     0,   131,   132,   133,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   189,   134,   135,   136,   137,
     138,     0,   125,   126,   180,   139,   140,   127,   128,   129,
       0,   130,     0,     0,   131,   132,   133,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   134,   135,
     136,   137,   138,     0,   125,   126,   180,   139,   140,   127,
     128,   129,     0,   130,     0,     0,   131,   132,   133,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   189,
     134,   135,   136,   137,   138,     0,   125,   126,     0,   139,
     140,   127,   128,   129,     0,   130,     0,     0,   131,   132,
     133,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   134,   135,   136,   137,   138,     0,     0,     0,
       0,   139,   140
};

static const yytype_int16 yycheck[] =
{
       0,   118,    53,   120,    49,     4,     0,    29,     7,     8,
       9,    49,   223,    12,    13,   292,    94,    94,    43,    92,
      19,    99,    99,    22,     1,    28,    99,    32,   305,    28,
      29,    24,    31,    32,    33,    34,    35,    36,   164,    30,
      85,    40,    41,    42,    43,    70,   257,    85,    53,    74,
      53,   240,   178,    52,   243,    54,    92,   269,    49,    72,
      12,    13,   188,    85,    46,    64,    41,    44,    43,   295,
      24,    70,    24,    72,   286,    74,    93,    99,    24,    78,
     306,    80,    95,    96,    94,    66,    67,    24,    69,    99,
      24,    72,    24,    92,    24,   284,   285,    24,    92,    85,
     289,    93,   291,    51,    52,    86,    87,    88,    89,    90,
      98,    99,   301,   302,    95,    96,    99,   117,    41,   118,
     119,   120,    93,    99,   185,    24,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,    48,   143,    24,   335,   124,    99,    99,
      48,    24,   151,   279,    24,    24,   155,   156,    94,   158,
     159,    94,   161,   162,    49,    24,   165,    28,   294,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,    24,
     297,    99,    28,   182,    24,    32,   247,    53,   117,    28,
     189,   308,   191,   244,    48,   321,    28,   323,   259,    28,
      48,   262,   263,   329,    28,   280,    66,    67,   305,    69,
      29,   188,    72,   238,    13,    -1,    -1,   278,    -1,    -1,
     219,   220,    -1,    -1,    -1,    -1,    -1,   288,    -1,    89,
      90,    -1,    -1,    -1,   295,    95,    96,    -1,    -1,   238,
      -1,    -1,   303,   304,   305,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   314,   253,   254,   255,   256,   309,   320,
      -1,   322,   261,    66,    67,    -1,    69,    -1,    -1,    72,
      -1,    -1,   249,    -1,    -1,   252,   337,   338,   277,    -1,
     331,    -1,    -1,    -1,    87,    88,    89,    90,    -1,    -1,
      -1,    -1,    95,    96,     0,     1,    -1,   274,   297,    -1,
      -1,    -1,    -1,    -1,    -1,    11,    12,    13,    14,   308,
      16,    17,    18,    19,    20,    -1,    22,    23,    24,    25,
      -1,    27,    -1,    29,    -1,    -1,    -1,    33,    -1,    35,
      36,    37,    -1,   333,    40,   335,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    50,    -1,    -1,    -1,    54,    55,
      56,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    84,    -1,
      -1,    87,    88,    -1,    -1,    -1,    92,    93,    -1,    -1,
      -1,    97,    11,    12,    13,    14,    -1,    16,    17,    18,
      19,    20,    -1,    22,    23,    24,    25,    -1,    27,    -1,
      29,    -1,    -1,    -1,    33,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    50,    -1,    -1,    -1,    54,    55,    56,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    -1,    -1,    87,    88,
      -1,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    11,
      12,    13,    14,    -1,    16,    17,    18,    19,    20,    -1,
      22,    23,    24,    25,    -1,    27,    28,    29,    -1,    -1,
      -1,    33,    -1,    35,    36,    37,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    56,    57,    -1,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,
      82,    -1,    84,    -1,    -1,    87,    88,    -1,    -1,    -1,
      92,    93,    -1,    -1,    -1,    97,    11,    12,    13,    14,
      -1,    16,    17,    18,    19,    20,    -1,    22,    23,    24,
      25,    -1,    27,    -1,    29,    -1,    -1,    -1,    33,    -1,
      35,    36,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    56,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    -1,    84,
      -1,    -1,    87,    88,    -1,    -1,    -1,    92,    93,    -1,
      -1,    -1,    97,    11,    12,    13,    14,    -1,    16,    17,
      18,    19,    20,    -1,    22,    23,    24,    25,    -1,    27,
      -1,    29,    -1,    -1,    -1,    33,    -1,    35,    36,    37,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,
      -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    -1,    84,    -1,    -1,    87,
      88,    -1,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,
      11,    12,    13,    14,    -1,    16,    17,    18,    19,    20,
      -1,    22,    23,    24,    25,    49,    27,    -1,    29,    -1,
      -1,    -1,    33,    -1,    35,    36,    60,    61,    -1,    -1,
      -1,    65,    66,    67,    45,    69,    47,    -1,    72,    73,
      74,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    68,    92,    -1,
      -1,    95,    96,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    84,    -1,    -1,    87,    88,    11,    12,
      13,    14,    93,    -1,    -1,    -1,    97,    -1,    -1,    22,
      23,    24,    25,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    34,    -1,    -1,    -1,    -1,    39,    -1,    41,    42,
      43,    44,    45,    -1,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    11,    12,    13,    14,
      -1,    -1,    75,    76,    77,    78,    79,    22,    23,    24,
      25,    -1,    27,    -1,    87,    88,    -1,    -1,    -1,    34,
      93,    -1,    -1,    -1,    97,    -1,    41,    -1,    43,    -1,
      45,    -1,    47,    11,    12,    13,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    58,    22,    23,    24,    25,    -1,    27,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    45,    -1,    47,
      -1,    -1,    87,    88,    -1,    -1,    -1,    -1,    93,    -1,
      -1,    -1,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    11,    12,    13,    14,    -1,    -1,    75,    76,    77,
      78,    79,    22,    23,    24,    25,    -1,    27,    -1,    87,
      88,    -1,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    47,    11,    12,
      13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    -1,    27,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    45,    -1,    47,    -1,    -1,    87,    88,    -1,
      -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    60,    61,    -1,    -1,
      93,    65,    66,    67,    97,    69,    70,    71,    72,    73,
      74,    43,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    -1,    60,    61,
      -1,    95,    96,    65,    66,    67,    -1,    69,    -1,    -1,
      72,    73,    74,    43,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,    -1,
      60,    61,    -1,    95,    96,    65,    66,    67,    -1,    69,
      -1,    -1,    72,    73,    74,    43,    -1,    -1,    -1,    -1,
      -1,    49,    -1,    -1,    -1,    -1,    86,    87,    88,    89,
      90,    -1,    60,    61,    -1,    95,    96,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    43,    -1,    -1,
      -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    -1,    60,    61,    -1,    95,    96,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    43,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    -1,    60,    61,    -1,    95,
      96,    65,    66,    67,    -1,    69,    -1,    -1,    72,    73,
      74,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    -1,    60,    61,
      -1,    95,    96,    65,    66,    67,    -1,    69,    -1,    -1,
      72,    73,    74,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,    -1,
      60,    61,    -1,    95,    96,    65,    66,    67,    -1,    69,
      -1,    -1,    72,    73,    74,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,
      90,    -1,    60,    61,    -1,    95,    96,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    -1,    60,    61,    -1,    95,    96,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    85,
      86,    87,    88,    89,    90,    -1,    60,    61,    -1,    95,
      96,    65,    66,    67,    -1,    69,    -1,    -1,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    -1,    60,    61,
      -1,    95,    96,    65,    66,    67,    -1,    69,    -1,    -1,
      72,    73,    74,    -1,    -1,    -1,    -1,    -1,    80,    81,
      -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,    -1,
      60,    61,    -1,    95,    96,    65,    66,    67,    -1,    69,
      -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    85,    86,    87,    88,    89,
      90,    -1,    60,    61,    94,    95,    96,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    -1,    60,    61,    94,    95,    96,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,
      86,    87,    88,    89,    90,    -1,    60,    61,    -1,    95,
      96,    65,    66,    67,    -1,    69,    -1,    -1,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    -1,    -1,    -1,
      -1,    95,    96
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    11,    12,    13,    14,    16,    17,    18,    19,
      20,    22,    23,    24,    25,    27,    29,    33,    35,    36,
      37,    40,    45,    46,    47,    50,    54,    55,    56,    57,
      59,    68,    75,    76,    77,    78,    79,    82,    83,    84,
      87,    88,    93,    97,   101,   103,   104,   105,   106,   107,
     114,   115,   116,   121,   122,   123,   124,   127,   130,   132,
     139,    92,   102,    24,    45,   123,   123,   123,   123,   141,
      24,   123,    93,   141,   131,    46,    24,   125,    34,    39,
      41,    42,    43,    44,   123,   128,   129,    24,   123,    51,
      52,    12,    13,    24,    24,    24,    24,   123,    58,   128,
     129,    24,   126,   123,   123,   123,   123,   123,   123,    24,
      93,   123,   123,   123,   123,   140,     0,   102,    93,    85,
      93,   113,   123,   105,   123,    60,    61,    65,    66,    67,
      69,    72,    73,    74,    86,    87,    88,    89,    90,    95,
      96,    93,   123,    99,   140,   123,   140,   140,    99,   123,
     123,    41,   129,   133,    38,    70,    71,    24,    49,    85,
     123,    49,    85,    48,   134,    51,   129,    99,    43,    49,
      43,    49,    43,    49,    43,    49,    80,    81,   135,    24,
      94,    98,    99,   103,   113,   123,   113,   123,    48,    85,
     119,    49,   102,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
      94,    24,   123,   118,    24,   123,   123,   123,   123,    49,
      85,   123,   123,   118,   123,    24,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   118,    94,    99,   123,
      94,   119,    94,   102,   118,   123,   123,   120,    24,   137,
     123,   123,   137,    43,    43,    43,    43,   136,   140,   120,
      30,    49,   120,   105,   102,   106,   119,    99,   102,   102,
     123,   123,   123,   123,   137,    94,   119,    49,   123,   119,
     119,    28,    24,    99,   138,   138,   102,    28,   123,   119,
      28,    53,   118,    32,    53,   110,   111,   117,    24,   120,
     120,   138,   119,   120,   120,   109,   110,   112,   117,   118,
     111,   119,   113,   120,   120,   119,   119,   112,   119,   113,
     105,    53,   108,    48,    28,   119,    28,    28,    28,    48,
     119,   118,   119,   118,    28,   118,   105,   106,   120,   119,
     119
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
#line 120 "engines/director/lingo/lingo-gr.y"
    { yyerrok; ;}
    break;

  case 5:
#line 123 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->_linenumber++;
		g_lingo->_colnumber = 1;
	;}
    break;

  case 10:
#line 134 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 11:
#line 140 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_after); ;}
    break;

  case 12:
#line 141 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_before); ;}
    break;

  case 13:
#line 142 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 14:
#line 148 "engines/director/lingo/lingo-gr.y"
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
#line 156 "engines/director/lingo/lingo-gr.y"
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
#line 164 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 17:
#line 170 "engines/director/lingo/lingo-gr.y"
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
#line 178 "engines/director/lingo/lingo-gr.y"
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
#line 188 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 23:
#line 198 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(5) - (7)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (7)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 2] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(7) - (7)].s), "repeat", true); ;}
    break;

  case 24:
#line 211 "engines/director/lingo/lingo-gr.y"
    {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[(3) - (10)].code));
		WRITE_UINT32(&finish, (yyvsp[(6) - (10)].code));
		WRITE_UINT32(&body, (yyvsp[(8) - (10)].code));
		WRITE_UINT32(&end, (yyvsp[(9) - (10)].code));
		WRITE_UINT32(&inc, 1);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 1] = init;	/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 2] = finish;/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 3] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 4] = inc;	/* increment */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 5] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(10) - (10)].s), "repeat", true); ;}
    break;

  case 25:
#line 229 "engines/director/lingo/lingo-gr.y"
    {
		inst init = 0, finish = 0, body = 0, end = 0, inc = 0;
		WRITE_UINT32(&init, (yyvsp[(3) - (11)].code));
		WRITE_UINT32(&finish, (yyvsp[(7) - (11)].code));
		WRITE_UINT32(&body, (yyvsp[(9) - (11)].code));
		WRITE_UINT32(&end, (yyvsp[(10) - (11)].code));
		WRITE_UINT32(&inc, -1);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 1] = init;	/* initial count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 2] = finish;/* final count value */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 3] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 4] = inc;	/* increment */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 5] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(11) - (11)].s), "repeat", true); ;}
    break;

  case 26:
#line 243 "engines/director/lingo/lingo-gr.y"
    {
			inst end = 0;
			WRITE_UINT32(&end, (yyvsp[(3) - (3)].code));
			g_lingo->code1(STOP);
			(*g_lingo->_currentScript)[(yyvsp[(1) - (3)].code) + 1] = end;
		;}
    break;

  case 27:
#line 249 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
			checkEnd((yyvsp[(6) - (6)].s), "tell", true); ;}
    break;

  case 28:
#line 252 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		;}
    break;

  case 29:
#line 257 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (7)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (7)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(7) - (7)].s), "if", true);

		g_lingo->processIf(0, 0); ;}
    break;

  case 30:
#line 267 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (10)].code));
		WRITE_UINT32(&else1, (yyvsp[(8) - (10)].code));
		WRITE_UINT32(&end, (yyvsp[(9) - (10)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(10) - (10)].s), "if", true);

		g_lingo->processIf(0, 0); ;}
    break;

  case 31:
#line 279 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (10)].code));
		WRITE_UINT32(&else1, (yyvsp[(7) - (10)].code));
		WRITE_UINT32(&end, (yyvsp[(9) - (10)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (10)].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(10) - (10)].s), "if", true);

		g_lingo->processIf(0, (yyvsp[(9) - (10)].code)); ;}
    break;

  case 32:
#line 291 "engines/director/lingo/lingo-gr.y"
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
#line 301 "engines/director/lingo/lingo-gr.y"
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
#line 311 "engines/director/lingo/lingo-gr.y"
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
#line 323 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 36:
#line 324 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 41:
#line 335 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 43:
#line 344 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 44:
#line 352 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 45:
#line 353 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 47:
#line 357 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 48:
#line 360 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 49:
#line 367 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 50:
#line 374 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 51:
#line 382 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 52:
#line 385 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 53:
#line 388 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 56:
#line 393 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 57:
#line 399 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_tellcode);
		g_lingo->code1(STOP); ;}
    break;

  case 58:
#line 403 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->codeConst((yyvsp[(1) - (1)].i)); ;}
    break;

  case 59:
#line 404 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_fconstpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 60:
#line 407 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 61:
#line 410 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 62:
#line 413 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 63:
#line 416 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 64:
#line 419 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 65:
#line 422 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 66:
#line 426 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 67:
#line 433 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 69:
#line 440 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 70:
#line 441 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 71:
#line 442 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 72:
#line 443 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 73:
#line 444 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); ;}
    break;

  case 74:
#line 445 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 75:
#line 446 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 76:
#line 447 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 77:
#line 448 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 78:
#line 449 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 79:
#line 450 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 80:
#line 451 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 81:
#line 452 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 82:
#line 453 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 83:
#line 454 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 84:
#line 455 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 85:
#line 456 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 86:
#line 457 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 87:
#line 458 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 88:
#line 459 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 89:
#line 460 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->codeArray((yyvsp[(2) - (3)].narg)); ;}
    break;

  case 90:
#line 461 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 91:
#line 462 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 92:
#line 463 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); ;}
    break;

  case 93:
#line 464 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); ;}
    break;

  case 94:
#line 465 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); ;}
    break;

  case 95:
#line 466 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); ;}
    break;

  case 96:
#line 467 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); ;}
    break;

  case 97:
#line 468 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); ;}
    break;

  case 98:
#line 469 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); ;}
    break;

  case 99:
#line 470 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); ;}
    break;

  case 100:
#line 473 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 103:
#line 476 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); ;}
    break;

  case 104:
#line 477 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeConst(0); // Push fake value on stack
							  g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 107:
#line 481 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 108:
#line 484 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 109:
#line 487 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 110:
#line 490 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code2(g_lingo->c_voidpush, g_lingo->_builtins[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 111:
#line 493 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 112:
#line 494 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 113:
#line 495 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].narg)); ;}
    break;

  case 114:
#line 496 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 115:
#line 497 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 116:
#line 498 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[(1) - (3)].s)); s += '-'; s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 117:
#line 501 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 118:
#line 502 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 119:
#line 505 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 120:
#line 506 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 121:
#line 517 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 122:
#line 518 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 123:
#line 519 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 124:
#line 520 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 125:
#line 523 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 126:
#line 526 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 131:
#line 539 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 132:
#line 540 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 133:
#line 543 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 134:
#line 546 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 135:
#line 549 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); ;}
    break;

  case 136:
#line 549 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[(1) - (3)].s), (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 137:
#line 579 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 138:
#line 580 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeConst(0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 139:
#line 585 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeFactory(*(yyvsp[(2) - (2)].s));
		;}
    break;

  case 140:
#line 588 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 141:
#line 589 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 142:
#line 593 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 143:
#line 594 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_ignoreMe = true; ;}
    break;

  case 144:
#line 594 "engines/director/lingo/lingo-gr.y"
    {
				g_lingo->codeConst(0); // Push fake value on stack
				g_lingo->code1(g_lingo->c_procret);
				g_lingo->define(*(yyvsp[(2) - (10)].s), (yyvsp[(4) - (10)].code), (yyvsp[(6) - (10)].narg));
				g_lingo->_indef = false;
				g_lingo->_ignoreMe = false;

				checkEnd((yyvsp[(10) - (10)].s), (yyvsp[(2) - (10)].s)->c_str(), false);
			;}
    break;

  case 145:
#line 604 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 146:
#line 605 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 147:
#line 606 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 148:
#line 607 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 149:
#line 610 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 150:
#line 614 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (2)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(2) - (2)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 151:
#line 622 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 152:
#line 623 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 153:
#line 624 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 154:
#line 627 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 155:
#line 628 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2967 "engines/director/lingo/lingo-gr.cpp"
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


#line 631 "engines/director/lingo/lingo-gr.y"


