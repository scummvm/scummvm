/* A Bison parser, made by GNU Bison 3.7.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_PRIVATE_ENGINES_PRIVATE_TOKENS_H_INCLUDED
# define YY_PRIVATE_ENGINES_PRIVATE_TOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef PRIVATE_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define PRIVATE_DEBUG 1
#  else
#   define PRIVATE_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define PRIVATE_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined PRIVATE_DEBUG */
#if PRIVATE_DEBUG
extern int PRIVATE_debug;
#endif

/* Token kinds.  */
#ifndef PRIVATE_TOKENTYPE
# define PRIVATE_TOKENTYPE
  enum PRIVATE_tokentype
  {
    PRIVATE_EMPTY = -2,
    PRIVATE_EOF = 0,               /* "end of file"  */
    PRIVATE_error = 256,           /* error  */
    PRIVATE_UNDEF = 257,           /* "invalid token"  */
    NAME = 258,                    /* NAME  */
    STRING = 259,                  /* STRING  */
    NUM = 260,                     /* NUM  */
    LTE = 261,                     /* LTE  */
    GTE = 262,                     /* GTE  */
    NEQ = 263,                     /* NEQ  */
    EQ = 264,                      /* EQ  */
    FALSETOK = 265,                /* FALSETOK  */
    TRUETOK = 266,                 /* TRUETOK  */
    NULLTOK = 267,                 /* NULLTOK  */
    IFTOK = 268,                   /* IFTOK  */
    ELSETOK = 269,                 /* ELSETOK  */
    RECT = 270,                    /* RECT  */
    GOTOTOK = 271,                 /* GOTOTOK  */
    DEBUGTOK = 272,                /* DEBUGTOK  */
    DEFINETOK = 273,               /* DEFINETOK  */
    SETTINGTOK = 274,              /* SETTINGTOK  */
    RANDOMTOK = 275                /* RANDOMTOK  */
  };
  typedef enum PRIVATE_tokentype PRIVATE_token_kind_t;
#endif

/* Value type.  */
#if ! defined PRIVATE_STYPE && ! defined PRIVATE_STYPE_IS_DECLARED
union PRIVATE_STYPE
{
#line 80 "engines/private/grammar.y"

        Private::Symbol *sym; /* symbol table pointer */
        int (**inst)();       /* machine instruction */
        char *s;              /* string value */
        int *i;               /* integer value */
        int narg;             /* auxiliary value to count function arguments */

#line 100 "engines/private/tokens.h"

};
typedef union PRIVATE_STYPE PRIVATE_STYPE;
# define PRIVATE_STYPE_IS_TRIVIAL 1
# define PRIVATE_STYPE_IS_DECLARED 1
#endif


extern PRIVATE_STYPE PRIVATE_lval;

int PRIVATE_parse (void);

#endif /* !YY_PRIVATE_ENGINES_PRIVATE_TOKENS_H_INCLUDED  */
