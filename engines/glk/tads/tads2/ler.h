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

#ifndef GLK_TADS_TADS2_LER
#define GLK_TADS_TADS2_LER

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/algorithm.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

// maximum length of a facility identifier
#define ERRFACMAX    6

enum ErrorCode {
	/* memory/cache manager errors */
	ERR_NOMEM    =  1,                                     /* out of memory */
	ERR_FSEEK    =  2,                             /* error seeking in file */
	ERR_FREAD    =  3,                           /* error reading from file */
	ERR_NOPAGE   =  4,                                /* no more page slots */
	ERR_REALCK   =  5,          /* attempting to reallocate a locked object */
	ERR_SWAPBIG  =  6,    /* swapfile limit reached - out of virtual memory */
	ERR_FWRITE   =  7,                                /* error writing file */
	ERR_SWAPPG   =  8,                    /* exceeded swap page table limit */
	ERR_CLIUSE   =  9,     /* requested client object number already in use */
	ERR_CLIFULL  = 10,                      /* client mapping table is full */
	ERR_NOMEM1   = 11,   /* swapping/garbage collection failed to find enuf */
	ERR_NOMEM2   = 12,            /* no memory to resize (expand) an object */
	ERR_OPSWAP   = 13,                          /* unable to open swap file */
	ERR_NOHDR    = 14,                     /* can't get a new object header */
	ERR_NOLOAD   = 15,   /* mcm cannot find object to load (internal error) */
	ERR_LCKFRE   = 16,     /* attempting to free a locked object (internal) */
	ERR_INVOBJ   = 17,                                    /* invalid object */
	ERR_BIGOBJ   = 18,  /* object too big - exceeds memory allocation limit */

	/* lexical analysis errors */
	ERR_INVTOK   = 100,                                    /* invalid token */
	ERR_STREOF   = 101,                /* end of file while scanning string */
	ERR_TRUNC    = 102,                      /* symbol too long - truncated */
	ERR_NOLCLSY  = 103,                   /* no space in local symbol table */
	ERR_PRPDIR   = 104,               /* invalid preprocessor (#) directive */
	ERR_INCNOFN  = 105,                /* no filename in #include directive */
	ERR_INCSYN   = 106,                          /* invalid #include syntax */
	ERR_INCSEAR  = 107,                         /* can't find included file */
	ERR_INCMTCH  = 108,       /* no matching delimiter in #include filename */
	ERR_MANYSYM  = 109,                    /* out of space for symbol table */
	ERR_LONGLIN  = 110,                                    /* line too long */
	ERR_INCRPT   = 111,           /* header file already included - ignored */
	ERR_PRAGMA   = 112,                         /* unknown pragma (ignored) */
	ERR_BADPELSE = 113,                                 /* unexpected #else */
	ERR_BADENDIF = 114,                                /* unexpected #endif */
	ERR_BADELIF  = 115,                                 /* unexpected #elif */
	ERR_MANYPIF  = 116,                             /* #if nesting too deep */
	ERR_DEFREDEF = 117,                           /* symbol already defined */
	ERR_PUNDEF   = 118,                        /* #undef symbol not defined */
	ERR_NOENDIF  = 119,                                   /* missing #endif */
	ERR_MACNEST  = 120,                         /* macros nested too deeply */
	ERR_BADISDEF = 121,          /* invalid argument for defined() operator */
	ERR_PIF_NA   = 122,                           /* #if is not implemented */
	ERR_PELIF_NA = 123,                         /* #elif is not implemented */
	ERR_P_ERROR  = 124,                              /* error directive: %s */
	ERR_LONG_FILE_MACRO = 125,               /* __FILE__ expansion too long */
	ERR_LONG_LINE_MACRO = 126,               /* __LINE__ expansion too long */

	/* undo errors */
	ERR_UNDOVF   = 200,                /* operation is too big for undo log */
	ERR_NOUNDO   = 201,                         /* no more undo information */
	ERR_ICUNDO   = 202,          /* incomplete undo (no previous savepoint) */

