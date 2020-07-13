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
extern void lex_unput(int c);
extern bool lex_check_parens();

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


#line 179 "engines/director/lingo/lingo-gr.cpp"

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
    BLTIN = 277,                   /* BLTIN  */
    FBLTIN = 278,                  /* FBLTIN  */
    RBLTIN = 279,                  /* RBLTIN  */
    THEFBLTIN = 280,               /* THEFBLTIN  */
    ID = 281,                      /* ID  */
    STRING = 282,                  /* STRING  */
    HANDLER = 283,                 /* HANDLER  */
    SYMBOL = 284,                  /* SYMBOL  */
    ENDCLAUSE = 285,               /* ENDCLAUSE  */
    tPLAYACCEL = 286,              /* tPLAYACCEL  */
    tMETHOD = 287,                 /* tMETHOD  */
    THEOBJECTPROP = 288,           /* THEOBJECTPROP  */
    tDOWN = 289,                   /* tDOWN  */
    tELSE = 290,                   /* tELSE  */
    tELSIF = 291,                  /* tELSIF  */
    tEXIT = 292,                   /* tEXIT  */
    tGLOBAL = 293,                 /* tGLOBAL  */
    tGO = 294,                     /* tGO  */
    tGOLOOP = 295,                 /* tGOLOOP  */
    tIF = 296,                     /* tIF  */
    tIN = 297,                     /* tIN  */
    tINTO = 298,                   /* tINTO  */
    tMACRO = 299,                  /* tMACRO  */
    tMOVIE = 300,                  /* tMOVIE  */
    tNEXT = 301,                   /* tNEXT  */
    tOF = 302,                     /* tOF  */
    tPREVIOUS = 303,               /* tPREVIOUS  */
    tPUT = 304,                    /* tPUT  */
    tREPEAT = 305,                 /* tREPEAT  */
    tSET = 306,                    /* tSET  */
    tTHEN = 307,                   /* tTHEN  */
    tTO = 308,                     /* tTO  */
    tWHEN = 309,                   /* tWHEN  */
    tWITH = 310,                   /* tWITH  */
    tWHILE = 311,                  /* tWHILE  */
    tFACTORY = 312,                /* tFACTORY  */
    tOPEN = 313,                   /* tOPEN  */
    tPLAY = 314,                   /* tPLAY  */
    tINSTANCE = 315,               /* tINSTANCE  */
    tGE = 316,                     /* tGE  */
    tLE = 317,                     /* tLE  */
    tEQ = 318,                     /* tEQ  */
    tNEQ = 319,                    /* tNEQ  */
    tAND = 320,                    /* tAND  */
    tOR = 321,                     /* tOR  */
    tNOT = 322,                    /* tNOT  */
    tMOD = 323,                    /* tMOD  */
    tAFTER = 324,                  /* tAFTER  */
    tBEFORE = 325,                 /* tBEFORE  */
    tCONCAT = 326,                 /* tCONCAT  */
    tCONTAINS = 327,               /* tCONTAINS  */
    tSTARTS = 328,                 /* tSTARTS  */
    tCHAR = 329,                   /* tCHAR  */
    tITEM = 330,                   /* tITEM  */
    tLINE = 331,                   /* tLINE  */
    tWORD = 332,                   /* tWORD  */
    tSPRITE = 333,                 /* tSPRITE  */
    tINTERSECTS = 334,             /* tINTERSECTS  */
    tWITHIN = 335,                 /* tWITHIN  */
    tTELL = 336,                   /* tTELL  */
    tPROPERTY = 337,               /* tPROPERTY  */
    tON = 338,                     /* tON  */
    tENDIF = 339,                  /* tENDIF  */
    tENDREPEAT = 340,              /* tENDREPEAT  */
    tENDTELL = 341                 /* tENDTELL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 162 "engines/director/lingo/lingo-gr.y"

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

#line 330 "engines/director/lingo/lingo-gr.cpp"

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
  YYSYMBOL_BLTIN = 22,                     /* BLTIN  */
  YYSYMBOL_FBLTIN = 23,                    /* FBLTIN  */
  YYSYMBOL_RBLTIN = 24,                    /* RBLTIN  */
  YYSYMBOL_THEFBLTIN = 25,                 /* THEFBLTIN  */
  YYSYMBOL_ID = 26,                        /* ID  */
  YYSYMBOL_STRING = 27,                    /* STRING  */
  YYSYMBOL_HANDLER = 28,                   /* HANDLER  */
  YYSYMBOL_SYMBOL = 29,                    /* SYMBOL  */
  YYSYMBOL_ENDCLAUSE = 30,                 /* ENDCLAUSE  */
  YYSYMBOL_tPLAYACCEL = 31,                /* tPLAYACCEL  */
  YYSYMBOL_tMETHOD = 32,                   /* tMETHOD  */
  YYSYMBOL_THEOBJECTPROP = 33,             /* THEOBJECTPROP  */
  YYSYMBOL_tDOWN = 34,                     /* tDOWN  */
  YYSYMBOL_tELSE = 35,                     /* tELSE  */
  YYSYMBOL_tELSIF = 36,                    /* tELSIF  */
  YYSYMBOL_tEXIT = 37,                     /* tEXIT  */
  YYSYMBOL_tGLOBAL = 38,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 39,                       /* tGO  */
  YYSYMBOL_tGOLOOP = 40,                   /* tGOLOOP  */
  YYSYMBOL_tIF = 41,                       /* tIF  */
  YYSYMBOL_tIN = 42,                       /* tIN  */
  YYSYMBOL_tINTO = 43,                     /* tINTO  */
  YYSYMBOL_tMACRO = 44,                    /* tMACRO  */
  YYSYMBOL_tMOVIE = 45,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 46,                     /* tNEXT  */
  YYSYMBOL_tOF = 47,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 48,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 49,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 50,                   /* tREPEAT  */
  YYSYMBOL_tSET = 51,                      /* tSET  */
  YYSYMBOL_tTHEN = 52,                     /* tTHEN  */
  YYSYMBOL_tTO = 53,                       /* tTO  */
  YYSYMBOL_tWHEN = 54,                     /* tWHEN  */
  YYSYMBOL_tWITH = 55,                     /* tWITH  */
  YYSYMBOL_tWHILE = 56,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 57,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 58,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 59,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 60,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 61,                       /* tGE  */
  YYSYMBOL_tLE = 62,                       /* tLE  */
  YYSYMBOL_tEQ = 63,                       /* tEQ  */
  YYSYMBOL_tNEQ = 64,                      /* tNEQ  */
  YYSYMBOL_tAND = 65,                      /* tAND  */
  YYSYMBOL_tOR = 66,                       /* tOR  */
  YYSYMBOL_tNOT = 67,                      /* tNOT  */
  YYSYMBOL_tMOD = 68,                      /* tMOD  */
  YYSYMBOL_tAFTER = 69,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 70,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 71,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 72,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 73,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 74,                     /* tCHAR  */
  YYSYMBOL_tITEM = 75,                     /* tITEM  */
  YYSYMBOL_tLINE = 76,                     /* tLINE  */
  YYSYMBOL_tWORD = 77,                     /* tWORD  */
  YYSYMBOL_tSPRITE = 78,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 79,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 80,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 81,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 82,                 /* tPROPERTY  */
  YYSYMBOL_tON = 83,                       /* tON  */
  YYSYMBOL_tENDIF = 84,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 85,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 86,                  /* tENDTELL  */
  YYSYMBOL_87_ = 87,                       /* '<'  */
  YYSYMBOL_88_ = 88,                       /* '>'  */
  YYSYMBOL_89_ = 89,                       /* '&'  */
  YYSYMBOL_90_ = 90,                       /* '+'  */
  YYSYMBOL_91_ = 91,                       /* '-'  */
  YYSYMBOL_92_ = 92,                       /* '*'  */
  YYSYMBOL_93_ = 93,                       /* '/'  */
  YYSYMBOL_94_ = 94,                       /* '%'  */
  YYSYMBOL_95_n_ = 95,                     /* '\n'  */
  YYSYMBOL_96_ = 96,                       /* '('  */
  YYSYMBOL_97_ = 97,                       /* ')'  */
  YYSYMBOL_98_ = 98,                       /* ','  */
  YYSYMBOL_99_ = 99,                       /* '['  */
  YYSYMBOL_100_ = 100,                     /* ']'  */
  YYSYMBOL_101_ = 101,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 102,                 /* $accept  */
  YYSYMBOL_program = 103,                  /* program  */
  YYSYMBOL_programline = 104,              /* programline  */
  YYSYMBOL_asgn = 105,                     /* asgn  */
  YYSYMBOL_stmtoneliner = 106,             /* stmtoneliner  */
  YYSYMBOL_stmt = 107,                     /* stmt  */
  YYSYMBOL_108_1 = 108,                    /* $@1  */
  YYSYMBOL_109_2 = 109,                    /* $@2  */
  YYSYMBOL_110_3 = 110,                    /* $@3  */
  YYSYMBOL_111_4 = 111,                    /* $@4  */
  YYSYMBOL_112_5 = 112,                    /* $@5  */
  YYSYMBOL_113_6 = 113,                    /* $@6  */
  YYSYMBOL_114_7 = 114,                    /* $@7  */
  YYSYMBOL_115_8 = 115,                    /* $@8  */
  YYSYMBOL_116_9 = 116,                    /* $@9  */
  YYSYMBOL_startrepeat = 117,              /* startrepeat  */
  YYSYMBOL_tellstart = 118,                /* tellstart  */
  YYSYMBOL_ifstmt = 119,                   /* ifstmt  */
  YYSYMBOL_elseifstmtlist = 120,           /* elseifstmtlist  */
  YYSYMBOL_elseifstmt = 121,               /* elseifstmt  */
  YYSYMBOL_jumpifz = 122,                  /* jumpifz  */
  YYSYMBOL_jump = 123,                     /* jump  */
  YYSYMBOL_varassign = 124,                /* varassign  */
  YYSYMBOL_if = 125,                       /* if  */
  YYSYMBOL_lbl = 126,                      /* lbl  */
  YYSYMBOL_stmtlist = 127,                 /* stmtlist  */
  YYSYMBOL_simpleexpr = 128,               /* simpleexpr  */
  YYSYMBOL_funccall = 129,                 /* funccall  */
  YYSYMBOL_130_10 = 130,                   /* $@10  */
  YYSYMBOL_expr = 131,                     /* expr  */
  YYSYMBOL_chunkexpr = 132,                /* chunkexpr  */
  YYSYMBOL_reference = 133,                /* reference  */
  YYSYMBOL_proc = 134,                     /* proc  */
  YYSYMBOL_135_11 = 135,                   /* $@11  */
  YYSYMBOL_136_12 = 136,                   /* $@12  */
  YYSYMBOL_137_13 = 137,                   /* $@13  */
  YYSYMBOL_globallist = 138,               /* globallist  */
  YYSYMBOL_propertylist = 139,             /* propertylist  */
  YYSYMBOL_instancelist = 140,             /* instancelist  */
  YYSYMBOL_gotofunc = 141,                 /* gotofunc  */
  YYSYMBOL_gotomovie = 142,                /* gotomovie  */
  YYSYMBOL_playfunc = 143,                 /* playfunc  */
  YYSYMBOL_144_14 = 144,                   /* $@14  */
  YYSYMBOL_defn = 145,                     /* defn  */
  YYSYMBOL_146_15 = 146,                   /* $@15  */
  YYSYMBOL_147_16 = 147,                   /* $@16  */
  YYSYMBOL_on = 148,                       /* on  */
  YYSYMBOL_149_17 = 149,                   /* $@17  */
  YYSYMBOL_argdef = 150,                   /* argdef  */
  YYSYMBOL_endargdef = 151,                /* endargdef  */
  YYSYMBOL_argstore = 152,                 /* argstore  */
  YYSYMBOL_macro = 153,                    /* macro  */
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
#define YYFINAL  103
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2022

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  102
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  59
/* YYNRULES -- Number of rules.  */
#define YYNRULES  185
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  385

