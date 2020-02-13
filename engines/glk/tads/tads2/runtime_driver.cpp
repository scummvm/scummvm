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

#include "glk/tads/tads2/built_in.h"
#include "glk/tads/tads2/character_map.h"
#include "glk/tads/tads2/command_line.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/file_io.h"
#include "glk/tads/tads2/memory_cache_heap.h"
#include "glk/tads/tads2/os.h"
#include "glk/tads/tads2/play.h"
#include "glk/tads/tads2/post_compilation.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/runtime_app.h"
#include "glk/tads/tads2/tokenizer.h"
#include "glk/tads/tads2/tads2.h"
#include "glk/tads/tads2/vocabulary.h"
#include "glk/tads/os_frob_tads.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* dummy setup function */
void supgnam(char *buf, tokthdef *tab, objnum sc)
{
    strcpy(buf, "???");
}

/* dummy file read functions */
void tok_read_defines(tokcxdef *tctx, osfildef *fp, errcxdef *ec)
{
    errsig(ec, ERR_UNKRSC);
}

/* dummy debugger functions */

int dbgbpset(dbgcxdef *ctx, char *addr, int *bpnum) { return 0; }

int dbgbpat(dbgcxdef *ctx, objnum objn, objnum self,
	uint ofs, int *bpnum, char *bpname, int toggle,
	char *condition, int *did_set) { return 0; }

int dbgbpatid(dbgcxdef *ctx, int bpnum, objnum target, objnum self,
	uint ofs, char *bpname, int toggle, char *cond,
	int *did_set) { return 0; }

int dbgisbp(dbgcxdef *ctx, objnum target, objnum self, uint ofs, int *bpnum) { return 0; }

int dbgisbpena(dbgcxdef *ctx, int bpnum) { return 0; }

int dbgbpdel(dbgcxdef *ctx, int bpnum) { return 0; }

int dbgbpdis(dbgcxdef *ctx, int bpnum, int disable) { return 0; }

int dbgbpsetcond(dbgcxdef *ctx, int bpnum, char *cond) { return 0; }

void dbgbplist(dbgcxdef *ctx, void(*dispfn)(void *ctx, const char *str, int len), void *dispctx) {}

void dbgbpenum(dbgcxdef *ctx, void(*cbfunc)(void *cbctx, int bpnum, const char *desc,
		const char *cond, int disabled), void *cbctx) {}

void dbgbpeach(dbgcxdef *ctx, void(*fn)(void *, int, uchar *, uint), void *fnctx) {}

int dbgbpgetinfo(dbgcxdef *ctx, int bpnum, char *descbuf, size_t descbuflen,
	char *condbuf, size_t condbuflen) { return 0; }

int dbgeval(dbgcxdef *ctx, char *expr,
	void(*dispfn)(void *dispctx, const char *str, int strl),
	void *dispctx, int level, int showtype) { return 0; }

int dbgevalext(dbgcxdef *ctx, char *expr,
	void(*dispfn)(void *dispctx, const char *str, int strl),
	void *dispctx, int level, int showtype, dattyp *dat,
	void(*aggcb)(void *aggctx, const char *subname,
		int subnamelen, const char *relationship),
	void *aggctx, int speculative) { return 0 ;}

void dbgenumlcl(dbgcxdef *ctx, int level,
	void(*func)(void *ctx, const char *lclnam, size_t lclnamlen),
	void *cbctx) {}

int dbgcompile(dbgcxdef *ctx, char *expr, dbgfdef *fr, objnum *objn,
	int speculative) { return 0; }

void dbgwhere(dbgcxdef *ctx, char *buf) {}

int dbgwxset(dbgcxdef *ctx, char *expr, int *wxnum, int level) { return 0; }

int dbgwxdel(dbgcxdef *ctx, int wxnum) { return 0; }

void dbgwxupd(dbgcxdef *ctx,
	void(*dispfn)(void *dispctx, const char *txt, int len),
	void *dispctx) {}

void dbgswitch(struct lindef **linp, struct lindef *newlin) {}

void dbguini(dbgcxdef *ctx, const char *game_filename) {}

