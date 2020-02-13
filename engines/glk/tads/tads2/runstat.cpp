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
#include "glk/tads/tads2/os.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/tokenizer.h"
#include "glk/tads/tads2/vocabulary.h"


namespace Glk {
namespace TADS {
namespace TADS2 {

static runcxdef *runctx;
static voccxdef *vocctx;
static tiocxdef *tioctx;

void runstat(void)
{
    objnum  locobj;
    int     savemoremode;

    /* get the location of the Me object */
    runppr(runctx, vocctx->voccxme, PRP_LOCATION, 0);

    /* if that's no an object, there's nothing we can do */
    if (runtostyp(runctx) != DAT_OBJECT)
    {
        rundisc(runctx);
        return;
    }

    /* get Me.location */
    locobj = runpopobj(runctx);

    /* flush any pending output */
    outflushn(0);

    /* switch to output display mode 1 (status line) */
    os_status(1);

    /* turn off MORE mode */
    savemoremode = setmore(0);

    /* call the statusLine method of the current room */
    runppr(runctx, locobj, PRP_STATUSLINE, 0);

    /* if we're in the status line, make sure the line gets flushed */
    if (os_get_status() != 0)
        tioputs(tioctx, "\\n");
    outflushn(0);

    /* restore the previous MORE mode */
    setmore(savemoremode);

    /* switch to output display mode 0 (main text area) */
    os_status(0);
}

void runistat(voccxdef *vctx, runcxdef *rctx, tiocxdef *tctx)
{
    runctx = rctx;
    vocctx = vctx;
    tioctx = tctx;
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