#define YYMAXUTOK   341


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
      95,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    94,    89,     2,
      96,    97,    92,    90,    98,    91,     2,    93,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   101,     2,
      87,     2,    88,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    99,     2,   100,     2,     2,     2,     2,     2,     2,
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
      85,    86
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   214,   214,   215,   217,   218,   219,   221,   228,   232,
     243,   250,   253,   260,   263,   270,   277,   284,   291,   297,
     304,   315,   326,   333,   341,   342,   343,   344,   346,   347,
     352,   365,   369,   372,   364,   399,   403,   406,   398,   433,
     440,   446,   432,   476,   484,   487,   488,   490,   492,   494,
     501,   509,   510,   512,   518,   522,   526,   530,   533,   535,
     536,   537,   539,   542,   545,   549,   553,   557,   565,   571,
     572,   573,   584,   587,   590,   593,   596,   602,   602,   607,
     611,   612,   613,   614,   617,   623,   624,   625,   626,   627,
     628,   629,   630,   631,   632,   633,   634,   635,   636,   637,
     638,   639,   640,   641,   642,   643,   644,   646,   647,   648,
     649,   650,   651,   652,   653,   655,   658,   660,   661,   662,
     663,   671,   672,   672,   673,   673,   674,   674,   675,   678,
     681,   684,   687,   691,   694,   698,   701,   705,   708,   719,
     720,   721,   722,   726,   730,   735,   736,   738,   742,   746,
     750,   750,   780,   780,   786,   787,   787,   793,   801,   807,
     807,   809,   810,   811,   813,   814,   815,   817,   819,   827,
     828,   829,   831,   832,   834,   836,   837,   838,   839,   841,
     842,   844,   845,   847,   851,   855
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
  "THEMENUITEMENTITY", "THEMENUITEMSENTITY", "FLOAT", "BLTIN", "FBLTIN",
  "RBLTIN", "THEFBLTIN", "ID", "STRING", "HANDLER", "SYMBOL", "ENDCLAUSE",
  "tPLAYACCEL", "tMETHOD", "THEOBJECTPROP", "tDOWN", "tELSE", "tELSIF",
  "tEXIT", "tGLOBAL", "tGO", "tGOLOOP", "tIF", "tIN", "tINTO", "tMACRO",
  "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT", "tREPEAT", "tSET",
  "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE", "tFACTORY", "tOPEN", "tPLAY",
  "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ", "tAND", "tOR", "tNOT", "tMOD",
  "tAFTER", "tBEFORE", "tCONCAT", "tCONTAINS", "tSTARTS", "tCHAR", "tITEM",
  "tLINE", "tWORD", "tSPRITE", "tINTERSECTS", "tWITHIN", "tTELL",
  "tPROPERTY", "tON", "tENDIF", "tENDREPEAT", "tENDTELL", "'<'", "'>'",
  "'&'", "'+'", "'-'", "'*'", "'/'", "'%'", "'\\n'", "'('", "')'", "','",
  "'['", "']'", "':'", "$accept", "program", "programline", "asgn",
  "stmtoneliner", "stmt", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7",
  "$@8", "$@9", "startrepeat", "tellstart", "ifstmt", "elseifstmtlist",
  "elseifstmt", "jumpifz", "jump", "varassign", "if", "lbl", "stmtlist",
  "simpleexpr", "funccall", "$@10", "expr", "chunkexpr", "reference",
  "proc", "$@11", "$@12", "$@13", "globallist", "propertylist",
  "instancelist", "gotofunc", "gotomovie", "playfunc", "$@14", "defn",
  "$@15", "$@16", "on", "$@17", "argdef", "endargdef", "argstore", "macro",
  "arglist", "nonemptyarglist", "list", "valuelist", "linearlist",
  "proplist", "proppair", YY_NULLPTR
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
     335,   336,   337,   338,   339,   340,   341,    60,    62,    38,
      43,    45,    42,    47,    37,    10,    40,    41,    44,    91,
      93,    58
};
#endif

