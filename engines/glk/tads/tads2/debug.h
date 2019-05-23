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

#ifndef GLK_TADS_TADS2_DEBUG
#define GLK_TADS_TADS2_DEBUG

#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/object.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/* forward declarations */
struct bifcxdef;
struct toksdef;
struct toktdef;
struct tokcxdef;

/* stack frame record */
struct dbgfdef
{
    struct runsdef *dbgfbp;                        /* base pointer of frame */
    objnum  dbgfself;             /* 'self' object (MCMONINV for functions) */
    objnum  dbgftarg;                               /* actual target object */
    prpnum  dbgfprop;                          /* property being evalutated */
    int     dbgfargc;                                /* number of arguments */
    int     dbgfbif;      /* set to built-in function number if in built-in */
    uint    dbgffr;         /* offset in object of local frame symbol table */
    uint    dbgflin;                      /* OPCLINE operand of latest line */
};
typedef struct dbgfdef dbgfdef;

/* max number of frames to store in debug frame memory */
#define DBGMAXFRAME  100

/* maximum number of breakpoints set concurrently */
#define DBGBPMAX 50

/* breakpoint structure */
struct dbgbpdef
{
    objnum dbgbpself;               /* the "self" object for the breakpoint */
    objnum dbgbptarg;            /* actual target object for the breakpoint */
    uint   dbgbpofs;                  /* offset in object of the breakpoint */
    uint   dbgbpflg;                                    /* breakpoint flags */
#define DBGBPFUSED  0x01                         /* breakpoint has been set */
#define DBGBPFNAME  0x02                 /* name of address has been stored */
#define DBGBPFCOND  0x04             /* breakpoint has a condition attached */
#define DBGBPFDISA  0x08                          /* breakpoint is disabled */
#define DBGBPFCONDNAME 0x10        /* condition name string has been stored */
    uint   dbgbpnam;       /* offset of address name within dbgcxnam buffer */
    uint   dbgbpcondnam;        /* offset of condition string within buffer */
    objnum dbgbpcond;        /* object containing compiled condition for bp */
};
typedef struct dbgbpdef dbgbpdef;

/* maximum number of watch expressions set concurrently */
#define DBGWXMAX 30

/* watch expression structure */
struct dbgwxdef
{
    objnum  dbgwxobj;              /* object containing compiled expression */
    objnum  dbgwxself;                         /* 'self' for the expression */
    uint    dbgwxnam;   /* offset of expression text within dbgcxnam buffer */
    uint    dbgwxflg;               /* flags for this watch expression slot */
#define DBGWXFUSED  0x01                            /* watch slot is in use */
#define DBGWXFNAME  0x02                   /* name of watch has been stored */
};
typedef struct dbgwxdef dbgwxdef;

/* amount of space for bp names (original address strings from user) */
#define DBGCXNAMSIZ 2048

/* debug context */
struct dbgcxdef
{
    struct tiocxdef *dbgcxtio;                          /* text i/o context */
    struct tokthdef *dbgcxtab;                              /* symbol table */
    struct mcmcxdef *dbgcxmem;              /* memory cache manager context */
    struct errcxdef *dbgcxerr;                    /* error handling context */
    struct    lindef *dbgcxlin;                    /* chain of line sources */
    int       dbgcxfcn;                          /* number of frames in use */
    int       dbgcxdep;          /* actual depth (if overflow frame buffer) */
    int       dbgcxfid;                        /* source file serial number */
    dbgfdef   dbgcxfrm[DBGMAXFRAME];                        /* stack frames */
    int       dbgcxflg;                          /* flags for debug session */
#define    DBGCXFSS   0x01                  /* single-stepping source lines */
#define    DBGCXFSO   0x02          /* stepping over a function/method call */
#define    DBGCXFOK   0x04                         /* debugger is linked in */
#define    DBGCXFIND  0x08     /* in debugger - suppress stack trace on err */
#define    DBGCXFGBP  0x10                  /* global breakpoints in effect */
#define    DBGCXFTRC  0x20                        /* call tracing activated */
#define    DBGCXFLIN2 0x40         /* new-style line records (line numbers) */
    int       dbgcxsof;                    /* frame depth at step-over time */
    dbgbpdef  dbgcxbp[DBGBPMAX];                             /* breakpoints */
    dbgwxdef  dbgcxwx[DBGWXMAX];                       /* watch expressions */
    struct    prscxdef *dbgcxprs;                        /* parsing context */
    struct    runcxdef *dbgcxrun;                      /* execution context */
    uint      dbgcxnamf;               /* next free byte of dbgcxnam buffer */
    uint      dbgcxnams;                         /* size of dbgcxnam buffer */
    char     *dbgcxnam;                       /* space for bp address names */
    char     *dbgcxhstp;                             /* call history buffer */
    uint      dbgcxhstl;                           /* history buffer length */
    uint      dbgcxhstf;             /* offset of next free byte of history */

