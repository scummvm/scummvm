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
    ID = 279,                      /* ID  */
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
    tENDTELL = 342                 /* tENDTELL  */
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

	struct {
		Common::String *s;
		int i;
	} ref;

#line 334 "engines/director/lingo/lingo-gr.cpp"

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
  YYSYMBOL_ID = 24,                        /* ID  */
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
  YYSYMBOL_96_n_ = 96,                     /* '\n'  */
  YYSYMBOL_97_ = 97,                       /* '('  */
  YYSYMBOL_98_ = 98,                       /* ')'  */
  YYSYMBOL_99_ = 99,                       /* ','  */
  YYSYMBOL_100_ = 100,                     /* '['  */
  YYSYMBOL_101_ = 101,                     /* ']'  */
  YYSYMBOL_102_ = 102,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 103,                 /* $accept  */
  YYSYMBOL_program = 104,                  /* program  */
  YYSYMBOL_programline = 105,              /* programline  */
  YYSYMBOL_asgn = 106,                     /* asgn  */
  YYSYMBOL_stmtoneliner = 107,             /* stmtoneliner  */
  YYSYMBOL_stmt = 108,                     /* stmt  */
  YYSYMBOL_109_1 = 109,                    /* $@1  */
  YYSYMBOL_110_2 = 110,                    /* $@2  */
  YYSYMBOL_111_3 = 111,                    /* $@3  */
  YYSYMBOL_112_4 = 112,                    /* $@4  */
  YYSYMBOL_113_5 = 113,                    /* $@5  */
  YYSYMBOL_114_6 = 114,                    /* $@6  */
  YYSYMBOL_115_7 = 115,                    /* $@7  */
  YYSYMBOL_116_8 = 116,                    /* $@8  */
  YYSYMBOL_117_9 = 117,                    /* $@9  */
  YYSYMBOL_startrepeat = 118,              /* startrepeat  */
  YYSYMBOL_tellstart = 119,                /* tellstart  */
  YYSYMBOL_ifstmt = 120,                   /* ifstmt  */
  YYSYMBOL_elseifstmtlist = 121,           /* elseifstmtlist  */
  YYSYMBOL_elseifstmt = 122,               /* elseifstmt  */
  YYSYMBOL_jumpifz = 123,                  /* jumpifz  */
  YYSYMBOL_jump = 124,                     /* jump  */
  YYSYMBOL_varassign = 125,                /* varassign  */
  YYSYMBOL_if = 126,                       /* if  */
  YYSYMBOL_lbl = 127,                      /* lbl  */
  YYSYMBOL_stmtlist = 128,                 /* stmtlist  */
  YYSYMBOL_simpleexpr = 129,               /* simpleexpr  */
  YYSYMBOL_expr = 130,                     /* expr  */
  YYSYMBOL_131_10 = 131,                   /* $@10  */
  YYSYMBOL_chunkexpr = 132,                /* chunkexpr  */
  YYSYMBOL_reference = 133,                /* reference  */
  YYSYMBOL_proc = 134,                     /* proc  */
  YYSYMBOL_135_11 = 135,                   /* $@11  */
  YYSYMBOL_136_12 = 136,                   /* $@12  */
  YYSYMBOL_137_13 = 137,                   /* $@13  */
  YYSYMBOL_138_14 = 138,                   /* $@14  */
  YYSYMBOL_globallist = 139,               /* globallist  */
  YYSYMBOL_propertylist = 140,             /* propertylist  */
  YYSYMBOL_instancelist = 141,             /* instancelist  */
  YYSYMBOL_gotofunc = 142,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 143,                /* gotomovie  */
  YYSYMBOL_playfunc = 144,                 /* playfunc  */
  YYSYMBOL_145_15 = 145,                   /* $@15  */
  YYSYMBOL_defn = 146,                     /* defn  */
  YYSYMBOL_147_16 = 147,                   /* $@16  */
  YYSYMBOL_148_17 = 148,                   /* $@17  */
  YYSYMBOL_on = 149,                       /* on  */
  YYSYMBOL_150_18 = 150,                   /* $@18  */
  YYSYMBOL_argdef = 151,                   /* argdef  */
  YYSYMBOL_endargdef = 152,                /* endargdef  */
  YYSYMBOL_argstore = 153,                 /* argstore  */
  YYSYMBOL_arglist = 154,                  /* arglist  */
  YYSYMBOL_nonemptyarglist = 155,          /* nonemptyarglist  */
  YYSYMBOL_list = 156,                     /* list  */
  YYSYMBOL_valuelist = 157,                /* valuelist  */
  YYSYMBOL_linearlist = 158,               /* linearlist  */
  YYSYMBOL_proplist = 159,                 /* proplist  */
  YYSYMBOL_proppair = 160                  /* proppair  */
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
#define YYFINAL  96
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1629

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  103
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  58
/* YYNRULES -- Number of rules.  */
#define YYNRULES  183
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  379