#define YYPACT_NINF (-349)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-176)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1422,   649,   367,   468,   823,  -349,  -349,   -43,  -349,   909,
    -349,  -349,  -349,   -36,  1137,    37,   133,     9,    23,  1164,
     944,  -349,  1164,  -349,  -349,    18,   -69,  -349,  -349,  -349,
    -349,  1164,  -349,  -349,  -349,  -349,  -349,  -349,  -349,   -35,
    -349,  -349,    10,  -349,    28,     7,  -349,  -349,  -349,  1164,
    1164,  1164,  1164,  1164,  1164,  1164,  1164,  1024,   561,  -349,
    -349,  1916,  -349,    14,  -349,  1024,    14,  1024,    14,  1051,
    1916,    64,   736,  -349,  -349,    73,  1164,  -349,    81,  -349,
    1164,  1497,  -349,    79,  -349,   138,  1463,   164,  -349,   -41,
      10,    10,   -34,   -15,   139,  -349,  1833,  1497,  -349,   166,
    1794,   178,   182,  -349,  1422,  1916,   189,  -349,  -349,  -349,
      10,  1051,  -349,  1530,  1563,  1596,  1629,  1883,  -349,  -349,
    1846,    21,   -25,   118,   119,  -349,  1916,   121,   124,   125,
    -349,  1164,  1164,  1164,  1164,  1164,  1164,  1164,  1164,  1164,
    1164,  1164,  1164,  1164,  1164,  1164,  1164,  1164,  1164,    60,
      70,    84,    75,  1164,    14,   189,  -349,   126,  1916,  1164,
    1846,  -349,  -349,    10,    48,    54,   137,    25,  1164,  1164,
    1164,    26,   179,  1164,  1164,  1164,  1164,  1164,  1164,  -349,
    -349,   127,  -349,  -349,  -349,   131,  -349,  -349,   180,  -349,
     -11,  -349,  1164,  1164,  1164,  1164,  1164,  1164,  1164,  1164,
    1164,  1164,  -349,  -349,  1164,  1164,  1164,  -349,  1164,    87,
     -28,   -28,   -28,   -28,  1929,  1929,  -349,     5,   -28,   -28,
     -28,   -28,     5,   -51,   -51,  -349,  -349,  1916,  -349,  -349,
    -349,  -349,  -349,  1916,    22,   207,  1916,   189,   191,    10,
    -349,  -349,  -349,  -349,  -349,  -349,  -349,  1164,  1164,  1916,
    1916,  1916,  1164,  1164,   209,  1916,  1916,  1916,  1916,  1916,
    1916,   210,  1461,  -349,   211,  -349,  -349,   212,  1916,  1662,
    1916,  1695,  1916,  1728,  1916,  1761,  1916,  1916,  1916,  1916,
    1916,  1916,   141,   118,   119,  -349,  1164,  -349,  -349,    58,
    1164,  -349,  1916,   254,  -349,  1916,  1916,    10,  -349,  -349,
    1383,  -349,  1383,  -349,  -349,  1164,  1164,  1164,  1164,    89,
    -349,  -349,  1916,  -349,  -349,  -349,  -349,    38,  -349,  -349,
    -349,   154,  -349,   573,  1916,  1916,  1916,  1916,  -349,  1383,
    -349,  -349,  -349,  -349,  1383,  1164,  1164,  -349,   153,   213,
    1383,  -349,   190,   216,   159,  1916,  1916,  -349,  1164,  -349,
     161,  -349,   148,  -349,  1164,   198,  -349,  1383,  1916,  -349,
     226,  -349,  1916,  1164,   171,   204,  -349,  -349,  -349,  1916,
    -349,  -349,  1227,  -349,  -349,  1383,  -349,  -349,  -349,  -349,
    1279,  -349,  -349,  1331,  -349
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     0,     0,   150,   155,   121,   122,     0,
     139,    57,   152,     0,     0,     0,     0,     0,     0,     0,
       0,   126,     0,   124,   159,     0,     2,    26,    28,     6,
      29,     0,    25,    27,   118,   119,     5,    58,    24,     0,
      62,    67,     0,    63,     0,    66,    65,    64,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    80,
      82,   170,    81,   129,    70,     0,    73,     0,    75,     0,
     172,   168,     0,    58,   120,     0,     0,   140,     0,   141,
       0,   142,   144,     0,    43,     0,   117,     0,    58,     0,
       0,     0,     0,     0,     0,   154,   131,   147,   149,     0,
       0,     0,     0,     1,     4,    54,   161,    71,    66,    68,
       0,     0,    98,     0,     0,     0,     0,     0,   103,   104,
     170,     0,    66,    65,    64,   176,   179,     0,   178,   177,
     181,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    66,     0,     0,   151,   161,   133,   123,   146,     0,
       0,   143,    58,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   148,
     137,   127,    48,    48,   135,   125,   160,     3,     0,   162,
       0,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    69,   128,     0,     0,     0,   174,     0,     0,
      94,    95,    92,    93,    96,    97,    89,   100,   101,   102,
      91,    90,    99,    85,    86,    87,    88,   171,    72,    74,
      76,    77,    79,   173,     0,     0,   145,   161,     0,     0,
       7,   116,     8,    10,    11,    12,    13,     0,     0,    54,
      17,    15,     0,     0,     0,    16,    14,    22,    23,    44,
     130,     0,     0,    59,     0,    59,   167,     0,   107,     0,
     109,     0,   111,     0,   113,     0,   105,   106,   185,   184,
     183,   180,     0,     0,     0,   182,     0,   167,   134,     0,
       0,   115,    39,    31,    47,    18,    19,     0,   138,    58,
      58,   136,    55,    59,   163,     0,     0,     0,     0,     0,
      59,   167,     9,    58,    56,    56,    59,     0,    46,    60,
      61,     0,    51,   158,   108,   110,   112,   114,    78,   156,
      59,    40,    32,    36,    55,     0,     0,    45,    58,   164,
     153,    54,     0,     0,     0,    20,    21,    59,     0,    52,
       0,   165,   157,    41,     0,     0,    30,    58,    54,    49,
       0,    47,    33,     0,     0,     0,   166,    59,    54,    37,
      50,    59,     0,    47,    54,    55,    42,    59,    47,    53,
       0,    59,    34,     0,    38
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -349,   155,  -349,  -349,    -5,     3,  -349,  -349,  -349,  -349,
    -349,  -349,  -349,  -349,  -349,  -348,    77,  -349,  -349,  -349,
    -247,  -328,   -57,  -349,   -72,  -100,    -8,     0,  -349,     1,
      19,    52,  -349,  -349,  -349,  -349,  -349,  -349,  -349,  -349,
     -12,  -349,  -349,  -349,  -349,  -349,  -349,  -349,  -151,  -349,
    -278,  -349,    43,   -23,  -349,  -349,  -349,  -349,    55
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    25,    26,    27,    28,   320,   314,   342,   368,   315,
     343,   374,   313,   341,   361,   316,   262,    30,   338,   349,
     188,   322,   332,    31,   106,   300,    59,    60,   286,    61,
      62,   242,    33,    75,   101,    99,   157,   185,   181,    34,
      82,    35,    72,    36,    83,    73,    37,   102,   190,   352,
     303,    38,   152,    71,    64,   127,   128,   129,   130
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      32,   155,   294,    29,   234,    70,   344,    74,    98,   310,
      81,    39,   169,   367,    84,    86,   168,   137,   103,   173,
      96,    97,   170,   100,    40,   377,   104,    41,    42,   174,
     381,    43,   105,   330,   109,    94,   108,    46,   175,    47,
     137,   146,   147,   138,    63,    66,    68,   379,   176,    95,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   126,
     107,   143,   144,   145,   146,   147,   120,   247,   120,   161,
     120,   111,   239,   137,   240,   110,   204,   158,   239,   252,
     243,   160,   171,   172,   266,   179,   289,   267,   248,   253,
     237,   335,    87,    88,   353,   144,   145,   146,   147,   156,
     121,   336,   191,   111,    32,   162,    80,    29,   149,    58,
     150,   365,   148,   282,   283,   154,   284,   287,   203,   148,
     267,   373,    50,    51,    52,    53,   159,   378,    50,    51,
      52,    53,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
      89,    90,    91,   311,   233,   238,   267,   228,   148,    92,
     236,   239,   153,   245,   163,   302,    93,   229,   148,   249,
     250,   251,   232,   148,   255,   256,   257,   258,   259,   260,
     111,   230,   231,   241,   241,   241,   328,   153,   347,   348,
     167,   177,   180,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   323,   184,   278,   279,   280,   186,   281,
     329,    50,    51,    52,    53,   189,   334,   244,   246,   205,
     206,   207,   208,   209,   235,   261,   254,   318,   321,   264,
     340,   291,   265,   288,   290,   297,   298,   301,   304,   351,
     337,   331,   204,   354,   356,   359,   360,   357,   292,   293,
     355,   363,   366,   295,   296,   370,   371,   299,   333,   187,
     263,     0,    32,   309,   285,     0,   350,   372,     0,     0,
       0,   375,     0,     0,     0,     0,     0,   380,     0,     0,
       0,   383,     0,     0,     0,   364,     0,    70,   -35,   317,
       0,   312,     0,     0,     0,     0,     0,     0,     0,     0,
      32,     0,    32,     0,     0,     0,   324,   325,   326,   327,
       0,     0,     0,     0,     0,   131,   132,   133,   134,   135,
     136,     0,   137,    32,     0,   138,   139,   140,     0,    32,
       0,     0,     0,     0,    32,     0,   345,   346,     0,     0,
      32,   141,   142,   143,   144,   145,   146,   147,     0,   358,
       0,     0,     0,     0,     0,   362,     0,    32,     0,     0,
       0,     0,     0,     0,   369,     0,     0,  -169,    39,     0,
       0,     0,    32,     0,     0,    32,     0,     0,     0,     0,
      32,    40,     0,    32,    41,    42,     0,     0,    43,  -169,
       2,     3,    44,    45,    46,     0,    47,  -169,  -169,     0,
      48,  -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,     0,
    -169,     0,  -169,  -169,  -169,     0,  -169,  -169,  -169,  -169,
    -169,  -169,  -169,     0,     0,  -169,  -169,  -169,  -169,  -169,
    -169,  -169,  -169,  -169,    49,  -169,  -169,  -169,  -169,  -169,
    -169,    50,    51,    52,    53,    54,  -169,  -169,  -169,  -169,
       0,  -169,  -169,  -169,  -169,  -169,  -169,    55,    56,  -169,
    -169,     0,  -169,    65,  -169,  -169,    58,  -169,  -169,    39,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    40,     0,     0,    41,    42,     0,     0,    43,
    -169,     2,     3,    44,    45,    46,     0,    47,  -169,  -169,
       0,    48,  -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,
       0,  -169,     0,  -169,  -169,  -169,     0,  -169,  -169,  -169,
    -169,  -169,  -169,  -169,     0,     0,  -169,  -169,  -169,  -169,
    -169,  -169,  -169,  -169,  -169,    49,  -169,  -169,  -169,  -169,
    -169,  -169,    50,    51,    52,    53,    54,  -169,  -169,  -169,
    -169,     0,  -169,  -169,  -169,  -169,  -169,  -169,    55,    56,
    -169,  -169,    39,  -169,    67,  -169,  -169,    58,  -169,     0,
       0,     0,     0,     0,     0,    40,     0,     0,    41,    42,
       0,     0,    43,     0,     2,     3,    44,   122,   123,     0,
     124,     0,     0,     0,    48,     1,     2,     3,     0,     4,
       0,     0,     0,   339,     5,     0,     0,     0,     0,     0,
       7,     8,     9,    10,    11,     0,     0,     0,     0,    13,
       0,     0,    14,    15,    16,     0,     0,    17,    49,     0,
       0,    19,    20,    21,     0,    50,    51,    52,    53,    54,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  -169,
      39,    55,    56,     0,    22,    23,     0,    80,     0,     0,
      58,  -175,   125,    40,     0,     0,    41,    42,   319,     0,
      43,  -169,     2,     3,    44,    45,    46,     0,    47,  -169,
    -169,     0,    48,     0,  -169,  -169,  -169,  -169,  -169,  -169,
    -169,     0,     0,     0,     0,  -169,     0,     0,  -169,  -169,
    -169,     0,     0,  -169,     0,     0,     0,  -169,  -169,  -169,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
       0,     0,     0,    50,    51,    52,    53,    54,     0,     0,
    -169,  -169,     0,  -169,  -169,  -169,  -169,    39,     0,    55,
      56,     0,     0,     0,  -169,    57,     0,  -169,    58,     0,
      40,     0,     0,    41,    42,     0,     0,    43,  -169,     2,
       3,    44,    45,    46,     0,    47,  -169,  -169,     0,    48,
       0,  -169,  -169,  -169,  -169,  -169,  -169,  -169,     0,     0,
       0,     0,  -169,     0,     0,  -169,  -169,  -169,     0,     0,
    -169,     0,     0,     0,  -169,  -169,  -169,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
      50,    51,    52,    53,    54,     0,     0,  -169,  -169,     0,
    -169,  -169,  -169,  -132,    39,     0,    55,    56,     0,     0,
       0,  -169,    80,     0,  -169,    58,     0,    40,     0,     0,
      41,    42,     0,     0,    43,  -132,     2,     3,    44,    45,
      46,     0,    47,  -132,  -132,     0,    48,     0,  -132,  -132,
    -132,  -132,  -132,  -132,  -132,     0,     0,     0,     0,  -132,
       0,     0,  -132,  -132,  -132,     0,     0,  -132,     0,     0,
       0,  -132,  -132,  -132,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,    50,    51,    52,
      53,    54,     0,     0,  -132,  -132,     0,  -132,  -132,  -132,
      39,     0,     0,    55,    56,     0,     0,     0,  -132,    69,
       0,     0,    58,    40,     0,     0,    41,    42,     0,     0,
      43,     0,     2,     3,    44,    45,    46,     0,    47,     0,
       0,     0,    48,     0,     0,    39,     0,     0,     0,     0,
       0,     0,     0,     0,    76,    77,    78,    79,    40,     0,
       0,    41,    42,     0,     0,    43,     0,     2,     3,    44,
      45,    46,     0,    47,     0,     0,    49,    48,     0,     0,
       0,     0,     0,    50,    51,    52,    53,    54,     0,    76,
       0,    78,     0,     0,     0,     0,     0,     0,     0,    55,
      56,     0,     0,     0,     0,    80,     0,     0,    58,     0,
       0,    49,     0,     0,     0,     0,     0,     0,    50,    51,
      52,    53,    54,     0,     0,    39,     0,     0,     0,     0,
       0,     0,     0,     0,    55,    56,     0,     0,    40,     0,
      80,    41,    42,    58,     0,    43,     0,     2,     3,    44,
      45,    46,    39,    47,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    40,     0,     0,    41,    42,
       0,     0,    43,     0,     2,     3,    44,   151,    46,     0,
      47,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,    50,    51,
      52,    53,    54,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    55,    56,     0,     0,    49,     0,
      80,  -169,  -169,    58,     0,    50,    51,    52,    53,    54,
       0,     0,     0,     0,     0,     0,     0,     0,    39,     0,
       0,    55,    56,     0,     0,     0,     0,    80,  -169,  -169,
      58,    40,     0,     0,    41,    42,     0,    85,    43,     0,
       2,     3,    44,    45,    46,    39,    47,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    40,     0,
       0,    41,    42,     0,     0,    43,     0,     2,     3,    44,
      45,    46,     0,    47,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,    50,    51,    52,    53,    54,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    55,    56,     0,
       0,    49,     0,    80,     0,     0,    58,     0,    50,    51,
      52,    53,    54,     0,     0,     0,     0,     0,     0,     1,
       2,     3,     0,     4,    55,    56,     0,     0,     5,     0,
      80,     0,     0,    58,     7,     8,     9,    10,    11,     0,
       0,     0,     0,    13,     0,     0,    14,    15,    16,     0,
       0,    17,     0,     0,     0,    19,    20,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,     2,     3,     0,     4,     0,     0,    22,    23,
       5,     0,   376,     0,     0,     0,     7,     8,     9,    10,
      11,     0,   319,     0,     0,    13,     0,     0,    14,    15,
      16,     0,     0,    17,     0,     0,     0,    19,    20,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     1,     2,     3,     0,     4,     0,     0,
      22,    23,     5,     0,   382,     0,     0,     0,     7,     8,
       9,    10,    11,     0,   319,     0,     0,    13,     0,     0,
      14,    15,    16,     0,     0,    17,     0,     0,     0,    19,
      20,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     1,     2,     3,     0,     4,
       0,     0,    22,    23,     5,     0,   384,     0,     0,     0,
       7,     8,     9,    10,    11,     0,   319,     0,     0,    13,
       0,     0,    14,    15,    16,     0,     0,    17,     0,     0,
       0,    19,    20,    21,     1,     2,     3,     0,     4,     0,
       0,     0,     0,     5,     6,     0,     0,     0,     0,     7,
       8,     9,    10,    11,    22,    23,    12,     0,    13,     0,
       0,    14,    15,    16,     0,     0,    17,     0,   319,    18,
      19,    20,    21,     1,     2,     3,     0,     4,     0,     0,
       0,     0,     5,     0,     0,     0,     0,     0,     7,     8,
       9,    10,     0,    22,    23,    24,   164,     0,     0,     0,
      14,     0,    16,     0,     0,     0,     0,     0,     0,    19,
      20,    21,     0,     0,   131,   132,   133,   134,   135,   136,
       0,   137,   165,   166,   138,   139,   140,     0,     0,     0,
       0,     0,    76,    23,    78,     0,     0,     0,     0,     0,
     141,   142,   143,   144,   145,   146,   147,     0,   131,   132,
     133,   134,   135,   136,     0,   137,     0,     0,   138,   139,
     140,     0,     0,     0,     0,     0,     0,   192,     0,     0,
       0,     0,     0,   193,   141,   142,   143,   144,   145,   146,
     147,   131,   132,   133,   134,   135,   136,     0,   137,     0,
       0,   138,   139,   140,     0,     0,     0,     0,     0,     0,
     194,     0,     0,     0,     0,     0,   195,   141,   142,   143,
     144,   145,   146,   147,   131,   132,   133,   134,   135,   136,
       0,   137,     0,     0,   138,   139,   140,     0,     0,     0,
       0,     0,     0,   196,     0,     0,     0,     0,     0,   197,
     141,   142,   143,   144,   145,   146,   147,   131,   132,   133,
     134,   135,   136,     0,   137,     0,     0,   138,   139,   140,
       0,     0,     0,     0,     0,     0,   198,     0,     0,     0,
       0,     0,   199,   141,   142,   143,   144,   145,   146,   147,
     131,   132,   133,   134,   135,   136,     0,   137,     0,     0,
     138,   139,   140,     0,     0,     0,     0,     0,     0,   305,
       0,     0,     0,     0,     0,     0,   141,   142,   143,   144,
     145,   146,   147,   131,   132,   133,   134,   135,   136,     0,
     137,     0,     0,   138,   139,   140,     0,     0,     0,     0,
       0,     0,   306,     0,     0,     0,     0,     0,     0,   141,
     142,   143,   144,   145,   146,   147,   131,   132,   133,   134,
     135,   136,     0,   137,     0,     0,   138,   139,   140,     0,
       0,     0,     0,     0,     0,   307,     0,     0,     0,     0,
       0,     0,   141,   142,   143,   144,   145,   146,   147,   131,
     132,   133,   134,   135,   136,     0,   137,     0,     0,   138,
     139,   140,     0,     0,     0,     0,     0,     0,   308,     0,
       0,     0,     0,     0,     0,   141,   142,   143,   144,   145,
     146,   147,   131,   132,   133,   134,   135,   136,     0,   137,
       0,     0,   138,   139,   140,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   182,   141,   142,
     143,   144,   145,   146,   147,   131,   132,   133,   134,   135,
     136,     0,   137,     0,     0,   138,   139,   140,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   141,   142,   143,   144,   145,   146,   147,   178,   183,
       0,     0,     0,     0,   131,   132,   133,   134,   135,   136,
       0,   137,     0,     0,   138,   139,   140,   131,   132,   133,
     134,   135,   136,     0,   137,     0,     0,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,     0,     0,     0,
       0,     0,     0,   141,   142,   143,   144,   145,   146,   147,
       0,     0,     0,   202,   131,   132,   133,   134,   135,   136,
       0,   137,     0,     0,   138,   139,   140,     0,     0,     0,
       0,     0,   200,   201,     0,     0,     0,     0,     0,     0,
     141,   142,   143,   144,   145,   146,   147,   131,   132,   133,
     134,   135,   136,     0,   137,     0,     0,   138,   139,   140,
     131,   132,   133,   134,     0,     0,     0,   137,     0,     0,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
       0,     0,     0,     0,     0,     0,   141,   142,   143,   144,
     145,   146,   147
};

