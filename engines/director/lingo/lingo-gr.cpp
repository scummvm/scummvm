/* A Bison parser, made by GNU Bison 3.6.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.6.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 54 "engines/director/lingo/lingo-gr.y"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/endian.h"
#include "common/hash-str.h"
#include "common/rect.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-object.h"

extern int yylex();
extern int yyparse();

using namespace Director;

static void yyerror(const char *s) {
	g_lingo->_hadError = true;
	warning("######################  LINGO: %s at line %d col %d", s, g_lingo->_linenumber, g_lingo->_colnumber);
}

static void checkEnd(Common::String *token, const char *expect, bool required) {
	if (required) {
		if (token->compareToIgnoreCase(expect)) {
			Common::String err = Common::String::format("end mismatch. Expected %s but got %s", expect, token->c_str());
			yyerror(err.c_str());
		}
	}
}

static void inArgs() { g_lingo->_indefStore = g_lingo->_indef; g_lingo->_indef = kStateInArgs; }
static void inDef()  { g_lingo->_indefStore = g_lingo->_indef; g_lingo->_indef = kStateInDef; }
static void inNone() { g_lingo->_indefStore = g_lingo->_indef; g_lingo->_indef = kStateNone; }
static void inLast() { g_lingo->_indef = g_lingo->_indefStore; }

static void startDef() {
	inArgs();
	g_lingo->_methodVarsStash = g_lingo->_methodVars;
	g_lingo->_methodVars = new VarTypeHash;

	for (VarTypeHash::iterator i = g_lingo->_methodVarsStash->begin(); i != g_lingo->_methodVarsStash->end(); ++i) {
		if (i->_value == kVarGlobal || i->_value == kVarProperty)
			(*g_lingo->_methodVars)[i->_key] = i->_value;
	}
	if (g_lingo->_inFactory) {
		for (DatumHash::iterator i = g_lingo->_assemblyContext->_properties.begin(); i != g_lingo->_assemblyContext->_properties.end(); ++i) {
			(*g_lingo->_methodVars)[i->_key] = kVarInstance;
		}
	}
}

static void endDef() {
	g_lingo->clearArgStack();
	inNone();

	delete g_lingo->_methodVars;
	g_lingo->_methodVars = g_lingo->_methodVarsStash;
	g_lingo->_methodVarsStash = nullptr;
}

static void startRepeat() {
	g_lingo->_repeatStack.push_back(new RepeatBlock);
}

static void endRepeat(uint exitPos, uint nextPos) {
	RepeatBlock *block = g_lingo->_repeatStack.back();
	g_lingo->_repeatStack.pop_back();
	for (uint i = 0; i < block->exits.size(); i++) {
		uint32 pos = block->exits[i];
		inst exit = 0;
		WRITE_UINT32(&exit, exitPos - (pos - 1));
		(*g_lingo->_currentAssembly)[pos] = exit;
	}
	for (uint i = 0; i < block->nexts.size(); i++) {
		uint32 pos = block->nexts[i];
		inst next = 0;
		WRITE_UINT32(&next, nextPos - (pos - 1));
		(*g_lingo->_currentAssembly)[pos] = next;
	}
	delete block;
}

static VarType globalCheck() {
	// If in a definition, assume variables are local unless
	// they were declared global with `global varname`
	if (g_lingo->_indef == kStateInDef) {
		return kVarLocal;
	}
	return kVarGlobal;
}

static void mVar(Common::String *s, VarType type) {
	if (!g_lingo->_methodVars->contains(*s)) {
		(*g_lingo->_methodVars)[*s] = type;
		if (type == kVarProperty || type == kVarInstance) {
			g_lingo->_assemblyContext->_properties[*s] = Datum();
		} else if (type == kVarGlobal) {
			g_lingo->varCreate(*s, true);
		}
	}
}


#line 177 "engines/director/lingo/lingo-gr.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED
# define YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    UNARY = 258,                   /* UNARY  */
    CASTREF = 259,                 /* CASTREF  */
    VOID = 260,                    /* VOID  */
    VAR = 261,                     /* VAR  */
    POINT = 262,                   /* POINT  */
    RECT = 263,                    /* RECT  */
    ARRAY = 264,                   /* ARRAY  */
    OBJECT = 265,                  /* OBJECT  */
    FIELDREF = 266,                /* FIELDREF  */
    LEXERROR = 267,                /* LEXERROR  */
    PARRAY = 268,                  /* PARRAY  */
    INT = 269,                     /* INT  */
    ARGC = 270,                    /* ARGC  */
    ARGCNORET = 271,               /* ARGCNORET  */
    THEENTITY = 272,               /* THEENTITY  */
    THEENTITYWITHID = 273,         /* THEENTITYWITHID  */
    THEMENUITEMENTITY = 274,       /* THEMENUITEMENTITY  */
    THEMENUITEMSENTITY = 275,      /* THEMENUITEMSENTITY  */
    FLOAT = 276,                   /* FLOAT  */
    THEFUNC = 277,                 /* THEFUNC  */
    THEFUNCINOF = 278,             /* THEFUNCINOF  */
    VARID = 279,                   /* VARID  */
    STRING = 280,                  /* STRING  */
    SYMBOL = 281,                  /* SYMBOL  */
    ENDCLAUSE = 282,               /* ENDCLAUSE  */
    tPLAYACCEL = 283,              /* tPLAYACCEL  */
    tMETHOD = 284,                 /* tMETHOD  */
    THEOBJECTPROP = 285,           /* THEOBJECTPROP  */
    CAST = 286,                    /* CAST  */
    FIELD = 287,                   /* FIELD  */
    SCRIPT = 288,                  /* SCRIPT  */
    WINDOW = 289,                  /* WINDOW  */
    tDOWN = 290,                   /* tDOWN  */
    tELSE = 291,                   /* tELSE  */
    tELSIF = 292,                  /* tELSIF  */
    tEXIT = 293,                   /* tEXIT  */
    tGLOBAL = 294,                 /* tGLOBAL  */
    tGO = 295,                     /* tGO  */
    tGOLOOP = 296,                 /* tGOLOOP  */
    tIF = 297,                     /* tIF  */
    tIN = 298,                     /* tIN  */
    tINTO = 299,                   /* tINTO  */
    tMACRO = 300,                  /* tMACRO  */
    tMOVIE = 301,                  /* tMOVIE  */
    tNEXT = 302,                   /* tNEXT  */
    tOF = 303,                     /* tOF  */
    tPREVIOUS = 304,               /* tPREVIOUS  */
    tPUT = 305,                    /* tPUT  */
    tREPEAT = 306,                 /* tREPEAT  */
    tSET = 307,                    /* tSET  */
    tTHEN = 308,                   /* tTHEN  */
    tTO = 309,                     /* tTO  */
    tWHEN = 310,                   /* tWHEN  */
    tWITH = 311,                   /* tWITH  */
    tWHILE = 312,                  /* tWHILE  */
    tFACTORY = 313,                /* tFACTORY  */
    tOPEN = 314,                   /* tOPEN  */
    tPLAY = 315,                   /* tPLAY  */
    tINSTANCE = 316,               /* tINSTANCE  */
    tGE = 317,                     /* tGE  */
    tLE = 318,                     /* tLE  */
    tEQ = 319,                     /* tEQ  */
    tNEQ = 320,                    /* tNEQ  */
    tAND = 321,                    /* tAND  */
    tOR = 322,                     /* tOR  */
    tNOT = 323,                    /* tNOT  */
    tMOD = 324,                    /* tMOD  */
    tAFTER = 325,                  /* tAFTER  */
    tBEFORE = 326,                 /* tBEFORE  */
    tCONCAT = 327,                 /* tCONCAT  */
    tCONTAINS = 328,               /* tCONTAINS  */
    tSTARTS = 329,                 /* tSTARTS  */
    tCHAR = 330,                   /* tCHAR  */
    tITEM = 331,                   /* tITEM  */
    tLINE = 332,                   /* tLINE  */
    tWORD = 333,                   /* tWORD  */
    tSPRITE = 334,                 /* tSPRITE  */
    tINTERSECTS = 335,             /* tINTERSECTS  */
    tWITHIN = 336,                 /* tWITHIN  */
    tTELL = 337,                   /* tTELL  */
    tPROPERTY = 338,               /* tPROPERTY  */
    tON = 339,                     /* tON  */
    tENDIF = 340,                  /* tENDIF  */
    tENDREPEAT = 341,              /* tENDREPEAT  */
    tENDTELL = 342,                /* tENDTELL  */
    tCAST = 343,                   /* tCAST  */
    tFIELD = 344,                  /* tFIELD  */
    tSCRIPT = 345,                 /* tSCRIPT  */
    tWINDOW = 346                  /* tWINDOW  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 160 "engines/director/lingo/lingo-gr.y"

	Common::String *s;
	int i;
	double f;
	int e[2];	// Entity + field
	int code;
	int narg;	/* number of arguments */
	Director::DatumArray *arr;

	struct {
		Common::String *obj;
		Common::String *prop;
	} objectprop;

