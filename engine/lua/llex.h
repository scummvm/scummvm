/*
** $Id$
** Lexical Analizer
** See Copyright Notice in lua.h
*/

#ifndef llex_h
#define llex_h

#include "lobject.h"
#include "lzio.h"


#define FIRST_RESERVED	260

/* maximum length of a reserved word (+1 for terminal 0) */
#define TOKEN_LEN	15

enum RESERVED {
  /* terminal symbols denoted by reserved words */
  AND = FIRST_RESERVED,
  DO, ELSE, ELSEIF, END, FUNCTION, IF, LOCAL, NIL, NOT, OR,
  REPEAT, RETURN, THEN, UNTIL, WHILE,
  /* other terminal symbols */
  NAME, CONC, DOTS, EQ, GE, LE, NE, NUMBER, STRING, EOS};


#define MAX_IFS 5

/* "ifstate" keeps the state of each nested $if the lexical is dealing with. */

struct ifState {
  int32 elsepart;  /* true if its in the $else part */
  int32 condition;  /* true if $if condition is true */
  int32 skip;  /* true if part must be skipped */
};


typedef struct LexState {
  int32 current;  /* look ahead character */
  int32 token;  /* look ahead token */
  struct FuncState *fs;  /* 'FuncState' is private for the parser */
  union {
    real r;
    TaggedString *ts;
  } seminfo;  /* semantics information */
  struct zio *lex_z;  /* input stream */
  int32 linenumber;  /* input line counter */
  int32 iflevel;  /* level of nested $if's (for lexical analysis) */
  struct ifState ifstate[MAX_IFS];
} LexState;


void luaX_init (void);
void luaX_setinput (LexState *LS, ZIO *z);
int32 luaX_lex (LexState *LS);
void luaX_syntaxerror (LexState *ls, const char *s, const char *token);
void luaX_error (LexState *ls, const char *s);
void luaX_token2str (LexState *ls, int32 token, char *s);


#endif
