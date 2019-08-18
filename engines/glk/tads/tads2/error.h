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

/* Error handling
 * This package defines a set of macros that allows code to raise and
 * handle exceptions.A macro is provided which signals an error, which
 * does a non - local goto to the innermost enclosing exception handler.
 * A set of macros sets up exception handling code.
 * 
 * To catch exceptions that occur inside a block of code(i.e., in the
 * code or in any subroutines called by the code), begin the block with
 * ERRBEGIN.At the end of the protected code, place the exception
 * handler, which starts with ERRCATCH.At the end of the exception
 * handler, place ERREND.If no exception occurs, execution goes
 * through the protected code, then resumes at the code following
 * the ERREND.
 * 
 * The exception handler can signal another error, which will cause
 * the next enclosing frame to catch the error.Alternatively, if
 * the exception handler doesn't signal an error or return, execution
 * continues at the code following the ERREND.Exceptions that are
 * signalled during exception handling will be caught by the next
 * enclosing frame, unless the exception handler code is itself
 * protected by another ERRBEGIN - ERREND block.
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

#ifndef GLK_TADS_TADS2_ERROR
#define GLK_TADS_TADS2_ERROR

#include "glk/tads/tads2/lib.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/*
 *   for compatility with old facility-free mechanism, signal with
 *   facility "TADS" 
 */
#define errsig(ctx, err) errsigf(ctx, "TADS", err)
#define errsig1(c, e, t, a) errsigf1(c,"TADS",e,t,a)
#define errsig2(c, e, t1, a1, t2, a2) errsigf2(c,"TADS",e,t1,a1,t2,a2)
#define errlog(c, e) errlogf(c, "TADS", e)
#define errlog1(c, e, t, a) errlogf1(c,"TADS",e,t,a)
#define errlog2(c, e, t1, a1, t2, a2) errlogf2(c,"TADS",e,t1,a1,t2,a2)



/*---------------------------- TADS Error Codes ----------------------------*/
/* memory/cache manager errors */
#define ERR_NOMEM   1                                      /* out of memory */
#define ERR_FSEEK   2                              /* error seeking in file */
#define ERR_FREAD   3                            /* error reading from file */
#define ERR_NOPAGE  4                                 /* no more page slots */
#define ERR_REALCK  5           /* attempting to reallocate a locked object */
#define ERR_SWAPBIG 6     /* swapfile limit reached - out of virtual memory */
#define ERR_FWRITE  7                                 /* error writing file */
#define ERR_SWAPPG  8                     /* exceeded swap page table limit */
#define ERR_CLIUSE  9      /* requested client object number already in use */
#define ERR_CLIFULL 10                      /* client mapping table is full */
#define ERR_NOMEM1  11   /* swapping/garbage collection failed to find enuf */
#define ERR_NOMEM2  12            /* no memory to resize (expand) an object */
#define ERR_OPSWAP  13                          /* unable to open swap file */
#define ERR_NOHDR   14                     /* can't get a new object header */
#define ERR_NOLOAD  15   /* mcm cannot find object to load (internal error) */
#define ERR_LCKFRE  16     /* attempting to free a locked object (internal) */
#define ERR_INVOBJ  17                                    /* invalid object */
#define ERR_BIGOBJ  18  /* object too big - exceeds memory allocation limit */

/* lexical analysis errors */
#define ERR_INVTOK  100                                    /* invalid token */
#define ERR_STREOF  101                /* end of file while scanning string */
#define ERR_TRUNC   102                      /* symbol too long - truncated */
#define ERR_NOLCLSY 103                   /* no space in local symbol table */
#define ERR_PRPDIR  104               /* invalid preprocessor (#) directive */
#define ERR_INCNOFN 105                /* no filename in #include directive */
#define ERR_INCSYN  106                          /* invalid #include syntax */
#define ERR_INCSEAR 107                         /* can't find included file */
#define ERR_INCMTCH 108       /* no matching delimiter in #include filename */
#define ERR_MANYSYM 109                    /* out of space for symbol table */
#define ERR_LONGLIN 110                                    /* line too long */
#define ERR_INCRPT  111           /* header file already included - ignored */
#define ERR_PRAGMA  112                         /* unknown pragma (ignored) */
#define ERR_BADPELSE 113                                /* unexpected #else */
#define ERR_BADENDIF 114                               /* unexpected #endif */
#define ERR_BADELIF 115                                 /* unexpected #elif */
#define ERR_MANYPIF 116                             /* #if nesting too deep */
#define ERR_DEFREDEF  117                 /* #define symbol already defined */
#define ERR_PUNDEF  118                        /* #undef symbol not defined */
#define ERR_NOENDIF 119                                   /* missing #endif */
#define ERR_MACNEST 120                         /* macros nested too deeply */
#define ERR_BADISDEF 121         /* invalid argument for defined() operator */
#define ERR_PIF_NA  122                           /* #if is not implemented */
#define ERR_PELIF_NA 123                        /* #elif is not implemented */
#define ERR_P_ERROR 124                              /* error directive: %s */
#define ERR_LONG_FILE_MACRO 125              /* __FILE__ expansion too long */
#define ERR_LONG_LINE_MACRO 126              /* __LINE__ expansion too long */

