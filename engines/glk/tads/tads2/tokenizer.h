/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GLK_TADS_TADS2_TOKENIZER
#define GLK_TADS_TADS2_TOKENIZER

#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/error_handling.h"
#include "glk/tads/tads2/line_source.h"
#include "glk/tads/tads2/line_source_file.h"
#include "glk/tads/tads2/memory_cache.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* number of entries in hash table - must be power of 2 */
#define TOKHASHSIZE 256

/* symbol types */
#define TOKSTUNK      0                  /* unknown symbol, not yet defined */
#define TOKSTFUNC     1                 /* function; value is object number */
#define TOKSTOBJ      2                   /* object; value is object number */
#define TOKSTPROP     3               /* property; value is property number */
#define TOKSTLOCAL    4             /* a local variable or formal parameter */
#define TOKSTSELF     5                         /* the pseudo-object "self" */
#define TOKSTBIFN     6                              /* a built-in function */
#define TOKSTFWDOBJ   7                        /* forward-referenced object */
#define TOKSTFWDFN    8                        /* forward-referenced object */
#define TOKSTINHERIT  9                    /* the pseudo-object "inherited" */
#define TOKSTEXTERN  10                             /* an external function */
#define TOKSTKW      11                   /* keyword; value is token number */
#define TOKSTLABEL   12                                  /* statement label */
#define TOKSTARGC    13                       /* 'argcount' pseudo-variable */
#define TOKSTPROPSPEC 14                 /* speculative evaluation property */

/* token types */
#define TOKTEOF     1

/* binary operators - keep these together (see prsbopl[] in prs.c) */
#define TOKTPLUS    2
#define TOKTMINUS   3
#define TOKTDIV     4
#define TOKTTIMES   5
#define TOKTNOT     6                                         /* ! or "not" */
#define TOKTEQ      7
#define TOKTNE      8
#define TOKTGT      9
#define TOKTGE      10
#define TOKTLT      11
#define TOKTLE      12
#define TOKTMOD     13
#define TOKTBAND    14
#define TOKTBOR     15
#define TOKTXOR     16
#define TOKTSHL     17
#define TOKTSHR     18
#define TOKTTILDE   30

/* 
 *   special 'dot' replacement for speculative evaluation mode -- this is
 *   strictly for marking parse tree nodes, and has the same meaning in a
 *   parse tree node as a regular TOKTDOT, but generates code that can't
 *   call methods 
 */
#define TOKTDOTSPEC 31

/* special node marker for explicit superclass inheritance nodes */
#define TOKTEXPINH  32

#define TOKTLPAR    50                                                 /* ( */
#define TOKTRPAR    51                                                 /* ) */
#define TOKTCOLON   52
#define TOKTDSTRING 53                           /* string in double quotes */
#define TOKTSSTRING 54                           /* string in single quotes */
#define TOKTNUMBER  55
#define TOKTSYMBOL  56
#define TOKTINVALID 57                             /* invalid lexical token */
#define TOKTLBRACK  58                                                 /* [ */
#define TOKTRBRACK  59                                                 /* ] */
#define TOKTLBRACE  60                                                 /* { */
#define TOKTRBRACE  61                                                 /* } */
#define TOKTSEM     62                                                 /* ; */
#define TOKTCOMMA   63
#define TOKTDOT     64                                                 /* . */
#define TOKTOR      65                                         /* | or "if" */
#define TOKTAND     66                                        /* & or "and" */
#define TOKTIF      67                                          /* keywords */
#define TOKTELSE    68
#define TOKTWHILE   69
#define TOKTFUNCTION 70
#define TOKTRETURN  71
#define TOKTLOCAL   72
#define TOKTOBJECT  73
#define TOKTBREAK   74
#define TOKTCONTINUE 75
#define TOKTLIST    76                                            /* a list */
#define TOKTNIL     77
#define TOKTTRUE    78
#define TOKTPASS    79
#define TOKTCLASS   80
#define TOKTEXIT    81
#define TOKTABORT   82
#define TOKTASKDO   83
#define TOKTASKIO   84
#define TOKTPOUND   85                                                 /* # */
#define TOKTQUESTION 86                                                /* ? */
#define TOKTCOMPOUND 87
#define TOKTIOSYN   88
#define TOKTDOSYN   89
#define TOKTEXTERN  90
#define TOKTFORMAT  91
#define TOKTDO      92
#define TOKTFOR     93
#define TOKTNEW     94
#define TOKTDELETE  95

