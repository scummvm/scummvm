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

#ifndef YY_HYPNO_ARC_ENGINES_HYPNO_TOKENS_ARC_H_INCLUDED
# define YY_HYPNO_ARC_ENGINES_HYPNO_TOKENS_ARC_H_INCLUDED
/* Debug traces.  */
#ifndef HYPNO_ARC_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define HYPNO_ARC_DEBUG 1
#  else
#   define HYPNO_ARC_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define HYPNO_ARC_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined HYPNO_ARC_DEBUG */
#if HYPNO_ARC_DEBUG
extern int HYPNO_ARC_debug;
#endif

/* Token type.  */
#ifndef HYPNO_ARC_TOKENTYPE
# define HYPNO_ARC_TOKENTYPE
  enum HYPNO_ARC_tokentype
  {
    NAME = 258,
    FILENAME = 259,
    BNTOK = 260,
    SNTOK = 261,
    NUM = 262,
    YXTOK = 263,
    CTOK = 264,
    DTOK = 265,
    HTOK = 266,
    HETOK = 267,
    RETTOK = 268,
    QTOK = 269,
    ENCTOK = 270,
    PTOK = 271,
    FTOK = 272,
    TTOK = 273,
    TPTOK = 274,
    ATOK = 275,
    VTOK = 276,
    OTOK = 277,
    O1TOK = 278,
    NTOK = 279,
    RTOK = 280,
    ITOK = 281,
    ZTOK = 282,
    FNTOK = 283,
    NONETOK = 284,
    A0TOK = 285,
    K0TOK = 286,
    P0TOK = 287,
    WTOK = 288,
    XTOK = 289,
    CB3TOK = 290,
    C02TOK = 291
  };
#endif

/* Value type.  */
#if ! defined HYPNO_ARC_STYPE && ! defined HYPNO_ARC_STYPE_IS_DECLARED

union HYPNO_ARC_STYPE
{
#line 55 "engines/hypno/grammar_arc.y" /* yacc.c:1909  */

	char *s; /* string value */
	int i;	 /* integer value */

#line 104 "engines/hypno/tokens_arc.h" /* yacc.c:1909  */
};

typedef union HYPNO_ARC_STYPE HYPNO_ARC_STYPE;
# define HYPNO_ARC_STYPE_IS_TRIVIAL 1
# define HYPNO_ARC_STYPE_IS_DECLARED 1
#endif


extern HYPNO_ARC_STYPE HYPNO_ARC_lval;

int HYPNO_ARC_parse (void);

#endif /* !YY_HYPNO_ARC_ENGINES_HYPNO_TOKENS_ARC_H_INCLUDED  */
