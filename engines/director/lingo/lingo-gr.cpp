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
     THEOBJECTFIELD = 289,
     ENDCLAUSE = 290,
     tPLAYACCEL = 291,
     tMETHOD = 292,
     tDOWN = 293,
     tELSE = 294,
     tELSIF = 295,
     tEXIT = 296,
     tGLOBAL = 297,
     tGO = 298,
     tIF = 299,
     tINTO = 300,
     tLOOP = 301,
     tMACRO = 302,
     tMOVIE = 303,
     tNEXT = 304,
     tOF = 305,
     tPREVIOUS = 306,
     tPUT = 307,
     tREPEAT = 308,
     tSET = 309,
     tTHEN = 310,
     tTO = 311,
     tWHEN = 312,
     tWITH = 313,
     tWHILE = 314,
     tNLELSE = 315,
     tFACTORY = 316,
     tOPEN = 317,
     tPLAY = 318,
     tDONE = 319,
     tINSTANCE = 320,
     tGE = 321,
     tLE = 322,
     tEQ = 323,
     tNEQ = 324,
     tAND = 325,
     tOR = 326,
     tNOT = 327,
     tMOD = 328,
     tAFTER = 329,
     tBEFORE = 330,
     tCONCAT = 331,
     tCONTAINS = 332,
     tSTARTS = 333,
     tCHAR = 334,
     tITEM = 335,
     tLINE = 336,
     tWORD = 337,
     tSPRITE = 338,
     tINTERSECTS = 339,
     tWITHIN = 340,
     tTELL = 341,
     tPROPERTY = 342,
     tON = 343,
     tME = 344,
     tENDIF = 345,
     tENDREPEAT = 346,
     tENDTELL = 347
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
#define THEOBJECTFIELD 289
#define ENDCLAUSE 290
#define tPLAYACCEL 291
#define tMETHOD 292
#define tDOWN 293
#define tELSE 294
#define tELSIF 295
#define tEXIT 296
#define tGLOBAL 297
#define tGO 298
#define tIF 299
#define tINTO 300
#define tLOOP 301
#define tMACRO 302
#define tMOVIE 303
#define tNEXT 304
#define tOF 305
#define tPREVIOUS 306
#define tPUT 307
#define tREPEAT 308
#define tSET 309
#define tTHEN 310
#define tTO 311
#define tWHEN 312
#define tWITH 313
#define tWHILE 314
#define tNLELSE 315
#define tFACTORY 316
#define tOPEN 317
#define tPLAY 318
#define tDONE 319
#define tINSTANCE 320
#define tGE 321
#define tLE 322
#define tEQ 323
#define tNEQ 324
#define tAND 325
#define tOR 326
#define tNOT 327
#define tMOD 328
#define tAFTER 329
#define tBEFORE 330
#define tCONCAT 331
#define tCONTAINS 332
#define tSTARTS 333
#define tCHAR 334
#define tITEM 335
#define tLINE 336
#define tWORD 337
#define tSPRITE 338
#define tINTERSECTS 339
#define tWITHIN 340
#define tTELL 341
#define tPROPERTY 342
#define tON 343
#define tME 344
#define tENDIF 345
#define tENDREPEAT 346
#define tENDTELL 347




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

	struct {
		Common::String *s;
		int e;
	} objectfield;
}
/* Line 193 of yacc.c.  */
#line 327 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 340 "engines/director/lingo/lingo-gr.cpp"

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
#define YYLAST   1621

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  107
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  41
/* YYNRULES -- Number of rules.  */
#define YYNRULES  158
/* YYNRULES -- Number of states.  */
#define YYNSTATES  339

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   347

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     101,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   100,    95,     2,
     102,   103,    98,    96,   106,    97,     2,    99,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      93,     2,    94,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   104,     2,   105,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92
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
     108,     0,    -1,   108,   101,   109,    -1,   109,    -1,     1,
     101,    -1,    -1,   138,    -1,   113,    -1,    52,   128,    45,
      30,    -1,    52,   128,    45,   129,    -1,    52,   128,    74,
     128,    -1,    52,   128,    75,   128,    -1,    54,    30,    68,
     128,    -1,    54,    15,    68,   128,    -1,    54,    30,    56,
     128,    -1,    54,    15,    56,   128,    -1,    54,    16,   128,
      56,   128,    -1,    54,    16,   127,    68,   128,    -1,   145,
      -1,   128,    -1,   130,    -1,   145,    -1,   128,    -1,   130,
      -1,   117,    -1,   111,    -1,   114,    -1,   118,   128,   123,
     124,   123,    91,    -1,   119,    68,   128,   123,    56,   128,
     123,   124,   123,    91,    -1,   119,    68,   128,   123,    38,
      56,   128,   123,   124,   123,    91,    -1,   125,   112,   123,
      -1,   126,   128,   101,   124,   123,    92,    -1,   126,   128,
      56,   128,    -1,   120,   128,   123,    55,   124,   123,   115,
     123,    90,    -1,   120,   128,   123,    55,   124,   123,   115,
      39,   122,   124,   123,    90,    -1,    -1,   116,   115,    -1,
     121,   128,   123,    55,   124,   123,    -1,   120,   128,   123,
      55,   111,   123,    39,   122,   111,   123,    90,    -1,   120,
     128,   123,    55,   111,   123,    90,    -1,    53,    59,    -1,
      53,    58,    30,    -1,    44,    -1,    40,    -1,    -1,    -1,
      -1,   124,   101,    -1,   124,   113,    -1,    57,    30,    55,
      -1,    86,    -1,    12,    -1,    17,    -1,    33,    -1,    31,
      -1,    30,    -1,   127,    -1,   129,    -1,    25,    -1,    26,
     128,    -1,    27,   147,    -1,    30,   102,   146,   103,    -1,
      15,    -1,    16,   128,    -1,   110,    -1,   128,    96,   128,
      -1,   128,    97,   128,    -1,   128,    98,   128,    -1,   128,
      99,   128,    -1,   128,    73,   128,    -1,   128,    94,   128,
      -1,   128,    93,   128,    -1,   128,    68,   128,    -1,   128,
      69,   128,    -1,   128,    66,   128,    -1,   128,    67,   128,
      -1,   128,    70,   128,    -1,   128,    71,   128,    -1,    72,
     128,    -1,   128,    95,   128,    -1,   128,    76,   128,    -1,
     128,    77,   128,    -1,   128,    78,   128,    -1,    96,   128,
      -1,    97,   128,    -1,   102,   128,   103,    -1,   104,   146,
     105,    -1,    83,   128,    84,   128,    -1,    83,   128,    85,
     128,    -1,    79,   128,    50,   128,    -1,    79,   128,    56,
     128,    50,   128,    -1,    80,   128,    50,   128,    -1,    80,
     128,    56,   128,    50,   128,    -1,    81,   128,    50,   128,
      -1,    81,   128,    56,   128,    50,   128,    -1,    82,   128,
      50,   128,    -1,    82,   128,    56,   128,    50,   128,    -1,
      89,   102,    30,   103,    -1,    89,   102,    30,   106,   146,
     103,    -1,    89,    -1,    29,   127,    -1,    52,   128,    -1,
     134,    -1,   136,    -1,    41,    53,    -1,    41,    -1,    42,
     131,    -1,    87,   132,    -1,    65,   133,    -1,    19,    -1,
      21,   128,    -1,    20,   128,    -1,    20,    -1,    22,   147,
      -1,    62,   128,    58,   128,    -1,    62,   128,    -1,    23,
      30,   146,    -1,    30,    -1,   131,   106,    30,    -1,    30,
      -1,   132,   106,    30,    -1,    30,    -1,   133,   106,    30,
      -1,    43,    46,    -1,    43,    49,    -1,    43,    51,    -1,
      43,   128,    -1,    43,   128,   135,    -1,    43,   135,    -1,
      50,    48,   128,    -1,    48,   128,    -1,    63,    64,    -1,
      63,   128,    -1,    63,   128,   135,    -1,    63,   135,    -1,
      -1,    36,   137,   146,    -1,    -1,    47,    30,   139,   122,
     142,   101,   144,   124,    -1,    61,    30,    -1,    -1,    37,
     140,   122,   142,   101,   144,   124,    -1,   141,   122,   142,
     101,   144,   124,    35,   143,    -1,   141,   122,   142,   101,
     144,   124,    -1,    88,    30,    -1,    -1,    30,    -1,   142,
     106,    30,    -1,   142,   101,   106,    30,    -1,    -1,    30,
      -1,   143,   106,    30,    -1,    -1,    30,   147,    -1,    -1,
     128,    -1,   146,   106,   128,    -1,   128,    -1,   147,   106,
     128,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   130,   130,   131,   132,   135,   136,   137,   140,   146,
     149,   150,   151,   157,   164,   170,   177,   183,   191,   192,
     193,   196,   197,   198,   199,   202,   203,   208,   219,   236,
     248,   253,   255,   260,   270,   282,   283,   286,   294,   304,
     317,   320,   327,   334,   342,   345,   348,   349,   350,   353,
     359,   363,   366,   369,   372,   375,   381,   382,   383,   386,
     389,   390,   393,   401,   407,   408,   409,   410,   411,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     445,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     461,   464,   467,   471,   472,   473,   474,   477,   478,   481,
     482,   485,   486,   497,   498,   499,   500,   504,   508,   514,
     515,   518,   519,   523,   527,   531,   531,   561,   561,   567,
     568,   568,   574,   582,   589,   591,   592,   593,   594,   597,
     598,   599,   602,   605,   613,   614,   615,   618,   619
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
  "SYMBOL", "THEOBJECTFIELD", "ENDCLAUSE", "tPLAYACCEL", "tMETHOD",
  "tDOWN", "tELSE", "tELSIF", "tEXIT", "tGLOBAL", "tGO", "tIF", "tINTO",
  "tLOOP", "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT",
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tNLELSE",
  "tFACTORY", "tOPEN", "tPLAY", "tDONE", "tINSTANCE", "tGE", "tLE", "tEQ",
  "tNEQ", "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
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
     345,   346,   347,    60,    62,    38,    43,    45,    42,    47,
      37,    10,    40,    41,    91,    93,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   107,   108,   108,   108,   109,   109,   109,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   111,   111,
     111,   112,   112,   112,   112,   113,   113,   113,   113,   113,
     113,   113,   113,   114,   114,   115,   115,   116,   117,   117,
     118,   119,   120,   121,   122,   123,   124,   124,   124,   125,
     126,   127,   127,   127,   127,   127,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     129,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   131,   131,   132,
     132,   133,   133,   134,   134,   134,   134,   134,   134,   135,
     135,   136,   136,   136,   136,   137,   136,   139,   138,   138,
     140,   138,   138,   138,   141,   142,   142,   142,   142,   143,
     143,   143,   144,   145,   146,   146,   146,   147,   147
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
     312,   -42,  -265,  -265,   972,  -265,  -265,   972,   972,   972,
     -10,  -265,   972,   972,    58,  1005,  -265,  -265,  -265,  -265,
      16,    20,   850,  -265,    43,   972,    24,    42,    70,    76,
     972,   911,    78,   972,   972,   972,   972,   972,   972,  -265,
      80,    81,    10,   972,   972,   972,   972,     6,  -265,  -265,
    -265,  -265,  -265,   972,    45,   972,   676,   972,  -265,  1522,
    -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,    12,   972,
    1522,  1522,  1522,  1522,     9,   972,  1522,     9,  -265,  -265,
     972,     9,   972,  -265,  -265,  -265,    11,  -265,   972,  -265,
      68,  -265,  1058,  -265,  -265,  1044,    88,  -265,    -1,   972,
       0,    64,  -265,  1416,  -265,  1058,  -265,  -265,    18,  -265,
    1094,  1128,  1162,  1196,  1488,  -265,    19,  -265,    92,  -265,
    -265,  1450,  1522,    -3,  -265,   403,  1522,   972,  1522,  -265,
    -265,   972,  1522,  -265,  -265,  1366,   972,   972,   972,   972,
     972,   972,   972,   972,   972,   972,   972,   972,   972,   972,
     972,   972,   972,    98,   972,  1044,   972,    23,  1450,   -13,
      23,    98,   100,  1522,   972,  -265,  -265,    75,   972,   972,
    -265,   972,   972,    63,  1402,   972,   972,  -265,   972,  -265,
     119,   972,   972,   972,   972,   972,   972,   972,   972,   972,
     972,   121,    -7,  -265,  -265,   972,  -265,  -265,  1522,    99,
    -265,  1522,   972,  -265,   -51,   -51,   -51,   -51,   143,   143,
    -265,   -35,   -51,   -51,   -51,   -51,   -35,   -19,   -19,  -265,
    -265,  -265,   -82,  1522,  -265,   -78,  -265,  1522,    98,  -265,
    -265,  1522,  1522,  1522,   -51,   972,   972,  1522,   -51,  1522,
    -265,  1522,  1230,  1522,  1264,  1522,  1298,  1522,  1332,  1522,
    1522,  -265,  -265,   972,  1522,   585,   -21,  -265,   101,  1522,
     585,    47,   125,    47,   -20,   -51,  1522,   972,   972,   972,
     972,    -5,  -265,  -265,    66,   102,   972,   585,   767,    67,
     131,  -265,  -265,  -265,    47,  1522,  1522,  1522,  1522,  -265,
    -265,   972,  1522,   122,  -265,  -265,  -265,   494,   585,  -265,
    1522,  -265,  -265,   127,   122,   972,   -24,   139,   585,  -265,
     585,  -265,    83,  -265,  1522,  -265,  -265,  -265,    69,   585,
      85,  -265,  -265,   115,   767,   158,   103,  -265,   585,  -265,
    -265,  -265,  -265,   105,   585,   106,  -265,  -265,  -265
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -265,  -265,    72,  -265,  -264,  -265,     1,  -265,  -112,  -265,
    -265,  -265,  -265,   144,  -265,   -81,    -8,   -94,  -265,  -265,
      -2,    -4,    32,   145,  -265,  -265,  -265,  -265,   -18,  -265,
    -265,  -265,  -265,  -265,  -265,  -151,  -265,  -247,   146,   -39,
      79
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -6
static const yytype_int16 yytable[] =
{
      70,    51,   161,    71,    72,    73,   124,   123,    76,    73,
     225,    73,    79,   106,   294,   315,   283,   275,    92,   261,
      75,    95,   142,   263,   262,   143,   103,   105,   262,   109,
     110,   111,   112,   113,   114,   276,   157,   299,   142,   119,
     120,   121,   122,   160,   148,   149,   150,   151,   152,   126,
      85,   128,   132,   135,   142,   171,   175,    98,    99,    67,
     330,   149,   150,   151,   152,   155,   316,   172,   176,    84,
       2,   122,   100,    94,   165,     5,   158,   264,   122,   151,
     152,   284,    96,    97,   163,   228,   262,   179,    78,    16,
     224,    17,    77,   195,    81,   174,   252,   173,   289,   253,
     101,   195,   194,   195,    14,   229,   102,   125,   107,   260,
     115,   117,   118,   127,   154,   156,   164,   162,   170,   177,
     199,   200,   192,   198,   180,   191,    51,   201,   221,   195,
     226,   235,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   240,
     122,   251,   223,   280,   257,   282,   278,   290,   291,   295,
     227,   296,   302,   277,   231,   232,   311,   233,   234,   317,
     329,   237,   238,   322,   239,   325,   327,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   297,   331,   298,
     256,   254,   313,   258,   332,   336,   338,   196,   259,   230,
     131,   133,   134,     0,     0,   308,     0,   310,     0,   136,
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
       0,    14,    15,    16,     0,    17,     0,     0,    18,    19,
       0,     0,     0,    20,    21,    22,    23,     0,     0,    24,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,    29,    30,    31,     0,    32,     0,     0,
       0,     0,     0,     0,    33,     0,     0,     0,     0,     0,
       0,    34,    35,    36,    37,    38,     0,     0,    39,    40,
      41,    42,     0,     0,     0,     0,     0,     0,    43,    44,
       0,     0,     0,    -5,    45,     2,    46,     0,     3,     4,
       5,     0,     6,     7,     8,     9,    10,     0,    11,    12,
      13,     0,    14,    15,    16,     0,    17,     0,     0,    18,
      19,     0,     0,     0,    20,    21,    22,    23,     0,     0,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,    29,    30,    31,     0,    32,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,    38,     0,     0,    39,
      40,    41,    42,     0,     0,     0,     0,     0,     0,    43,
      44,     0,     0,     0,     0,    45,     2,    46,     0,     3,
       4,     5,     0,     6,     7,     8,     9,    10,     0,    11,
      12,    13,     0,    14,    15,    16,     0,    17,     0,   307,
      18,     0,     0,     0,     0,    20,    21,    22,    23,     0,
       0,     0,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    30,    31,     0,    32,
       0,     0,     0,     0,     0,     0,    33,     0,     0,     0,
       0,     0,     0,    34,    35,    36,    37,    38,     0,     0,
      39,    40,     0,    42,     0,     0,     0,     0,     0,     0,
      43,    44,     0,     0,     0,   272,    45,     2,    46,     0,
       3,     4,     5,     0,     6,     7,     8,     9,    10,     0,
      11,    12,    13,     0,    14,    15,    16,     0,    17,     0,
       0,    18,     0,     0,     0,     0,    20,    21,    22,    23,
       0,     0,     0,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    30,    31,     0,
      32,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,     0,     0,    34,    35,    36,    37,    38,     0,
       0,    39,    40,     0,    42,     0,     0,     0,     0,     0,
       0,    43,    44,     0,     0,     0,   272,    45,     2,    46,
       0,     3,     4,     5,     0,     6,     7,     8,     9,    10,
       0,    11,    12,    13,     0,    14,    15,    16,     0,    17,
       0,     0,    18,     0,     0,     0,     0,    20,    21,    22,
      23,     0,     0,     0,     0,     0,     0,     0,    25,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    30,    31,
       0,    32,     0,     0,     0,     0,     0,     0,    33,     0,
       0,     0,     0,     0,     0,    34,    35,    36,    37,    38,
       0,     0,     0,    40,     0,    42,     0,     0,     0,     0,
       0,     0,    43,    44,     0,     0,     0,     0,    45,     2,
      46,     0,     3,     4,     5,     0,     6,     7,     8,     9,
      10,     0,    11,    12,    13,     0,    14,    15,    16,     0,
      17,     0,     0,    18,     0,     0,     0,     0,    20,    21,
      22,     0,     0,     0,     0,     0,     0,     0,     0,    25,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    30,
      31,     0,    32,     0,     0,     0,     0,     0,     0,    33,
       0,     0,     0,     0,     0,     0,    34,    35,    36,    37,
      38,     0,     0,     0,    40,     0,    42,     0,     0,     0,
       0,     0,     2,    43,    44,     3,     4,     5,     0,    45,
       0,    46,     0,     0,     0,    11,    12,    13,     0,    14,
      68,    16,     0,    17,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    87,     0,    88,    89,
      90,    91,    69,     0,    27,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    33,     2,     0,     0,     3,     4,     5,    34,
      35,    36,    37,    38,     0,     0,    11,    12,    13,    42,
      14,    68,    16,     0,    17,     0,    43,    44,     0,     0,
       0,     0,    45,     0,    46,     0,     0,     0,     0,    88,
       0,    90,     0,    69,     0,    27,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,     0,     0,     0,     0,
       0,     0,     0,    33,     2,     0,     0,     3,     4,     5,
      34,    35,    36,    37,    38,     0,     0,    11,    12,    13,
      42,    14,    68,    16,     0,    17,     0,    43,    44,     0,
       0,     0,     0,    45,     0,    46,     0,     2,     0,     0,
       3,     4,     5,     0,    69,     0,    27,     0,     0,     0,
      11,    12,    13,     0,    14,    68,    16,     0,    17,     0,
       0,     0,     0,     0,    33,     0,     0,     0,     0,     0,
       0,    34,    35,    36,    37,    38,     0,    69,     0,    27,
       0,    42,     0,     0,     0,     0,     0,     0,    43,    44,
       0,     0,     0,     0,    45,     0,    46,    33,     0,     0,
       0,     0,     0,     0,    34,    35,    36,    37,    38,   167,
       0,     0,     0,     0,    42,     0,     0,     0,     0,     0,
       0,    43,    44,     0,     0,     0,    88,    80,    90,    46,
     136,   137,   138,   139,   140,   141,     0,   142,   168,   169,
     143,   144,   145,     0,   136,   137,   138,   139,   140,   141,
       0,   142,     0,     0,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   181,     0,     0,     0,     0,     0,
     182,   146,   147,   148,   149,   150,   151,   152,     0,     0,
     136,   137,   138,   139,   140,   141,     0,   142,     0,     0,
     143,   144,   145,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,     0,   184,     0,     0,   146,   147,   148,
     149,   150,   151,   152,   136,   137,   138,   139,   140,   141,
       0,   142,     0,     0,   143,   144,   145,     0,     0,     0,
       0,     0,   185,     0,     0,     0,     0,     0,   186,     0,
       0,   146,   147,   148,   149,   150,   151,   152,   136,   137,
     138,   139,   140,   141,     0,   142,     0,     0,   143,   144,
     145,     0,     0,     0,     0,     0,   187,     0,     0,     0,
       0,     0,   188,     0,     0,   146,   147,   148,   149,   150,
     151,   152,   136,   137,   138,   139,   140,   141,     0,   142,
       0,     0,   143,   144,   145,     0,     0,     0,     0,     0,
     267,     0,     0,     0,     0,     0,     0,     0,     0,   146,
     147,   148,   149,   150,   151,   152,   136,   137,   138,   139,
     140,   141,     0,   142,     0,     0,   143,   144,   145,     0,
       0,     0,     0,     0,   268,     0,     0,     0,     0,     0,
       0,     0,     0,   146,   147,   148,   149,   150,   151,   152,
     136,   137,   138,   139,   140,   141,     0,   142,     0,     0,
     143,   144,   145,     0,     0,     0,     0,     0,   269,     0,
       0,     0,     0,     0,     0,     0,     0,   146,   147,   148,
     149,   150,   151,   152,   136,   137,   138,   139,   140,   141,
       0,   142,     0,     0,   143,   144,   145,     0,     0,     0,
       0,     0,   270,     0,     0,     0,     0,     0,     0,     0,
       0,   146,   147,   148,   149,   150,   151,   152,   136,   137,
     138,   139,   140,   141,     0,   142,     0,     0,   143,   144,
     145,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   202,     0,     0,   146,   147,   148,   149,   150,
     151,   152,   136,   137,   138,   139,   140,   141,     0,   142,
       0,     0,   143,   144,   145,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   236,   146,
     147,   148,   149,   150,   151,   152,     0,   203,   136,   137,
     138,   139,   140,   141,   178,   142,     0,     0,   143,   144,
     145,     0,   136,   137,   138,   139,   140,   141,     0,   142,
       0,     0,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,     0,     0,     0,     0,     0,     0,   146,
     147,   148,   149,   150,   151,   152,   136,   137,   138,   139,
     140,   141,     0,   142,     0,     0,   143,   144,   145,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   146,   147,   148,   149,   150,   151,   152,
       0,     0,     0,   193,   136,   137,   138,   139,   140,   141,
       0,   142,     0,     0,   143,   144,   145,     0,     0,     0,
       0,     0,   189,   190,     0,     0,     0,     0,     0,     0,
       0,   146,   147,   148,   149,   150,   151,   152,   136,   137,
     138,   139,   140,   141,     0,   142,     0,     0,   143,   144,
     145,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   146,   147,   148,   149,   150,
     151,   152
};