static const yytype_int16 yycheck[] =
{
       0,    73,   249,     0,   155,     4,   334,    50,    20,   287,
       9,     1,    53,   361,    50,    14,    88,    68,     0,    53,
      19,    20,    63,    22,    14,   373,    95,    17,    18,    63,
     378,    21,    31,   311,    42,    26,    26,    27,    53,    29,
      68,    92,    93,    71,     1,     2,     3,   375,    63,    26,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      95,    89,    90,    91,    92,    93,    65,    42,    67,    81,
      69,    96,    24,    68,    26,    47,   101,    76,    24,    53,
      26,    80,    90,    91,    95,    97,   237,    98,    63,    63,
     162,    53,    55,    56,   341,    90,    91,    92,    93,    26,
      57,    63,   110,    96,   104,    26,    96,   104,    65,    99,
      67,   358,    98,    26,    27,    72,    29,    95,    97,    98,
      98,   368,    74,    75,    76,    77,    45,   374,    74,    75,
      76,    77,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
      17,    18,    19,    95,   153,   163,    98,    97,    98,    26,
     159,    24,    98,    26,    26,   265,    33,    97,    98,   168,
     169,   170,    97,    98,   173,   174,   175,   176,   177,   178,
      96,    97,    98,   164,   165,   166,    97,    98,    35,    36,
      26,    52,    26,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   303,    26,   204,   205,   206,    26,   208,
     310,    74,    75,    76,    77,    26,   316,   165,   166,   101,
     101,   100,    98,    98,    98,    98,    47,   299,   300,    98,
     330,   239,    52,    26,    43,    26,    26,    26,    26,    26,
      86,   313,   101,    53,    85,    84,    98,   347,   247,   248,
      34,    53,    26,   252,   253,    84,    52,   262,   315,   104,
     183,    -1,   262,   286,   209,    -1,   338,   367,    -1,    -1,
      -1,   371,    -1,    -1,    -1,    -1,    -1,   377,    -1,    -1,
      -1,   381,    -1,    -1,    -1,   357,    -1,   286,    34,   297,
      -1,   290,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     300,    -1,   302,    -1,    -1,    -1,   305,   306,   307,   308,
      -1,    -1,    -1,    -1,    -1,    61,    62,    63,    64,    65,
      66,    -1,    68,   323,    -1,    71,    72,    73,    -1,   329,
      -1,    -1,    -1,    -1,   334,    -1,   335,   336,    -1,    -1,
     340,    87,    88,    89,    90,    91,    92,    93,    -1,   348,
      -1,    -1,    -1,    -1,    -1,   354,    -1,   357,    -1,    -1,
      -1,    -1,    -1,    -1,   363,    -1,    -1,     0,     1,    -1,
      -1,    -1,   372,    -1,    -1,   375,    -1,    -1,    -1,    -1,
     380,    14,    -1,   383,    17,    18,    -1,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    29,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    -1,
      43,    -1,    45,    46,    47,    -1,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      -1,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,   100,     0,     1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    14,    -1,    -1,    17,    18,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      -1,    43,    -1,    45,    46,    47,    -1,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    -1,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,     1,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    17,    18,
      -1,    -1,    21,    -1,    23,    24,    25,    26,    27,    -1,
      29,    -1,    -1,    -1,    33,    22,    23,    24,    -1,    26,
      -1,    -1,    -1,    30,    31,    -1,    -1,    -1,    -1,    -1,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      -1,    -1,    49,    50,    51,    -1,    -1,    54,    67,    -1,
      -1,    58,    59,    60,    -1,    74,    75,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     0,
       1,    90,    91,    -1,    81,    82,    -1,    96,    -1,    -1,
      99,   100,   101,    14,    -1,    -1,    17,    18,    95,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    29,    30,
      31,    -1,    33,    -1,    35,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,    -1,    -1,    49,    50,
      51,    -1,    -1,    54,    -1,    -1,    -1,    58,    59,    60,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    -1,    -1,
      81,    82,    -1,    84,    85,    86,     0,     1,    -1,    90,
      91,    -1,    -1,    -1,    95,    96,    -1,    98,    99,    -1,
      14,    -1,    -1,    17,    18,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    29,    30,    31,    -1,    33,
      -1,    35,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    -1,    -1,    49,    50,    51,    -1,    -1,
      54,    -1,    -1,    -1,    58,    59,    60,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    -1,    -1,    81,    82,    -1,
      84,    85,    86,     0,     1,    -1,    90,    91,    -1,    -1,
      -1,    95,    96,    -1,    98,    99,    -1,    14,    -1,    -1,
      17,    18,    -1,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    30,    31,    -1,    33,    -1,    35,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
      -1,    -1,    49,    50,    51,    -1,    -1,    54,    -1,    -1,
      -1,    58,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    -1,    -1,    81,    82,    -1,    84,    85,    86,
       1,    -1,    -1,    90,    91,    -1,    -1,    -1,    95,    96,
      -1,    -1,    99,    14,    -1,    -1,    17,    18,    -1,    -1,
      21,    -1,    23,    24,    25,    26,    27,    -1,    29,    -1,
      -1,    -1,    33,    -1,    -1,     1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    48,    14,    -1,
      -1,    17,    18,    -1,    -1,    21,    -1,    23,    24,    25,
      26,    27,    -1,    29,    -1,    -1,    67,    33,    -1,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    -1,    45,
      -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      91,    -1,    -1,    -1,    -1,    96,    -1,    -1,    99,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    -1,    -1,     1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    -1,    -1,    14,    -1,
      96,    17,    18,    99,    -1,    21,    -1,    23,    24,    25,
      26,    27,     1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,    17,    18,
      -1,    -1,    21,    -1,    23,    24,    25,    26,    27,    -1,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    -1,    -1,    67,    -1,
      96,    97,    98,    99,    -1,    74,    75,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
      -1,    90,    91,    -1,    -1,    -1,    -1,    96,    97,    98,
      99,    14,    -1,    -1,    17,    18,    -1,    20,    21,    -1,
      23,    24,    25,    26,    27,     1,    29,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    14,    -1,
      -1,    17,    18,    -1,    -1,    21,    -1,    23,    24,    25,
      26,    27,    -1,    29,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,    -1,
      -1,    67,    -1,    96,    -1,    -1,    99,    -1,    74,    75,
      76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    -1,    26,    90,    91,    -1,    -1,    31,    -1,
      96,    -1,    -1,    99,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    -1,
      -1,    54,    -1,    -1,    -1,    58,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    23,    24,    -1,    26,    -1,    -1,    81,    82,
      31,    -1,    85,    -1,    -1,    -1,    37,    38,    39,    40,
      41,    -1,    95,    -1,    -1,    46,    -1,    -1,    49,    50,
      51,    -1,    -1,    54,    -1,    -1,    -1,    58,    59,    60,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    23,    24,    -1,    26,    -1,    -1,
      81,    82,    31,    -1,    85,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    -1,    95,    -1,    -1,    46,    -1,    -1,
      49,    50,    51,    -1,    -1,    54,    -1,    -1,    -1,    58,
      59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    23,    24,    -1,    26,
      -1,    -1,    81,    82,    31,    -1,    85,    -1,    -1,    -1,
      37,    38,    39,    40,    41,    -1,    95,    -1,    -1,    46,
      -1,    -1,    49,    50,    51,    -1,    -1,    54,    -1,    -1,
      -1,    58,    59,    60,    22,    23,    24,    -1,    26,    -1,
      -1,    -1,    -1,    31,    32,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    81,    82,    44,    -1,    46,    -1,
      -1,    49,    50,    51,    -1,    -1,    54,    -1,    95,    57,
      58,    59,    60,    22,    23,    24,    -1,    26,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    -1,    81,    82,    83,    43,    -1,    -1,    -1,
      49,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    58,
      59,    60,    -1,    -1,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    45,    82,    47,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    93,    -1,    61,    62,
      63,    64,    65,    66,    -1,    68,    -1,    -1,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,
      -1,    -1,    -1,    53,    87,    88,    89,    90,    91,    92,
      93,    61,    62,    63,    64,    65,    66,    -1,    68,    -1,
      -1,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    -1,    -1,    -1,    53,    87,    88,    89,
      90,    91,    92,    93,    61,    62,    63,    64,    65,    66,
      -1,    68,    -1,    -1,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    -1,    -1,    -1,    53,
      87,    88,    89,    90,    91,    92,    93,    61,    62,    63,
      64,    65,    66,    -1,    68,    -1,    -1,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,
      -1,    -1,    53,    87,    88,    89,    90,    91,    92,    93,
      61,    62,    63,    64,    65,    66,    -1,    68,    -1,    -1,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    93,    61,    62,    63,    64,    65,    66,    -1,
      68,    -1,    -1,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    89,    90,    91,    92,    93,    61,    62,    63,    64,
      65,    66,    -1,    68,    -1,    -1,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    93,    61,
      62,    63,    64,    65,    66,    -1,    68,    -1,    -1,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    93,    61,    62,    63,    64,    65,    66,    -1,    68,
      -1,    -1,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    87,    88,
      89,    90,    91,    92,    93,    61,    62,    63,    64,    65,
      66,    -1,    68,    -1,    -1,    71,    72,    73,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    88,    89,    90,    91,    92,    93,    55,    95,
      -1,    -1,    -1,    -1,    61,    62,    63,    64,    65,    66,
      -1,    68,    -1,    -1,    71,    72,    73,    61,    62,    63,
      64,    65,    66,    -1,    68,    -1,    -1,    71,    72,    73,
      87,    88,    89,    90,    91,    92,    93,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    88,    89,    90,    91,    92,    93,
      -1,    -1,    -1,    97,    61,    62,    63,    64,    65,    66,
      -1,    68,    -1,    -1,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    93,    61,    62,    63,
      64,    65,    66,    -1,    68,    -1,    -1,    71,    72,    73,
      61,    62,    63,    64,    -1,    -1,    -1,    68,    -1,    -1,
      71,    72,    73,    87,    88,    89,    90,    91,    92,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    93
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    22,    23,    24,    26,    31,    32,    37,    38,    39,
      40,    41,    44,    46,    49,    50,    51,    54,    57,    58,
      59,    60,    81,    82,    83,   103,   104,   105,   106,   107,
     119,   125,   129,   134,   141,   143,   145,   148,   153,     1,
      14,    17,    18,    21,    25,    26,    27,    29,    33,    67,
      74,    75,    76,    77,    78,    90,    91,    96,    99,   128,
     129,   131,   132,   154,   156,    96,   154,    96,   154,    96,
     131,   155,   144,   147,    50,   135,    45,    46,    47,    48,
      96,   131,   142,   146,    50,    20,   131,    55,    56,    17,
      18,    19,    26,    33,    26,    26,   131,   131,   142,   137,
     131,   136,   149,     0,    95,   131,   126,    95,    26,   128,
      47,    96,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   154,    26,    27,    29,   101,   131,   157,   158,   159,
     160,    61,    62,    63,    64,    65,    66,    68,    71,    72,
      73,    87,    88,    89,    90,    91,    92,    93,    98,   154,
     154,    26,   154,    98,   154,   126,    26,   138,   131,    45,
     131,   142,    26,    26,    43,    69,    70,    26,   126,    53,
      63,   128,   128,    53,    63,    53,    63,    52,    55,   142,
      26,   140,    53,    95,    26,   139,    26,   103,   122,    26,
     150,   128,    47,    53,    47,    53,    47,    53,    47,    53,
      79,    80,    97,    97,   101,   101,   101,   100,    98,    98,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,    97,    97,
      97,    98,    97,   131,   150,    98,   131,   126,   128,    24,
      26,   132,   133,    26,   133,    26,   133,    42,    63,   131,
     131,   131,    53,    63,    47,   131,   131,   131,   131,   131,
     131,    98,   118,   118,    98,    52,    95,    98,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,    26,    27,    29,   160,   130,    95,    26,   150,
      43,   128,   131,   131,   122,   131,   131,    26,    26,   106,
     127,    26,   127,   152,    26,    47,    47,    47,    47,   155,
     152,    95,   131,   114,   108,   111,   117,   128,   126,    95,
     107,   126,   123,   127,   131,   131,   131,   131,    97,   127,
     152,   126,   124,   124,   127,    53,    63,    86,   120,    30,
     127,   115,   109,   112,   123,   131,   131,    35,    36,   121,
     126,    26,   151,   122,    53,    34,    85,   127,   131,    84,
      98,   116,   131,    53,   126,   122,    26,   117,   110,   131,
      84,    52,   127,   122,   113,   127,    85,   117,   122,   123,
     127,   117,    85,   127,    85
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   102,   103,   103,   104,   104,   104,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   106,   106,   106,   106,   107,   107,
     107,   108,   109,   110,   107,   111,   112,   113,   107,   114,
     115,   116,   107,   107,   107,   107,   107,   117,   118,   119,
     119,   120,   120,   121,   122,   123,   124,   125,   126,   127,
     127,   127,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   129,   129,   129,   129,   129,   130,   129,   129,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   132,   132,   132,
     132,   132,   132,   132,   132,   133,   133,   134,   134,   134,
     134,   134,   135,   134,   136,   134,   137,   134,   134,   134,
     134,   134,   134,   138,   138,   139,   139,   140,   140,   141,
     141,   141,   141,   141,   141,   142,   142,   143,   143,   143,
     144,   143,   146,   145,   145,   147,   145,   145,   145,   149,
     148,   150,   150,   150,   151,   151,   151,   152,   153,   154,
     154,   154,   155,   155,   156,   157,   157,   157,   157,   158,
     158,   159,   159,   160,   160,   160
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     0,     1,     1,     4,     4,     6,
       4,     4,     4,     4,     4,     4,     4,     4,     5,     5,
       8,     8,     4,     4,     1,     1,     1,     1,     1,     1,
       9,     0,     0,     0,    15,     0,     0,     0,    16,     0,
       0,     0,    13,     2,     4,     7,     6,     0,     0,     9,
      11,     0,     2,     6,     0,     0,     0,     1,     0,     0,
       2,     2,     1,     1,     1,     1,     1,     1,     2,     3,
       1,     2,     4,     2,     4,     2,     4,     0,     7,     4,
       1,     1,     1,     3,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     2,     2,     4,     4,     4,     6,     4,
       6,     4,     6,     4,     6,     2,     1,     2,     1,     1,
       2,     1,     0,     3,     0,     3,     0,     3,     4,     2,
       4,     2,     1,     1,     3,     1,     3,     1,     3,     1,
       2,     2,     2,     3,     2,     3,     2,     2,     3,     2,
       0,     3,     0,     8,     2,     0,     7,     8,     6,     0,
       3,     0,     1,     3,     0,     1,     3,     0,     2,     0,
       1,     3,     1,     3,     3,     0,     1,     1,     1,     1,
       3,     1,     3,     3,     3,     3
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
    case 22: /* BLTIN  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1871 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 23: /* FBLTIN  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1877 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 24: /* RBLTIN  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1883 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 25: /* THEFBLTIN  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1889 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 26: /* ID  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1895 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 27: /* STRING  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1901 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 28: /* HANDLER  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1907 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 29: /* SYMBOL  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1913 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 30: /* ENDCLAUSE  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1919 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 31: /* tPLAYACCEL  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1925 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 32: /* tMETHOD  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1931 "engines/director/lingo/lingo-gr.cpp"
        break;

    case 148: /* on  */
#line 210 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 1937 "engines/director/lingo/lingo-gr.cpp"
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
#line 221 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2224 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 8:
#line 228 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[-2].code); }
#line 2232 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 9:
#line 232 "engines/director/lingo/lingo-gr.y"
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
#line 2248 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10:
#line 243 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_after);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2260 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11:
#line 250 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_after);
		(yyval.code) = (yyvsp[-2].code); }
