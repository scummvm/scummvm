/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

/* Token type.  */
#ifndef HYPNO_MIS_TOKENTYPE
# define HYPNO_MIS_TOKENTYPE
  enum HYPNO_MIS_tokentype
  {
    NAME = 258,
    FILENAME = 259,
    FLAG = 260,
    COMMENT = 261,
    GSSWITCH = 262,
    COMMAND = 263,
    NUM = 264,
    HOTSTOK = 265,
    CUTSTOK = 266,
    BACKTOK = 267,
    INTRTOK = 268,
    RETTOK = 269,
    TIMETOK = 270,
    PALETOK = 271,
    BBOXTOK = 272,
    OVERTOK = 273,
    WALNTOK = 274,
    MICETOK = 275,
    PLAYTOK = 276,
    ENDTOK = 277,
    MENUTOK = 278,
    SMENTOK = 279,
    ESCPTOK = 280,
    NRTOK = 281,
    AMBITOK = 282,
    GLOBTOK = 283,
    TONTOK = 284,
    TOFFTOK = 285,
    TALKTOK = 286,
    INACTOK = 287,
    FDTOK = 288,
    BOXXTOK = 289,
    ESCAPETOK = 290,
    SECONDTOK = 291,
    INTROTOK = 292,
    DEFAULTTOK = 293,
    PG = 294,
    PA = 295,
    PD = 296,
    PH = 297,
    PF = 298,
    PE = 299,
    PP = 300,
    PI = 301,
    PL = 302,
    PS = 303
  };
#endif

/* Value type.  */
#if ! defined HYPNO_MIS_STYPE && ! defined HYPNO_MIS_STYPE_IS_DECLARED

union HYPNO_MIS_STYPE
{
#line 57 "engines/hypno/grammar_mis.y" /* yacc.c:1909  */

	char *s; /* string value */
	int i;	 /* integer value */

#line 116 "engines/hypno/tokens_mis.h" /* yacc.c:1909  */
};

typedef union HYPNO_MIS_STYPE HYPNO_MIS_STYPE;
# define HYPNO_MIS_STYPE_IS_TRIVIAL 1
# define HYPNO_MIS_STYPE_IS_DECLARED 1
#endif


extern HYPNO_MIS_STYPE HYPNO_MIS_lval;

int HYPNO_MIS_parse (void);

#endif /* !YY_HYPNO_MIS_ENGINES_HYPNO_TOKENS_MIS_H_INCLUDED  */
