/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/util.h"
#include "scumm/scumm.h"
#include "scumm/intern.h"
#include "scumm/resource.h"


namespace Scumm {

void ScummEngine::setupAmigaPalette() {
	setPalColor( 0,   0,   0,   0);
	setPalColor( 1,   0,   0, 187);
	setPalColor( 2,   0, 187,   0);
	setPalColor( 3,   0, 187, 187);
	setPalColor( 4, 187,   0,   0);
	setPalColor( 5, 187,   0, 187);
	setPalColor( 6, 187, 119,   0);
	setPalColor( 7, 187, 187, 187);
	setPalColor( 8, 119, 119, 119);
	setPalColor( 9, 119, 119, 255);
	setPalColor(10,   0, 255,   0);
	setPalColor(11,   0, 255, 255);
	setPalColor(12, 255, 136, 136);
	setPalColor(13, 255,   0, 255);
	setPalColor(14, 255, 255,   0);
	setPalColor(15, 255, 255, 255);
}

void ScummEngine::setupEGAPalette() {
	setPalColor( 0,   0,   0,   0);
	setPalColor( 1,   0,   0, 168);
	setPalColor( 2,   0, 168,   0);
	setPalColor( 3,   0, 168, 168);
	setPalColor( 4, 168,   0,   0);
	setPalColor( 5, 168,   0, 168);
	setPalColor( 6, 168,  84,   0);
	setPalColor( 7, 168, 168, 168);
	setPalColor( 8,  84,  84,  84);
	setPalColor( 9,  84,  84, 252);
	setPalColor(10,  84, 252,  84);
	setPalColor(11,  84, 252, 252);
	setPalColor(12, 252,  84,  84);
	setPalColor(13, 252,  84, 252);
	setPalColor(14, 252, 252,  84);
	setPalColor(15, 252, 252, 252);
}

void ScummEngine::setupV1ManiacPalette() {
	setPalColor( 0,   0,   0,   0);
	setPalColor( 1, 252, 252, 252);
	setPalColor( 2, 168,   0,   0);
	setPalColor( 3,   0, 168, 168);
	setPalColor( 4, 168,   0, 168);
	setPalColor( 5,   0, 168,   0);
	setPalColor( 6,   0,   0, 168);
	setPalColor( 7, 252, 252,  84);
	setPalColor( 8, 252,  84,  84);
	setPalColor( 9, 168,  84,   0);
	setPalColor(10, 252,  84,  84);
	setPalColor(11,  84,  84,  84);
	setPalColor(12, 168, 168, 168);
	setPalColor(13,  84, 252,  84);
	setPalColor(14,  84,  84, 252);
	setPalColor(15,  84,  84,  84);
	setPalColor(16, 252,  84, 252);
}

void ScummEngine::setupV1ZakPalette() {
	setPalColor( 0,   0,   0,   0);
	setPalColor( 1, 252, 252, 252);
	setPalColor( 2, 168,   0,   0);
	setPalColor( 3,   0, 168, 168);
	setPalColor( 4, 168,   0, 168);
	setPalColor( 5,   0, 168,   0);
	setPalColor( 6,   0,   0, 168);
	setPalColor( 7, 252, 252,  84);
	setPalColor( 8, 252,  84,  84);
	setPalColor( 9, 168,  84,   0);
	setPalColor(10, 252,  84,  84);
	setPalColor(11,  84,  84,  84);
	setPalColor(12, 168, 168, 168);
	setPalColor(13,  84, 252,  84);
	setPalColor(14,  84,  84, 252);
	setPalColor(15, 168, 168, 168);
	setPalColor(16, 252,  84, 252);
}

void ScummEngine::setPaletteFromPtr(const byte *ptr) {
	int i;
	byte *dest, r, g, b;
	int numcolor;

	if (_features & GF_SMALL_HEADER) {
		if (_features & GF_OLD256)
			numcolor = 256;
		else
			numcolor = READ_LE_UINT16(ptr + 6) / 3;
		ptr += 8;
	} else {
		numcolor = getResourceDataSize(ptr) / 3;
	}

	checkRange(256, 0, numcolor, "Too many colors (%d) in Palette");

	dest = _currentPalette;

	for (i = 0; i < numcolor; i++) {
		r = *ptr++;
		g = *ptr++;
		b = *ptr++;

		// This comparison might look weird, but it's what the disassembly (DOTT) says!
		// FIXME: Fingolfin still thinks it looks weird: the value 252 = 4*63 clearly comes from
		// the days 6/6/6 palettes were used, OK. But it breaks MonkeyVGA, so I had to add a
		// check for that. And somebody before me added a check for V7 games, turning this
		// off there, too... I wonder if it hurts other games, too? What exactly is broken
		// if we remove this patch?
		// Since it also causes problems in Zak256, I am turning it off for all V4 games and older.
		if ((_version <= 4) || (_version >= 7) || (i <= 15 || r < 252 || g < 252 || b < 252)) {
			*dest++ = r;
			*dest++ = g;
			*dest++ = b;
		} else {
			dest += 3;
		}
	}
	setDirtyColors(0, numcolor - 1);
}

void ScummEngine::setPaletteFromRes() {
	byte *ptr;
	ptr = getResourceAddress(rtRoom, _roomResource) + _CLUT_offs;
	setPaletteFromPtr(ptr);
}

void ScummEngine::setDirtyColors(int min, int max) {
	if (_palDirtyMin > min)
		_palDirtyMin = min;
	if (_palDirtyMax < max)
		_palDirtyMax = max;
}

void ScummEngine::initCycl(const byte *ptr) {
	int j;
	ColorCycle *cycl;

	memset(_colorCycle, 0, sizeof(_colorCycle));

	if (_features & GF_SMALL_HEADER) {
		cycl = _colorCycle;
		ptr += 6;
		for (j = 0; j < 16; ++j, ++cycl) {
			uint16 delay = READ_BE_UINT16(ptr);
			ptr += 2;
			byte start = *ptr++;
			byte end = *ptr++;

			if (!delay || delay == 0x0aaa || start >= end)
				continue;

			cycl->counter = 0;
			cycl->delay = 16384 / delay;
			cycl->flags = 2;
			cycl->start = start;
			cycl->end = end;
		}
	} else {
		while ((j = *ptr++) != 0) {
			if (j < 1 || j > 16) {
				error("Invalid color cycle index %d", j);
			}
			cycl = &_colorCycle[j - 1];

			ptr += 2;
			cycl->counter = 0;
			cycl->delay = 16384 / READ_BE_UINT16(ptr);
			ptr += 2;
			cycl->flags = READ_BE_UINT16(ptr);
			ptr += 2;
			cycl->start = *ptr++;
			cycl->end = *ptr++;
		}
	}
}

void ScummEngine::stopCycle(int i) {
	ColorCycle *cycl;

	checkRange(16, 0, i, "Stop Cycle %d Out Of Range");
	if (i != 0) {
		_colorCycle[i - 1].delay = 0;
		return;
	}

	for (i = 0, cycl = _colorCycle; i < 16; i++, cycl++)
		cycl->delay = 0;
}

/**
 * Cycle the colors in the given palette in the intervael [cycleStart, cycleEnd]
 * either one step forward or backward.
 */
static void doCyclePalette(byte *palette, int cycleStart, int cycleEnd, int size, bool forward) {
	byte *start = palette + cycleStart * size;
	byte *end = palette + cycleEnd * size;
	int num = cycleEnd - cycleStart;
	byte tmp[6];
	
	assert(size <= 6);
	
	if (forward) {
		memmove(tmp, end, size);
		memmove(start + size, start, num * size);
		memmove(start, tmp, size);
	} else {
		memmove(tmp, start, size);
		memmove(start, start + size, num * size);
		memmove(end, tmp, size);
	}
}

/**
 * Adjust an 'indirect' color palette for the color cycling performed on its
 * master palette. An indirect palette is a palette which contains indices
 * pointing into another palette - it provides a level of indirection to map
 * palette colors to other colors. Now when the target palette is cycled, the
 * indirect palette suddenly point at the wrong color(s). This function takes
 * care of adjusting an indirect palette by searching through it and replacing
 * all indices that are in the cycle range by the new (cycled) index.
 *
 * Finally, the palette entries still have to be cycled normally.
 */
static void doCycleIndirectPalette(byte *palette, int cycleStart, int cycleEnd, bool forward) {
	int num = cycleEnd - cycleStart + 1;
	int i;
	int offset = forward ? 1 : num - 1;
	
	for (i = 0; i < 256; i++) {
		if (cycleStart <= palette[i] && palette[i] <= cycleEnd) {
			palette[i] = (palette[i] - cycleStart + offset) % num + cycleStart;
		}
	}
	
	doCyclePalette(palette, cycleStart, cycleEnd, 1, forward);
}


void ScummEngine::cyclePalette() {
	ColorCycle *cycl;
	int valueToAdd;
	int i, j;

	if (VAR_TIMER == 0xFF) {
		// FIXME - no idea if this is right :-/
		// Needed for both V2 and V8 at this time
		valueToAdd = VAR(VAR_TIMER_NEXT);
	} else {
		valueToAdd = VAR(VAR_TIMER);
		if (valueToAdd < VAR(VAR_TIMER_NEXT))
			valueToAdd = VAR(VAR_TIMER_NEXT);
	}

	if (!_colorCycle)							// FIXME
		return;

	for (i = 0, cycl = _colorCycle; i < 16; i++, cycl++) {
		if (!cycl->delay || cycl->start > cycl->end)
			continue;
		cycl->counter += valueToAdd;
		if (cycl->counter >= cycl->delay) {
			cycl->counter %= cycl->delay;

			setDirtyColors(cycl->start, cycl->end);
			moveMemInPalRes(cycl->start, cycl->end, cycl->flags & 2);

			if (!(_features & GF_SMALL_HEADER && _version > 2))
				doCyclePalette(_currentPalette, cycl->start, cycl->end, 3, !(cycl->flags & 2));

			// Also cycle the other, indirect palettes
			if (_proc_special_palette) {
				doCycleIndirectPalette(_proc_special_palette, cycl->start, cycl->end, !(cycl->flags & 2));
			}

			if (_shadowPalette) {
				if (_version >= 7) {
					for (j = 0; j < NUM_SHADOW_PALETTE; j++)
						doCycleIndirectPalette(_shadowPalette + j * 256, cycl->start, cycl->end, !(cycl->flags & 2));
				} else {
					doCycleIndirectPalette(_shadowPalette, cycl->start, cycl->end, !(cycl->flags & 2));
				}
			}
		}
	}
}

/**
 * Perform color cycling on the palManipulate data, too, otherwise
 * color cycling will be disturbed by the palette fade.
 */
void ScummEngine::moveMemInPalRes(int start, int end, byte direction) {
	if (!_palManipCounter)
		return;

	doCyclePalette(_palManipPalette, start, end, 3, !direction);
	doCyclePalette(_palManipIntermediatePal, start, end, 6, !direction);
}

void ScummEngine::palManipulateInit(int resID, int start, int end, int time) {
	byte *pal, *target, *between;
	byte *string1, *string2, *string3;
	int i;

	string1 = getStringAddress(resID);
	string2 = getStringAddress(resID + 1);
	string3 = getStringAddress(resID + 2);
	if (!string1 || !string2 || !string3) {
		warning("palManipulateInit(%d,%d,%d,%d): Cannot obtain string resources %d, %d and %d",
				resID, start, end, time, resID, resID + 1, resID + 2);
		return;
	}

	string1 += start;
	string2 += start;
	string3 += start;

	_palManipStart = start;
	_palManipEnd = end;
	_palManipCounter = 0;

	if (!_palManipPalette)
		_palManipPalette = (byte *)calloc(0x300, 1);
	if (!_palManipIntermediatePal)
		_palManipIntermediatePal = (byte *)calloc(0x600, 1);

	pal = _currentPalette + start * 3;
	target = _palManipPalette + start * 3;
	between = _palManipIntermediatePal + start * 6;

	for (i = start; i < end; ++i) {
		*target++ = *string1++;
		*target++ = *string2++;
		*target++ = *string3++;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
	}

	_palManipCounter = time;
}

void ScummEngine_v6::palManipulateInit(int resID, int start, int end, int time) {
	byte *pal, *target, *between;
	const byte *new_pal;
	int i;

	new_pal = getPalettePtr(resID);

	new_pal += start*3;

	_palManipStart = start;
	_palManipEnd = end;
	_palManipCounter = 0;

	if (!_palManipPalette)
		_palManipPalette = (byte *)calloc(0x300, 1);
	if (!_palManipIntermediatePal)
		_palManipIntermediatePal = (byte *)calloc(0x600, 1);

	pal = _currentPalette + start * 3;
	target = _palManipPalette + start * 3;
	between = _palManipIntermediatePal + start * 6;

	for (i = start; i < end; ++i) {
		*target++ = *new_pal++;
		*target++ = *new_pal++;
		*target++ = *new_pal++;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
	}

	_palManipCounter = time;
}


void ScummEngine::palManipulate() {
	byte *target, *pal, *between;
	int i, j;

	if (!_palManipCounter || !_palManipPalette || !_palManipIntermediatePal)
		return;
	
	target = _palManipPalette + _palManipStart * 3;
	pal = _currentPalette + _palManipStart * 3;
	between = _palManipIntermediatePal + _palManipStart * 6;

	for (i = _palManipStart; i < _palManipEnd; ++i) {
		j = (*((uint16 *)between) += ((*target++ << 8) - *((uint16 *)between)) / _palManipCounter);
		*pal++ = j >> 8;
		between += 2;
		j = (*((uint16 *)between) += ((*target++ << 8) - *((uint16 *)between)) / _palManipCounter);
		*pal++ = j >> 8;
		between += 2;
		j = (*((uint16 *)between) += ((*target++ << 8) - *((uint16 *)between)) / _palManipCounter);
		*pal++ = j >> 8;
		between += 2;
	}
	setDirtyColors(_palManipStart, _palManipEnd);
	_palManipCounter--;
}

void ScummEngine::setupShadowPalette(int slot, int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	byte *table;
	int i;
	byte *curpal;

	if (slot < 0 || slot >= NUM_SHADOW_PALETTE)
		error("setupShadowPalette: invalid slot %d", slot);

	if (startColor < 0 || startColor > 255 || endColor < 0 || startColor > 255 || endColor < startColor)
		error("setupShadowPalette: invalid range from %d to %d", startColor, endColor);

	table = _shadowPalette + slot * 256;
	for (i = 0; i < 256; i++)
		table[i] = i;

	table += startColor;
	curpal = _currentPalette + startColor * 3;
	for (i = startColor; i <= endColor; i++) {
		*table++ = remapPaletteColor((curpal[0] * redScale) >> 8,
									 (curpal[1] * greenScale) >> 8,
									 (curpal[2] * blueScale) >> 8,
									 (uint) - 1);
		curpal += 3;
	}
}

static inline uint colorWeight(int red, int green, int blue) {
	return 3 * red * red + 6 * green * green + 2 * blue * blue;
}


void ScummEngine::setupShadowPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	const byte *basepal = getPalettePtr(_curPalIndex);
	const byte *pal = basepal;
	const byte *compareptr;
	byte *table = _shadowPalette;
	int i;

