/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 */

#include "common/util.h"
#include "common/system.h"
#include "common/str.h"

namespace Common {

StringTokenizer::StringTokenizer(const String &str, const String &delimiters) : _str(str), _delimiters(delimiters) {
	reset();
}

void StringTokenizer::reset() {
	_tokenBegin = _tokenEnd = 0;
}

bool StringTokenizer::empty() const {
	// Search for the next token's start (i.e. the next non-delimiter character)
	for (uint i = _tokenEnd; i < _str.size(); i++) {
		if (!_delimiters.contains(_str[i]))
			return false; // Found a token so the tokenizer is not empty
	}
	// Didn't find any more tokens so the tokenizer is empty
	return true;
}

String StringTokenizer::nextToken() {
	// Seek to next token's start (i.e. jump over the delimiters before next token)
	for (_tokenBegin = _tokenEnd; _tokenBegin < _str.size() && _delimiters.contains(_str[_tokenBegin]); _tokenBegin++)
		;
	// Seek to the token's end (i.e. jump over the non-delimiters)
	for (_tokenEnd = _tokenBegin; _tokenEnd < _str.size() && !_delimiters.contains(_str[_tokenEnd]); _tokenEnd++)
		;
	// Return the found token
	return String(_str.c_str() + _tokenBegin, _tokenEnd - _tokenBegin);
}


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

String tag2string(uint32 tag) {
	char str[5];
	str[0] = (char)(tag >> 24);
	str[1] = (char)(tag >> 16);
	str[2] = (char)(tag >> 8);
	str[3] = (char)tag;
	str[4] = '\0';
	// Replace non-printable chars by dot
	for (int i = 0; i < 4; ++i) {
		if (!isprint(str[i]))
			str[i] = '.';
	}
	return Common::String(str);
}


#pragma mark -


RandomSource::RandomSource() {
	// Use system time as RNG seed. Normally not a good idea, if you are using
	// a RNG for security purposes, but good enough for our purposes.
	assert(g_system);
	uint32 seed = g_system->getMillis();
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

uint RandomSource::getRandomBit(void) {
	_randSeed = 0xDEADBF03 * (_randSeed + 1);
	_randSeed = (_randSeed >> 13) | (_randSeed << 19);
	return _randSeed & 1;
}

uint RandomSource::getRandomNumberRng(uint min, uint max) {
	return getRandomNumber(max - min) + min;
}


const LanguageDescription g_languages[] = {
	{"zh", "Chinese (Taiwan)", ZH_TWN},
	{"cz", "Czech", CZ_CZE},
	{"nl", "Dutch", NL_NLD},
	{"en", "English", EN_ANY}, // Generic English (when only one game version exist)
	{"gb", "English (GB)", EN_GRB},
	{"us", "English (US)", EN_USA},
	{"fr", "French", FR_FRA},
	{"de", "German", DE_DEU},
	{"gr", "Greek", GR_GRE},
	{"hb", "Hebrew", HB_ISR},
	{"it", "Italian", IT_ITA},
	{"jp", "Japanese", JA_JPN},
	{"kr", "Korean", KO_KOR},
	{"nb", "Norwegian Bokm\xE5l", NB_NOR},
	{"pl", "Polish", PL_POL},
	{"br", "Portuguese", PT_BRA},
	{"ru", "Russian", RU_RUS},
	{"es", "Spanish", ES_ESP},
	{"se", "Swedish", SE_SWE},
	{0, 0, UNK_LANG}
};

Language parseLanguage(const String &str) {
	if (str.empty())
		return UNK_LANG;

	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (str.equalsIgnoreCase(l->code))
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


const PlatformDescription g_platforms[] = {
	{"2gs", "2gs", "2gs", "Apple IIgs", kPlatformApple2GS },
	{"3do", "3do", "3do", "3DO", kPlatform3DO},
	{"acorn", "acorn", "acorn", "Acorn", kPlatformAcorn},
	{"amiga", "ami", "amiga", "Amiga", kPlatformAmiga},
	{"atari", "atari-st", "st", "Atari ST", kPlatformAtariST},
	{"c64", "c64", "c64", "Commodore 64", kPlatformC64},
	{"pc", "dos", "ibm", "DOS", kPlatformPC},
	{"pc98", "pc98", "pc98", "PC-98", kPlatformPC98},
	{"wii", "wii", "wii", "Nintendo Wii", kPlatformWii},

	// The 'official' spelling seems to be "FM-TOWNS" (e.g. in the Indy4 demo).
	// However, on the net many variations can be seen, like "FMTOWNS",
	// "FM TOWNS", "FmTowns", etc.
	{"fmtowns", "towns", "fm", "FM-TOWNS", kPlatformFMTowns},

	{"linux", "linux", "linux", "Linux", kPlatformLinux},
	{"macintosh", "mac", "mac", "Macintosh", kPlatformMacintosh},
	{"pce", "pce", "pce", "PC-Engine", kPlatformPCEngine },
	{"nes", "nes", "nes", "NES", kPlatformNES},
	{"segacd", "segacd", "sega", "SegaCD", kPlatformSegaCD},
	{"windows", "win", "win", "Windows", kPlatformWindows},
	{"playstation", "psx", "PSX", "Playstation", kPlatformPSX},


	{0, 0, 0, "Default", kPlatformUnknown}
};

Platform parsePlatform(const String &str) {
	if (str.empty())
		return kPlatformUnknown;

	// Handle some special case separately, for compatibility with old config
	// files.
	if (str == "1")
		return kPlatformAmiga;
	else if (str == "2")
		return kPlatformAtariST;
	else if (str == "3")
		return kPlatformMacintosh;

	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (str.equalsIgnoreCase(l->code) || str.equalsIgnoreCase(l->code2) || str.equalsIgnoreCase(l->abbrev))
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

const char *getPlatformAbbrev(Platform id) {
	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->abbrev;
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