#define YYMAXUTOK   342


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
      96,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    95,    90,     2,
      97,    98,    93,    91,    99,    92,     2,    94,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   102,     2,
      88,     2,    89,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   100,     2,   101,     2,     2,     2,     2,     2,     2,
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
      85,    86,    87
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   218,   218,   219,   221,   222,   223,   225,   232,   236,
     247,   254,   257,   264,   267,   274,   281,   288,   295,   301,
     308,   319,   330,   337,   345,   346,   348,   349,   354,   367,
     371,   374,   366,   401,   405,   408,   400,   435,   442,   448,
     434,   478,   486,   489,   490,   492,   494,   496,   503,   511,
     512,   514,   520,   524,   528,   532,   535,   537,   538,   539,
     541,   544,   547,   551,   555,   563,   571,   577,   578,   579,
     581,   582,   583,   589,   589,   594,   597,   600,   603,   609,
     610,   611,   612,   613,   614,   615,   616,   617,   618,   619,
     620,   621,   622,   623,   624,   625,   626,   627,   628,   629,
     630,   632,   643,   644,   645,   646,   647,   648,   649,   650,
     652,   653,   664,   675,   688,   689,   690,   691,   699,   700,
     700,   701,   701,   702,   702,   703,   706,   709,   715,   715,
     720,   723,   727,   730,   734,   737,   741,   744,   755,   756,
     757,   758,   762,   766,   771,   772,   774,   778,   782,   786,
     786,   816,   816,   822,   823,   823,   829,   837,   843,   843,
     845,   846,   847,   849,   850,   851,   853,   855,   856,   857,
     859,   860,   862,   864,   865,   866,   867,   869,   870,   872,
     873,   875,   879,   883
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
  "THEFUNCINOF", "ID", "STRING", "SYMBOL", "ENDCLAUSE", "tPLAYACCEL",
  "tMETHOD", "THEOBJECTPROP", "CAST", "FIELD", "SCRIPT", "WINDOW", "tDOWN",
  "tELSE", "tELSIF", "tEXIT", "tGLOBAL", "tGO", "tGOLOOP", "tIF", "tIN",
  "tINTO", "tMACRO", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT",
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE",
  "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tITEM", "tLINE", "tWORD", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tENDIF",
  "tENDREPEAT", "tENDTELL", "'<'", "'>'", "'&'", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'\\n'", "'('", "')'", "','", "'['", "']'", "':'",
  "$accept", "program", "programline", "asgn", "stmtoneliner", "stmt",
  "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9",
  "startrepeat", "tellstart", "ifstmt", "elseifstmtlist", "elseifstmt",
  "jumpifz", "jump", "varassign", "if", "lbl", "stmtlist", "simpleexpr",
  "expr", "$@10", "chunkexpr", "reference", "proc", "$@11", "$@12", "$@13",
  "$@14", "globallist", "propertylist", "instancelist", "gotofunc",
  "gotomovie", "playfunc", "$@15", "defn", "$@16", "$@17", "on", "$@18",
  "argdef", "endargdef", "argstore", "arglist", "nonemptyarglist", "list",
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
      38,    43,    45,    42,    47,    37,    10,    40,    41,    44,
      91,    93,    58
};
#endif