/* undo errors */
#define ERR_UNDOVF  200                /* operation is too big for undo log */
#define ERR_NOUNDO  201                         /* no more undo information */
#define ERR_ICUNDO  202          /* incomplete undo (no previous savepoint) */

/* parser errors */
#define ERR_REQTOK  300    /* expected token (arg 1) - found something else */
#define ERR_REQSYM  301                                  /* expected symbol */
#define ERR_REQPRP  302                         /* expected a property name */
#define ERR_REQOPN  303                                 /* expected operand */
#define ERR_REQARG  304       /* expected comma or closing paren (arg list) */
#define ERR_NONODE  305                      /* no space for new parse node */
#define ERR_REQOBJ  306                             /* epxected object name */
#define ERR_REQEXT  307           /* redefining symbol as external function */
#define ERR_REQFCN  308                    /* redefining symbol as function */
#define ERR_NOCLASS 309  /* can't use CLASS with function/external function */
#define ERR_REQUNO  310                          /* required unary operator */
#define ERR_REQBIN  311                         /* required binary operator */
#define ERR_INVBIN  312                          /* invalid binary operator */
#define ERR_INVASI  313                               /* invalid assignment */
#define ERR_REQVAR  314                           /* required variable name */
#define ERR_LCLSYN  315        /* required comma or semicolon in local list */
#define ERR_REQRBR  316   /* required right brace (eof before end of group) */
#define ERR_BADBRK  317                          /* 'break' without 'while' */
#define ERR_BADCNT  318                       /* 'continue' without 'while' */
#define ERR_BADELS  319                              /* 'else' without 'if' */
#define ERR_WEQASI  320 /* warning: possible use of '=' where ':=' intended */
#define ERR_EOF     321                           /* unexpected end of file */
#define ERR_SYNTAX  322                             /* general syntax error */
#define ERR_INVOP   323                             /* invalid operand type */
#define ERR_NOMEMLC 324             /* no memory for new local symbol table */
#define ERR_NOMEMAR 325              /* no memory for argument symbol table */
#define ERR_FREDEF  326   /* redefining a function which is already defined */
#define ERR_NOSW    327      /* 'case' or 'default' and not in switch block */
#define ERR_SWRQCN  328           /* constant required in switch case value */
#define ERR_REQLBL  329                        /* label required for 'goto' */
#define ERR_NOGOTO  330                       /* 'goto' label never defined */
#define ERR_MANYSC  331                 /* too many superclasses for object */
#define ERR_OREDEF  332                      /* redefining symbol as object */
#define ERR_PREDEF  333               /* property being redefined in object */
#define ERR_BADPVL  334                           /* invalid property value */
#define ERR_BADVOC  335                    /* bad vocabulary property value */
#define ERR_BADTPL  336       /* bad template property value (need sstring) */
#define ERR_LONGTPL 337             /* template base property name too long */
#define ERR_MANYTPL 338     /* too many templates (internal compiler limit) */
#define ERR_BADCMPD 339   /* bad value for compound word (sstring required) */
#define ERR_BADFMT  340     /* bad value for format string (sstring needed) */
#define ERR_BADSYN  341     /* invalid value for synonym (sstring required) */
#define ERR_UNDFSYM 342                                 /* undefined symbol */
#define ERR_BADSPEC 343                                 /* bad special word */
#define ERR_NOSELF  344                 /* "self" not valid in this context */
#define ERR_STREND  345            /* warning: possible unterminated string */
#define ERR_MODRPLX 346    /* modify/replace not allowed with external func */
#define ERR_MODFCN  347                 /* modify not allowed with function */
#define ERR_MODFWD  348     /* modify/replace not allowed with forward func */
#define ERR_MODOBJ  349    /* modify can only be used with a defined object */
#define ERR_RPLSPEC 350                 /* warning - replacing specialWords */
#define ERR_SPECNIL 351        /* nil only allowed with modify specialWords */
#define ERR_BADLCL  353   /* 'local' statement must precede executable code */
#define ERR_IMPPROP 354          /* implied verifier '%s' is not a property */
#define ERR_BADTPLF 355                    /* invalid command template flag */
#define ERR_NOTPLFLG 356      /* flags are not allowed with old file format */
#define ERR_AMBIGBIN 357          /* warning: operator '%s' could be binary */
#define ERR_PIA     358           /* warning: possibly incorrect assignment */
#define ERR_BADSPECEXPR 359               /* invalid speculation evaluation */

