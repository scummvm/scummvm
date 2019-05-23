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

/* Definitions for code execution
 *
 * The preprocessor symbol RUNFAST can be defined if run - time checking
 * of stack overflow, stack underflow, and other unusual but potentially
 * dangerous conditions is to be turned off.This will result in somewhat
 * faster run-time performance, but run - time errors could be disastrous.
 */

#ifndef GLK_TADS_TADS2_RUN
#define GLK_TADS_TADS2_RUN

#include "common/scummsys.h"
#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/debug.h"
#include "glk/tads/tads2/object.h"
#include "glk/tads/tads2/memory_cache.h"
#include "glk/tads/tads2/memory_cache_swap.h"
#include "glk/tads/tads2/opcode.h"
#include "glk/tads/tads2/property.h"
#include "glk/tads/tads2/text_io.h"
#include "glk/tads/tads2/tokenizer.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* forward declarations */
struct bifcxdef;

/* stack element - the stack is an array of these structures */
struct runsdef {
    uchar  runstyp;                                      /* type of element */
    union {
        long    runsvnum;                                  /* numeric value */
        objnum  runsvobj;                                   /* object value */
        prpnum  runsvprp;                          /* property number value */
        uchar  *runsvstr;                              /* string/list value */
	} runsv;

	runsdef() : runstyp(0) {
		runsv.runsvnum = 0;
	}
};

/* external function control structure */
struct runxdef {
    char    runxnam[TOKNAMMAX + 1];            /* name of external function */
    int   (*runxptr)(void *);          /* pointer to memory containing code */
};

/* external function context structure - passed to user exits */
struct runuxdef {
    struct runcxdef osfar_t *runuxctx;                  /* run-time context */
    struct runufdef osfar_t *runuxvec;               /* vector of functions */
    int                      runuxargc;   /* count of arguments to function */
};

/* external function callback vector */
struct runufdef {
    int    (osfar_t *runuftyp)(runuxdef *);         /* type of top of stack */
    long   (osfar_t *runufnpo)(runuxdef *);                 /* pop a number */
    uchar *(osfar_t *runufspo)(runuxdef *);                 /* pop a string */
    void   (osfar_t *runufdsc)(runuxdef *); /* discard item at top of stack */
    void   (osfar_t *runufnpu)(runuxdef *, long);          /* push a number */
    void   (osfar_t *runufspu)(runuxdef *, uchar *); /* push alloc'd string */
    void   (osfar_t *runufcspu)(runuxdef *, char *);     /* push a C-string */
    uchar *(osfar_t *runufsal)(runuxdef *, int);   /* allocate a new string */
    void   (osfar_t *runuflpu)(runuxdef *, int);/* push DAT_TRUE or DAT_NIL */
};

/* execution context */
struct runcxdef {
    errcxdef   *runcxerr;                       /* error management context */
    mcmcxdef   *runcxmem;    /* cache manager context for object references */
    runsdef    *runcxstk;                      /* base of interpreter stack */
    runsdef    *runcxstop;                                  /* top of stack */
    runsdef    *runcxsp;     /* current stack pointer (stack grows upwards) */
    runsdef    *runcxbp;                                    /* base pointer */
    uchar      *runcxheap;          /* run-time variable-length object heap */
    uchar      *runcxhp;                            /* current heap pointer */
    uchar      *runcxhtop;                                   /* top of heap */
    objucxdef  *runcxundo;                                  /* undo context */
    tiocxdef   *runcxtio;                               /* text I/O context */
    void       *runcxbcx;        /* context for built-in callback functions */
    void     (**runcxbi)(struct bifcxdef *ctx, int argc);
                                                      /* built-in functions */
    struct dbgcxdef *runcxdbg;                          /* debugger context */
    struct voccxdef *runcxvoc;             /* player command parser context */
    void      (*runcxdmd)(void *ctx, objnum obj, prpnum prp);
                                         /* demand-loader callback function */
    void       *runcxdmc;                 /* demand-loader callback context */
    runxdef    *runcxext;                        /* external function array */
    int         runcxexc;                    /* count of external functions */
    uint        runcxlofs;        /* offset of last line record encountered */
    char       *runcxgamename;                     /* name of the .GAM file */
    char       *runcxgamepath;      /* absolute directory path of .GAM file */
};

