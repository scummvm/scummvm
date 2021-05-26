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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/video.h"

namespace Saga2 {

/*****************************************************************************/
/* Constants used for paths                                                  */

const char DEFAULT_VID_PATH[] = ".\\";
const char DEFAULT_DGV_PATH[] = "";
const char DEFAULT_SMK_PATH[] = "";


/*****************************************************************************/
/* pre blit / post blit                                                      */

void mouseHook(const bool) {
}

#ifndef _WIN32
/*****************************************************************************/
/* path defaults                                                             */

void pathHook(char *vPath, char *dPath, char *sPath) {
	strcpy(vPath, DEFAULT_VID_PATH); // base path for videos
	strcpy(dPath, DEFAULT_DGV_PATH); // base path for videos
	strcpy(sPath, DEFAULT_SMK_PATH); // base path for videos
}

/*****************************************************************************/
/* path building                                                             */

void nameHook(char *targ, const char *bpath, const char *tpath, const char *fname) {
	if (NULL == strchr(fname, '\\') && NULL == strchr(fname, ':')) {
		strcpy(targ, bpath);
		strcat(targ, tpath);
	} else
		targ[0] = '\0';
	strcat(targ, fname);
}

/*****************************************************************************/
/* AIL hooks                                                                 */

HDIGDRIVER &audioInitHook(void) {
	HDIGDRIVER *d = new HDIGDRIVER;
#if 0
#ifndef _WIN32
	AIL_startup();
	AIL_install_DIG_INI(d);
	if (*d == NULL)
		AIL_shutdown();
#endif

#endif
	return *d;
}

void audioTermHook(HDIGDRIVER d) {
#if 0
#ifndef _WIN32
	if (d)
		AIL_uninstall_DIG_driver(d);
	AIL_shutdown();
#endif
#endif
}


/*****************************************************************************/
/* Blitting                                                                  */

void bltBufferHook(
    const void  *buffer,
    const uint16  bufferWidth,
    const uint16  bufferHeight,
    const uint16  left,
    const uint16  top,
    const uint16  width,
    const uint16  height
) {
}

/*****************************************************************************/
/* Palette Setting                                                           */

#ifndef _WIN32
void setPaletteHook(
    void *paletteMinusFour
) {
#if 0
	cSVGASetPalette(paletteMinusFour);
#endif
}
#endif

/*****************************************************************************/
/* Error Hook                                                                */

void videoErrorHook(
    const char *errMsg
) {
	fprintf(stderr, errMsg);
}
#endif

} // end of namespace Saga2