/* code generation errors */
#define ERR_OBJOVF  400     /* object cannot grow any bigger - code too big */
#define ERR_NOLBL   401                  /* no more temporary labels/fixups */
#define ERR_LBNOSET 402                 /* (internal error) label never set */
#define ERR_INVLSTE 403                /* invalid datatype for list element */
#define ERR_MANYDBG 404  /* too many debugger line records (internal limit) */

/* vocabulary setup errors */
#define ERR_VOCINUS 450            /* vocabulary being redefined for object */
#define ERR_VOCMNPG 451          /* too many vocwdef pages (internal limit) */
#define ERR_VOCREVB 452                             /* redefining same verb */
#define ERR_VOCREVB2 453            /* redefining same verb - two arguments */

/* set-up errors */
#define ERR_LOCNOBJ 500           /* location of object %s is not an object */
#define ERR_CNTNLST 501                /* contents of object %s is not list */
#define ERR_SUPOVF  502           /* overflow trying to build contents list */
#define ERR_RQOBJNF 503                     /* required object %s not found */
#define ERR_WRNONF  504                    /* warning - object %s not found */
#define ERR_MANYBIF 505     /* too many built-in functions (internal error) */

/* fio errors */
#define ERR_OPWGAM  600                  /* unable to open game for writing */
#define ERR_WRTGAM  601                       /* error writing to game file */
#define ERR_FIOMSC  602              /* too many sc's for writing in fiowrt */
#define ERR_UNDEFF  603                               /* undefined function */
#define ERR_UNDEFO  604                                 /* undefined object */
#define ERR_UNDEF   605                          /* undefined symbols found */
#define ERR_OPRGAM  606                  /* unable to open game for reading */
#define ERR_RDGAM   607                          /* error reading game file */
#define ERR_BADHDR  608     /* file has invalid header - not TADS game file */
#define ERR_UNKRSC  609               /* unknown resource type in .gam file */
#define ERR_UNKOTYP 610              /* unknown object type in OBJ resource */
#define ERR_BADVSN  611     /* file saved by different incompatible version */
#define ERR_LDGAM   612                   /* error loading object on demand */
#define ERR_LDBIG   613  /* object too big for load region (prob. internal) */
#define ERR_UNXEXT  614                 /* did not expect external function */
#define ERR_WRTVSN  615      /* compiler cannot write the requested version */
#define ERR_VNOCTAB 616         /* format version cannot be used with -ctab */
#define ERR_BADHDRRSC 617        /* invalid resource file header in file %s */
#define ERR_RDRSC   618                /* error reading resource file "xxx" */

/* character mapping errors */
#define ERR_CHRNOFILE 700          /* unable to load character mapping file */

/* user interrupt */
#define ERR_USRINT  990       /* user requested cancel of current operation */

/* run-time errors */
#define ERR_STKOVF  1001                                  /* stack overflow */
#define ERR_HPOVF   1002                                   /* heap overflow */
#define ERR_REQNUM  1003                          /* numeric value required */
#define ERR_STKUND  1004                                 /* stack underflow */
#define ERR_REQLOG  1005                          /* logical value required */
#define ERR_INVCMP  1006      /* invalid datatypes for magnitude comparison */
#define ERR_REQSTR  1007                           /* string value required */
#define ERR_INVADD  1008              /* invalid datatypes for '+' operator */
#define ERR_INVSUB  1009       /* invalid datatypes for binary '-' operator */
#define ERR_REQVOB  1010                            /* require object value */
#define ERR_REQVFN  1011                       /* required function pointer */
#define ERR_REQVPR  1012                  /* required property number value */