#line 333 "engines/director/lingo/lingo-gr.cpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ENGINES_DIRECTOR_LINGO_LINGO_GR_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_UNARY = 3,                      /* UNARY  */
  YYSYMBOL_CASTREF = 4,                    /* CASTREF  */
  YYSYMBOL_VOID = 5,                       /* VOID  */
  YYSYMBOL_VAR = 6,                        /* VAR  */
  YYSYMBOL_POINT = 7,                      /* POINT  */
  YYSYMBOL_RECT = 8,                       /* RECT  */
  YYSYMBOL_ARRAY = 9,                      /* ARRAY  */
  YYSYMBOL_OBJECT = 10,                    /* OBJECT  */
  YYSYMBOL_FIELDREF = 11,                  /* FIELDREF  */
  YYSYMBOL_LEXERROR = 12,                  /* LEXERROR  */
  YYSYMBOL_PARRAY = 13,                    /* PARRAY  */
  YYSYMBOL_INT = 14,                       /* INT  */
  YYSYMBOL_ARGC = 15,                      /* ARGC  */
  YYSYMBOL_ARGCNORET = 16,                 /* ARGCNORET  */
  YYSYMBOL_THEENTITY = 17,                 /* THEENTITY  */
  YYSYMBOL_THEENTITYWITHID = 18,           /* THEENTITYWITHID  */
  YYSYMBOL_THEMENUITEMENTITY = 19,         /* THEMENUITEMENTITY  */
  YYSYMBOL_THEMENUITEMSENTITY = 20,        /* THEMENUITEMSENTITY  */
  YYSYMBOL_FLOAT = 21,                     /* FLOAT  */
  YYSYMBOL_THEFUNC = 22,                   /* THEFUNC  */
  YYSYMBOL_THEFUNCINOF = 23,               /* THEFUNCINOF  */
  YYSYMBOL_VARID = 24,                     /* VARID  */
  YYSYMBOL_STRING = 25,                    /* STRING  */
  YYSYMBOL_SYMBOL = 26,                    /* SYMBOL  */
  YYSYMBOL_ENDCLAUSE = 27,                 /* ENDCLAUSE  */
  YYSYMBOL_tPLAYACCEL = 28,                /* tPLAYACCEL  */
  YYSYMBOL_tMETHOD = 29,                   /* tMETHOD  */
  YYSYMBOL_THEOBJECTPROP = 30,             /* THEOBJECTPROP  */
  YYSYMBOL_CAST = 31,                      /* CAST  */
  YYSYMBOL_FIELD = 32,                     /* FIELD  */
  YYSYMBOL_SCRIPT = 33,                    /* SCRIPT  */
  YYSYMBOL_WINDOW = 34,                    /* WINDOW  */
  YYSYMBOL_tDOWN = 35,                     /* tDOWN  */
  YYSYMBOL_tELSE = 36,                     /* tELSE  */
  YYSYMBOL_tELSIF = 37,                    /* tELSIF  */
  YYSYMBOL_tEXIT = 38,                     /* tEXIT  */
  YYSYMBOL_tGLOBAL = 39,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 40,                       /* tGO  */
  YYSYMBOL_tGOLOOP = 41,                   /* tGOLOOP  */
  YYSYMBOL_tIF = 42,                       /* tIF  */
  YYSYMBOL_tIN = 43,                       /* tIN  */
  YYSYMBOL_tINTO = 44,                     /* tINTO  */
  YYSYMBOL_tMACRO = 45,                    /* tMACRO  */
  YYSYMBOL_tMOVIE = 46,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 47,                     /* tNEXT  */
  YYSYMBOL_tOF = 48,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 49,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 50,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 51,                   /* tREPEAT  */
  YYSYMBOL_tSET = 52,                      /* tSET  */
  YYSYMBOL_tTHEN = 53,                     /* tTHEN  */
  YYSYMBOL_tTO = 54,                       /* tTO  */
  YYSYMBOL_tWHEN = 55,                     /* tWHEN  */
  YYSYMBOL_tWITH = 56,                     /* tWITH  */
  YYSYMBOL_tWHILE = 57,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 58,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 59,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 60,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 61,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 62,                       /* tGE  */
  YYSYMBOL_tLE = 63,                       /* tLE  */
  YYSYMBOL_tEQ = 64,                       /* tEQ  */
  YYSYMBOL_tNEQ = 65,                      /* tNEQ  */
  YYSYMBOL_tAND = 66,                      /* tAND  */
  YYSYMBOL_tOR = 67,                       /* tOR  */
  YYSYMBOL_tNOT = 68,                      /* tNOT  */
  YYSYMBOL_tMOD = 69,                      /* tMOD  */
  YYSYMBOL_tAFTER = 70,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 71,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 72,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 73,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 74,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 75,                     /* tCHAR  */
  YYSYMBOL_tITEM = 76,                     /* tITEM  */
  YYSYMBOL_tLINE = 77,                     /* tLINE  */
  YYSYMBOL_tWORD = 78,                     /* tWORD  */
  YYSYMBOL_tSPRITE = 79,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 80,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 81,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 82,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 83,                 /* tPROPERTY  */
  YYSYMBOL_tON = 84,                       /* tON  */
  YYSYMBOL_tENDIF = 85,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 86,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 87,                  /* tENDTELL  */
  YYSYMBOL_88_ = 88,                       /* '<'  */
  YYSYMBOL_89_ = 89,                       /* '>'  */
  YYSYMBOL_90_ = 90,                       /* '&'  */
  YYSYMBOL_91_ = 91,                       /* '+'  */
  YYSYMBOL_92_ = 92,                       /* '-'  */
  YYSYMBOL_93_ = 93,                       /* '*'  */
  YYSYMBOL_94_ = 94,                       /* '/'  */
  YYSYMBOL_95_ = 95,                       /* '%'  */
  YYSYMBOL_tCAST = 96,                     /* tCAST  */
  YYSYMBOL_tFIELD = 97,                    /* tFIELD  */
  YYSYMBOL_tSCRIPT = 98,                   /* tSCRIPT  */
  YYSYMBOL_tWINDOW = 99,                   /* tWINDOW  */
  YYSYMBOL_100_n_ = 100,                   /* '\n'  */
  YYSYMBOL_101_ = 101,                     /* '('  */
  YYSYMBOL_102_ = 102,                     /* ')'  */
  YYSYMBOL_103_ = 103,                     /* ','  */
  YYSYMBOL_104_ = 104,                     /* '['  */
  YYSYMBOL_105_ = 105,                     /* ']'  */
  YYSYMBOL_106_ = 106,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 107,                 /* $accept  */
  YYSYMBOL_program = 108,                  /* program  */
  YYSYMBOL_programline = 109,              /* programline  */
  YYSYMBOL_ID = 110,                       /* ID  */
  YYSYMBOL_asgn = 111,                     /* asgn  */
  YYSYMBOL_stmtoneliner = 112,             /* stmtoneliner  */
  YYSYMBOL_stmt = 113,                     /* stmt  */
  YYSYMBOL_114_1 = 114,                    /* $@1  */
  YYSYMBOL_115_2 = 115,                    /* $@2  */
  YYSYMBOL_116_3 = 116,                    /* $@3  */
  YYSYMBOL_117_4 = 117,                    /* $@4  */
  YYSYMBOL_118_5 = 118,                    /* $@5  */
  YYSYMBOL_119_6 = 119,                    /* $@6  */
  YYSYMBOL_120_7 = 120,                    /* $@7  */
  YYSYMBOL_121_8 = 121,                    /* $@8  */
  YYSYMBOL_122_9 = 122,                    /* $@9  */
  YYSYMBOL_startrepeat = 123,              /* startrepeat  */
  YYSYMBOL_tellstart = 124,                /* tellstart  */
  YYSYMBOL_ifstmt = 125,                   /* ifstmt  */
  YYSYMBOL_elseifstmtlist = 126,           /* elseifstmtlist  */
  YYSYMBOL_elseifstmt = 127,               /* elseifstmt  */
  YYSYMBOL_jumpifz = 128,                  /* jumpifz  */
  YYSYMBOL_jump = 129,                     /* jump  */
  YYSYMBOL_varassign = 130,                /* varassign  */
  YYSYMBOL_if = 131,                       /* if  */
  YYSYMBOL_lbl = 132,                      /* lbl  */
  YYSYMBOL_stmtlist = 133,                 /* stmtlist  */
  YYSYMBOL_simpleexprnoparens = 134,       /* simpleexprnoparens  */
  YYSYMBOL_135_10 = 135,                   /* $@10  */
  YYSYMBOL_simpleexpr = 136,               /* simpleexpr  */
  YYSYMBOL_expr = 137,                     /* expr  */
  YYSYMBOL_chunkexpr = 138,                /* chunkexpr  */
  YYSYMBOL_reference = 139,                /* reference  */
  YYSYMBOL_proc = 140,                     /* proc  */
  YYSYMBOL_141_11 = 141,                   /* $@11  */
  YYSYMBOL_142_12 = 142,                   /* $@12  */
  YYSYMBOL_143_13 = 143,                   /* $@13  */
  YYSYMBOL_144_14 = 144,                   /* $@14  */
  YYSYMBOL_globallist = 145,               /* globallist  */
  YYSYMBOL_propertylist = 146,             /* propertylist  */
  YYSYMBOL_instancelist = 147,             /* instancelist  */
  YYSYMBOL_gotofunc = 148,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 149,                /* gotomovie  */
  YYSYMBOL_playfunc = 150,                 /* playfunc  */
  YYSYMBOL_151_15 = 151,                   /* $@15  */
  YYSYMBOL_defn = 152,                     /* defn  */
  YYSYMBOL_153_16 = 153,                   /* $@16  */
  YYSYMBOL_154_17 = 154,                   /* $@17  */
  YYSYMBOL_on = 155,                       /* on  */
  YYSYMBOL_156_18 = 156,                   /* $@18  */
  YYSYMBOL_argdef = 157,                   /* argdef  */
  YYSYMBOL_endargdef = 158,                /* endargdef  */
  YYSYMBOL_argstore = 159,                 /* argstore  */
  YYSYMBOL_arglist = 160,                  /* arglist  */
  YYSYMBOL_nonemptyarglist = 161,          /* nonemptyarglist  */
  YYSYMBOL_list = 162,                     /* list  */
  YYSYMBOL_valuelist = 163,                /* valuelist  */
  YYSYMBOL_linearlist = 164,               /* linearlist  */
  YYSYMBOL_proplist = 165,                 /* proplist  */
  YYSYMBOL_proppair = 166                  /* proppair  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  99
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2302

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  107
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  60
/* YYNRULES -- Number of rules.  */
#define YYNRULES  189
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  388

