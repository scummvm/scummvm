/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include <stdio.h>

#include "common/util.h"
#include "base/engine.h"
#include "d_draw.h"
#include "driver96.h"
#include "../sword2.h"

#define PALTABLESIZE 64 * 64 * 64

uint8 palCopy[256][4];
uint8 fadePalette[256][4];

uint8 paletteMatch[PALTABLESIZE];

uint8 fadeStatus = RDFADE_NONE;

static int32 fadeStartTime;
static int32 fadeTotalTime;

// --------------------------------------------------------------------------
// int32 RestorePalette(void)
//
// This function restores the palette, and should be called whenever the
// screen mode changes, or something like that.
// --------------------------------------------------------------------------

int32 RestorePalette(void) {
	g_system->set_palette((const byte *) palCopy, 0, 256);
	return RD_OK;
}

uint8 GetMatch(uint8 r, uint8 g, uint8 b) {
	int32 diff;
	int32 min;
	int16 diffred, diffgreen, diffblue;
	int16 i;
	uint8 minIndex;

	diffred = palCopy[0][0] - r;
	diffgreen = palCopy[0][1] - g;
	diffblue = palCopy[0][2] - b;

	diff = diffred * diffred + diffgreen * diffgreen + diffblue * diffblue;
	min = diff;
	minIndex = 0;
	if (diff > 0) {
		i = 1;
		while (i < 256)	{
			diffred = palCopy[i][0] - r;
			diffgreen = palCopy[i][1] - g;
			diffblue = palCopy[i][2] - b;

			diff = diffred * diffred + diffgreen * diffgreen + diffblue * diffblue;
			if (diff < min) {
				min = diff;
				minIndex = (uint8) i;
				if (min == 0)
					break;
			}
			i++;
		}
	}

	// Here, minIndex is the index of the matchpalette which is closest.
	return minIndex;
}

/**
 * Sets or creates a table of palette indices which will be searched later for
 * a quick palette match.
 * @param data either the palette match table, or NULL to create a new table
 * from the current palCopy
 */

int32 UpdatePaletteMatchTable(uint8 *data) {
	if (!data) {
		int16 red, green, blue;
		uint8 *p;

		// Create palette match table

		// FIXME: Does this case ever happen?

		p = &paletteMatch[0];
		for (red = 0; red < 256; red += 4) {
			for (green = 0; green < 256; green += 4) {
				for (blue = 0; blue < 256; blue += 4) {
					*p++ = GetMatch((uint8) red, (uint8) green, (uint8) blue);
				}
			}
		}
	} else {
		// The provided data is th new palette match table
		memcpy(paletteMatch, data, PALTABLESIZE);
	}

	return RD_OK;
}

/**
 * Matches a colour triplet to a palette index.
 * @param r red colour component
 * @param g green colour component
 * @param b blue colour component
 * @return the palette index of the closest matching colour in the palette
 */

// FIXME: This used to be inlined - probably a good idea - but the
// linker complained when I tried to use it in sprite.cpp.

uint8 QuickMatch(uint8 r, uint8 g, uint8 b) {
	return paletteMatch[((int32) (r >> 2) << 12) + ((int32) (g >> 2) << 6) + (b >> 2)];
}

/**
 * Sets the palette.
 * @param startEntry the first colour entry to set
 * @param noEntries the number of colour entries to set
 * @param colourTable the new colour entries
 */

int32 BS2_SetPalette(int16 startEntry, int16 noEntries, uint8 *colourTable, uint8 fadeNow) {
	if (noEntries == 0) {
		RestorePalette();
		return RD_OK;
	}

	memcpy(&palCopy[startEntry][0], colourTable, noEntries * 4);
	if (fadeNow == RDPAL_INSTANT)
		g_system->set_palette((byte *) palCopy, startEntry, noEntries);

	return RD_OK;
}

int32 DimPalette(void) {
	byte *p = (byte *) palCopy;
	uint32 i;

	for (i = 0; i < 256 * 4; i++)
		p[i] /= 2;
	g_system->set_palette(p, 0, 256);
	return RD_OK;
}

/**
 * Fades the palette up from black to the current palette.
 * @param time the time it will take the palette to fade up
 */

int32 FadeUp(float time) {
	if (fadeStatus != RDFADE_BLACK && fadeStatus != RDFADE_NONE)
		return RDERR_FADEINCOMPLETE;

	fadeTotalTime = (int32) (time * 1000);
	fadeStatus = RDFADE_UP;
	fadeStartTime = SVM_timeGetTime();

	return RD_OK;
}

/**
 * Fades the palette down to black from the current palette.
 * @param time the time it will take the palette to fade down
 */

int32 FadeDown(float time) {
	if (fadeStatus != RDFADE_BLACK && fadeStatus != RDFADE_NONE)
		return RDERR_FADEINCOMPLETE;

	fadeTotalTime = (int32) (time * 1000);
	fadeStatus = RDFADE_DOWN;
	fadeStartTime = SVM_timeGetTime();

	return RD_OK;
}

/**
 * Get the current fade status
 * @return RDFADE_UP (fading up), RDFADE_DOWN (fading down), RDFADE_NONE
 * (not faded), or RDFADE_BLACK (completely faded down)
 */

uint8 GetFadeStatus(void) {
	return fadeStatus;
}

void WaitForFade(void) {
	while (GetFadeStatus() != RDFADE_NONE && GetFadeStatus() != RDFADE_BLACK) {
		ServiceWindows();
		g_system->delay_msecs(20);
	}
}

void FadeServer() {
	static int32 previousTime = 0;
	const byte *newPalette = (const byte *) fadePalette;
	int32 currentTime;
	int16 fadeMultiplier;
	int16 i;

	// This used to be called through a timer, but is now called from
	// ServiceWindows() instead, since that's the only place where we
	// actually update the screen.

	// If we're not in the process of fading, do nothing.
	if (fadeStatus != RDFADE_UP && fadeStatus != RDFADE_DOWN)
		return;

	// I don't know if this is necessary, but let's limit how often the
	// palette is updated, just to be safe.
	currentTime = SVM_timeGetTime();
	if (currentTime - previousTime <= 25)
		return;

	previousTime = currentTime;

	if (fadeStatus == RDFADE_UP) {
		if (currentTime >= fadeStartTime + fadeTotalTime) {
			fadeStatus = RDFADE_NONE;
			newPalette = (const byte *) palCopy;
		} else {
			fadeMultiplier = (int16) (((int32) (currentTime - fadeStartTime) * 256) / fadeTotalTime);
			for (i = 0; i < 256; i++) {
				fadePalette[i][0] = (palCopy[i][0] * fadeMultiplier) >> 8;
				fadePalette[i][1] = (palCopy[i][1] * fadeMultiplier) >> 8;
				fadePalette[i][2] = (palCopy[i][2] * fadeMultiplier) >> 8;
			}
		}
	} else {
		if (currentTime >= fadeStartTime + fadeTotalTime) {
			fadeStatus = RDFADE_BLACK;
			memset(fadePalette, 0, sizeof(fadePalette));
		} else {
			fadeMultiplier = (int16) (((int32) (fadeTotalTime - (currentTime - fadeStartTime)) * 256) / fadeTotalTime);
			for (i = 0; i < 256; i++) {
				fadePalette[i][0] = (palCopy[i][0] * fadeMultiplier) >> 8;
				fadePalette[i][1] = (palCopy[i][1] * fadeMultiplier) >> 8;
				fadePalette[i][2] = (palCopy[i][2] * fadeMultiplier) >> 8;
			}
		}
	}

	g_system->set_palette(newPalette, 0, 256);
}