#define YYPACT_NINF (-337)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-174)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1036,   367,  -337,  -337,   -21,  -337,   542,  -337,  -337,  -337,
      15,   775,   152,   171,    13,    44,   801,   581,  -337,   801,
    -337,  -337,    41,    -5,  -337,  -337,  -337,  -337,   801,  -337,
    -337,  -337,  -337,  -337,    22,  -337,  -337,     3,  -337,    50,
       3,    -1,  -337,  -337,  -337,  -337,  -337,  -337,  -337,   801,
     801,   801,   801,   801,   801,   801,   801,   662,   278,  -337,
    1522,  -337,  -337,    43,  -337,   455,  -337,  -337,   116,   801,
    -337,   103,  -337,   801,  1103,  -337,   122,  -337,   127,  1070,
     140,  -337,   -46,     3,     3,   -22,    17,   124,  -337,  1439,
    1103,  -337,   176,  1400,   186,   190,  -337,  1036,  1522,   196,
    -337,  -337,  -337,     3,  -337,   688,  -337,  1136,  1169,  1202,
    1235,  1489,  -337,  -337,    77,  1452,   -39,     0,   129,   132,
    -337,  1522,   123,   136,   137,  -337,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,    43,   196,  -337,   138,  1522,   801,
    1452,  -337,  -337,     3,     1,    29,    33,    -8,   801,   801,
     801,    28,   191,   801,   801,   801,   801,   801,   801,  -337,
    -337,   141,  -337,  -337,  -337,   142,  -337,  -337,   173,  -337,
     -11,  -337,   100,   114,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,  -337,  -337,  -337,  -337,   801,   801,
     801,  -337,   801,   178,    76,    76,    76,    76,  1535,  1535,
    -337,    21,    76,    76,    76,    76,    21,   -54,   -54,  -337,
    -337,  1522,    20,   218,  1522,   196,   199,  -337,  -337,  -337,
    -337,  -337,  -337,   801,   801,  1522,  1522,  1522,   801,   801,
     220,  1522,  1522,  1522,  1522,  1522,  1522,   221,    34,  -337,
     222,  -337,  -337,   223,  -337,  -337,  -337,  1522,  1268,  1522,
    1301,  1522,  1334,  1522,  1367,  1522,  1522,   801,  1522,  1522,
    1522,  1522,   146,   129,   132,  -337,  -337,  -337,    48,   801,
    1522,  1037,  -337,  1522,  1522,     3,  -337,  -337,   997,  -337,
     997,  -337,  -337,   801,   801,   801,   801,   801,  1522,   118,
    -337,  -337,  1522,  -337,  -337,  -337,  -337,   109,  -337,  -337,
    -337,   162,  -337,   867,   120,  1522,  1522,  1522,  1522,  -337,
     801,   997,  -337,  -337,  -337,  -337,   997,   801,   801,  -337,
     193,   226,  -337,  1522,   997,  -337,   197,   217,   167,  1522,
    1522,  -337,   801,  -337,   169,  -337,   156,  -337,   801,   202,
    -337,   997,  1522,  -337,   233,  -337,  1522,   801,   174,   205,
    -337,  -337,  -337,  1522,  -337,  -337,   892,  -337,  -337,   997,
    -337,  -337,  -337,  -337,   930,  -337,  -337,   959,  -337
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,   149,   154,   118,   119,     0,   138,    55,   151,
       0,     0,     0,     0,     0,     0,     0,     0,   123,     0,
     121,   158,     0,     2,    24,    26,     6,    27,     0,    25,
     115,   116,     5,    56,     0,    60,    65,     0,    61,     0,
       0,    64,    63,    62,    78,   111,   101,   112,   113,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    70,
     168,   110,    71,   131,    68,     0,    56,   117,     0,     0,
     139,     0,   140,     0,   141,   143,     0,    41,     0,   114,
       0,    56,     0,     0,     0,     0,     0,     0,   153,   126,
     146,   148,     0,     0,     0,     0,     1,     4,    52,   160,
      69,    64,    66,     0,    76,     0,    92,     0,     0,     0,
       0,     0,    97,    98,    64,   168,     0,    64,    63,    62,
     174,   177,     0,   176,   175,   179,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   150,   160,   132,   120,   145,     0,
       0,   142,    56,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   147,
     136,   124,    46,    46,   134,   122,   159,     3,     0,   161,
       0,    77,    64,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   127,   128,    67,   130,     0,     0,
       0,   172,     0,     0,    88,    89,    86,    87,    90,    91,
      83,    94,    95,    96,    85,    84,    93,    79,    80,    81,
      82,   169,     0,     0,   144,   160,     0,     7,     8,    10,
      11,    12,    13,     0,     0,    52,    17,    15,     0,     0,
       0,    16,    14,    22,    23,    42,   125,     0,     0,    57,
       0,    57,   166,     0,    72,    73,    75,   102,     0,   104,
       0,   106,     0,   108,     0,    99,   100,     0,   183,   182,
     181,   178,     0,     0,     0,   180,   166,   133,     0,     0,
      37,    29,    45,    18,    19,     0,   137,    56,    56,   135,
      53,    57,   162,     0,     0,     0,     0,     0,   170,     0,
      57,   166,     9,    56,    54,    54,    57,     0,    44,    58,
      59,     0,    49,   157,     0,   103,   105,   107,   109,   129,
       0,   155,    57,    38,    30,    34,    53,     0,     0,    43,
      56,   163,    74,   171,   152,    52,     0,     0,     0,    20,
      21,    57,     0,    50,     0,   164,   156,    39,     0,     0,
      28,    56,    52,    47,     0,    45,    31,     0,     0,     0,
     165,    57,    52,    35,    48,    57,     0,    45,    52,    53,
      40,    57,    45,    51,     0,    57,    32,     0,    36
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -337,   163,  -337,  -337,    14,     2,  -337,  -337,  -337,  -337,
    -337,  -337,  -337,  -337,  -337,  -336,    90,  -337,  -337,  -337,
    -197,  -314,   -41,  -337,   -65,  -150,   -14,    -6,  -337,    51,
    -337,  -337,  -337,  -337,  -337,  -337,  -337,  -337,  -337,  -337,
     -10,  -337,  -337,  -337,  -337,  -337,  -337,  -337,  -142,  -337,
    -267,   -51,   -27,  -337,  -337,  -337,  -337,    64
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    22,    23,    24,    25,   310,   304,   336,   362,   305,
     337,   368,   303,   335,   355,   306,   248,    27,   330,   343,
     178,   312,   324,    28,    99,   288,    59,    60,   293,    61,
      62,    29,    68,    94,    92,   267,   147,   175,   171,    30,
      75,    31,    65,    32,    76,    66,    33,    95,   180,   346,
     291,    63,   299,    64,   122,   123,   124,   125
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      74,   145,    26,   222,    34,    79,   116,    91,   159,   300,
      89,    90,   338,    93,   144,   132,   158,    35,   160,   361,
      36,    37,    98,   102,    38,   227,   104,   101,    42,    43,
      67,   371,   163,    46,   322,   233,   375,    87,   282,   141,
     142,    96,   164,   106,   107,   108,   109,   110,   111,   112,
     113,   115,   121,   229,   183,   373,   234,   231,     1,   197,
     143,    46,     2,   148,   151,    46,    77,   150,    88,   161,
     162,   165,     4,     5,     6,     7,    50,    51,    52,    53,
     169,   166,   238,   278,    11,   252,    13,   225,   253,   181,
     132,    97,   239,    16,    17,    18,   105,   105,   103,    26,
      73,   290,   198,    58,    50,    51,    52,    53,    50,    51,
      52,    53,   139,   140,   141,   142,   276,    20,   100,   253,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   347,   226,
     146,   313,   143,   224,   301,   132,   152,   253,   133,   149,
     321,   153,   235,   236,   237,   359,   326,   241,   242,   243,
     244,   245,   246,   327,   157,   367,   138,   139,   140,   141,
     142,   372,   334,   328,   105,   194,   195,   167,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,    82,    83,
      84,   351,   268,   269,   270,    85,   271,   105,   254,   255,
     170,    86,   272,   273,   274,   228,   230,   232,    80,    81,
     174,   366,   256,   143,   176,   369,   319,   320,   332,   320,
     179,   374,   308,   311,   201,   377,   251,   280,   281,   341,
     342,   199,   283,   284,   200,   202,   203,   223,   323,   240,
     247,   250,   277,   279,   285,   286,   289,   292,   198,   329,
     345,   348,   349,   350,   353,   354,   357,   360,   365,   364,
     177,   298,   287,   249,   325,   344,   314,   275,     0,     0,
       0,   307,     0,   302,     0,     0,     0,     0,     0,    34,
       0,     0,     0,     0,     0,     0,   358,   298,   315,   316,
     317,   318,    35,     0,     0,    36,    37,     0,     0,    38,
      39,    40,   117,   118,   119,     0,     0,     0,    44,    45,
      46,    47,    48,     0,   333,     0,     0,     0,     0,     0,
       0,   339,   340,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   352,     0,     0,     0,
       0,     0,   356,     0,     0,     0,    49,     0,     0,     0,
       0,   363,     0,    50,    51,    52,    53,    54,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  -167,    34,    55,
      56,     0,     0,     0,     0,    73,     0,     0,    58,  -173,
     120,    35,     0,     0,    36,    37,     0,     0,    38,    39,
      40,    41,    42,    43,  -167,  -167,     0,    44,    45,    46,
      47,    48,     0,  -167,  -167,  -167,  -167,  -167,  -167,  -167,
       0,     0,     0,     0,  -167,     0,     0,  -167,  -167,  -167,
       0,     0,  -167,     0,     0,     0,  -167,  -167,  -167,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,    50,    51,    52,    53,    54,     0,     0,  -167,
    -167,     0,  -167,  -167,  -167,  -167,    34,     0,    55,    56,
       0,     0,     0,  -167,    57,     0,  -167,    58,     0,    35,
       0,     0,    36,    37,     0,     0,    38,    39,    40,    41,
      42,    43,  -167,  -167,     0,    44,    45,    46,    47,    48,
       0,  -167,  -167,  -167,  -167,  -167,  -167,  -167,     0,     0,
       0,     0,  -167,     0,     0,  -167,  -167,  -167,     0,     0,
    -167,     0,     0,     0,  -167,  -167,  -167,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
      50,    51,    52,    53,    54,     0,     0,  -167,  -167,     0,
    -167,  -167,  -167,    34,     0,     0,    55,    56,     0,     0,
       0,  -167,    73,     0,  -167,    58,    35,     0,     0,    36,
      37,     0,     0,    38,    39,    40,    41,    42,    43,     0,
       0,     0,    44,    45,    46,    47,    48,     0,     0,     0,
       0,     0,    34,     0,     0,     0,     0,     0,    69,    70,
      71,    72,     0,     0,     0,    35,     0,     0,    36,    37,
       0,     0,    38,    39,    40,    41,    42,    43,     0,     0,
      49,    44,    45,    46,    47,    48,     0,    50,    51,    52,
      53,    54,     0,     0,     0,     0,     0,    69,     0,    71,
       0,     0,     0,    55,    56,     0,     0,     0,     0,    73,
       0,     0,    58,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,    50,    51,    52,    53,
      54,     0,     0,    34,     0,     0,     0,     0,     0,     0,
       0,     0,    55,    56,     0,     0,    35,     0,    73,    36,
      37,    58,     0,    38,    39,    40,   114,    42,    43,    34,
       0,     0,    44,    45,    46,    47,    48,     0,     0,     0,
       0,     0,    35,     0,     0,    36,    37,     0,     0,    38,
      39,    40,   182,    42,    43,     0,     0,     0,    44,    45,
      46,    47,    48,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,    50,    51,    52,
      53,    54,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    55,    56,     0,    49,     0,     0,    73,
    -167,  -167,    58,    50,    51,    52,    53,    54,     0,     0,
       0,     0,     0,     0,     0,     0,    34,     0,     0,    55,
      56,     0,     0,     0,     0,    73,  -167,  -167,    58,    35,
       0,     0,    36,    37,     0,    78,    38,    39,    40,    41,
      42,    43,    34,     0,     0,    44,    45,    46,    47,    48,
       0,     0,     0,     0,     0,    35,     0,     0,    36,    37,
       0,     0,    38,    39,    40,    41,    42,    43,     0,     0,
       0,    44,    45,    46,    47,    48,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
      50,    51,    52,    53,    54,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    55,    56,     0,    49,
       0,     0,    73,     0,     0,    58,    50,    51,    52,    53,
      54,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,    55,    56,   331,     2,     0,     0,    73,     0,
       0,    58,     0,     0,     0,     4,     5,     6,     7,     8,
       0,     0,     0,     0,    10,     0,     1,    11,    12,    13,
       2,     0,    14,     0,     0,     0,    16,    17,    18,     0,
       4,     5,     6,     7,     8,     0,     0,     0,     0,    10,
       0,     0,    11,    12,    13,     0,     0,    14,     0,    19,
      20,    16,    17,    18,     1,     0,     0,     0,     2,     0,
       0,     0,     0,   309,     0,     0,     0,     0,     4,     5,
       6,     7,     8,     0,    19,    20,     0,    10,   370,     0,
      11,    12,    13,     1,     0,    14,     0,     2,   309,    16,
      17,    18,     0,     0,     0,     0,     0,     4,     5,     6,
       7,     8,     0,     0,     0,     0,    10,     0,     0,    11,
      12,    13,    19,    20,    14,     0,   376,     0,    16,    17,
      18,     1,     0,     0,     0,     2,   309,     0,     0,     0,
       0,     0,     0,     0,     0,     4,     5,     6,     7,     8,
       0,    19,    20,     0,    10,   378,     0,    11,    12,    13,
       0,     0,    14,     0,     0,   309,    16,    17,    18,     0,
       1,     0,     0,     0,     2,     3,     0,     0,     0,     0,
       0,     0,   -33,     0,     4,     5,     6,     7,     8,    19,
      20,     9,     0,    10,     0,     0,    11,    12,    13,     0,
       0,    14,     0,   309,    15,    16,    17,    18,     0,   126,
     127,   128,   129,   130,   131,     0,   132,     0,     0,   133,
     134,   135,     0,     0,   154,     0,     0,     0,    19,    20,
      21,     0,     0,     0,     0,   136,   137,   138,   139,   140,
     141,   142,   126,   127,   128,   129,   130,   131,     0,   132,
     155,   156,   133,   134,   135,     0,     0,     0,     0,    69,
       0,    71,     0,     0,     0,     0,     0,     0,   136,   137,
     138,   139,   140,   141,   142,   126,   127,   128,   129,   130,
     131,     0,   132,     0,     0,   133,   134,   135,     0,     0,
       0,     0,     0,     0,   184,     0,     0,     0,     0,     0,
     185,   136,   137,   138,   139,   140,   141,   142,   126,   127,
     128,   129,   130,   131,     0,   132,     0,     0,   133,   134,
     135,     0,     0,     0,     0,     0,     0,   186,     0,     0,
       0,     0,     0,   187,   136,   137,   138,   139,   140,   141,
     142,   126,   127,   128,   129,   130,   131,     0,   132,     0,
       0,   133,   134,   135,     0,     0,     0,     0,     0,     0,
     188,     0,     0,     0,     0,     0,   189,   136,   137,   138,
     139,   140,   141,   142,   126,   127,   128,   129,   130,   131,
       0,   132,     0,     0,   133,   134,   135,     0,     0,     0,
       0,     0,     0,   190,     0,     0,     0,     0,     0,   191,
     136,   137,   138,   139,   140,   141,   142,   126,   127,   128,
     129,   130,   131,     0,   132,     0,     0,   133,   134,   135,
       0,     0,     0,     0,     0,     0,   294,     0,     0,     0,
       0,     0,     0,   136,   137,   138,   139,   140,   141,   142,
     126,   127,   128,   129,   130,   131,     0,   132,     0,     0,
     133,   134,   135,     0,     0,     0,     0,     0,     0,   295,
       0,     0,     0,     0,     0,     0,   136,   137,   138,   139,
     140,   141,   142,   126,   127,   128,   129,   130,   131,     0,
     132,     0,     0,   133,   134,   135,     0,     0,     0,     0,
       0,     0,   296,     0,     0,     0,     0,     0,     0,   136,
     137,   138,   139,   140,   141,   142,   126,   127,   128,   129,
     130,   131,     0,   132,     0,     0,   133,   134,   135,     0,
       0,     0,     0,     0,     0,   297,     0,     0,     0,     0,
       0,     0,   136,   137,   138,   139,   140,   141,   142,   126,
     127,   128,   129,   130,   131,     0,   132,     0,     0,   133,
     134,   135,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   172,   136,   137,   138,   139,   140,
     141,   142,   126,   127,   128,   129,   130,   131,     0,   132,
       0,     0,   133,   134,   135,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   136,   137,
     138,   139,   140,   141,   142,   168,   173,     0,     0,     0,
       0,   126,   127,   128,   129,   130,   131,     0,   132,     0,
       0,   133,   134,   135,   126,   127,   128,   129,   130,   131,
       0,   132,     0,     0,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,     0,     0,     0,     0,     0,     0,
     136,   137,   138,   139,   140,   141,   142,     0,     0,     0,
     196,   126,   127,   128,   129,   130,   131,     0,   132,     0,
       0,   133,   134,   135,     0,     0,     0,     0,     0,   192,
     193,     0,     0,     0,     0,     0,     0,   136,   137,   138,
     139,   140,   141,   142,   126,   127,   128,   129,   130,   131,
       0,   132,     0,     0,   133,   134,   135,   126,   127,   128,
     129,     0,     0,     0,   132,     0,     0,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,     0,     0,     0,
       0,     0,     0,   136,   137,   138,   139,   140,   141,   142
};