	/* parser errors */
	ERR_REQTOK   = 300,    /* expected token (arg 1) - found something else */
	ERR_REQSYM   = 301,                                  /* expected symbol */
	ERR_REQPRP   = 302,                         /* expected a property name */
	ERR_REQOPN   = 303,                                 /* expected operand */
	ERR_REQARG   = 304,       /* expected comma or closing paren (arg list) */
	ERR_NONODE   = 305,                      /* no space for new parse node */
	ERR_REQOBJ   = 306,                             /* epxected object name */
	ERR_REQEXT   = 307,           /* redefining symbol as external function */
	ERR_REQFCN   = 308,                    /* redefining symbol as function */
	ERR_NOCLASS  = 309,  /* can't use CLASS with function/external function */
	ERR_REQUNO   = 310,                          /* required unary operator */
	ERR_REQBIN   = 311,                         /* required binary operator */
	ERR_INVBIN   = 312,                          /* invalid binary operator */
	ERR_INVASI   = 313,                               /* invalid assignment */
	ERR_REQVAR   = 314,                           /* required variable name */
	ERR_LCLSYN   = 315,        /* required comma or semicolon in local list */
	ERR_REQRBR   = 316,   /* required right brace (eof before end of group) */
	ERR_BADBRK   = 317,                          /* 'break' without 'while' */
	ERR_BADCNT   = 318,                       /* 'continue' without 'while' */
	ERR_BADELS   = 319,                              /* 'else' without 'if' */
	ERR_WEQASI   = 320, /* warning: possible use of '=' where ':=' intended */
	ERR_EOF      = 321,                           /* unexpected end of file */
	ERR_SYNTAX   = 322,                             /* general syntax error */
	ERR_INVOP    = 323,                             /* invalid operand type */
	ERR_NOMEMLC  = 324,             /* no memory for new local symbol table */
	ERR_NOMEMAR  = 325,              /* no memory for argument symbol table */
	ERR_FREDEF   = 326,   /* redefining a function which is already defined */
	ERR_NOSW     = 327,      /* 'case' or 'default' and not in switch block */
	ERR_SWRQCN   = 328,           /* constant required in switch case value */
	ERR_REQLBL   = 329,                        /* label required for 'goto' */
	ERR_NOGOTO   = 330,                       /* 'goto' label never defined */
	ERR_MANYSC   = 331,                 /* too many superclasses for object */
	ERR_OREDEF   = 332,                      /* redefining symbol as object */
	ERR_PREDEF   = 333,               /* property being redefined in object */
	ERR_BADPVL   = 334,                           /* invalid property value */
	ERR_BADVOC   = 335,                    /* bad vocabulary property value */
	ERR_BADTPL   = 336,       /* bad template property value (need sstring) */
	ERR_LONGTPL  = 337,             /* template base property name too long */
	ERR_MANYTPL  = 338,     /* too many templates (internal compiler limit) */
	ERR_BADCMPD  = 339,   /* bad value for compound word (sstring required) */
	ERR_BADFMT   = 340,     /* bad value for format string (sstring needed) */
	ERR_BADSYN   = 341,     /* invalid value for synonym (sstring required) */
	ERR_UNDFSYM  = 342,                                 /* undefined symbol */
	ERR_BADSPEC  = 343,                                 /* bad special word */
	ERR_NOSELF   = 344,                 /* "self" not valid in this context */
	ERR_STREND   = 345,            /* warning: possible unterminated string */
	ERR_MODRPLX  = 346,    /* modify/replace not allowed with external func */
	ERR_MODFCN   = 347,                 /* modify not allowed with function */
	ERR_MODFWD   = 348,     /* modify/replace not allowed with forward func */
	ERR_MODOBJ   = 349,    /* modify can only be used with a defined object */
	ERR_RPLSPEC  = 350,                 /* warning - replacing specialWords */
	ERR_SPECNIL  = 351,        /* nil only allowed with modify specialWords */
	ERR_BADLCL   = 353,   /* 'local' statement must precede executable code */
	ERR_IMPPROP  = 354,          /* implied verifier '%s' is not a property */
	ERR_BADTPLF  = 355,                    /* invalid command template flag */
	ERR_NOTPLFLG  = 356,      /* flags are not allowed with old file format */
	ERR_AMBIGBIN  = 357,          /* warning: operator '%s' could be binary */
	ERR_PIA       = 358,          /* warning: possibly incorrect assignment */
	ERR_BADSPECEXPR = 359,                /* invalid speculation evaluation */