#line 2268 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12:
#line 253 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		mVar((yyvsp[0].s), globalCheck());
		g_lingo->code1(LC::c_before);
		(yyval.code) = (yyvsp[-2].code);
		delete (yyvsp[0].s); }
#line 2280 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13:
#line 260 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_before);
		(yyval.code) = (yyvsp[-2].code); }
#line 2288 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14:
#line 263 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2300 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15:
#line 270 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2312 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16:
#line 277 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_varpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		mVar((yyvsp[-2].s), globalCheck());
		g_lingo->code1(LC::c_assign);
		(yyval.code) = (yyvsp[0].code);
		delete (yyvsp[-2].s); }
#line 2324 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17:
#line 284 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-2].e)[0]);
		g_lingo->codeInt((yyvsp[-2].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2336 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 18:
#line 291 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2347 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19:
#line 297 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_swap);
		g_lingo->code1(LC::c_theentityassign);
		g_lingo->codeInt((yyvsp[-3].e)[0]);
		g_lingo->codeInt((yyvsp[-3].e)[1]);
		(yyval.code) = (yyvsp[0].code); }
#line 2358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20:
#line 304 "engines/director/lingo/lingo-gr.y"
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
#line 2374 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21:
#line 315 "engines/director/lingo/lingo-gr.y"
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
#line 2390 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 22:
#line 326 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2402 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23:
#line 333 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_objectpropassign);
		g_lingo->codeString((yyvsp[-2].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[-2].objectprop).prop->c_str());
		delete (yyvsp[-2].objectprop).obj;
		delete (yyvsp[-2].objectprop).prop;
		(yyval.code) = (yyvsp[0].code); }