void dbguini2(dbgcxdef *ctx) {}

int dbgu_err_resume(dbgcxdef *ctx) { return 0; }

int dbgu_find_src(const char *origname, int origlen,
	char *fullname, size_t full_len, int must_find_file) { return 0; }

void dbgucmd(dbgcxdef *ctx, int bphit, int err, unsigned int *exec_ofs) {}

void dbguquitting(dbgcxdef *ctx) {}

void dbguterm(dbgcxdef *ctx) {}

void dbguerr(dbgcxdef *ctx, int errnum, char *msg) {}

void trchid(void) {}
void trcsho(void) {}

struct runsdef *dbgfrfind(dbgcxdef *ctx, objnum frobj, uint frofs)
{
    VARUSED(frobj);
    VARUSED(frofs);
    errsig(ctx->dbgcxerr, ERR_INACTFR);
    return 0;
}

void dbgss(struct dbgcxdef *ctx, uint ofs, int instr, int err,
           uchar *noreg *p)
{
    VARUSED(ctx);
    VARUSED(ofs);
    VARUSED(instr);
    VARUSED(err);
    VARUSED(p);
}

int dbgstart(struct dbgcxdef *ctx)
{
    VARUSED(ctx);
    return TRUE;
}

/* printf-style formatting */
static void trdptf(const char *fmt, ...)
{
    char buf[256];
    va_list va;

    /* format the string */
    va_start(va, fmt);
    vsprintf(buf, fmt, va);
    va_end(va);

    /* print the formatted buffer */
    os_printz(buf);
}


/*
 *   display a range of usage messages 
 */
static void trdusage_show_range(errcxdef *ec, int msg_first, int msg_last)
{
    int  i;
    char buf[128];

    for (i = msg_first ; i <= msg_last ; ++i)
    {
        errmsg(ec, buf, (uint)sizeof(buf), i);
        trdptf("%s\n", buf);
    }
}


/*
 *   display a range of usage messages, then throw the usage error
 */
static void trdusage_range(errcxdef *ec, int msg_first, int msg_last)
{
    /* show the message range */
    trdusage_show_range(ec, msg_first, msg_last);

    /* signal the usage error */
    errsig(ec, ERR_USAGE);
}

/*
 *   display general run-time usage information 
 */
static void trdusage(errcxdef *ec)
{
    int first;

    /* 
     *   if we have an app display name, display it instead of the
     *   hard-coded text in the message identifying the app 
     */
    first = ERR_TRUS1;
    if (ec->errcxappctx != 0 && ec->errcxappctx->usage_app_name != 0)
    {
        char buf[128];
        char buf2[128];
        erradef argv[1];
        
        /* get the parameterized usage message */
        errmsg(ec, buf, (uint)sizeof(buf), ERR_TRUSPARM);

        /* format in the application name */
        argv[0].errastr = ec->errcxappctx->usage_app_name;
        errfmt(buf2, (int)sizeof(buf2), buf, 1, argv);
        
        /* display it */
        trdptf("%s\n", buf2);

        /* start at the next message */
        ++first;
    }

    /* display the main option list messages */
    trdusage_show_range(ec, first, ERR_TRUSL);

    /* display the OS-specific option messages, if any */
    trdusage_show_range(ec, ERR_TRUS_OS_FIRST, ERR_TRUS_OS_LAST);

    /* display the usage footer messages */
    trdusage_range(ec, ERR_TRUSFT1, ERR_TRUSFTL);
}

/*
 *   display -s suboptions 
 */
static void trdusage_s(errcxdef *ec)
{
    trdusage_range(ec, ERR_TRSUS1, ERR_TRSUSL);
}


