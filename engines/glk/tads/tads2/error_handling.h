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

/* Library error handling definitions
 * All of the functions and macros in here are named ERRxxx because
 * this file was based on the TADS err.h, which used the ERRxxx naming
 * convention, and it would be a lot of trouble to change.
 * 
 * This package defines a set of macros that allows code to raise and
 * handle exceptions.  A macro is provided which signals an error, which
 * does a non-local goto to the innermost enclosing exception handler.
 * A set of macros sets up exception handling code.
 * 
 * To catch exceptions that occur inside a block of code (i.e., in the
 * code or in any subroutines called by the code), begin the block with
 * ERRBEGIN.  At the end of the protected code, place the exception
 * handler, which starts with ERRCATCH.  At the end of the exception
 * handler, place ERREND.  If no exception occurs, execution goes
 * through the protected code, then resumes at the code following
 * the ERREND.
 * 
 * The exception handler can signal another error, which will cause
 * the next enclosing frame to catch the error.  Alternatively, if
 * the exception handler doesn't signal an error or return, execution
 * continues at the code following the ERREND.  Exceptions that are
 * signalled during exception handling will be caught by the next
 * enclosing frame, unless the exception handler code is itself
 * protected by another ERRBEGIN-ERREND block.
 * 
 * To signal an error, use errsig().
 * 
 * To use a string argument in a signalled error, cover the string
 * with errstr(ctx, str, len); for example:
 * 
 * errsig1(ctx, ERR_XYZ, ERRTSTR, errstr(ctx, buf, strlen(buf)));
 * 
 * This copies the string into a buffer that is unaffected by
 * stack resetting during error signalling.
 */

#ifndef GLK_TADS_TADS2_ERROR_HANDLING
#define GLK_TADS_TADS2_ERROR_HANDLING

#include "glk/tads/tads2/lib.h"
#include "glk/tads/os_frob_tads.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/**
 * Maximum length of a facility identifier
 */
#define ERRFACMAX    6

union erradef {
    int   erraint;                                      /* integer argument */
    const char *errastr;                            /* text string argument */
};

struct errdef {
    struct errdef *errprv;                          /* previous error frame */
    int            errcode;        /* error code of exception being handled */
    char           errfac[ERRFACMAX+1];        /* facility of current error */
    erradef        erraav[10];                      /* parameters for error */
    int            erraac;                   /* count of parameters in argc */
};

#define ERRBUFSIZ 512

/**
 * Seek location record for an error message by number
 */
struct errmfdef {
    uint  errmfnum;                                         /* error number */
    ulong errmfseek;                       /* seek location of this message */
};
typedef struct errmfdef errmfdef;

struct errcxdef {
    errdef   *errcxptr;                              /* current error frame */
    void    (*errcxlog)(void *, const char *fac, int err, int argc, erradef *);
                                         /* error logging callback function */
    void     *errcxlgc;               /* context for error logging callback */
    int       errcxofs;                        /* offset in argument buffer */
    char      errcxbuf[ERRBUFSIZ];            /* space for argument strings */
    osfildef *errcxfp;                /* message file, if one is being used */
    errmfdef *errcxseek;              /* seek locations of messages in file */
    uint      errcxsksz;                         /* size of errcxseek array */
    ulong     errcxbase;   /* offset in physical file of logical error file */
    struct appctxdef *errcxappctx;              /* host application context */
};

/**
 * Begin protected code
 */
#define ERRBEGIN(ctx) \
  { \
    errdef fr_; \
    if (1 /*(fr_.errcode = setjmp(fr_.errbuf)) == 0 */) \
    { \
      fr_.errprv = (ctx)->errcxptr; \
      (ctx)->errcxptr = &fr_;

/**
 * End protected code, begin error handler
 */
#define ERRCATCH(ctx, e) \
      assert(1==1 && (ctx)->errcxptr != fr_.errprv); \
      (ctx)->errcxptr = fr_.errprv; \
    } \
    else \
    { \
      assert(2==2 && (ctx)->errcxptr != fr_.errprv); \
      (e) = fr_.errcode; \
      (ctx)->errcxptr = fr_.errprv;
        
/* retrieve argument (int, string) in current error frame */
#define errargint(argnum) (fr_.erraav[argnum].erraint)
#define errargstr(argnum) (fr_.erraav[argnum].errastr)

    
#define ERREND(ctx) \
    } \
  }

