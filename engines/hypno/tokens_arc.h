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

/* Token kinds.  */
#ifndef HYPNO_ARC_TOKENTYPE
# define HYPNO_ARC_TOKENTYPE
  enum HYPNO_ARC_tokentype
  {
    HYPNO_ARC_EMPTY = -2,
    HYPNO_ARC_EOF = 0,             /* "end of file"  */
    HYPNO_ARC_error = 256,         /* error  */
    HYPNO_ARC_UNDEF = 257,         /* "invalid token"  */
    NAME = 258,                    /* NAME  */
    FILENAME = 259,                /* FILENAME  */
    BNTOK = 260,                   /* BNTOK  */
    SNTOK = 261,                   /* SNTOK  */
    KNTOK = 262,                   /* KNTOK  */
    YXTOK = 263,                   /* YXTOK  */
    FNTOK = 264,                   /* FNTOK  */
    ENCTOK = 265,                  /* ENCTOK  */
    ONTOK = 266,                   /* ONTOK  */
    NUM = 267,                     /* NUM  */
    BYTE = 268,                    /* BYTE  */
    COMMENT = 269,                 /* COMMENT  */
    ALTOK = 270,                   /* ALTOK  */
    AVTOK = 271,                   /* AVTOK  */
    ABTOK = 272,                   /* ABTOK  */
    CTOK = 273,                    /* CTOK  */
    DTOK = 274,                    /* DTOK  */
    HTOK = 275,                    /* HTOK  */
    HETOK = 276,                   /* HETOK  */
    HLTOK = 277,                   /* HLTOK  */
    H12TOK = 278,                  /* H12TOK  */
    HUTOK = 279,                   /* HUTOK  */
    RETTOK = 280,                  /* RETTOK  */
    QTOK = 281,                    /* QTOK  */
    RESTOK = 282,                  /* RESTOK  */
    PTOK = 283,                    /* PTOK  */
    FTOK = 284,                    /* FTOK  */
    TTOK = 285,                    /* TTOK  */
    TATOK = 286,                   /* TATOK  */
    TPTOK = 287,                   /* TPTOK  */
    ATOK = 288,                    /* ATOK  */
    VTOK = 289,                    /* VTOK  */
    OTOK = 290,                    /* OTOK  */
    LTOK = 291,                    /* LTOK  */
    MTOK = 292,                    /* MTOK  */
    NTOK = 293,                    /* NTOK  */
    NSTOK = 294,                   /* NSTOK  */
    RTOK = 295,                    /* RTOK  */
    R01TOK = 296,                  /* R01TOK  */
    ITOK = 297,                    /* ITOK  */
    I1TOK = 298,                   /* I1TOK  */
    GTOK = 299,                    /* GTOK  */
    JTOK = 300,                    /* JTOK  */
    J0TOK = 301,                   /* J0TOK  */
    KTOK = 302,                    /* KTOK  */
    UTOK = 303,                    /* UTOK  */
    ZTOK = 304,                    /* ZTOK  */
    NONETOK = 305,                 /* NONETOK  */
    A0TOK = 306,                   /* A0TOK  */
    P0TOK = 307,                   /* P0TOK  */
    WTOK = 308,                    /* WTOK  */
    XTOK = 309,                    /* XTOK  */
    CB3TOK = 310,                  /* CB3TOK  */
    C02TOK = 311                   /* C02TOK  */
  };
  typedef enum HYPNO_ARC_tokentype HYPNO_ARC_token_kind_t;
#endif

/* Value type.  */
#if ! defined HYPNO_ARC_STYPE && ! defined HYPNO_ARC_STYPE_IS_DECLARED
union HYPNO_ARC_STYPE
{
#line 53 "engines/hypno/grammar_arc.y"

	char *s; /* string value */
	int i;	 /* integer value */

#line 133 "engines/hypno/tokens_arc.h"

};
typedef union HYPNO_ARC_STYPE HYPNO_ARC_STYPE;
# define HYPNO_ARC_STYPE_IS_TRIVIAL 1
# define HYPNO_ARC_STYPE_IS_DECLARED 1
#endif


extern HYPNO_ARC_STYPE HYPNO_ARC_lval;


int HYPNO_ARC_parse (void);


#endif /* !YY_HYPNO_ARC_ENGINES_HYPNO_TOKENS_ARC_H_INCLUDED  */