static const yytype_int16 yycheck[] =
{
       6,    66,     0,   145,     1,    11,    57,    17,    54,   276,
      16,    17,   326,    19,    65,    69,    81,    14,    64,   355,
      17,    18,    28,    37,    21,    24,    40,    24,    25,    26,
      51,   367,    54,    32,   301,    43,   372,    24,   235,    93,
      94,     0,    64,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    24,   105,   369,    64,    24,    24,    98,
      99,    32,    28,    69,    74,    32,    51,    73,    24,    83,
      84,    54,    38,    39,    40,    41,    75,    76,    77,    78,
      90,    64,    54,   225,    50,    96,    52,   152,    99,   103,
      69,    96,    64,    59,    60,    61,    97,    97,    48,    97,
      97,   251,   102,   100,    75,    76,    77,    78,    75,    76,
      77,    78,    91,    92,    93,    94,    96,    83,    96,    99,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   335,   153,
      24,   291,    99,   149,    96,    69,    24,    99,    72,    46,
     300,    24,   158,   159,   160,   352,   306,   163,   164,   165,
     166,   167,   168,    54,    24,   362,    90,    91,    92,    93,
      94,   368,   322,    64,    97,    98,    99,    53,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,    17,    18,
      19,   341,   198,   199,   200,    24,   202,    97,    98,    99,
      24,    30,    24,    25,    26,   154,   155,   156,    56,    57,
      24,   361,    98,    99,    24,   365,    98,    99,    98,    99,
      24,   371,   287,   288,   101,   375,    53,   233,   234,    36,
      37,   102,   238,   239,   102,    99,    99,    99,   303,    48,
      99,    99,    24,    44,    24,    24,    24,    24,   102,    87,
      24,    54,    35,    86,    85,    99,    54,    24,    53,    85,
      97,   267,   248,   173,   305,   330,   293,   203,    -1,    -1,
      -1,   285,    -1,   279,    -1,    -1,    -1,    -1,    -1,     1,
      -1,    -1,    -1,    -1,    -1,    -1,   351,   293,   294,   295,
     296,   297,    14,    -1,    -1,    17,    18,    -1,    -1,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    30,    31,
      32,    33,    34,    -1,   320,    -1,    -1,    -1,    -1,    -1,
      -1,   327,   328,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   342,    -1,    -1,    -1,
      -1,    -1,   348,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,   357,    -1,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     0,     1,    91,
      92,    -1,    -1,    -1,    -1,    97,    -1,    -1,   100,   101,
     102,    14,    -1,    -1,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    31,    32,
      33,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,
      -1,    -1,    55,    -1,    -1,    -1,    59,    60,    61,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    -1,    85,    86,    87,     0,     1,    -1,    91,    92,
      -1,    -1,    -1,    96,    97,    -1,    99,   100,    -1,    14,
      -1,    -1,    17,    18,    -1,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    42,    -1,    -1,
      -1,    -1,    47,    -1,    -1,    50,    51,    52,    -1,    -1,
      55,    -1,    -1,    -1,    59,    60,    61,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    -1,
      85,    86,    87,     1,    -1,    -1,    91,    92,    -1,    -1,
      -1,    96,    97,    -1,    99,   100,    14,    -1,    -1,    17,
      18,    -1,    -1,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    30,    31,    32,    33,    34,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    -1,    -1,    -1,    14,    -1,    -1,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    -1,    -1,
      68,    30,    31,    32,    33,    34,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    46,    -1,    48,
      -1,    -1,    -1,    91,    92,    -1,    -1,    -1,    -1,    97,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    -1,    -1,    14,    -1,    97,    17,
      18,   100,    -1,    21,    22,    23,    24,    25,    26,     1,
      -1,    -1,    30,    31,    32,    33,    34,    -1,    -1,    -1,
      -1,    -1,    14,    -1,    -1,    17,    18,    -1,    -1,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    30,    31,
      32,    33,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    -1,    68,    -1,    -1,    97,
      98,    99,   100,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    91,
      92,    -1,    -1,    -1,    -1,    97,    98,    99,   100,    14,
      -1,    -1,    17,    18,    -1,    20,    21,    22,    23,    24,
      25,    26,     1,    -1,    -1,    30,    31,    32,    33,    34,
      -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    17,    18,
      -1,    -1,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    30,    31,    32,    33,    34,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    -1,    68,
      -1,    -1,    97,    -1,    -1,   100,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    91,    92,    27,    28,    -1,    -1,    97,    -1,
      -1,   100,    -1,    -1,    -1,    38,    39,    40,    41,    42,
      -1,    -1,    -1,    -1,    47,    -1,    24,    50,    51,    52,
      28,    -1,    55,    -1,    -1,    -1,    59,    60,    61,    -1,
      38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    50,    51,    52,    -1,    -1,    55,    -1,    82,
      83,    59,    60,    61,    24,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    96,    -1,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    -1,    82,    83,    -1,    47,    86,    -1,
      50,    51,    52,    24,    -1,    55,    -1,    28,    96,    59,
      60,    61,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    82,    83,    55,    -1,    86,    -1,    59,    60,
      61,    24,    -1,    -1,    -1,    28,    96,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    39,    40,    41,    42,
      -1,    82,    83,    -1,    47,    86,    -1,    50,    51,    52,
      -1,    -1,    55,    -1,    -1,    96,    59,    60,    61,    -1,
      24,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    -1,    38,    39,    40,    41,    42,    82,
      83,    45,    -1,    47,    -1,    -1,    50,    51,    52,    -1,
      -1,    55,    -1,    96,    58,    59,    60,    61,    -1,    62,
      63,    64,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    -1,    -1,    44,    -1,    -1,    -1,    82,    83,
      84,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    46,
      -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    62,    63,    64,    65,    66,
      67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,
      54,    88,    89,    90,    91,    92,    93,    94,    62,    63,
      64,    65,    66,    67,    -1,    69,    -1,    -1,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    54,    88,    89,    90,    91,    92,    93,
      94,    62,    63,    64,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    -1,    -1,    -1,    -1,    54,    88,    89,    90,
      91,    92,    93,    94,    62,    63,    64,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    54,
      88,    89,    90,    91,    92,    93,    94,    62,    63,    64,
      65,    66,    67,    -1,    69,    -1,    -1,    72,    73,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      62,    63,    64,    65,    66,    67,    -1,    69,    -1,    -1,
      72,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    62,    63,    64,    65,    66,    67,    -1,
      69,    -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    62,    63,    64,    65,
      66,    67,    -1,    69,    -1,    -1,    72,    73,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    62,
      63,    64,    65,    66,    67,    -1,    69,    -1,    -1,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    88,    89,    90,    91,    92,
      93,    94,    62,    63,    64,    65,    66,    67,    -1,    69,
      -1,    -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    56,    96,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    62,    63,    64,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    88,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    -1,    -1,    -1,
      98,    62,    63,    64,    65,    66,    67,    -1,    69,    -1,
      -1,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    62,    63,    64,    65,    66,    67,
      -1,    69,    -1,    -1,    72,    73,    74,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    -1,    -1,    72,    73,    74,
      88,    89,    90,    91,    92,    93,    94,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    24,    28,    29,    38,    39,    40,    41,    42,    45,
      47,    50,    51,    52,    55,    58,    59,    60,    61,    82,
      83,    84,   104,   105,   106,   107,   108,   120,   126,   134,
     142,   144,   146,   149,     1,    14,    17,    18,    21,    22,
      23,    24,    25,    26,    30,    31,    32,    33,    34,    68,
      75,    76,    77,    78,    79,    91,    92,    97,   100,   129,
     130,   132,   133,   154,   156,   145,   148,    51,   135,    46,
      47,    48,    49,    97,   130,   143,   147,    51,    20,   130,
      56,    57,    17,    18,    19,    24,    30,    24,    24,   130,
     130,   143,   137,   130,   136,   150,     0,    96,   130,   127,
      96,    24,   129,    48,   129,    97,   130,   130,   130,   130,
     130,   130,   130,   130,    24,   130,   154,    24,    25,    26,
     102,   130,   157,   158,   159,   160,    62,    63,    64,    65,
      66,    67,    69,    72,    73,    74,    88,    89,    90,    91,
      92,    93,    94,    99,   154,   127,    24,   139,   130,    46,
     130,   143,    24,    24,    44,    70,    71,    24,   127,    54,
      64,   129,   129,    54,    64,    54,    64,    53,    56,   143,
      24,   141,    54,    96,    24,   140,    24,   104,   123,    24,
     151,   129,    24,   154,    48,    54,    48,    54,    48,    54,
      48,    54,    80,    81,    98,    99,    98,    98,   102,   102,
     102,   101,    99,    99,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   151,    99,   130,   127,   129,    24,   132,    24,
     132,    24,   132,    43,    64,   130,   130,   130,    54,    64,
      48,   130,   130,   130,   130,   130,   130,    99,   119,   119,
      99,    53,    96,    99,    98,    99,    98,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   138,   130,   130,
     130,   130,    24,    25,    26,   160,    96,    24,   151,    44,
     130,   130,   123,   130,   130,    24,    24,   107,   128,    24,
     128,   153,    24,   131,    48,    48,    48,    48,   130,   155,
     153,    96,   130,   115,   109,   112,   118,   129,   127,    96,
     108,   127,   124,   128,   155,   130,   130,   130,   130,    98,
      99,   128,   153,   127,   125,   125,   128,    54,    64,    87,
     121,    27,    98,   130,   128,   116,   110,   113,   124,   130,
     130,    36,    37,   122,   127,    24,   152,   123,    54,    35,
      86,   128,   130,    85,    99,   117,   130,    54,   127,   123,
      24,   118,   111,   130,    85,    53,   128,   123,   114,   128,
      86,   118,   123,   124,   128,   118,    86,   128,    86
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   103,   104,   104,   105,   105,   105,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   107,   107,   108,   108,   108,   109,
     110,   111,   108,   112,   113,   114,   108,   115,   116,   117,
     108,   108,   108,   108,   108,   118,   119,   120,   120,   121,
     121,   122,   123,   124,   125,   126,   127,   128,   128,   128,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     130,   130,   130,   131,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     133,   133,   133,   133,   134,   134,   134,   134,   134,   135,
     134,   136,   134,   137,   134,   134,   134,   134,   138,   134,
     134,   134,   139,   139,   140,   140,   141,   141,   142,   142,
     142,   142,   142,   142,   143,   143,   144,   144,   144,   145,
     144,   147,   146,   146,   148,   146,   146,   146,   150,   149,
     151,   151,   151,   152,   152,   152,   153,   154,   154,   154,
     155,   155,   156,   157,   157,   157,   157,   158,   158,   159,
     159,   160,   160,   160
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     4,     4,     6,
       4,     4,     4,     4,     4,     4,     4,     4,     5,     5,
       8,     8,     4,     4,     1,     1,     1,     1,     9,     0,
       0,     0,    15,     0,     0,     0,    16,     0,     0,     0,
      13,     2,     4,     7,     6,     0,     0,     9,    11,     0,
       2,     6,     0,     0,     0,     1,     0,     0,     2,     2,
       1,     1,     1,     1,     1,     1,     2,     3,     1,     2,
       1,     1,     4,     0,     7,     4,     2,     3,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     3,     3,     3,     3,     2,     2,     4,
       4,     1,     4,     6,     4,     6,     4,     6,     4,     6,
       1,     1,     1,     1,     2,     1,     1,     2,     1,     0,
       3,     0,     3,     0,     3,     4,     2,     4,     0,     7,
       4,     2,     1,     3,     1,     3,     1,     3,     1,     2,
       2,     2,     3,     2,     3,     2,     2,     3,     2,     0,
       3,     0,     8,     2,     0,     7,     8,     6,     0,     3,
       0,     1,     3,     0,     1,     3,     0,     0,     1,     3,
       1,     3,     3,     0,     1,     1,     1,     1,     3,     1,
       3,     3,     3,     3
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
#line 214 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1792 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* THEFUNCINOF  */
#line 214 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1798 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* ID  */
#line 214 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1804 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* STRING  */
#line 214 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1810 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* SYMBOL  */
#line 214 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1816 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* ENDCLAUSE  */
#line 214 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1822 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* tPLAYACCEL  */
#line 214 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1828 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* tMETHOD  */
#line 214 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1834 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 149: /* on  */
#line 214 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1840 "engines/director/lingo/lingo-gr.cpp"
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
  case 7:
#line 225 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2127 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 232 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2135 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 236 "engines/director/lingo/lingo-gr.y"
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
#line 2151 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 247 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_after);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2163 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 254 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_after);
		(yyval.code) = (yyvsp[-2].code); }