	/* code generation errors */
	ERR_OBJOVF   = 400,     /* object cannot grow any bigger - code too big */
	ERR_NOLBL    = 401,                  /* no more temporary labels/fixups */
	ERR_LBNOSET  = 402,                 /* (internal error) label never set */
	ERR_INVLSTE  = 403,                /* invalid datatype for list element */
	ERR_MANYDBG  = 404,  /* too many debugger line records (internal limit) */

	/* vocabulary setup errors */
	ERR_VOCINUS  = 450,            /* vocabulary being redefined for object */
	ERR_VOCMNPG  = 451,          /* too many vocwdef pages (internal limit) */
	ERR_VOCREVB  = 452,                             /* redefining same verb */
	ERR_VOCREVB2 = 453,             /* redefining same verb - two arguments */

	/* set-up errors */
	ERR_LOCNOBJ  = 500,           /* location of object %s is not an object */
	ERR_CNTNLST  = 501,                /* contents of object %s is not list */
	ERR_SUPOVF   = 502,           /* overflow trying to build contents list */
	ERR_RQOBJNF  = 503,                     /* required object %s not found */
	ERR_WRNONF   = 504,                    /* warning - object %s not found */
	ERR_MANYBIF  = 505,     /* too many built-in functions (internal error) */

	/* fio errors */
	ERR_OPWGAM    = 600,                 /* unable to open game for writing */
	ERR_WRTGAM    = 601,                      /* error writing to game file */
	ERR_FIOMSC    = 602,             /* too many sc's for writing in fiowrt */
	ERR_UNDEFF    = 603,                              /* undefined function */
	ERR_UNDEFO    = 604,                                /* undefined object */
	ERR_UNDEF     = 605,                         /* undefined symbols found */
	ERR_OPRGAM    = 606,                 /* unable to open game for reading */
	ERR_RDGAM     = 607,                         /* error reading game file */
	ERR_BADHDR    = 608,    /* file has invalid header - not TADS game file */
	ERR_UNKRSC    = 609,              /* unknown resource type in .gam file */
	ERR_UNKOTYP   = 610,             /* unknown object type in OBJ resource */
	ERR_BADVSN    = 611,    /* file saved by different incompatible version */
	ERR_LDGAM     = 612,                  /* error loading object on demand */
	ERR_LDBIG     = 613, /* object too big for load region (prob. internal) */
	ERR_UNXEXT    = 614,                /* did not expect external function */
	ERR_WRTVSN    = 615,     /* compiler cannot write the requested version */
	ERR_VNOCTAB   = 616,        /* format version cannot be used with -ctab */
	ERR_BADHDRRSC = 617,         /* invalid resource file header in file %s */
	ERR_RDRSC     = 618,               /* error reading resource file "xxx" */

	/* character mapping errors */
	ERR_CHRNOFILE = 700,           /* unable to load character mapping file */

	/* user interrupt */
	ERR_USRINT    = 990,      /* user requested cancel of current operation */

	/* run-time errors */
	ERR_STKOVF    = 1001,                                 /* stack overflow */
	ERR_HPOVF     = 1002,                                  /* heap overflow */
	ERR_REQNUM    = 1003,                         /* numeric value required */
	ERR_STKUND    = 1004,                                /* stack underflow */
	ERR_REQLOG    = 1005,                         /* logical value required */
	ERR_INVCMP    = 1006,     /* invalid datatypes for magnitude comparison */
	ERR_REQSTR    = 1007,                          /* string value required */
	ERR_INVADD    = 1008,             /* invalid datatypes for '+' operator */
	ERR_INVSUB    = 1009,      /* invalid datatypes for binary '-' operator */
	ERR_REQVOB    = 1010,                           /* require object value */
	ERR_REQVFN    = 1011,                      /* required function pointer */
	ERR_REQVPR    = 1012,                 /* required property number value */

	/* non-error conditions:  run-time EXIT, ABORT, ASKIO, ASKDO */
	ERR_RUNEXIT    = 1013,                     /* 'exit' statement executed */
	ERR_RUNABRT    = 1014,                    /* 'abort' statement executed */
	ERR_RUNASKD    = 1015,                    /* 'askdo' statement executed */
	ERR_RUNASKI    = 1016,           /* 'askio' executed; int arg 1 is prep */
	ERR_RUNQUIT    = 1017,                               /* 'quit' executed */
	ERR_RUNRESTART = 1018,                              /* 'reset' executed */
	ERR_RUNEXITOBJ = 1019,                            /* 'exitobj' executed */