#line 2414 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30:
#line 352 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		inst start = 0, end = 0;
		WRITE_UINT32(&start, (yyvsp[-6].code) - (yyvsp[-1].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-4].code) + 2);
		(*g_lingo->_currentAssembly)[(yyvsp[-4].code)] = end;		/* end, if cond fails */
		(*g_lingo->_currentAssembly)[(yyvsp[-1].code)] = start;	/* looping back */
		endRepeat((yyvsp[-1].code) + 1, (yyvsp[-6].code));	}
#line 2426 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 31:
#line 365 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2434 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32:
#line 369 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2441 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33:
#line 372 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_le); }
#line 2447 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34:
#line 372 "engines/director/lingo/lingo-gr.y"
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
#line 2472 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35:
#line 399 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_varpush);
				  g_lingo->codeString((yyvsp[-2].s)->c_str());
				  mVar((yyvsp[-2].s), globalCheck()); }
#line 2480 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36:
#line 403 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_eval);
				  g_lingo->codeString((yyvsp[-4].s)->c_str()); }
#line 2487 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37:
#line 406 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_ge); }
#line 2493 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38:
#line 407 "engines/director/lingo/lingo-gr.y"
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
#line 2518 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39:
#line 433 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);
				  g_lingo->codeInt(0);
				  Common::String count("count");
				  g_lingo->codeFunc(&count, 1);
				  g_lingo->code1(LC::c_intpush);	// start counter
				  g_lingo->codeInt(1); }
