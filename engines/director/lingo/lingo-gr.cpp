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
     tON = 337,
     tME = 338
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
#define tON 337
#define tME 338




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
#line 302 "engines/director/lingo/lingo-gr.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 315 "engines/director/lingo/lingo-gr.cpp"

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
#define YYFINAL  115
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1612

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  99
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  42
/* YYNRULES -- Number of rules.  */
#define YYNRULES  154
/* YYNRULES -- Number of states.  */
#define YYNSTATES  337

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   338

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      91,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    90,    85,     2,
      92,    93,    88,    86,    98,    87,     2,    89,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      95,    84,    94,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    96,     2,    97,     2,     2,     2,     2,     2,     2,
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
      75,    76,    77,    78,    79,    80,    81,    82,    83
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    12,    14,    15,    17,    19,
      21,    26,    31,    36,    41,    46,    52,    57,    62,    68,
      70,    72,    74,    76,    84,    95,   107,   111,   119,   130,
     141,   148,   159,   170,   171,   175,   178,   180,   183,   185,
     192,   194,   201,   203,   207,   211,   214,   218,   220,   222,
     223,   224,   225,   228,   231,   235,   237,   239,   241,   243,
     245,   248,   253,   255,   257,   260,   262,   266,   270,   274,
     278,   282,   286,   290,   294,   298,   302,   306,   310,   313,
     317,   321,   325,   329,   332,   335,   339,   343,   348,   353,
     358,   365,   370,   377,   382,   389,   394,   401,   404,   406,
     408,   411,   413,   416,   419,   421,   424,   427,   429,   432,
     437,   444,   449,   452,   456,   458,   462,   464,   468,   471,
     474,   477,   480,   484,   487,   490,   492,   496,   499,   502,
     505,   509,   512,   513,   517,   518,   527,   530,   531,   540,
     541,   542,   553,   554,   556,   560,   565,   566,   569,   570,
     572,   576,   578,   582,   585
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     100,     0,    -1,   100,   101,   102,    -1,   102,    -1,     1,
     101,    -1,    91,    -1,    -1,   130,    -1,   137,    -1,   105,
      -1,    45,   121,    38,    24,    -1,    45,   121,    70,   121,
      -1,    45,   121,    71,   121,    -1,    47,    24,    84,   121,
      -1,    47,    12,    84,   121,    -1,    47,    13,   121,    84,
     121,    -1,    47,    24,    49,   121,    -1,    47,    12,    49,
     121,    -1,    47,    13,   121,    49,   121,    -1,   121,    -1,
     122,    -1,   104,    -1,   106,    -1,   113,    92,   112,    93,
     119,   118,    28,    -1,   114,    84,   121,   118,    49,   121,
     118,   119,   118,    28,    -1,   114,    84,   121,   118,    30,
      49,   121,   118,   119,   118,    28,    -1,   120,   121,   118,
      -1,   115,   112,    48,   101,   119,   118,    28,    -1,   115,
     112,    48,   101,   119,   118,    53,   119,   118,    28,    -1,
     115,   112,    48,   101,   119,   118,   117,   108,   118,    28,
      -1,   115,   112,    48,   117,   104,   118,    -1,   115,   112,
      48,   117,   104,   118,    53,   117,   104,   118,    -1,   115,
     112,    48,   117,   104,   118,   109,   118,   107,   118,    -1,
      -1,    53,   117,   104,    -1,   108,   111,    -1,   111,    -1,
     109,   110,    -1,   110,    -1,   116,   112,    48,   117,   105,
     118,    -1,   109,    -1,   116,   112,    48,   117,   119,   118,
      -1,   121,    -1,   121,    84,   121,    -1,    92,   112,    93,
      -1,    46,    52,    -1,    46,    51,    24,    -1,    37,    -1,
      32,    -1,    -1,    -1,    -1,   119,   101,    -1,   119,   105,
      -1,    50,    24,    48,    -1,    11,    -1,    14,    -1,    27,
      -1,    25,    -1,    22,    -1,    23,   121,    -1,    24,    92,
     138,    93,    -1,    24,    -1,    12,    -1,    13,   121,    -1,
     103,    -1,   121,    86,   121,    -1,   121,    87,   121,    -1,
     121,    88,   121,    -1,   121,    89,   121,    -1,   121,    69,
     121,    -1,   121,    94,   121,    -1,   121,    95,   121,    -1,
     121,    65,   121,    -1,   121,    60,   121,    -1,   121,    61,
     121,    -1,   121,    66,   121,    -1,   121,    67,   121,    -1,
      68,   121,    -1,   121,    85,   121,    -1,   121,    72,   121,
      -1,   121,    73,   121,    -1,   121,    74,   121,    -1,    86,
     121,    -1,    87,   121,    -1,    92,   121,    93,    -1,    96,
     138,    97,    -1,    79,   121,    80,   121,    -1,    79,   121,
      81,   121,    -1,    75,   121,    43,   121,    -1,    75,   121,
      49,   121,    43,   121,    -1,    76,   121,    43,   121,    -1,
      76,   121,    49,   121,    43,   121,    -1,    77,   121,    43,
     121,    -1,    77,   121,    49,   121,    43,   121,    -1,    78,
     121,    43,   121,    -1,    78,   121,    49,   121,    43,   121,
      -1,    45,   121,    -1,   125,    -1,   128,    -1,    33,    46,
      -1,    33,    -1,    35,   123,    -1,    59,   124,    -1,    16,
      -1,    18,   121,    -1,    17,   121,    -1,    17,    -1,    19,
     140,    -1,    83,    92,    24,    93,    -1,    83,    92,    24,
      98,   138,    93,    -1,    56,   121,    51,   121,    -1,    56,
     121,    -1,    20,    24,   138,    -1,    24,    -1,   123,    98,
      24,    -1,    24,    -1,   124,    98,    24,    -1,    36,    39,
      -1,    36,    42,    -1,    36,    44,    -1,    36,   126,    -1,
      36,   126,   127,    -1,    36,   127,    -1,    34,   121,    -1,
     121,    -1,    43,    41,   121,    -1,    41,   121,    -1,    57,
      58,    -1,    57,   126,    -1,    57,   126,   127,    -1,    57,
     127,    -1,    -1,    29,   129,   138,    -1,    -1,    40,    24,
     131,   117,   135,   101,   136,   119,    -1,    54,    24,    -1,
      -1,    55,    24,   132,   117,   135,   101,   136,   119,    -1,
      -1,    -1,    82,    24,   133,   117,   134,   135,   101,   136,
     119,    28,    -1,    -1,    24,    -1,   135,    98,    24,    -1,
     135,   101,    98,    24,    -1,    -1,    24,   139,    -1,    -1,
     121,    -1,   138,    98,   121,    -1,   121,    -1,   139,    98,
     121,    -1,   121,   101,    -1,   140,    98,   121,   101,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   118,   118,   119,   120,   123,   128,   129,   130,   131,
     134,   140,   141,   142,   148,   156,   164,   170,   178,   188,
     189,   192,   193,   198,   211,   229,   243,   251,   261,   273,
     285,   295,   305,   317,   318,   321,   322,   325,   326,   329,
     337,   338,   346,   347,   348,   351,   354,   361,   368,   376,
     379,   382,   383,   384,   387,   393,   394,   397,   400,   403,
     406,   409,   412,   416,   423,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   463,   464,   465,
     466,   467,   469,   470,   471,   474,   477,   480,   483,   484,
     485,   486,   487,   488,   491,   492,   495,   496,   507,   508,
     509,   510,   513,   516,   521,   522,   525,   526,   529,   530,
     533,   536,   539,   539,   569,   569,   575,   578,   578,   583,
     584,   583,   594,   595,   596,   597,   600,   604,   612,   613,
     614,   617,   618,   621,   622
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
  "tINTERSECTS", "tWITHIN", "tON", "tME", "'='", "'&'", "'+'", "'-'",
  "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "'>'", "'<'", "'['", "']'",
  "','", "$accept", "program", "nl", "programline", "asgn", "stmtoneliner",
  "stmt", "ifstmt", "elsestmtoneliner", "elseifstmt", "elseifstmtoneliner",
  "elseifstmtoneliner1", "elseifstmt1", "cond", "repeatwhile",
  "repeatwith", "if", "elseif", "begin", "end", "stmtlist", "when", "expr",
  "proc", "globallist", "instancelist", "gotofunc", "gotoframe",
  "gotomovie", "playfunc", "@1", "defn", "@2", "@3", "@4", "@5", "argdef",
  "argstore", "macro", "arglist", "nonemptyarglist", "nonemptyarglistnl", 0
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
     335,   336,   337,   338,    61,    38,    43,    45,    42,    47,
      37,    10,    40,    41,    62,    60,    91,    93,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    99,   100,   100,   100,   101,   102,   102,   102,   102,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   104,
     104,   105,   105,   105,   105,   105,   105,   106,   106,   106,
     106,   106,   106,   107,   107,   108,   108,   109,   109,   110,
     111,   111,   112,   112,   112,   113,   114,   115,   116,   117,
     118,   119,   119,   119,   120,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   123,   123,   124,   124,   125,   125,
     125,   125,   125,   125,   126,   126,   127,   127,   128,   128,
     128,   128,   129,   128,   131,   130,   130,   132,   130,   133,
     134,   130,   135,   135,   135,   135,   136,   137,   138,   138,
     138,   139,   139,   140,   140
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     2,     1,     0,     1,     1,     1,
       4,     4,     4,     4,     4,     5,     4,     4,     5,     1,
       1,     1,     1,     7,    10,    11,     3,     7,    10,    10,
       6,    10,    10,     0,     3,     2,     1,     2,     1,     6,
       1,     6,     1,     3,     3,     2,     3,     1,     1,     0,
       0,     0,     2,     2,     3,     1,     1,     1,     1,     1,
       2,     4,     1,     1,     2,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     2,     2,     3,     3,     4,     4,     4,
       6,     4,     6,     4,     6,     4,     6,     2,     1,     1,
       2,     1,     2,     2,     1,     2,     2,     1,     2,     4,
       6,     4,     2,     3,     1,     3,     1,     3,     2,     2,
       2,     2,     3,     2,     2,     1,     3,     2,     2,     2,
       3,     2,     0,     3,     0,     8,     2,     0,     8,     0,
       0,    10,     0,     1,     3,     4,     0,     2,     0,     1,
       3,     1,     3,     2,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    55,    63,     0,    56,   104,   107,     0,     0,
       0,    59,     0,    62,    58,    57,   132,   101,     0,     0,
      47,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   148,     0,     3,    65,    21,     9,    22,     0,
       0,     0,     0,    19,    20,    98,    99,     7,     8,     5,
       4,    62,     0,    64,   106,   105,     0,   108,   148,    60,
     148,   151,   147,   148,   100,   114,   102,     0,   118,     0,
     119,     0,   120,   125,   121,   123,   134,    97,     0,    45,
       0,     0,     0,     0,   136,   137,   112,   128,   129,   131,
     116,   103,    78,     0,     0,     0,     0,     0,   139,     0,
      83,    84,     0,   149,     0,     1,     6,     0,     0,     0,
       0,    42,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   148,
       0,   153,     0,   113,   149,     0,     0,   133,     0,   124,
     127,     0,   122,    49,     0,     0,     0,    46,     0,     0,
       0,     0,     0,    54,    49,     0,   130,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    49,     0,
      85,    86,     0,     2,     0,    50,     0,     0,    49,     0,
      26,    74,    75,    73,    76,    77,    70,    80,    81,    82,
      79,    66,    67,    68,    69,    71,    72,     0,    61,   152,
     115,   126,   142,    10,    11,    12,    17,    14,     0,     0,
      16,    13,   142,   111,   117,    89,     0,    91,     0,    93,
       0,    95,     0,    87,    88,   140,   109,   148,   150,    51,
       0,    44,    51,     0,    43,   154,   143,     0,    18,    15,
       0,     0,     0,     0,     0,   142,     0,    50,     0,     0,
      50,    50,     0,   146,   146,    90,    92,    94,    96,     0,
     110,    52,    53,     0,     0,    50,    49,    30,   144,     0,
      51,    51,   146,    23,    50,    51,    27,    51,     0,    48,
      49,    50,    38,     0,   145,   135,   138,    51,    51,    50,
      50,    50,    40,    36,     0,     0,    37,    33,     0,     0,
      50,     0,     0,    35,     0,     0,    50,    49,    50,    49,
     141,     0,    24,    28,    29,    49,    31,     0,    32,     0,
      25,    51,    34,    50,    50,    39,    41
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    43,   271,    44,    45,    46,   272,    48,   318,   301,
     302,   292,   303,   120,    49,    50,    51,   293,   212,   190,
     257,    52,    53,    54,    76,   101,    55,    84,    85,    56,
      73,    57,   153,   164,   178,   255,   247,   280,    58,   145,
      72,    67
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -264
static const yytype_int16 yypact[] =
{
     297,   -60,  -264,  -264,   907,  -264,  -264,   907,   907,   907,
      34,  -264,   907,   984,  -264,  -264,  -264,    29,    67,   804,
    -264,    71,   907,    46,    55,    75,    76,    84,   907,   881,
      85,   907,   907,   907,   907,   907,   907,    89,    25,   907,
     907,   907,   907,     2,  -264,  -264,  -264,  -264,  -264,    30,
      37,  1010,   907,  1517,  -264,  -264,  -264,  -264,  -264,  -264,
    -264,    31,   907,  1517,  1517,  1517,  1423,    43,   907,  1517,
     907,  1517,    44,   907,  -264,  -264,    47,   907,  -264,   907,
    -264,   103,  -264,  1517,   -16,  -264,  -264,  1043,   122,  -264,
     -38,   907,   -34,   100,  -264,  -264,  1330,  -264,   -16,  -264,
    -264,    51,   -56,  1075,  1107,  1139,  1171,  1361,  -264,   126,
     -56,   -56,  1455,  1517,    17,  -264,   383,  1010,   907,  1010,
     105,  1486,  1517,   907,   907,   907,   907,   907,   907,   907,
     907,   907,   907,   907,   907,   907,   907,   907,   907,   907,
    1043,  -264,   907,    53,  1455,   -32,   907,    53,   130,  1517,
    1517,   907,  -264,  -264,   131,   907,   907,  -264,   907,   907,
     704,   907,   907,  -264,  -264,   907,  -264,   137,   907,   907,
     907,   907,   907,   907,   907,   907,   907,   907,  -264,   -12,
    -264,  -264,   907,  -264,    72,  1517,    74,  1392,   -60,   907,
    -264,    16,    16,    16,   -56,   -56,   -56,  1517,    16,    16,
     120,   155,   155,   -56,   -56,  1517,  1517,  1423,  -264,  1517,
    -264,  1517,   140,  -264,  1517,  1517,  1517,  1517,   907,   907,
    1517,  1517,   140,  1517,  -264,  1517,  1203,  1517,  1235,  1517,
    1267,  1517,  1299,  1517,  1517,  -264,  -264,   907,  1517,  -264,
      -4,  -264,  -264,   727,  1517,  -264,  -264,    -2,  1517,  1517,
      -2,   907,   907,   907,   907,   140,    14,   555,   119,   907,
     555,  -264,   157,    81,    81,  1517,  1517,  1517,  1517,    -2,
    -264,  -264,  -264,   156,   907,  1517,    -6,   -25,  -264,   161,
    -264,  -264,    81,  -264,  1517,  -264,  -264,  -264,   162,  -264,
    -264,   162,  -264,  1010,  -264,   555,   555,  -264,  -264,   555,
     555,   162,   162,  -264,  1010,   727,  -264,   135,   145,   469,
     555,   167,   169,  -264,   170,   152,  -264,  -264,  -264,  -264,
    -264,   173,  -264,  -264,  -264,  -264,  -264,   727,  -264,   641,
    -264,   641,  -264,  -264,   555,  -264,  -264
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -264,  -264,    11,    86,  -264,  -240,     0,  -264,  -264,  -264,
     -74,  -243,   -97,  -113,  -264,  -264,  -264,  -237,   -94,   -27,
    -225,  -264,     1,  -264,  -264,  -264,  -264,   176,    -8,  -264,
    -264,  -264,  -264,  -264,  -264,  -264,  -198,  -263,  -264,   -24,
    -264,  -264
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -7
static const yytype_int16 yytable[] =
{
      47,   281,   115,   261,   184,    63,   186,   289,    64,    65,
      66,   158,    60,    69,    71,   161,   129,   260,   114,   297,
      83,    99,   286,    87,   250,    79,   258,    81,   290,    96,
      83,    59,   102,   103,   104,   105,   106,   107,   137,   138,
     110,   111,   112,   113,   143,   259,   159,   287,   306,   147,
     162,   304,   121,   122,   116,   295,   296,   269,    68,   306,
     299,   208,   300,   140,   304,   316,   182,    90,    91,   113,
     222,   144,   309,   310,   113,    74,   152,   141,   149,    92,
     150,   236,   126,   127,   235,   128,   237,   332,   129,    59,
     166,    75,   160,    59,   243,    86,   262,    88,    89,    93,
      94,   132,   133,   134,   135,   136,   334,   270,    95,   100,
     137,   138,   182,   108,   181,   182,    47,   109,   121,   185,
     187,   118,   117,   139,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     113,   142,   146,   207,   151,   148,   157,   209,   163,   167,
     179,   182,   211,   188,   210,   213,   214,   215,   240,   216,
     217,   224,   220,   221,   246,   239,   223,   241,   274,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   279,
     308,   278,   288,   238,   283,   294,   126,   127,   317,   128,
     244,   315,   129,   319,   289,   322,   305,   323,   324,   242,
     325,   330,   183,   291,   313,    98,   133,   134,   135,   136,
       0,     0,     0,   256,   137,   138,     0,     0,   245,   248,
     249,   126,   127,   327,   128,   329,     0,   129,     0,     0,
     273,   331,     0,   276,   277,     0,     0,     0,   113,     0,
       0,     0,     0,   135,   136,     0,     0,     0,   285,   137,
     138,     0,   265,   266,   267,   268,     0,   298,   263,     0,
     275,   264,     0,     0,   307,     0,     0,     0,     0,     0,
       0,     0,   311,   312,   314,   284,     0,     0,     0,     0,
     282,     0,     0,   321,     0,     0,     0,     0,     0,   326,
       0,   328,     0,     0,   121,     0,     0,    -6,     1,     0,
       0,     0,     0,     0,     0,   121,   335,   336,     2,     3,
       4,     5,     0,     6,     7,     8,     9,    10,     0,    11,
      12,    13,    14,     0,    15,     0,    16,     0,     0,   333,
      17,   333,    18,    19,    20,     0,     0,    21,     0,     0,
       0,     0,    22,    23,    24,     0,     0,    25,     0,     0,
       0,    26,    27,    28,    29,     0,    30,     0,     0,     0,
       0,     0,     0,     0,     0,    31,     0,     0,     0,     0,
       0,     0,    32,    33,    34,    35,    36,     0,     0,    37,
      38,     0,     0,    39,    40,     0,     0,     0,    -6,    41,
       0,     0,     0,    42,     2,     3,     4,     5,     0,     6,
       7,     8,     9,    10,     0,    11,    12,    13,    14,     0,
      15,     0,    16,     0,     0,     0,    17,     0,    18,    19,
      20,     0,     0,    21,     0,     0,     0,     0,    22,    23,
      24,     0,     0,    25,     0,     0,     0,    26,    27,    28,
      29,     0,    30,     0,     0,     0,     0,     0,     0,     0,
       0,    31,     0,     0,     0,     0,     0,     0,    32,    33,
      34,    35,    36,     0,     0,    37,    38,     0,     0,    39,
      40,     0,     0,     0,     0,    41,     0,     0,     0,    42,
       2,     3,     4,     5,     0,     6,     7,     8,     9,    10,
       0,    11,    12,    61,    14,     0,    15,   320,    16,     0,
       0,     0,    17,     0,    18,    19,    20,     0,     0,     0,
       0,     0,     0,     0,    22,    23,    24,     0,     0,    25,
       0,     0,     0,     0,     0,    28,    29,     0,    30,     0,
       0,     0,     0,     0,     0,     0,     0,    31,     0,     0,
       0,     0,     0,     0,    32,    33,    34,    35,    36,     0,
       0,     0,    38,     0,     0,    39,    40,     0,     0,     0,
      59,    41,     0,     0,     0,    42,     2,     3,     4,     5,
       0,     6,     7,     8,     9,    10,     0,    11,    12,    61,
      14,     0,    15,     0,    16,     0,     0,     0,    17,     0,
      18,    19,    20,     0,     0,     0,     0,     0,     0,     0,
      22,    23,    24,     0,     0,    25,     0,     0,     0,     0,
       0,    28,    29,     0,    30,     0,     0,     0,     0,     0,
       0,     0,     0,    31,     0,     0,     0,     0,     0,     0,
      32,    33,    34,    35,    36,     0,     0,     0,    38,     0,
       0,    39,    40,     0,     0,     0,    59,    41,     0,     0,
       0,    42,     2,     3,     4,     5,     0,     6,     7,     8,
       9,    10,     0,    11,    12,    61,    14,     0,    15,     0,
      16,     0,     0,     0,    17,     0,    18,    19,    20,     0,
       0,     0,     0,     0,     0,     0,    22,    23,    24,     0,
       0,    25,     0,     0,     0,     0,     0,    28,    29,     0,
      30,     0,     0,     0,     0,     0,     0,     0,     0,    31,
       0,     0,     0,     0,     0,     0,    32,    33,    34,    35,
      36,     0,     0,     0,    38,     0,     0,    39,    40,     0,
       0,     0,     0,    41,     0,     0,     0,    42,     2,     3,
       4,     5,     0,     6,     7,     8,     9,    10,     0,    11,
      12,    61,    14,   218,    15,     0,    16,     0,     0,     0,
      17,     0,    18,    19,   123,   124,     0,     0,     0,   125,
     126,   127,    22,   128,    24,     0,   129,   130,   131,     0,
       0,     0,     0,    28,    29,     0,    30,     0,   219,   132,
     133,   134,   135,   136,     0,    31,     0,     0,   137,   138,
       0,     0,    32,    33,    34,    35,    36,     0,     0,     0,
      38,     0,     0,    39,    40,     2,     3,     4,     5,    41,
       0,     0,     0,    42,     0,     0,    11,    12,    61,    14,
       0,    15,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,    78,     0,    79,    80,    81,    82,    62,
       0,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    31,     0,     0,     0,     0,     0,     0,    32,
      33,    34,    35,    36,     0,     0,     0,     0,     0,     0,
      39,    40,     2,     3,     4,     5,    41,     0,     0,     0,
      42,     0,     0,    11,    12,    61,    14,     0,    15,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     2,     3,
       4,     5,    79,     0,    81,     0,    62,     0,    24,    11,
      12,    61,    14,     0,    15,     0,     0,     0,     0,    97,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
       0,     0,    62,     0,    24,     0,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,    39,    40,     0,
       0,     0,     0,    41,     0,    31,     0,    42,     0,     0,
       0,     0,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,     0,    39,    40,     2,     3,     4,     5,    41,
       0,     0,     0,    42,     0,     0,    11,    12,    61,    14,
       0,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     2,     3,     4,     5,     0,     0,     0,     0,    62,
       0,    24,    11,    12,    61,    14,     0,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    31,     0,     0,    62,     0,    24,     0,    32,
      33,    34,    35,    36,     0,     0,     0,     0,     0,     0,
      39,    40,     0,     0,     0,     0,    70,     0,    31,     0,
      42,   154,     0,     0,     0,    32,    33,    34,    35,    36,
       0,     0,     0,     0,     0,     0,    39,    40,     0,     0,
       0,     0,   119,   123,   124,     0,    42,     0,   125,   126,
     127,     0,   128,   155,   156,   129,   130,   131,   168,     0,
       0,     0,     0,     0,   169,     0,     0,     0,   132,   133,
     134,   135,   136,     0,     0,   123,   124,   137,   138,     0,
     125,   126,   127,     0,   128,     0,     0,   129,   130,   131,
     170,     0,     0,     0,     0,     0,   171,     0,     0,     0,
     132,   133,   134,   135,   136,     0,     0,   123,   124,   137,
     138,     0,   125,   126,   127,     0,   128,     0,     0,   129,
     130,   131,   172,     0,     0,     0,     0,     0,   173,     0,
       0,     0,   132,   133,   134,   135,   136,     0,     0,   123,
     124,   137,   138,     0,   125,   126,   127,     0,   128,     0,
       0,   129,   130,   131,   174,     0,     0,     0,     0,     0,
     175,     0,     0,     0,   132,   133,   134,   135,   136,     0,
       0,   123,   124,   137,   138,     0,   125,   126,   127,     0,
     128,     0,     0,   129,   130,   131,   251,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   132,   133,   134,   135,
     136,     0,     0,   123,   124,   137,   138,     0,   125,   126,
     127,     0,   128,     0,     0,   129,   130,   131,   252,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   132,   133,
     134,   135,   136,     0,     0,   123,   124,   137,   138,     0,
     125,   126,   127,     0,   128,     0,     0,   129,   130,   131,
     253,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     132,   133,   134,   135,   136,     0,     0,   123,   124,   137,
     138,     0,   125,   126,   127,     0,   128,     0,     0,   129,
     130,   131,   254,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   132,   133,   134,   135,   136,     0,     0,   123,
     124,   137,   138,     0,   125,   126,   127,     0,   128,     0,
       0,   129,   130,   131,     0,     0,     0,     0,     0,     0,
       0,   165,     0,     0,   132,   133,   134,   135,   136,     0,
     123,   124,     0,   137,   138,   125,   126,   127,     0,   128,
       0,     0,   129,   130,   131,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   132,   133,   134,   135,   136,
       0,   123,   124,     0,   137,   138,   125,   126,   127,     0,
     128,     0,     0,   129,   130,   131,     0,     0,     0,     0,
       0,   176,   177,     0,     0,     0,   132,   133,   134,   135,
     136,     0,   123,   124,     0,   137,   138,   125,   126,   127,
       0,   128,     0,     0,   129,   130,   131,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   189,   132,   133,   134,
     135,   136,     0,   123,   124,   180,   137,   138,   125,   126,
     127,     0,   128,     0,     0,   129,   130,   131,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   132,   133,
     134,   135,   136,     0,    59,   123,   124,   137,   138,     0,
     125,   126,   127,     0,   128,     0,     0,   129,   130,   131,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     132,   133,   134,   135,   136,     0,   123,   124,   180,   137,
     138,   125,   126,   127,     0,   128,     0,     0,   129,   130,
     131,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     189,   132,   133,   134,   135,   136,     0,   123,   124,     0,
     137,   138,   125,   126,   127,     0,   128,     0,     0,   129,
     130,   131,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   132,   133,   134,   135,   136,     0,     0,     0,
       0,   137,   138
};

static const yytype_int16 yycheck[] =
{
       0,   264,     0,   243,   117,     4,   119,    32,     7,     8,
       9,    49,     1,    12,    13,    49,    72,   242,    42,   282,
      19,    29,    28,    22,   222,    41,    30,    43,    53,    28,
      29,    91,    31,    32,    33,    34,    35,    36,    94,    95,
      39,    40,    41,    42,    68,    49,    84,    53,   291,    73,
      84,   288,    51,    52,    43,   280,   281,   255,    24,   302,
     285,    93,   287,    62,   301,   305,    98,    12,    13,    68,
     164,    70,   297,   298,    73,    46,    84,    66,    77,    24,
      79,    93,    66,    67,   178,    69,    98,   327,    72,    91,
      98,    24,    91,    91,   188,    24,    98,    51,    52,    24,
      24,    85,    86,    87,    88,    89,   331,    93,    24,    24,
      94,    95,    98,    24,    97,    98,   116,    92,   117,   118,
     119,    84,    92,    92,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    98,    98,   142,    41,    98,    24,   146,    48,    98,
      24,    98,   151,    48,    24,    24,   155,   156,   185,   158,
     159,    24,   161,   162,    24,    93,   165,    93,    49,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,    98,
     293,    24,   276,   182,    28,    24,    66,    67,    53,    69,
     189,   304,    72,    48,    32,    28,   290,    28,    28,   188,
      48,    28,   116,   277,   301,    29,    86,    87,    88,    89,
      -1,    -1,    -1,   237,    94,    95,    -1,    -1,   207,   218,
     219,    66,    67,   317,    69,   319,    -1,    72,    -1,    -1,
     257,   325,    -1,   260,   261,    -1,    -1,    -1,   237,    -1,
      -1,    -1,    -1,    88,    89,    -1,    -1,    -1,   275,    94,
      95,    -1,   251,   252,   253,   254,    -1,   284,   247,    -1,
     259,   250,    -1,    -1,   291,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   299,   300,   301,   274,    -1,    -1,    -1,    -1,
     269,    -1,    -1,   310,    -1,    -1,    -1,    -1,    -1,   316,
      -1,   318,    -1,    -1,   293,    -1,    -1,     0,     1,    -1,
      -1,    -1,    -1,    -1,    -1,   304,   333,   334,    11,    12,
      13,    14,    -1,    16,    17,    18,    19,    20,    -1,    22,
      23,    24,    25,    -1,    27,    -1,    29,    -1,    -1,   329,
      33,   331,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    50,    -1,    -1,
      -1,    54,    55,    56,    57,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    -1,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      -1,    -1,    -1,    96,    11,    12,    13,    14,    -1,    16,
      17,    18,    19,    20,    -1,    22,    23,    24,    25,    -1,
      27,    -1,    29,    -1,    -1,    -1,    33,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    50,    -1,    -1,    -1,    54,    55,    56,
      57,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    -1,    -1,    86,
      87,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    96,
      11,    12,    13,    14,    -1,    16,    17,    18,    19,    20,
      -1,    22,    23,    24,    25,    -1,    27,    28,    29,    -1,
      -1,    -1,    33,    -1,    35,    36,    37,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    83,    -1,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    -1,    -1,    -1,    96,    11,    12,    13,    14,
      -1,    16,    17,    18,    19,    20,    -1,    22,    23,    24,
      25,    -1,    27,    -1,    29,    -1,    -1,    -1,    33,    -1,
      35,    36,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    56,    57,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    83,    -1,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    -1,    -1,
      -1,    96,    11,    12,    13,    14,    -1,    16,    17,    18,
      19,    20,    -1,    22,    23,    24,    25,    -1,    27,    -1,
      29,    -1,    -1,    -1,    33,    -1,    35,    36,    37,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    56,    57,    -1,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    83,    -1,    -1,    86,    87,    -1,
      -1,    -1,    -1,    92,    -1,    -1,    -1,    96,    11,    12,
      13,    14,    -1,    16,    17,    18,    19,    20,    -1,    22,
      23,    24,    25,    49,    27,    -1,    29,    -1,    -1,    -1,
      33,    -1,    35,    36,    60,    61,    -1,    -1,    -1,    65,
      66,    67,    45,    69,    47,    -1,    72,    73,    74,    -1,
      -1,    -1,    -1,    56,    57,    -1,    59,    -1,    84,    85,
      86,    87,    88,    89,    -1,    68,    -1,    -1,    94,    95,
      -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      83,    -1,    -1,    86,    87,    11,    12,    13,    14,    92,
      -1,    -1,    -1,    96,    -1,    -1,    22,    23,    24,    25,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    34,    -1,
      -1,    -1,    -1,    39,    -1,    41,    42,    43,    44,    45,
      -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    11,    12,    13,    14,    92,    -1,    -1,    -1,
      96,    -1,    -1,    22,    23,    24,    25,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,    11,    12,
      13,    14,    41,    -1,    43,    -1,    45,    -1,    47,    22,
      23,    24,    25,    -1,    27,    -1,    -1,    -1,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    45,    -1,    47,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    -1,
      -1,    -1,    -1,    92,    -1,    68,    -1,    96,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    11,    12,    13,    14,    92,
      -1,    -1,    -1,    96,    -1,    -1,    22,    23,    24,    25,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,    45,
      -1,    47,    22,    23,    24,    25,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    45,    -1,    47,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    -1,    -1,    -1,    -1,    92,    -1,    68,    -1,
      96,    38,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    -1,    -1,
      -1,    -1,    92,    60,    61,    -1,    96,    -1,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    43,    -1,
      -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,    85,    86,
      87,    88,    89,    -1,    -1,    60,    61,    94,    95,    -1,
      65,    66,    67,    -1,    69,    -1,    -1,    72,    73,    74,
      43,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      85,    86,    87,    88,    89,    -1,    -1,    60,    61,    94,
      95,    -1,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    43,    -1,    -1,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    85,    86,    87,    88,    89,    -1,    -1,    60,
      61,    94,    95,    -1,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    43,    -1,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    85,    86,    87,    88,    89,    -1,
      -1,    60,    61,    94,    95,    -1,    65,    66,    67,    -1,
      69,    -1,    -1,    72,    73,    74,    43,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    85,    86,    87,    88,
      89,    -1,    -1,    60,    61,    94,    95,    -1,    65,    66,
      67,    -1,    69,    -1,    -1,    72,    73,    74,    43,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,    86,
      87,    88,    89,    -1,    -1,    60,    61,    94,    95,    -1,
      65,    66,    67,    -1,    69,    -1,    -1,    72,    73,    74,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      85,    86,    87,    88,    89,    -1,    -1,    60,    61,    94,
      95,    -1,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    85,    86,    87,    88,    89,    -1,    -1,    60,
      61,    94,    95,    -1,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    85,    86,    87,    88,    89,    -1,
      60,    61,    -1,    94,    95,    65,    66,    67,    -1,    69,
      -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    85,    86,    87,    88,    89,
      -1,    60,    61,    -1,    94,    95,    65,    66,    67,    -1,
      69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,
      -1,    80,    81,    -1,    -1,    -1,    85,    86,    87,    88,
      89,    -1,    60,    61,    -1,    94,    95,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    84,    85,    86,    87,
      88,    89,    -1,    60,    61,    93,    94,    95,    65,    66,
      67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,    86,
      87,    88,    89,    -1,    91,    60,    61,    94,    95,    -1,
      65,    66,    67,    -1,    69,    -1,    -1,    72,    73,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      85,    86,    87,    88,    89,    -1,    60,    61,    93,    94,
      95,    65,    66,    67,    -1,    69,    -1,    -1,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      84,    85,    86,    87,    88,    89,    -1,    60,    61,    -1,
      94,    95,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    85,    86,    87,    88,    89,    -1,    -1,    -1,
      -1,    94,    95
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    11,    12,    13,    14,    16,    17,    18,    19,
      20,    22,    23,    24,    25,    27,    29,    33,    35,    36,
      37,    40,    45,    46,    47,    50,    54,    55,    56,    57,
      59,    68,    75,    76,    77,    78,    79,    82,    83,    86,
      87,    92,    96,   100,   102,   103,   104,   105,   106,   113,
     114,   115,   120,   121,   122,   125,   128,   130,   137,    91,
     101,    24,    45,   121,   121,   121,   121,   140,    24,   121,
      92,   121,   139,   129,    46,    24,   123,    34,    39,    41,
      42,    43,    44,   121,   126,   127,    24,   121,    51,    52,
      12,    13,    24,    24,    24,    24,   121,    58,   126,   127,
      24,   124,   121,   121,   121,   121,   121,   121,    24,    92,
     121,   121,   121,   121,   138,     0,   101,    92,    84,    92,
     112,   121,   121,    60,    61,    65,    66,    67,    69,    72,
      73,    74,    85,    86,    87,    88,    89,    94,    95,    92,
     121,   101,    98,   138,   121,   138,    98,   138,    98,   121,
     121,    41,   127,   131,    38,    70,    71,    24,    49,    84,
     121,    49,    84,    48,   132,    51,   127,    98,    43,    49,
      43,    49,    43,    49,    43,    49,    80,    81,   133,    24,
      93,    97,    98,   102,   112,   121,   112,   121,    48,    84,
     118,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,    93,   121,
      24,   121,   117,    24,   121,   121,   121,   121,    49,    84,
     121,   121,   117,   121,    24,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   117,    93,    98,   121,    93,
     118,    93,   101,   117,   121,   101,    24,   135,   121,   121,
     135,    43,    43,    43,    43,   134,   138,   119,    30,    49,
     119,   104,    98,   101,   101,   121,   121,   121,   121,   135,
      93,   101,   105,   118,    49,   121,   118,   118,    24,    98,
     136,   136,   101,    28,   121,   118,    28,    53,   117,    32,
      53,   109,   110,   116,    24,   119,   119,   136,   118,   119,
     119,   108,   109,   111,   116,   117,   110,   118,   112,   119,
     119,   118,   118,   111,   118,   112,   104,    53,   107,    48,
      28,   118,    28,    28,    28,    48,   118,   117,   118,   117,
      28,   117,   104,   105,   119,   118,   118
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
#line 251 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0, end = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (7)].code));
		WRITE_UINT32(&end, (yyvsp[(6) - (7)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 1] = then;	/* thenpart */
		(*g_lingo->_currentScript)[(yyvsp[(1) - (7)].code) + 3] = end;	/* end, if cond fails */

		checkEnd((yyvsp[(7) - (7)].s), "if", true);

		g_lingo->processIf(0, 0); ;}
    break;

  case 28:
#line 261 "engines/director/lingo/lingo-gr.y"
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

  case 29:
#line 273 "engines/director/lingo/lingo-gr.y"
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

  case 30:
#line 285 "engines/director/lingo/lingo-gr.y"
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
#line 295 "engines/director/lingo/lingo-gr.y"
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
#line 305 "engines/director/lingo/lingo-gr.y"
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
#line 317 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = 0; ;}
    break;

  case 34:
#line 318 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 39:
#line 329 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(4) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 41:
#line 338 "engines/director/lingo/lingo-gr.y"
    {
		inst then = 0;
		WRITE_UINT32(&then, (yyvsp[(5) - (6)].code));
		(*g_lingo->_currentScript)[(yyvsp[(1) - (6)].code) + 1] = then;	/* thenpart */

		g_lingo->codeLabel((yyvsp[(1) - (6)].code)); ;}
    break;

  case 42:
#line 346 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); ;}
    break;

  case 43:
#line 347 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_eq, STOP); ;}
    break;

  case 45:
#line 351 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->code3(g_lingo->c_repeatwhilecode, STOP, STOP); ;}
    break;

  case 46:
#line 354 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code3(g_lingo->c_repeatwithcode, STOP, STOP);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str());
		delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 47:
#line 361 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(0);  // Do not skip end
		g_lingo->codeLabel(0); ;}
    break;

  case 48:
#line 368 "engines/director/lingo/lingo-gr.y"
    {
		inst skipEnd;
		WRITE_UINT32(&skipEnd, 1); // We have to skip end to avoid multiple executions
		(yyval.code) = g_lingo->code1(g_lingo->c_ifcode);
		g_lingo->code3(STOP, STOP, STOP);
		g_lingo->code1(skipEnd); ;}
    break;

  case 49:
#line 376 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 50:
#line 379 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(STOP); (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 51:
#line 382 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->_currentScript->size(); ;}
    break;

  case 54:
#line 387 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_whencode);
		g_lingo->code1(STOP);
		g_lingo->codeString((yyvsp[(2) - (3)].s)->c_str());
		delete (yyvsp[(2) - (3)].s); ;}
    break;

  case 55:
#line 393 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->codeConst((yyvsp[(1) - (1)].i)); ;}
    break;

  case 56:
#line 394 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_fconstpush);
		g_lingo->codeFloat((yyvsp[(1) - (1)].f)); ;}
    break;

  case 57:
#line 397 "engines/director/lingo/lingo-gr.y"
    {											// D3
		(yyval.code) = g_lingo->code1(g_lingo->c_symbolpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 58:
#line 400 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_stringpush);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); ;}
    break;

  case 59:
#line 403 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 60:
#line 406 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 61:
#line 409 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeFunc((yyvsp[(1) - (4)].s), (yyvsp[(3) - (4)].narg));
		delete (yyvsp[(1) - (4)].s); ;}
    break;

  case 62:
#line 412 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_eval);
		g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str());
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 63:
#line 416 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->codeConst(0); // Put dummy id
		g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (1)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (1)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 64:
#line 423 "engines/director/lingo/lingo-gr.y"
    {
		(yyval.code) = g_lingo->code1(g_lingo->c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[(1) - (2)].e)[0]);
		WRITE_UINT32(&f, (yyvsp[(1) - (2)].e)[1]);
		g_lingo->code2(e, f); ;}
    break;

  case 66:
#line 430 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_add); ;}
    break;

  case 67:
#line 431 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_sub); ;}
    break;

  case 68:
#line 432 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mul); ;}
    break;

  case 69:
#line 433 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_div); ;}
    break;

  case 70:
#line 434 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_mod); ;}
    break;

  case 71:
#line 435 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gt); ;}
    break;

  case 72:
#line 436 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lt); ;}
    break;

  case 73:
#line 437 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_neq); ;}
    break;

  case 74:
#line 438 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ge); ;}
    break;

  case 75:
#line 439 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_le); ;}
    break;

  case 76:
#line 440 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_and); ;}
    break;

  case 77:
#line 441 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_or); ;}
    break;

  case 78:
#line 442 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_not); ;}
    break;

  case 79:
#line 443 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_ampersand); ;}
    break;

  case 80:
#line 444 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_concat); ;}
    break;

  case 81:
#line 445 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_contains); ;}
    break;

  case 82:
#line 446 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_starts); ;}
    break;

  case 83:
#line 447 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); ;}
    break;

  case 84:
#line 448 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (2)].code); g_lingo->code1(g_lingo->c_negate); ;}
    break;

  case 85:
#line 449 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = (yyvsp[(2) - (3)].code); ;}
    break;

  case 86:
#line 450 "engines/director/lingo/lingo-gr.y"
    { (yyval.code) = g_lingo->codeArray((yyvsp[(2) - (3)].narg)); ;}
    break;

  case 87:
#line 451 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_intersects); ;}
    break;

  case 88:
#line 452 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_within); ;}
    break;

  case 89:
#line 453 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charOf); ;}
    break;

  case 90:
#line 454 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_charToOf); ;}
    break;

  case 91:
#line 455 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemOf); ;}
    break;

  case 92:
#line 456 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_itemToOf); ;}
    break;

  case 93:
#line 457 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineOf); ;}
    break;

  case 94:
#line 458 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_lineToOf); ;}
    break;

  case 95:
#line 459 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordOf); ;}
    break;

  case 96:
#line 460 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_wordToOf); ;}
    break;

  case 97:
#line 463 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_printtop); ;}
    break;

  case 100:
#line 466 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_exitRepeat); ;}
    break;

  case 101:
#line 467 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeConst(0); // Push fake value on stack
							  g_lingo->code1(g_lingo->c_procret); ;}
    break;

  case 104:
#line 471 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 105:
#line 474 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 106:
#line 477 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->_builtins[*(yyvsp[(1) - (2)].s)]->u.func);
		delete (yyvsp[(1) - (2)].s); ;}
    break;

  case 107:
#line 480 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code2(g_lingo->c_voidpush, g_lingo->_builtins[*(yyvsp[(1) - (1)].s)]->u.func);
		delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 108:
#line 483 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeFunc((yyvsp[(1) - (2)].s), (yyvsp[(2) - (2)].narg)); ;}
    break;

  case 109:
#line 484 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (4)].s), 0); ;}
    break;

  case 110:
#line 485 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeMe((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].narg)); ;}
    break;

  case 111:
#line 486 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_open); ;}
    break;

  case 112:
#line 487 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code2(g_lingo->c_voidpush, g_lingo->c_open); ;}
    break;

  case 113:
#line 488 "engines/director/lingo/lingo-gr.y"
    { Common::String s(*(yyvsp[(1) - (3)].s)); s += '-'; s += *(yyvsp[(2) - (3)].s); g_lingo->codeFunc(&s, (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 114:
#line 491 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 115:
#line 492 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_global); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 116:
#line 495 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(1) - (1)].s)->c_str()); delete (yyvsp[(1) - (1)].s); ;}
    break;

  case 117:
#line 496 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_instance); g_lingo->codeString((yyvsp[(3) - (3)].s)->c_str()); delete (yyvsp[(3) - (3)].s); ;}
    break;

  case 118:
#line 507 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoloop); ;}
    break;

  case 119:
#line 508 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotonext); ;}
    break;

  case 120:
#line 509 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_gotoprevious); ;}
    break;

  case 121:
#line 510 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 122:
#line 513 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 123:
#line 516 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_goto); ;}
    break;

  case 128:
#line 529 "engines/director/lingo/lingo-gr.y"
    { g_lingo->code1(g_lingo->c_playdone); ;}
    break;

  case 129:
#line 530 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(1);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 130:
#line 533 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(3);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 131:
#line 536 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeConst(2);
		g_lingo->code1(g_lingo->c_play); ;}
    break;

  case 132:
#line 539 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeSetImmediate(true); ;}
    break;

  case 133:
#line 539 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[(1) - (3)].s), (yyvsp[(3) - (3)].narg)); ;}
    break;

  case 134:
#line 569 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 135:
#line 570 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeConst(0); // Push fake value on stack
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg));
			g_lingo->_indef = false; ;}
    break;

  case 136:
#line 575 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->codeFactory(*(yyvsp[(2) - (2)].s));
		;}
    break;

  case 137:
#line 578 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; ;}
    break;

  case 138:
#line 579 "engines/director/lingo/lingo-gr.y"
    {
			g_lingo->code1(g_lingo->c_procret);
			g_lingo->define(*(yyvsp[(2) - (8)].s), (yyvsp[(4) - (8)].code), (yyvsp[(5) - (8)].narg) + 1, &g_lingo->_currentFactory);
			g_lingo->_indef = false; ;}
    break;

  case 139:
#line 583 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_indef = true; g_lingo->_currentFactory.clear(); ;}
    break;

  case 140:
#line 584 "engines/director/lingo/lingo-gr.y"
    { g_lingo->_ignoreMe = true; ;}
    break;

  case 141:
#line 584 "engines/director/lingo/lingo-gr.y"
    {
				g_lingo->codeConst(0); // Push fake value on stack
				g_lingo->code1(g_lingo->c_procret);
				g_lingo->define(*(yyvsp[(2) - (10)].s), (yyvsp[(4) - (10)].code), (yyvsp[(6) - (10)].narg));
				g_lingo->_indef = false;
				g_lingo->_ignoreMe = false;

				checkEnd((yyvsp[(10) - (10)].s), (yyvsp[(2) - (10)].s)->c_str(), false);
			;}
    break;

  case 142:
#line 594 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 143:
#line 595 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(1) - (1)].s)); (yyval.narg) = 1; ;}
    break;

  case 144:
#line 596 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(3) - (3)].s)); (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 145:
#line 597 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArg((yyvsp[(4) - (4)].s)); (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;

  case 146:
#line 600 "engines/director/lingo/lingo-gr.y"
    { g_lingo->codeArgStore(); ;}
    break;

  case 147:
#line 604 "engines/director/lingo/lingo-gr.y"
    {
		g_lingo->code1(g_lingo->c_call);
		g_lingo->codeString((yyvsp[(1) - (2)].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[(2) - (2)].narg));
		g_lingo->code1(numpar); ;}
    break;

  case 148:
#line 612 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 0; ;}
    break;

  case 149:
#line 613 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 150:
#line 614 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 151:
#line 617 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 152:
#line 618 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (3)].narg) + 1; ;}
    break;

  case 153:
#line 621 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = 1; ;}
    break;

  case 154:
#line 622 "engines/director/lingo/lingo-gr.y"
    { (yyval.narg) = (yyvsp[(1) - (4)].narg) + 1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2952 "engines/director/lingo/lingo-gr.cpp"
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


#line 625 "engines/director/lingo/lingo-gr.y"


