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

#include "glk/tads/tads2/play.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/file_io.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

void plygo(runcxdef *run, voccxdef *voc, tiocxdef *tio, objnum preinit, char *restore_fname) {
   int       err;
    errcxdef *ec = run->runcxerr;
    char      filbuf[128];
    int       first_time;
    int noreg inited = FALSE;

    NOREG((&inited));

    first_time = TRUE;

    /* 
     *   Write out the special <?T2> HTML sequence, in case we're on an HTML
     *   system.  This tells the HTML parser to use the parsing rules for
     *   TADS 2 callers. 
     */
    outformat("\\H+<?T2>\\H-");
    
startover:
    if (!inited)
    {
        /* use Me as the format-string actor for preinit and init */
        tiosetactor(voc->voccxtio, voc->voccxme);

        /*
         *   Run preinit, if it hasn't been run yet.  Note that we only
         *   do this the first time through.  If we come back here via the
         *   restart function, preinit will already have been run in the
         *   restart function itself, so we don't need to run it again.
         */
        if (first_time)
        {
            /* make a note that we've been through here once already */
            first_time = FALSE;

            /* remember the preinit function for later use in restarting */
            voc->voccxpreinit = preinit;

            /* run the preinit() function */
            ERRBEGIN(ec)
            {
                /* reset the interpreter */
                runrst(run);

                /* reset the parser */
                voc_stk_ini(voc, (uint)VOC_STACK_SIZE);

                /* run preinit */
                if (preinit != MCMONINV)
                    runfn(run, preinit, 0);
            }
            ERRCATCH(ec, err)
            {
                /* if they restarted, go back and start over */
                if (err == ERR_RUNRESTART)
                    goto startover;

                /* resignal the error */
                errrse(ec);
            }
            ERREND(ec);
        }
        
        /* 
         *   Run the "init" function.  Do NOT run init if we're restoring
         *   a game directly from the command line AND there's an
         *   initRestore function defined. 
         */
        if (restore_fname == 0 || voc->voccxinitrestore == MCMONINV)
        {
            ERRBEGIN(ec)
            {
                /* reset the interpreter */
                runrst(run);

                /* reset the parser */
                voc_stk_ini(voc, (uint)VOC_STACK_SIZE);

                /* run init */
                runfn(run, (objnum)voc->voccxini, 0);
            }
            ERRCATCH(ec, err)
            {
                /* if they restarted, go back and start over */
                if (err == ERR_RUNRESTART)
                    goto startover;
                
                /* resignal the error */
                errrse(ec);
            }
            ERREND(ec);
        }
    }
    
    /* next time through, we'll need to run init again */
    inited = FALSE;

    /* 
     *   check for startup parameter file to restore - if there's a
     *   system-specific parameter file specified, pretend that it was
     *   specified as the restore file 
     */
    if (os_paramfile(filbuf))
        restore_fname = filbuf;

    /* check for a file to restore */
    if (restore_fname != 0)
    {
        /*
         *   Check to see if the game file supports the initRestore
         *   function.  If so, call it to restore the game.  If not,
         *   restore the game directly. 
         */
        if (voc->voccxinitrestore != MCMONINV)
        {
            char restore_buf[OSFNMAX*2];
            char *src;
            char *dst;
            
            /* convert any backslashes to double backslashes */
            for (src = restore_fname, dst = restore_buf ;
                 *src != '\0' && dst + 2 < restore_buf + sizeof(restore_buf) ;
                 ++src)
            {
                switch(*src)
                {
                case '\\':
                    /* it's a backslash - double it */
                    *dst++ = '\\';
                    *dst++ = '\\';
                    break;

                default:
                    /* copy the character as-is */
                    *dst++ = *src;
                }
            }
            
            /* 
             *   all the game's initRestore function with the name of
             *   saved game file to restore as the argument 
             */

            /* reset the interpreter */
            runrst(run);

            /* reset the parser */
            voc_stk_ini(voc, (uint)VOC_STACK_SIZE);

            /* push the game file name and run initRestore */
            runpstr(run, restore_buf, dst - restore_buf, 0);
            runfn(run, (objnum)voc->voccxinitrestore, 1);
        }
        else
        {
            /* restore the game */
            os_printz("\n\n[Restoring saved game]\n\n");
            err = fiorso(voc, restore_fname);
            if (err)
            {
                char buf[60 + OSFNMAX];

                sprintf(buf, "\n\nError: unable to restore file \"%s\"\n\n",
                        restore_fname);
                os_printz(buf);
            }
        }

        /* forget the saved game name, in case we restore */
        restore_fname = 0;
    }

    /* clear out the redo command buffer */
    voc->voccxredobuf[0] = '\0';
    
    /* read and execute commands */
    for (;;)
    {
        char buf[128];
        
        err = 0;
        ERRBEGIN(ec)
            
        /* read a new command if there's nothing to redo */
        if (!voc->voccxredo)
        {
            /* reset hidden output so we're showing output */
            tioshow(tio);
            tioflush(tio);

            /* clear the interpreter stack */
            runrst(run);

            /* read a command */
            vocread(voc, MCMONINV, MCMONINV, buf, (int)sizeof(buf), 0);

            /* special qa checking */
            if (buf[0] == '@')
            {
                int   quiet = FALSE;
                char *p;
                
                p = buf + 1;
                if (*p == '@')
                {
                    /* turn off MORE mode */
                    setmore(0);

                    /* set NONSTOP mode in the OS layer */
                    os_nonstop_mode(TRUE);

                    /* skip the extra '@' */
                    ++p;
                }
                else if (*p == '!')
                {
                    quiet = TRUE;
                    ++p;
                }
                while (*p != '\0' && t_isspace(*p)) ++p;
                if (*p != '\0')
                {
                    /* open the named file */
                    qasopn(p, quiet);
                }
                else
                {
                    char fname[256];

                    /* no file was named - ask the user to select a file */
                    if (tio_askfile("Read script file:", fname, sizeof(fname),
                                    OS_AFP_OPEN, OSFTCMD) == 0)
                        qasopn(fname, quiet);
                }
                goto end_loop;
            }
        }

        /* 
         *   If there's redo in the redo buffer, use it now.  If the
         *   buffer is empty and the redo flag is set, we'll just
         *   re-execute whatever's in our internal buffer.
         */
        if (voc->voccxredo && voc->voccxredobuf[0] != '\0')
        {
            /* copy the redo buffer into our internal buffer */
            strcpy(buf, voc->voccxredobuf);

            /* we've consumed it now, so clear it out */
            voc->voccxredobuf[0] = '\0';
        }

        /* we've now consumed the redo */
        voc->voccxredo = FALSE;

        /* clear any pending break that's queued up */
        (void)os_break();

        /* execute the command */
        (void)voccmd(voc, buf, (uint)sizeof(buf));
        
    end_loop:
        ERRCATCH(ec, err)
        {
            if (err != ERR_RUNQUIT
                && err != ERR_RUNRESTART
                && !(err == ERR_RUNABRT && voc->voccxredo))
                errclog(ec);
        }
        ERREND(ec);

        /* on interrupt, undo last command (which was partially executed) */
        if (err == ERR_USRINT && voc->voccxundo)
        {
            ERRBEGIN(ec)
                objundo(voc->voccxmem, voc->voccxundo);
            ERRCATCH(ec, err)
                if (err != ERR_NOUNDO && err != ERR_ICUNDO)
                    errrse(ec);
            ERREND(ec)
        }
            
        /* if they want to quit, we're done */
        if (err == ERR_RUNQUIT)
            break;
        else if (err == ERR_RUNRESTART)
            goto startover;
    }

    /*
     *   If we're quitting, give the debugger one last chance at taking
     *   control.  If it just returns, we can go ahead and terminate, but
     *   if it wants it can restart the game by calling bifrst() as
     *   normal.  
     */
    ERRBEGIN(ec)
    {
        /* clear anything in the debugger stack trace */
        run->runcxdbg->dbgcxfcn = 0;
        run->runcxdbg->dbgcxdep = 0;

        /* tell the debugger the game has exited */
        dbguquitting(run->runcxdbg);
    }
    ERRCATCH(ec, err)
    {
        switch(err)
        {
        case ERR_RUNRESTART:
            /* they restarted the game - re-enter the play loop */
            goto startover;

        case ERR_RUNQUIT:
            /* quitting - proceed to return as normal */
            break;

        default:
            /* resignal any other error */
            errrse(ec);
        }
    }
    ERREND(ec);
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