/* assignment operators - keep these together */
#define TOKTINC     150                                               /* ++ */
#define TOKTPOSTINC 151                              /* MUST BE TOKTINC + 1 */
#define TOKTDEC     152                                               /* -- */
#define TOKTPOSTDEC 153                              /* MUST BE TOKTDEC + 1 */
#define TOKTPLEQ    154                                               /* += */
#define TOKTMINEQ   155                                               /* -= */
#define TOKTDIVEQ   156                                               /* /= */
#define TOKTTIMEQ   157                                               /* *= */
#define TOKTASSIGN  158                                /* simple assignment */
#define TOKTMODEQ   159                     /* %= (mod and assign) operator */
#define TOKTBANDEQ  160                                               /* &= */
#define TOKTBOREQ   161                                               /* |= */
#define TOKTXOREQ   162                              /* ^= (xor and assign) */
#define TOKTSHLEQ   163                      /* <<= (shift left and assign) */
#define TOKTSHREQ   164                      /* >>= (shift right and assign */

#define TOKTSWITCH  200
#define TOKTCASE    201
#define TOKTDEFAULT 202
#define TOKTGOTO    203
#define TOKTELLIPSIS 204                                             /* ... */
#define TOKTSPECIAL 205                                   /* "specialWords" */
#define TOKTREPLACE 206                                          /* replace */
#define TOKTMODIFY  207                                           /* modify */

#define TOKTEQEQ    208                                /* the '==' operator */
#define TOKTPOINTER 209                                  /* the -> operator */

/* the longest a symbol name can be */
#define TOKNAMMAX 39

/* symbol table entry */
struct toksdef {
    uchar    tokstyp;                                 /* type of the symbol */
    uchar    tokshsh;                               /* hash value of symbol */
    ushort   toksval;              /* value of the symbol (depends on type) */
    ushort   toksfr;               /* frame offset of symbol (for debugger) */
    uchar    tokslen;                        /* length of the symbol's name */
    char     toksnam[TOKNAMMAX];                          /* name of symbol */
};

/* symbol table entry without 'name' portion - for allocation purposes */
struct toks1def {
    uchar    tokstyp;
    uchar    tokshsh;
    ushort   toksval;
    ushort   toksfr;
    uchar    tokslen;
    char     toksnam[1];
};

/* generic symbol table object - other symbol tables are subclasses */
struct toktdef {
    void     (*toktfadd)(toktdef *tab, char *name, int namel, int typ,
                         int val, int hash);                  /* add symbol */
    int      (*toktfsea)(toktdef *tab, char *name, int namel, int hash,
                         toksdef *ret);              /* search symbol table */
    void     (*toktfset)(toktdef *tab, toksdef *sym);
                             /* update val & typ of symbol to those in *sym */
    void     (*toktfeach)(toktdef *tab,
                          void (*fn)(void *ctx, toksdef *sym),
                          void *fnctx);            /* call fn for each sym */
    toktdef   *toktnxt;                 /* next symbol table to be searched */
    errcxdef  *tokterr;                           /* error handling context */
};

/* maximum number of pools (TOKTSIZE bytes each) for symbols */
#define TOKPOOLMAX 128

/* pointer to a symbol in a hashed symbol table */
struct tokthpdef {
    mcmon  tokthpobj;                /* cache manager object number of page */
    uint   tokthpofs;                  /* offset within page of this symbol */
};

/* extended symbol entry in a hashed symbol table */
struct tokshdef {
    tokthpdef tokshnxt;              /* pointer to next symbol in the table */
    toksdef   tokshsc;                  /* superclass - normal symbol entry */
};

/* hashing symbol table (subclass of generic symbol table) */
struct tokthdef {
    toktdef   tokthsc;              /* generic symbol table superclass data */
    mcmcxdef *tokthmem;                           /* memory manager context */
    tokthpdef tokthhsh[TOKHASHSIZE];                          /* hash table */
    uint      tokthpcnt;            /* number of memory pools for toksdef's */
    mcmon     tokthpool[TOKPOOLMAX];          /* memory pools for toksdef's */
    uint      tokthfinal[TOKPOOLMAX];        /* actual sizes of these pools */
    uchar    *tokthcpool;                           /* current pool pointer */
    ushort    tokthsize;               /* remaining size of top memory pool */
    ushort    tokthofs;             /* allocation offset in top memory pool */
};

/* size of toksdef pools to allocate for hashed symbol tables */
#define TOKTHSIZE 4096

/*
 *   Linear cache-object-embedded symbol table.  This type of symbol
 *   table is used for frame parameter/local variable lists.  It is best
 *   for small tables, because it isn't broken up into hash buckets, so it
 *   is searched linearly.  As a result, it's small enough to be embedded
 *   in code.
 */
struct toktldef {
    toktdef   toktlsc;              /* generic symbol table superclass data */
    uchar    *toktlptr;                      /* base of linear symbol table */
    uchar    *toktlnxt;                          /* next free byte in table */
    uint      toktlcnt;                   /* number of objects in the table */
    uint      toktlsiz;                     /* bytes remaining in the table */
};

