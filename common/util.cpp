/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
#include "base/engine.h"
#include "common/util.h"

namespace Common {

//
// Print hexdump of the data passed in
//
void hexdump(const byte * data, int len, int bytesPerLine) {
	assert(1 <= bytesPerLine && bytesPerLine <= 32);
	int i;
	byte c;
	int offset = 0;
	while (len >= bytesPerLine) {
		printf("%06x: ", offset);
		for (i = 0; i < bytesPerLine; i++) {
			printf("%02x ", data[i]);
			if (i % 4 == 3)
				printf(" ");
		}
		printf(" |");
		for (i = 0; i < bytesPerLine; i++) {
			c = data[i];
			if (c < 32 || c >= 127)
				c = '.';
			printf("%c", c);
		}
		printf("|\n");
		data += bytesPerLine;
		len -= bytesPerLine;
		offset += bytesPerLine;
	}

	if (len <= 0) 
		return;

	printf("%06x: ", offset);
	for (i = 0; i < bytesPerLine; i++) {
		if (i < len)
			printf("%02x ", data[i]);
		else
			printf("   ");
		if (i % 4 == 3)
			printf(" ");
	}
	printf(" |");
	for (i = 0; i < len; i++) {
		c = data[i];
		if (c < 32 || c >= 127)
			c = '.';
		printf("%c", c);
	}
	for (; i < bytesPerLine; i++)
		printf(" ");
	printf("|\n");
}

#pragma mark -


RandomSource::RandomSource() {
	// Use system time as RNG seed. Normally not a good idea, if you are using
	// a RNG for security purposes, but good enough for our purposes.
	setSeed(time(0));
}

void RandomSource::setSeed(uint32 seed) {
	_randSeed = seed;
}

uint RandomSource::getRandomNumber(uint max) {
	/* TODO: my own random number generator */
	_randSeed = 0xDEADBF03 * (_randSeed + 1);
	_randSeed = (_randSeed >> 13) | (_randSeed << 19);
	return _randSeed % (max + 1);
}

uint RandomSource::getRandomNumberRng(uint min, uint max) {
	return getRandomNumber(max - min) + min;
}

#pragma mark -


StackLock::StackLock(OSystem::MutexRef mutex, OSystem *syst)
	: _mutex(mutex), _syst(syst) {
	if (syst == 0)
		_syst = g_system;
	lock();
}

StackLock::~StackLock() {
	unlock();
}

void StackLock::lock() {
	assert(_syst);
	_syst->lock_mutex(_mutex);
}

void StackLock::unlock() {
	assert(_syst);
	_syst->unlock_mutex(_mutex);
}


#pragma mark -


const LanguageDescription g_languages[] = {
	{"en", "English (US)", EN_USA},
	{"de", "German", DE_DEU},
	{"fr", "French", FR_FRA},
	{"it", "Italian", IT_ITA},
	{"pt", "Portuguese", PT_BRA},
	{"es", "Spanish", ES_ESP},
	{"jp", "Japanese", JA_JPN},
	{"zh", "Chinese (Taiwan)", ZH_TWN},
	{"kr", "Korean", KO_KOR},
	{"gb", "English (GB)", EN_GRB},
	{"se", "Swedish", SE_SWE},
	{"hb", "Hebrew", HB_HEB},
	{"ru", "Russian", RU_RUS},
	{"cz", "Czech", CZ_CZE},
	{0, 0, UNK_LANG}
};

Language parseLanguage(const String &str) {
	if (str.isEmpty())
		return UNK_LANG;

	const char *s = str.c_str();
	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (!scumm_stricmp(l->code, s))
			return l->id;
	}

	return UNK_LANG;
}

const char *getLanguageCode(Language id) {
	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->code;
	}
	return 0;
}

const char *getLanguageDescription(Language id) {
	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->description;
	}
	return 0;
}


#pragma mark -


struct PlatformDescription {
	const char *code;
	const char *description;
	Common::Platform id;
};

static const PlatformDescription g_platforms[] = {
	{"pc", "PC", kPlatformPC},
	{"amiga", "Amiga", kPlatformAmiga},
	{"atari", "Atari ST", kPlatformAtariST},
	{"macintosh", "Macintosh", kPlatformMacintosh},
	{0, "Default", kPlatformUnknown}
};

Platform parsePlatform(const String &str) {
	if (str.isEmpty())
		return kPlatformUnknown;

	const char *s = str.c_str();
	
	// Handle some special case separately, for compatibility with old config
	// files.
	if (!scumm_stricmp(s, "amiga") || !scumm_stricmp(s, "1"))
		return kPlatformAmiga;
	else if (!scumm_stricmp(s, "atari-st") || !scumm_stricmp(s, "2"))
		return kPlatformAtariST;
	else if (!scumm_stricmp(s, "mac") || !scumm_stricmp(s, "3"))
		return kPlatformMacintosh;

	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (!scumm_stricmp(l->code, s))
			return l->id;
	}

	return kPlatformUnknown;
}


const char *getPlatformCode(Platform id) {
	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->code;
	}
	return 0;
}

const char *getPlatformDescription(Platform id) {
	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->description;
	}
	return l->description;
}



}	// End of namespace Common
