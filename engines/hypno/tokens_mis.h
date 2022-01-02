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
    WALNTOK = 264,
    NUM = 265,
    HOTSTOK = 266,
    CUTSTOK = 267,
    BACKTOK = 268,
    INTRTOK = 269,
    RETTOK = 270,
    TIMETOK = 271,
    PALETOK = 272,
    BBOXTOK = 273,
    OVERTOK = 274,
    MICETOK = 275,
    PLAYTOK = 276,
    ENDTOK = 277,
    MENUTOK = 278,
    SMENTOK = 279,
    ESCPTOK = 280,
    NRTOK = 281,
    AMBITOK = 282,
    SWPTTOK = 283,
    MPTRTOK = 284,
    GLOBTOK = 285,
    TONTOK = 286,
    TOFFTOK = 287,
    TALKTOK = 288,
    INACTOK = 289,
    FDTOK = 290,
    BOXXTOK = 291,
    ESCAPETOK = 292,
    SECONDTOK = 293,
    INTROTOK = 294,
    DEFAULTTOK = 295,
    PG = 296,
    PA = 297,
    PD = 298,
    PH = 299,
    PF = 300,
    PE = 301,
    PP = 302,
    PI = 303,
    PL = 304,
    PS = 305
  };
#endif

/* Value type.  */
#if ! defined HYPNO_MIS_STYPE && ! defined HYPNO_MIS_STYPE_IS_DECLARED

union HYPNO_MIS_STYPE
{
#line 56 "engines/hypno/grammar_mis.y" /* yacc.c:1909  */

	char *s; /* string value */
	int i;	 /* integer value */

#line 118 "engines/hypno/tokens_mis.h" /* yacc.c:1909  */
};

typedef union HYPNO_MIS_STYPE HYPNO_MIS_STYPE;
# define HYPNO_MIS_STYPE_IS_TRIVIAL 1
# define HYPNO_MIS_STYPE_IS_DECLARED 1
#endif


extern HYPNO_MIS_STYPE HYPNO_MIS_lval;

int HYPNO_MIS_parse (void);

#endif /* !YY_HYPNO_MIS_ENGINES_HYPNO_TOKENS_MIS_H_INCLUDED  */