    /* 
     *   This member is for the use of the user interface code.  If the
     *   user interface implementation needs to store additional context,
     *   it can allocate a structure of its own (it should probably do
     *   this in dbguini()) and store a pointer to that structure here.
     *   Since the user interface entrypoints always have the debugger
     *   context passed as a parameter, the user interface code can
     *   recover its extra context information by following this pointer
     *   and casting it to its private structure type.  The TADS code
     *   won't do anything with this pointer except initialize it to null
     *   when initializing the debugger context.  
     */
    void     *dbgcxui;
};
typedef struct dbgcxdef dbgcxdef;


/* ======================================================================== */
/*
 *   Compiler interface.  These routines are called by the compiler to
 *   inform the debug record generator about important events as
 *   compilation proceeds. 
 */


/*
 *   Tell the current line source that we're compiling an executable
 *   line, and tell it the object number and offset of the code within the
 *   object. 
 */
void dbgclin(struct tokcxdef *tokctx, objnum objn, uint ofs);

/* size of information given to line source via lincmpinf method */
#define DBGLINFSIZ   4



/* ======================================================================== */
/*
 *   Run-time interface.  These routines are called by the run-time
 *   system to apprise the debugger of important events during execution.
 */


/*
 *   Determine if the debugger is present.  Returns true if so, false if
 *   not.  This should return false for any stand-alone version of the
 *   executable that isn't linked with the debugger.  If this returns
 *   true, dbgucmd() must not have a trivial implementation -- dbgucmd()
 *   must at least let the user quit out of the game.
 *   
 *   This can be switched at either link time or compile time.  If DBG_OFF
 *   is defined, we'll force this to return false; otherwise, we'll let
 *   the program define the appropriate implementation through the linker.
 */
#ifdef DBG_OFF
#define dbgpresent() (false)
#else
int dbgpresent();
#endif


/* add a debug tracing record */
/* void dbgenter(dbgcxdef *ctx, runsdef *bp, objnum self, objnum target,
                 prpnum prop, int binum, int argc); */ 

/* tell debugger where the current line's local frame table is located */
/* void dbgframe(dbgcxdef *ctx, uint ofsfr, ofslin); */

/* 
 *   Single-step interrupt: the run-time has reached a new source line.
 *   ofs is the offset from the start of the object of the line record,
 *   and p is the current execution pointer.  *p can be changed upon
 *   return, in which case the run-time will continue from the new
 *   position; however, the new address must be within the same function
 *   or method as it was originally.
 */
/* void dbgssi(dbgcxdef *ctx, uint ofs, int instr,
               int err, uchar *noreg *p); */

/* pop debug trace level */
/* void dbgleave(dbgcxdef *ctx, int exittype); */
#define DBGEXRET   0                                /* return with no value */
#define DBGEXVAL   1                                 /* return with a value */
#define DBGEXPASS  2                       /* use 'pass' to exit a function */

/* dump the stack into text output */
/* void dbgdump(dbgcxdef *ctx); */

/* reset debug stack (throw away entire contents) */
/* void dbgrst(dbgcxdef *ctx); */

/* activate debugger if possible; returns TRUE if no debugger is present */
int dbgstart(dbgcxdef *ctx);

/* add a string to the history buffer */
void dbgaddhist(dbgcxdef *ctx, char *buf, int bufl);

/*
 *   Find a base pointer, given the object+offset of the frame.  If the
 *   frame is not active, this routine signals ERR_INACTFR; otherwise, the
 *   bp value for the frame is returned. 
 */
struct runsdef *dbgfrfind(dbgcxdef *ctx, objnum frobj, uint frofs);


/* ======================================================================== */
/*
 *   User Interface Support routines.  These routines are called by the
 *   user interface layer to get information from the debugger and perform
 *   debugging operations. 
 */


/* get a symbol name; returns length of name */
int dbgnam(dbgcxdef *ctx, char *outbuf, int typ, int val);

/*
 *   Get information about current line.  It is assumed that the caller
 *   knows the size of the line information .
 */
void dbglget(dbgcxdef *ctx, uchar *buf);

/*
 *   Get information about a line in an enclosing stack frame.  Level 0 is
 *   the current line, level 1 is the first enclosing frame, and so on.
 *   Returns 0 on success, non-zero if the frame level is invalid.  
 */