	// This is a correction of the patch supplied for BUG #588501.
	// It has been tested in all four known rooms where unkRoomFunc3 is used:
	//
	// 1) FOA Room 53: subway departing Knossos for Atlantis.
	// 2) FOA Room 48: subway crashing into the Atlantis entrance area
	// 3) FOA Room 82: boat/sub shadows while diving near Thera
	// 4) FOA Room 23: the big machine room inside Atlantis
	//
	// The implementation behaves well in all tests.
	// Pixel comparisons show that the resulting palette entries being
	// derived from the shadow palette generated here occassionally differ
	// slightly from the ones derived in the LEC executable.
	// Not sure yet why, but the differences are VERY minor.
	//
	// There seems to be no explanation for why this function is called
	// from within Room 23 (the big machine), as it has no shadow effects
	// and thus doesn't result in any visual differences.

	for (i = 0; i <= 255; i++) {
		int r = (int) (*pal++ * redScale) >> 8;
		int g = (int) (*pal++ * greenScale) >> 8;
		int b = (int) (*pal++ * blueScale) >> 8;

		// The following functionality is similar to remapPaletteColor, except
		// 1) we have to work off the original CLUT rather than the current palette, and
		// 2) the target shadow palette entries must be bounded to the upper and lower
		//    bounds provided by the opcode. (This becomes significant in Room 48, but
		//    is not an issue in all other known case studies.)
		int j;
		int ar, ag, ab;
		uint sum, bestsum, bestitem = 0;

		if (r > 255)
			r = 255;
		if (g > 255)
			g = 255;
		if (b > 255)
			b = 255;

		bestsum = (uint)-1;

		r &= ~3;
		g &= ~3;
		b &= ~3;

		compareptr = basepal + startColor * 3;
		for (j = startColor; j <= endColor; j++, compareptr += 3) {
			ar = compareptr[0] & ~3;
			ag = compareptr[1] & ~3;
			ab = compareptr[2] & ~3;
			if (ar == r && ag == g && ab == b) {
				bestitem = j;
				break;
			}

			sum = colorWeight(ar - r, ag - g, ab - b);

			if (sum < bestsum) {
				bestsum = sum;
				bestitem = j;
			}
		}
		*table++ = bestitem;
	}
}