#define YYMAXUTOK   346


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     100,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    95,    90,     2,
     101,   102,    93,    91,   103,    92,     2,    94,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   106,     2,
      88,     2,    89,     2,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87,    96,    97,    98,    99
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   214,   214,   215,   217,   218,   219,   221,   222,   223,
     224,   225,   227,   234,   238,   249,   256,   259,   266,   269,
     276,   283,   290,   297,   303,   310,   321,   332,   339,   347,
     348,   350,   351,   356,   369,   373,   376,   368,   403,   407,
     410,   402,   437,   444,   450,   436,   480,   488,   491,   492,
     494,   496,   498,   505,   513,   514,   516,   522,   526,   530,
     534,   537,   539,   540,   541,   543,   546,   549,   550,   551,
     555,   559,   560,   568,   574,   577,   580,   586,   587,   588,
     589,   595,   595,   600,   603,   611,   613,   614,   616,   617,
     618,   619,   620,   621,   622,   623,   624,   625,   626,   627,
     628,   629,   630,   631,   632,   633,   634,   636,   639,   640,
     641,   642,   643,   644,   645,   646,   648,   649,   652,   655,
     659,   660,   661,   662,   670,   671,   671,   672,   672,   673,
     673,   674,   677,   680,   686,   686,   691,   694,   698,   701,
     705,   708,   712,   715,   726,   727,   728,   729,   733,   737,
     742,   743,   745,   749,   753,   757,   757,   787,   787,   793,
     794,   794,   800,   808,   814,   814,   816,   817,   818,   820,
     821,   822,   824,   826,   827,   828,   830,   831,   833,   835,
     836,   837,   838,   840,   841,   843,   844,   846,   850,   854
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  static const char *const yy_sname[] =
  {
  "end of file", "error", "invalid token", "UNARY", "CASTREF", "VOID",
  "VAR", "POINT", "RECT", "ARRAY", "OBJECT", "FIELDREF", "LEXERROR",
  "PARRAY", "INT", "ARGC", "ARGCNORET", "THEENTITY", "THEENTITYWITHID",
  "THEMENUITEMENTITY", "THEMENUITEMSENTITY", "FLOAT", "THEFUNC",
  "THEFUNCINOF", "VARID", "STRING", "SYMBOL", "ENDCLAUSE", "tPLAYACCEL",
  "tMETHOD", "THEOBJECTPROP", "CAST", "FIELD", "SCRIPT", "WINDOW", "tDOWN",
  "tELSE", "tELSIF", "tEXIT", "tGLOBAL", "tGO", "tGOLOOP", "tIF", "tIN",
  "tINTO", "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT",
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE",
  "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tENDIF",
  "tENDREPEAT", "tENDTELL", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "tCAST", "tFIELD", "tSCRIPT", "tWINDOW", "'\\n'", "'('",
  "')'", "','", "'['", "']'", "':'", "$accept", "program", "programline",
  "ID", "asgn", "stmtoneliner", "stmt", "$@1", "$@2", "$@3", "$@4", "$@5",
  "$@6", "$@7", "$@8", "$@9", "startrepeat", "tellstart", "ifstmt",
  "elseifstmtlist", "elseifstmt", "jumpifz", "jump", "varassign", "if",
  "lbl", "stmtlist", "simpleexprnoparens", "$@10", "simpleexpr", "expr",
  "chunkexpr", "reference", "proc", "$@11", "$@12", "$@13", "$@14",
  "globallist", "propertylist", "instancelist", "gotofunc", "gotomovie",
  "playfunc", "$@15", "defn", "$@16", "$@17", "on", "$@18", "argdef",
  "endargdef", "argstore", "arglist", "nonemptyarglist", "list",
  "valuelist", "linearlist", "proplist", "proppair", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,    60,    62,
      38,    43,    45,    42,    47,    37,   343,   344,   345,   346,
      10,    40,    41,    44,    91,    93,    58
};
#endif