int dbglgetlvl(dbgcxdef *ctx, uchar *buf, int level);

/*
 *   Set a breakpoint by symbolic address: "function" or
 *   "object.property".  The string may contain whitespace characters
 *   around each symbol; it must be null-terminated.  If an error occurs,
 *   the error number is returned.  bpnum returns with the breakpoint
 *   number if err == 0.  If the condition string is given (and is not an
 *   empty string), the condition is compiled in the scope of the
 *   breakpoint and attached as the breakpoint condition.  
 */
int dbgbpset(dbgcxdef *ctx, char *addr, int *bpnum);

/* 
 *   Set a breakpoint at an object + offset location.  If 'toggle' is
 *   true, and there's already a breakpoint at the given location, we'll
 *   clear the breakpoint; in this case, *did_set will return false to
 *   indicate that an existing breakpoint was cleared rather than a new
 *   breakpoint created.  *did_set will return true if a new breakpoint
 *   was set.  
 */
int dbgbpat(dbgcxdef *ctx, objnum objn, objnum self,
            uint ofs, int *bpnum, char *bpname, int toggle,
            char *condition, int *did_set);

/* 
 *   Set a breakpoint at an object + offset location, optionally with a
 *   condition, using an existing breakpoint slot.  If the slot is already
 *   in use, we'll return an error.  
 */
int dbgbpatid(dbgcxdef *ctx, int bpnum, objnum target, objnum self,
              uint ofs, char *bpname, int toggle, char *cond,
              int *did_set);

/*
 *   Determine if there's a breakpoint at a given code location.  Fills in
 *   *bpnum with the breakpoint identifier and returns true if a
 *   breakpoint is found at the given location; returns false if there are
 *   no breakpoints matching the description.  
 */
int dbgisbp(dbgcxdef *ctx, objnum target, objnum self, uint ofs, int *bpnum);

/*
 *   Determine if the given breakpoint is enabled 
 */
int dbgisbpena(dbgcxdef *ctx, int bpnum);

/*
 *   Delete a breakpoint by breakpoint number (as returned from
 *   dbgbpset).  Returns error number, or 0 for success. 
 */
int dbgbpdel(dbgcxdef *ctx, int bpnum);

/* disable or enable a breakpoint, by breakpoint number; returns error num */
int dbgbpdis(dbgcxdef *ctx, int bpnum, int disable);

/*
 *   Set a new condition for the given breakpoint.  Replaces any existing
 *   condition.  If an error occurs, we'll leave the old condition as it
 *   was and return a non-zero error code; on success, we'll update the
 *   condition and return zero. 
 */
int dbgbpsetcond(dbgcxdef *ctx, int bpnum, char *cond);

/* list breakpoints, using user callback to do display */
void dbgbplist(dbgcxdef *ctx,
               void (*dispfn)(void *ctx, const char *str, int len),
               void *dispctx);

/* enumerate breakpoints */
void dbgbpenum(dbgcxdef *ctx,
               void (*cbfunc)(void *cbctx, int bpnum, const char *desc,
                              const char *cond, int disabled), void *cbctx);

/* call callback with lindef data for each breakpoint currently set */
void dbgbpeach(dbgcxdef *ctx,
               void (*fn)(void *, int, uchar *, uint),
               void *fnctx);

/* 
 *   Get information on a specific breakpoint.  Returns zero on success,
 *   non-zero on failure. 
 */
int dbgbpgetinfo(dbgcxdef *ctx, int bpnum, char *descbuf, size_t descbuflen,
                 char *condbuf, size_t condbuflen);

/* 
 *   Evaluate an expression (a text string to be parsed) at a particular
 *   stack context level; returns error number.  Invokes the callback
 *   function repeatedly to display the value string, and ends the display
 *   with a newline.  If showtype is true, we'll include a type name
 *   prefix, otherwise we'll simply display the value.  
 */
int dbgeval(dbgcxdef *ctx, char *expr,
            void (*dispfn)(void *dispctx, const char *str, int strl),
            void *dispctx, int level, int showtype);

/*
 *   Evaluate an expression, extended version.  For aggregate values
 *   (objects, lists), we'll invoke a callback function for each value
 *   contained by the aggregate value, passing the callback the name and
 *   relationship of the subitem.  The relationship is simply the operator
 *   that should be used to join the parent expression and the subitem
 *   name to form the full subitem expression; for objects, it's ".", and
 *   for lists it's null (because for lists the subitem names will include
 *   brackets).  'speculative' is passed to dbgcompile; see the comments
 *   there for information on the purpose of this flag.  
 */