/** This function create the specialPalette used for semi-transparency in SamnMax */
void ScummEngine::createSpecialPalette(int16 from, int16 to, int16 redScale, int16 greenScale, int16 blueScale,
			int16 startColor, int16 endColor) {
	const byte *palPtr, *curPtr;
	const byte *searchPtr;

	uint bestResult;
	uint currentResult;

	byte currentIndex;

	int i, j;

	palPtr = getPalettePtr(_curPalIndex);

	for (i = 0; i < 256; i++)
		_proc_special_palette[i] = i;

	curPtr = palPtr + startColor * 3;

	for (i = startColor; i < endColor; i++) {
		int r = (int) (*curPtr++ * redScale) >> 8;
		int g = (int) (*curPtr++ * greenScale) >> 8;
		int b = (int) (*curPtr++ * blueScale) >> 8;

		if (r > 255)
			r = 255;
		if (g > 255)
			g = 255;
		if (b > 255)
			b = 255;

		searchPtr = palPtr + from * 3;
		bestResult = (uint)-1;
		currentIndex = (byte) from;

		for (j = from; j <= to; j++) {
			int ar = (*searchPtr++);
			int ag = (*searchPtr++);
			int ab = (*searchPtr++);

			currentResult = colorWeight(ar - r, ag - g, ab - b);

			if (currentResult < bestResult) {
				_proc_special_palette[i] = currentIndex;
				bestResult = currentResult;
			}
			currentIndex++;
		}
	}
}

