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
 */

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include "common/scummsys.h"
#include "common/str.h"


/**
 * Check whether a given pointer is aligned correctly.
 * Note that 'alignment' must be a power of two!
 */
#define IS_ALIGNED(value, alignment) \
          ((((size_t)value) & ((alignment) - 1)) == 0)


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

/**
 * Macro which determines the number of entries in a fixed size array.
 */
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))


/**
 * @def SCUMMVM_CURRENT_FUNCTION
 * This macro evaluates to the current function's name on compilers supporting this.
 */
#if defined(__GNUC__)
# define SCUMMVM_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#  define SCUMMVM_CURRENT_FUNCTION	__func__
#elif defined(_MSC_VER) && _MSC_VER >= 1300
#  define SCUMMVM_CURRENT_FUNCTION __FUNCTION__
#else
#  define SCUMMVM_CURRENT_FUNCTION "<unknown>"
#endif

namespace Common {

/**
 * Print a hexdump of the data passed in. The number of bytes per line is
 * customizable.
 * @param data	the data to be dumped
 * @param len	the lenght of that data
 * @param bytesPerLine	number of bytes to print per line (default: 16)
 * @param startOffset	shift the shown offsets by the starting offset (default: 0)
 */
extern void hexdump(const byte * data, int len, int bytesPerLine = 16, int startOffset = 0);


/**
 * Parse a string for a boolean value.
 * The strings "true", "yes", and "1" are interpreted as true.
 * The strings "false", "no", and "0" are interpreted as false.
 * This function ignores case.
 *
 * @param[in] val			the string to parse
 * @param[out] valAsBool	the parsing result
 * @return 	true if the string parsed correctly, false if an error occurred.
 */
bool parseBool(const Common::String &val, bool &valAsBool);

/**
 * List of game language.
 */
enum Language {
	ZH_CNA,
	ZH_TWN,
	CZ_CZE,
	NL_NLD,
	EN_ANY,     // Generic English (when only one game version exist)
	EN_GRB,
	EN_USA,
	FR_FRA,
	DE_DEU,
	GR_GRE,
	HE_ISR,
	HU_HUN,
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
	//const char *unixLocale;
	const char *description;
	Common::Language id;
};

extern const LanguageDescription g_languages[];


/** Convert a string containing a language name into a Language enum value. */
extern Language parseLanguage(const String &str);
extern const char *getLanguageCode(Language id);
extern const char *getLanguageDescription(Language id);

// locale <-> Language conversion is disabled, since it is not used currently
/*extern const char *getLanguageLocale(Language id);
extern Language parseLanguageFromLocale(const char *locale);*/

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
	kPlatformCoCo3,
	kPlatformLinux,
	kPlatformAcorn,
	kPlatformSegaCD,
	kPlatform3DO,
	kPlatformPCEngine,
	kPlatformApple2GS,
	kPlatformPC98,
	kPlatformWii,
	kPlatformPSX,
	kPlatformCDi,
	kPlatformIOS,

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

enum GameGUIOption {
	GUIO_NONE		= 0,
	GUIO_NOSUBTITLES	= (1 << 0),
	GUIO_NOMUSIC		= (1 << 1),
	GUIO_NOSPEECH		= (1 << 2),
	GUIO_NOSFX			= (1 << 3),
	GUIO_NOMIDI			= (1 << 4),
	GUIO_NOLAUNCHLOAD	= (1 << 5),

	GUIO_MIDIPCSPK		= (1 << 6),
	GUIO_MIDICMS		= (1 << 7),
	GUIO_MIDIPCJR		= (1 << 8),
	GUIO_MIDIADLIB		= (1 << 9),
	GUIO_MIDIC64        = (1 << 10),
	GUIO_MIDIAMIGA      = (1 << 11),
	GUIO_MIDIAPPLEIIGS  = (1 << 12),
	GUIO_MIDITOWNS		= (1 << 13),
	GUIO_MIDIPC98		= (1 << 14),
	GUIO_MIDIMT32		= (1 << 15),
	GUIO_MIDIGM			= (1 << 16)
};

bool checkGameGUIOption(GameGUIOption option, const String &str);
bool checkGameGUIOptionLanguage(Language lang, const String &str);
uint32 parseGameGUIOptions(const String &str);
const String getGameGUIOptionsDescription(uint32 options);
const String getGameGUIOptionsDescriptionLanguage(Language lang);

/**
 * Updates the GUI options of the current config manager
 * domain, when they differ to the ones passed as
 * parameter.
 */
void updateGameGUIOptions(const uint32 options, const String &langOption);

}	// End of namespace Common

#endif