static void trdmain1(errcxdef *ec, int argc, char *argv[],
                     appctxdef *appctx, const char *save_ext)
{
    osfildef  *swapfp = (osfildef *)0;
    runcxdef   runctx;
    bifcxdef   bifctx;
    voccxdef   vocctx;
    void     (*bif[100])(struct bifcxdef *, int);
    mcmcxdef  *mctx = 0;
    mcmcx1def *globalctx = 0;
    dbgcxdef   dbg;
    supcxdef   supctx;
    char      *swapname = 0;
    char       swapbuf[OSFNMAX];
    char     **argp;
    char      *arg;
    char      *infile;
    char       infile_abs[OSFNMAX];      /* fully-qualified input file name */
    char       infile_path[OSFNMAX];         /* absolute path to input file */
    const char *exefile;           /* try with executable file if no infile */
    ulong      swapsize = 0xffffffffL;        /* allow unlimited swap space */
    int        swapena = OS_DEFAULT_SWAP_ENABLED;      /* swapping enabled? */
    int        i;
    int        pause = FALSE;                 /* pause after finishing game */
    fiolcxdef  fiolctx;
    noreg int  loadopen = FALSE;
    char       inbuf[OSFNMAX];
    ulong      cachelimit = 0xffffffff;
    ushort     undosiz = TRD_UNDOSIZ;      /* default undo context size 16k */
    objucxdef *undoptr = 0;
    uint       flags;         /* flags used to write the file we're reading */
    objnum     preinit;         /* preinit object, if we need to execute it */
    uint       heapsiz = TRD_HEAPSIZ;
    uint       stksiz = TRD_STKSIZ;
    runsdef   *mystack;
    uchar     *myheap;
    extern osfildef *cmdfile;     /* hacky v1 qa interface - command log fp */
    extern osfildef *logfp;        /* hacky v1 qa interface - output log fp */
    int        preload = FALSE;              /* TRUE => preload all objects */
    ulong      totsize;
    extern voccxdef *main_voc_ctx;
    int        safety_read, safety_write;          /* file I/O safety level */
    char      *restore_file = 0;                    /* .SAV file to restore */
    char      *charmap = 0;                           /* character map file */
    int        charmap_none;       /* explicitly do not use a character set */
    int        doublespace = TRUE;        /* formatter double-space setting */
    
    NOREG((&loadopen))

    /* initialize the output formatter */
    out_init();

    /* set safety level to 2 by default - read any/write current dir only */
    safety_read = safety_write = 2;

    /* no -ctab- yet */
    charmap_none = FALSE;

    /* parse arguments */
    for (i = 1, argp = argv + 1 ; i < argc ; ++argp, ++i)
    {
        arg = *argp;
        if (*arg == '-')
        {
            switch(*(arg+1))
            {
            case 'c':
                if (!strcmp(arg+1, "ctab"))
                {
                    /* get the character mapping table */
                    charmap = cmdarg(ec, &argp, &i, argc, 4, trdusage);
                }
                else if (!strcmp(arg+1, "ctab-"))
                {
                    /* use the default mapping */
                    charmap_none = TRUE;
                }
                else
                    trdusage(ec);
                break;
                     
            case 'r':
                /* restore a game */
                restore_file = cmdarg(ec, &argp, &i, argc, 1, trdusage);
                break;
                
            case 'i':
                qasopn(cmdarg(ec, &argp, &i, argc, 1, trdusage), TRUE);
                break;
                
            case 'o':
                cmdfile = osfopwt(cmdarg(ec, &argp, &i, argc, 1, trdusage),
                                  OSFTCMD);
                break;
                
            case 'l':
                logfp = osfopwt(cmdarg(ec, &argp, &i, argc, 1, trdusage),
                                OSFTCMD);
                break;

            case 'p':
                if (!scumm_stricmp(arg, "-plain"))
                {
                    os_plain();
                    break;
                }
                pause = cmdtog(ec, pause, arg, 1, trdusage);
                break;

            case 'd':
                if (!scumm_strnicmp(arg, "-double", 7))
                {
                    /* get the argument value */
                    doublespace = cmdtog(ec, doublespace, arg, 6, trdusage);

                    /* set the double-space mode in the formatter */
                    out_set_doublespace(doublespace);
                    break;
                }
                break;

            case 's':
                {
                    char *p;

                    /* get the option */
                    p = cmdarg(ec, &argp, &i, argc, 1, trdusage);

                    /* if they're asking for help, display detailed usage */
                    if (*p == '?')
                        trdusage_s(ec);

                    /* get the safety level from the argument */
                    safety_read = *p - '0';
                    safety_write = (*(p+1) != '\0' ? *(p+1) - '0' :
                                    safety_read);

                    /* range-check the values */
                    if (safety_read < 0 || safety_read > 4
                        || safety_write < 0 || safety_write > 4)
                        trdusage_s(ec);

                    /* tell the host system about the setting */
                    if (appctx != 0 && appctx->set_io_safety_level != 0)
                        (*appctx->set_io_safety_level)
                            (appctx->io_safety_level_ctx,
                             safety_read, safety_write);
                }
                break;
                
            case 'm':
                switch(*(arg + 2))
                {
                case 's':
                    stksiz = atoi(cmdarg(ec, &argp, &i, argc, 2, trdusage));
                    break;
                    
                case 'h':
                    heapsiz = atoi(cmdarg(ec, &argp, &i, argc, 2, trdusage));
                    break;
                    
                default:
                    cachelimit = atol(cmdarg(ec, &argp, &i, argc, 1,
                                             trdusage));
                    break;
                }
                break;
                
            case 't':
                /* swap file options:  -tf file, -ts size, -t- (no swap) */
                switch(*(arg+2))
                {
                case 'f':
                    swapname = cmdarg(ec, &argp, &i, argc, 2, trdusage);
                    break;
                    
                case 's':
                    swapsize = atol(cmdarg(ec, &argp, &i, argc, 2, trdusage));
                    break;
                    
                case 'p':
                    preload = cmdtog(ec, preload, arg, 2, trdusage);
                    break;
                    
                default:
                    swapena = cmdtog(ec, swapena, arg, 1, trdusage);
                    break;
                }
                break;
                
            case 'u':
                undosiz = atoi(cmdarg(ec, &argp, &i, argc, 1, trdusage));
                break;
                
            default:
                trdusage(ec);
            }
        }
        else break;
    }

    /* presume we won't take the .gam from the application executable */
    exefile = 0;

    /* get input name argument, and make sure it's the last argument */
    if (i == argc)
    {
        osfildef *fp;
        ulong     curpos;
        ulong     endpos;
        int       use_exe;

        /*
         *   There's no input name argument, so we need to find the game
         *   to play some other way.  First, check to see if we have a
         *   game to restore, and if so whether it has the .GAM name
         *   encoded into it.  Next, look to see if there's a game
         *   attached to the executable file; if so, use it.  If not, see
         *   if the host system wants to provide a name through its
         *   callback.  
         */
        
        /* presume we won't find a game attached to the executable file */
        infile = 0;
        use_exe = FALSE;

        /* 
         *   see if we have a saved game to restore, and it specifies the
         *   GAM file that saved it 
         */
        if (restore_file != 0)
        {
            /* try getting the game name from the restore file */
            if (fiorso_getgame(restore_file, inbuf, sizeof(inbuf)))
            {
                /* got it - use this file */
                infile = inbuf;
            }
        }
        
        /* 
         *   it that didn't work, try to read from os-dependent part of
         *   program being executed 
         */
        if (infile == 0)
        {
            /* try opening the executable file */
            exefile = (argv && argv[0] ? argv[0] : "TRX");
            fp = os_exeseek(exefile, "TGAM");
            if (fp != 0)
            {
                /* see if there's a game file attached to the executable */
                curpos = osfpos(fp);
                osfseek(fp, 0L, OSFSK_END);
                endpos = osfpos(fp);
                osfcls(fp);
                
                /* if we found it, use it */
                if (endpos != curpos)
                    use_exe = TRUE;
            }
        }
            
        /* 
         *   if we didn't find a game in the executable, try the host
         *   system callback 
         */
        if (infile == 0 && !use_exe)
        {
            /* 
             *   ask the host system callback what to do - if we don't
             *   have a host system callback, or the callback 
             */
            if (appctx != 0 && appctx->get_game_name != 0)
            {
                /* call the host system callback */
                if ((*appctx->get_game_name)(appctx->get_game_name_ctx,
                                             inbuf, sizeof(inbuf)))
                {
                    /* the host system provided a name - use it */
                    infile = inbuf;
                }
                else
                {
                    /* 
                     *   the host didn't provide a name - simply display a
                     *   message indicating that no game file has been
                     *   chosen, and return 
                     */
                    trdptf("\n");
                    trdptf("(No game has been selected.)\n");
                    return;
                }
            }
            else
            {
                /* 
                 *   we've run out of ways to get a filename - give the
                 *   user the usage message and quit 
                 */
                trdusage(ec);
            }
        }
    }
    else
    {
        infile = *argp;
        if (i + 1 != argc)
            trdusage(ec);

#ifndef OS_HATES_EXTENSIONS
        /*
         *   If original name exists, use it; otherwise, try adding .GAM.
         *   Note that this code is ifdef'd so that platforms that don't
         *   use filename extensions in the manner conventional for DOS
         *   and Unix won't use this code. 
         */
        if (osfacc(infile))
        {
            strcpy(inbuf, infile);
            os_defext(inbuf, "gam");
            infile = inbuf;
        }
#endif /* !defined(OS_HATES_EXTENSIONS) */
    }
    
    /* open up the swap file */
    if (swapena && swapsize)
    {
        swapfp = os_create_tempfile(swapname, swapbuf);
        if (swapname == 0) swapname = swapbuf;
        if (swapfp == 0) errsig(ec, ERR_OPSWAP);
    }
    
    /* load the character map */
    if (charmap_none)
        cmap_override();
    else if (cmap_load(charmap))
        errsig(ec, ERR_INVCMAP);

    ERRBEGIN(ec)

    /* initialize cache manager context */
    globalctx = mcmini(cachelimit, 128, swapsize, swapfp, swapname, ec);
    mctx = mcmcini(globalctx, 128, fioldobj, &fiolctx,
                   objrevert, (void *)0);
    mctx->mcmcxrvc = mctx;

    /* set up an undo context */
    if (undosiz)
        undoptr = objuini(mctx, undosiz, vocdundo, vocdusz, &vocctx);
    else
        undoptr = (objucxdef *)0;

    /* set up vocabulary context */
    vocini(&vocctx, ec, mctx, &runctx, undoptr, 100, 100, 200);

    /*
     *   save a pointer to the voc context globally, so that certain
     *   external routines (such as Unix-style signal handlers) can reach
     *   it 
     */
    main_voc_ctx = &vocctx;
    
    /* allocate stack and heap */
    totsize = (ulong)stksiz * (ulong)sizeof(runsdef);
    if (totsize != (size_t)totsize)
        errsig1(ec, ERR_STKSIZE, ERRTINT, (uint)(65535/sizeof(runsdef)));
    mystack = (runsdef *)mchalo(ec, (size_t)totsize, "runtime stack");
    myheap = mchalo(ec, heapsiz, "runtime heap");

    /* get the absolute path for the input file */
    if (infile != 0)
        os_get_abs_filename(infile_abs, sizeof(infile_abs), infile);
    else if (exefile != 0)
        os_get_abs_filename(infile_abs, sizeof(infile_abs), exefile);
    else
        infile_abs[0] = '\0';
    os_get_path_name(infile_path, sizeof(infile_path), infile_abs);

    /* set up execution context */
    runctx.runcxerr = ec;
    runctx.runcxmem = mctx;
    runctx.runcxstk = mystack;
    runctx.runcxstop = &mystack[stksiz];
    runctx.runcxsp = mystack;
    runctx.runcxbp = mystack;
    runctx.runcxheap = myheap;
    runctx.runcxhp = myheap;
    runctx.runcxhtop = &myheap[heapsiz];
    runctx.runcxundo = undoptr;
    runctx.runcxbcx = &bifctx;
    runctx.runcxbi = bif;
    runctx.runcxtio = (tiocxdef *)0;
    runctx.runcxdbg = &dbg;
    runctx.runcxvoc = &vocctx;
    runctx.runcxdmd = supcont;
    runctx.runcxdmc = &supctx;
    runctx.runcxext = 0;
    runctx.runcxgamename = infile;
    runctx.runcxgamepath = infile_path;

    /* set up setup context */
    supctx.supcxerr = ec;
    supctx.supcxmem = mctx;
    supctx.supcxtab = (tokthdef *)0;
    supctx.supcxbuf = (uchar *)0;
    supctx.supcxlen = 0;
    supctx.supcxvoc = &vocctx;
    supctx.supcxrun = &runctx;
    
    /* set up debug context */
    dbg.dbgcxtio = (tiocxdef *)0;
    dbg.dbgcxmem = mctx;
    dbg.dbgcxerr = ec;
    dbg.dbgcxtab = (tokthdef *)0;
    dbg.dbgcxfcn = 0;
    dbg.dbgcxdep = 0;
    dbg.dbgcxflg = 0;
    dbg.dbgcxlin = (lindef *)0;                      /* no line sources yet */
    
    /* set up built-in function context */
    bifctx.bifcxerr = ec;
    bifctx.bifcxrun = &runctx;
    bifctx.bifcxtio = (tiocxdef *)0;
    bifctx.bifcxrnd = 0;
    bifctx.bifcxrndset = FALSE;
    bifctx.bifcxappctx = appctx;
    bifctx.bifcxsafetyr = safety_read;
    bifctx.bifcxsafetyw = safety_write;
    bifctx.bifcxsavext = save_ext;

    /* initialize the regular expression parser context */
    re_init(&bifctx.bifcxregex, ec);
    
    /* add the built-in functions, keywords, etc */
    supbif(&supctx, bif, (int)(sizeof(bif)/sizeof(bif[0])));
    
    /* set up status line hack */
    runistat(&vocctx, &runctx, (tiocxdef *)0);

    /* turn on the "busy" cursor before loading */
    os_csr_busy(TRUE);
    
    /* read the game from the binary file */
    fiord(mctx, &vocctx, (struct tokcxdef *)0,
          infile, exefile, &fiolctx, &preinit, &flags,
          (struct tokpdef *)0, (uchar **)0, (uint *)0, (uint *)0,
          (preload ? 2 : 0), appctx, argv[0]);
    loadopen = TRUE;

    /* turn off the "busy" cursor */
    os_csr_busy(FALSE);
    
    /* play the game */
    plygo(&runctx, &vocctx, (tiocxdef *)0, preinit, restore_file);
    
    /* close load file */
    fiorcls(&fiolctx);
    
    if (pause)
    {
        trdptf("[press a key to exit]");
        os_waitc();
        trdptf("\n");
    }
    
    /* close and delete swapfile, if one was opened */
    trd_close_swapfile(&runctx);

    /* make sure the script file is closed, if we have one */
    qasclose();

    ERRCLEAN(ec)
        /* close and delete swapfile, if one was opened */
        trd_close_swapfile(&runctx);
        
        /* close the load file if one was opened */
        if (loadopen)
            fiorcls(&fiolctx);

        /* vocctx is going out of scope - forget the global reference to it */
        main_voc_ctx = 0;

        /* delete the voc context */
        vocterm(&vocctx);

        /* delete the undo context */
        if (undoptr != 0)
            objuterm(undoptr);

        /* release the object cache structures */
        if (mctx != 0)
            mcmcterm(mctx);
        if (globalctx != 0)
            mcmterm(globalctx);
    ERRENDCLN(ec)

    /* vocctx is going out of scope - forget the global reference to it */
    main_voc_ctx = 0;

    /* delete the voc contxt */
    vocterm(&vocctx);

    /* delete the undo context */
    if (undoptr != 0)
        objuterm(undoptr);

    /* release the object cache structures */
    mcmcterm(mctx);
    mcmterm(globalctx);
}