void ScummEngine::darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	if (_roomResource == 0) // FIXME - HACK to get COMI demo working
		return;

	if (startColor <= endColor) {
		const byte *cptr;
		byte *cur;
		int j;
		int color;

		cptr = getPalettePtr(_curPalIndex) + startColor * 3;
		cur = _currentPalette + startColor * 3;

		for (j = startColor; j <= endColor; j++) {
			// FIXME: Hack to fix Amiga palette adjustments
			if ((_features & GF_AMIGA && _version == 5) && (j >= 16 && j < 81)) {
				cptr += 3;
				cur += 3;
				continue;
			}

			color = *cptr++;
			color = color * redScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;

			color = *cptr++;
			color = color * greenScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;

			color = *cptr++;
			color = color * blueScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;
		}
		setDirtyColors(startColor, endColor);
	}
}

static int HSL2RGBHelper(int n1, int n2, int hue) {
	if (hue > 360)
		hue = hue - 360;
	else if (hue < 0)
		hue = hue + 360;

	if (hue < 60)
		return n1 + (n2 - n1) * hue / 60;
	if (hue < 180)
		return n2;
	if (hue < 240)
		return n1 + (n2 - n1) * (240 - hue) / 60;
	return n1;
}

/**
 * This function scales the HSL (Hue, Saturation and Lightness)
 * components of the palette colors. It's used in CMI when Guybrush
 * walks from the beach towards the swamp.
 */