struct tokdef {
    int      toktyp;                                   /* type of the token */
    int      toklen;           /* length of token text, if a symbolic token */
    long     tokval;                        /* numeric value, if applicable */
    ushort   tokofs;
    uint     tokhash;              /* token hash value, if a symbolic token */
    char     toknam[TOKNAMMAX+1];     /* text of token, if a symbolic token */
    toksdef  toksym;                    /* symbol from table matching token */
};

/* special character sequence */
#define TOKSCMAX  3            /* maximum length of a special char sequence */
struct tokscdef {
    tokscdef *tokscnxt;          /* next sequence with same first character */
    int       toksctyp;             /* token type corresponding to sequence */
    int       toksclen;                           /* length of the sequence */
    char      tokscstr[TOKSCMAX+1];                  /* the sequence itself */
};

/*
 *   Compare a special character sequence - for efficiency, define
 *   something special for the maximum length available (TOKSCMAX).
 *   Note that the first character will always be equal, or the
 *   string wouldn't even get to the point of being tested by this
 *   macro.
 */
#if TOKSCMAX == 3
# define toksceq(str1, str2, len1, len2) \
    ((len2) >= (len1) \
     && ((len1) == 1 \
         || ((str1)[1] == (str2)[1] \
             && ((len1) == 2 \
                 || (str1)[2] == (str2)[2]))))
#endif /* TOKSCMAX == 3 */
#ifndef toksceq
# define toksceq(str1, str2, len) (!memcmp(str1, str2, (size_t)(len)))
#endif /* toksceq */

/* special character sequence list table entry */
struct tokldef {
    int  tokltyp;                   /* token type corresponding to sequence */
    char toklstr[TOKSCMAX+1];                   /* the text of the sequence */
};

/* include path structure */
struct tokpdef {
    tokpdef *tokpnxt;                                  /* next path in list */
    int      tokplen;                           /* length of directory name */
    char     tokpdir[1];                             /* directory to search */
};

/* #define symbol structure */
struct tokdfdef {
    tokdfdef *nxt;                    /* next symbol in the same hash chain */
    char     *nm;                                     /* name of the symbol */
    int       len;                                  /* length of the symbol */
    int       explen;                            /* length of the expansion */
    char      expan[1];                                 /* expansion buffer */
};

/* #define hash table information */
#define TOKDFHSHSIZ   64
#define TOKDFHSHMASK  63

/* maximum #if nesting */
#define TOKIFNEST     64

/* #if state */
#define TOKIF_IF_YES    1             /* processing a true #if/#ifdef block */
#define TOKIF_IF_NO     2            /* processing a false #if/#ifdef block */
#define TOKIF_ELSE_YES  3                   /* processing a true #else part */
#define TOKIF_ELSE_NO   4                  /* processing a false #else part */

/* maximum macro expansion nesting */
#define TOKMACNEST  20

/* lexical analysis context */
struct tokcxdef {
    errcxdef *tokcxerr;                           /* error handling context */
    mcmcxdef *tokcxmem;                            /* cache manager context */
    struct    dbgcxdef *tokcxdbg;                       /* debugger context */
    lindef   *tokcxlin;                                      /* line source */
    tokpdef  *tokcxinc;                        /* head of include path list */
    toktdef  *tokcxstab;              /* current head of symbol table chain */
    void     *tokcxscx;    /* context for string storage callback functions */
    ushort  (*tokcxsst)(void *ctx);
               /* start storing a string; return offset of string's storage */
    void    (*tokcxsad)(void *ctx, const char *str, ushort len);
                                              /* add characters to a string */
    void    (*tokcxsend)(void *ctx);               /* finish storing string */
    const char *tokcxmsav[TOKMACNEST]; /* saved positions for macro expansion */
    ushort    tokcxmsvl[TOKMACNEST];   /* saved lengths for macro expansion */
    int       tokcxmlvl;                             /* macro nesting level */
    int       tokcxflg;                                            /* flags */
#   define    TOKCXFINMAC    0x01         /* doing <<expr>> macro expansion */
#   define    TOKCXCASEFOLD  0x02              /* fold upper and lower case */
#   define    TOKCXFCMODE    0x04                /* parse using C operators */
#   define    TOKCXF_EMBED_PAREN_PRE 0x08        /* embedded expr - did '(' */
#   define    TOKCXF_EMBED_PAREN_AFT 0x10    /* embedded expr - must do ')' */
#   define    TOKCXFLIN2     0x20                 /* new-style line records */
    tokdef    tokcxcur;                                    /* current token */
    char     *tokcxbuf;                            /* buffer for long lines */
    ushort    tokcxbsz;                         /* size of long line buffer */
    const char *tokcxptr;                       /* pointer into line source */
    ushort    tokcxlen;                         /* length of text in buffer */
    uchar     tokcxinx[256];                   /* special character indices */
    tokdfdef *tokcxdf[TOKDFHSHSIZ];       /* hash table for #define symbols */
    int       tokcxifcnt;           /* number of #endif's we expect to find */
    char      tokcxif[TOKIFNEST];       /* #if state for each nesting level */
    int       tokcxifcur;             /* current #if state, obeying nesting */
    linfdef  *tokcxhdr;              /* list of previously included headers */
    tokscdef *tokcxsc[1];                        /* special character table */
};