#line 2529 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40:
#line 440 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_stackpeek);	// get counter
				  g_lingo->codeInt(0);
				  g_lingo->code1(LC::c_stackpeek);	// get array size
				  g_lingo->codeInt(2);
				  g_lingo->code1(LC::c_le); }
#line 2539 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41:
#line 446 "engines/director/lingo/lingo-gr.y"
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
#line 2554 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42:
#line 456 "engines/director/lingo/lingo-gr.y"
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
#line 2578 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43:
#line 476 "engines/director/lingo/lingo-gr.y"
                        {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->nexts.push_back(pos);
		} else {
			warning("# LINGO: next repeat not inside repeat block");
		} }
#line 2591 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44:
#line 484 "engines/director/lingo/lingo-gr.y"
                              {
		g_lingo->code1(LC::c_whencode);
		g_lingo->codeString((yyvsp[-2].s)->c_str()); }
#line 2599 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45:
#line 487 "engines/director/lingo/lingo-gr.y"
                                                          { g_lingo->code1(LC::c_telldone); }
#line 2605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46:
#line 488 "engines/director/lingo/lingo-gr.y"
                                                    { g_lingo->code1(LC::c_telldone); }
#line 2611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47:
#line 490 "engines/director/lingo/lingo-gr.y"
                                { startRepeat(); }
#line 2617 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48:
#line 492 "engines/director/lingo/lingo-gr.y"
                                { g_lingo->code1(LC::c_tell); }
#line 2623 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49:
#line 494 "engines/director/lingo/lingo-gr.y"
                                                                                         {
		inst else1 = 0, end3 = 0;
		WRITE_UINT32(&else1, (yyvsp[-3].code) + 1 - (yyvsp[-6].code) + 1);
		WRITE_UINT32(&end3, (yyvsp[-1].code) - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-6].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = end3;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-3].code), (yyvsp[-1].code)); }
#line 2635 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50:
#line 501 "engines/director/lingo/lingo-gr.y"
                                                                                                          {
		inst else1 = 0, end = 0;
		WRITE_UINT32(&else1, (yyvsp[-5].code) + 1 - (yyvsp[-8].code) + 1);
		WRITE_UINT32(&end, (yyvsp[-1].code) - (yyvsp[-5].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-8].code)] = else1;		/* elsepart */
		(*g_lingo->_currentAssembly)[(yyvsp[-5].code)] = end;		/* end, if cond fails */
		g_lingo->processIf((yyvsp[-5].code), (yyvsp[-1].code)); }
#line 2647 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53:
#line 512 "engines/director/lingo/lingo-gr.y"
                                                                {
		inst else1 = 0;
		WRITE_UINT32(&else1, (yyvsp[0].code) + 1 - (yyvsp[-3].code) + 1);
		(*g_lingo->_currentAssembly)[(yyvsp[-3].code)] = else1;	/* end, if cond fails */
		g_lingo->codeLabel((yyvsp[0].code)); }
#line 2657 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54:
#line 518 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jumpifz, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2665 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55:
#line 522 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code2(LC::c_jump, 0);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2673 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56:
#line 526 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_assign);
		(yyval.code) = g_lingo->_currentAssembly->size() - 1; }
#line 2681 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57:
#line 530 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeLabel(0); }
#line 2688 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58:
#line 533 "engines/director/lingo/lingo-gr.y"
                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2694 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59:
#line 535 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = g_lingo->_currentAssembly->size(); }
#line 2700 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62:
#line 539 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt((yyvsp[0].i)); }
#line 2708 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63:
#line 542 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_floatpush);
		g_lingo->codeFloat((yyvsp[0].f)); }
#line 2716 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64:
#line 545 "engines/director/lingo/lingo-gr.y"
                        {											// D3
		(yyval.code) = g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2725 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65:
#line 549 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2734 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66:
#line 553 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_eval);
		g_lingo->codeString((yyvsp[0].s)->c_str());
		delete (yyvsp[0].s); }
#line 2743 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67:
#line 557 "engines/director/lingo/lingo-gr.y"
                        {
		(yyval.code) = g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(0); // Put dummy id
		g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[0].e)[0]);
		WRITE_UINT32(&f, (yyvsp[0].e)[1]);
		g_lingo->code2(e, f); }
#line 2756 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68:
#line 565 "engines/director/lingo/lingo-gr.y"
                                     {
		(yyval.code) = g_lingo->code1(LC::c_theentitypush);
		inst e = 0, f = 0;
		WRITE_UINT32(&e, (yyvsp[-1].e)[0]);
		WRITE_UINT32(&f, (yyvsp[-1].e)[1]);
		g_lingo->code2(e, f); }
#line 2767 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69:
#line 571 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.code) = (yyvsp[-1].code); }
#line 2773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71:
#line 573 "engines/director/lingo/lingo-gr.y"
                                {
		// Director parser till D3 was forgiving for any hanging parentheses
		if (g_lingo->_ignoreError) {
			warning("# LINGO: Ignoring trailing paren before %d:%d", g_lingo->_linenumber, g_lingo->_colnumber);
			g_lingo->_ignoreError = false;
			lex_unput('\n');	// We ate '\n', so put it back, otherwise lines will be joined
		} else {
			yyerrok;
		}
	}
#line 2788 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72:
#line 584 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2796 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73:
#line 587 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2804 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74:
#line 590 "engines/director/lingo/lingo-gr.y"
                                 {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2812 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75:
#line 593 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 2820 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76:
#line 596 "engines/director/lingo/lingo-gr.y"
                                      {
			g_lingo->code1(LC::c_lazyeval);
			g_lingo->codeString((yyvsp[-1].s)->c_str());
			g_lingo->codeFunc((yyvsp[-3].s), 1);
			delete (yyvsp[-3].s);
			delete (yyvsp[-1].s); }
#line 2831 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77:
#line 602 "engines/director/lingo/lingo-gr.y"
                                      { g_lingo->code1(LC::c_lazyeval); g_lingo->codeString((yyvsp[-1].s)->c_str()); }
#line 2837 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78:
#line 603 "engines/director/lingo/lingo-gr.y"
                                                    {
			g_lingo->codeFunc((yyvsp[-6].s), (yyvsp[-1].narg) + 1);
			delete (yyvsp[-6].s);
			delete (yyvsp[-4].s); }
#line 2846 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79:
#line 607 "engines/director/lingo/lingo-gr.y"
                                {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 2854 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 80:
#line 611 "engines/director/lingo/lingo-gr.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 2860 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83:
#line 614 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyval.code) = g_lingo->codeFunc((yyvsp[-2].s), 1);
		delete (yyvsp[-2].s); }
#line 2868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84:
#line 617 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_objectproppush);
		g_lingo->codeString((yyvsp[0].objectprop).obj->c_str());
		g_lingo->codeString((yyvsp[0].objectprop).prop->c_str());
		delete (yyvsp[0].objectprop).obj;
		delete (yyvsp[0].objectprop).prop; }
#line 2879 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85:
#line 623 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_add); }
#line 2885 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86:
#line 624 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_sub); }
#line 2891 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87:
#line 625 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mul); }
#line 2897 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88:
#line 626 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_div); }
#line 2903 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89:
#line 627 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_mod); }
#line 2909 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90:
#line 628 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gt); }
#line 2915 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91:
#line 629 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lt); }
#line 2921 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92:
#line 630 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_eq); }
#line 2927 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93:
#line 631 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_neq); }
#line 2933 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94:
#line 632 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ge); }
#line 2939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95:
#line 633 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_le); }
#line 2945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96:
#line 634 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_and); }
#line 2951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97:
#line 635 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_or); }
#line 2957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98:
#line 636 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_not); }
#line 2963 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99:
#line 637 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_ampersand); }
#line 2969 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100:
#line 638 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_concat); }
#line 2975 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 101:
#line 639 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_contains); }
#line 2981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 102:
#line 640 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_starts); }
#line 2987 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103:
#line 641 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); }
#line 2993 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104:
#line 642 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[0].code); g_lingo->code1(LC::c_negate); }
#line 2999 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 105:
#line 643 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_intersects); }
#line 3005 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106:
#line 644 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_within); }
#line 3011 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 107:
#line 646 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_charOf); }
#line 3017 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 108:
#line 647 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_charToOf); }
#line 3023 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 109:
#line 648 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_itemOf); }
#line 3029 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 110:
#line 649 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_itemToOf); }
#line 3035 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 111:
#line 650 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_lineOf); }
#line 3041 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 112:
#line 651 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_lineToOf); }
#line 3047 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 113:
#line 652 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_wordOf); }
#line 3053 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 114:
#line 653 "engines/director/lingo/lingo-gr.y"
                                        { g_lingo->code1(LC::c_wordToOf); }
