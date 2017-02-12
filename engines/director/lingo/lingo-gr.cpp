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
     FBLTINARGLIST = 279,
     RBLTIN = 280,
     RBLTINONEARG = 281,
     ID = 282,
     STRING = 283,
     HANDLER = 284,
     SYMBOL = 285,
     ENDCLAUSE = 286,
     tPLAYACCEL = 287,
     tDOWN = 288,
     tELSE = 289,
     tNLELSIF = 290,
     tEXIT = 291,
     tFRAME = 292,
     tGLOBAL = 293,
     tGO = 294,
     tIF = 295,
     tINTO = 296,
     tLOOP = 297,
     tMACRO = 298,
     tMOVIE = 299,
     tNEXT = 300,
     tOF = 301,
     tPREVIOUS = 302,
     tPUT = 303,
     tREPEAT = 304,
     tSET = 305,
     tTHEN = 306,
     tTO = 307,
     tWHEN = 308,
     tWITH = 309,
     tWHILE = 310,
     tNLELSE = 311,
     tFACTORY = 312,
     tMETHOD = 313,
     tOPEN = 314,
     tPLAY = 315,
     tDONE = 316,
     tINSTANCE = 317,
     tGE = 318,
     tLE = 319,
     tGT = 320,
     tLT = 321,
     tEQ = 322,
     tNEQ = 323,
     tAND = 324,
     tOR = 325,
     tNOT = 326,
     tMOD = 327,
     tAFTER = 328,
     tBEFORE = 329,
     tCONCAT = 330,
     tCONTAINS = 331,
     tSTARTS = 332,
     tCHAR = 333,
     tITEM = 334,
     tLINE = 335,
     tWORD = 336,
     tSPRITE = 337,
     tINTERSECTS = 338,
     tWITHIN = 339,
     tTELL = 340,
     tPROPERTY = 341,
     tON = 342,
     tME = 343
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
#define FBLTINARGLIST 279
#define RBLTIN 280
#define RBLTINONEARG 281
#define ID 282
#define STRING 283
#define HANDLER 284
#define SYMBOL 285
#define ENDCLAUSE 286
#define tPLAYACCEL 287
#define tDOWN 288
#define tELSE 289
#define tNLELSIF 290
#define tEXIT 291
#define tFRAME 292
#define tGLOBAL 293
#define tGO 294
#define tIF 295
#define tINTO 296
#define tLOOP 297
#define tMACRO 298
#define tMOVIE 299
#define tNEXT 300
#define tOF 301
#define tPREVIOUS 302
#define tPUT 303
#define tREPEAT 304
#define tSET 305
#define tTHEN 306
#define tTO 307
#define tWHEN 308
#define tWITH 309
#define tWHILE 310
#define tNLELSE 311
#define tFACTORY 312
#define tMETHOD 313
#define tOPEN 314
#define tPLAY 315
#define tDONE 316
#define tINSTANCE 317
#define tGE 318
#define tLE 319
#define tGT 320
#define tLT 321
#define tEQ 322
#define tNEQ 323
#define tAND 324
#define tOR 325
#define tNOT 326
#define tMOD 327
#define tAFTER 328
#define tBEFORE 329
#define tCONCAT 330
#define tCONTAINS 331
#define tSTARTS 332
#define tCHAR 333
#define tITEM 334
#define tLINE 335
#define tWORD 336
#define tSPRITE 337
#define tINTERSECTS 338
#define tWITHIN 339
#define tTELL 340
#define tPROPERTY 341
#define tON 342
#define tME 343




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
#line 312 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 325 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  123
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1879

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  44
/* YYNRULES -- Number of rules.  */
#define YYNRULES  163
/* YYNRULES -- Number of states.  */
#define YYNSTATES  358

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   343

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      96,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    95,    90,     2,
      97,    98,    93,    91,   103,    92,     2,    94,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     100,    89,    99,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   101,     2,   102,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    14,    15,    17,    19,
      21,    26,    31,    36,    41,    46,    51,    57,    62,    67,
      73,    75,    77,    79,    81,    89,   100,   112,   116,   123,
     128,   136,   147,   158,   165,   176,   187,   188,   192,   195,
     197,   200,   202,   209,   211,   218,   220,   224,   228,   231,
     235,   237,   239,   240,   241,   242,   245,   248,   252,   254,
     256,   258,   260,   262,   264,   267,   270,   275,   280,   282,
     284,   287,   289,   293,   297,   301,   305,   309,   313,   317,
     321,   325,   329,   333,   337,   340,   344,   348,   352,   356,
     359,   362,   366,   370,   375,   380,   385,   392,   397,   404,
     409,   416,   421,   428,   431,   434,   436,   438,   441,   443,
     446,   449,   452,   454,   457,   460,   462,   465,   470,   475,
     482,   487,   490,   494,   496,   500,   502,   506,   508,   512,
     515,   518,   521,   524,   528,   531,   534,   536,   540,   543,
     546,   549,   553,   556,   557,   561,   562,   571,   574,   575,
     584,   585,   586,   597,   598,   600,   604,   609,   610,   613,
     614,   616,   620,   622
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     105,     0,    -1,   105,   106,   107,    -1,   107,    -1,     1,
     106,    -1,    96,    -1,    -1,   138,    -1,   145,    -1,   110,
      -1,    48,   127,    41,    27,    -1,    48,   127,    41,   128,
      -1,    48,   127,    73,   127,    -1,    48,   127,    74,   127,
      -1,    50,    27,    89,   127,    -1,    50,    12,    89,   127,
      -1,    50,    13,   127,    89,   127,    -1,    50,    27,    52,
     127,    -1,    50,    12,    52,   127,    -1,    50,    13,   127,
      52,   127,    -1,   127,    -1,   129,    -1,   109,    -1,   111,
      -1,   118,    97,   117,    98,   124,   123,    31,    -1,   119,
      89,   127,   123,    52,   127,   123,   124,   123,    31,    -1,
     119,    89,   127,   123,    33,    52,   127,   123,   124,   123,
      31,    -1,   125,   109,   123,    -1,   126,   127,   106,   124,
     123,    31,    -1,   126,   127,    52,   127,    -1,   120,   117,
      51,   106,   124,   123,    31,    -1,   120,   117,    51,   106,
     124,   123,    56,   124,   123,    31,    -1,   120,   117,    51,
     106,   124,   123,   122,   113,   123,    31,    -1,   120,   117,
      51,   122,   109,   123,    -1,   120,   117,    51,   122,   109,
     123,    56,   122,   109,   123,    -1,   120,   117,    51,   122,
     109,   123,   114,   123,   112,   123,    -1,    -1,    56,   122,
     109,    -1,   113,   116,    -1,   116,    -1,   114,   115,    -1,
     115,    -1,   121,   117,    51,   122,   110,   123,    -1,   114,
      -1,   121,   117,    51,   122,   124,   123,    -1,   127,    -1,
     127,    89,   127,    -1,    97,   117,    98,    -1,    49,    55,
      -1,    49,    54,    27,    -1,    40,    -1,    35,    -1,    -1,
      -1,    -1,   124,   106,    -1,   124,   110,    -1,    53,    27,
      51,    -1,    85,    -1,    11,    -1,    14,    -1,    30,    -1,
      28,    -1,    22,    -1,    23,   127,    -1,    24,   147,    -1,
      24,    97,   147,    98,    -1,    27,    97,   146,    98,    -1,
      27,    -1,    12,    -1,    13,   127,    -1,   108,    -1,   127,
      91,   127,    -1,   127,    92,   127,    -1,   127,    93,   127,
      -1,   127,    94,   127,    -1,   127,    72,   127,    -1,   127,
      99,   127,    -1,   127,   100,   127,    -1,   127,    68,   127,
      -1,   127,    63,   127,    -1,   127,    64,   127,    -1,   127,
      69,   127,    -1,   127,    70,   127,    -1,    71,   127,    -1,
     127,    90,   127,    -1,   127,    75,   127,    -1,   127,    76,
     127,    -1,   127,    77,   127,    -1,    91,   127,    -1,    92,
     127,    -1,    97,   127,    98,    -1,   101,   146,   102,    -1,
      82,   127,    83,   127,    -1,    82,   127,    84,   127,    -1,
      78,   127,    46,   127,    -1,    78,   127,    52,   127,    46,
     127,    -1,    79,   127,    46,   127,    -1,    79,   127,    52,
     127,    46,   127,    -1,    80,   127,    46,   127,    -1,    80,
     127,    52,   127,    46,   127,    -1,    81,   127,    46,   127,
      -1,    81,   127,    52,   127,    46,   127,    -1,    26,   127,
      -1,    48,   127,    -1,   133,    -1,   136,    -1,    36,    49,
      -1,    36,    -1,    38,   130,    -1,    86,   131,    -1,    62,
     132,    -1,    16,    -1,    18,   127,    -1,    17,   127,    -1,
      17,    -1,    19,   147,    -1,    19,    97,   147,    98,    -1,
      88,    97,    27,    98,    -1,    88,    97,    27,   103,   146,
      98,    -1,    59,   127,    54,   127,    -1,    59,   127,    -1,
      20,    27,   146,    -1,    27,    -1,   130,   103,    27,    -1,
      27,    -1,   131,   103,    27,    -1,    27,    -1,   132,   103,
      27,    -1,    39,    42,    -1,    39,    45,    -1,    39,    47,
      -1,    39,   134,    -1,    39,   134,   135,    -1,    39,   135,
      -1,    37,   127,    -1,   127,    -1,    46,    44,   127,    -1,
      44,   127,    -1,    60,    61,    -1,    60,   134,    -1,    60,
     134,   135,    -1,    60,   135,    -1,    -1,    32,   137,   146,
      -1,    -1,    43,    27,   139,   122,   143,   106,   144,   124,
      -1,    57,    27,    -1,    -1,    58,    27,   140,   122,   143,
     106,   144,   124,    -1,    -1,    -1,    87,    27,   141,   122,
     142,   143,   106,   144,   124,    31,    -1,    -1,    27,    -1,
     143,   103,    27,    -1,   143,   106,   103,    27,    -1,    -1,
      27,   147,    -1,    -1,   127,    -1,   146,   103,   127,    -1,
     127,    -1,   147,   103,   127,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   118,   118,   119,   120,   123,   128,   129,   130,   131,
     134,   140,   143,   144,   145,   151,   159,   167,   173,   181,
     191,   192,   195,   196,   201,   214,   232,   246,   252,   255,
     260,   270,   282,   294,   304,   314,   326,   327,   330,   331,
     334,   335,   338,   346,   347,   355,   356,   357,   360,   363,
     370,   377,   385,   388,   391,   392,   393,   396,   402,   406,
     407,   410,   413,   416,   419,   422,   423,   424,   427,   431,
     438,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   478,   483,   484,   485,   486,   487,   489,
     490,   491,   492,   495,   498,   501,   505,   506,   507,   508,
     509,   510,   511,   514,   515,   518,   519,   522,   523,   534,
     535,   536,   537,   540,   543,   548,   549,   552,   553,   556,
     557,   560,   563,   566,   566,   596,   596,   602,   605,   605,
     610,   611,   610,   621,   622,   623,   624,   627,   631,   639,
     640,   641,   644,   645
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
  "FBLTINNOARGS", "FBLTINONEARG", "FBLTINARGLIST", "RBLTIN",
  "RBLTINONEARG", "ID", "STRING", "HANDLER", "SYMBOL", "ENDCLAUSE",
  "tPLAYACCEL", "tDOWN", "tELSE", "tNLELSIF", "tEXIT", "tFRAME", "tGLOBAL",
  "tGO", "tIF", "tINTO", "tLOOP", "tMACRO", "tMOVIE", "tNEXT", "tOF",
  "tPREVIOUS", "tPUT", "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH",
  "tWHILE", "tNLELSE", "tFACTORY", "tMETHOD", "tOPEN", "tPLAY", "tDONE",
  "tINSTANCE", "tGE", "tLE", "tGT", "tLT", "tEQ", "tNEQ", "tAND", "tOR",
  "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS",
  "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE", "tINTERSECTS", "tWITHIN",
  "tTELL", "tPROPERTY", "tON", "tME", "'='", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'\\n'", "'('", "')'", "'>'", "'<'", "'['", "']'", "','",
  "$accept", "program", "nl", "programline", "asgn", "stmtoneliner",
  "stmt", "ifstmt", "elsestmtoneliner", "elseifstmt", "elseifstmtoneliner",
  "elseifstmtoneliner1", "elseifstmt1", "cond", "repeatwhile",
  "repeatwith", "if", "elseif", "begin", "end", "stmtlist", "when", "tell",
  "expr", "reference", "proc", "globallist", "propertylist",
  "instancelist", "gotofunc", "gotoframe", "gotomovie", "playfunc", "@1",
  "defn", "@2", "@3", "@4", "@5", "argdef", "argstore", "macro", "arglist",
  "nonemptyarglist", 0
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
     335,   336,   337,   338,   339,   340,   341,   342,   343,    61,
      38,    43,    45,    42,    47,    37,    10,    40,    41,    62,
      60,    91,    93,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   104,   105,   105,   105,   106,   107,   107,   107,   107,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     109,   109,   110,   110,   110,   110,   110,   110,   110,   110,
     111,   111,   111,   111,   111,   111,   112,   112,   113,   113,
     114,   114,   115,   116,   116,   117,   117,   117,   118,   119,
     120,   121,   122,   123,   124,   124,   124,   125,   126,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   128,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   130,   130,   131,   131,   132,   132,   133,
     133,   133,   133,   133,   133,   134,   134,   135,   135,   136,
     136,   136,   136,   137,   136,   139,   138,   138,   140,   138,
     141,   142,   138,   143,   143,   143,   143,   144,   145,   146,
     146,   146,   147,   147
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     0,     1,     1,     1,
       4,     4,     4,     4,     4,     4,     5,     4,     4,     5,
       1,     1,     1,     1,     7,    10,    11,     3,     6,     4,
       7,    10,    10,     6,    10,    10,     0,     3,     2,     1,
       2,     1,     6,     1,     6,     1,     3,     3,     2,     3,
       1,     1,     0,     0,     0,     2,     2,     3,     1,     1,
       1,     1,     1,     1,     2,     2,     4,     4,     1,     1,
       2,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     3,     3,     3,     3,     2,
       2,     3,     3,     4,     4,     4,     6,     4,     6,     4,
       6,     4,     6,     2,     2,     1,     1,     2,     1,     2,
       2,     2,     1,     2,     2,     1,     2,     4,     4,     6,
       4,     2,     3,     1,     3,     1,     3,     1,     3,     2,
       2,     2,     2,     3,     2,     2,     1,     3,     2,     2,
       2,     3,     2,     0,     3,     0,     8,     2,     0,     8,
       0,     0,    10,     0,     1,     3,     4,     0,     2,     0,
       1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    59,    69,     0,    60,   112,   115,     0,     0,
       0,    63,     0,     0,    68,    62,    61,   143,   108,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    58,     0,
       0,     0,     0,     0,     0,   159,     0,     3,    71,    22,
       9,    23,     0,     0,     0,     0,     0,    20,    21,   105,
     106,     7,     8,     5,     4,    68,     0,    70,   114,   113,
       0,   162,   116,   159,    64,     0,    65,   159,   158,   159,
     107,   123,   109,     0,   129,     0,   130,     0,   131,   136,
     132,   134,   145,   104,     0,    48,     0,     0,     0,     0,
     147,   148,   121,   139,   140,   142,   127,   111,    84,     0,
       0,     0,     0,     0,   125,   110,   150,     0,    89,    90,
       0,   160,     0,     1,     6,     0,     0,     0,     0,    45,
      53,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   159,     0,
     162,     0,     0,   122,     0,   160,     0,   144,     0,   135,
     138,     0,   133,    52,     0,     0,     0,    49,     0,     0,
       0,     0,     0,    57,    52,     0,   141,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    52,
       0,    91,    92,     0,     2,     0,    53,     0,     0,    52,
       0,    27,     0,    54,    80,    81,    79,    82,    83,    76,
      86,    87,    88,    85,    72,    73,    74,    75,    77,    78,
     117,   163,    66,    67,   124,   137,   153,     0,    10,    11,
      12,    13,    18,    15,     0,     0,    17,    14,   153,   120,
     128,    95,     0,    97,     0,    99,     0,   101,     0,    93,
      94,   126,   151,   118,   159,   161,    54,     0,    47,    54,
       0,    46,    29,    53,   154,     0,   103,    19,    16,     0,
       0,     0,     0,     0,   153,     0,    53,     0,     0,    53,
      53,    55,    56,     0,     0,   157,   157,    96,    98,   100,
     102,     0,   119,     0,     0,    53,    52,    33,    28,   155,
       0,    54,    54,   157,    24,    53,    54,    30,    54,     0,
      51,    52,    53,    41,     0,   156,   146,   149,    54,    54,
      53,    53,    53,    43,    39,     0,     0,    40,    36,     0,
       0,    53,     0,     0,    38,     0,     0,    53,    52,    53,
      52,   152,     0,    25,    31,    32,    52,    34,     0,    35,
       0,    26,    54,    37,    53,    53,    42,    44
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    46,   281,    47,    48,    49,   282,    51,   339,   322,
     323,   313,   324,   128,    52,    53,    54,   314,   226,   201,
     263,    55,    56,    57,   229,    58,    82,   115,   107,    59,
      90,    91,    60,    79,    61,   163,   174,   189,   274,   265,
     301,    62,   156,    72
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -275
static const yytype_int16 yypact[] =
{
     342,   -64,  -275,  -275,   991,  -275,  -275,   991,   991,  1063,
      33,  -275,   991,  1103,  1175,  -275,  -275,  -275,    15,    39,
     879,  -275,    43,   991,    61,    60,    52,    54,    66,   991,
     951,    68,   991,   991,   991,   991,   991,   991,  -275,    75,
      78,    14,   991,   991,   991,   991,     3,  -275,  -275,  -275,
    -275,  -275,    22,    28,  1215,   797,   991,  1779,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,    25,   991,  1779,  1779,  1779,
     991,  1779,    48,   991,  1779,   991,    48,   991,    48,   991,
    -275,  -275,    49,   991,  -275,   991,  -275,    81,  -275,  1779,
       5,  -275,  -275,  1250,   128,  -275,   -50,   991,   -41,   106,
    -275,  -275,  1614,  -275,     5,  -275,  -275,    53,   -74,  1283,
    1316,  1349,  1382,  1647,  -275,    55,  -275,   132,   -74,   -74,
    1713,  1779,    27,  -275,   433,  1215,   991,  1215,   109,  1746,
    -275,  1547,   991,   991,   991,   991,   991,   991,   991,   991,
     991,   991,   991,   991,   991,   991,   991,   991,   991,  1250,
    1713,    -9,   991,    58,    -6,  1713,    -2,    58,   136,  1779,
    1779,   991,  -275,  -275,   105,   991,   991,  -275,   991,   991,
    1581,   991,   991,  -275,  -275,   991,  -275,   137,   991,   991,
     991,   991,   991,   991,   991,   991,   991,   991,   138,  -275,
      11,  -275,  -275,   991,  -275,    70,  1779,    73,  1680,   -64,
     991,  -275,   991,  -275,   239,   239,   239,   -74,   -74,   -74,
    1779,   239,   239,   771,    13,    13,   -74,   -74,  1779,  1779,
    -275,  1779,  -275,  -275,  -275,  1779,   147,   991,  -275,  -275,
    1779,  1779,  1779,  1779,   991,   991,  1779,  1779,   147,  1779,
    -275,  1779,  1415,  1779,  1448,  1779,  1481,  1779,  1514,  1779,
    1779,  -275,  -275,  -275,   991,  1779,  -275,    -5,  -275,  -275,
     797,  1779,  1779,   615,  -275,   -34,  1779,  1779,  1779,   -34,
     991,   991,   991,   991,   147,    20,   615,   123,   991,   615,
    -275,  -275,  -275,   158,   151,    87,    87,  1779,  1779,  1779,
    1779,   -34,  -275,   160,   991,  1779,    -4,   -16,  -275,  -275,
     165,  -275,  -275,    87,  -275,  1779,  -275,  -275,  -275,   161,
    -275,  -275,   161,  -275,  1215,  -275,   615,   615,  -275,  -275,
     615,   615,   161,   161,  -275,  1215,   797,  -275,   139,   142,
     524,   615,   167,   168,  -275,   169,   153,  -275,  -275,  -275,
    -275,  -275,   171,  -275,  -275,  -275,  -275,  -275,   797,  -275,
     706,  -275,   706,  -275,  -275,   615,  -275,  -275
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -275,  -275,    19,    82,  -275,   -51,     0,  -275,  -275,  -275,
     -90,  -270,  -114,  -109,  -275,  -275,  -275,  -268,   -99,   -19,
    -198,  -275,  -275,     1,  -275,  -275,  -275,  -275,  -275,  -275,
     180,   -13,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -215,
    -274,  -275,   -23,    -7
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -7
static const yytype_int16 yytable[] =
{
      50,   138,   168,   123,   130,    67,    76,    78,    68,    69,
      71,   171,   302,    74,    71,    71,   195,   105,   197,   310,
      64,    89,   122,   269,    93,   146,   147,   307,   277,   318,
     102,    89,    63,   108,   109,   110,   111,   112,   113,   169,
     311,   325,   327,   118,   119,   120,   121,   278,   172,    85,
     153,    87,   308,   327,   325,   129,   157,   131,   276,   291,
      73,   279,    63,   151,    80,   124,    81,   149,   154,   284,
      92,   150,    96,    97,   121,   238,   150,   162,   155,    99,
     121,   100,   135,   136,   159,   137,   160,    98,   138,   220,
     252,   176,   222,   101,   152,   106,   223,   152,   170,    63,
     260,   193,   114,   316,   317,   116,   144,   145,   320,   253,
     321,   117,   146,   147,   254,    94,    95,   126,   292,   125,
     330,   331,   148,   193,    50,   161,   129,   196,   198,   192,
     193,   227,   228,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   121,
     203,   152,   158,   221,   355,   167,   177,   173,   188,   190,
     199,   193,   225,   224,   240,   251,   230,   231,   256,   232,
     233,   258,   236,   237,   264,   294,   239,   257,   299,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   298,
     300,   304,   315,   340,   255,   338,   310,   309,   343,   344,
     345,   261,   351,   262,   346,   329,   194,   312,   334,   280,
     104,     0,   326,     0,     0,     0,   336,     0,   259,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   266,     0,
       0,   275,     0,     0,     0,   267,   268,     0,     0,   348,
       0,   350,     0,     0,   283,     0,     0,   352,     0,     0,
       0,     0,     0,     0,     0,   121,     0,   293,     0,     0,
     296,   297,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   287,   288,   289,   290,   337,   306,     0,     0,   295,
       0,     0,     0,     0,   285,     0,   319,     0,   286,     0,
       0,     0,     0,   328,     0,   305,     0,   353,     0,     0,
       0,   332,   333,   335,     0,     0,     0,     0,   135,   136,
     303,   137,   342,     0,   138,   129,     0,     0,   347,     0,
     349,     0,     0,     0,     0,     0,   129,     0,     0,   141,
     142,   143,   144,   145,     0,   356,   357,     0,   146,   147,
       0,     0,    -6,     1,     0,     0,     0,     0,     0,     0,
     354,     0,   354,     2,     3,     4,     5,     0,     6,     7,
       8,     9,    10,     0,    11,    12,    13,     0,     0,    14,
      15,     0,    16,     0,    17,     0,     0,     0,    18,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,     0,
      23,    24,    25,     0,     0,    26,     0,     0,     0,    27,
      28,    29,    30,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,    32,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    36,    37,     0,     0,    38,    39,    40,
      41,     0,     0,    42,    43,     0,     0,     0,    -6,    44,
       0,     0,     0,    45,     2,     3,     4,     5,     0,     6,
       7,     8,     9,    10,     0,    11,    12,    13,     0,     0,
      14,    15,     0,    16,     0,    17,     0,     0,     0,    18,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
       0,    23,    24,    25,     0,     0,    26,     0,     0,     0,
      27,    28,    29,    30,     0,    31,     0,     0,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,    38,    39,
      40,    41,     0,     0,    42,    43,     0,     0,     0,     0,
      44,     0,     0,     0,    45,     2,     3,     4,     5,     0,
       6,     7,     8,     9,    10,     0,    11,    12,    13,     0,
       0,    65,    15,     0,    16,   341,    17,     0,     0,     0,
      18,     0,    19,    20,    21,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,    26,     0,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,     0,     0,    38,
      39,     0,    41,     0,     0,    42,    43,     0,     0,     0,
      63,    44,     0,     0,     0,    45,     2,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,     0,    65,    15,     0,    16,     0,    17,     0,     0,
       0,    18,     0,    19,    20,    21,     0,     0,     0,     0,
       0,     0,     0,    23,    24,    25,     0,     0,    26,     0,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,     0,     0,     0,     0,     0,    32,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,    37,     0,     0,
      38,    39,     0,    41,     0,     0,    42,    43,     0,     0,
       0,    63,    44,     0,     0,     0,    45,     2,     3,     4,
       5,     0,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,     0,    65,    15,     0,    16,     0,    17,     0,
       0,     0,    18,     0,    19,    20,    21,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,     0,     0,    26,
       0,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,     0,     0,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,    37,     0,
       0,    38,    39,     0,    41,     0,     0,    42,    43,     0,
       0,     0,     0,    44,     0,     0,     0,    45,     2,     3,
       4,     5,     0,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,     0,    65,    15,     0,    16,     0,    17,
       0,     0,     0,    18,     0,    19,    20,     0,     0,     0,
     135,   136,     0,   137,     0,    23,   138,    25,     0,     0,
       0,     0,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,   142,   143,   144,   145,     0,     0,    32,     0,
     146,   147,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,     0,    39,     0,    41,     0,     0,    42,    43,
       2,     3,     4,     5,    44,     0,     0,     0,    45,     0,
       0,    11,    12,    13,     0,     0,    65,    15,     0,    16,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,    84,     0,    85,    86,    87,    88,    66,     0,    25,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,    37,     2,     3,     4,     5,     0,     0,     0,     0,
      42,    43,     0,    11,    12,    13,    44,     0,    65,    15,
      45,    16,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,     0,     0,    85,     0,    87,     0,    66,
       0,    25,     2,     3,     4,     5,     0,     0,     0,     0,
       0,     0,   103,    11,    12,    13,     0,     0,    65,    15,
       0,    16,    32,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,    37,     0,     0,     0,     0,     0,    66,
       0,    25,    42,    43,     0,     0,     0,     0,    44,     0,
       0,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    32,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,    37,     2,     3,     4,     5,     0,     0,
       0,     0,    42,    43,     0,    11,    12,    13,    44,     0,
      65,    15,    45,    16,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    66,     0,    25,     2,     3,     4,     5,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
      65,    15,     0,    16,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     0,     0,     0,     0,
       0,    66,     0,    25,    42,    43,     0,     0,     0,     0,
      70,     0,     0,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,    37,     2,     3,     4,     5,
       0,     0,     0,     0,    42,    43,     0,    11,    12,    13,
      75,     0,    65,    15,    45,    16,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    66,     0,    25,     2,     3,     4,     5,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,    65,    15,     0,    16,    32,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,    37,     0,     0,
       0,     0,     0,    66,     0,    25,    42,    43,     0,     0,
       0,     0,    77,     0,     0,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    32,     0,     0,     0,
       0,   164,     0,    33,    34,    35,    36,    37,     0,     0,
       0,     0,     0,     0,     0,     0,    42,    43,     0,     0,
       0,     0,   127,   132,   133,     0,    45,     0,   134,   135,
     136,     0,   137,   165,   166,   138,   139,   140,     0,   178,
       0,     0,     0,     0,     0,   179,     0,     0,     0,     0,
     141,   142,   143,   144,   145,     0,   132,   133,     0,   146,
     147,   134,   135,   136,     0,   137,     0,     0,   138,   139,
     140,     0,   180,     0,     0,     0,     0,     0,   181,     0,
       0,     0,     0,   141,   142,   143,   144,   145,     0,   132,
     133,     0,   146,   147,   134,   135,   136,     0,   137,     0,
       0,   138,   139,   140,     0,   182,     0,     0,     0,     0,
       0,   183,     0,     0,     0,     0,   141,   142,   143,   144,
     145,     0,   132,   133,     0,   146,   147,   134,   135,   136,
       0,   137,     0,     0,   138,   139,   140,     0,   184,     0,
       0,     0,     0,     0,   185,     0,     0,     0,     0,   141,
     142,   143,   144,   145,     0,   132,   133,     0,   146,   147,
     134,   135,   136,     0,   137,     0,     0,   138,   139,   140,
       0,   270,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   141,   142,   143,   144,   145,     0,   132,   133,
       0,   146,   147,   134,   135,   136,     0,   137,     0,     0,
     138,   139,   140,     0,   271,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   141,   142,   143,   144,   145,
       0,   132,   133,     0,   146,   147,   134,   135,   136,     0,
     137,     0,     0,   138,   139,   140,     0,   272,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   141,   142,
     143,   144,   145,     0,   132,   133,     0,   146,   147,   134,
     135,   136,     0,   137,     0,     0,   138,   139,   140,     0,
     273,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   141,   142,   143,   144,   145,     0,   132,   133,     0,
     146,   147,   134,   135,   136,     0,   137,     0,     0,   138,
     139,   140,     0,     0,     0,     0,     0,     0,     0,   202,
       0,     0,     0,     0,   141,   142,   143,   144,   145,     0,
     132,   133,     0,   146,   147,   134,   135,   136,     0,   137,
       0,     0,   138,   139,   140,     0,     0,     0,     0,     0,
       0,     0,     0,   234,     0,     0,     0,   141,   142,   143,
     144,   145,     0,    63,   132,   133,   146,   147,     0,   134,
     135,   136,     0,   137,     0,     0,   138,   139,   140,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   175,     0,
     235,   141,   142,   143,   144,   145,     0,   132,   133,     0,
     146,   147,   134,   135,   136,     0,   137,     0,     0,   138,
     139,   140,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   141,   142,   143,   144,   145,     0,
     132,   133,     0,   146,   147,   134,   135,   136,     0,   137,
       0,     0,   138,   139,   140,     0,     0,     0,     0,     0,
     186,   187,     0,     0,     0,     0,     0,   141,   142,   143,
     144,   145,     0,   132,   133,     0,   146,   147,   134,   135,
     136,     0,   137,     0,     0,   138,   139,   140,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   200,
     141,   142,   143,   144,   145,     0,   132,   133,   191,   146,
     147,   134,   135,   136,     0,   137,     0,     0,   138,   139,
     140,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   141,   142,   143,   144,   145,     0,   132,
     133,   191,   146,   147,   134,   135,   136,     0,   137,     0,
       0,   138,   139,   140,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   200,   141,   142,   143,   144,
     145,     0,   132,   133,     0,   146,   147,   134,   135,   136,
       0,   137,     0,     0,   138,   139,   140,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   141,
     142,   143,   144,   145,     0,     0,     0,     0,   146,   147
};

static const yytype_int16 yycheck[] =
{
       0,    75,    52,     0,    55,     4,    13,    14,     7,     8,
       9,    52,   286,    12,    13,    14,   125,    30,   127,    35,
       1,    20,    45,   238,    23,    99,   100,    31,    33,   303,
      29,    30,    96,    32,    33,    34,    35,    36,    37,    89,
      56,   309,   312,    42,    43,    44,    45,    52,    89,    44,
      73,    46,    56,   323,   322,    54,    79,    56,   256,   274,
      27,   259,    96,    70,    49,    46,    27,    66,    75,   103,
      27,    70,    12,    13,    73,   174,    75,    90,    77,    27,
      79,    27,    69,    70,    83,    72,    85,    27,    75,    98,
     189,   104,    98,    27,   103,    27,    98,   103,    97,    96,
     199,   103,    27,   301,   302,    27,    93,    94,   306,    98,
     308,    97,    99,   100,   103,    54,    55,    89,    98,    97,
     318,   319,    97,   103,   124,    44,   125,   126,   127,   102,
     103,    26,    27,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     131,   103,   103,   152,   352,    27,   103,    51,   103,    27,
      51,   103,   161,    27,    27,    27,   165,   166,    98,   168,
     169,    98,   171,   172,    27,    52,   175,   196,    27,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,    31,
     103,    31,    27,    51,   193,    56,    35,   296,    31,    31,
      31,   200,    31,   202,    51,   314,   124,   297,   322,   260,
      30,    -1,   311,    -1,    -1,    -1,   325,    -1,   199,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   227,    -1,
      -1,   254,    -1,    -1,    -1,   234,   235,    -1,    -1,   338,
      -1,   340,    -1,    -1,   263,    -1,    -1,   346,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   254,    -1,   276,    -1,    -1,
     279,   280,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   270,   271,   272,   273,   326,   295,    -1,    -1,   278,
      -1,    -1,    -1,    -1,   265,    -1,   305,    -1,   269,    -1,
      -1,    -1,    -1,   312,    -1,   294,    -1,   348,    -1,    -1,
      -1,   320,   321,   322,    -1,    -1,    -1,    -1,    69,    70,
     291,    72,   331,    -1,    75,   314,    -1,    -1,   337,    -1,
     339,    -1,    -1,    -1,    -1,    -1,   325,    -1,    -1,    90,
      91,    92,    93,    94,    -1,   354,   355,    -1,    99,   100,
      -1,    -1,     0,     1,    -1,    -1,    -1,    -1,    -1,    -1,
     350,    -1,   352,    11,    12,    13,    14,    -1,    16,    17,
      18,    19,    20,    -1,    22,    23,    24,    -1,    -1,    27,
      28,    -1,    30,    -1,    32,    -1,    -1,    -1,    36,    -1,
      38,    39,    40,    -1,    -1,    43,    -1,    -1,    -1,    -1,
      48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,    57,
      58,    59,    60,    -1,    62,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    -1,    -1,    85,    86,    87,
      88,    -1,    -1,    91,    92,    -1,    -1,    -1,    96,    97,
      -1,    -1,    -1,   101,    11,    12,    13,    14,    -1,    16,
      17,    18,    19,    20,    -1,    22,    23,    24,    -1,    -1,
      27,    28,    -1,    30,    -1,    32,    -1,    -1,    -1,    36,
      -1,    38,    39,    40,    -1,    -1,    43,    -1,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,
      57,    58,    59,    60,    -1,    62,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    -1,    -1,    85,    86,
      87,    88,    -1,    -1,    91,    92,    -1,    -1,    -1,    -1,
      97,    -1,    -1,    -1,   101,    11,    12,    13,    14,    -1,
      16,    17,    18,    19,    20,    -1,    22,    23,    24,    -1,
      -1,    27,    28,    -1,    30,    31,    32,    -1,    -1,    -1,
      36,    -1,    38,    39,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    59,    60,    -1,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    -1,    -1,    85,
      86,    -1,    88,    -1,    -1,    91,    92,    -1,    -1,    -1,
      96,    97,    -1,    -1,    -1,   101,    11,    12,    13,    14,
      -1,    16,    17,    18,    19,    20,    -1,    22,    23,    24,
      -1,    -1,    27,    28,    -1,    30,    -1,    32,    -1,    -1,
      -1,    36,    -1,    38,    39,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    49,    50,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    59,    60,    -1,    62,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    -1,    -1,
      85,    86,    -1,    88,    -1,    -1,    91,    92,    -1,    -1,
      -1,    96,    97,    -1,    -1,    -1,   101,    11,    12,    13,
      14,    -1,    16,    17,    18,    19,    20,    -1,    22,    23,
      24,    -1,    -1,    27,    28,    -1,    30,    -1,    32,    -1,
      -1,    -1,    36,    -1,    38,    39,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    59,    60,    -1,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    -1,
      -1,    85,    86,    -1,    88,    -1,    -1,    91,    92,    -1,
      -1,    -1,    -1,    97,    -1,    -1,    -1,   101,    11,    12,
      13,    14,    -1,    16,    17,    18,    19,    20,    -1,    22,
      23,    24,    -1,    -1,    27,    28,    -1,    30,    -1,    32,
      -1,    -1,    -1,    36,    -1,    38,    39,    -1,    -1,    -1,
      69,    70,    -1,    72,    -1,    48,    75,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    -1,    62,
      -1,    -1,    91,    92,    93,    94,    -1,    -1,    71,    -1,
      99,   100,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      -1,    -1,    -1,    86,    -1,    88,    -1,    -1,    91,    92,
      11,    12,    13,    14,    97,    -1,    -1,    -1,   101,    -1,
      -1,    22,    23,    24,    -1,    -1,    27,    28,    -1,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    37,    -1,    -1,    -1,
      -1,    42,    -1,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      91,    92,    -1,    22,    23,    24,    97,    -1,    27,    28,
     101,    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    -1,    46,    -1,    48,
      -1,    50,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    61,    22,    23,    24,    -1,    -1,    27,    28,
      -1,    30,    71,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    91,    92,    -1,    -1,    -1,    -1,    97,    -1,
      -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    91,    92,    -1,    22,    23,    24,    97,    -1,
      27,    28,   101,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    11,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    23,    24,    -1,    -1,
      27,    28,    -1,    30,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    91,    92,    -1,    -1,    -1,    -1,
      97,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    11,    12,    13,    14,
      -1,    -1,    -1,    -1,    91,    92,    -1,    22,    23,    24,
      97,    -1,    27,    28,   101,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    11,    12,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,
      -1,    -1,    27,    28,    -1,    30,    71,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    91,    92,    -1,    -1,
      -1,    -1,    97,    -1,    -1,    -1,   101,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,
      -1,    41,    -1,    78,    79,    80,    81,    82,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    -1,    -1,
      -1,    -1,    97,    63,    64,    -1,   101,    -1,    68,    69,
      70,    -1,    72,    73,    74,    75,    76,    77,    -1,    46,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      90,    91,    92,    93,    94,    -1,    63,    64,    -1,    99,
     100,    68,    69,    70,    -1,    72,    -1,    -1,    75,    76,
      77,    -1,    46,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    -1,    63,
      64,    -1,    99,   100,    68,    69,    70,    -1,    72,    -1,
      -1,    75,    76,    77,    -1,    46,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    90,    91,    92,    93,
      94,    -1,    63,    64,    -1,    99,   100,    68,    69,    70,
      -1,    72,    -1,    -1,    75,    76,    77,    -1,    46,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    -1,    63,    64,    -1,    99,   100,
      68,    69,    70,    -1,    72,    -1,    -1,    75,    76,    77,
      -1,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    -1,    63,    64,
      -1,    99,   100,    68,    69,    70,    -1,    72,    -1,    -1,
      75,    76,    77,    -1,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,
      -1,    63,    64,    -1,    99,   100,    68,    69,    70,    -1,
      72,    -1,    -1,    75,    76,    77,    -1,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    -1,    63,    64,    -1,    99,   100,    68,
      69,    70,    -1,    72,    -1,    -1,    75,    76,    77,    -1,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    90,    91,    92,    93,    94,    -1,    63,    64,    -1,
      99,   100,    68,    69,    70,    -1,    72,    -1,    -1,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    -1,
      63,    64,    -1,    99,   100,    68,    69,    70,    -1,    72,
      -1,    -1,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    90,    91,    92,
      93,    94,    -1,    96,    63,    64,    99,   100,    -1,    68,
      69,    70,    -1,    72,    -1,    -1,    75,    76,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      89,    90,    91,    92,    93,    94,    -1,    63,    64,    -1,
      99,   100,    68,    69,    70,    -1,    72,    -1,    -1,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    -1,
      63,    64,    -1,    99,   100,    68,    69,    70,    -1,    72,
      -1,    -1,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,
      93,    94,    -1,    63,    64,    -1,    99,   100,    68,    69,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    -1,    63,    64,    98,    99,
     100,    68,    69,    70,    -1,    72,    -1,    -1,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    -1,    63,
      64,    98,    99,   100,    68,    69,    70,    -1,    72,    -1,
      -1,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    -1,    63,    64,    -1,    99,   100,    68,    69,    70,
      -1,    72,    -1,    -1,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    -1,    99,   100
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    11,    12,    13,    14,    16,    17,    18,    19,
      20,    22,    23,    24,    27,    28,    30,    32,    36,    38,
      39,    40,    43,    48,    49,    50,    53,    57,    58,    59,
      60,    62,    71,    78,    79,    80,    81,    82,    85,    86,
      87,    88,    91,    92,    97,   101,   105,   107,   108,   109,
     110,   111,   118,   119,   120,   125,   126,   127,   129,   133,
     136,   138,   145,    96,   106,    27,    48,   127,   127,   127,
      97,   127,   147,    27,   127,    97,   147,    97,   147,   137,
      49,    27,   130,    37,    42,    44,    45,    46,    47,   127,
     134,   135,    27,   127,    54,    55,    12,    13,    27,    27,
      27,    27,   127,    61,   134,   135,    27,   132,   127,   127,
     127,   127,   127,   127,    27,   131,    27,    97,   127,   127,
     127,   127,   146,     0,   106,    97,    89,    97,   117,   127,
     109,   127,    63,    64,    68,    69,    70,    72,    75,    76,
      77,    90,    91,    92,    93,    94,    99,   100,    97,   127,
     127,   147,   103,   146,   147,   127,   146,   146,   103,   127,
     127,    44,   135,   139,    41,    73,    74,    27,    52,    89,
     127,    52,    89,    51,   140,    54,   135,   103,    46,    52,
      46,    52,    46,    52,    46,    52,    83,    84,   103,   141,
      27,    98,   102,   103,   107,   117,   127,   117,   127,    51,
      89,   123,    52,   106,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
      98,   127,    98,    98,    27,   127,   122,    26,    27,   128,
     127,   127,   127,   127,    52,    89,   127,   127,   122,   127,
      27,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,    27,   122,    98,   103,   127,    98,   123,    98,   106,
     122,   127,   127,   124,    27,   143,   127,   127,   127,   143,
      46,    46,    46,    46,   142,   146,   124,    33,    52,   124,
     109,   106,   110,   123,   103,   106,   106,   127,   127,   127,
     127,   143,    98,   123,    52,   127,   123,   123,    31,    27,
     103,   144,   144,   106,    31,   127,   123,    31,    56,   122,
      35,    56,   114,   115,   121,    27,   124,   124,   144,   123,
     124,   124,   113,   114,   116,   121,   122,   115,   123,   117,
     124,   124,   123,   123,   116,   123,   117,   109,    56,   112,
      51,    31,   123,    31,    31,    31,    51,   123,   122,   123,
     122,    31,   122,   109,   110,   124,   123,   123
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
    {
			g_lingo->code1(g_lingo->c_assign);
			(yyval.code) = (yyvsp[(2) - (4)].code); ;}
    break;

  case 12:
#line 143 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_after); ;}
    break;

  case 13:
#line 144 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_before); ;}
    break;

  case 14:
#line 145 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 15:
#line 151 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (4)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (4)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 16:
#line 159 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (5)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (5)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 17:
#line 167 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 18:
#line 173 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (4)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (4)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 19:
#line 181 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(2) - (5)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(2) - (5)].e)[1]);
		g_lingo->code2(e, f);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 20:
#line 191 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_xpop); ;}
    break;

  case 24:
#line 201 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(5) - (7)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (7)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 2] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(7) - (7)].s), "repeat", true); ;}
    break;

  case 25:
#line 214 "engines/director/lingo/lingo-gr.y"
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

  case 26:
#line 232 "engines/director/lingo/lingo-gr.y"
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

  case 27:
#line 246 "engines/director/lingo/lingo-gr.y"
    {
			inst end = 0;
			WRITE_UINT32(&end, (yyvsp[(3) - (3)].code));
			g_lingo->code1(STOP);
			(*g_lingo->_currentScript)[(yyvsp[(1) - (3)].code) + 1] = end;
		;}
    break;

  case 28:
#line 252 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
			checkEnd((yyvsp[(6) - (6)].s), "tell", true); ;}
    break;

  case 29:
#line 255 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		;}
    break;

  case 30:
#line 260 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (7)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (7)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(7) - (7)].s), "if", true);

		g_lingo->processIf(0, 0); ;}
    break;

  case 31:
#line 270 "engines/director/lingo/lingo-gr.y"
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

  case 32:
#line 282 "engines/director/lingo/lingo-gr.y"
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

  case 33:
#line 294 "engines/director/lingo/lingo-gr.y"
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

  case 34:
#line 304 "engines/director/lingo/lingo-gr.y"
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

  case 35:
#line 314 "engines/director/lingo/lingo-gr.y"
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

  case 36:
#line 326 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 37:
#line 327 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 42:
#line 338 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 44:
#line 347 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 45:
#line 355 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 46:
#line 356 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 48:
#line 360 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 49:
#line 363 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 50:
#line 370 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 51:
#line 377 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 52:
#line 385 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 53:
#line 388 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 54:
#line 391 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 57:
#line 396 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 58:
#line 402 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_tellcode);
		g_lingo->code1(STOP); ;}
    break;

  case 59:
#line 406 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->codeConst((yyvsp[(1) - (1)].i)); ;}
    break;

  case 60:
#line 407 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_fconstpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 61:
#line 410 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 62:
#line 413 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 63:
#line 416 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 64:
#line 419 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 65:
#line 422 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 66:
#line 423 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg)); ;}
    break;

  case 67:
#line 424 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 68:
#line 427 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 69:
#line 431 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 70:
#line 438 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 72:
#line 445 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 73:
#line 446 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 74:
#line 447 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 75:
#line 448 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 76:
#line 449 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); ;}
    break;

  case 77:
#line 450 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 78:
#line 451 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 79:
#line 452 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 80:
#line 453 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 81:
#line 454 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 82:
#line 455 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 83:
#line 456 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 84:
#line 457 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 85:
#line 458 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 86:
#line 459 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 87:
#line 460 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 88:
#line 461 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 89:
#line 462 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 90:
#line 463 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 91:
#line 464 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 92:
#line 465 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->codeArray((yyvsp[(2) - (3)].narg)); ;}
    break;

  case 93:
#line 466 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 94:
#line 467 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 95:
#line 468 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); ;}
    break;

  case 96:
#line 469 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); ;}
    break;

  case 97:
#line 470 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); ;}
    break;

  case 98:
#line 471 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); ;}
    break;

  case 99:
#line 472 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); ;}
    break;

  case 100:
#line 473 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); ;}
    break;

  case 101:
#line 474 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); ;}
    break;

  case 102:
#line 475 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); ;}
    break;

  case 103:
#line 478 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 104:
#line 483 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 107:
#line 486 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); ;}
    break;

  case 108:
#line 487 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeConst(0); // Push fake value on stack
							  g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 112:
#line 492 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 113:
#line 495 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 114:
#line 498 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 115:
#line 501 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_voidpush);
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 1);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 116:
#line 505 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 117:
#line 506 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg)); ;}
    break;

  case 118:
#line 507 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 119:
#line 508 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].narg)); ;}
    break;

  case 120:
#line 509 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 121:
#line 510 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 122:
#line 511 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[(1) - (3)].s)); s += '-'; s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 123:
#line 514 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 124:
#line 515 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 125:
#line 518 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 126:
#line 519 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 127:
#line 522 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 128:
#line 523 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 129:
#line 534 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 130:
#line 535 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 131:
#line 536 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 132:
#line 537 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 133:
#line 540 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 134:
#line 543 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 139:
#line 556 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 140:
#line 557 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 141:
#line 560 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 142:
#line 563 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 143:
#line 566 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); ;}
    break;

  case 144:
#line 566 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[(1) - (3)].s), (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 145:
#line 596 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 146:
#line 597 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeConst(0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 147:
#line 602 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeFactory(*(yyvsp[(2) - (2)].s));
		;}
    break;

  case 148:
#line 605 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 149:
#line 606 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 150:
#line 610 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 151:
#line 611 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_ignoreMe = true; ;}
    break;

  case 152:
#line 611 "engines/director/lingo/lingo-gr.y"
    {
				g_lingo->codeConst(0); // Push fake value on stack
				g_lingo->code1(g_lingo->c_procret);
				g_lingo->define(*(yyvsp[(2) - (10)].s), (yyvsp[(4) - (10)].code), (yyvsp[(6) - (10)].narg));
				g_lingo->_indef = false;
				g_lingo->_ignoreMe = false;

				checkEnd((yyvsp[(10) - (10)].s), (yyvsp[(2) - (10)].s)->c_str(), false);
			;}
    break;

  case 153:
#line 621 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 154:
#line 622 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 155:
#line 623 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 156:
#line 624 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 157:
#line 627 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 158:
#line 631 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (2)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(2) - (2)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 159:
#line 639 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 160:
#line 640 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 161:
#line 641 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 162:
#line 644 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 163:
#line 645 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3083 "engines/director/lingo/lingo-gr.cpp"
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


#line 648 "engines/director/lingo/lingo-gr.y"