/* non-error conditions:  run-time EXIT, ABORT, ASKIO, ASKDO */
#define ERR_RUNEXIT 1013                       /* 'exit' statement executed */
#define ERR_RUNABRT 1014                      /* 'abort' statement executed */
#define ERR_RUNASKD 1015                      /* 'askdo' statement executed */
#define ERR_RUNASKI 1016             /* 'askio' executed; int arg 1 is prep */
#define ERR_RUNQUIT 1017                                 /* 'quit' executed */
#define ERR_RUNRESTART 1018                             /* 'reset' executed */
#define ERR_RUNEXITOBJ 1019                           /* 'exitobj' executed */

#define ERR_REQVLS  1020                             /* list value required */
#define ERR_LOWINX  1021              /* index value too low (must be >= 1) */
#define ERR_HIGHINX 1022  /* index value too high (must be <= length(list)) */
#define ERR_INVTBIF 1023              /* invalid type for built-in function */
#define ERR_INVVBIF 1024             /* invalid value for built-in function */
#define ERR_BIFARGC 1025           /* wrong number of arguments to built-in */
#define ERR_ARGC    1026      /* wrong number of arguments to user function */
#define ERR_FUSEVAL 1027     /* string/list not allowed for fuse/daemon arg */
#define ERR_BADSETF 1028      /* internal error in setfuse/setdaemon/notify */
#define ERR_MANYFUS 1029                                  /* too many fuses */
#define ERR_MANYDMN 1030                                /* too many daemons */
#define ERR_MANYNFY 1031                              /* too many notifiers */
#define ERR_NOFUSE  1032                       /* fuse not found in remfuse */
#define ERR_NODMN   1033                   /* daemon not found in remdaemon */
#define ERR_NONFY   1034                  /* notifier not found in unnotify */
#define ERR_BADREMF 1035    /* internal error in remfuse/remdaemon/unnotify */
#define ERR_DMDLOOP 1036     /* load-on-demand loop: property not being set */
#define ERR_UNDFOBJ 1037             /* undefined object in vocabulary tree */
#define ERR_BIFCSTR 1038            /* c-string conversion overflows buffer */
#define ERR_INVOPC  1039                                  /* invalid opcode */
#define ERR_RUNNOBJ 1040     /* runtime error: property taken of non-object */
#define ERR_EXTLOAD 1041           /* unable to load external function "%s" */
#define ERR_EXTRUN  1042          /* error executing external function "%s" */
#define ERR_CIRCSYN 1043                                /* circular synonym */
#define ERR_DIVZERO 1044                                  /* divide by zero */
#define ERR_BADDEL  1045      /* can only delete objects created with "new" */
#define ERR_BADNEWSC 1046    /* superclass for "new" cannot be a new object */
#define ERR_VOCSTK  1047              /* insufficient space in parser stack */
#define ERR_BADFILE 1048                             /* invalid file handle */

#define ERR_RUNEXITPRECMD 1049                    /* exited from preCommand */

/* run-time parser errors */
#define ERR_PRS_SENT_UNK  1200         /* sentence structure not recognized */
#define ERR_PRS_VERDO_FAIL 1201                         /* verDoVerb failed */
#define ERR_PRS_VERIO_FAIL 1202                         /* verIoVerb failed */
#define ERR_PRS_NO_VERDO  1203            /* no verDoVerb for direct object */
#define ERR_PRS_NO_VERIO  1204            /* no verIoVerb for direct object */
#define ERR_PRS_VAL_DO_FAIL  1205        /* direct object validation failed */
#define ERR_PRS_VAL_IO_FAIL  1206      /* indirect object validation failed */

/* compiler/runtime/debugger driver errors */
#define ERR_USAGE   1500                                   /* invalid usage */
#define ERR_OPNINP  1501                        /* error opening input file */
#define ERR_NODBG   1502                 /* game not compiled for debugging */
#define ERR_ERRFIL  1503               /* unable to open error capture file */
#define ERR_PRSCXSIZ 1504              /* parse pool + local size too large */
#define ERR_STKSIZE 1505                            /* stack size too large */
#define ERR_OPNSTRFIL 1506             /* error opening string capture file */
#define ERR_INVCMAP 1507                      /* invalid character map file */

