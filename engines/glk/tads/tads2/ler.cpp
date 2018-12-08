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

int errfmt(char *outbuf, int outbufl, char *fmt, int argc, erradef *argv) {
    int    outlen = 0;
    int    argi   = 0;
    int    len;
    char   buf[20];
    char  *p = nullptr;
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

#if defined(DEBUG) && !defined(ERR_NO_MACRO)
void errjmp(jmp_buf buf, int e) {
    longjmp(buf, e);
}
#endif /* DEBUG */

#ifdef ERR_NO_MACRO

void errsign(errcxdef *ctx, int e, char *facility) {
    strncpy(ctx->errcxptr->errfac, facility, ERRFACMAX);
    ctx->errcxptr->errfac[ERRFACMAX] = '\0';
    ctx->errcxofs = 0;
    longjmp(ctx->errcxptr->errbuf, e);
}

void errsigf(errcxdef *ctx, char *facility, int e) {
    errargc(ctx, 0);
    errsign(ctx, e, facility);
}

char *errstr(errcxdef *ctx, const char *str, int len) {
    char *ret = &ctx->errcxbuf[ctx->errcxofs];
    
    memcpy(ret, str, (size_t)len);
    ret[len] = '\0';
    ctx->errcxofs += len + 1;
    return(ret);
}

void errrse1(errcxdef *ctx, errdef *fr) {
    errargc(ctx, fr->erraac);
    memcpy(ctx->errcxptr->erraav, fr->erraav,
           (size_t)(fr->erraac * sizeof(erradef)));
    errsign(ctx, fr->errcode, fr->errfac);
}

void errlogn(errcxdef *ctx, int err, char *facility) {
    ctx->errcxofs = 0;
    (*ctx->errcxlog)(ctx->errcxlgc, facility, err, ctx->errcxptr->erraac,
                     ctx->errcxptr->erraav);
}

void errlogf(errcxdef *ctx, char *facility, int err) {
    errargc(ctx, 0);
    errlogn(ctx, err, facility);
}

#endif /* ERR_NO_MACRO */

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