#define YYPACT_NINF (-327)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-180)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1697,  -327,  -327,  -327,   -42,  -327,   150,  -327,  -327,  -327,
     -27,  1407,     6,    21,    89,    89,  1439,  1280,  -327,  1439,
    -327,  -327,  -327,  -327,  -327,  -327,    58,   -71,  1104,  -327,
    -327,  -327,  -327,  1439,  -327,  -327,  -327,  -327,  -327,  1192,
    -327,  -327,    89,   -12,  -327,  -327,  1527,  -327,    20,  1527,
    -327,  -327,  -327,  1439,  -327,    30,  -327,  1439,  1439,  1439,
    1439,  1439,  1439,  1527,  1527,   596,   703,   810,   917,  1439,
    1011,   -21,  -327,  -327,  1764,  -327,  -327,  -327,  -327,    89,
    -327,    89,   980,    89,  -327,   -23,  1527,  1527,     2,     3,
      50,  -327,  2129,  1764,  -327,    89,  1797,    89,    89,  -327,
    1697,  1315,  2195,   -18,  2195,    89,   -18,    89,  -327,    11,
    -327,  -327,  1527,  -327,  2195,  1439,  -327,  1844,  1877,  1910,
    1943,  2162,  -327,  -327,  -327,  -327,  -327,  -327,  2088,    31,
      40,  -327,   -79,  2195,     1,    13,    22,  -327,  1315,  1439,
    1439,  1439,  1439,  1439,  1439,  1439,  1439,  1439,  1439,  1439,
    1439,  1439,  1439,  1439,  1439,  1439,  -327,  -327,  1527,   106,
     106,   106,   -32,  1439,  1439,  1439,     7,   100,  1439,  1439,
    1439,  1439,  1439,  1439,  -327,  -327,    46,  -327,  -327,  -327,
      47,  -327,  -327,     9,  2088,    -9,  1439,    99,  -327,   -77,
     -16,    89,  -327,  2195,  1439,  1439,  1439,  1439,  1439,  1439,
    1439,  1439,  1439,  1439,  -327,  1439,  1439,  1439,  -327,  1439,
      -7,    26,    19,   -44,   -44,   -44,   -44,  2208,  2208,  -327,
     -57,   -44,   -44,   -44,   -44,   -57,   -39,   -39,  -327,  -327,
      89,   110,  -327,  -327,  -327,  -327,  -327,  -327,  1439,  1439,
    2195,  2195,  2195,  1439,  1439,    89,  2195,  2195,  2195,  2195,
    2195,  2195,    89,  1726,  -327,    89,  -327,  -327,  -327,  2195,
    -327,  -327,    89,  -327,  -327,  2195,  1976,  2195,  2009,  2195,
    2042,  2195,  2075,  2195,  2195,  2195,  2195,  2195,  2195,    31,
      40,    49,  -327,  -327,  -327,  -327,    -4,  1439,  2195,   265,
    -327,  2195,  2195,  1527,  -327,  -327,  1538,  -327,  1439,  1538,
    -327,  -327,  -327,  1439,  1439,  1439,  1439,  1439,  -327,  2195,
    -327,  -327,  -327,  -327,    10,  -327,  -327,  -327,    69,  2195,
      29,  -327,   445,  1538,  2195,  2195,  2195,  2195,    32,  -327,
    -327,  -327,  -327,  1538,  1439,  1439,  -327,  -327,  1439,   108,
      89,  -327,  1538,  -327,   104,   128,    76,  2195,  2195,  2195,
    -327,  1439,  -327,    80,  -327,    63,  -327,  1439,   115,  -327,
    1538,  2195,  -327,    89,  -327,  2195,  1439,    85,   124,  -327,
    -327,  -327,  2195,  -327,  -327,   479,  -327,  -327,  1538,  -327,
    -327,  -327,  -327,  1605,  -327,  -327,  1634,  -327
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     7,   155,   160,   124,   125,     0,   144,    60,   157,
       0,     0,     0,     0,     0,     0,     0,     0,   129,     0,
     127,   164,     8,     9,    10,    11,     0,     2,     0,    29,
      31,     6,    32,     0,    30,   121,   122,     5,    61,     0,
      61,   123,     0,     0,    65,    72,     0,    66,     0,     0,
      70,    69,    76,     0,   145,     0,   146,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,    86,    88,   147,   116,    71,   149,    79,     0,
      46,     0,   120,     0,    61,     0,     0,     0,     0,     0,
       0,   159,   132,   152,   154,     0,     0,     0,     0,     1,
       4,     0,   174,   137,    57,   166,   156,   166,   138,   126,
      85,    73,     0,    74,   151,     0,   102,     0,     0,     0,
       0,     0,    67,    68,   117,   107,   118,   119,     0,    70,
      69,   180,    84,   183,     0,   182,   181,   185,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   148,    61,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   153,   142,   130,    51,    51,   140,
     128,   165,     3,    84,   174,     0,     0,     0,   167,     0,
       0,     0,    75,   150,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,   178,     0,
       0,    84,     0,    98,    99,    96,    97,   100,   101,    93,
     104,   105,   106,    95,    94,   103,    89,    90,    91,    92,
     166,     0,    12,    13,    15,    16,    17,    18,     0,     0,
      57,    22,    20,     0,     0,     0,    27,    28,    21,    19,
      47,   131,     0,     0,    62,     0,   133,   134,   136,   175,
      62,   172,     0,   172,   139,   108,     0,   110,     0,   112,
       0,   114,     0,    77,    78,   188,   187,   189,   184,     0,
       0,     0,   186,    80,    81,    83,     0,     0,    42,    34,
      50,    23,    24,     0,   143,    61,    61,   141,     0,    58,
      62,   168,    62,     0,     0,     0,     0,     0,   172,    14,
      61,    59,    59,    62,     0,    49,    63,    64,     0,   176,
       0,    54,   163,   161,   109,   111,   113,   115,     0,    62,
      43,    35,    39,    58,     0,     0,    48,   135,     0,    61,
     169,    82,   158,    57,     0,     0,     0,    25,    26,   177,
      62,     0,    55,     0,   170,   162,    44,     0,     0,    33,
      61,    57,    52,     0,    50,    36,     0,     0,     0,   171,
      62,    57,    40,    53,    62,     0,    50,    57,    58,    45,
      62,    50,    56,     0,    62,    37,     0,    41
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -327,    78,  -327,     0,  -327,   -74,     4,  -327,  -327,  -327,
    -327,  -327,  -327,  -327,  -327,  -327,  -299,    12,  -327,  -327,
    -327,  -235,  -326,  -123,  -327,   -24,  -227,   127,  -327,   -43,
     250,   -20,  -327,  -327,  -327,  -327,  -327,  -327,  -327,  -327,
    -327,  -327,   -15,  -327,  -327,  -327,  -327,  -327,  -327,  -327,
    -106,  -327,  -255,   -29,  -107,  -327,  -327,  -327,  -327,    -3
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    26,    27,    71,    29,    30,   317,   311,   344,   371,
     312,   345,   377,   310,   343,   364,   313,   253,    32,   339,
     352,   187,   321,   331,    33,   105,   296,    72,   307,    73,
     102,    75,    76,    34,    42,    97,    95,   298,   109,   180,
     176,    35,    77,    36,    39,    37,    79,    40,    38,    98,
     189,   355,   300,   103,   320,    78,   134,   135,   136,   137
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      28,   190,    94,   111,    31,   290,   113,   346,   302,    41,
     106,   238,   145,    89,    90,    91,   107,     1,   279,   280,
     122,   123,   138,   261,    80,   145,   262,   207,   146,   100,
     145,   164,   239,   299,   152,   153,   154,   155,    85,    86,
      87,   165,   108,   166,   167,     1,   151,   152,   153,   154,
     155,    88,   382,   329,   154,   155,   168,   170,    99,   156,
     163,   243,    83,    84,   334,   370,   169,   171,   112,   192,
     132,   244,   185,   322,   335,   323,   115,   380,   174,   157,
     138,   158,   384,   162,   263,   186,   333,   262,   110,    22,
      23,    24,    25,   258,   186,   175,   308,   179,   181,   262,
      28,   183,   342,   172,    31,   188,   208,   188,   356,   212,
     138,   256,   257,     1,   191,   231,   209,    22,    23,    24,
      25,   285,   186,   360,   286,   210,   368,   138,   283,   284,
       1,   337,   338,   230,   341,   338,   376,   205,   211,   233,
     235,   237,   381,   375,   350,   351,   206,   378,   245,   252,
     255,    43,   260,   383,   287,   207,   336,   386,   357,   232,
     234,   236,   359,   358,    44,   362,   363,    45,    46,   366,
     373,    47,    48,    49,     1,    50,    51,   374,   182,   295,
      52,    58,    59,    60,    61,    22,    23,    24,    25,   332,
     254,   264,   124,   125,   126,   127,    53,    54,    55,    56,
     328,     0,    22,    66,    24,    25,     0,   282,     0,     0,
     281,     0,     0,     0,     0,     0,     0,     0,    57,     0,
       0,     0,     0,     0,     0,    58,    59,    60,    61,    62,
     188,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    63,    64,     0,     0,   293,    65,    66,    67,    68,
     314,    69,   294,    28,    70,   297,    74,     0,     0,     0,
       0,    82,   301,     0,     0,     0,    92,    93,     0,    96,
       0,   315,   318,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   104,     0,     0,   330,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    28,     0,     0,    28,
     -38,     0,     0,   114,     0,     0,     0,   116,   117,   118,
     119,   120,   121,     0,     0,   353,     0,     0,     0,   128,
     133,     0,    28,    28,     0,     0,     0,   139,   140,   141,
     142,   143,   144,    28,   145,     0,   367,   146,   147,   148,
     354,     0,    28,     0,     0,     0,     0,     0,     0,     0,
       0,   184,     0,   149,   150,   151,   152,   153,   154,   155,
      28,     0,     0,   369,     0,   193,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    28,     0,     0,    28,     0,
       0,     0,     0,    28,     0,     0,    28,     0,     0,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,     0,     0,     0,     0,
       0,     0,     0,   240,   241,   242,     0,     0,   246,   247,
     248,   249,   250,   251,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   259,     0,     0,     0,
       0,     0,     0,     0,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,     0,   275,   276,   277,     0,   278,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     1,
       0,     0,   340,     2,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     4,     5,     6,     7,     8,   288,   289,
       0,     0,    10,   291,   292,    11,    12,    13,     0,     0,
      14,     0,     0,     1,    16,    17,    18,     2,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     5,     6,
       7,     8,     0,     0,     0,     0,    10,    19,    20,    11,
      12,    13,     0,     0,    14,     0,     0,   309,    16,    17,
      18,    22,    23,    24,    25,   316,     0,     0,   319,     0,
       0,     0,     0,   324,   325,   326,   327,   319,     0,     0,
       0,    19,    20,     0,     0,   379,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,    23,    24,    25,   316,
       0,     0,     0,     0,   347,   348,     0,     0,   349,     0,
       0,     0,     0,     0,     0,     0,    -8,    43,     0,     0,
       0,   361,     0,     0,     0,     0,     0,   365,     0,     0,
      44,     0,     0,    45,    46,     0,   372,    47,    48,    49,
       1,    50,    51,    -8,    -8,     0,    52,     0,     0,     0,
       0,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,     0,
      -8,     0,    -8,    -8,    -8,     0,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,     0,     0,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,    -8,     0,    -8,    -8,    -8,    -8,    -8,
      -8,    58,    59,    60,    61,    62,    -8,    -8,    -8,    -8,
       0,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,     0,    65,    66,    67,    68,    -8,    -8,    -8,    -8,
      70,    -8,    -8,    -9,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    44,     0,     0,
      45,    46,     0,     0,    47,    48,    49,     1,    50,    51,
      -9,    -9,     0,    52,     0,     0,     0,     0,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,     0,    -9,     0,    -9,
      -9,    -9,     0,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
       0,     0,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,     0,    -9,    -9,    -9,    -9,    -9,    -9,    58,    59,
      60,    61,    62,    -9,    -9,    -9,    -9,     0,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,     0,    65,
      66,    67,    68,    -9,    -9,    -9,    -9,    70,    -9,    -9,
     -10,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    44,     0,     0,    45,    46,     0,
       0,    47,    48,    49,     1,    50,    51,   -10,   -10,     0,
      52,     0,     0,     0,     0,   -10,   -10,   -10,   -10,   -10,
     -10,   -10,   -10,     0,   -10,     0,   -10,   -10,   -10,     0,
     -10,   -10,   -10,   -10,   -10,   -10,   -10,     0,     0,   -10,
     -10,   -10,   -10,   -10,   -10,   -10,   -10,   -10,     0,   -10,
     -10,   -10,   -10,   -10,   -10,    58,    59,    60,    61,    62,
     -10,   -10,   -10,   -10,     0,   -10,   -10,   -10,   -10,   -10,
     -10,   -10,   -10,   -10,   -10,     0,    65,    66,    67,    68,
     -10,   -10,   -10,   -10,    70,   -10,   -10,   -11,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    44,     0,     0,    45,    46,     0,     0,    47,    48,
      49,     1,    50,    51,   -11,   -11,     0,    52,     0,     0,
       0,     0,   -11,   -11,   -11,   -11,   -11,   -11,   -11,   -11,
       0,   -11,     0,   -11,   -11,   -11,     0,   -11,   -11,   -11,
     -11,   -11,   -11,   -11,     0,     0,   -11,   -11,   -11,   -11,
     -11,   -11,   -11,   -11,   -11,     0,   -11,   -11,   -11,   -11,
     -11,   -11,    58,    59,    60,    61,    62,   -11,   -11,   -11,
     -11,     0,   -11,   -11,   -11,   -11,   -11,   -11,   -11,   -11,
     -11,   -11,    43,    65,    66,    67,    68,   -11,   -11,   -11,
     -11,    70,   -11,   -11,   159,    44,     0,     0,    45,    46,
       0,     0,    47,    48,    49,     1,   129,   130,     0,     0,
       0,    52,   139,   140,   141,   142,   143,   144,     0,   145,
     160,   161,   146,   147,   148,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   149,   150,
     151,   152,   153,   154,   155,     0,     0,     0,     0,    57,
       0,     0,     0,     0,     0,     0,    58,    59,    60,    61,
      62,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    63,    64,  -173,    43,     0,    65,    66,    67,
      68,     0,    69,     0,     0,    70,  -179,   131,    44,     0,
       0,    45,    46,     0,     0,    47,    48,    49,     1,    50,
      51,  -173,  -173,     0,    52,     0,     0,     0,     0,     0,
    -173,  -173,  -173,  -173,  -173,  -173,  -173,     0,     0,     0,
       0,  -173,     0,     0,  -173,  -173,  -173,     0,     0,  -173,
       0,     0,     0,  -173,  -173,  -173,     0,     0,     0,     0,
       0,     0,    57,     0,     0,     0,     0,     0,     0,    58,
      59,    60,    61,    62,     0,     0,  -173,  -173,     0,  -173,
    -173,  -173,  -173,    43,     0,    63,    64,     0,     0,     0,
      65,    66,    67,    68,  -173,   101,    44,  -173,    70,    45,
      46,     0,     0,    47,    48,    49,     1,    50,    51,  -173,
    -173,     0,    52,     0,     0,     0,     0,     0,  -173,  -173,
    -173,  -173,  -173,  -173,  -173,     0,     0,     0,     0,  -173,
       0,     0,  -173,  -173,  -173,     0,     0,  -173,     0,     0,
       0,  -173,  -173,  -173,     0,     0,     0,     0,     0,     0,
      57,     0,     0,     0,     0,     0,     0,    58,    59,    60,
      61,    62,     0,     0,  -173,  -173,     0,  -173,  -173,  -173,
       0,    43,     0,    63,    64,     0,     0,     0,    65,    66,
      67,    68,  -173,    69,    44,  -173,    70,    45,    46,     0,
       0,    47,    48,    49,     1,    50,    51,     0,     0,     0,
      52,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    53,     0,    55,    44,
       0,     0,    45,    46,     0,     0,    47,    48,    49,     1,
      50,    51,     0,     0,     0,    52,     0,     0,    57,     0,
       0,     0,     0,     0,     0,    58,    59,    60,    61,    62,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    63,    64,     0,     0,     0,    65,    66,    67,    68,
       0,    69,     0,    57,    70,     0,     0,     0,     0,     0,
      58,    59,    60,    61,    62,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    63,    64,    43,     0,
       0,    65,    66,    67,    68,     0,    69,  -173,  -173,    70,
       0,    44,     0,     0,    45,    46,     0,    81,    47,    48,
      49,     1,    50,    51,     0,     0,     0,    52,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    44,     0,     0,    45,    46,     0,     0,
      47,    48,    49,     1,    50,    51,     0,     0,     0,    52,
       0,     0,     0,     0,     0,    57,     0,     0,     0,     0,
       0,     0,    58,    59,    60,    61,    62,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,    64,
       0,     0,     0,    65,    66,    67,    68,    57,    69,     0,
       0,    70,     0,     0,    58,    59,    60,    61,    62,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    43,     0,
      63,    64,     0,     0,     0,    65,    66,    67,    68,     0,
      69,    44,     0,    70,    45,    46,     0,     0,    47,    48,
      49,     1,    50,    51,     0,     0,     0,    52,     0,     0,
       0,     0,     1,     0,     0,     0,     2,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     4,     5,     6,     7,
       8,     0,     0,     0,     0,    10,     0,     0,    11,    12,
      13,     0,     0,    14,     0,     0,     0,    16,    17,    18,
       0,     0,    58,    59,    60,    61,    62,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    63,    64,
      19,    20,     0,    65,    66,    67,    68,     0,    69,     1,
       0,    70,     0,     2,    22,    23,    24,    25,   316,     0,
       0,     0,     0,     4,     5,     6,     7,     8,     0,     0,
       0,     0,    10,     0,     0,    11,    12,    13,     1,     0,
      14,     0,     2,     0,    16,    17,    18,     0,     0,     0,
       0,     0,     4,     5,     6,     7,     8,     0,     0,     0,
       0,    10,     0,     0,    11,    12,    13,    19,    20,    14,
       0,   385,     0,    16,    17,    18,     0,     0,     0,     0,
       0,    22,    23,    24,    25,   316,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,    20,     0,     0,
     387,     1,     0,     0,     0,     2,     3,     0,     0,     0,
      22,    23,    24,    25,   316,     4,     5,     6,     7,     8,
       0,     0,     9,     0,    10,     0,     0,    11,    12,    13,
       1,     0,    14,     0,     2,    15,    16,    17,    18,     0,
       0,     0,     0,     0,     4,     5,     6,     7,     0,     0,
       0,     0,     0,     0,     0,     0,    11,     0,    13,    19,
      20,    21,     0,     0,     0,    16,    17,    18,     0,     0,
       0,     0,     0,    22,    23,    24,    25,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    20,
      53,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,    23,    24,    25,   139,   140,   141,   142,
     143,   144,     0,   145,     0,     0,   146,   147,   148,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   177,   149,   150,   151,   152,   153,   154,   155,   139,
     140,   141,   142,   143,   144,     0,   145,     0,     0,   146,
     147,   148,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   149,   150,   151,   152,   153,
     154,   155,   194,     0,     0,     0,     0,   178,   195,     0,
       0,     0,     0,     0,     0,     0,   139,   140,   141,   142,
     143,   144,     0,   145,     0,     0,   146,   147,   148,     0,
       0,     0,     0,     0,     0,   196,     0,     0,     0,     0,
       0,   197,   149,   150,   151,   152,   153,   154,   155,   139,
     140,   141,   142,   143,   144,     0,   145,     0,     0,   146,
     147,   148,     0,     0,     0,     0,     0,     0,   198,     0,
       0,     0,     0,     0,   199,   149,   150,   151,   152,   153,
     154,   155,   139,   140,   141,   142,   143,   144,     0,   145,
       0,     0,   146,   147,   148,     0,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,     0,   201,   149,   150,
     151,   152,   153,   154,   155,   139,   140,   141,   142,   143,
     144,     0,   145,     0,     0,   146,   147,   148,     0,     0,
       0,     0,     0,     0,   303,     0,     0,     0,     0,     0,
       0,   149,   150,   151,   152,   153,   154,   155,   139,   140,
     141,   142,   143,   144,     0,   145,     0,     0,   146,   147,
     148,     0,     0,     0,     0,     0,     0,   304,     0,     0,
       0,     0,     0,     0,   149,   150,   151,   152,   153,   154,
     155,   139,   140,   141,   142,   143,   144,     0,   145,     0,
       0,   146,   147,   148,     0,     0,     0,     0,     0,     0,
     305,     0,     0,     0,     0,     0,     0,   149,   150,   151,
     152,   153,   154,   155,   139,   140,   141,   142,   143,   144,
       0,   145,     0,     0,   146,   147,   148,     0,     0,     0,
       0,     0,     0,   306,     0,     0,     0,     0,     0,     0,
     149,   150,   151,   152,   153,   154,   155,   139,   140,   141,
     142,   143,   144,     0,   145,     0,     0,   146,   147,   148,
     139,   140,   141,   142,   143,   144,     0,   145,     0,     0,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
       0,     0,     0,     0,     0,     0,   149,   150,   151,   152,
     153,   154,   155,     0,     0,   173,     0,     0,     0,     0,
     204,   139,   140,   141,   142,   143,   144,     0,   145,     0,
       0,   146,   147,   148,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   149,   150,   151,
     152,   153,   154,   155,   139,   140,   141,   142,   143,   144,
       0,   145,     0,     0,   146,   147,   148,     0,     0,     0,
       0,     0,   202,   203,     0,     0,     0,     0,     0,     0,
     149,   150,   151,   152,   153,   154,   155,   139,   140,   141,
     142,   143,   144,     0,   145,     0,     0,   146,   147,   148,
     139,   140,   141,   142,     0,     0,     0,   145,     0,     0,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
       0,     0,     0,     0,     0,     0,   149,   150,   151,   152,
     153,   154,   155
};