#line 3059 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 115:
#line 655 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-1].s), 1);
		delete (yyvsp[-1].s); }
#line 3067 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117:
#line 660 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_printtop); }
#line 3073 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120:
#line 663 "engines/director/lingo/lingo-gr.y"
                                                {
		if (g_lingo->_repeatStack.size()) {
			g_lingo->code2(LC::c_jump, 0);
			int pos = g_lingo->_currentAssembly->size() - 1;
			g_lingo->_repeatStack.back()->exits.push_back(pos);
		} else {
			warning("# LINGO: exit repeat not inside repeat block");
		} }
#line 3086 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121:
#line 671 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_procret); }
#line 3092 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122:
#line 672 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3098 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123:
#line 672 "engines/director/lingo/lingo-gr.y"
                                                                                 { inLast(); }
#line 3104 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124:
#line 673 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3110 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125:
#line 673 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 3116 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126:
#line 674 "engines/director/lingo/lingo-gr.y"
                                                        { inArgs(); }
#line 3122 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127:
#line 674 "engines/director/lingo/lingo-gr.y"
                                                                                   { inLast(); }
#line 3128 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128:
#line 675 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->codeFunc((yyvsp[-3].s), (yyvsp[-1].narg));
		delete (yyvsp[-3].s); }
#line 3136 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129:
#line 678 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->codeFunc((yyvsp[-1].s), (yyvsp[0].narg));
		delete (yyvsp[-1].s); }
#line 3144 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130:
#line 681 "engines/director/lingo/lingo-gr.y"
                                        {
		Common::String open("open");
		g_lingo->codeFunc(&open, 2); }
#line 3152 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131:
#line 684 "engines/director/lingo/lingo-gr.y"
                                                {
		Common::String open("open");
		g_lingo->codeFunc(&open, 1); }
#line 3160 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132:
#line 687 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->codeFunc((yyvsp[0].s), 0);
		delete (yyvsp[0].s); }
#line 3168 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133:
#line 691 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3176 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134:
#line 694 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarGlobal);
		delete (yyvsp[0].s); }
#line 3184 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135:
#line 698 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3192 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136:
#line 701 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarProperty);
		delete (yyvsp[0].s); }
#line 3200 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137:
#line 705 "engines/director/lingo/lingo-gr.y"
                                                {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3208 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138:
#line 708 "engines/director/lingo/lingo-gr.y"
                                        {
		mVar((yyvsp[0].s), kVarInstance);
		delete (yyvsp[0].s); }
#line 3216 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139:
#line 719 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoloop); }
#line 3222 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140:
#line 720 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->code1(LC::c_gotonext); }
#line 3228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141:
#line 721 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->code1(LC::c_gotoprevious); }
#line 3234 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142:
#line 722 "engines/director/lingo/lingo-gr.y"
                                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_goto); }
#line 3243 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143:
#line 726 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_goto); }
#line 3252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144:
#line 730 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_goto); }
#line 3261 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147:
#line 738 "engines/director/lingo/lingo-gr.y"
                                        { // "play #done" is also caught by this
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(1);
		g_lingo->code1(LC::c_play); }
#line 3270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148:
#line 742 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(3);
		g_lingo->code1(LC::c_play); }
#line 3279 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149:
#line 746 "engines/director/lingo/lingo-gr.y"
                                                {
		g_lingo->code1(LC::c_intpush);
		g_lingo->codeInt(2);
		g_lingo->code1(LC::c_play); }
#line 3288 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150:
#line 750 "engines/director/lingo/lingo-gr.y"
                     { g_lingo->codeSetImmediate(true); }
#line 3294 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151:
#line 750 "engines/director/lingo/lingo-gr.y"
                                                                  {
		g_lingo->codeSetImmediate(false);
		g_lingo->codeFunc((yyvsp[-2].s), (yyvsp[0].narg));
		delete (yyvsp[-2].s); }
#line 3303 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152:
#line 780 "engines/director/lingo/lingo-gr.y"
             { startDef(); }
#line 3309 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153:
#line 781 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3319 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154:
#line 786 "engines/director/lingo/lingo-gr.y"
                        { g_lingo->codeFactory(*(yyvsp[0].s)); delete (yyvsp[0].s); }
#line 3325 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155:
#line 787 "engines/director/lingo/lingo-gr.y"
                  { startDef(); (*g_lingo->_methodVars)["me"] = kVarArgument; }
#line 3331 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 156:
#line 788 "engines/director/lingo/lingo-gr.y"
                                                                        {
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-6].s), (yyvsp[-4].code), (yyvsp[-3].narg) + 1);
		endDef();
		delete (yyvsp[-6].s); }
#line 3341 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 157:
#line 793 "engines/director/lingo/lingo-gr.y"
                                                                   {	// D3
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-7].s), (yyvsp[-6].code), (yyvsp[-5].narg));
		endDef();

		checkEnd((yyvsp[-1].s), (yyvsp[-7].s)->c_str(), false);
		delete (yyvsp[-7].s);
		delete (yyvsp[-1].s); }
#line 3354 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158:
#line 801 "engines/director/lingo/lingo-gr.y"
                                               {	// D4. No 'end' clause
		g_lingo->code1(LC::c_procret);
		g_lingo->codeDefine(*(yyvsp[-5].s), (yyvsp[-4].code), (yyvsp[-3].narg));
		endDef();
		delete (yyvsp[-5].s); }
#line 3364 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159:
#line 807 "engines/director/lingo/lingo-gr.y"
         { startDef(); }
#line 3370 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160:
#line 807 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = (yyvsp[0].s); }
#line 3376 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161:
#line 809 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3382 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162:
#line 810 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = 1; delete (yyvsp[0].s); }
#line 3388 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163:
#line 811 "engines/director/lingo/lingo-gr.y"
                                                { g_lingo->codeArg((yyvsp[0].s)); mVar((yyvsp[0].s), kVarArgument); (yyval.narg) = (yyvsp[-2].narg) + 1; delete (yyvsp[0].s); }
#line 3394 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165:
#line 814 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3400 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166:
#line 815 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3406 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167:
#line 817 "engines/director/lingo/lingo-gr.y"
                                        { inDef(); }
#line 3412 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168:
#line 819 "engines/director/lingo/lingo-gr.y"
                                        {
		g_lingo->code1(LC::c_call);
		g_lingo->codeString((yyvsp[-1].s)->c_str());
		inst numpar = 0;
		WRITE_UINT32(&numpar, (yyvsp[0].narg));
		g_lingo->code1(numpar);
		delete (yyvsp[-1].s); }
#line 3424 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169:
#line 827 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 0; }
#line 3430 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170:
#line 828 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.narg) = 1; }
#line 3436 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171:
#line 829 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3442 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172:
#line 831 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = 1; }
#line 3448 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173:
#line 832 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3454 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174:
#line 834 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = (yyvsp[-1].code); }
#line 3460 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175:
#line 836 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.code) = g_lingo->code2(LC::c_arraypush, 0); }
#line 3466 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176:
#line 837 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.code) = g_lingo->code2(LC::c_proparraypush, 0); }
#line 3472 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177:
#line 838 "engines/director/lingo/lingo-gr.y"
                         { (yyval.code) = g_lingo->code1(LC::c_proparraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3478 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178:
#line 839 "engines/director/lingo/lingo-gr.y"
                     { (yyval.code) = g_lingo->code1(LC::c_arraypush); (yyval.code) = g_lingo->codeInt((yyvsp[0].narg)); }
#line 3484 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179:
#line 841 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3490 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180:
#line 842 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3496 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181:
#line 844 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.narg) = 1; }
#line 3502 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182:
#line 845 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.narg) = (yyvsp[-2].narg) + 1; }
#line 3508 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183:
#line 847 "engines/director/lingo/lingo-gr.y"
                          {
		g_lingo->code1(LC::c_symbolpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3517 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184:
#line 851 "engines/director/lingo/lingo-gr.y"
                                {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3526 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185:
#line 855 "engines/director/lingo/lingo-gr.y"
                        {
		g_lingo->code1(LC::c_stringpush);
		g_lingo->codeString((yyvsp[-2].s)->c_str());
		delete (yyvsp[-2].s); }
#line 3535 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 3539 "engines/director/lingo/lingo-gr.cpp"

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

#line 861 "engines/director/lingo/lingo-gr.y"


int yyreport_syntax_error(const yypcontext_t *ctx) {
	int res = 0;

	if (lex_check_parens()) {
		g_lingo->_ignoreError = true;
		return 0;
	}

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