	ERR_REQVLS     = 1020,                             /* list value required */
	ERR_LOWINX     = 1021,              /* index value too low (must be >= 1) */
	ERR_HIGHINX    = 1022,  /* index value too high (must be <= length(list)) */
	ERR_INVTBIF    = 1023,              /* invalid type for built-in function */
	ERR_INVVBIF    = 1024,             /* invalid value for built-in function */
	ERR_BIFARGC    = 1025,           /* wrong number of arguments to built-in */
	ERR_ARGC       = 1026,      /* wrong number of arguments to user function */
	ERR_FUSEVAL    = 1027,     /* string/list not allowed for fuse/daemon arg */
	ERR_BADSETF    = 1028,      /* internal error in setfuse/setdaemon/notify */
	ERR_MANYFUS    = 1029,                                  /* too many fuses */
	ERR_MANYDMN    = 1030,                                /* too many daemons */
	ERR_MANYNFY    = 1031,                              /* too many notifiers */
	ERR_NOFUSE     = 1032,                       /* fuse not found in remfuse */
	ERR_NODMN      = 1033,                   /* daemon not found in remdaemon */
	ERR_NONFY      = 1034,                  /* notifier not found in unnotify */
	ERR_BADREMF    = 1035,    /* internal error in remfuse/remdaemon/unnotify */
	ERR_DMDLOOP    = 1036,     /* load-on-demand loop: property not being set */
	ERR_UNDFOBJ    = 1037,             /* undefined object in vocabulary tree */
	ERR_BIFCSTR    = 1038,            /* c-string conversion overflows buffer */
	ERR_INVOPC     = 1039,                                  /* invalid opcode */
	ERR_RUNNOBJ    = 1040,     /* runtime error: property taken of non-object */
	ERR_EXTLOAD    = 1041,           /* unable to load external function "%s" */
	ERR_EXTRUN     = 1042,          /* error executing external function "%s" */
	ERR_CIRCSYN    = 1043,                                /* circular synonym */
	ERR_DIVZERO    = 1044,                                  /* divide by zero */
	ERR_BADDEL     = 1045,      /* can only delete objects created with "new" */
	ERR_BADNEWSC   = 1046,     /* superclass for "new" cannot be a new object */
	ERR_VOCSTK     = 1047,              /* insufficient space in parser stack */
	ERR_BADFILE    = 1048,                             /* invalid file handle */

	ERR_RUNEXITPRECMD = 1049,                       /* exited from preCommand */

	/* run-time parser errors */
	ERR_PRS_SENT_UNK    = 1200,          /* sentence structure not recognized */
	ERR_PRS_VERDO_FAIL  = 1201,                           /* verDoVerb failed */
	ERR_PRS_VERIO_FAIL  = 1202,                           /* verIoVerb failed */
	ERR_PRS_NO_VERDO    = 1203,             /* no verDoVerb for direct object */
	ERR_PRS_NO_VERIO    = 1204,             /* no verIoVerb for direct object */
	ERR_PRS_VAL_DO_FAIL = 1205,            /* direct object validation failed */
	ERR_PRS_VAL_IO_FAIL = 1206,          /* indirect object validation failed */

	/* compiler/runtime/debugger driver errors */
	ERR_USAGE     = 1500,                                /* invalid usage */
	ERR_OPNINP    = 1501,                     /* error opening input file */
	ERR_NODBG     = 1502,              /* game not compiled for debugging */
	ERR_ERRFIL    = 1503,            /* unable to open error capture file */
	ERR_PRSCXSIZ  = 1504,            /* parse pool + local size too large */
	ERR_STKSIZE   = 1505,                         /* stack size too large */
	ERR_OPNSTRFIL = 1506,            /* error opening string capture file */
	ERR_INVCMAP   = 1507,                   /* invalid character map file */