#line 2171 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 257 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_before);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2183 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 264 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_before);
		(yyval.code) = (yyvsp[-2].code); }
#line 2191 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 267 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2203 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 274 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2215 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 281 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2227 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 288 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2239 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 295 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2250 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 301 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2261 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20:
#line 308 "engines/director/lingo/lingo-gr.y"
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
#line 2277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21:
#line 319 "engines/director/lingo/lingo-gr.y"
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
#line 2293 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22:
#line 330 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2305 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23:
#line 337 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2317 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 28:
#line 354 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-6].code) - (yyvsp[-1].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-4].code) + 2);
		(*g_lingo->_currentAssembly)[(yyvsp[-4].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentAssembly)[(yyvsp[-1].code)] = start;	/* looping back */
		endRepeat((yyvsp[-1].code) + 1, (yyvsp[-6].code));	}
#line 2329 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29:
#line 367 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2337 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 371 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2344 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 374 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_le); }
#line 2350 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 374 "engines/director/lingo/lingo-gr.y"
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
#line 2375 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 401 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2383 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 405 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2390 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 408 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_ge); }
#line 2396 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 409 "engines/director/lingo/lingo-gr.y"
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
#line 2421 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 435 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  Common::String count("count");
				  g_lingo->codeFunc(&count, 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
#line 2432 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 442 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
#line 2442 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 448 "engines/director/lingo/lingo-gr.y"
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
#line 2457 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 458 "engines/director/lingo/lingo-gr.y"
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
#line 2481 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 478 "engines/director/lingo/lingo-gr.y"
                        {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->nexts.push_back(pos);
		} else {
			warning("# LINGO: next repeat not inside repeat block");
		} }
