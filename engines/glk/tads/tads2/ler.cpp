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

#include "ler.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

#define TRDLOGERR_PREFIX "\n[An error has occurred within TADS: "

int errcxdef::errfmt(char *outbuf, int outbufl, char *fmt, int argc, erradef *argv) {
    int    outlen = 0;
    int    argi   = 0;
    int    len;
    char   buf[20];
    const char  *p = nullptr;
    char   fmtchar;

    while (*fmt != '\0' && outbufl > 1) {
        switch(*fmt) {
        case '\\':
            ++fmt;
            len = 1;
            switch(*fmt) {
            case '\0':
                --fmt;
                break;
            case '\n':
                p = "\n";
                break;
            case '\t':
                p = "\t";
                break;
            default:
                p = fmt;
                break;
            }
            break;
            
        case '%':
            ++fmt;
            fmtchar = *fmt;
            if (argi >= argc) fmtchar = 1;          // too many - ignore it
            switch(fmtchar) {
            case '\0':
                --fmt;
                len = 0;
                break;
                
            case '%':
                p = "%";
                len = 1;
                break;
                
            case 'd':
                sprintf(buf, "%d", argv[argi].erraint);
                len = strlen(buf);
                p = buf;
                break;
                
            case 'u':
                sprintf(buf, "%u", argv[argi].erraint);
                len = strlen(buf);
                p = buf;
                break;
                
            case 's':
                p = argv[argi].errastr;
                len = strlen(p);
                break;
                
            default:
                p = "";
                len = 0;
                --argi;
                break;
            }
            ++argi;
            break;
            
        default:
            p = fmt;
            len = 1;
            break;
        }

        /* copy output that was set up above */
        if (len != 0) {
            if (outbufl >= len) {
                memcpy(outbuf, p, (size_t)len);
                outbufl -= len;
                outbuf += len;
            } else if (outbufl > 1) {
                memcpy(outbuf, p, (size_t)outbufl - 1);
                outbufl = 1;
            }
            outlen += len;
        }
        ++fmt;
    }

    // add a null terminator
    if (outbufl != 0)
        *outbuf++ = '\0';

    // return the length
    return outlen;
}

void errcxdef::errcxlog(void *ctx0, char *fac, int err, int argc, erradef *argv) {
#ifdef TODO
	errcxdef *ctx = (errcxdef *)ctx0;
	char      buf[256];
	char      msg[256];

	// display the prefix message to the console and log file
	sprintf(buf, TRDLOGERR_PREFIX, fac, err);
	trdptf("%s", buf);
	out_logfile_print(buf, false);

	/* display the error message text to the console and log file */
	errmsg(ctx, msg, (uint)sizeof(msg), err);
	errfmt(buf, (int)sizeof(buf), msg, argc, argv);
	trdptf("%s]\n", buf);
	out_logfile_print(buf, false);
	out_logfile_print("]", true);
#endif
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