/* execute a function, given the function object number */
void runfn(runcxdef *ctx, noreg objnum objn, int argc);

/*
 *   Execute p-code given a pointer to the code.  p is the actual pointer
 *   to the first byte of code to be executed. self is the object to be
 *   used for the special 'self' pseudo-object, and target is the object
 *   whose data are actually being executed.  targprop is the property being
 *   executed; 0 is used for functions. 
 */
void runexe(runcxdef *ctx, uchar *p, objnum self, objnum target,
            prpnum targprop, int argc);

/* push a value onto the stack */
void runpush(runcxdef *ctx, dattyp typ, runsdef *val);

/* push a value onto the stack that's already in the heap */
void runrepush(runcxdef *ctx, runsdef *val);

/* push a number onto the stack */
void runpnum(runcxdef *ctx, long val);

/* push an object onto the stack */
void runpobj(runcxdef *ctx, objnum obj);

/* push nil */
void runpnil(runcxdef *ctx);

/* push a value onto the stack from a buffer (propdef, list) */
void runpbuf(runcxdef *ctx, int typ, void *val);

/* push a counted-length string onto the stack */
void runpstr(runcxdef *ctx, const char *str, int len, int sav);

/*
 *   Push a C-style string onto the stack, converting escape codes.  If
 *   the character contains backslashes, newline, or tab characters, we'll
 *   convert these characters to their escaped equivalent.
 */
void runpushcstr(runcxdef *ctx, const char *str, size_t len, int sav);

/*
 *   Push a property onto the stack.  codepp is a pointer to the caller's
 *   code pointer, which will be updated if necessary; callobj and
 *   callofsp are the object and starting offset within the object of the
 *   code being executed by the caller, which are needed to update
 *   *codepp.  Property 0 is used if a function is being executed.  obj
 *   and prop are the object and property number whose value is to be
 *   pushed.  If 'inh' is TRUE, it means that only a property inherited
 *   by 'obj' is to be considered; this is used for "pass"/"inherited"
 *   operations, with the current target object given as 'obj'.
 */
void runpprop(runcxdef *ctx, uchar *noreg *codepp, objnum callobj,
              prpnum callprop, noreg objnum obj, prpnum prop, int inh,
              int argc, objnum self);

/* top level runpprop, when caller is not executing in an object */
/* void runppr(runcxdef *ctx, objnum obj, prpnum prp, int argc); */
#define runppr(ctx, obj, prp, argc) \
 runpprop(ctx, (uchar **)0, (objnum)0, (prpnum)0, obj, prp, FALSE, argc, obj)

/* discard top element on stack */
/* void rundisc(runcxdef *ctx); */
#define rundisc(ctx) (runstkund(ctx), (--((ctx)->runcxsp)))

/* pop the top element on the stack */
/* void runpop(runcxdef *ctx, runsdef *val); */
#define runpop(ctx, v) \
 (runstkund(ctx), memcpy(v, (--((ctx)->runcxsp)), (size_t)sizeof(runsdef)))

/* pop a numeric value, signalling an error if not a number */
/* long runpopnum(runcxdef *ctx); */
#define runpopnum(ctx) \
 (runstkund(ctx), ((--((ctx)->runcxsp))->runstyp!=DAT_NUMBER ? \
  (runsig(ctx,ERR_REQNUM), (long)0) : \
  ((ctx)->runcxsp->runsv.runsvnum)))

/* pop an object, signalling an error if not an object */
/* objnum runpopobj(runcxdef *ctx); */
#define runpopobj(ctx) \
 (runstkund(ctx), ((--(ctx)->runcxsp))->runstyp!=DAT_OBJECT ? \
  (runsig(ctx,ERR_REQVOB), (objnum)0) : \
  ((ctx)->runcxsp->runsv.runsvobj))

