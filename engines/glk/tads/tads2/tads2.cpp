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

#include "glk/tads/tads2/tads2.h"
#include "glk/tads/tads2/built_in.h"
#include "glk/tads/tads2/debug.h"
#include "glk/tads/tads2/file_io.h"
#include "glk/tads/tads2/post_compilation.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/vocabulary.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

TADS2::TADS2(OSystem *syst, const GlkGameDescription &gameDesc) : TADS(syst, gameDesc) {
}

void TADS2::runGame() {
	errcxdef errctx;
	errctx.errcxlgc = &errctx;
	errctx.errcxfp = nullptr;
	errctx.errcxofs = 0;
	errctx.errcxappctx = nullptr;

	/* copyright-date-string */
#ifdef T2_COPYRIGHT_NOTICE
	trdptf("%s - A %s TADS %s Interpreter.\n",
		G_tads_oem_app_name, G_tads_oem_display_mode,
		TADS_RUNTIME_VERSION);
	trdptf("%sopyright (c) 1993, 2012 by Michael J. Roberts.\n",
		G_tads_oem_copyright_prefix ? "TADS c" : "C");
	trdptf("%s\n", G_tads_oem_author);
#endif

	trdmain1(&errctx);
}

void TADS2::trdmain1(errcxdef *errctx) {
	osfildef  *swapfp = (osfildef *)0;
	runcxdef   runctx;
	bifcxdef   bifctx;
	voccxdef   vocctx;
	void(*bif[100])(bifcxdef *, int);
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
	char      *exefile;            /* try with executable file if no infile */
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
#ifdef TODO
	NOREG((&loadopen))

		/* initialize the output formatter */
		out_init();

	/* set safety level to 2 by default - read any/write current dir only */
	safety_read = safety_write = 2;

	/* no -ctab- yet */
	charmap_none = FALSE;

	/* parse arguments */
	for (i = 1, argp = argv + 1; i < argc; ++argp, ++i)
	{
		arg = *argp;
		if (*arg == '-')
		{
			switch (*(arg + 1))
			{
			case 'c':
				if (!strcmp(arg + 1, "ctab"))
				{
					/* get the character mapping table */
					charmap = cmdarg(ec, &argp, &i, argc, 4, trdusage);
				}
				else if (!strcmp(arg + 1, "ctab-"))
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
				if (!stricmp(arg, "-plain"))
				{
					os_plain();
					break;
				}
				pause = cmdtog(ec, pause, arg, 1, trdusage);
				break;

			case 'd':
				if (!strnicmp(arg, "-double", 7))
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
				safety_write = (*(p + 1) != '\0' ? *(p + 1) - '0' :
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
				switch (*(arg + 2))
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
				switch (*(arg + 2))
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
		errsig1(ec, ERR_STKSIZE, ERRTINT, (uint)(65535 / sizeof(runsdef)));
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
	CLRSTRUCT(bifctx);
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
	supbif(&supctx, bif, (int)(sizeof(bif) / sizeof(bif[0])));

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
#endif
}

void TADS2::trdptf(const char *fmt, ...) {
	va_list va;

	// format the string */
	va_start(va, fmt);
	Common::String msg = Common::String::vformat(fmt, va);
	va_end(va);

	// print the formatted buffer
	os_printz(msg);
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
