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

#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/vocabulary.h"
#include "glk/tads/os_frob_tads.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/*
 *  Globals for the script reader
 */
osfildef *scrfp = (osfildef *)0;                             /* script file */
int scrquiet = 0;             /* flag: true ==> script is NOT shown as read */

int qasopn(char *scrnam, int quiet) {
    if (scrfp) return 1;                     /* already reading from script */
    if ((scrfp = osfoprt(scrnam, OSFTCMD)) == 0) return 1;
    scrquiet = quiet;
    return 0;
}

void qasclose() {
    /* only close the script file if there's one open */
    if (scrfp)
    {
        osfcls(scrfp);
        scrfp = 0;                                   /* no more script file */
        scrquiet = 0;
    }
}

char *qasgets(char *buf, int bufl) {
    /* shouldn't be here at all if there's no script file */
    if (scrfp == 0)
        return 0;

    /* update status line */
    runstat();

    /* keep going until we find something we like */
    for (;;)
    {
        char c;
        
        /*
         *   Read the next character of input.  If it's not a newline,
         *   there's more on the same line, so read the rest and see what
         *   to do.  
         */
        c = osfgetc(scrfp);
        if (c != '\n' && c != '\r')
        {
            /* read the rest of the line */
            if (!osfgets(buf, bufl, scrfp))
            {
                /* end of file:  close the script and return eof */
                qasclose();
                return 0;
            }
            
            /* if the line started with '>', strip '\n' and return line */
            if (c == '>')
            {
                int l;

                /* remove the trailing newline */
                if ((l = strlen(buf)) > 0
                    && (buf[l-1] == '\n' || buf[l-1] == '\r'))
                    buf[l-1] = 0;

                /* 
                 *   if we're not in quiet mode, echo the command to the
                 *   display 
                 */
                if (!scrquiet)
                    outformat(buf);

                /* flush the current line without adding any blank lines */
                outflushn(1);

                /* return the command */
                return buf;
            }
        } else if ((int)c == EOF) {
            /* end of file - close the script and return eof */
            qasclose();
            return 0;
        }
    }
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