/* pop an object or nil - returns MCMONINV if the value is nil */
#define runpopobjnil(ctx) \
  (runstkund(ctx), ((--(ctx)->runcxsp))->runstyp==DAT_OBJECT ? \
   ((ctx)->runcxsp->runsv.runsvobj) : \
   ((ctx)->runcxsp->runstyp==DAT_NIL ? MCMONINV : \
    (runsig(ctx,ERR_REQVOB), (objnum)0)))

/* pop a list, signalling an error if not a list */
/* uchar *runpoplst(runcxdef *ctx); */
#define runpoplst(ctx) \
 (runstkund(ctx), ((--(ctx)->runcxsp))->runstyp!=DAT_LIST ? \
  (runsig(ctx,ERR_REQVLS), (uchar *)0) : \
  (uchar *)((ctx)->runcxsp->runsv.runsvstr))

/* pop a property number, signalling an error if not a property number */
/* prpnum runpopprp(runcxdef *ctx); */
#define runpopprp(ctx) \
 (runstkund(ctx), ((--(ctx)->runcxsp))->runstyp!=DAT_PROPNUM ? \
  (runsig(ctx,ERR_REQVPR), (prpnum)0) : \
  ((ctx)->runcxsp->runsv.runsvprp))


/* pop function pointer */
/* objnum runpopfn(runcxdef *ctx); */
#define runpopfn(ctx) \
  ((objnum)(runstkund(ctx), ((--(ctx)->runcxsp))->runstyp!=DAT_FNADDR ? \
                      (runsig(ctx,ERR_REQVFN), (objnum)0) : \
                      ((ctx)->runcxsp->runsv.runsvobj)))

/* pop a string value */
/* char *runpopstr(runcxdef *ctx); */
#define runpopstr(ctx) \
 (runstkund(ctx), ((--((ctx)->runcxsp))->runstyp!=DAT_SSTRING ? \
  (runsig(ctx,ERR_REQSTR), (uchar *)0) : \
  ((ctx)->runcxsp->runsv.runsvstr)))


/* pop a logical value - TRUE for DAT_TRUE, FALSE for DAT_NIL */
/* int runpoplog(runcxdef *ctx); */
#define runpoplog(ctx) \
 ((--((ctx)->runcxsp))->runstyp==DAT_TRUE ? TRUE : \
  (ctx)->runcxsp->runstyp==DAT_NIL ? FALSE : \
  (runsig(ctx, ERR_REQLOG), 0))

/* get type of top of stack */
/* int runtostyp(runcxdef *ctx); */
#define runtostyp(ctx) (((ctx)->runcxsp - 1)->runstyp)

/* determine if top of stack is logical value (returns TRUE if so) */
/* int runtoslog(runcxdef *ctx); */
#define runtoslog(ctx) \
 (runtostyp(ctx) == DAT_TRUE || runtostyp(ctx) == DAT_NIL)

/* convert C logical to TADS logical (TRUE->DAT_TRUE, FALSE->DAT_NIL) */
/* int runclog(int log); */
#define runclog(l) ((l) ? DAT_TRUE : DAT_NIL)

/* compare magnitudes of numbers/strings on top of stack; strcmp-like value */
int runmcmp(runcxdef *ctx);

/* TRUE if items at top of stack are equal, FALSE otherwise */
int runeq(runcxdef *ctx);

/* check for stack underflow */
/* void runstkund(runcxdef *ctx); */

/* check for stack overflow */
/* void runstkovf(runcxdef *ctx); */

/* 
 *   Check to ensure we have enough arguments to pass to a function or method
 *   call - this simply ensures we have enough data in the current frame.
 *   This is important because the called function will be able to write to
 *   our frame.  If we don't have enough arguments, we'll push enough 'nil'
 *   values to meet the need.  
 */
#define runcheckargc(ctx, nargc) \
    while ((ctx)->runcxsp - (ctx)->runcxbp < *(nargc)) \
        runpnil(ctx)

#ifdef RUNFAST
# define runstkovf(ctx) (DISCARD 0)
# define runstkund(ctx) (DISCARD 0)
#else /* RUNFAST */
# define runstkovf(ctx) \
 ((ctx)->runcxsp >= (ctx)->runcxstop ? (runsig(ctx, ERR_STKOVF), \
 DISCARD 0) : DISCARD 0)