void ScummEngine::desaturatePalette(int hueScale, int satScale, int lightScale, int startColor, int endColor) {

	if (startColor <= endColor) {
		const byte *cptr;
		byte *cur;
		int j;

		cptr = getPalettePtr(_curPalIndex) + startColor * 3;
		cur = _currentPalette + startColor * 3;

		for (j = startColor; j <= endColor; j++) {
			int R = *cptr++;
			int G = *cptr++;
			int B = *cptr++;

			// RGB to HLS (Foley and VanDam)

			const int min = MIN(R, MIN(G, B));
			const int max = MAX(R, MAX(G, B));
			const int diff = (max - min);
			const int sum = (max + min);

			if (diff != 0) {
				int H, S, L;
				
				if (sum <= 255)
					S = 255 * diff / sum;
				else
					S = 255 * diff / (255 * 2 - sum);

				if (R == max)
					H = 60 * (G - B) / diff;
				else if (G == max)
					H = 120 + 60 * (B - R) / diff;
				else
					H = 240 + 60 * (R - G) / diff;

				if (H < 0)
					H = H + 360;

				// Scale the result
	
				H = (H * hueScale) / 255;
				S = (S * satScale) / 255;
				L = (sum * lightScale) / 255;
	
				// HLS to RGB (Foley and VanDam)
	
				int m1, m2;
				if (L <= 255)
					m2 = L * (255 + S) / (255 * 2);
				else
					m2 = L * (255 - S) / (255 * 2) + S;

				m1 = L - m2;

				R = HSL2RGBHelper(m1, m2, H + 120);
				G = HSL2RGBHelper(m1, m2, H);
				B = HSL2RGBHelper(m1, m2, H - 120);
			} else {
				// Maximal color = minimal color -> R=G=B -> it's a grayscale.
				R = G = B = (R * lightScale) / 255;
			}

			*cur++ = R;
			*cur++ = G;
			*cur++ = B;
		}

		setDirtyColors(startColor, endColor);
	}
}