/* end protected code, begin cleanup (no handling; just cleaning up) */
#define ERRCLEAN(ctx) \
      assert((ctx)->errcxptr != fr_.errprv); \
      (ctx)->errcxptr = fr_.errprv; \
    } \
    else \
    { \
      assert((ctx)->errcxptr != fr_.errprv); \
      (ctx)->errcxptr = fr_.errprv;

#define ERRENDCLN(ctx) \
      errrse(ctx); \
    } \
  }



/* argument types for errors with arguments */
#define ERRTINT  erraint
#define ERRTSTR  errastr

/* set argument count in error frame */
#define errargc(ctx,cnt) ((ctx)->errcxptr->erraac=(cnt))

/* enter string argument; returns pointer to argument used in errargv */
#ifdef ERR_NO_MACRO
char *errstr(errcxdef *ctx, const char *str, int len);
#else /* ERR_NO_MACRO */
  
#define errstr(ctx,str,len) \
  ((memcpy(&(ctx)->errcxbuf[(ctx)->errcxofs],str,(size_t)len), \
   (ctx)->errcxofs += (len), \
   (ctx)->errcxbuf[(ctx)->errcxofs++] = '\0'), \
   &(ctx)->errcxbuf[(ctx)->errcxofs-(len)-1])

#endif /* ERR_NO_MACRO */

/* set argument in error frame argument vector */
#define errargv(ctx,index,typ,arg) \
  ((ctx)->errcxptr->erraav[index].typ=(arg))

/* signal an error with argument count already set */
#ifdef ERR_NO_MACRO
void errsign(errcxdef *ctx, int e, const char *facility);
#else /* ERR_NO_MACRO */
# ifdef DEBUG
void errjmp(jump_buf buf, int e);
#  define errsign(ctx, e, fac) \
   (strncpy((ctx)->errcxptr->errfac, fac, ERRFACMAX),\
    (ctx)->errcxptr->errfac[ERRFACMAX]='\0',\
    (ctx)->errcxofs=0, errjmp((ctx)->errcxptr->errbuf, e))
# else /* DEBUG */
#  define errsign(ctx, e, fac) \
   (strncpy((ctx)->errcxptr->errfac, fac, ERRFACMAX),\
    (ctx)->errcxptr->errfac[ERRFACMAX]='\0',\
    (ctx)->errcxofs=0, longjmp((ctx)->errcxptr->errbuf, e))
# endif /* DEBUG */
#endif /* ERR_NO_MACRO */


/* signal an error with no arguments */
#ifdef ERR_NO_MACRO
void errsigf(errcxdef *ctx, const char *facility, int err);
#else /* ERR_NO_MACRO */
#define errsigf(ctx, fac, e) (errargc(ctx,0),errsign(ctx,e,fac))
#endif /* ERR_NO_MACRO */
  
/* signal an error with one argument */
#define errsigf1(ctx, fac, e, typ1, arg1) \
  (errargv(ctx,0,typ1,arg1),errargc(ctx,1),errsign(ctx,e,fac))

/* signal an error with two arguments */
#define errsigf2(ctx, fac, e, typ1, arg1, typ2, arg2) \
  (errargv(ctx,0,typ1,arg1), errargv(ctx,1,typ2,arg2), \
   errargc(ctx,2), errsign(ctx,e,fac))

/* resignal the current error - only usable within exception handlers */
#ifdef ERR_NO_MACRO
void errrse1(errcxdef *ctx, errdef *fr);
# define errrse(ctx) errrse1(ctx, &fr_)
#else /* ERR_NO_MACRO */

/* void errrse(errcxdef *ctx); */
# define errrse(ctx) \
  (errargc(ctx, fr_.erraac),\
   memcpy((ctx)->errcxptr->erraav, fr_.erraav, \
    (size_t)(fr_.erraac*sizeof(erradef))),\
   errsign(ctx, fr_.errcode, fr_.errfac))