/*
 *   If the OS configuration so desires, use a less technical format for
 *   run-time error messages by leaving out the numeric error code.  Note
 *   that we'll only do this if the error messages are linked directly
 *   into the run-time, since we need the numeric code as a last resort
 *   when the error message may not be present.  
 */
#ifdef OS_SKIP_ERROR_CODES
# ifdef ERR_LINK_MESSAGES
#  define TRDLOGERR_PREFIX "\n[An error has occurred within TADS: "
# endif
#endif

/*
 *   If we didn't define a different error prefix format, use the default
 *   format with the numeric error code. 
 */
#ifndef TRDLOGERR_PREFIX
# define TRDLOGERR_PREFIX "\n[%s-%d: "
#endif

/* log an error */
static void trdlogerr(void *ctx0, const char *fac, int err, int argc, erradef *argv) {
    errcxdef *ctx = (errcxdef *)ctx0;
    char      buf[256];
    char      msg[256];

    /* display the prefix message to the console and log file */
    sprintf(buf, TRDLOGERR_PREFIX, fac, err);
    trdptf("%s", buf);
    out_logfile_print(buf, FALSE);

    /* display the error message text to the console and log file */
    errmsg(ctx, msg, (uint)sizeof(msg), err);
    errfmt(buf, (int)sizeof(buf), msg, argc, argv);
    trdptf("%s]\n", buf);
    out_logfile_print(buf, FALSE);
    out_logfile_print("]", TRUE);
}