#line 2494 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 486 "engines/director/lingo/lingo-gr.y"
                              {
		g_lingo->code1(LC::c_whencode);
		g_lingo->codeString((yyvsp[-2].s)->c_str()); }
#line 2502 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 489 "engines/director/lingo/lingo-gr.y"
                                                          { g_lingo->code1(LC::c_telldone); }
#line 2508 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 490 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->code1(LC::c_telldone); }
#line 2514 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 492 "engines/director/lingo/lingo-gr.y"
                                { startRepeat(); }
#line 2520 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 494 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_tell); }
#line 2526 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 496 "engines/director/lingo/lingo-gr.y"
                                                                                         {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2538 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 503 "engines/director/lingo/lingo-gr.y"
                                                                                                          {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].code) + 1 - (yyvsp[-8].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-8].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-5].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-5].code), (yyvsp[-1].code)); }
#line 2550 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51:
#line 514 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2560 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52:
#line 520 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2568 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 524 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 528 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2584 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 532 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2591 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 535 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2597 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 537 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2603 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60:
#line 541 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61:
#line 544 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2619 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 547 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2628 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 551 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 555 "engines/director/lingo/lingo-gr.y"
                        {
		if (g_lingo->_builtinConsts.contains(*(yyvsp[0].s))) {
			(yyval.code) = g_lingo->code1(LC::c_constpush);
		} else {
			(yyval.code) = g_lingo->code1(LC::c_eval);
		}
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2650 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 563 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2663 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 571 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2674 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 577 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2680 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 579 "engines/director/lingo/lingo-gr.y"
                                { yyerrok; }
#line 2686 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70:
#line 581 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2692 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 583 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_lazyeval);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeFunc((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 2703 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 589 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_lazyeval); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 2709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 590 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeFunc((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 2718 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 594 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2726 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 597 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 2734 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 600 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2742 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 603 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_objectproppush);
		g_lingo->codeString((yyvsp[0].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectprop).prop->c_str());
		delete (yyvsp[0].objectprop).obj;
		delete (yyvsp[0].objectprop).prop; }