	/* debugger errors */
	ERR_BPSYM         = 2000,                 /* symbol not found for breakpoint */
	ERR_BPPROP        = 2002,             /* breakpoint symbol is not a property */
	ERR_BPFUNC        = 2003,             /* breakpoint symbol is not a function */
	ERR_BPNOPRP       = 2004,              /* property is not defined for object */
	ERR_BPPRPNC       = 2005,                            /* property is not code */
	ERR_BPSET         = 2006,         /* breakpoint already set at this location */
	ERR_BPNOTLN       = 2007,     /* breakpoint is not at a line (OPCLINE instr) */
	ERR_MANYBP        = 2008,                            /* too many breakpoints */
	ERR_BPNSET        = 2009,            /* breakpoint to be deleted was not set */
	ERR_DBGMNSY       = 2010,  /* too many symbols in debug expression (int lim) */
	ERR_NOSOURC       = 2011,                   /* unable to find source file %s */
	ERR_WTCHLCL       = 2012,        /* illegal to assign to local in watch expr */
	ERR_INACTFR       = 2013, /* inactive frame (expression value not available) */
	ERR_MANYWX        = 2014,                      /* too many watch expressions */
	ERR_WXNSET        = 2015,                              /* watchpoint not set */
	ERR_EXTRTXT       = 2016,               /* extraneous text at end of command */
	ERR_BPOBJ         = 2017,              /* breakpoint symbol is not an object */
	ERR_DBGINACT      = 2018,                          /* debugger is not active */
	ERR_BPINUSE       = 2019,                      /* breakpoint is already used */
	ERR_RTBADSPECEXPR = 2020,                  /* invalid speculative expression */
	ERR_NEEDLIN2      = 2021,     /* -ds2 information not found - must recompile */

	/* usage error messages */
	ERR_TCUS1         = 3000,                          /* first tc usage message */
	ERR_TCUSL         = 3024,                           /* last tc usage message */
	ERR_TCTGUS1       = 3030,                         /* first tc toggle message */
	ERR_TCTGUSL       = 3032,
	ERR_TCZUS1        = 3040,            /* first tc -Z suboptions usage message */
	ERR_TCZUSL        = 3041,
	ERR_TC1US1        = 3050,            /* first tc -1 suboptions usage message */
	ERR_TC1USL        = 3058,
	ERR_TCMUS1        = 3070,            /* first tc -m suboptions usage message */
	ERR_TCMUSL        = 3076,
	ERR_TCVUS1        = 3080,                /* first -v suboption usage message */
	ERR_TCVUSL        = 3082,
	ERR_TRUSPARM      = 3099,
	ERR_TRUS1         = 3100,                          /* first tr usage message */
	ERR_TRUSL         = 3117,
	ERR_TRUSFT1       = 3118,                       /* first tr "footer" message */
	ERR_TRUSFTL       = 3119,                        /* last tr "footer" message */
	ERR_TRSUS1        = 3150,            /* first tr -s suboptions usage message */
	ERR_TRSUSL        = 3157,
	ERR_TDBUSPARM     = 3199,
	ERR_TDBUS1        = 3200,                         /* first tdb usage message */
	ERR_TDBUSL        = 3214,                          /* last tdb usage message */

	/* TR 16-bit MSDOS-specific usage messages */
	ERR_TRUS_DOS_1   = 3300,
	ERR_TRUS_DOS_L   = 3300,

	/* TR 32-bit MSDOS console mode usage messages */
	ERR_TRUS_DOS32_1 = 3310,
	ERR_TRUS_DOS32_L = 3312,

	/* TADS/Graphic errors */
	ERR_GNOFIL       = 4001,                      /* can't find graphics file %s */
	ERR_GNORM        = 4002,                               /* can't find room %s */
	ERR_GNOOBJ       = 4003,                    /* can't find hot spot object %s */
	ERR_GNOICN       = 4004                         /* can't find icon object %s */
};

/*
 *   Special error flag - this is returned from execmd() when preparseCmd
 *   returns a command list.  This indicates to voc1cmd that it should try
 *   the command over again, using the words in the new list.
 */
#define ERR_PREPRSCMDREDO  30000             /* preparseCmd returned a list */
#define ERR_PREPRSCMDCAN   30001    /* preparseCmd returned 'nil' to cancel */

union erradef {
	int   erraint;		// integer argument
	char *errastr;		// text string argument
};

struct errdef {
	errdef *       errprv;               // previous error frame
	int            errcode;              // error code of exception being handled
	char           errfac[ERRFACMAX+1];  // facility of current error
	erradef        erraav[10];           // parameters for error
	int            erraac;               // count of parameters in argc
//    jmp_buf        errbuf;               // jump buffer for current error frame
};

#define ERRBUFSIZ 512

class TADS2;