/* allocate and initialize a lexical analysis context */
tokcxdef *tokcxini(errcxdef *errctx, mcmcxdef *mctx, tokldef *sctab);

/* add an include path to a token handling context */
void tokaddinc(tokcxdef *ctx, char *path, int pathlen);

/* compute the hash value of a string */
uint tokhsh(char *nam);

/* 
 *   Fold case of a token if we're in case-insensitive mode.  This should
 *   be called any time a token is constructed artificially; it need not
 *   be used the token is read through the tokenizer, because the
 *   tokenizer will always adjust a token as needed before returning it. 
 */
void tok_case_fold(tokcxdef *ctx, tokdef *tok);

/* initialize a hashed symbol table */
void tokthini(errcxdef *errctx, mcmcxdef *memctx, toktdef *toktab1);

/* add a symbol to a hashed symbol table */
void tokthadd(toktdef *toktab, char *name, int namel,
              int typ, int val, int hash);

/* update a symbol in a hashed symbol table */
void tokthset(toktdef *toktab, toksdef *sym);

/* search a hashed symbol table for a symbol */
int tokthsea(toktdef *tab, char *name, int namel, int hash,
             toksdef *ret);

/* call a function for each symbol in a hashed symbol table */
void toktheach(toktdef *tab, void (*cb)(void *ctx, toksdef *sym),
               void *ctx);

/* find a symbol given type and value */
int tokthfind(toktdef *tab, int typ, uint val, toksdef *sym);

/* initialize a linear symbol table */
void toktlini(errcxdef *errctx, toktldef *toktab,
              uchar *mem, uint siz);

/* add a symbol to a linear symbol table */
void toktladd(toktdef *toktab, char *name, int namel,
              int typ, int val, int hash);
                
/* search a linear symbol table */
int toktlsea(toktdef *tab, char *name, int namel, int hash,
             toksdef *ret);

/* update a symbol in a linear symbol table */
void toktlset(toktdef *toktab, toksdef *sym);

/* call a function for each symbol in a local symbol table */
void toktleach(toktdef *tab, void (*cb)(void *ctx, toksdef *sym),
               void *ctx);

/* delete all symbols from a linear table */
void toktldel(toktldef *tab);

/* get next token, removing it from input stream */
int toknext(tokcxdef *ctx);

/* general function to get/peek at next token */
int tokget1(tokcxdef *ctx, tokdef *tok, int consume);

/* add a symbol to the #define symbol table */
void tok_add_define(tokcxdef *ctx, const char *sym, int len,
                    const char *expan, int explen);

/* 
 *   add a symbol to the #define symbol table, folding case if we're
 *   operating in case-insensitive mode 
 */
void tok_add_define_cvtcase(tokcxdef *ctx, const char *sym, int len,
                            const char *expan, int explen);

/* add a symbol to the #define symbol table as a number */
void tok_add_define_num_cvtcase(tokcxdef *ctx, const char *sym, int len, int num);

/* undefine a #define symbol */
void tok_del_define(tokcxdef *ctx, char *sym, int len);

/* read/write preprocessor symbols from/to a file */
void tok_read_defines(tokcxdef *ctx, osfildef *fp, errcxdef *ec);

/* write preprocessor state to a file */
void tok_write_defines(tokcxdef *ctx, osfildef *fp, errcxdef *ec);


/* determine if a char is a valid non-initial character in a symbol name */
#define TOKISSYM(c) \
    (Common::isAlpha((uchar)(c)) || Common::isDigit((uchar)(c)) || (c)=='_' || (c)=='$')

/* numeric conversion and checking macros */
#define TOKISHEX(c) \
    (Common::isDigit((uchar)(c))||((c)>='a'&&(c)<='f')||((c)>='A'&&(c)<='F'))
#define TOKISOCT(c) \
    (Common::isDigit((uchar)(c))&&!((c)=='8'||(c)=='9'))

#define TOKHEX2INT(c) \
    (Common::isDigit((uchar)c)?(c)-'0':((c)>='a'?(c)-'a'+10:(c)-'A'+10))
#define TOKOCT2INT(c) ((c)-'0')
#define TOKDEC2INT(c) ((c)-'0')

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
