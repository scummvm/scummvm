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

#include "glk/tads/tads2/character_map.h"
#include "glk/tads/tads2/text_io.h"
#include "glk/tads/tads2/os.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/*
 *   Global variable with the current command logging file.  If this is
 *   not null, we'll log each command that we read to this file.  
 */
osfildef *cmdfile;

/*
 *   External global with the current script input file.  If this is
 *   non-null, we'll read commands from this file rather than from the
 *   keyboard.  
 */
extern osfildef *scrfp;

/*
 *   External global indicating script echo status.  If we're reading from
 *   a script input file (i.e., scrfp is non-null), and this variable is
 *   true, it indicates that we're in "quiet" mode reading the script, so
 *   we will not echo commands that we read from the script file to the
 *   display. 
 */
extern int scrquiet;

/*
 *   getstring reads a string from the keyboard, doing all necessary
 *   output flushing.  Prompting is to be done by the caller.  This
 *   routine should be called instead of os_gets.
 */
int getstring(const char *prompt, char *buf, int bufl)
{
    char  *result;
    int    savemoremode;
	int    retval = 0;

    /* show prompt if one was given and flush output */
    savemoremode = setmore(0);
    if (prompt != 0)
    {
        /* display the prompt text */
        outformat(prompt);

        /* make sure it shows up in the log file as well */
        out_logfile_print(prompt, FALSE);
    }
    outflushn(0);
    outreset();

    /* read from the command input file if we have one */
    if (scrfp != 0)
    {
        int quiet = scrquiet;
        
        /* try reading from command input file */
        if ((result = qasgets(buf, bufl)) == 0)
        {
            /*
             *   End of command input file; return to reading the
             *   keyboard.  If we didn't already show the prompt, show it
             *   now.
             *   
             *   Note that qasgets() will have closed the script file
             *   before returning eof, so we won't directly read the
             *   command here but instead handle it later when we check to
             *   see if we need to read from the keyboard.  
             */
            if (quiet && prompt != 0)
                outformat(prompt);
            outflushn(0);
            outreset();

            /*
             *   Guarantee that moremode is turned back on.  (moremode can
             *   be turned off for one of two reasons: we're printing the
             *   prompt, or we're reading from a script with no pauses.
             *   In either case, moremode should be turned back on at this
             *   point. -CDN) 
             */
            savemoremode = 1;

            /* turn off NONSTOP mode now that we're done with the script */
            os_nonstop_mode(FALSE);
        }

        /* success */
        retval = 0;
    }

    /* if we don't have a script file, read from the keyboard */
    if (scrfp == 0)
    {
        /* update the status line */
        runstat();
        
        /* read a line from the keyboard */
        result = (char *)os_gets((uchar *)buf, bufl);
        
        /* 
         *   if the result is null, we're at eof, so return a non-zero
         *   value; otherwise, we successfully read a command, so return
         *   zero 
         */
        retval = (result == 0);
    }

    /* restore the original "more" mode */
    setmore(savemoremode);

    /* check the result */
    if (retval != 0)
    {
        /* we got an error reading the command - return the error */
        return retval;
    }
    else
    {
        char *p;
        
        /* 
         *   we got a command, or at least a partial command (if we timed
         *   out, we may still have a partial line in the buffer) - write
         *   the input line to the log and/or command files, as
         *   appropriate 
         */
        out_logfile_print(buf, TRUE);
        if (cmdfile != 0)
        {
            os_fprintz(cmdfile, ">");
            os_fprintz(cmdfile, buf);
            os_fprintz(cmdfile, "\n");
        }

        /* translate the input to the internal character set */
        for (p = buf ; *p != '\0' ; ++p)
            *p = cmap_n2i(*p);

        /* success */
        return retval;
    }
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
