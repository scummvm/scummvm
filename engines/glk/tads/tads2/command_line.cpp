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

#include "glk/tads/tads2/command_line.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* get a toggle argument */
int cmdtog(errcxdef *ec, int prv, char *argp, int ofs,
           void (*usagefn)(errcxdef *))
{
    switch(argp[ofs + 1])
    {
    case '+':
        return(TRUE);
        
    case '-':
        return(FALSE);
        
    case '\0':
        return(!prv);
        
    default:
        /* invalid - display usage if we have a callback for it */
        if (usagefn != 0)
            (*usagefn)(ec);
        NOTREACHEDV(int);
        return 0;
    }
}

/* get an argument to a switch */
char *cmdarg(errcxdef *ec, char ***argpp, int *ip, int argc, int ofs,
             void (*usagefn)(errcxdef *))
{
    char *ret;

    /* 
     *   check to see if the argument is appended directly to the option;
     *   if not, look at the next string 
     */
    ret = (**argpp) + ofs + 1;
    if (*ret == '\0')
    {
        /* 
         *   it's not part of this string - get the argument from the next
         *   string in the vector 
         */
        ++(*ip);
        ++(*argpp);
        ret = (*ip >= argc ? 0 : **argpp);
    }

    /* 
     *   if we didn't find the argument, it's an error - display usage if
     *   we have a valid usage callback
     */
    if ((ret == 0 || *ret == 0) && usagefn != 0)
        (*usagefn)(ec);

    return ret;
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
