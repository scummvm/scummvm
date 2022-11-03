/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_HYPNO_MIS_ENGINES_HYPNO_TOKENS_MIS_H_INCLUDED
# define YY_HYPNO_MIS_ENGINES_HYPNO_TOKENS_MIS_H_INCLUDED
/* Debug traces.  */
#ifndef HYPNO_MIS_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define HYPNO_MIS_DEBUG 1
#  else
#   define HYPNO_MIS_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define HYPNO_MIS_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined HYPNO_MIS_DEBUG */
#if HYPNO_MIS_DEBUG
extern int HYPNO_MIS_debug;
#endif

/* Token kinds.  */
#ifndef HYPNO_MIS_TOKENTYPE
# define HYPNO_MIS_TOKENTYPE
  enum HYPNO_MIS_tokentype
  {
    HYPNO_MIS_EMPTY = -2,
    HYPNO_MIS_EOF = 0,             /* "end of file"  */
    HYPNO_MIS_error = 256,         /* error  */
    HYPNO_MIS_UNDEF = 257,         /* "invalid token"  */
    NAME = 258,                    /* NAME  */
    FILENAME = 259,                /* FILENAME  */
    FLAG = 260,                    /* FLAG  */
    COMMENT = 261,                 /* COMMENT  */
    GSSWITCH = 262,                /* GSSWITCH  */
    COMMAND = 263,                 /* COMMAND  */
    WALNTOK = 264,                 /* WALNTOK  */
    ENCTOK = 265,                  /* ENCTOK  */
    NUM = 266,                     /* NUM  */
    HOTSTOK = 267,                 /* HOTSTOK  */
    CUTSTOK = 268,                 /* CUTSTOK  */
    BACKTOK = 269,                 /* BACKTOK  */
    INTRTOK = 270,                 /* INTRTOK  */
    RETTOK = 271,                  /* RETTOK  */
    TIMETOK = 272,                 /* TIMETOK  */
    PALETOK = 273,                 /* PALETOK  */
    BBOXTOK = 274,                 /* BBOXTOK  */
    OVERTOK = 275,                 /* OVERTOK  */
    MICETOK = 276,                 /* MICETOK  */
    SONDTOK = 277,                 /* SONDTOK  */
    PLAYTOK = 278,                 /* PLAYTOK  */
    ENDTOK = 279,                  /* ENDTOK  */
    MENUTOK = 280,                 /* MENUTOK  */
    SMENTOK = 281,                 /* SMENTOK  */
    ESCPTOK = 282,                 /* ESCPTOK  */
    NRTOK = 283,                   /* NRTOK  */
    AMBITOK = 284,                 /* AMBITOK  */
    SWPTTOK = 285,                 /* SWPTTOK  */
    MPTRTOK = 286,                 /* MPTRTOK  */
    GLOBTOK = 287,                 /* GLOBTOK  */
    TONTOK = 288,                  /* TONTOK  */
    TOFFTOK = 289,                 /* TOFFTOK  */
    TALKTOK = 290,                 /* TALKTOK  */
    INACTOK = 291,                 /* INACTOK  */
    FDTOK = 292,                   /* FDTOK  */
    BOXXTOK = 293,                 /* BOXXTOK  */
    ESCAPETOK = 294,               /* ESCAPETOK  */
    SECONDTOK = 295,               /* SECONDTOK  */
    INTROTOK = 296,                /* INTROTOK  */
    DEFAULTTOK = 297,              /* DEFAULTTOK  */
    PG = 298,                      /* PG  */
    PA = 299,                      /* PA  */
    PD = 300,                      /* PD  */
    PH = 301,                      /* PH  */
    PF = 302,                      /* PF  */
    PE = 303,                      /* PE  */
    PP = 304,                      /* PP  */
    PI = 305,                      /* PI  */
    PL = 306,                      /* PL  */
    PS = 307                       /* PS  */
  };
  typedef enum HYPNO_MIS_tokentype HYPNO_MIS_token_kind_t;
#endif

/* Value type.  */
#if ! defined HYPNO_MIS_STYPE && ! defined HYPNO_MIS_STYPE_IS_DECLARED
union HYPNO_MIS_STYPE
{
#line 55 "engines/hypno/grammar_mis.y"

	char *s; /* string value */
	int i;	 /* integer value */

#line 129 "engines/hypno/tokens_mis.h"

};
typedef union HYPNO_MIS_STYPE HYPNO_MIS_STYPE;
# define HYPNO_MIS_STYPE_IS_TRIVIAL 1
# define HYPNO_MIS_STYPE_IS_DECLARED 1
#endif


extern HYPNO_MIS_STYPE HYPNO_MIS_lval;


int HYPNO_MIS_parse (void);


#endif /* !YY_HYPNO_MIS_ENGINES_HYPNO_TOKENS_MIS_H_INCLUDED  */