int ScummEngine::remapPaletteColor(int r, int g, int b, uint threshold) {
	int i;
	int ar, ag, ab;
	uint sum, bestsum, bestitem = 0;
	byte *pal = _currentPalette;

	if (r > 255)
		r = 255;
	if (g > 255)
		g = 255;
	if (b > 255)
		b = 255;

	bestsum = (uint) - 1;

	r &= ~3;
	g &= ~3;
	b &= ~3;

	for (i = 0; i < 256; i++, pal += 3) {
		ar = pal[0] & ~3;
		ag = pal[1] & ~3;
		ab = pal[2] & ~3;
		if (ar == r && ag == g && ab == b)
			return i;

		sum = colorWeight(ar - r, ag - g, ab - b);

		if (sum < bestsum) {
			bestsum = sum;
			bestitem = i;
		}
	}

	if (threshold != (uint) - 1 && bestsum > colorWeight(threshold, threshold, threshold)) {
		// Best match exceeded threshold. Try to find an unused palette entry and
		// use it for our purpose.
		pal = _currentPalette + (256 - 2) * 3;
		for (i = 254; i > 48; i--, pal -= 3) {
			if (pal[0] >= 252 && pal[1] >= 252 && pal[2] >= 252) {
				setPalColor(i, r, g, b);
				return i;
			}
		}
	}

	return bestitem;
}