#line 2753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 609 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 610 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2765 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81:
#line 611 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2771 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82:
#line 612 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2777 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 613 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 614 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2789 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 615 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2795 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 616 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2801 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 617 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2807 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 618 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 619 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2819 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 620 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2825 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 621 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2831 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 622 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 623 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2843 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 624 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2849 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 625 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2855 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 626 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 627 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); }
#line 2867 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 628 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2873 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 629 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_intersects); }
#line 2879 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 630 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 2885 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 632 "engines/director/lingo/lingo-gr.y"
                                                {
		if ((yyvsp[0].ref).s) {
			(yyval.code) = g_lingo->code1(LC::c_stringpush);
			g_lingo->codeString((yyvsp[0].ref).s->c_str());
			delete (yyvsp[0].ref).s;
		} else {
			(yyval.code) = g_lingo->code1(LC::c_intpush);
			g_lingo->codeInt((yyvsp[0].ref).i);
		}
		Common::String field("field");
		g_lingo->codeFunc(&field, 1); }
#line 2901 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 643 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charOf); }
#line 2907 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 644 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charToOf); }
#line 2913 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 645 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemOf); }
#line 2919 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 646 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_itemToOf); }
#line 2925 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 647 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineOf); }
#line 2931 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 648 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_lineToOf); }
#line 2937 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 649 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordOf); }
#line 2943 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 650 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_wordToOf); }
#line 2949 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 653 "engines/director/lingo/lingo-gr.y"
                                        {
		if ((yyvsp[0].ref).s) {
			(yyval.code) = g_lingo->code1(LC::c_stringpush);
			g_lingo->codeString((yyvsp[0].ref).s->c_str());
			delete (yyvsp[0].ref).s;
		} else {
			(yyval.code) = g_lingo->code1(LC::c_intpush);
			g_lingo->codeInt((yyvsp[0].ref).i);
		}
		Common::String cast("cast");
		g_lingo->codeFunc(&cast, 1); }