// seek location record for an error message by number
struct errmfdef {
	uint  errmfnum;   // error number
	size_t errmfseek; // seek location of this message
};

class errcxdef {
public:
	errdef   *errcxptr;               // current error frame
	void     *errcxlgc;               // context for error logging callback
	int       errcxofs;               // offset in argument buffer
	char      errcxbuf[ERRBUFSIZ];    // space for argument strings
	Common::SeekableReadStream *errcxfp;  // message file, if one is being used
	errmfdef *errcxseek;              // seek locations of messages in file
	uint      errcxsksz;              // size of errcxseek array
	size_t    errcxbase;              // offset in physical file of logical error file
	TADS2 *   errcxappctx;            // host application context
public:
	/**
	 *   Format an error message, sprintf-style, using arguments in an
	 *   erradef array (which is passed to the error-logging callback).
	 *   Returns the length of the output string, even if the actual
	 *   output string was truncated because the outbuf was too short.
	 *   (If called with outbufl == 0, nothing will be written out, but
	 *   the size of the buffer needed, minus the terminating null byte,
	 *   will be computed and returned.)
	 */
	static int errfmt(char *outbuf, int outbufl, char *fmt, int argc, erradef *argv);
 public:
	errcxdef() : errcxptr(nullptr), errcxlgc(nullptr), errcxofs(0),
		errcxseek(nullptr), errcxsksz(0), errcxbase(0), errcxappctx(nullptr) {
		Common::fill(&errcxbuf[0], &errcxbuf[ERRBUFSIZ], '\0');
	}

	/**
	 * Error logging method
	 */
	void errcxlog(void *ctx0, char *fac, int err, int argc, erradef *argv);
};

// begin protected code
#define ERRBEGIN(ctx) \
  { \
	errdef fr_; \
	if ((fr_.errcode = setjmp(fr_.errbuf)) == 0) \
	{ \
	  fr_.errprv = (ctx)->errcxptr; \
	  (ctx)->errcxptr = &fr_;

// end protected code, begin error handler
#define ERRCATCH(ctx, e) \
	  assert(1==1 && (ctx)->errcxptr != fr_.errprv); \
	  (ctx)->errcxptr = fr_.errprv; \
	} \
	else \
	{ \
	  assert(2==2 && (ctx)->errcxptr != fr_.errprv); \
	  (e) = fr_.errcode; \
	  (ctx)->errcxptr = fr_.errprv;

// retrieve argument (int, string) in current error frame
#define errargint(argnum) (fr_.erraav[argnum].erraint)
#define errargstr(argnum) (fr_.erraav[argnum].errastr)


#define ERREND(ctx) \
	} \
  }

// end protected code, begin cleanup (no handling; just cleaning up)
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



// argument types for errors with arguments
#define ERRTINT  erraint
#define ERRTSTR  errastr

// set argument count in error frame
#define errargc(ctx,cnt) ((ctx)->errcxptr->erraac=(cnt))

// enter string argument; returns pointer to argument used in errargv
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

// signal an error with argument count already set
#ifdef ERR_NO_MACRO
void errsign(errcxdef *ctx, int e, char *facility);
#else /* ERR_NO_MACRO */
# ifdef DEBUG
void errjmp(jmp_buf buf, int e);
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


// signal an error with no arguments
#ifdef ERR_NO_MACRO
void errsigf(errcxdef *ctx, char *facility, int err);
#else /* ERR_NO_MACRO */
#define errsigf(ctx, fac, e) (errargc(ctx,0),errsign(ctx,e,fac))
#endif /* ERR_NO_MACRO */

// signal an error with one argument
#define errsigf1(ctx, fac, e, typ1, arg1) \
  (errargv(ctx,0,typ1,arg1),errargc(ctx,1),errsign(ctx,e,fac))

// signal an error with two arguments
#define errsigf2(ctx, fac, e, typ1, arg1, typ2, arg2) \
  (errargv(ctx,0,typ1,arg1), errargv(ctx,1,typ2,arg2), \
   errargc(ctx,2), errsign(ctx,e,fac))

// resignal the current error - only usable within exception handlers
#ifdef ERR_NO_MACRO
void errrse1(errcxdef *ctx, errdef *fr);
# define errrse(ctx) errrse1(ctx, &fr_)
#else /* ERR_NO_MACRO */

