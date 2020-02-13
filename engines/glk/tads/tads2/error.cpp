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

#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/error_handling.h"
#include "glk/tads/tads2/tokenizer.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* format an error message, sprintf-style, using an erradef array */
int errfmt(char *outbuf, int outbufl, const char *fmt, int argc, const erradef *argv) {
    int    outlen = 0;
    int    argi   = 0;
    int    len = 0;
    char   buf[20];
	const char  *p = nullptr;
    char   fmtchar;

    while (*fmt)
    {
        switch(*fmt)
        {
        case '\\':
            ++fmt;
            len = 1;
            switch(*fmt)
            {
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
            if (argi >= argc) fmtchar = 1;          /* too many - ignore it */
            switch(fmtchar)
            {
            case '\0':
                --fmt;
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
                
            case 's':
                p = argv[argi].errastr;
                len = strlen(p);
                break;
                
            case 't':
                {
                    int i;
                    static struct
                    {
                        int   tokid;
                        const char *toknam;
                    } toklist[] =
                    {
                        { TOKTSEM, "semicolon" },
                        { TOKTCOLON, "colon" },
                        { TOKTFUNCTION, "\"function\"" },
                        { TOKTCOMMA, "comma" },
                        { TOKTLBRACE, "left brace ('{')" },
                        { TOKTRPAR, "right paren (')')" },
                        { TOKTRBRACK, "right square bracket (']')" },
                        { TOKTWHILE, "\"while\"" },
                        { TOKTLPAR, "left paren ('(')" },
                        { TOKTEQ, "'='" },
                        { 0, (const char *)nullptr }
                    };
                    
                    for (i = 0 ; toklist[i].toknam ; ++i)
                    {
                        if (toklist[i].tokid == argv[argi].erraint)
                        {
                            p = toklist[i].toknam;
                            break;
                        }
                    }
                    if (!toklist[i].toknam)
                        p = "<unknown token>";
                    len = strlen(p);
                    break;
                }
                
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
        if (len)
        {
            if (outbufl >= len)
            {
                memcpy(outbuf, p, (size_t)len);
                outbufl -= len;
                outbuf += len;
            }
            else if (outbufl)
            {
                memcpy(outbuf, p, (size_t)outbufl);
                outbufl = 0;
            }
            outlen += len;
        }
        ++fmt;
    }
    
    if (outbufl) *outbuf++ = '\0';
    return(outlen);
}

#ifdef DEBUG
#error longjump unsupported in ScummVM
#endif /* DEBUG */



#ifdef ERR_NO_MACRO

/* base error signal function */
void errsign(errcxdef *ctx, int e) {
    ctx->errcxofs = 0;
#if 0
	longjmp(ctx->errcxptr->errbuf, e);
#else
	error("errsign");
#endif
}

/* log an error: base function */
void errlogn(errcxdef *ctx, int err) {
	error("errlogn");
}

#endif /* ERR_NO_MACRO */

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
