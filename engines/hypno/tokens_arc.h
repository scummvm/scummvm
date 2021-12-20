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
    KNTOK = 262,
    YXTOK = 263,
    NUM = 264,
    COMMENT = 265,
    CTOK = 266,
    DTOK = 267,
    HTOK = 268,
    HETOK = 269,
    RETTOK = 270,
    QTOK = 271,
    ENCTOK = 272,
    PTOK = 273,
    FTOK = 274,
    TTOK = 275,
    TPTOK = 276,
    ATOK = 277,
    VTOK = 278,
    OTOK = 279,
    ONTOK = 280,
    NTOK = 281,
    RTOK = 282,
    R0TOK = 283,
    ITOK = 284,
    JTOK = 285,
    ZTOK = 286,
    FNTOK = 287,
    NONETOK = 288,
    A0TOK = 289,
    P0TOK = 290,
    WTOK = 291,
    XTOK = 292,
    CB3TOK = 293,
    C02TOK = 294
  };
#endif

/* Value type.  */
#if ! defined HYPNO_ARC_STYPE && ! defined HYPNO_ARC_STYPE_IS_DECLARED

union HYPNO_ARC_STYPE
{
#line 54 "engines/hypno/grammar_arc.y" /* yacc.c:1909  */

	char *s; /* string value */
	int i;	 /* integer value */

#line 107 "engines/hypno/tokens_arc.h" /* yacc.c:1909  */
};

typedef union HYPNO_ARC_STYPE HYPNO_ARC_STYPE;
# define HYPNO_ARC_STYPE_IS_TRIVIAL 1
# define HYPNO_ARC_STYPE_IS_DECLARED 1
#endif


extern HYPNO_ARC_STYPE HYPNO_ARC_lval;

int HYPNO_ARC_parse (void);

#endif /* !YY_HYPNO_ARC_ENGINES_HYPNO_TOKENS_ARC_H_INCLUDED  */