static const yytype_int16 yycheck[] =
{
       0,   107,    17,    46,     0,   240,    49,   333,   263,    51,
      39,    43,    69,    13,    14,    15,    40,    24,    25,    26,
      63,    64,   101,   100,    51,    69,   103,   106,    72,   100,
      69,    54,    64,   260,    91,    92,    93,    94,    17,    18,
      19,    64,    42,    86,    87,    24,    90,    91,    92,    93,
      94,    30,   378,   308,    93,    94,    54,    54,     0,    74,
      84,    54,    56,    57,    54,   364,    64,    64,    48,   112,
      70,    64,   101,   300,    64,   302,    46,   376,    93,    79,
     101,    81,   381,    83,   100,   103,   313,   103,   100,    96,
      97,    98,    99,   102,   103,    95,   100,    97,    98,   103,
     100,   101,   329,    53,   100,   105,   105,   107,   343,   138,
     101,   102,   103,    24,   103,   158,   103,    96,    97,    98,
      99,   102,   103,   350,   230,   103,   361,   101,   102,   103,
      24,   102,   103,   157,   102,   103,   371,   106,   138,   159,
     160,   161,   377,   370,    36,    37,   106,   374,    48,   103,
     103,     1,    53,   380,    44,   106,    87,   384,    54,   159,
     160,   161,    86,    35,    14,    85,   103,    17,    18,    54,
      85,    21,    22,    23,    24,    25,    26,    53,   100,   253,
      30,    75,    76,    77,    78,    96,    97,    98,    99,   312,
     178,   191,    65,    66,    67,    68,    46,    47,    48,    49,
     307,    -1,    96,    97,    98,    99,    -1,   210,    -1,    -1,
     210,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,
     230,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    -1,    -1,   245,    96,    97,    98,    99,
     293,   101,   252,   253,   104,   255,     6,    -1,    -1,    -1,
      -1,    11,   262,    -1,    -1,    -1,    16,    17,    -1,    19,
      -1,   295,   296,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    -1,    -1,   310,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   296,    -1,    -1,   299,
      35,    -1,    -1,    53,    -1,    -1,    -1,    57,    58,    59,
      60,    61,    62,    -1,    -1,   339,    -1,    -1,    -1,    69,
      70,    -1,   322,   323,    -1,    -1,    -1,    62,    63,    64,
      65,    66,    67,   333,    69,    -1,   360,    72,    73,    74,
     340,    -1,   342,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,    -1,    88,    89,    90,    91,    92,    93,    94,
     360,    -1,    -1,   363,    -1,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   375,    -1,    -1,   378,    -1,
      -1,    -1,    -1,   383,    -1,    -1,   386,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,   164,   165,    -1,    -1,   168,   169,
     170,   171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,    -1,   205,   206,   207,    -1,   209,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    39,    40,    41,    42,   238,   239,
      -1,    -1,    47,   243,   244,    50,    51,    52,    -1,    -1,
      55,    -1,    -1,    24,    59,    60,    61,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    -1,    47,    82,    83,    50,
      51,    52,    -1,    -1,    55,    -1,    -1,   287,    59,    60,
      61,    96,    97,    98,    99,   100,    -1,    -1,   298,    -1,
      -1,    -1,    -1,   303,   304,   305,   306,   307,    -1,    -1,
      -1,    82,    83,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    96,    97,    98,    99,   100,
      -1,    -1,    -1,    -1,   334,   335,    -1,    -1,   338,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     0,     1,    -1,    -1,
      -1,   351,    -1,    -1,    -1,    -1,    -1,   357,    -1,    -1,
      14,    -1,    -1,    17,    18,    -1,   366,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    -1,
      44,    -1,    46,    47,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    -1,    -1,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    -1,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,     0,     1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,
      17,    18,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    -1,    44,    -1,    46,
      47,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      -1,    -1,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    -1,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
       0,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    14,    -1,    -1,    17,    18,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,
      40,    41,    42,    -1,    44,    -1,    46,    47,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    -1,    -1,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    -1,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,     0,     1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    14,    -1,    -1,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      -1,    44,    -1,    46,    47,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    -1,    -1,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    -1,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     1,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,    44,    14,    -1,    -1,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    30,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,     0,     1,    -1,    96,    97,    98,
      99,    -1,   101,    -1,    -1,   104,   105,   106,    14,    -1,
      -1,    17,    18,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      36,    37,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,    55,
      -1,    -1,    -1,    59,    60,    61,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    -1,    85,
      86,    87,     0,     1,    -1,    91,    92,    -1,    -1,    -1,
      96,    97,    98,    99,   100,   101,    14,   103,   104,    17,
      18,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,    -1,
      -1,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    -1,    85,    86,    87,
      -1,     1,    -1,    91,    92,    -1,    -1,    -1,    96,    97,
      98,    99,   100,   101,    14,   103,   104,    17,    18,    -1,
      -1,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,    48,    14,
      -1,    -1,    17,    18,    -1,    -1,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    30,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,    -1,    68,   104,    -1,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,     1,    -1,
      -1,    96,    97,    98,    99,    -1,   101,   102,   103,   104,
      -1,    14,    -1,    -1,    17,    18,    -1,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    -1,    -1,    17,    18,    -1,    -1,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      -1,    -1,    -1,    96,    97,    98,    99,    68,   101,    -1,
      -1,   104,    -1,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
      91,    92,    -1,    -1,    -1,    96,    97,    98,    99,    -1,
     101,    14,    -1,   104,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,
      52,    -1,    -1,    55,    -1,    -1,    -1,    59,    60,    61,
      -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      82,    83,    -1,    96,    97,    98,    99,    -1,   101,    24,
      -1,   104,    -1,    28,    96,    97,    98,    99,   100,    -1,
      -1,    -1,    -1,    38,    39,    40,    41,    42,    -1,    -1,
      -1,    -1,    47,    -1,    -1,    50,    51,    52,    24,    -1,
      55,    -1,    28,    -1,    59,    60,    61,    -1,    -1,    -1,
      -1,    -1,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    50,    51,    52,    82,    83,    55,
      -1,    86,    -1,    59,    60,    61,    -1,    -1,    -1,    -1,
      -1,    96,    97,    98,    99,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    82,    83,    -1,    -1,
      86,    24,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,
      96,    97,    98,    99,   100,    38,    39,    40,    41,    42,
      -1,    -1,    45,    -1,    47,    -1,    -1,    50,    51,    52,
      24,    -1,    55,    -1,    28,    58,    59,    60,    61,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    52,    82,
      83,    84,    -1,    -1,    -1,    59,    60,    61,    -1,    -1,
      -1,    -1,    -1,    96,    97,    98,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,
      46,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    96,    97,    98,    99,    62,    63,    64,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    88,    89,    90,    91,    92,    93,    94,    62,
      63,    64,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    48,    -1,    -1,    -1,    -1,   100,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,
      -1,    54,    88,    89,    90,    91,    92,    93,    94,    62,
      63,    64,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      -1,    -1,    -1,    -1,    54,    88,    89,    90,    91,    92,
      93,    94,    62,    63,    64,    65,    66,    67,    -1,    69,
      -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    -1,    -1,    -1,    -1,    54,    88,    89,
      90,    91,    92,    93,    94,    62,    63,    64,    65,    66,
      67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    62,    63,
      64,    65,    66,    67,    -1,    69,    -1,    -1,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    62,    63,    64,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    62,    63,    64,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    62,    63,    64,
      65,    66,    67,    -1,    69,    -1,    -1,    72,    73,    74,
      62,    63,    64,    65,    66,    67,    -1,    69,    -1,    -1,
      72,    73,    74,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    -1,    -1,    56,    -1,    -1,    -1,    -1,
     102,    62,    63,    64,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    62,    63,    64,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    62,    63,    64,
      65,    66,    67,    -1,    69,    -1,    -1,    72,    73,    74,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    -1,    -1,
      72,    73,    74,    88,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    24,    28,    29,    38,    39,    40,    41,    42,    45,
      47,    50,    51,    52,    55,    58,    59,    60,    61,    82,
      83,    84,    96,    97,    98,    99,   108,   109,   110,   111,
     112,   113,   125,   131,   140,   148,   150,   152,   155,   151,
     154,    51,   141,     1,    14,    17,    18,    21,    22,    23,
      25,    26,    30,    46,    47,    48,    49,    68,    75,    76,
      77,    78,    79,    91,    92,    96,    97,    98,    99,   101,
     104,   110,   134,   136,   137,   138,   139,   149,   162,   153,
      51,    20,   137,    56,    57,    17,    18,    19,    30,   110,
     110,   110,   137,   137,   149,   143,   137,   142,   156,     0,
     100,   101,   137,   160,   137,   132,   160,   132,   110,   145,
     100,   136,    48,   136,   137,    46,   137,   137,   137,   137,
     137,   137,   136,   136,   134,   134,   134,   134,   137,    25,
      26,   106,   110,   137,   163,   164,   165,   166,   101,    62,
      63,    64,    65,    66,    67,    69,    72,    73,    74,    88,
      89,    90,    91,    92,    93,    94,   149,   110,   110,    44,
      70,    71,   110,   132,    54,    64,   136,   136,    54,    64,
      54,    64,    53,    56,   149,   110,   147,    54,   100,   110,
     146,   110,   108,   110,   137,   160,   103,   128,   110,   157,
     157,   103,   136,   137,    48,    54,    48,    54,    48,    54,
      48,    54,    80,    81,   102,   106,   106,   106,   105,   103,
     103,   110,   160,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     132,   136,   110,   138,   110,   138,   110,   138,    43,    64,
     137,   137,   137,    54,    64,    48,   137,   137,   137,   137,
     137,   137,   103,   124,   124,   103,   102,   103,   102,   137,
      53,   100,   103,   100,   110,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,    25,
      26,   110,   166,   102,   103,   102,   157,    44,   137,   137,
     128,   137,   137,   110,   110,   112,   133,   110,   144,   133,
     159,   110,   159,    48,    48,    48,    48,   135,   100,   137,
     120,   114,   117,   123,   136,   132,   100,   113,   132,   137,
     161,   129,   133,   133,   137,   137,   137,   137,   161,   159,
     132,   130,   130,   133,    54,    64,    87,   102,   103,   126,
      27,   102,   133,   121,   115,   118,   129,   137,   137,   137,
      36,    37,   127,   132,   110,   158,   128,    54,    35,    86,
     133,   137,    85,   103,   122,   137,    54,   132,   128,   110,
     123,   116,   137,    85,    53,   133,   128,   119,   133,    86,
     123,   128,   129,   133,   123,    86,   133,    86
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   107,   108,   108,   109,   109,   109,   110,   110,   110,
     110,   110,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   112,
     112,   113,   113,   113,   114,   115,   116,   113,   117,   118,
     119,   113,   120,   121,   122,   113,   113,   113,   113,   113,
     123,   124,   125,   125,   126,   126,   127,   128,   129,   130,
     131,   132,   133,   133,   133,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   135,   134,   134,   134,   134,   136,   136,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   139,   139,   139,   139,
     140,   140,   140,   140,   140,   141,   140,   142,   140,   143,
     140,   140,   140,   140,   144,   140,   140,   140,   145,   145,
     146,   146,   147,   147,   148,   148,   148,   148,   148,   148,
     149,   149,   150,   150,   150,   151,   150,   153,   152,   152,
     154,   152,   152,   152,   156,   155,   157,   157,   157,   158,
     158,   158,   159,   160,   160,   160,   161,   161,   162,   163,
     163,   163,   163,   164,   164,   165,   165,   166,   166,   166
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     4,     4,     6,     4,     4,     4,     4,     4,
       4,     4,     4,     5,     5,     8,     8,     4,     4,     1,
       1,     1,     1,     9,     0,     0,     0,    15,     0,     0,
       0,    16,     0,     0,     0,    13,     2,     4,     7,     6,
       0,     0,     9,    11,     0,     2,     6,     0,     0,     0,
       1,     0,     0,     2,     2,     1,     1,     2,     2,     1,
       1,     1,     1,     2,     2,     3,     1,     4,     4,     1,
       4,     0,     7,     4,     1,     2,     1,     3,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     3,     3,     3,     3,     2,     4,     6,
       4,     6,     4,     6,     4,     6,     1,     2,     2,     2,
       2,     1,     1,     2,     1,     0,     3,     0,     3,     0,
       3,     4,     2,     4,     0,     7,     4,     2,     1,     3,
       1,     3,     1,     3,     1,     2,     2,     2,     3,     2,
       3,     2,     2,     3,     2,     0,     3,     0,     8,     2,
       0,     7,     8,     6,     0,     3,     0,     1,     3,     0,
       1,     3,     0,     0,     1,     3,     1,     3,     3,     0,
       1,     1,     1,     1,     3,     1,     3,     3,     3,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




/* The kind of the lookahead of this context.  */
static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx) YY_ATTRIBUTE_UNUSED;

