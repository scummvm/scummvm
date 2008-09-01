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
 * $URL$
 * $Id$
 *
 */


#include "common/endian.h"

#include "sword1/credits.h"
#include "sword1/screen.h"
#include "sword1/sword1.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

#include "common/file.h"
#include "common/util.h"
#include "common/events.h"
#include "common/system.h"


#define CREDITS_X 480
#define CREDITS_Y 300
#define BUFSIZE_Y 640

#define START_X ((640 - CREDITS_X) / 2)
#define START_Y ((480 - CREDITS_Y) / 2)

#define SCROLL_TIMING (2000 / 59) // 29.5 frames per second

#define LOGO_FADEUP_TIME (133 * 1000)
#define LOGO_FADEDOWN_TIME (163 * 1000)

namespace Sword1 {

enum {
	FONT_PAL = 0,
	FONT,
	TEXT,
	REVO_PAL,
	REVO_LOGO,
	F_EOF
};

enum {
	FNT_LFT = 0,   // left column
	FNT_RGT,	   // right column
	FNT_CEN,	   // centered
	FNT_BIG = 64,  // big font
	FNT_EOL = 128, // linebreak
	FNT_EOB = 255  // end of textblock
};


CreditsPlayer::CreditsPlayer(OSystem *pSystem, Audio::Mixer *pMixer) {
	_system = pSystem;
	_mixer = pMixer;
	_smlFont = _bigFont = NULL;
}

bool spaceInBuf(uint16 blitSta, uint16 blitEnd, uint16 renderDest) {
	if (blitEnd > blitSta) {
		if ((renderDest > blitEnd) || (renderDest + 15 < blitSta))
			return true;
	} else {
		if ((renderDest > blitEnd) && (renderDest + 15 < blitSta))
			return true;
	}
	return false;
}

void CreditsPlayer::play(void) {
	Audio::AudioStream *bgSoundStream = Audio::AudioStream::openStreamFile("credits");
	if (bgSoundStream == NULL) {
		warning("\"credits.ogg\" not found, skipping credits sequence");
		return;
	}
	ArcFile credFile;
	if (!credFile.open("credits.dat")) {
		warning("\"credits.dat\" not found, skipping credits sequence");
		return;
	}

	uint8 *palSrc = credFile.fetchFile(FONT_PAL, &_palLen);
	for (uint32 cnt = 0; cnt < _palLen; cnt++)
		_palette[(cnt / 3) * 4 + cnt % 3] = palSrc[cnt];
	_palLen /= 3;

	generateFonts(&credFile);

	uint8 *textData = credFile.fetchFile(TEXT);
	textData += READ_LE_UINT32(textData + SwordEngine::_systemVars.language * 4);

	uint8 *screenBuf = (uint8*)malloc(CREDITS_X * BUFSIZE_Y);
	memset(screenBuf, 0, CREDITS_X * BUFSIZE_Y);
	_system->copyRectToScreen(screenBuf, 640, 0, 0, 640, 480);
	_system->setPalette(_palette, 0, _palLen);

	// everything's initialized, time to render and show the credits.
	Audio::SoundHandle bgSound;
	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &bgSound, bgSoundStream, 0);

