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
     tMETHOD = 291,
     tDOWN = 292,
     tELSE = 293,
     tELSIF = 294,
     tEXIT = 295,
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
     tOPEN = 316,
     tPLAY = 317,
     tDONE = 318,
     tINSTANCE = 319,
     tGE = 320,
     tLE = 321,
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
     tME = 343,
     tENDIF = 344,
     tENDREPEAT = 345,
     tENDTELL = 346
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
#define tMETHOD 291
#define tDOWN 292
#define tELSE 293
#define tELSIF 294
#define tEXIT 295
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
#define tOPEN 316
#define tPLAY 317
#define tDONE 318
#define tINSTANCE 319
#define tGE 320
#define tLE 321
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
#define tENDIF 344
#define tENDREPEAT 345
#define tENDTELL 346




/* Copy the first part of user declarations.  */
#line 52 "engines/director/lingo/lingo-gr.y"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/endian.h"
#include "common/hash-str.h"
#include "common/rect.h"

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
#line 84 "engines/director/lingo/lingo-gr.y"
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
#line 320 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 333 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  124
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1612

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  41
/* YYNRULES -- Number of rules.  */
#define YYNRULES  158
/* YYNRULES -- Number of states.  */
#define YYNSTATES  339

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   346

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     100,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    99,    94,     2,
     101,   102,    97,    95,   105,    96,     2,    98,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      92,     2,    93,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   103,     2,   104,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    13,    15,    17,    22,
      27,    32,    37,    42,    47,    52,    57,    63,    69,    71,
      73,    75,    77,    79,    81,    83,    85,    87,    94,   105,
     117,   121,   128,   133,   143,   156,   157,   160,   167,   179,
     187,   190,   194,   196,   198,   199,   200,   201,   204,   207,
     211,   213,   215,   217,   219,   221,   223,   225,   227,   229,
     232,   235,   240,   242,   245,   247,   251,   255,   259,   263,
     267,   271,   275,   279,   283,   287,   291,   295,   299,   302,
     306,   310,   314,   318,   321,   324,   328,   332,   337,   342,
     347,   354,   359,   366,   371,   378,   383,   390,   395,   402,
     404,   407,   410,   412,   414,   417,   419,   422,   425,   428,
     430,   433,   436,   438,   441,   446,   449,   453,   455,   459,
     461,   465,   467,   471,   474,   477,   480,   483,   487,   490,
     494,   497,   500,   503,   507,   510,   511,   515,   516,   525,
     528,   529,   537,   546,   553,   556,   557,   559,   563,   568,
     569,   571,   575,   576,   579,   580,   582,   586,   588
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     107,     0,    -1,   107,   100,   108,    -1,   108,    -1,     1,
     100,    -1,    -1,   137,    -1,   112,    -1,    51,   127,    44,
      30,    -1,    51,   127,    44,   128,    -1,    51,   127,    73,
     127,    -1,    51,   127,    74,   127,    -1,    53,    30,    67,
     127,    -1,    53,    15,    67,   127,    -1,    53,    30,    55,
     127,    -1,    53,    15,    55,   127,    -1,    53,    16,   127,
      55,   127,    -1,    53,    16,   126,    67,   127,    -1,   144,
      -1,   127,    -1,   129,    -1,   144,    -1,   127,    -1,   129,
      -1,   116,    -1,   110,    -1,   113,    -1,   117,   127,   122,
     123,   122,    90,    -1,   118,    67,   127,   122,    55,   127,
     122,   123,   122,    90,    -1,   118,    67,   127,   122,    37,
      55,   127,   122,   123,   122,    90,    -1,   124,   111,   122,
      -1,   125,   127,   100,   123,   122,    91,    -1,   125,   127,
      55,   127,    -1,   119,   127,   122,    54,   123,   122,   114,
     122,    89,    -1,   119,   127,   122,    54,   123,   122,   114,
      38,   121,   123,   122,    89,    -1,    -1,   115,   114,    -1,
     120,   127,   122,    54,   123,   122,    -1,   119,   127,   122,
      54,   110,   122,    38,   121,   110,   122,    89,    -1,   119,
     127,   122,    54,   110,   122,    89,    -1,    52,    58,    -1,
      52,    57,    30,    -1,    43,    -1,    39,    -1,    -1,    -1,
      -1,   123,   100,    -1,   123,   112,    -1,    56,    30,    54,
      -1,    85,    -1,    12,    -1,    17,    -1,    33,    -1,    31,
      -1,    30,    -1,   126,    -1,   128,    -1,    25,    -1,    26,
     127,    -1,    27,   146,    -1,    30,   101,   145,   102,    -1,
      15,    -1,    16,   127,    -1,   109,    -1,   127,    95,   127,
      -1,   127,    96,   127,    -1,   127,    97,   127,    -1,   127,
      98,   127,    -1,   127,    72,   127,    -1,   127,    93,   127,
      -1,   127,    92,   127,    -1,   127,    67,   127,    -1,   127,
      68,   127,    -1,   127,    65,   127,    -1,   127,    66,   127,
      -1,   127,    69,   127,    -1,   127,    70,   127,    -1,    71,
     127,    -1,   127,    94,   127,    -1,   127,    75,   127,    -1,
     127,    76,   127,    -1,   127,    77,   127,    -1,    95,   127,
      -1,    96,   127,    -1,   101,   127,   102,    -1,   103,   145,
     104,    -1,    82,   127,    83,   127,    -1,    82,   127,    84,
     127,    -1,    78,   127,    49,   127,    -1,    78,   127,    55,
     127,    49,   127,    -1,    79,   127,    49,   127,    -1,    79,
     127,    55,   127,    49,   127,    -1,    80,   127,    49,   127,
      -1,    80,   127,    55,   127,    49,   127,    -1,    81,   127,
      49,   127,    -1,    81,   127,    55,   127,    49,   127,    -1,
      88,   101,    30,   102,    -1,    88,   101,    30,   105,   145,
     102,    -1,    88,    -1,    29,   126,    -1,    51,   127,    -1,
     133,    -1,   135,    -1,    40,    52,    -1,    40,    -1,    41,
     130,    -1,    86,   131,    -1,    64,   132,    -1,    19,    -1,
      21,   127,    -1,    20,   127,    -1,    20,    -1,    22,   146,
      -1,    61,   127,    57,   127,    -1,    61,   127,    -1,    23,
      30,   145,    -1,    30,    -1,   130,   105,    30,    -1,    30,
      -1,   131,   105,    30,    -1,    30,    -1,   132,   105,    30,
      -1,    42,    45,    -1,    42,    48,    -1,    42,    50,    -1,
      42,   127,    -1,    42,   127,   134,    -1,    42,   134,    -1,
      49,    47,   127,    -1,    47,   127,    -1,    62,    63,    -1,
      62,   127,    -1,    62,   127,   134,    -1,    62,   134,    -1,
      -1,    35,   136,   145,    -1,    -1,    46,    30,   138,   121,
     141,   100,   143,   123,    -1,    60,    30,    -1,    -1,    36,
     139,   121,   141,   100,   143,   123,    -1,   140,   121,   141,
     100,   143,   123,    34,   142,    -1,   140,   121,   141,   100,
     143,   123,    -1,    87,    30,    -1,    -1,    30,    -1,   141,
     105,    30,    -1,   141,   100,   105,    30,    -1,    -1,    30,
      -1,   142,   105,    30,    -1,    -1,    30,   146,    -1,    -1,
     127,    -1,   145,   105,   127,    -1,   127,    -1,   146,   105,
     127,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   125,   125,   126,   127,   130,   131,   132,   135,   141,
     144,   145,   146,   152,   159,   165,   172,   178,   186,   187,
     188,   191,   192,   193,   194,   197,   198,   203,   214,   231,
     243,   248,   250,   255,   265,   277,   278,   281,   289,   299,
     312,   315,   322,   329,   337,   340,   343,   344,   345,   348,
     354,   358,   361,   364,   367,   370,   376,   377,   378,   381,
     384,   385,   388,   396,   402,   403,   404,   405,   406,   407,
     408,   409,   410,   411,   412,   413,   414,   415,   416,   417,
     418,   419,   420,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     440,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     456,   459,   462,   466,   467,   468,   469,   472,   473,   476,
     477,   480,   481,   492,   493,   494,   495,   499,   503,   509,
     510,   513,   514,   518,   522,   526,   526,   556,   556,   562,
     563,   563,   569,   577,   584,   586,   587,   588,   589,   592,
     593,   594,   597,   600,   608,   609,   610,   613,   614
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
  "SYMBOL", "ENDCLAUSE", "tPLAYACCEL", "tMETHOD", "tDOWN", "tELSE",
  "tELSIF", "tEXIT", "tGLOBAL", "tGO", "tIF", "tINTO", "tLOOP", "tMACRO",
  "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET",
  "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tNLELSE", "tFACTORY",
  "tOPEN", "tPLAY", "tDONE", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tME", "tENDIF",
  "tENDREPEAT", "tENDTELL", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'\\n'", "'('", "')'", "'['", "']'", "','", "$accept",
  "program", "programline", "asgn", "stmtoneliner", "stmtonelinerwithif",
  "stmt", "ifstmt", "elseifstmtlist", "elseifstmt", "ifoneliner",
  "repeatwhile", "repeatwith", "if", "elseif", "begin", "end", "stmtlist",
  "when", "tell", "simpleexpr", "expr", "reference", "proc", "globallist",
  "propertylist", "instancelist", "gotofunc", "gotomovie", "playfunc",
  "@1", "defn", "@2", "@3", "on", "argdef", "endargdef", "argstore",
  "macro", "arglist", "nonemptyarglist", 0
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
     345,   346,    60,    62,    38,    43,    45,    42,    47,    37,
      10,    40,    41,    91,    93,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   106,   107,   107,   107,   108,   108,   108,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   110,   110,
     110,   111,   111,   111,   111,   112,   112,   112,   112,   112,
     112,   112,   112,   113,   113,   114,   114,   115,   116,   116,
     117,   118,   119,   120,   121,   122,   123,   123,   123,   124,
     125,   126,   126,   126,   126,   126,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     128,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   130,   130,   131,
     131,   132,   132,   133,   133,   133,   133,   133,   133,   134,
     134,   135,   135,   135,   135,   136,   135,   138,   137,   137,
     139,   137,   137,   137,   140,   141,   141,   141,   141,   142,
     142,   142,   143,   144,   145,   145,   145,   146,   146
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     0,     1,     1,     4,     4,
       4,     4,     4,     4,     4,     4,     5,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     6,    10,    11,
       3,     6,     4,     9,    12,     0,     2,     6,    11,     7,
       2,     3,     1,     1,     0,     0,     0,     2,     2,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     4,     1,     2,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     2,     2,     3,     3,     4,     4,     4,
       6,     4,     6,     4,     6,     4,     6,     4,     6,     1,
       2,     2,     1,     1,     2,     1,     2,     2,     2,     1,
       2,     2,     1,     2,     4,     2,     3,     1,     3,     1,
       3,     1,     3,     2,     2,     2,     2,     3,     2,     3,
       2,     2,     2,     3,     2,     0,     3,     0,     8,     2,
       0,     7,     8,     6,     2,     0,     1,     3,     4,     0,
       1,     3,     0,     2,     0,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    51,    62,     0,    52,   109,   112,     0,     0,
       0,    58,     0,     0,     0,    55,    54,    53,   135,   140,
     105,     0,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,    99,     0,     0,     0,   154,     0,     3,    64,
      25,     7,    26,     0,     0,     0,     0,     0,    56,    19,
      57,    20,   102,   103,     6,    44,    18,     4,    55,     0,
      63,   111,   110,   157,   113,   154,    59,    60,    55,   100,
     154,   153,   154,    44,   104,   117,   106,   123,     0,   124,
       0,   125,   126,   128,   137,   101,     0,    40,     0,     0,
       0,     0,   139,   115,   131,   132,   134,   121,   108,    78,
       0,     0,     0,     0,     0,   119,   107,   144,     0,    83,
      84,     0,   155,     0,     1,     5,    45,     0,    45,    45,
      24,     0,    22,    23,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   145,   154,     0,     0,   116,   155,     0,
     136,   145,     0,   130,     0,   127,    44,     0,     0,     0,
      41,     0,     0,    56,     0,     0,     0,    49,     0,   133,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,    86,     0,     2,    46,    45,     0,
      30,    45,     0,    46,    74,    75,    72,    73,    76,    77,
      69,    80,    81,    82,    71,    70,    79,    65,    66,    67,
      68,   146,     0,   158,    61,     0,   118,   129,   145,     8,
       9,    10,    11,    15,    13,     0,     0,    14,    12,   114,
     122,    89,     0,    91,     0,    93,     0,    95,     0,    87,
      88,   120,    97,   154,   156,    45,     0,    46,     0,    32,
      45,   152,     0,   152,     0,    17,    16,     0,     0,     0,
       0,     0,    47,    48,     0,     0,     0,    45,     0,     0,
       0,    46,   147,    46,   152,    90,    92,    94,    96,    98,
      27,     0,    45,    35,    45,    31,   148,   143,   141,    46,
      45,    46,    43,    45,    35,     0,     0,   149,   138,    46,
      45,    44,     0,    36,    45,    44,    39,   150,   142,    45,
       0,    46,    33,     0,     0,     0,     0,    28,    45,    46,
      45,   151,    29,     0,    45,     0,    34,    37,    38
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    47,    48,    49,    50,   129,   273,    52,   303,   304,
     130,    53,    54,    55,   305,   153,   197,   255,    56,    57,
      58,    59,    60,    61,    86,   116,   108,    62,    93,    63,
      82,    64,   166,    83,    65,   222,   318,   281,    66,   159,
      74
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -265
static const yytype_int16 yypact[] =
{
     312,   -76,  -265,  -265,   964,  -265,  -265,   964,   964,   964,
      20,  -265,   964,   964,    42,   997,  -265,  -265,  -265,  -265,
       4,    49,   844,  -265,    53,   964,   -38,    52,    60,    64,
     964,   904,    75,   964,   964,   964,   964,   964,   964,  -265,
      77,    78,     9,   964,   964,   964,   964,     6,  -265,  -265,
    -265,  -265,  -265,   964,    46,   964,   672,   964,  -265,  1514,
    -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,    13,   964,
    1514,  1514,  1514,  1514,    10,   964,  1514,    10,  -265,  -265,
     964,    10,   964,  -265,  -265,  -265,    11,  -265,   964,  -265,
      70,  -265,  1050,  -265,  -265,  1036,    88,  -265,     2,   964,
       3,    65,  -265,  1408,  -265,  1050,  -265,  -265,    17,  -265,
    1086,  1120,  1154,  1188,  1480,  -265,    19,  -265,    95,  -265,
    -265,  1442,  1514,     7,  -265,   402,  1514,   964,  1514,  -265,
    -265,   964,  1514,  -265,  -265,  1358,   964,   964,   964,   964,
     964,   964,   964,   964,   964,   964,   964,   964,   964,   964,
     964,   964,   964,    98,   964,  1036,   964,    24,  1442,   -13,
      24,    98,   100,  1514,   964,  -265,  -265,    73,   964,   964,
    -265,   964,   964,    82,  1394,   964,   964,  -265,   964,  -265,
     101,   964,   964,   964,   964,   964,   964,   964,   964,   964,
     964,   121,    -6,  -265,  -265,   964,  -265,  -265,  1514,    99,
    -265,  1514,   964,  -265,   -50,   -50,   -50,   -50,   144,   144,
    -265,   -34,   -50,   -50,   -50,   -50,   -34,   -17,   -17,  -265,
    -265,  -265,   -77,  1514,  -265,   -14,  -265,  1514,    98,  -265,
    -265,  1514,  1514,  1514,   -50,   964,   964,  1514,   -50,  1514,
    -265,  1514,  1222,  1514,  1256,  1514,  1290,  1514,  1324,  1514,
    1514,  -265,  -265,   964,  1514,   582,   -20,  -265,   102,  1514,
     582,    50,   124,    50,   -12,   -50,  1514,   964,   964,   964,
     964,    -1,  -265,  -265,    67,   103,   964,   582,   762,    68,
     131,  -265,  -265,  -265,    50,  1514,  1514,  1514,  1514,  -265,
    -265,   964,  1514,   123,  -265,  -265,  -265,   492,   582,  -265,
    1514,  -265,  -265,   128,   123,   964,   -23,   139,   582,  -265,
     582,  -265,    81,  -265,  1514,  -265,  -265,  -265,    71,   582,
      83,  -265,  -265,   134,   762,   145,   104,  -265,   582,  -265,
    -265,  -265,  -265,   106,   582,   107,  -265,  -265,  -265
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -265,  -265,    72,  -265,  -264,  -265,     1,  -265,  -112,  -265,
    -265,  -265,  -265,   143,  -265,   -81,    -8,   -94,  -265,  -265,
      -2,    -4,    33,   146,  -265,  -265,  -265,  -265,   -18,  -265,
    -265,  -265,  -265,  -265,  -265,  -151,  -265,  -247,   147,   -39,
      85
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -6
static const yytype_int16 yytable[] =
{
      70,    51,   161,    71,    72,    73,   124,   123,    76,    73,
     225,    73,    79,   106,   294,   315,   283,   275,    92,    96,
      97,    95,   142,   261,    67,   143,   103,   105,   262,   109,
     110,   111,   112,   113,   114,   276,   157,   299,   142,   119,
     120,   121,   122,   160,   148,   149,   150,   151,   152,   126,
      75,   128,   132,   135,     2,   142,    84,   171,   175,     5,
     330,   149,   150,   151,   152,   155,   316,    98,    99,   172,
     176,   122,    78,    16,   165,    17,   158,   264,   122,    85,
     151,   152,   100,    94,   163,   228,   263,   179,   284,   224,
     101,   262,   195,   262,   102,   174,   252,   173,    77,   253,
      81,   289,    14,   229,   195,   107,   125,   115,   117,   260,
     118,   194,   195,   127,   154,   156,   162,   164,   170,   177,
     199,   200,   180,   198,   191,   192,    51,   201,   221,   195,
     226,   240,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   235,
     122,   251,   223,   257,   282,   280,   278,   290,   291,   295,
     227,   296,   302,   277,   231,   232,   311,   233,   234,   317,
     322,   237,   238,   327,   239,   331,   325,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   297,   329,   298,
     256,   254,   313,   258,   332,   336,   338,   196,   259,   131,
     230,     0,   133,   134,     0,   308,     0,   310,     0,   136,
     137,   138,   139,     0,   271,   319,   142,     0,     0,   143,
     144,   145,     0,     0,     0,     0,     0,   328,     0,     0,
     321,   265,   266,     0,   324,   334,   146,   147,   148,   149,
     150,   151,   152,     0,     0,     0,     0,   274,     0,   122,
       0,     0,   279,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   285,   286,   287,   288,     0,     0,   293,
       0,     0,   292,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   301,     0,   306,   300,     0,     0,
       0,     0,   309,     0,     0,   312,     0,     0,     0,     0,
       0,   314,   320,     0,     0,     0,   323,     0,     0,     0,
       0,   326,    -5,     1,     0,     0,     0,     0,     0,     0,
     333,     0,   335,     0,     2,     0,   337,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,    14,    15,    16,     0,    17,     0,    18,    19,     0,
       0,     0,    20,    21,    22,    23,     0,     0,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,    29,    30,    31,     0,    32,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,    39,    40,    41,
      42,     0,     0,     0,     0,     0,     0,    43,    44,     0,
       0,     0,    -5,    45,     2,    46,     0,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,    14,    15,    16,     0,    17,     0,    18,    19,     0,
       0,     0,    20,    21,    22,    23,     0,     0,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,    29,    30,    31,     0,    32,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,    39,    40,    41,
      42,     0,     0,     0,     0,     0,     0,    43,    44,     0,
       0,     0,     0,    45,     2,    46,     0,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,    14,    15,    16,     0,    17,   307,    18,     0,     0,
       0,     0,    20,    21,    22,    23,     0,     0,     0,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    30,    31,     0,    32,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,    39,    40,     0,
      42,     0,     0,     0,     0,     0,     0,    43,    44,     0,
       0,     0,   272,    45,     2,    46,     0,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,    14,    15,    16,     0,    17,     0,    18,     0,     0,
       0,     0,    20,    21,    22,    23,     0,     0,     0,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    30,    31,     0,    32,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,    39,    40,     0,
      42,     0,     0,     0,     0,     0,     0,    43,    44,     0,
       0,     0,   272,    45,     2,    46,     0,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,    14,    15,    16,     0,    17,     0,    18,     0,     0,
       0,     0,    20,    21,    22,    23,     0,     0,     0,     0,
       0,     0,     0,    25,     0,    27,     0,     0,     0,     0,
       0,     0,     0,    30,    31,     0,    32,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,     0,    40,     0,
      42,     0,     0,     0,     0,     0,     0,    43,    44,     0,
       0,     0,     0,    45,     2,    46,     0,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    13,
       0,    14,    15,    16,     0,    17,     0,    18,     0,     0,
       0,     0,    20,    21,    22,     0,     0,     0,     0,     0,
       0,     0,     0,    25,     0,    27,     0,     0,     0,     0,
       0,     0,     0,    30,    31,     0,    32,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
      34,    35,    36,    37,    38,     0,     0,     0,    40,     0,
      42,     0,     0,     0,     0,     0,     2,    43,    44,     3,
       4,     5,     0,    45,     0,    46,     0,     0,     0,    11,
      12,    13,     0,    14,    68,    16,     0,    17,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    87,
       0,    88,    89,    90,    91,    69,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     2,     0,     0,     3,
       4,     5,    34,    35,    36,    37,    38,     0,     0,    11,
      12,    13,    42,    14,    68,    16,     0,    17,     0,    43,
      44,     0,     0,     0,     0,    45,     0,    46,     0,     0,
       0,    88,     0,    90,     0,    69,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   104,     0,     0,
       0,     0,     0,     0,     0,    33,     2,     0,     0,     3,
       4,     5,    34,    35,    36,    37,    38,     0,     0,    11,
      12,    13,    42,    14,    68,    16,     0,    17,     0,    43,
      44,     0,     0,     0,     0,    45,     0,    46,     0,     2,
       0,     0,     3,     4,     5,    69,     0,    27,     0,     0,
       0,     0,    11,    12,    13,     0,    14,    68,    16,     0,
      17,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,    69,     0,
      27,     0,    42,     0,     0,     0,     0,     0,     0,    43,
      44,     0,     0,     0,     0,    45,     0,    46,    33,     0,
       0,     0,     0,     0,     0,    34,    35,    36,    37,    38,
     167,     0,     0,     0,     0,    42,     0,     0,     0,     0,
       0,     0,    43,    44,     0,     0,     0,    88,    80,    90,
      46,   136,   137,   138,   139,   140,   141,     0,   142,   168,
     169,   143,   144,   145,     0,   136,   137,   138,   139,   140,
     141,     0,   142,     0,     0,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   181,     0,     0,     0,     0,
       0,   182,   146,   147,   148,   149,   150,   151,   152,     0,
       0,   136,   137,   138,   139,   140,   141,     0,   142,     0,
       0,   143,   144,   145,     0,     0,     0,     0,     0,   183,
       0,     0,     0,     0,     0,   184,     0,     0,   146,   147,
     148,   149,   150,   151,   152,   136,   137,   138,   139,   140,
     141,     0,   142,     0,     0,   143,   144,   145,     0,     0,
       0,     0,     0,   185,     0,     0,     0,     0,     0,   186,
       0,     0,   146,   147,   148,   149,   150,   151,   152,   136,
     137,   138,   139,   140,   141,     0,   142,     0,     0,   143,
     144,   145,     0,     0,     0,     0,     0,   187,     0,     0,
       0,     0,     0,   188,     0,     0,   146,   147,   148,   149,
     150,   151,   152,   136,   137,   138,   139,   140,   141,     0,
     142,     0,     0,   143,   144,   145,     0,     0,     0,     0,
       0,   267,     0,     0,     0,     0,     0,     0,     0,     0,
     146,   147,   148,   149,   150,   151,   152,   136,   137,   138,
     139,   140,   141,     0,   142,     0,     0,   143,   144,   145,
       0,     0,     0,     0,     0,   268,     0,     0,     0,     0,
       0,     0,     0,     0,   146,   147,   148,   149,   150,   151,
     152,   136,   137,   138,   139,   140,   141,     0,   142,     0,
       0,   143,   144,   145,     0,     0,     0,     0,     0,   269,
       0,     0,     0,     0,     0,     0,     0,     0,   146,   147,
     148,   149,   150,   151,   152,   136,   137,   138,   139,   140,
     141,     0,   142,     0,     0,   143,   144,   145,     0,     0,
       0,     0,     0,   270,     0,     0,     0,     0,     0,     0,
       0,     0,   146,   147,   148,   149,   150,   151,   152,   136,
     137,   138,   139,   140,   141,     0,   142,     0,     0,   143,
     144,   145,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   202,     0,     0,   146,   147,   148,   149,
     150,   151,   152,   136,   137,   138,   139,   140,   141,     0,
     142,     0,     0,   143,   144,   145,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   236,
     146,   147,   148,   149,   150,   151,   152,     0,   203,   136,
     137,   138,   139,   140,   141,   178,   142,     0,     0,   143,
     144,   145,     0,   136,   137,   138,   139,   140,   141,     0,
     142,     0,     0,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,     0,     0,     0,     0,     0,     0,     0,
     146,   147,   148,   149,   150,   151,   152,   136,   137,   138,
     139,   140,   141,     0,   142,     0,     0,   143,   144,   145,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   146,   147,   148,   149,   150,   151,
     152,     0,     0,     0,   193,   136,   137,   138,   139,   140,
     141,     0,   142,     0,     0,   143,   144,   145,     0,     0,
       0,     0,     0,   189,   190,     0,     0,     0,     0,     0,
       0,     0,   146,   147,   148,   149,   150,   151,   152,   136,
     137,   138,   139,   140,   141,     0,   142,     0,     0,   143,
     144,   145,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   146,   147,   148,   149,
     150,   151,   152
};

static const yytype_int16 yycheck[] =
{
       4,     0,    83,     7,     8,     9,     0,    46,    12,    13,
     161,    15,    14,    31,   278,    38,   263,    37,    22,    57,
      58,    25,    72,   100,   100,    75,    30,    31,   105,    33,
      34,    35,    36,    37,    38,    55,    75,   284,    72,    43,
      44,    45,    46,    82,    94,    95,    96,    97,    98,    53,
      30,    55,    56,    57,    12,    72,    52,    55,    55,    17,
     324,    95,    96,    97,    98,    69,    89,    15,    16,    67,
      67,    75,    30,    31,    92,    33,    80,   228,    82,    30,
      97,    98,    30,    30,    88,   166,   100,   105,   100,   102,
      30,   105,   105,   105,    30,    99,   102,    99,    13,   105,
      15,   102,    29,    30,   105,    30,   100,    30,    30,   203,
     101,   104,   105,    67,   101,   105,   105,    47,    30,    54,
     128,   129,   105,   127,   105,    30,   125,   131,    30,   105,
      30,    30,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,    67,
     154,    30,   156,    54,    30,   105,    54,    90,    55,    91,
     164,    30,    39,   257,   168,   169,    38,   171,   172,    30,
      89,   175,   176,    90,   178,    30,   105,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   281,    54,   283,
     198,   195,   304,   201,    90,    89,    89,   125,   202,    56,
     167,    -1,    56,    56,    -1,   299,    -1,   301,    -1,    65,
      66,    67,    68,    -1,   253,   309,    72,    -1,    -1,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,   321,    -1,    -1,
     311,   235,   236,    -1,   315,   329,    92,    93,    94,    95,
      96,    97,    98,    -1,    -1,    -1,    -1,   255,    -1,   253,
      -1,    -1,   260,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   267,   268,   269,   270,    -1,    -1,   277,
      -1,    -1,   276,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   292,    -1,   294,   291,    -1,    -1,
      -1,    -1,   300,    -1,    -1,   303,    -1,    -1,    -1,    -1,
      -1,   305,   310,    -1,    -1,    -1,   314,    -1,    -1,    -1,
      -1,   319,     0,     1,    -1,    -1,    -1,    -1,    -1,    -1,
     328,    -1,   330,    -1,    12,    -1,   334,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    -1,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    -1,    35,    36,    -1,
      -1,    -1,    40,    41,    42,    43,    -1,    -1,    46,    -1,
      -1,    -1,    -1,    51,    52,    53,    -1,    -1,    56,    -1,
      -1,    -1,    60,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    -1,    -1,    85,    86,    87,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    -1,
      -1,    -1,   100,   101,    12,   103,    -1,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    -1,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    -1,    35,    36,    -1,
      -1,    -1,    40,    41,    42,    43,    -1,    -1,    46,    -1,
      -1,    -1,    -1,    51,    52,    53,    -1,    -1,    56,    -1,
      -1,    -1,    60,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    -1,    -1,    85,    86,    87,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    -1,
      -1,    -1,    -1,   101,    12,   103,    -1,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    -1,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    34,    35,    -1,    -1,
      -1,    -1,    40,    41,    42,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    52,    53,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    -1,    -1,    85,    86,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    -1,
      -1,    -1,   100,   101,    12,   103,    -1,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    -1,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    -1,    35,    -1,    -1,
      -1,    -1,    40,    41,    42,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    52,    53,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    -1,    -1,    85,    86,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    -1,
      -1,    -1,   100,   101,    12,   103,    -1,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    -1,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    -1,    35,    -1,    -1,
      -1,    -1,    40,    41,    42,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    -1,    -1,    -1,    86,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    95,    96,    -1,
      -1,    -1,    -1,   101,    12,   103,    -1,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    -1,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    -1,    35,    -1,    -1,
      -1,    -1,    40,    41,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    -1,    -1,    -1,    86,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    12,    95,    96,    15,
      16,    17,    -1,   101,    -1,   103,    -1,    -1,    -1,    25,
      26,    27,    -1,    29,    30,    31,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    12,    -1,    -1,    15,
      16,    17,    78,    79,    80,    81,    82,    -1,    -1,    25,
      26,    27,    88,    29,    30,    31,    -1,    33,    -1,    95,
      96,    -1,    -1,    -1,    -1,   101,    -1,   103,    -1,    -1,
      -1,    47,    -1,    49,    -1,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    12,    -1,    -1,    15,
      16,    17,    78,    79,    80,    81,    82,    -1,    -1,    25,
      26,    27,    88,    29,    30,    31,    -1,    33,    -1,    95,
      96,    -1,    -1,    -1,    -1,   101,    -1,   103,    -1,    12,
      -1,    -1,    15,    16,    17,    51,    -1,    53,    -1,    -1,
      -1,    -1,    25,    26,    27,    -1,    29,    30,    31,    -1,
      33,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    -1,    51,    -1,
      53,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    95,
      96,    -1,    -1,    -1,    -1,   101,    -1,   103,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      44,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    95,    96,    -1,    -1,    -1,    47,   101,    49,
     103,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      74,    75,    76,    77,    -1,    65,    66,    67,    68,    69,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    92,    93,
      94,    95,    96,    97,    98,    49,    -1,    -1,    -1,    -1,
      -1,    55,    92,    93,    94,    95,    96,    97,    98,    -1,
      -1,    65,    66,    67,    68,    69,    70,    -1,    72,    -1,
      -1,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    65,    66,    67,    68,    69,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    65,
      66,    67,    68,    69,    70,    -1,    72,    -1,    -1,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    65,    66,    67,    68,    69,    70,    -1,
      72,    -1,    -1,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    65,    66,    67,
      68,    69,    70,    -1,    72,    -1,    -1,    75,    76,    77,
      -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    65,    66,    67,    68,    69,    70,    -1,    72,    -1,
      -1,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    65,    66,    67,    68,    69,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    65,
      66,    67,    68,    69,    70,    -1,    72,    -1,    -1,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    65,    66,    67,    68,    69,    70,    -1,
      72,    -1,    -1,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      92,    93,    94,    95,    96,    97,    98,    -1,   100,    65,
      66,    67,    68,    69,    70,    57,    72,    -1,    -1,    75,
      76,    77,    -1,    65,    66,    67,    68,    69,    70,    -1,
      72,    -1,    -1,    75,    76,    77,    92,    93,    94,    95,
      96,    97,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    65,    66,    67,
      68,    69,    70,    -1,    72,    -1,    -1,    75,    76,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    -1,    -1,    -1,   102,    65,    66,    67,    68,    69,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    65,
      66,    67,    68,    69,    70,    -1,    72,    -1,    -1,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    15,    16,    17,    19,    20,    21,    22,
      23,    25,    26,    27,    29,    30,    31,    33,    35,    36,
      40,    41,    42,    43,    46,    51,    52,    53,    56,    60,
      61,    62,    64,    71,    78,    79,    80,    81,    82,    85,
      86,    87,    88,    95,    96,   101,   103,   107,   108,   109,
     110,   112,   113,   117,   118,   119,   124,   125,   126,   127,
     128,   129,   133,   135,   137,   140,   144,   100,    30,    51,
     127,   127,   127,   127,   146,    30,   127,   146,    30,   126,
     101,   146,   136,   139,    52,    30,   130,    45,    47,    48,
      49,    50,   127,   134,    30,   127,    57,    58,    15,    16,
      30,    30,    30,   127,    63,   127,   134,    30,   132,   127,
     127,   127,   127,   127,   127,    30,   131,    30,   101,   127,
     127,   127,   127,   145,     0,   100,   127,    67,   127,   111,
     116,   119,   127,   129,   144,   127,    65,    66,    67,    68,
      69,    70,    72,    75,    76,    77,    92,    93,    94,    95,
      96,    97,    98,   121,   101,   127,   105,   145,   127,   145,
     145,   121,   105,   127,    47,   134,   138,    44,    73,    74,
      30,    55,    67,   126,   127,    55,    67,    54,    57,   134,
     105,    49,    55,    49,    55,    49,    55,    49,    55,    83,
      84,   105,    30,   102,   104,   105,   108,   122,   127,   122,
     122,   127,    55,   100,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,    30,   141,   127,   102,   141,    30,   127,   121,    30,
     128,   127,   127,   127,   127,    67,    55,   127,   127,   127,
      30,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,    30,   102,   105,   127,   123,   122,    54,   122,   127,
     123,   100,   105,   100,   141,   127,   127,    49,    49,    49,
      49,   145,   100,   112,   122,    37,    55,   123,    54,   122,
     105,   143,    30,   143,   100,   127,   127,   127,   127,   102,
      90,    55,   127,   122,   110,    91,    30,   123,   123,   143,
     127,   122,    39,   114,   115,   120,   122,    34,   123,   122,
     123,    38,   122,   114,   127,    38,    89,    30,   142,   123,
     122,   121,    89,   122,   121,   105,   122,    90,   123,    54,
     110,    30,    90,   122,   123,   122,    89,   122,    89
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
#line 127 "engines/director/lingo/lingo-gr.y"
    { yyerrok; ;}
    break;

  case 8:
#line 135 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 9:
#line 141 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_assign);
			(yyval.code) = (yyvsp[(2) - (4)].code); ;}
    break;

  case 10:
#line 144 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_after); ;}
    break;

  case 11:
#line 145 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_before); ;}
    break;

  case 12:
#line 146 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 13:
#line 152 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[1]);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 14:
#line 159 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 15:
#line 165 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[1]);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 16:
#line 172 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[1]);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 17:
#line 178 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[1]);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 27:
#line 203 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(3) - (6)].code) - (yyvsp[(1) - (6)].code));
		WRITE_UINT32(&end, (yyvsp[(5) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 2] = end; ;}
    break;

  case 28:
#line 214 "engines/director/lingo/lingo-gr.y"
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

  case 29:
#line 231 "engines/director/lingo/lingo-gr.y"
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

  case 30:
#line 243 "engines/director/lingo/lingo-gr.y"
    {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[(3) - (3)].code) - (yyvsp[(1) - (3)].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (3)].code) + 1] = end; ;}
    break;

  case 31:
#line 248 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented"); ;}
    break;

  case 32:
#line 250 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		;}
    break;

  case 33:
#line 255 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (9)].code) - (yyvsp[(1) - (9)].code));
		WRITE_UINT32(&end, (yyvsp[(8) - (9)].code) - (yyvsp[(1) - (9)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (9)].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[(1) - (9)].code), (yyvsp[(8) - (9)].code) - (yyvsp[(1) - (9)].code), 0); ;}
    break;

  case 34:
#line 265 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (12)].code) - (yyvsp[(1) - (12)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (12)].code) - (yyvsp[(1) - (12)].code));
		WRITE_UINT32(&end, (yyvsp[(11) - (12)].code) - (yyvsp[(1) - (12)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (12)].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[(1) - (12)].code), (yyvsp[(11) - (12)].code) - (yyvsp[(1) - (12)].code), (yyvsp[(9) - (12)].code) - (yyvsp[(1) - (12)].code)); ;}
    break;

  case 37:
#line 281 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 38:
#line 289 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, else1 = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (11)].code) - (yyvsp[(1) - (11)].code));
		WRITE_UINT32(&else1, (yyvsp[(6) - (11)].code) - (yyvsp[(1) - (11)].code));
		WRITE_UINT32(&end, (yyvsp[(10) - (11)].code) - (yyvsp[(1) - (11)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 2] = else1;	/* elsepart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (11)].code) + 3] = end;	/* end, if cond fails */

		g_lingo->processIf((yyvsp[(1) - (11)].code), (yyvsp[(10) - (11)].code) - (yyvsp[(1) - (11)].code), (yyvsp[(8) - (11)].code) - (yyvsp[(1) - (11)].code)); ;}
    break;

  case 39:
#line 299 "engines/director/lingo/lingo-gr.y"
    {
			inst then = 0, else1 = 0, end = 0;
			WRITE_UINT32(&then, (yyvsp[(3) - (7)].code) - (yyvsp[(1) - (7)].code));
			WRITE_UINT32(&else1, 0);
			WRITE_UINT32(&end, (yyvsp[(6) - (7)].code) - (yyvsp[(1) - (7)].code));
			(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 1] = then;	/* thenpart */
			(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 2] = else1;	/* elsepart */
			(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 3] = end;	/* end, if cond fails */

			g_lingo->processIf((yyvsp[(1) - (7)].code), (yyvsp[(6) - (7)].code) - (yyvsp[(1) - (7)].code), (yyvsp[(6) - (7)].code) - (yyvsp[(1) - (7)].code)); ;}
    break;

  case 40:
#line 312 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 41:
#line 315 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 42:
#line 322 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 43:
#line 329 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 44:
#line 337 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 45:
#line 340 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 46:
#line 343 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 49:
#line 348 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 50:
#line 354 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_tellcode);
		g_lingo->code1(STOP); ;}
    break;

  case 51:
#line 358 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt((yyvsp[(1) - (1)].i)); ;}
    break;

  case 52:
#line 361 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_floatpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 53:
#line 364 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 54:
#line 367 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 55:
#line 370 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 56:
#line 376 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(1) - (1)].code); ;}
    break;

  case 58:
#line 378 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 59:
#line 381 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 60:
#line 384 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 61:
#line 385 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 62:
#line 388 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 63:
#line 396 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 65:
#line 403 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 66:
#line 404 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 67:
#line 405 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 68:
#line 406 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 69:
#line 407 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); ;}
    break;

  case 70:
#line 408 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 71:
#line 409 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 72:
#line 410 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_eq); ;}
    break;

  case 73:
#line 411 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 74:
#line 412 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 75:
#line 413 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 76:
#line 414 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 77:
#line 415 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 78:
#line 416 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 79:
#line 417 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 80:
#line 418 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 81:
#line 419 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 82:
#line 420 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 83:
#line 421 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 84:
#line 422 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 85:
#line 423 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 86:
#line 424 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_arraypush); g_lingo->codeArray((yyvsp[(2) - (3)].narg)); ;}
    break;

  case 87:
#line 425 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 88:
#line 426 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 89:
#line 427 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); ;}
    break;

  case 90:
#line 428 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); ;}
    break;

  case 91:
#line 429 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); ;}
    break;

  case 92:
#line 430 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); ;}
    break;

  case 93:
#line 431 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); ;}
    break;

  case 94:
#line 432 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); ;}
    break;

  case 95:
#line 433 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); ;}
    break;

  case 96:
#line 434 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); ;}
    break;

  case 97:
#line 435 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 98:
#line 436 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].narg)); ;}
    break;

  case 99:
#line 437 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe(nullptr, 0); ;}
    break;

  case 100:
#line 440 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 101:
#line 445 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 104:
#line 448 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); ;}
    break;

  case 105:
#line 449 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 109:
#line 453 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 110:
#line 456 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 111:
#line 459 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 112:
#line 462 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_voidpush);
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 1);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 113:
#line 466 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 114:
#line 467 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 115:
#line 468 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 116:
#line 469 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[(1) - (3)].s)); s += '-'; s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 117:
#line 472 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 118:
#line 473 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 119:
#line 476 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 120:
#line 477 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 121:
#line 480 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 122:
#line 481 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 123:
#line 492 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 124:
#line 493 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 125:
#line 494 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 126:
#line 495 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 127:
#line 499 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 128:
#line 503 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 131:
#line 513 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 132:
#line 514 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 133:
#line 518 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 134:
#line 522 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 135:
#line 526 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); ;}
    break;

  case 136:
#line 526 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[(1) - (3)].s), (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 137:
#line 556 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = kStateInArgs; g_lingo->_currentFactory.clear(); ;}
    break;

  case 138:
#line 557 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->clearArgStack();
			g_lingo->_indef = kStateNone; ;}
    break;

  case 139:
#line 562 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFactory(*(yyvsp[(2) - (2)].s)); ;}
    break;

  case 140:
#line 563 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = kStateInArgs; ;}
    break;

  case 141:
#line 564 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(1) - (7)].s), (yyvsp[(3) - (7)].code), (yyvsp[(4) - (7)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->clearArgStack();
			g_lingo->_indef = kStateNone; ;}
    break;

  case 142:
#line 569 "engines/director/lingo/lingo-gr.y"
    {	// D3
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(yyvsp[(1) - (8)].s), (yyvsp[(2) - (8)].code), (yyvsp[(3) - (8)].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[(7) - (8)].s), (yyvsp[(1) - (8)].s)->c_str(), false); ;}
    break;

  case 143:
#line 577 "engines/director/lingo/lingo-gr.y"
    {	// D4. No 'end' clause
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(yyvsp[(1) - (6)].s), (yyvsp[(2) - (6)].code), (yyvsp[(3) - (6)].narg));
		g_lingo->_indef = kStateNone;
		g_lingo->clearArgStack();
		g_lingo->_ignoreMe = false; ;}
    break;

  case 144:
#line 584 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); g_lingo->_indef = kStateInArgs; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; ;}
    break;

  case 145:
#line 586 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 146:
#line 587 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 147:
#line 588 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 148:
#line 589 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 152:
#line 597 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); g_lingo->_indef = kStateInDef; ;}
    break;

  case 153:
#line 600 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (2)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(2) - (2)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 154:
#line 608 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 155:
#line 609 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 156:
#line 610 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 157:
#line 613 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 158:
#line 614 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2953 "engines/director/lingo/lingo-gr.cpp"
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


#line 617 "engines/director/lingo/lingo-gr.y"