/* debugger errors */
#define ERR_BPSYM   2000                 /* symbol not found for breakpoint */
#define ERR_BPPROP  2002             /* breakpoint symbol is not a property */
#define ERR_BPFUNC  2003             /* breakpoint symbol is not a function */
#define ERR_BPNOPRP 2004              /* property is not defined for object */
#define ERR_BPPRPNC 2005                            /* property is not code */
#define ERR_BPSET   2006         /* breakpoint already set at this location */
#define ERR_BPNOTLN 2007     /* breakpoint is not at a line (OPCLINE instr) */
#define ERR_MANYBP  2008                            /* too many breakpoints */
#define ERR_BPNSET  2009            /* breakpoint to be deleted was not set */
#define ERR_DBGMNSY 2010  /* too many symbols in debug expression (int lim) */
#define ERR_NOSOURC 2011                   /* unable to find source file %s */
#define ERR_WTCHLCL 2012        /* illegal to assign to local in watch expr */
#define ERR_INACTFR 2013 /* inactive frame (expression value not available) */
#define ERR_MANYWX  2014                      /* too many watch expressions */
#define ERR_WXNSET  2015                              /* watchpoint not set */
#define ERR_EXTRTXT 2016               /* extraneous text at end of command */
#define ERR_BPOBJ   2017              /* breakpoint symbol is not an object */
#define ERR_DBGINACT 2018                         /* debugger is not active */
#define ERR_BPINUSE 2019                      /* breakpoint is already used */
#define ERR_RTBADSPECEXPR 2020            /* invalid speculative expression */
#define ERR_NEEDLIN2 2021    /* -ds2 information not found - must recompile */

/* usage error messages */
#define ERR_TCUS1   3000                          /* first tc usage message */
#define ERR_TCUSL   3024                           /* last tc usage message */
#define ERR_TCTGUS1 3030                         /* first tc toggle message */
#define ERR_TCTGUSL 3032
#define ERR_TCZUS1  3040            /* first tc -Z suboptions usage message */
#define ERR_TCZUSL  3041
#define ERR_TC1US1  3050            /* first tc -1 suboptions usage message */
#define ERR_TC1USL  3058
#define ERR_TCMUS1  3070            /* first tc -m suboptions usage message */
#define ERR_TCMUSL  3076
#define ERR_TCVUS1  3080                /* first -v suboption usage message */
#define ERR_TCVUSL  3082
#define ERR_TRUSPARM 3099
#define ERR_TRUS1   3100                          /* first tr usage message */
#define ERR_TRUSL   3117
#define ERR_TRUSFT1 3118                       /* first tr "footer" message */
#define ERR_TRUSFTL 3119                        /* last tr "footer" message */
#define ERR_TRSUS1  3150            /* first tr -s suboptions usage message */
#define ERR_TRSUSL  3157
#define ERR_TDBUSPARM 3199
#define ERR_TDBUS1  3200                         /* first tdb usage message */
#define ERR_TDBUSL  3214                          /* last tdb usage message */

/* TR 16-bit MSDOS-specific usage messages */
#define ERR_TRUS_DOS_1 3300
#define ERR_TRUS_DOS_L 3300

/* TR 32-bit MSDOS console mode usage messages */
#define ERR_TRUS_DOS32_1  3310
#define ERR_TRUS_DOS32_L  3312

/* TADS/Graphic errors */
#define ERR_GNOFIL  4001                     /* can't find graphics file %s */
#define ERR_GNORM   4002                              /* can't find room %s */
#define ERR_GNOOBJ  4003                   /* can't find hot spot object %s */
#define ERR_GNOICN  4004                       /* can't find icon object %s */


/*
 *   Special error flag - this is returned from execmd() when preparseCmd
 *   returns a command list.  This indicates to voc1cmd that it should try
 *   the command over again, using the words in the new list. 
 */
#define ERR_PREPRSCMDREDO  30000             /* preparseCmd returned a list */
#define ERR_PREPRSCMDCAN   30001    /* preparseCmd returned 'nil' to cancel */

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