	int relDelay = 0;
	uint16 scrollY = 0;
	uint16 renderY = BUFSIZE_Y / 2;
	uint16 clearY = 0xFFFF;
	bool clearLine = false;
	while (((*textData != FNT_EOB) || (scrollY != renderY)) && !g_engine->quit()) {
		if ((int32)_mixer->getSoundElapsedTime(bgSound) - relDelay < (SCROLL_TIMING * 2)) { // sync to audio
			if (scrollY < BUFSIZE_Y - CREDITS_Y)
				_system->copyRectToScreen(screenBuf + scrollY * CREDITS_X, CREDITS_X, START_X, START_Y, CREDITS_X, CREDITS_Y);
			else {
				_system->copyRectToScreen(screenBuf + scrollY * CREDITS_X, CREDITS_X, START_X, START_Y, CREDITS_X, BUFSIZE_Y - scrollY);
				_system->copyRectToScreen(screenBuf, CREDITS_X, START_X, START_Y + BUFSIZE_Y - scrollY, CREDITS_X, CREDITS_Y - (BUFSIZE_Y - scrollY));
			}
			_system->updateScreen();
		} else
			warning("frame skipped");

		while (spaceInBuf(scrollY, (scrollY + CREDITS_Y) % BUFSIZE_Y, renderY) && (*textData != FNT_EOB)) {
			if (*textData & FNT_EOL) {
				renderY = (renderY + 16) % BUFSIZE_Y; // linebreak
				clearLine = true;
				*textData &= ~FNT_EOL;
			}
			if (spaceInBuf(scrollY, (scrollY + CREDITS_Y) % BUFSIZE_Y, renderY)) {
				if (clearLine)
					memset(screenBuf + renderY * CREDITS_X, 0, 16 * CREDITS_X);
				clearLine = false;
				renderLine(screenBuf, textData + 1, renderY, *textData);
				if (*textData & FNT_BIG)
					renderY += 16;
				while (*++textData != 0) // search for the start of next string
					;
				textData++;
			}
			if (*textData == FNT_EOB)
				clearY = renderY;
		}
		if ((*textData == FNT_EOB) && spaceInBuf(scrollY, (scrollY + CREDITS_Y) % BUFSIZE_Y, clearY)) {
			memset(screenBuf + clearY * CREDITS_X, 0, 16 * CREDITS_X);
			clearY = (clearY + 16) % BUFSIZE_Y;
		}

		relDelay += SCROLL_TIMING;
		delay(relDelay - (int32)_mixer->getSoundElapsedTime(bgSound));
		scrollY = (scrollY + 1) % BUFSIZE_Y;
	}
	free(_smlFont);
	free(_bigFont);
	_smlFont = _bigFont = NULL;
	free(screenBuf);

	// credits done, now show the revolution logo
	uint8 *revoBuf = credFile.decompressFile(REVO_LOGO);
	uint8 *revoPal = credFile.fetchFile(REVO_PAL, &_palLen);
	_palLen /= 3;
	while ((_mixer->getSoundElapsedTime(bgSound) < LOGO_FADEUP_TIME) && !g_engine->quit()) {
		delay(100);
	}
	memset(_palette, 0, 256 * 4);
	_system->setPalette(_palette, 0, 256);
	_system->copyRectToScreen(revoBuf, 480, START_X, START_Y, CREDITS_X, CREDITS_Y);
	_system->updateScreen();

	fadePalette(revoPal, true, _palLen);
	while ((_mixer->getSoundElapsedTime(bgSound) < LOGO_FADEDOWN_TIME) && !g_engine->quit()) {
		delay(100);
	}
	fadePalette(revoPal, false, _palLen);
	delay(3000);

	if (g_engine->quit())
		_mixer->stopAll();
	free(revoBuf);
}

void CreditsPlayer::fadePalette(uint8 *srcPal, bool fadeup, uint16 len) {
	int8 fadeDir = fadeup ? 1 : -1;
	int fadeStart = fadeup ? 0 : 12;

	int relDelay = _system->getMillis();
	for (int fadeStep = fadeStart; (fadeStep >= 0) && (fadeStep <= 12) && !g_engine->quit(); fadeStep += fadeDir) {
		for (uint16 cnt = 0; cnt < len * 3; cnt++)
			_palette[(cnt / 3) * 4 + (cnt % 3)] = (srcPal[cnt] * fadeStep) / 12;
		_system->setPalette(_palette, 0, 256);
		relDelay += 1000 / 12;
		delay(relDelay - _system->getMillis());
	}
}

void CreditsPlayer::renderLine(uint8 *screenBuf, uint8 *line, uint16 yBufPos, uint8 flags) {
	uint8 *font;
	uint16 fntSize = 16;
	if (flags & FNT_BIG) {
		font = _bigFont;
		fntSize = 32;
		flags &= ~FNT_BIG;
	} else
		font = _smlFont;

	uint16 width = getWidth(font, line);
	uint16 xBufPos = (flags == FNT_CEN) ? (CREDITS_X - width) / 2 : ((flags == FNT_LFT) ? (234 - width) : 255);
	uint8 *bufDest = screenBuf + yBufPos * CREDITS_X + xBufPos;
	while (*line) {
		uint8 *chrSrc = font + _numChars + (*line - 1) * fntSize * fntSize;
		for (uint16 cnty = 0; cnty < fntSize; cnty++) {
			for (uint16 cntx = 0; cntx < fntSize; cntx++)
				bufDest[cnty * CREDITS_X + cntx] = chrSrc[cntx];
			chrSrc += fntSize;
		}
		bufDest += font[*line++ - 1];
	}
}