#endif /* ERR_NO_MACRO */

/*
 *   For use in an error handler (ERRCATCH..ERREND) only: Copy the
 *   parameters from the error currently being handled to the enclosing
 *   frame.  This is useful when "keeping" an error being handled - i.e.,
 *   the arguments will continue to be used outside of the
 *   ERRCATCH..ERREND code. 
 */
/* void errkeepargs(errcxdef *ctx); */
#define errkeepargs(ctx) errcopyargs(ctx, &fr_)

/* 
 *   copy the parameters for an error from another frame into the current
 *   frame - this can be used when we want to be able to display an error
 *   that occurred in an inner frame within code that is protected by a
 *   new enclosing error frame 
 */
/* void errcopyargs(errcxdef *ctx, errdef *fr); */
#define errcopyargs(ctx, fr) \
   (errargc((ctx), (fr)->erraac), \
    memcpy((ctx)->errcxptr->erraav, (fr)->erraav, \
           (size_t)((fr)->erraac*sizeof(erradef))))

/* log error that's been caught, using arguments already caught */
#define errclog(ctx) \
 ((*(ctx)->errcxlog)((ctx)->errcxlgc,fr_.errfac,fr_.errcode,\
  fr_.erraac,fr_.erraav))

/* log an error that's been set up but not signalled yet */
#define errprelog(ctx, err) \
 ((*(ctx)->errcxlog)((ctx)->errcxlgc,(ctx)->errcxptr->errfac,\
   err,(ctx)->errcxptr->erraac,\
   (ctx)->errcxptr->erraav))

/* log an error (no signalling, just reporting) */
#ifdef ERR_NO_MACRO
void errlogn(errcxdef *ctx, int err, const char *facility);
#else /* ERR_NO_MACRO */

#define errlogn(ctx,err,fac) \
 ((ctx)->errcxofs=0,\
  (*(ctx)->errcxlog)((ctx)->errcxlgc,fac,err,(ctx)->errcxptr->erraac,\
  (ctx)->errcxptr->erraav))

#endif /* ERR_NO_MACRO */

/* log an error with no arguments */
#ifdef ERR_NO_MACRO
void errlogf(errcxdef *ctx, const char *facility, int err);
#else /* ERR_NO_MACRO */

/* void errlogf(errcxdef *ctx, char *facility, int err); */
#define errlogf(ctx,fac,err) (errargc(ctx,0),errlogn(ctx,err,fac))

#endif /* ERR_NO_MACRO */

/* log an error with one argument */
#define errlogf1(ctx, fac, e, typ1, arg1) \
 (errargv(ctx,0,typ1,arg1),errargc(ctx,1),errlogn(ctx,e,fac))
  
/* log an error with two arguments */
#define errlogf2(ctx, fac, e, typ1, arg1, typ2, arg2) \
 (errargv(ctx,0,typ1,arg1),errargv(ctx,1,typ2,arg2),\
  errargc(ctx,2),errlogn(ctx,e,fac))


/*
 *   Format an error message, sprintf-style, using arguments in an
 *   erradef array (which is passed to the error-logging callback).
 *   Returns the length of the output string, even if the actual
 *   output string was truncated because the outbuf was too short.
 *   (If called with outbufl == 0, nothing will be written out, but
 *   the size of the buffer needed, minus the terminating null byte,
 *   will be computed and returned.)
 */
int errfmt(char *outbuf, int outbufl, char *fmt, int argc,
           erradef *argv);
  
/* get the text of an error */
void errmsg(errcxdef *ctx, char *outbuf, uint outbufl, uint err);
  
/* initialize error subsystem, opening error message file if necessary */
void errini(errcxdef *ctx, osfildef *fp);

/* allocate and initialize error context, free error context */
errcxdef *lerini();
void      lerfre(errcxdef *ctx);

/* error message structure - number + text */
struct errmdef {
    uint   errmerr;                                         /* error number */
    char  *errmtxt;                                /* text of error message */
};

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
