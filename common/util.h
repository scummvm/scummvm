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

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/list.h"

#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

template<typename T> inline T ABS (T x)			{ return (x>=0) ? x : -x; }
template<typename T> inline T MIN (T a, T b)	{ return (a<b) ? a : b; }
template<typename T> inline T MAX (T a, T b)	{ return (a>b) ? a : b; }
template<typename T> inline T CLIP (T v, T amin, T amax)
		{ if (v < amin) return amin; else if (v > amax) return amax; else return v; }

/**
 * Template method which swaps the vaulues of its two parameters.
 */
template<typename T> inline void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }

#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

namespace Common {

/**
 * Simple DOS-style pattern matching function (understands * and ? like used in DOS).
 * Taken from exult/files/listfiles.cc
 *
 * Token meaning:
 *		"*": any character, any amount of times.
 *		"?": any character, only once.
 *
 * Example strings/patterns:
 *		String: monkey.s??	 Pattern: monkey.s01	=> true
 *		String: monkey.s??	 Pattern: monkey.s101	=> false
 *		String: monkey.s?1	 Pattern: monkey.s99	=> false
 *		String: monkey.s*	 Pattern: monkey.s101	=> true
 *		String: monkey.s*1	 Pattern: monkey.s99	=> false
 *
 * @param str Text to be matched against the given pattern.
 * @param pat Glob pattern.
 *
 * @return true if str matches the pattern, false otherwise.
 */
bool matchString(const char *str, const char *pat);

/**
 * A simple non-optimized string tokenizer.
 *
 * Example of use:
 * StringTokenizer("Now, this is a test!", " ,!") gives tokens "Now", "this", "is", "a" and "test" using nextToken().
 */
class StringTokenizer {
public:
	/**
	 * Creates a StringTokenizer.
	 * @param str The string to be tokenized.
	 * @param delimiters String containing all the delimiter characters (i.e. the characters to be ignored).
	 * @note Uses space, horizontal tab, carriage return, newline, form feed and vertical tab as delimiters by default.
	 */
	StringTokenizer(const String &str, const String &delimiters = " \t\r\n\f\v");
	void reset();       //!< Resets the tokenizer to its initial state
	bool empty() const; //!< Returns true if there are no more tokens left in the string, false otherwise
	String nextToken(); //!< Returns the next token from the string (Or an empty string if there are no more tokens)

private:
	const String _str;        //!< The string to be tokenized
	const String _delimiters; //!< String containing all the delimiter characters
	uint         _tokenBegin; //!< Latest found token's begin (Valid after a call to nextToken(), zero otherwise)
	uint         _tokenEnd;   //!< Latest found token's end (Valid after a call to nextToken(), zero otherwise)
};

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

	uint32 getSeed() {
		return _randSeed;
	}

	/**
	 * Generates a random unsigned integer in the interval [0, max].
	 * @param max	the upper bound
	 * @return	a random number in the interval [0, max].
	 */
	uint getRandomNumber(uint max);
	/**
	 * Generates a random unsigned integer in the interval [0, 1].
	 * Identical to getRandomNumber(1), but faster, hopefully.
	 * @return	a random number in the interval [0, max].
	 */
	uint getRandomBit(void);
	/**
	 * Generates a random unsigned integer in the interval [min, max].
	 * @param min	the lower bound
	 * @param max	the upper bound
	 * @return	a random number in the interval [min, max].
	 */
	uint getRandomNumberRng(uint min, uint max);
};

/**
 * List of game language.
 */
enum Language {
	ZH_TWN,
	CZ_CZE,
	NL_NLD,
	EN_ANY,     // Generic English (when only one game version exist)
	EN_GRB,
	EN_USA,
	FR_FRA,
	DE_DEU,
	GR_GRE,
	HB_ISR,
	IT_ITA,
	JA_JPN,
	KO_KOR,
	NB_NOR,
	PL_POL,
	PT_BRA,
	RU_RUS,
	ES_ESP,
	SE_SWE,

	UNK_LANG = -1	// Use default language (i.e. none specified)
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
	kPlatformPC,
	kPlatformAmiga,
	kPlatformAtariST,
	kPlatformMacintosh,
	kPlatformFMTowns,
	kPlatformWindows,
	kPlatformNES,
	kPlatformC64,
	kPlatformLinux,
	kPlatformAcorn,
	kPlatformSegaCD,
	kPlatform3DO,
	kPlatformPCEngine,

	kPlatformApple2GS,
	kPlatformPC98,

	kPlatformUnknown = -1
};

struct PlatformDescription {
	const char *code;
	const char *code2;
	const char *abbrev;
	const char *description;
	Common::Platform id;
};

extern const PlatformDescription g_platforms[];

/** Convert a string containing a platform name into a Platform enum value. */
extern Platform parsePlatform(const String &str);
extern const char *getPlatformCode(Platform id);
extern const char *getPlatformAbbrev(Platform id);
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

enum HerculesDimensions {
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
 * @param description the description which shows up in the debugger
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

typedef List<EngineDebugLevel> DebugLevelContainer;

/**
 * Lists all debug levels
 * @return returns a arry with all debug levels
 */
const DebugLevelContainer &listSpecialDebugLevels();

/**
 * Return the active debug flag mask (i.e. all active debug flags ORed
 * together into a single uint32).
 */
uint32 getEnabledSpecialDebugLevels();


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

extern int gDebugLevel;

char *scumm_strrev(char *str);

Common::String tag2string(uint32 tag);
#define tag2str(x)	tag2string(x).c_str()


#endif