uint16 CreditsPlayer::getWidth(uint8 *font, uint8 *line) {
	uint16 width = 0;
	while (*line)
		width += font[*line++ - 1];
	return width;
}

void CreditsPlayer::generateFonts(ArcFile *arcFile) {
	_bigFont = arcFile->decompressFile(FONT);
	_numChars = *_bigFont;
	memmove(_bigFont, _bigFont + 1, _numChars * (32 * 32 + 1));
	_smlFont = (uint8*)malloc(_numChars * (32 * 32 + 1));
	uint8 *src = _bigFont + _numChars;
	uint8 *dst = _smlFont + _numChars;
	for (uint16 cnt = 0; cnt < _numChars; cnt++) {
		_smlFont[cnt] = (_bigFont[cnt]++ + 1) / 2; // width table
		for (uint16 cnty = 0; cnty < 16; cnty++) {
			for (uint16 cntx = 0; cntx < 16; cntx++) {
				uint8 resR = (uint8)((_palette[src[0] * 4 + 0] + _palette[src[1] * 4 + 0] + _palette[src[32] * 4 + 0] + _palette[src[33] * 4 + 0]) >> 2);
				uint8 resG = (uint8)((_palette[src[0] * 4 + 1] + _palette[src[1] * 4 + 1] + _palette[src[32] * 4 + 1] + _palette[src[33] * 4 + 1]) >> 2);
				uint8 resB = (uint8)((_palette[src[0] * 4 + 2] + _palette[src[1] * 4 + 2] + _palette[src[32] * 4 + 2] + _palette[src[33] * 4 + 2]) >> 2);
				*dst++ = getPalIdx(resR, resG, resB);
				src += 2;
			}
			src += 32;
		}
	}
}

uint8 CreditsPlayer::getPalIdx(uint8 r, uint8 g, uint8 b) {
	for (uint16 cnt = 0; cnt < _palLen; cnt++)
		if ((_palette[cnt * 4 + 0] == r) && (_palette[cnt * 4 + 1] == g) && (_palette[cnt * 4 + 2] == b))
			return (uint8)cnt;
	assert(_palLen < 256);
	_palette[_palLen * 4 + 0] = r;
	_palette[_palLen * 4 + 1] = g;
	_palette[_palLen * 4 + 2] = b;
	return (uint8)_palLen++;
}

void CreditsPlayer::delay(int msecs) {

	Common::Event event;
	uint32 start = _system->getMillis();
	do {
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			default:
				break;
			}
		}

		_system->updateScreen();

		if (msecs > 0)
			_system->delayMillis(10);

	} while ((_system->getMillis() < start + msecs) && !g_engine->quit());
}

ArcFile::ArcFile(void) {
	_buf = NULL;
}

ArcFile::~ArcFile(void) {
	if (_buf)
		free(_buf);
}

bool ArcFile::open(const char *name) {
	Common::File arc;
	if (!arc.open(name))
		return false;
	_bufPos = _buf = (uint8*)malloc(arc.size());
	arc.read(_buf, arc.size());
	arc.close();
	return true;
}

void ArcFile::enterPath(uint32 id) {
	_bufPos += READ_LE_UINT32(_bufPos + id * 4);
}

uint8 *ArcFile::fetchFile(uint32 fileId, uint32 *size) {
	if (size)
		*size = READ_LE_UINT32(_bufPos + (fileId + 1) * 4) - READ_LE_UINT32(_bufPos + fileId * 4);
	return _bufPos + READ_LE_UINT32(_bufPos + fileId * 4);
}

uint8 *ArcFile::decompressFile(uint32 fileId) {
	uint32 size;
	uint8 *srcBuf = fetchFile(fileId, &size);
	uint8 *dstBuf = (uint8*)malloc(READ_LE_UINT32(srcBuf));
	uint8 *srcPos = srcBuf + 4;
	uint8 *dstPos = dstBuf;
	while (srcPos < srcBuf + size) {
		uint16 len = READ_LE_UINT16(srcPos);
		memset(dstPos, 0, len);
		dstPos += len;
		srcPos += 2;
		if (srcPos < srcBuf + size) {
			len = *srcPos++;
			memcpy(dstPos, srcPos, len);
			dstPos += len;
			srcPos += len;
		}
	}
	return dstBuf;
}

} // end of namespace Sword1
