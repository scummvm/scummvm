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
 * $URL$
 * $Id$
 */

#include "common/util.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "gui/debugger.h"
#include "engines/engine.h"

#include <stdarg.h>	// For va_list etc.

#ifdef _WIN32_WCE
// This is required for the debugger attachment
extern bool isSmartphone(void);
#endif

#ifdef __PLAYSTATION2__
	// for those replaced fopen/fread/etc functions
	typedef unsigned long	uint64;
	typedef signed long	int64;
	#include "backends/platform/ps2/fileio.h"

	#define fprintf				ps2_fprintf
	#define fputs(str, file)	ps2_fputs(str, file)
	#define fflush(a)			ps2_fflush(a)
#endif

#ifdef __DS__
	#include "backends/fs/ds/ds-fs.h"

	void	std_fprintf(FILE* handle, const char* fmt, ...);
	void	std_fflush(FILE* handle);

	#define fprintf(file, fmt, ...)				do { char str[128]; sprintf(str, fmt, ##__VA_ARGS__); DS::std_fwrite(str, strlen(str), 1, file); } while(0)
	#define fputs(str, file)					DS::std_fwrite(str, strlen(str), 1, file)
	#define fflush(file)						DS::std_fflush(file)
#endif


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
void hexdump(const byte * data, int len, int bytesPerLine, int startOffset) {
	assert(1 <= bytesPerLine && bytesPerLine <= 32);
	int i;
	byte c;
	int offset = startOffset;
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


#pragma mark -


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
	{"hu", "Hungarian", HU_HUN},
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


#pragma mark -


const PlatformDescription g_platforms[] = {
	{"2gs", "2gs", "2gs", "Apple IIgs", kPlatformApple2GS},
	{"3do", "3do", "3do", "3DO", kPlatform3DO},
	{"acorn", "acorn", "acorn", "Acorn", kPlatformAcorn},
	{"amiga", "ami", "amiga", "Amiga", kPlatformAmiga},
	{"atari", "atari-st", "st", "Atari ST", kPlatformAtariST},
	{"c64", "c64", "c64", "Commodore 64", kPlatformC64},
	{"pc", "dos", "ibm", "DOS", kPlatformPC},
	{"pc98", "pc98", "pc98", "PC-98", kPlatformPC98},
	{"wii", "wii", "wii", "Nintendo Wii", kPlatformWii},
	{"coco3", "coco3", "coco3", "CoCo3", kPlatformCoCo3},

	// The 'official' spelling seems to be "FM-TOWNS" (e.g. in the Indy4 demo).
	// However, on the net many variations can be seen, like "FMTOWNS",
	// "FM TOWNS", "FmTowns", etc.
	{"fmtowns", "towns", "fm", "FM-TOWNS", kPlatformFMTowns},

	{"linux", "linux", "linux", "Linux", kPlatformLinux},
	{"macintosh", "mac", "mac", "Macintosh", kPlatformMacintosh},
	{"pce", "pce", "pce", "PC-Engine", kPlatformPCEngine},
	{"nes", "nes", "nes", "NES", kPlatformNES},
	{"segacd", "segacd", "sega", "SegaCD", kPlatformSegaCD},
	{"windows", "win", "win", "Windows", kPlatformWindows},
	{"playstation", "psx", "psx", "Sony PlayStation", kPlatformPSX},
	{"cdi", "cdi", "cdi", "Phillips CD-i", kPlatformCDi},

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

	const RenderModeDescription *l = g_renderModes;
	for (; l->code; ++l) {
		if (str.equalsIgnoreCase(l->code))
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

const struct GameOpt {
	uint32 option;
	const char *desc;
} g_gameOptions[] = {
	{ GUIO_NOSUBTITLES, "sndNoSubs" },
	{ GUIO_NOMUSIC, "sndNoMusic" },
	{ GUIO_NOSPEECH, "sndNoSpeech" },
	{ GUIO_NOSFX, "sndNoSFX" },
	{ GUIO_NOMIDI, "sndNoMIDI" },
	{ GUIO_NOLAUNCHLOAD, "launchNoLoad" },
	{ GUIO_NONE, 0 }
};

bool checkGameGUIOption(GameGUIOption option, const String &str) {
	for (int i = 0; g_gameOptions[i].desc; i++) {
		if (g_gameOptions[i].option & option) {
			if (str.contains(g_gameOptions[i].desc))
				return true;
			else
				return false;
		}
	}
	return false;
}

uint32 parseGameGUIOptions(const String &str) {
	uint32 res = 0;

	for (int i = 0; g_gameOptions[i].desc; i++)
		if (str.contains(g_gameOptions[i].desc))
			res |= g_gameOptions[i].option;

	return res;
}

String getGameGUIOptionsDescription(uint32 options) {
	String res = "";

	for (int i = 0; g_gameOptions[i].desc; i++)
		if (options & g_gameOptions[i].option)
			res += String(g_gameOptions[i].desc) + " ";

	res.trim();

	return res;
}

void updateGameGUIOptions(const uint32 options) {
	if ((options && !ConfMan.hasKey("guioptions")) ||
	    (ConfMan.hasKey("guioptions") && options != parseGameGUIOptions(ConfMan.get("guioptions")))) {
		ConfMan.set("guioptions", getGameGUIOptionsDescription(options));
		ConfMan.flushToDisk();
	}
}

}	// End of namespace Common



#ifndef DISABLE_TEXT_CONSOLE

void warning(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

#if !defined (__SYMBIAN32__)
	fprintf(stderr, "WARNING: %s!\n", buf);
#endif

#if defined( USE_WINDBG )
	strcat(buf, "\n");
#if defined( _WIN32_WCE )
	TCHAR buf_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf) + 1, buf_unicode, sizeof(buf_unicode));
	OutputDebugString(buf_unicode);
#else
	OutputDebugString(buf);
#endif
#endif
}

