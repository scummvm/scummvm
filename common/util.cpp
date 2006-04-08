/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"
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
#if defined (__SYMBIAN32__) && defined (__WINS__)
	uint32 seed = 0; // Symbian produces RT crash on time(0)
#else
	uint32 seed = time(0);
#endif
	setSeed(seed);
}

void RandomSource::setSeed(uint32 seed) {
	_randSeed = seed;
}

uint RandomSource::getRandomNumber(uint max) {
	_randSeed = 0xDEADBF03 * (_randSeed + 1);
	_randSeed = (_randSeed >> 13) | (_randSeed << 19);
	return _randSeed % (max + 1);
}

uint RandomSource::getRandomNumberRng(uint min, uint max) {
	return getRandomNumber(max - min) + min;
}


#pragma mark -


const LanguageDescription g_languages[] = {
	{"en", "English", EN_ANY}, // Generic English (when only one game version exist)
	{"us", "English (US)", EN_USA},
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
	{"hb", "Hebrew", HB_ISR},
	{"ru", "Russian", RU_RUS},
	{"cz", "Czech", CZ_CZE},
	{"nl", "Dutch", NL_NLD},
	{"nb", "Norwegian Bokm\xE5l", NB_NOR},
	{"pl", "Polish", PL_POL},
	{0, 0, UNK_LANG}
};

Language parseLanguage(const String &str) {
	if (str.empty())
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


const PlatformDescription g_platforms[] = {
	{"3do", "3do", "3DO", kPlatform3DO},
	{"acorn", "acorn", "Acorn", kPlatformAcorn},
	{"amiga", "ami", "Amiga", kPlatformAmiga},
	{"atari", "atari-st", "Atari ST", kPlatformAtariST},
	{"c64", "c64", "Commodore 64", kPlatformC64},
	{"pc", "dos", "DOS", kPlatformPC},

	// The 'official' spelling seems to be "FM-TOWNS" (e.g. in the Indy4 demo).
	// However, on the net many variations can be seen, like "FMTOWNS",
	// "FM TOWNS", "FmTowns", etc.
	{"fmtowns", "towns", "FM-TOWNS", kPlatformFMTowns},

	{"linux", "linux", "Linux", kPlatformLinux},
	{"macintosh", "mac", "Macintosh", kPlatformMacintosh},
	{"nes", "nes", "NES", kPlatformNES},
	{"segacd", "segacd", "SegaCD", kPlatformSegaCD},
	{"windows", "win", "Windows", kPlatformWindows},

	{0, 0, "Default", kPlatformUnknown}
};

Platform parsePlatform(const String &str) {
	if (str.empty())
		return kPlatformUnknown;

	const char *s = str.c_str();

	// Handle some special case separately, for compatibility with old config
	// files.
	if (!strcmp(s, "1"))
		return kPlatformAmiga;
	else if (!strcmp(s, "2"))
		return kPlatformAtariST;
	else if (!strcmp(s, "3"))
		return kPlatformMacintosh;

	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (!scumm_stricmp(l->code, s) || !scumm_stricmp(l->code2, s))
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


#pragma mark -


const RenderModeDescription g_renderModes[] = {
	{"hercGreen", "Hercules Green", kRenderHercG},
	{"hercAmber", "Hercules Amber", kRenderHercA},
	{"cga", "CGA", kRenderCGA},
	{"ega", "EGA", kRenderEGA},
	{"amiga", "Amiga", kRenderAmiga},
	{0, 0, kRenderDefault}
};

RenderMode parseRenderMode(const String &str) {
	if (str.empty())
		return kRenderDefault;

	const char *s = str.c_str();
	const RenderModeDescription *l = g_renderModes;
	for (; l->code; ++l) {
		if (!scumm_stricmp(l->code, s))
			return l->id;
	}

	return kRenderDefault;
}

const char *getRenderModeCode(RenderMode id) {
	const RenderModeDescription *l = g_renderModes;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->code;
	}
	return 0;
}

const char *getRenderModeDescription(RenderMode id) {
	const RenderModeDescription *l = g_renderModes;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->description;
	}
	return 0;
}

#pragma mark -

Array<EngineDebugLevel> gDebugLevels;
uint32 gDebugLevelsEnabled = 0;

bool addSpecialDebugLevel(uint32 level, const String &option, const String &description) {
	for (uint i = 0; i < gDebugLevels.size(); ++i) {
		if (!scumm_stricmp(option.c_str(), gDebugLevels[i].option.c_str())) {
			warning("Declared engine debug level '%s' again", option.c_str());
			gDebugLevels[i] = EngineDebugLevel(level, option, description);
			return true;
		}
	}
	gDebugLevels.push_back(EngineDebugLevel(level, option, description));
	return true;
}

void clearAllSpecialDebugLevels() {
	gDebugLevelsEnabled = 0;
	gDebugLevels.clear();
}

bool enableSpecialDebugLevel(const String &option) {
	for (uint i = 0; i < gDebugLevels.size(); ++i) {
		if (!scumm_stricmp(option.c_str(), gDebugLevels[i].option.c_str())) {
			gDebugLevelsEnabled |= gDebugLevels[i].level;
			gDebugLevels[i].enabled = true;
			return true;
		}
	}
	return false;
}

void enableSpecialDebugLevelList(const String &option) {
	uint start = 0;
	uint end = 0;
	
	const char *str = option.c_str();
	for (end = start + 1; end <= option.size(); ++end) {
		if (str[end] == ',' || end == option.size()) {
			if (!enableSpecialDebugLevel(Common::String(&str[start], end-start))) {
				warning("Engine does not support debug level '%s'", Common::String(&str[start], end-start).c_str());
			}
			start = end + 1;
		}
	}
}

bool disableSpecialDebugLevel(const String &option) {
	for (uint i = 0; i < gDebugLevels.size(); ++i) {
		if (!scumm_stricmp(option.c_str(), gDebugLevels[i].option.c_str())) {
			gDebugLevelsEnabled &= ~gDebugLevels[i].level;
			gDebugLevels[i].enabled = false;
			return true;
		}
	}
	return false;
}

const Array<EngineDebugLevel> &listSpecialDebugLevels() {
	return gDebugLevels;
}

}	// End of namespace Common



/**
 * The debug level. Initially set to -1, indicating that no debug output
 * should be shown. Positive values usually imply an increasing number of
 * debug output shall be generated, the higher the value, the more verbose the
 * information (although the exact semantics are up to the engines).
 */
int gDebugLevel = -1;



static void debugHelper(char *buf, bool caret = true) {
#ifndef _WIN32_WCE
	if (caret)
		printf("%s\n", buf);
	else
		printf("%s", buf);
#endif

#if defined( USE_WINDBG )
	if (caret)
		strcat(buf, "\n");
#if defined( _WIN32_WCE )
	TCHAR buf_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf) + 1, buf_unicode, sizeof(buf_unicode));
	OutputDebugString(buf_unicode);
#else
	OutputDebugString(buf);
#endif
#endif

	fflush(stdout);
}

void CDECL debug(int level, const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	if (level > gDebugLevel)
		return;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	debugHelper(buf);
}

void CDECL debugN(int level, const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	if (level > gDebugLevel)
		return;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	debugHelper(buf, false);
}

void CDECL debug(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	debugHelper(buf);
}

void CDECL debugC(int level, uint32 engine_level, const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	if (level > gDebugLevel || !(Common::gDebugLevelsEnabled & engine_level))
		return;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	debugHelper(buf);
}