/*
 *   close and delete the swap file 
 */
void trd_close_swapfile(runcxdef *runctx)
{
    extern voccxdef *main_voc_ctx;
    mcmcxdef        *mctx;
    mcmcx1def       *globalctx;
    mcscxdef        *mcsctx;

    /* if no run context was supplied, find it from the main voc context */
    if (runctx == 0)
    {
        /* if there is no main voc context, we're out of luck */
        if (main_voc_ctx == 0)
            return;

        /* get the run context */
        runctx = main_voc_ctx->voccxrun;
    }

    /* get the other relevant contexts */
    mctx = runctx->runcxmem;
    globalctx = mctx->mcmcxgl;
    mcsctx = &globalctx->mcmcxswc;

    /* if we have a swap file open, close it */
    if (mcsctx->mcscxfp != 0)
    {
        /* close the file */
        osfcls(mcsctx->mcscxfp);

        /* forget about the file, so we don't try to close it again */
        mcsctx->mcscxfp = (osfildef *)0;
    }

    /* if we have a filename, delete the file */
    if (mcsctx->mcscxfname != 0)
    {
        /* delete the file */
        osfdel_temp(mcsctx->mcscxfname);

        /* forget the filename, so we don't try to delete the file again */
        mchfre(mcsctx->mcscxfname);
        mcsctx->mcscxfname = 0;
    }
}