#endif

void NORETURN error(const char *s, ...) {
	char buf_input[STRINGBUFLEN];
	char buf_output[STRINGBUFLEN];
	va_list va;

	// Generate the full error message
	va_start(va, s);
	vsnprintf(buf_input, STRINGBUFLEN, s, va);
	va_end(va);


	// Next, give the active engine (if any) a chance to augment the message
	if (g_engine) {
		g_engine->errorString(buf_input, buf_output, STRINGBUFLEN);
	} else {
		strncpy(buf_output, buf_input, STRINGBUFLEN);
	}

	buf_output[STRINGBUFLEN-3] = '\0';
	buf_output[STRINGBUFLEN-2] = '\0';
	buf_output[STRINGBUFLEN-1] = '\0';
	strcat(buf_output, "!\n");


	// Print the error message to stderr
	fputs(buf_output, stderr);

	// Unless this error -originated- within the debugger itself, we
	// now invoke the debugger, if available / supported.
	if (g_engine) {
		GUI::Debugger *debugger = g_engine->getDebugger();
#ifdef _WIN32_WCE
		if (isSmartphone())
			debugger = 0;
#endif
		if (debugger && !debugger->isAttached()) {
			debugger->attach(buf_output);
			debugger->onFrame();
		}
	}


#if defined( USE_WINDBG )
#if defined( _WIN32_WCE )
	TCHAR buf_output_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf_output, strlen(buf_output) + 1, buf_output_unicode, sizeof(buf_output_unicode));
	OutputDebugString(buf_output_unicode);
#ifndef DEBUG
	drawError(buf_output);
#else
	int cmon_break_into_the_debugger_if_you_please = *(int *)(buf_output + 1);	// bus error
	printf("%d", cmon_break_into_the_debugger_if_you_please);			// don't optimize the int out
#endif
#else
	OutputDebugString(buf_output);
#endif
#endif

#ifdef PALMOS_MODE
	extern void PalmFatalError(const char *err);
	PalmFatalError(buf_output);
#endif

#ifdef __SYMBIAN32__
	Symbian::FatalError(buf_output);
#endif
	// Finally exit. quit() will terminate the program if g_system is present
	if (g_system)
		g_system->quit();

	exit(1);
}