# define runstkund(ctx) \
 ((ctx)->runcxsp == (ctx)->runcxstk ? runsig(ctx, ERR_STKUND), \
 DISCARD 0 : DISCARD 0)
#endif /* RUNFAST */

/* reserve space in heap, collecting garbage if necessary */
/* void runhres(runcxdef *ctx, uint siz, uint below); */
#define runhres(ctx, siz, below) \
 ((uint)((ctx)->runcxhtop - (ctx)->runcxhp) > (uint)(siz) ? DISCARD 0 : \
 (runhcmp(ctx, siz, below, (runsdef *)0, (runsdef *)0, (runsdef *)0),\
  DISCARD 0))

/* reserve space, with various amounts of saving */
#define runhres1(ctx, siz, below, val1) \
  ((uint)((ctx)->runcxhtop - (ctx)->runcxhp) > (uint)(siz) ? DISCARD 0 : \
  (runhcmp(ctx, siz, below, val1, (runsdef *)0, (runsdef *)0), DISCARD 0))

#define runhres2(ctx, siz, below, val1, val2) \
 ((uint)((ctx)->runcxhtop - (ctx)->runcxhp) > (uint)(siz) ? DISCARD 0 : \
 (runhcmp(ctx, siz, below, val1, val2, (runsdef *)0), DISCARD 0))

#define runhres3(ctx, siz, below, val1, val2, val3) \
 ((uint)((ctx)->runcxhtop - (ctx)->runcxhp) > (uint)(siz) ? DISCARD 0 : \
 (runhcmp(ctx, siz, below, val1, val2, val3), DISCARD 0))

/* garbage collect heap, making sure 'siz' bytes are available afterwards */
void runhcmp(runcxdef *ctx, uint siz, uint below,
             runsdef *val1, runsdef *val2, runsdef *val3);

/* determine size of a data item */
int runsiz(runsdef *item);

/* find a sublist within a list, returning pointer to sublist or NULL */
uchar *runfind(uchar *list, runsdef *item);

/* add two runsdef values, returning result in *val */
void runadd(runcxdef *ctx, runsdef *val, runsdef *val2, uint below);

/*
 *   subtract val2 from val, returning result in *val; return TRUE if
 *   value changed, FALSE otherwise (this is returned when subtracting
 *   something from a list that isn't in the list) 
 */
int runsub(runcxdef *ctx, runsdef *val, runsdef *val2, uint below);

/* restore code pointer from object.property + offset */
uchar *runcprst(runcxdef *ctx, uint ofs, objnum obj, prpnum prop);

/* leave a stack frame, removing arguments */
/* void runleave(runcxdef *ctx, uint parms); */
#define runleave(ctx, parms) \
 (((ctx)->runcxsp = (ctx)->runcxbp), \
  ((ctx)->runcxbp = (runsdef *)((--((ctx)->runcxsp))->runsv.runsvstr)), \
  ((ctx)->runcxsp -= (parms)))

/* reset run-time: throw away entire stack and heap */
/* void runrst(runcxdef *ctx); */
#define runrst(ctx) (((ctx)->runcxsp = (ctx)->runcxstk), \
                     ((ctx)->runcxhp = (ctx)->runcxheap), \
                     dbgrst(ctx->runcxdbg))

/* set up runtime status line display */
void runistat(struct voccxdef *vctx, struct runcxdef *rctx,
                  struct tiocxdef *tctx);

/* signal a run-time error - allows debugger trapping */
void runsign(runcxdef *ctx, int err);

/* sign a run-time error with zero arguments */
#define runsig(ctx, err) (errargc((ctx)->runcxerr,0),runsign(ctx,err))

/* signal a run-time error with one argument */
#define runsig1(ctx, err, typ, arg) \
  (errargv((ctx)->runcxerr,0,typ,arg),errargc((ctx)->runcxerr,1),\
   runsign(ctx,err))

/* draw status line */
void runstat();

/* initialize output status */
void runistat(struct voccxdef *vctx, struct runcxdef *rctx,
              struct tiocxdef *tctx);

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