static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx)
{
  return yyctx->yytoken;
}



/* User defined function to report a syntax error.  */
static int
yyreport_syntax_error (const yypcontext_t *yyctx);

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case 22: /* THEFUNC  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1937 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* THEFUNCINOF  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1943 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* VARID  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1949 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* STRING  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1955 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* SYMBOL  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1961 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* ENDCLAUSE  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1967 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* tPLAYACCEL  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1973 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* tMETHOD  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1979 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 110: /* ID  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1985 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 155: /* on  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1991 "engines/director/lingo/lingo-gr.cpp"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize;

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yynerrs = 0;
  yystate = 0;
  yyerrstatus = 0;

  yystacksize = YYINITDEPTH;
  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;


  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 8:
#line 222 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 2272 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 223 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 2278 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 224 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 2284 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 225 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 2290 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 227 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2302 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 234 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2310 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 238 "engines/director/lingo/lingo-gr.y"
                                                                {
		if (!(yyvsp[-3].s)->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		g_lingo->code1(LC::c_themenuentitypush);
		g_lingo->codeInt((yyvsp[-4].e)[0]);
		g_lingo->codeInt((yyvsp[-4].e)[1]);
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code); }
#line 2326 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 249 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_after);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2338 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 256 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_after);
		(yyval.code) = (yyvsp[-2].code); }
#line 2346 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 259 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_before);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 266 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_before);
		(yyval.code) = (yyvsp[-2].code); }
#line 2366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 269 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2378 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20:
#line 276 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2390 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21:
#line 283 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2402 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22:
#line 290 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2414 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23:
#line 297 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2425 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24:
#line 303 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2436 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25:
#line 310 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if (!(yyvsp[-3].s)->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		warning("STUB: menuItem entity");
		g_lingo->code1(LC::c_themenuitementityassign);
		g_lingo->codeInt((yyvsp[-6].e)[0]);
		g_lingo->codeInt((yyvsp[-6].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2452 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26:
#line 321 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if (!(yyvsp[-3].s)->equalsIgnoreCase("menu")) {
			warning("LEXER: keyword 'menu' expected");
			YYERROR;
		}

		warning("STUB: menuItem entity");
		g_lingo->code1(LC::c_themenuitementityassign);
		g_lingo->codeInt((yyvsp[-6].e)[0]);
		g_lingo->codeInt((yyvsp[-6].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2468 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27:
#line 332 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2480 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28:
#line 339 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 356 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-6].code) - (yyvsp[-1].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-4].code) + 2);
		(*g_lingo->_currentAssembly)[(yyvsp[-4].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentAssembly)[(yyvsp[-1].code)] = start;	/* looping back */
		endRepeat((yyvsp[-1].code) + 1, (yyvsp[-6].code));	}