// void errrse(errcxdef *ctx);
# define errrse(ctx) \
  (errargc(ctx, fr_.erraac),\
   memcpy((ctx)->errcxptr->erraav, fr_.erraav, \
	(size_t)(fr_.erraac*sizeof(erradef))),\
   errsign(ctx, fr_.errcode, fr_.errfac))

#endif /* ERR_NO_MACRO */

/**
 *   For use in an error handler (ERRCATCH..ERREND) only: Copy the
 *   parameters from the error currently being handled to the enclosing
 *   frame.  This is useful when "keeping" an error being handled - i.e.,
 *   the arguments will continue to be used outside of the
 *   ERRCATCH..ERREND code.
 */
#define errkeepargs(ctx) errcopyargs(ctx, &fr_)

/**
 *   copy the parameters for an error from another frame into the current
 *   frame - this can be used when we want to be able to display an error
 *   that occurred in an inner frame within code that is protected by a
 *   new enclosing error frame
 */
#define errcopyargs(ctx, fr) \
   (errargc((ctx), (fr)->erraac), \
	memcpy((ctx)->errcxptr->erraav, (fr)->erraav, \
		   (size_t)((fr)->erraac*sizeof(erradef))))

// log error that's been caught, using arguments already caught
#define errclog(ctx) \
 ((*(ctx)->errcxlog)((ctx)->errcxlgc,fr_.errfac,fr_.errcode,\
  fr_.erraac,fr_.erraav))

// log an error that's been set up but not signalled yet
#define errprelog(ctx, err) \
 ((*(ctx)->errcxlog)((ctx)->errcxlgc,(ctx)->errcxptr->errfac,\
   err,(ctx)->errcxptr->erraac,\
   (ctx)->errcxptr->erraav))

// log an error (no signalling, just reporting)
#ifdef ERR_NO_MACRO
void errlogn(errcxdef *ctx, int err, char *facility);
#else /* ERR_NO_MACRO */

#define errlogn(ctx,err,fac) \
 ((ctx)->errcxofs=0,\
  (*(ctx)->errcxlog)((ctx)->errcxlgc,fac,err,(ctx)->errcxptr->erraac,\
  (ctx)->errcxptr->erraav))

#endif /* ERR_NO_MACRO */

// log an error with no arguments
#ifdef ERR_NO_MACRO
void errlogf(errcxdef *ctx, char *facility, int err);
#else /* ERR_NO_MACRO */

// void errlogf(errcxdef *ctx, char *facility, int err);
#define errlogf(ctx,fac,err) (errargc(ctx,0),errlogn(ctx,err,fac))

#endif /* ERR_NO_MACRO */

// log an error with one argument
#define errlogf1(ctx, fac, e, typ1, arg1) \
 (errargv(ctx,0,typ1,arg1),errargc(ctx,1),errlogn(ctx,e,fac))

// log an error with two arguments
#define errlogf2(ctx, fac, e, typ1, arg1, typ2, arg2) \
 (errargv(ctx,0,typ1,arg1),errargv(ctx,1,typ2,arg2),\
  errargc(ctx,2),errlogn(ctx,e,fac))

/**
 * For compatility with old facility-free mechanism, signal with facility "TADS"
 */
#define errsig(ctx, err) errsigf(ctx, "TADS", err)
#define errsig1(c, e, t, a) errsigf1(c,"TADS",e,t,a)
//#define errsig2(c, e, t1, a1, t2, a2) errsigf2(c,"TADS",e,t1,a1,t2,a2)
#define errlog(c, e) errlogf(c, "TADS", e)
#define errlog1(c, e, t, a) errlogf1(c,"TADS",e,t,a)
#define errlog2(c, e, t1, a1, t2, a2) errlogf2(c,"TADS",e,t1,a1,t2,a2)

#define errsig2(c, e, t1, a1, t2, a2) error("Error occurred")

// get the text of an error
void errmsg(errcxdef *ctx, char *outbuf, uint outbufl, uint err);

// initialize error subsystem, opening error message file if necessary
void errini(errcxdef *ctx, Common::SeekableReadStream *fp);

// allocate and initialize error context, free error context
errcxdef *lerini();
void      lerfre(errcxdef *ctx);

// error message structure - number + text
struct errmdef {
	uint   errmerr;		// error number
	char  *errmtxt;     // text of error message
};

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