#line 2965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 664 "engines/director/lingo/lingo-gr.y"
                                        {
		if ((yyvsp[0].ref).s) {
			(yyval.code) = g_lingo->code1(LC::c_stringpush);
			g_lingo->codeString((yyvsp[0].ref).s->c_str());
			delete (yyvsp[0].ref).s;
		} else {
			(yyval.code) = g_lingo->code1(LC::c_intpush);
			g_lingo->codeInt((yyvsp[0].ref).i);
		}
		Common::String script("script");
		g_lingo->codeFunc(&script, 1); }
#line 2981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 675 "engines/director/lingo/lingo-gr.y"
                                        {
		if ((yyvsp[0].ref).s) {
			(yyval.code) = g_lingo->code1(LC::c_stringpush);
			g_lingo->codeString((yyvsp[0].ref).s->c_str());
			delete (yyvsp[0].ref).s;
		} else {
			(yyval.code) = g_lingo->code1(LC::c_intpush);
			g_lingo->codeInt((yyvsp[0].ref).i);
		}
		Common::String window("window");
		g_lingo->codeFunc(&window, 1); }
#line 2997 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 688 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_printtop); }
#line 3003 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 691 "engines/director/lingo/lingo-gr.y"
                                                {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->exits.push_back(pos);
		} else {
			warning("# LINGO: exit repeat not inside repeat block");
		} }
#line 3016 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118:
#line 699 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 3022 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119:
#line 700 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3028 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 700 "engines/director/lingo/lingo-gr.y"
                                                                                 { inLast(); }
#line 3034 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 701 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3040 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 701 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 3046 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 702 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3052 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 702 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 3058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 703 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String open("open");
		g_lingo->codeFunc(&open, 2); }
#line 3066 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 706 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String open("open");
		g_lingo->codeFunc(&open, 1); }
#line 3074 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 709 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_lazyeval);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeCmd((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 3085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 715 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_lazyeval); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 3091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 716 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeCmd((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 3100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 720 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeCmd((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3108 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 723 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeCmd((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3116 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 727 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 730 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3132 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 734 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3140 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 737 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 741 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3156 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 744 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3164 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 755 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3170 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 756 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3176 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 757 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3182 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 758 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3191 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 762 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3200 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 766 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3209 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146:
#line 774 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3218 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 778 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3227 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 782 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3236 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 786 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3242 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 786 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3251 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 816 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3257 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 817 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3267 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 822 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3273 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 823 "engines/director/lingo/lingo-gr.y"
                  { startDef(); (*g_lingo->_methodVars)["me"] = kVarArgument; }
#line 3279 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 824 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1);
		endDef();
		delete (yyvsp[-6].s); }
#line 3289 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 829 "engines/director/lingo/lingo-gr.y"
                                                                   {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3302 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 837 "engines/director/lingo/lingo-gr.y"
                                               {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3312 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 843 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3318 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 843 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = (yyvsp[0].s); }
#line 3324 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 845 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3330 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 846 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3336 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 847 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3342 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164:
#line 850 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3348 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 851 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3354 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 853 "engines/director/lingo/lingo-gr.y"
                                        { inDef(); }
#line 3360 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 855 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 856 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 857 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3378 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 859 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3384 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 860 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3390 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172:
#line 862 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3396 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173:
#line 864 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3402 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174:
#line 865 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3408 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175:
#line 866 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3414 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176:
#line 867 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3420 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177:
#line 869 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3426 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178:
#line 870 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3432 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179:
#line 872 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3438 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180:
#line 873 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3444 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181:
#line 875 "engines/director/lingo/lingo-gr.y"
                          {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3453 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182:
#line 879 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3462 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183:
#line 883 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3471 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3475 "engines/director/lingo/lingo-gr.cpp"

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

#line 889 "engines/director/lingo/lingo-gr.y"


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