#line 2504 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 369 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2512 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 373 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2519 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 376 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_le); }
#line 2525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 376 "engines/director/lingo/lingo-gr.y"
                                                                                                      {

		int nextPos = g_lingo->_currentAssembly->size();
		g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[-12].s)->c_str());
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_add);
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-12].s)->c_str());
		g_lingo->code1(LC::c_assign);
		g_lingo->code2(LC::c_jump, 0);
		int pos = g_lingo->_currentAssembly->size() - 1;

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-8].code) - pos + 2);
		WRITE_UINT32(&end, pos - (yyvsp[-3].code) + 2);
		(*g_lingo->_currentAssembly)[pos] = loop;		/* final count value */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end;	/* end, if cond fails */
		endRepeat(pos + 1, nextPos); }
#line 2550 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 403 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2558 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 407 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2565 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 410 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_ge); }
#line 2571 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 411 "engines/director/lingo/lingo-gr.y"
                                                                {

		int nextPos = g_lingo->_currentAssembly->size();
		g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[-13].s)->c_str());
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_sub);
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-13].s)->c_str());
		g_lingo->code1(LC::c_assign);
		g_lingo->code2(LC::c_jump, 0);
		int pos = g_lingo->_currentAssembly->size() - 1;

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-9].code) - pos + 2);
		WRITE_UINT32(&end, pos - (yyvsp[-3].code) + 2);
		(*g_lingo->_currentAssembly)[pos] = loop;		/* final count value */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end;	/* end, if cond fails */
		endRepeat(pos + 1, nextPos); }