int dbgevalext(dbgcxdef *ctx, char *expr,
               void (*dispfn)(void *dispctx, const char *str, int strl),
               void *dispctx, int level, int showtype, dattyp *dat,
               void (*aggcb)(void *aggctx, const char *subname,
                             int subnamelen, const char *relationship),
               void *aggctx, int speculative);

/* 
 *   enumerate local variables at a given stack context level by calling
 *   the given function once for each local variable 
 */
void dbgenumlcl(dbgcxdef *ctx, int level,
                void (*func)(void *ctx, const char *lclnam, size_t lclnamlen),
                void *cbctx);

/* 
 *   Compile an expression in a given frame context.  Returns an error
 *   number.  Allocates a new object to contain the compiled code, and
 *   returns the object number in *objn; the caller is responsible for
 *   freeing the object when done with it.
 *   
 *   If 'speculative' is set to true, we'll prohibit the expression from
 *   making any assignments or calling any methods or functions.  This
 *   mode can be used to try compiling an expression that the user could
 *   conceivably be interested in but has not expressly evaluated; for
 *   example, this can be used to implement "tooltip evaluation," where
 *   the debugger automatically shows a little pop-up window with the
 *   expression under the mouse cursor if the mouse cursor is left
 *   hovering over some text for a few moments.  In such cases, since the
 *   user hasn't explicitly requested evaluation, it would be bad to make
 *   any changes to game state, hence the prohibition of assignments or
 *   calls.  
 */
int dbgcompile(dbgcxdef *ctx, char *expr, dbgfdef *fr, objnum *objn,
               int speculative);

/* display a stack traceback through a user callback */
void dbgstktr(dbgcxdef *ctx,
              void (*dispfn)(void *dispctx, const char *str, int strl),
              void *dispctx, int level, int toponly, int include_markers);

/* format a display of where execution is stopped into a buffer */
void dbgwhere(dbgcxdef *ctx, char *buf);

/* set a watch expression; returns error or 0 for success */
int dbgwxset(dbgcxdef *ctx, char *expr, int *wxnum, int level);

/* delete a watch expression */
int dbgwxdel(dbgcxdef *ctx, int wxnum);

/* update all watch expressions */
void dbgwxupd(dbgcxdef *ctx,
              void (*dispfn)(void *dispctx, const char *txt, int len),
              void *dispctx);

/* switch to a new active lindef */
void dbgswitch(struct lindef **linp, struct lindef *newlin);



/* ======================================================================== */
/*
 *   User Interface Routines.  The routines are called by the debugger
 *   to perform user interaction.
 */

/* 
 *   Debugger user interface initialization, phase one.  TADS calls this
 *   routine during startup, before reading the .GAM file, to let the user
 *   interface perform any initialization it requires before the .GAM file
 *   is loaded.  
 */
void dbguini(dbgcxdef *ctx, const char *game_filename);

/*
 *   Debugger user interface initialization, phase two.  TADS calls this
 *   routine during startup, after read the .GAM file.  The debugger user
 *   interface code can perform any required initialization that depends
 *   on the .GAM file having been read.  
 */
void dbguini2(dbgcxdef *ctx);

/*
 *   Determine if the debugger can resume from a run-time error.  This
 *   reflects the capabilities of the user interface of the debugger.  In
 *   particular, if the UI provides a way to change the instruction
 *   pointer, then the debugger can resume from an error, since the user
 *   can always move past the run-time error and continue execution.  If
 *   the UI doesn't let the user change the instruction pointer, resuming
 *   from an error won't work, since the program will keep hitting the
 *   same error and re-entering the debugger.  If this returns false, the
 *   run-time will trap to the debugger on an error, but will simply abort
 *   the current command when the debugger returns.  If this returns true,
 *   the run-time will trap to the debugger on an error with the
 *   instruction pointer set back to the start of the line containing the
 *   error, and will thus re-try the same line of code when the debugger
 *   returns, unless the debugger explicitly moves the instruction pointer
 *   before returning.  
 */
int dbgu_err_resume(dbgcxdef *ctx);