/* main - called by os main after setting up arguments */
int trdmain(int argc, char *argv[], appctxdef *appctx, const char *save_ext)
{
    errcxdef  errctx;
    int       err;
    osfildef *fp;
    
    errctx.errcxlog = trdlogerr;
    errctx.errcxlgc = &errctx;
    errctx.errcxfp  = (osfildef *)0;
    errctx.errcxofs = 0;
    errctx.errcxappctx = appctx;
    fp = oserrop(argv[0]);
    errini(&errctx, fp);
    
    /* copyright-date-string */
#ifndef NO_T2_COPYRIGHT_NOTICE
    trdptf("%s - A %s TADS %s Interpreter.\n",
           G_tads_oem_app_name, G_tads_oem_display_mode,
           TADS_RUNTIME_VERSION);
    trdptf("%sopyright (c) 1993, 2012 by Michael J. Roberts.\n",
           G_tads_oem_copyright_prefix ? "TADS c" : "C");
    trdptf("%s\n", G_tads_oem_author);
#endif
    
    ERRBEGIN(&errctx)
        trdmain1(&errctx, argc, argv, appctx, save_ext);
    ERRCATCH(&errctx, err)
        /* 
         *   log the error, unless it's usage (in which case we logged it
         *   already) or we're simply quitting the game 
         */
        if (err != ERR_USAGE && err != ERR_RUNQUIT)
            errclog(&errctx);
    
        /* close the error file */
        if (errctx.errcxfp != 0)
            osfcls(errctx.errcxfp);

        /* pause before exiting if the OS desires it */
        os_expause();

        /* return failure unless we're simply quitting the game */
        return (err == ERR_RUNQUIT ? OSEXSUCC : OSEXFAIL);
    ERREND(&errctx)
        
    /* close the error file if we opened it */
    if (errctx.errcxfp != 0)
        osfcls(errctx.errcxfp);

    /* successful completion */
    return(OSEXSUCC);
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
