/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/video.h"
#include "gob/util.h"
#include "gob/global.h"
#include "gob/palanim.h"

namespace Gob {

int16 pal_fadeValue = 1;

char pal_toFadeRed[256];
char pal_toFadeGreen[256];
char pal_toFadeBlue[256];

char pal_fadeColor(char from, char to) {
	if ((int16)from - pal_fadeValue > (int16)to)
		return from - pal_fadeValue;
	else if ((int16)from + pal_fadeValue < (int16)to)
		return from + pal_fadeValue;
	else
		return to;
}

char pal_fadeStep(int16 oper) {
	char newRed;
	char newGreen;
	char newBlue;
	char stop;
	int16 i;

	if (colorCount != 256)
		error("pal_fadeStep: Only 256 color mode is supported!");

	if (oper == 0) {
		stop = 1;
		if (setAllPalette) {
			if (inVM != 0)
				error("pal_fade: inVM != 0 not supported.");

			for (i = 0; i < 256; i++) {
				newRed =
				    pal_fadeColor(redPalette[i],
				    pal_toFadeRed[i]);
				newGreen =
				    pal_fadeColor(greenPalette[i],
				    pal_toFadeGreen[i]);
				newBlue =
				    pal_fadeColor(bluePalette[i],
				    pal_toFadeBlue[i]);

				if (redPalette[i] != newRed
				    || greenPalette[i] != newGreen
				    || bluePalette[i] != newBlue) {
					
					vid_setPalElem(i, newRed, newGreen, newBlue, 0, 0x13);

					redPalette[i] = newRed;
					greenPalette[i] = newGreen;
					bluePalette[i] = newBlue;
					stop = 0;
				}
			}
		} else {
			for (i = 0; i < 16; i++) {

				vid_setPalElem(i,
				    pal_fadeColor(redPalette[i],
					pal_toFadeRed[i]),
				    pal_fadeColor(greenPalette[i],
					pal_toFadeGreen[i]),
				    pal_fadeColor(bluePalette[i],
					pal_toFadeBlue[i]), -1, videoMode);

				if (redPalette[i] != pal_toFadeRed[i] ||
				    greenPalette[i] != pal_toFadeGreen[i] ||
				    bluePalette[i] != pal_toFadeBlue[i])
					stop = 0;
			}
		}
		return stop;
	} else if (oper == 1) {
		stop = 1;
		for (i = 0; i < 16; i++) {
			vid_setPalElem(i,
			    pal_fadeColor(redPalette[i], pal_toFadeRed[i]),
			    greenPalette[i], bluePalette[i], -1, videoMode);

			if (redPalette[i] != pal_toFadeRed[i])
				stop = 0;
		}
		return stop;
	} else if (oper == 2) {
		stop = 1;
		for (i = 0; i < 16; i++) {
			vid_setPalElem(i,
			    redPalette[i],
			    pal_fadeColor(greenPalette[i], pal_toFadeGreen[i]),
			    bluePalette[i], -1, videoMode);

			if (greenPalette[i] != pal_toFadeGreen[i])
				stop = 0;
		}
		return stop;
	} else if (oper == 3) {
		stop = 1;
		for (i = 0; i < 16; i++) {
			vid_setPalElem(i,
			    redPalette[i],
			    greenPalette[i],
			    pal_fadeColor(bluePalette[i], pal_toFadeBlue[i]),
			    -1, videoMode);

			if (bluePalette[i] != pal_toFadeBlue[i])
				stop = 0;
		}
		return stop;
	}
	return 1;
}

void pal_fade(PalDesc *palDesc, int16 fade, int16 allColors) {
	char stop;
	int16 i;

	if (fade < 0)
		pal_fadeValue = -fade;
	else
		pal_fadeValue = 2;

	if (colorCount < 256) {
		if (palDesc != 0)
			vid_setFullPalette(palDesc);
		return;
	}

	if (setAllPalette == 0) {
		if (palDesc == 0) {
			for (i = 0; i < 16; i++) {
				pal_toFadeRed[i] = 0;
				pal_toFadeGreen[i] = 0;
				pal_toFadeBlue[i] = 0;
			}
		} else {
			for (i = 0; i < 16; i++) {
				pal_toFadeRed[i] = palDesc->vgaPal[i].red;
				pal_toFadeGreen[i] = palDesc->vgaPal[i].green;
				pal_toFadeBlue[i] = palDesc->vgaPal[i].blue;
			}
		}
	} else {
		if (inVM != 0)
			error("pal_fade: inVM != 0 is not supported");

		if (palDesc == 0) {
			for (i = 0; i < 256; i++) {
				pal_toFadeRed[i] = 0;
				pal_toFadeGreen[i] = 0;
				pal_toFadeBlue[i] = 0;
			}
		} else {
			for (i = 0; i < 256; i++) {
				pal_toFadeRed[i] = palDesc->vgaPal[i].red;
				pal_toFadeGreen[i] = palDesc->vgaPal[i].green;
				pal_toFadeBlue[i] = palDesc->vgaPal[i].blue;
			}
		}
	}

	if (allColors == 0) {

		do {
			if (tmpPalBuffer == 0)
				vid_waitRetrace(videoMode);

			stop = pal_fadeStep(0);

			if (fade > 0)
				util_delay(fade);
		} while (stop == 0);

		if (palDesc != 0)
			vid_setFullPalette(palDesc);
		else
			util_clearPalette();
	}

	if (allColors == 1) {

		do {
			vid_waitRetrace(videoMode);
			stop = pal_fadeStep(1);
		} while (stop == 0);

		do {
			vid_waitRetrace(videoMode);
			stop = pal_fadeStep(2);
		} while (stop == 0);

		do {
			vid_waitRetrace(videoMode);
			stop = pal_fadeStep(3);
		} while (stop == 0);

		if (palDesc != 0)
			vid_setFullPalette(palDesc);
		else
			util_clearPalette();
	}

	free(tmpPalBuffer);
	tmpPalBuffer = 0;
}

}				// End of namespace Gob
