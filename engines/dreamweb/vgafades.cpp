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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "dreamweb/dreamweb.h"

namespace DreamGen {

uint8 *DreamBase::mainPalette() {
// TODO: Turn these into plain C arrays
	return getSegment(data.word(kBuffers)).ptr(kMaingamepal, 256 * 3);
}

uint8 *DreamBase::startPalette() {
	return getSegment(data.word(kBuffers)).ptr(kStartpal, 256 * 3);
}

uint8 *DreamBase::endPalette() {
	return getSegment(data.word(kBuffers)).ptr(kEndpal, 256 * 3);
}

void DreamBase::clearStartPal() {
	memset(startPalette(), 0, 256 * 3);
}

void DreamBase::clearEndPal() {
	memset(endPalette(), 0, 256 * 3);
}

void DreamBase::palToStartPal() {
	memcpy(startPalette(), mainPalette(), 256 * 3);
}

void DreamBase::endPalToStart() {
	memcpy(startPalette(), endPalette(), 256 * 3);
}

void DreamBase::startPalToEnd() {
	memcpy(endPalette(), startPalette(), 256 * 3);
}

void DreamBase::palToEndPal() {
	memcpy(endPalette(), mainPalette(), 256 * 3);
}

void DreamBase::fadeDOS() {
	return; // FIXME later

	engine->waitForVSync();
	//processEvents will be called from vsync
	uint8 *dst = startPalette();
	engine->getPalette(dst, 0, 64);
	for (int fade = 0; fade < 64; ++fade) {
		for (int c = 0; c < 768; ++c) { //original sources decrement 768 values -> 256 colors
			if (dst[c]) {
				--dst[c];
			}
		}
		engine->setPalette(dst, 0, 64);
		engine->waitForVSync();
	}
}

void DreamBase::doFade() {
	if (data.byte(kFadedirection) == 0)
		return;

	engine->processEvents();
	uint8 *src = startPalette() + 3 * data.byte(kColourpos);
	engine->setPalette(src, data.byte(kColourpos), data.byte(kNumtofade));

	data.byte(kColourpos) += data.byte(kNumtofade);
	if (data.byte(kColourpos) == 0)
		fadeCalculation();
}

void DreamBase::fadeCalculation() {
	if (data.byte(kFadecount) == 0) {
		data.byte(kFadedirection) = 0;
		return;
	}

	uint8 *startPal = startPalette();
	const uint8 *endPal = endPalette();
	for (size_t i = 0; i < 256 * 3; ++i) {
		uint8 s = startPal[i];
		uint8 e = endPal[i];
		if (s == e)
			continue;
		else if (s > e)
			--startPal[i];
		else {
			if (data.byte(kFadecount) <= e)
				++startPal[i];
		}
	}
	--data.byte(kFadecount);
}

void DreamBase::fadeUpYellows() {
	palToEndPal();
	memset(endPalette() + 231 * 3, 0, 8 * 3);
	memset(endPalette() + 246 * 3, 0, 1 * 3);
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	hangOn(128);
}

void DreamBase::fadeUpMonFirst() {
	palToStartPal();
	palToEndPal();
	memset(startPalette() + 231 * 3, 0, 8 * 3);
	memset(startPalette() + 246 * 3, 0, 1 * 3);
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	hangOn(64);
	playChannel1(26);
	hangOn(64);
}


void DreamBase::fadeDownMon() {
	palToStartPal();
	palToEndPal();
	memset(endPalette() + 231 * 3, 0, 8 * 3);
	memset(endPalette() + 246 * 3, 0, 1 * 3);
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	hangOn(64);
}

void DreamBase::fadeUpMon() {
	palToStartPal();
	palToEndPal();
	memset(startPalette() + 231 * 3, 0, 8 * 3);
	memset(startPalette() + 246 * 3, 0, 1 * 3);
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	hangOn(128);
}

void DreamBase::initialMonCols() {
	palToStartPal();
	memset(startPalette() + 230 * 3, 0, 9 * 3);
	memset(startPalette() + 246 * 3, 0, 1 * 3);
	engine->processEvents();
	engine->setPalette(startPalette() + 230 * 3, 230, 18);
}

void DreamBase::fadeScreenUp() {
	clearStartPal();
	palToEndPal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
}

void DreamBase::fadeScreenUps() {
	clearStartPal();
	palToEndPal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 64;
}

void DreamBase::fadeScreenUpHalf() {
	endPalToStart();
	palToEndPal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 31;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 32;
}

void DreamBase::fadeScreenDown() {
	palToStartPal();
	clearEndPal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
}

void DreamBase::fadeScreenDowns() {
	palToStartPal();
	clearEndPal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 64;
}

void DreamBase::fadeScreenDownHalf() {
	palToStartPal();
	palToEndPal();

	const uint8 *startPal = startPalette();
	uint8 *endPal = endPalette();
	for (int i = 0; i < 256 * 3; ++i) {
		*endPal >>= 1;
		endPal++;
	}

	memcpy(endPal + (56*3), startPal + (56*3), 3*5);
	memcpy(endPal + (77*3), startPal + (77*3), 3*2);

	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 31;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 32;
}


void DreamBase::clearPalette() {
	data.byte(kFadedirection) = 0;
	clearStartPal();
	dumpCurrent();
}

// Converts palette to grey scale, summed using formula
// .20xred + .59xGreen + .11xBlue
void DreamBase::greyscaleSum() {
	byte *src = mainPalette();
	byte *dst = endPalette();

	for (int i = 0; i < 256; ++i) {
		const unsigned int r = 20 * *src++;
		const unsigned int g = 59 * *src++;
		const unsigned int b = 11 * *src++;
		const byte grey = (r + b + g) / 100;
		byte tmp;

		tmp = grey;
		//if (tmp != 0)	// FIXME: The assembler code has this check commented out. Bug or feature?
			tmp += data.byte(kAddtored);
		*dst++ = tmp;

		tmp = grey;
		if (tmp != 0)
			tmp += data.byte(kAddtogreen);
		*dst++ = tmp;

		tmp = grey;
		if (tmp != 0)
			tmp += data.byte(kAddtoblue);
		*dst++ = tmp;
	}
}

void DreamBase::allPalette() {
	memcpy(startPalette(), mainPalette(), 3 * 256);
	dumpCurrent();
}

void DreamBase::dumpCurrent() {
	uint8 *pal = startPalette();

	engine->waitForVSync();
	engine->processEvents();
	engine->setPalette(pal, 0, 128);

	pal += 128 * 3;

	engine->waitForVSync();
	engine->processEvents();
	engine->setPalette(pal, 128, 128);
}

void DreamGenContext::rollEndCredits2() {
	rollEm();
}

} // End of namespace DreamGen
