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

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/array.h"

#if defined (__INNOTEK_LIBC__) || (defined (__amigaos4__) && defined(__NEWLIB__))
#undef MIN
#undef MAX
#endif

template<typename T> inline T ABS (T x)			{ return (x>=0) ? x : -x; }
#if !defined(MIN)
template<typename T> inline T MIN (T a, T b)	{ return (a<b) ? a : b; }
#endif
#if !defined(MAX)
template<typename T> inline T MAX (T a, T b)	{ return (a>b) ? a : b; }
#endif

/**
 * Template method which swaps the vaulues of its two parameters.
 */
template<typename T> inline void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }

#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

namespace Common {

/**
 * Print a hexdump of the data passed in. The number of bytes per line is
 * customizable.
 * @param data	the data to be dumped
 * @param len	the lenght of that data
 * @param bytesPerLine	number of bytes to print per line (default: 16)
 */
extern void hexdump(const byte * data, int len, int bytesPerLine = 16);

/**
 * Simple random number generator. Although it is definitely not suitable for
 * cryptographic purposes, it serves our purposes just fine.
 */
class RandomSource {
private:
	uint32 _randSeed;

public:
	RandomSource();
	void setSeed(uint32 seed);

	/**
	 * Generates a random unsigned integer in the interval [0, max].
	 * @param max	the upper bound
	 * @return	a random number in the interval [0, max].
	 */
	uint getRandomNumber(uint max);
	/**
	 * Generates a random unsigned integer in the interval [min, max].
	 * @param min	the lower bound
	 * @param max	the upper bound
	 * @return	a random number in the interval [min, max].
	 */
	uint getRandomNumberRng(uint min, uint max);
};

/**
 * List of language ids.
 * @note The order and mappings of the values 0..8 are *required* to stay the
 * way they are now, as scripts in COMI rely on them. So don't touch them.
 * I am working on removing this restriction.
 */
enum Language {
	UNK_LANG = -1,	// Use default language (i.e. none specified)
	EN_USA = 0,
	DE_DEU = 1,
	FR_FRA = 2,
	IT_ITA = 3,
	PT_BRA = 4,
	ES_ESP = 5,
	JA_JPN = 6,
	ZH_TWN = 7,
	KO_KOR = 8,
	SE_SWE = 9,
	EN_GRB = 10,
	HB_ISR = 20,
	RU_RUS = 21,
	CZ_CZE = 22,
	NL_NLD = 23,
	NB_NOR = 24,
	PL_POL = 25
};

struct LanguageDescription {
	const char *code;
	const char *description;
	Common::Language id;
};

extern const LanguageDescription g_languages[];


/** Convert a string containing a language name into a Language enum value. */
extern Language parseLanguage(const String &str);
extern const char *getLanguageCode(Language id);
extern const char *getLanguageDescription(Language id);

/**
 * List of game platforms. Specifying a platform for a target can be used to
 * give the game engines a hint for which platform the game data file are.
 * This may be optional or required, depending on the game engine and the
 * game in question.
 */
enum Platform {
	kPlatformUnknown = -1,
	kPlatformPC = 0,
	kPlatformAmiga = 1,
	kPlatformAtariST = 2,
	kPlatformMacintosh = 3,
	kPlatformFMTowns = 4,
	kPlatformWindows = 5,
	kPlatformNES = 6,
	kPlatformC64 = 7,
	kPlatformLinux = 8,
	kPlatformAcorn = 9,
	kPlatformSegaCD = 10,
	kPlatform3DO = 11
/*
	kPlatformPCEngine
*/
};

struct PlatformDescription {
	const char *code;
	const char *code2;
	const char *description;
	Common::Platform id;
};

extern const PlatformDescription g_platforms[];

/** Convert a string containing a platform name into a Platform enum value. */
extern Platform parsePlatform(const String &str);
extern const char *getPlatformCode(Platform id);
extern const char *getPlatformDescription(Platform id);

/**
 * List of render modes. It specifies which original graphics mode
 * to use. Some targets used postprocessing dithering routines for
 * reducing color depth of final image which let it to be rendered on
 * such low-level adapters as CGA or Hercules.
 */
enum RenderMode {
	kRenderDefault = 0,
	kRenderEGA = 1,
	kRenderCGA = 2,
	kRenderHercG = 3,
	kRenderHercA = 4,
	kRenderAmiga = 5
};

enum HerculesDimesnions {
	kHercW = 720,
	kHercH = 350
};

struct RenderModeDescription {
	const char *code;
	const char *description;
	Common::RenderMode id;
};

extern const RenderModeDescription g_renderModes[];

/** Convert a string containing a render mode name into a RenderingMode enum value. */
extern RenderMode parseRenderMode(const String &str);
extern const char *getRenderModeCode(RenderMode id);
extern const char *getRenderModeDescription(RenderMode id);


struct EngineDebugLevel {
	EngineDebugLevel() : option(""), description(""), level(0), enabled(false) {}
	EngineDebugLevel(uint32 l, const String &o, const String &d)
		: option(o), description(d), level(l), enabled(false) {}
	EngineDebugLevel(const EngineDebugLevel &copy)
		: option(copy.option), description(copy.description), level(copy.level), enabled(copy.enabled) {}

	EngineDebugLevel &operator =(const EngineDebugLevel &copy) {
		option = copy.option;
		description = copy.description;
		level = copy.level;
		enabled = copy.enabled;
		return *this;
	}

	String option;
	String description;

	uint32 level;
	bool enabled;
};

/**
 * Adds a engine debug level.
 * @param level the level flag (should be OR-able i.e. first one should be 1 than 2,4,...)
 * @param option the option name which is used in the debugger/on the command line to enable
 *               this special debug level, the option will be compared case !insentiv! later
 * @param descripton the description which shows up in the debugger
 * @return true on success false on failure
 */
bool addSpecialDebugLevel(uint32 level, const String &option, const String &description);

/**
 * Resets all engine debug levels
 */
void clearAllSpecialDebugLevels();

/**
 * Enables a engine debug level
 * @param option the option which should be enabled
 * @return true on success false on failure
 */
bool enableSpecialDebugLevel(const String &option);

// only used for parsing the levels from the commandline
void enableSpecialDebugLevelList(const String &option);

/**
 * Disables a engine debug level
 * @param option the option to disable
 * @return true on success false on failure
 */
bool disableSpecialDebugLevel(const String &option);

/**
 * Lists all debug levels
 * @return returns a arry with all debug levels
 */
const Array<EngineDebugLevel> &listSpecialDebugLevels();

}	// End of namespace Common


#if defined(__GNUC__)
void CDECL error(const char *s, ...) GCC_PRINTF(1, 2) NORETURN;
#else
void CDECL NORETURN error(const char *s, ...);
#endif

void CDECL warning(const char *s, ...) GCC_PRINTF(1, 2);

void CDECL debug(int level, const char *s, ...) GCC_PRINTF(2, 3);
void CDECL debug(const char *s, ...) GCC_PRINTF(1, 2);
void CDECL debugN(int level, const char *s, ...) GCC_PRINTF(2, 3);
void CDECL debugC(int level, uint32 engine_level, const char *s, ...) GCC_PRINTF(3, 4);

void checkHeap();

extern int gDebugLevel;


#endif