void ScummEngine::swapPalColors(int a, int b) {
	byte *ap, *bp;
	byte t;

	if ((uint) a >= 256 || (uint) b >= 256)
		error("swapPalColors: invalid values, %d, %d", a, b);

	ap = &_currentPalette[a * 3];
	bp = &_currentPalette[b * 3];

	t = ap[0];
	ap[0] = bp[0];
	bp[0] = t;
	t = ap[1];
	ap[1] = bp[1];
	bp[1] = t;
	t = ap[2];
	ap[2] = bp[2];
	bp[2] = t;

	setDirtyColors(a, a);
	setDirtyColors(b, b);
}

void ScummEngine::copyPalColor(int dst, int src) {
	byte *dp, *sp;

	if ((uint) dst >= 256 || (uint) src >= 256)
		error("copyPalColor: invalid values, %d, %d", dst, src);

	dp = &_currentPalette[dst * 3];
	sp = &_currentPalette[src * 3];

	dp[0] = sp[0];
	dp[1] = sp[1];
	dp[2] = sp[2];

	setDirtyColors(dst, dst);
}

void ScummEngine::setPalColor(int idx, int r, int g, int b) {
	_currentPalette[idx * 3 + 0] = r;
	_currentPalette[idx * 3 + 1] = g;
	_currentPalette[idx * 3 + 2] = b;
	setDirtyColors(idx, idx);
}

void ScummEngine::setPalette(int palindex) {
	const byte *pals;

	_curPalIndex = palindex;
	pals = getPalettePtr(_curPalIndex);
	setPaletteFromPtr(pals);
}

const byte *ScummEngine::findPalInPals(const byte *pal, int idx) {
	const byte *offs;
	uint32 size;

	pal = findResource(MKID('WRAP'), pal);
	if (pal == NULL)
		return NULL;

	offs = findResourceData(MKID('OFFS'), pal);
	if (offs == NULL)
		return NULL;

	size = getResourceDataSize(offs) / 4;

	if ((uint32)idx >= (uint32)size)
		return NULL;

	return offs + READ_LE_UINT32(offs + idx * sizeof(uint32));
}

const byte *ScummEngine::getPalettePtr(int palindex) {
	const byte *cptr;

	cptr = getResourceAddress(rtRoom, _roomResource);
	assert(cptr);
	if (_CLUT_offs) {
		cptr += _CLUT_offs;
	} else {
		cptr = findPalInPals(cptr + _PALS_offs, palindex);
	}
	assert(cptr);
	return cptr;
}

void ScummEngine::updatePalette() {
	if (_palDirtyMax == -1)
		return;

	bool noir_mode = (_gameId == GID_SAMNMAX && readVar(0x8000));
	int first = _palDirtyMin;
	int num = _palDirtyMax - first + 1;
	int i;

	byte palette_colors[1024];
	byte *p = palette_colors;

	for (i = _palDirtyMin; i <= _palDirtyMax; i++) {
		byte *data;

		if (_features & GF_SMALL_HEADER && _version > 2)
			data = _currentPalette + _shadowPalette[i] * 3;
		else
			data = _currentPalette + i * 3;

		// Sam & Max film noir mode. Convert the colours to grayscale
		// before uploading them to the backend.

		if (noir_mode) {
			int r, g, b;
			byte brightness;

			r = data[0];
			g = data[1];
			b = data[2];

			brightness = (byte)((0.299 * r + 0.587 * g + 0.114 * b) + 0.5);

			*p++ = brightness;
			*p++ = brightness;
			*p++ = brightness;
			*p++ = 0;
		} else {
			*p++ = data[0];
			*p++ = data[1];
			*p++ = data[2];
			*p++ = 0;
		}
	}
	
	_system->set_palette(palette_colors, first, num);

	_palDirtyMax = -1;
	_palDirtyMin = 256;
}

} // End of namespace Scumm