/*
 *   Find a source file.  origname is the name of the source file as it
 *   appears in the game's debugging information; this routine should
 *   figure out where the file actually is, and put the fully-qualified
 *   path to the file in fullname.  The debugger calls this after it
 *   exhausts all of its other methods of finding a source file (such as
 *   searching the include path).
 *   
 *   Return true if the source file should be considered valid, false if
 *   not.  Most implementations will simply return true if the file was
 *   found, false if not; however, this approach will cause the debugger
 *   to terminate with an error at start-up if the user hasn't set up the
 *   debugger's include path correctly before running the debugger.  Some
 *   implementations, in particular GUI implementations, may wish to wait
 *   to find a file until the file is actually needed, rather than pester
 *   the user with file search dialogs repeatedly at start-up.
 *   
 *   must_find_file specifies how to respond if we can't find the file.
 *   If must_find_file is true, we should always return false if we can't
 *   find the file.  If must_find_file is false, however, we can
 *   optionally return true even if we can't find the file.  Doing so
 *   indicates that the debugger UI will defer locating the file until it
 *   is actually needed.
 *   
 *   If this routine returns true without actually finding the file, it
 *   should set fullname[0] to '\0' to indicate that fullname doesn't
 *   contain a valid filename.  
 */
int dbgu_find_src(const char *origname, int origlen,
                  char *fullname, size_t full_len, int must_find_file);


/* 
 *   Debugger user interface main command loop.  If err is non-zero, the
 *   debugger was entered because a run-time error occurred; otherwise, if
 *   bphit is non-zero, it's the number of the breakpoint that was
 *   encountered; otherwise, the debugger was entered through a
 *   single-step of some kind.  exec_ofs is the byte offset within the
 *   target object of the next instruction to be executed.  This can be
 *   changed upon return, in which case execution will continue from the
 *   new offset, but the offset must be within the same method of the same
 *   object (or within the same function) as it was upon entry. 
 */
void dbgucmd(dbgcxdef *ctx, int bphit, int err, unsigned int *exec_ofs);

/*
 *   Debugger UI - quitting game.  The runtime calls this routine just
 *   before the play loop is about to terminate after the game code has
 *   called the "quit" built-in function.  If the debugger wants, it can
 *   take control here (just as with dbgucmd()) for as long as it wants.
 *   If the debugger wants to restart the game, it should call bifrst().
 *   If this routine returns without signalling a RUN_RESTART error, TADS
 *   will terminate.  If a RUN_RESTART error is signalled, TADS will
 *   resume the play loop.  
 */
void dbguquitting(dbgcxdef *ctx);

/* 
 *   debugger user interface termination - this routine is called when the
 *   debugger is about to terminate, so that the user interface can close
 *   itself down (close windows, release memory, etc) 
 */
void dbguterm(dbgcxdef *ctx);

/*
 *   Debugger user interface: display an error.  This is called mainly so
 *   that the debugger can display an error using special output
 *   formatting if the error occurs while debugging. 
 */
void dbguerr(dbgcxdef *ctx, int errnum, char *msg);

/* turn hidden output tracing on/off */
void trchid(void);
void trcsho(void);


/* ======================================================================== */
/*
 *   optional debugger macros - these compile to nothing when compiling a
 *   version for use without the debugger 
 */

#ifdef DBG_OFF
#define dbgenter(ctx, bp, self, target, prop, binum, argc)
#define dbgleave(ctx, exittype) ((void)0)
#define dbgdump(ctx) ((void)0)
#define dbgrst(ctx) ((void)0)
#define dbgframe(ctx, frofs, linofs)
#define dbgssi(ctx, ofs, instr, err, p) ((void)0)
#else /* DBG_OFF */
#define dbgenter(ctx, bp, self, target, prop, binum, argc) \
   dbgent(ctx, bp, self, target, prop, binum, argc)
#define dbgleave(ctx, exittype) dbglv(ctx, exittype)
#define dbgdump(ctx) dbgds(ctx)
#define dbgrst(ctx) ((ctx)->dbgcxfcn = (ctx)->dbgcxdep = 0)
#define dbgframe(ctx, frofs, linofs) \
   (((ctx)->dbgcxfrm[(ctx)->dbgcxfcn - 1].dbgffr = (frofs)), \
    ((ctx)->dbgcxfrm[(ctx)->dbgcxfcn - 1].dbgflin = (linofs)))
#define dbgssi(ctx, ofs, instr, err, p) dbgss(ctx, ofs, instr, err, p)
#endif /* DBG_OFF */

/* ======================================================================== */
/* private internal routines */

void dbgent(dbgcxdef *ctx, struct runsdef *bp, objnum self, objnum target,
            prpnum prop, int binum, int argc);

void dbglv(dbgcxdef *ctx, int exittype);

void dbgds(dbgcxdef *ctx);

void dbgss(dbgcxdef *ctx, uint ofs, int instr, int err, uchar *noreg *p);

void dbgpval(struct dbgcxdef *ctx, struct runsdef *val,
             void (*dispfn)(void *, const char *, int),
             void *dispctx, int showtype);

int dbgtabsea(struct toktdef *tab, char *name, int namel, int hash,
              struct toksdef *ret);

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