#line 2596 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 437 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  Common::String count("count");
				  g_lingo->codeFunc(&count, 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
#line 2607 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 444 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
#line 2617 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 450 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get list
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(1);
				  Common::String getAt("getAt");
				  g_lingo->codeFunc(&getAt, 2);
				  g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-6].s)->c_str());
				  mVar((yyvsp[-6].s), globalCheck());
				  g_lingo->code1(LC::c_assign); }
#line 2632 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 460 "engines/director/lingo/lingo-gr.y"
                                                        {

		int nextPos = g_lingo->_currentAssembly->size();
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_add);			// Increment counter

		int jump = g_lingo->code2(LC::c_jump, 0);

		int end2 = g_lingo->code1(LC::c_stackdrop);	// remove list, size, counter
		g_lingo->codeInt(3);

		inst loop = 0, end = 0;
		WRITE_UINT32(&loop, (yyvsp[-6].code) - jump);
		WRITE_UINT32(&end, end2 - (yyvsp[-4].code) + 1);

		(*g_lingo->_currentAssembly)[jump + 1] = loop;		/* final count value */
		(*g_lingo->_currentAssembly)[(yyvsp[-4].code)] = end;		/* end, if cond fails */
		endRepeat(end2, nextPos); }
#line 2656 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 480 "engines/director/lingo/lingo-gr.y"
                        {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->nexts.push_back(pos);
		} else {
			warning("# LINGO: next repeat not inside repeat block");
		} }
#line 2669 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 488 "engines/director/lingo/lingo-gr.y"
                              {
		g_lingo->code1(LC::c_whencode);
		g_lingo->codeString((yyvsp[-2].s)->c_str()); }
#line 2677 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 491 "engines/director/lingo/lingo-gr.y"
                                                          { g_lingo->code1(LC::c_telldone); }
#line 2683 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 492 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->code1(LC::c_telldone); }
#line 2689 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 494 "engines/director/lingo/lingo-gr.y"
                                { startRepeat(); }
#line 2695 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 496 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_tell); }
#line 2701 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 498 "engines/director/lingo/lingo-gr.y"
                                                                                         {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2713 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 505 "engines/director/lingo/lingo-gr.y"
                                                                                                          {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].code) + 1 - (yyvsp[-8].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-8].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-5].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-5].code), (yyvsp[-1].code)); }
#line 2725 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 516 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 522 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2743 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 526 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2751 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 530 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 534 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2766 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 537 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2772 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 539 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2778 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 543 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2786 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 546 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2794 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 549 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[0].code); }
#line 2800 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 550 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2806 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 551 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2815 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 555 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2824 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 560 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 568 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2848 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 574 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2856 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 577 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2864 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 580 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_objectproppush);
		g_lingo->codeString((yyvsp[0].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectprop).prop->c_str());
		delete (yyvsp[0].objectprop).obj;
		delete (yyvsp[0].objectprop).prop; }
#line 2875 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 586 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_intersects); }
#line 2881 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 587 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 2887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 589 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_lazyeval);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeFunc((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 2898 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 595 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_lazyeval); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 2904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 596 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeFunc((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 2913 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 600 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2921 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 603 "engines/director/lingo/lingo-gr.y"
                        {
		if (g_lingo->_builtinConsts.contains(*(yyvsp[0].s))) {
			(yyval.code) = g_lingo->code1(LC::c_constpush);
		} else {
			(yyval.code) = g_lingo->code1(LC::c_eval);
		}
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2934 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 611 "engines/director/lingo/lingo-gr.y"
                                { yyerrok; }
#line 2940 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 614 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 616 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2952 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 617 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2958 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 618 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2964 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 619 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2970 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 620 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2976 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 621 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2982 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 622 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2988 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 623 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2994 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 624 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 3000 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 625 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 3006 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 626 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 3012 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 627 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 3018 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 628 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 3024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 629 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 3030 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 630 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 3036 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 631 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 3042 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 632 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 3048 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 633 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 3054 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 634 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 3060 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 636 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String field("field");
		g_lingo->codeFunc(&field, 1); }
#line 3068 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 639 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charOf); }
#line 3074 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 640 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charToOf); }
#line 3080 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 641 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemOf); }
#line 3086 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 642 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_itemToOf); }
#line 3092 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 643 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineOf); }
#line 3098 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 644 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_lineToOf); }
#line 3104 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 645 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordOf); }
#line 3110 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 646 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_wordToOf); }
#line 3116 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 649 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String cast("cast");
		g_lingo->codeFunc(&cast, 1); }
#line 3124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 652 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String script("script");
		g_lingo->codeFunc(&script, 1); }
#line 3132 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 655 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String window("window");
		g_lingo->codeFunc(&window, 1); }
#line 3140 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 659 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_printtop); }
#line 3146 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 662 "engines/director/lingo/lingo-gr.y"
                                                {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->exits.push_back(pos);
		} else {
			warning("# LINGO: exit repeat not inside repeat block");
		} }
#line 3159 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 670 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 3165 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 671 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3171 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 671 "engines/director/lingo/lingo-gr.y"
                                                                                 { inLast(); }
#line 3177 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 672 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 672 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 3189 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 673 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3195 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 673 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 3201 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 674 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String open("open");
		g_lingo->codeFunc(&open, 2); }
#line 3209 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 677 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String open("open");
		g_lingo->codeFunc(&open, 1); }
#line 3217 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 680 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_lazyeval);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeCmd((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 3228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 686 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_lazyeval); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 3234 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 687 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeCmd((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 3243 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 691 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeCmd((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3251 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 694 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeCmd((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3259 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 698 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3267 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 701 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3275 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 705 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3283 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 708 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3291 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 712 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3299 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 715 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 726 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145:
#line 727 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3319 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 728 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3325 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 729 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 733 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3343 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 737 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3352 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 745 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3361 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 749 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 753 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 757 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3385 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 757 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3394 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 787 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3400 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 788 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3410 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 793 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3416 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 794 "engines/director/lingo/lingo-gr.y"
                  { startDef(); (*g_lingo->_methodVars)["me"] = kVarArgument; }
#line 3422 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 795 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1);
		endDef();
		delete (yyvsp[-6].s); }
#line 3432 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 800 "engines/director/lingo/lingo-gr.y"
                                                                   {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3445 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 808 "engines/director/lingo/lingo-gr.y"
                                               {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3455 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 814 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3461 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 814 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = (yyvsp[0].s); }
#line 3467 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 816 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3473 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 817 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3479 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 818 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3485 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 821 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3491 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 822 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3497 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172:
#line 824 "engines/director/lingo/lingo-gr.y"
                                        { inDef(); }
#line 3503 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173:
#line 826 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3509 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174:
#line 827 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3515 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175:
#line 828 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3521 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176:
#line 830 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3527 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177:
#line 831 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3533 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178:
#line 833 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3539 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179:
#line 835 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3545 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180:
#line 836 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3551 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181:
#line 837 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3557 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182:
#line 838 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3563 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183:
#line 840 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3569 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184:
#line 841 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3575 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185:
#line 843 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3581 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186:
#line 844 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3587 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187:
#line 846 "engines/director/lingo/lingo-gr.y"
                          {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3596 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188:
#line 850 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189:
#line 854 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3614 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3618 "engines/director/lingo/lingo-gr.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        if (yyreport_syntax_error (&yyctx) == 2)
          goto yyexhaustedlab;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

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


#if 1
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 860 "engines/director/lingo/lingo-gr.y"


int yyreport_syntax_error(const yypcontext_t *ctx) {
	int res = 0;

	Common::String msg = "syntax error, ";

	// Report the unexpected token.
	yysymbol_kind_t lookahead = yypcontext_token(ctx);
	if (lookahead != YYSYMBOL_YYEMPTY)
		msg += Common::String::format("unexpected %s", yysymbol_name(lookahead));

	// Report the tokens expected at this point.
	enum { TOKENMAX = 10 };
	yysymbol_kind_t expected[TOKENMAX];

	int n = yypcontext_expected_tokens(ctx, expected, TOKENMAX);
	if (n < 0)
		// Forward errors to yyparse.
		res = n;
	else
		for (int i = 0; i < n; ++i)
			msg += Common::String::format("%s %s", i == 0 ? ": expected" : " or", yysymbol_name(expected[i]));

	yyerror(msg.c_str());

	return res;
}