static const yytype_int16 yycheck[] =
{
       4,     0,    83,     7,     8,     9,     0,    46,    12,    13,
     161,    15,    14,    31,   278,    39,   263,    38,    22,   101,
      30,    25,    73,   101,   106,    76,    30,    31,   106,    33,
      34,    35,    36,    37,    38,    56,    75,   284,    73,    43,
      44,    45,    46,    82,    95,    96,    97,    98,    99,    53,
      30,    55,    56,    57,    73,    56,    56,    15,    16,   101,
     324,    96,    97,    98,    99,    69,    90,    68,    68,    53,
      12,    75,    30,    30,    92,    17,    80,   228,    82,    98,
      99,   101,    58,    59,    88,   166,   106,   105,    30,    31,
     103,    33,    13,   106,    15,    99,   103,    99,   103,   106,
      30,   106,   105,   106,    29,    30,    30,   101,    30,   203,
      30,    30,   102,    68,   102,   106,    48,   106,    30,    55,
     128,   129,    30,   127,   106,   106,   125,   131,    30,   106,
      30,    68,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,    30,
     154,    30,   156,   106,    55,    30,    55,    91,    56,    92,
     164,    30,    40,   257,   168,   169,    39,   171,   172,    30,
      55,   175,   176,    90,   178,   106,    91,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   281,    30,   283,
     198,   195,   304,   201,    91,    90,    90,   125,   202,   167,
      56,    56,    56,    -1,    -1,   299,    -1,   301,    -1,    66,
      67,    68,    69,    -1,   253,   309,    73,    -1,    -1,    76,
      77,    78,    -1,    -1,    -1,    -1,    -1,   321,    -1,    -1,
     311,   235,   236,    -1,   315,   329,    93,    94,    95,    96,
      97,    98,    99,    -1,    -1,    -1,    -1,   255,    -1,   253,
      -1,    -1,   260,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   267,   268,   269,   270,    -1,    -1,   277,
      -1,    -1,   276,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   292,    -1,   294,   291,    -1,    -1,
      -1,    -1,   300,    -1,    -1,   303,    -1,    -1,    -1,    -1,
      -1,   305,   310,    -1,    -1,    -1,   314,    -1,    -1,    -1,
      -1,   319,     0,     1,    -1,    -1,    -1,    -1,    -1,    -1,
     328,    -1,   330,    -1,    12,    -1,   334,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    -1,    25,    26,    27,
      -1,    29,    30,    31,    -1,    33,    -1,    -1,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    44,    -1,    -1,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    61,    62,    63,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    -1,    -1,    86,    87,
      88,    89,    -1,    -1,    -1,    -1,    -1,    -1,    96,    97,
      -1,    -1,    -1,   101,   102,    12,   104,    -1,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    -1,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    -1,    -1,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    44,    -1,    -1,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    61,    62,    63,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    79,    80,    81,    82,    83,    -1,    -1,    86,
      87,    88,    89,    -1,    -1,    -1,    -1,    -1,    -1,    96,
      97,    -1,    -1,    -1,    -1,   102,    12,   104,    -1,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    -1,    25,
      26,    27,    -1,    29,    30,    31,    -1,    33,    -1,    35,
      36,    -1,    -1,    -1,    -1,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    79,    80,    81,    82,    83,    -1,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      96,    97,    -1,    -1,    -1,   101,   102,    12,   104,    -1,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    -1,
      25,    26,    27,    -1,    29,    30,    31,    -1,    33,    -1,
      -1,    36,    -1,    -1,    -1,    -1,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    -1,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    96,    97,    -1,    -1,    -1,   101,   102,    12,   104,
      -1,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      -1,    25,    26,    27,    -1,    29,    30,    31,    -1,    33,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      -1,    -1,    -1,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    96,    97,    -1,    -1,    -1,    -1,   102,    12,
     104,    -1,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    -1,    25,    26,    27,    -1,    29,    30,    31,    -1,
      33,    -1,    -1,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    -1,    -1,    -1,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    12,    96,    97,    15,    16,    17,    -1,   102,
      -1,   104,    -1,    -1,    -1,    25,    26,    27,    -1,    29,
      30,    31,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    12,    -1,    -1,    15,    16,    17,    79,
      80,    81,    82,    83,    -1,    -1,    25,    26,    27,    89,
      29,    30,    31,    -1,    33,    -1,    96,    97,    -1,    -1,
      -1,    -1,   102,    -1,   104,    -1,    -1,    -1,    -1,    48,
      -1,    50,    -1,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    72,    12,    -1,    -1,    15,    16,    17,
      79,    80,    81,    82,    83,    -1,    -1,    25,    26,    27,
      89,    29,    30,    31,    -1,    33,    -1,    96,    97,    -1,
      -1,    -1,    -1,   102,    -1,   104,    -1,    12,    -1,    -1,
      15,    16,    17,    -1,    52,    -1,    54,    -1,    -1,    -1,
      25,    26,    27,    -1,    29,    30,    31,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    -1,    52,    -1,    54,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    96,    97,
      -1,    -1,    -1,    -1,   102,    -1,   104,    72,    -1,    -1,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    45,
      -1,    -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    96,    97,    -1,    -1,    -1,    48,   102,    50,   104,
      66,    67,    68,    69,    70,    71,    -1,    73,    74,    75,
      76,    77,    78,    -1,    66,    67,    68,    69,    70,    71,
      -1,    73,    -1,    -1,    76,    77,    78,    93,    94,    95,
      96,    97,    98,    99,    50,    -1,    -1,    -1,    -1,    -1,
      56,    93,    94,    95,    96,    97,    98,    99,    -1,    -1,
      66,    67,    68,    69,    70,    71,    -1,    73,    -1,    -1,
      76,    77,    78,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    93,    94,    95,
      96,    97,    98,    99,    66,    67,    68,    69,    70,    71,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    93,    94,    95,    96,    97,    98,    99,    66,    67,
      68,    69,    70,    71,    -1,    73,    -1,    -1,    76,    77,
      78,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99,    66,    67,    68,    69,    70,    71,    -1,    73,
      -1,    -1,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,    66,    67,    68,    69,
      70,    71,    -1,    73,    -1,    -1,    76,    77,    78,    -1,
      -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    98,    99,
      66,    67,    68,    69,    70,    71,    -1,    73,    -1,    -1,
      76,    77,    78,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    98,    99,    66,    67,    68,    69,    70,    71,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    98,    99,    66,    67,
      68,    69,    70,    71,    -1,    73,    -1,    -1,    76,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99,    66,    67,    68,    69,    70,    71,    -1,    73,
      -1,    -1,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    93,
      94,    95,    96,    97,    98,    99,    -1,   101,    66,    67,
      68,    69,    70,    71,    58,    73,    -1,    -1,    76,    77,
      78,    -1,    66,    67,    68,    69,    70,    71,    -1,    73,
      -1,    -1,    76,    77,    78,    93,    94,    95,    96,    97,
      98,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,    66,    67,    68,    69,
      70,    71,    -1,    73,    -1,    -1,    76,    77,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    98,    99,
      -1,    -1,    -1,   103,    66,    67,    68,    69,    70,    71,
      -1,    73,    -1,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    84,    85,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    98,    99,    66,    67,
      68,    69,    70,    71,    -1,    73,    -1,    -1,    76,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    15,    16,    17,    19,    20,    21,    22,
      23,    25,    26,    27,    29,    30,    31,    33,    36,    37,
      41,    42,    43,    44,    47,    52,    53,    54,    57,    61,
      62,    63,    65,    72,    79,    80,    81,    82,    83,    86,
      87,    88,    89,    96,    97,   102,   104,   108,   109,   110,
     111,   113,   114,   118,   119,   120,   125,   126,   127,   128,
     129,   130,   134,   136,   138,   141,   145,   101,    30,    52,
     128,   128,   128,   128,   147,    30,   128,   147,    30,   127,
     102,   147,   137,   140,    53,    30,   131,    46,    48,    49,
      50,    51,   128,   135,    30,   128,    58,    59,    15,    16,
      30,    30,    30,   128,    64,   128,   135,    30,   133,   128,
     128,   128,   128,   128,   128,    30,   132,    30,   102,   128,
     128,   128,   128,   146,     0,   101,   128,    68,   128,   112,
     117,   120,   128,   130,   145,   128,    66,    67,    68,    69,
      70,    71,    73,    76,    77,    78,    93,    94,    95,    96,
      97,    98,    99,   122,   102,   128,   106,   146,   128,   146,
     146,   122,   106,   128,    48,   135,   139,    45,    74,    75,
      30,    56,    68,   127,   128,    56,    68,    55,    58,   135,
     106,    50,    56,    50,    56,    50,    56,    50,    56,    84,
      85,   106,    30,   103,   105,   106,   109,   123,   128,   123,
     123,   128,    56,   101,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,    30,   142,   128,   103,   142,    30,   128,   122,    30,
     129,   128,   128,   128,   128,    68,    56,   128,   128,   128,
      30,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,    30,   103,   106,   128,   124,   123,    55,   123,   128,
     124,   101,   106,   101,   142,   128,   128,    50,    50,    50,
      50,   146,   101,   113,   123,    38,    56,   124,    55,   123,
     106,   144,    30,   144,   101,   128,   128,   128,   128,   103,
      91,    56,   128,   123,   111,    92,    30,   124,   124,   144,
     128,   123,    40,   115,   116,   121,   123,    35,   124,   123,
     124,    39,   123,   115,   128,    39,    90,    30,   143,   124,
     123,   122,    90,   123,   122,   106,   123,    91,   124,    55,
     111,    30,    91,   123,   124,   123,    90,   123,    90
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
#line 132 "engines/director/lingo/lingo-gr.y"
    { yyerrok; ;}
    break;

  case 8:
#line 140 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(4) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(2) - (4)].code);
		delete (yyvsp[(4) - (4)].s); ;}
    break;

  case 9:
#line 146 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_assign);
			(yyval.code) = (yyvsp[(2) - (4)].code); ;}
    break;

  case 10:
#line 149 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_after); ;}
    break;

  case 11:
#line 150 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_before); ;}
    break;

  case 12:
#line 151 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 13:
#line 157 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[1]);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 14:
#line 164 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_varpush);
		g_lingo->codeString((yyvsp[(2) - (4)].s)->c_str());
		g_lingo->code1(g_lingo->c_assign);
		(yyval.code) = (yyvsp[(4) - (4)].code);
		delete (yyvsp[(2) - (4)].s); ;}
    break;

  case 15:
#line 170 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (4)].e)[1]);
		(yyval.code) = (yyvsp[(4) - (4)].code); ;}
    break;

  case 16:
#line 177 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[1]);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 17:
#line 183 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_swap);
		g_lingo->code1(g_lingo->c_theentityassign);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[0]);
		g_lingo->codeInt((yyvsp[(2) - (5)].e)[1]);
		(yyval.code) = (yyvsp[(5) - (5)].code); ;}
    break;

  case 27:
#line 208 "engines/director/lingo/lingo-gr.y"
    {
		inst body = 0, end = 0;
		WRITE_UINT32(&body, (yyvsp[(3) - (6)].code) - (yyvsp[(1) - (6)].code));
		WRITE_UINT32(&end, (yyvsp[(5) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = body;	/* body of loop */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 2] = end; ;}
    break;

  case 28:
#line 219 "engines/director/lingo/lingo-gr.y"
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
#line 236 "engines/director/lingo/lingo-gr.y"
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
#line 248 "engines/director/lingo/lingo-gr.y"
    {
		inst end = 0;
		WRITE_UINT32(&end, (yyvsp[(3) - (3)].code) - (yyvsp[(1) - (3)].code));
		g_lingo->code1(STOP);
		(*g_lingo->_currentScript)[(yyvsp[(1) - (3)].code) + 1] = end; ;}
    break;

  case 31:
#line 253 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented"); ;}
    break;

  case 32:
#line 255 "engines/director/lingo/lingo-gr.y"
    {
			warning("STUB: TELL is not implemented");
		;}
    break;

  case 33:
#line 260 "engines/director/lingo/lingo-gr.y"
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
#line 270 "engines/director/lingo/lingo-gr.y"
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
#line 286 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(3) - (6)].code) - (yyvsp[(1) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 38:
#line 294 "engines/director/lingo/lingo-gr.y"
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
#line 304 "engines/director/lingo/lingo-gr.y"
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
#line 317 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 41:
#line 320 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 42:
#line 327 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 43:
#line 334 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 44:
#line 342 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 45:
#line 345 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 46:
#line 348 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 49:
#line 353 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 50:
#line 359 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_tellcode);
		g_lingo->code1(STOP); ;}
    break;

  case 51:
#line 363 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt((yyvsp[(1) - (1)].i)); ;}
    break;

  case 52:
#line 366 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_floatpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 53:
#line 369 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 54:
#line 372 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 55:
#line 375 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 56:
#line 381 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(1) - (1)].code); ;}
    break;

  case 58:
#line 383 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 59:
#line 386 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 60:
#line 389 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 61:
#line 390 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 62:
#line 393 "engines/director/lingo/lingo-gr.y"
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
#line 401 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 65:
#line 408 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 66:
#line 409 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 67:
#line 410 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 68:
#line 411 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 69:
#line 412 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); ;}
    break;

  case 70:
#line 413 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 71:
#line 414 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 72:
#line 415 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_eq); ;}
    break;

  case 73:
#line 416 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 74:
#line 417 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 75:
#line 418 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 76:
#line 419 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 77:
#line 420 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 78:
#line 421 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 79:
#line 422 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 80:
#line 423 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 81:
#line 424 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 82:
#line 425 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 83:
#line 426 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 84:
#line 427 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 85:
#line 428 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 86:
#line 429 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code1(g_lingo->c_arraypush); g_lingo->codeArray((yyvsp[(2) - (3)].narg)); ;}
    break;

  case 87:
#line 430 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 88:
#line 431 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 89:
#line 432 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); ;}
    break;

  case 90:
#line 433 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); ;}
    break;

  case 91:
#line 434 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); ;}
    break;

  case 92:
#line 435 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); ;}
    break;

  case 93:
#line 436 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); ;}
    break;

  case 94:
#line 437 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); ;}
    break;

  case 95:
#line 438 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); ;}
    break;

  case 96:
#line 439 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); ;}
    break;

  case 97:
#line 440 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 98:
#line 441 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].narg)); ;}
    break;

  case 99:
#line 442 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe(nullptr, 0); ;}
    break;

  case 100:
#line 445 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 101:
#line 450 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 104:
#line 453 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); ;}
    break;

  case 105:
#line 454 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 109:
#line 458 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 0);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 110:
#line 461 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 111:
#line 464 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeFunc((yyvsp[(1) - (2)].s), 1);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 112:
#line 467 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_voidpush);
		g_lingo->codeFunc((yyvsp[(1) - (1)].s), 1);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 113:
#line 471 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 114:
#line 472 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 115:
#line 473 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 116:
#line 474 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[(1) - (3)].s)); s += '-'; s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 117:
#line 477 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 118:
#line 478 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 119:
#line 481 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 120:
#line 482 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_property); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 121:
#line 485 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 122:
#line 486 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 123:
#line 497 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 124:
#line 498 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 125:
#line 499 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 126:
#line 500 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 127:
#line 504 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 128:
#line 508 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 131:
#line 518 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 132:
#line 519 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 133:
#line 523 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 134:
#line 527 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 135:
#line 531 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); ;}
    break;

  case 136:
#line 531 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[(1) - (3)].s), (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 137:
#line 561 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = kStateInArgs; g_lingo->_currentFactory.clear(); ;}
    break;

  case 138:
#line 562 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->clearArgStack();
			g_lingo->_indef = kStateNone; ;}
    break;

  case 139:
#line 567 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFactory(*(yyvsp[(2) - (2)].s)); ;}
    break;

  case 140:
#line 568 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = kStateInArgs; ;}
    break;

  case 141:
#line 569 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(1) - (7)].s), (yyvsp[(3) - (7)].code), (yyvsp[(4) - (7)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->clearArgStack();
			g_lingo->_indef = kStateNone; ;}
    break;

  case 142:
#line 574 "engines/director/lingo/lingo-gr.y"
    {	// D3
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(yyvsp[(1) - (8)].s), (yyvsp[(2) - (8)].code), (yyvsp[(3) - (8)].narg));
		g_lingo->clearArgStack();
		g_lingo->_indef = kStateNone;
		g_lingo->_ignoreMe = false;

		checkEnd((yyvsp[(7) - (8)].s), (yyvsp[(1) - (8)].s)->c_str(), false); ;}
    break;

  case 143:
#line 582 "engines/director/lingo/lingo-gr.y"
    {	// D4. No 'end' clause
		g_lingo->code1(g_lingo->c_procret);
		g_lingo->define(*(yyvsp[(1) - (6)].s), (yyvsp[(2) - (6)].code), (yyvsp[(3) - (6)].narg));
		g_lingo->_indef = kStateNone;
		g_lingo->clearArgStack();
		g_lingo->_ignoreMe = false; ;}
    break;

  case 144:
#line 589 "engines/director/lingo/lingo-gr.y"
    { (yyval.s) = (yyvsp[(2) - (2)].s); g_lingo->_indef = kStateInArgs; g_lingo->_currentFactory.clear(); g_lingo->_ignoreMe = true; ;}
    break;

  case 145:
#line 591 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 146:
#line 592 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 147:
#line 593 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 148:
#line 594 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 152:
#line 602 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); g_lingo->_indef = kStateInDef; ;}
    break;

  case 153:
#line 605 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (2)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(2) - (2)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 154:
#line 613 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 155:
#line 614 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 156:
#line 615 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 157:
#line 618 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 158:
#line 619 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2962 "engines/director/lingo/lingo-gr.cpp"
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


#line 622 "engines/director/lingo/lingo-gr.y"


