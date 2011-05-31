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

#include "common/util.h"
#include "common/translation.h"
#include "common/config-manager.h"
#include "common/debug.h"

namespace Common {

//
// Print hexdump of the data passed in
//
void hexdump(const byte *data, int len, int bytesPerLine, int startOffset) {
	assert(1 <= bytesPerLine && bytesPerLine <= 32);
	int i;
	byte c;
	int offset = startOffset;
	while (len >= bytesPerLine) {
		debugN("%06x: ", offset);
		for (i = 0; i < bytesPerLine; i++) {
			debugN("%02x ", data[i]);
			if (i % 4 == 3)
				debugN(" ");
		}
		debugN(" |");
		for (i = 0; i < bytesPerLine; i++) {
			c = data[i];
			if (c < 32 || c >= 127)
				c = '.';
			debugN("%c", c);
		}
		debugN("|\n");
		data += bytesPerLine;
		len -= bytesPerLine;
		offset += bytesPerLine;
	}

	if (len <= 0)
		return;

	debugN("%06x: ", offset);
	for (i = 0; i < bytesPerLine; i++) {
		if (i < len)
			debugN("%02x ", data[i]);
		else
			debugN("   ");
		if (i % 4 == 3)
			debugN(" ");
	}
	debugN(" |");
	for (i = 0; i < len; i++) {
		c = data[i];
		if (c < 32 || c >= 127)
			c = '.';
		debugN("%c", c);
	}
	for (; i < bytesPerLine; i++)
		debugN(" ");
	debugN("|\n");
}


#pragma mark -


bool parseBool(const Common::String &val, bool &valAsBool) {
	if (val.equalsIgnoreCase("true") ||
		val.equalsIgnoreCase("yes") ||
		val.equals("1")) {
		valAsBool = true;
		return true;
	}
	if (val.equalsIgnoreCase("false") ||
		val.equalsIgnoreCase("no") ||
		val.equals("0")) {
		valAsBool = false;
		return true;
	}

	return false;
}


#pragma mark -


const LanguageDescription g_languages[] = {
	{ "zh-cn"/*, "zh_CN"*/, "Chinese (China)", ZH_CNA },
	{    "zh"/*, "zh_TW"*/, "Chinese (Taiwan)", ZH_TWN },
	{    "cz"/*, "cs_CZ"*/, "Czech", CZ_CZE },
	{    "nl"/*, "nl_NL"*/, "Dutch", NL_NLD },
	{    "en"/*,    "en"*/, "English", EN_ANY }, // Generic English (when only one game version exist)
	{    "gb"/*, "en_GB"*/, "English (GB)", EN_GRB },
	{    "us"/*, "en_US"*/, "English (US)", EN_USA },
	{    "fr"/*, "fr_FR"*/, "French", FR_FRA },
	{    "de"/*, "de_DE"*/, "German", DE_DEU },
	{    "gr"/*, "el_GR"*/, "Greek", GR_GRE },
	{    "he"/*, "he_IL"*/, "Hebrew", HE_ISR },
	{    "hb"/*, "he_IL"*/, "Hebrew", HE_ISR }, // Deprecated
	{    "hu"/*, "hu_HU"*/, "Hungarian", HU_HUN },
	{    "it"/*, "it_IT"*/, "Italian", IT_ITA },
	{    "jp"/*, "ja_JP"*/, "Japanese", JA_JPN },
	{    "kr"/*, "ko_KR"*/, "Korean", KO_KOR },
	{    "nb"/*, "nb_NO"*/, "Norwegian Bokm\xE5l", NB_NOR }, // TODO Someone should verify the unix locale
	{    "pl"/*, "pl_PL"*/, "Polish", PL_POL },
	{    "br"/*, "pt_BR"*/, "Portuguese", PT_BRA },
	{    "ru"/*, "ru_RU"*/, "Russian", RU_RUS },
	{    "es"/*, "es_ES"*/, "Spanish", ES_ESP },
	{    "se"/*, "sv_SE"*/, "Swedish", SE_SWE },
	{       0/*,       0*/, 0, UNK_LANG }
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

/*Language parseLanguageFromLocale(const char *locale) {
	if (!locale || !*locale)
		return UNK_LANG;

	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (!strcmp(l->unixLocale, locale))
			return l->id;
	}

	return UNK_LANG;
}*/

const char *getLanguageCode(Language id) {
	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->code;
	}
	return 0;
}

/*const char *getLanguageLocale(Language id) {
	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->unixLocale;
	}
	return 0;
}*/

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
	{ "2gs", "2gs", "2gs", "Apple IIgs", kPlatformApple2GS },
	{ "3do", "3do", "3do", "3DO", kPlatform3DO },
	{ "acorn", "acorn", "acorn", "Acorn", kPlatformAcorn },
	{ "amiga", "ami", "amiga", "Amiga", kPlatformAmiga },
	{ "atari", "atari-st", "st", "Atari ST", kPlatformAtariST },
	{ "c64", "c64", "c64", "Commodore 64", kPlatformC64 },
	{ "pc", "dos", "ibm", "DOS", kPlatformPC },
	{ "pc98", "pc98", "pc98", "PC-98", kPlatformPC98 },
	{ "wii", "wii", "wii", "Nintendo Wii", kPlatformWii },
	{ "coco3", "coco3", "coco3", "CoCo3", kPlatformCoCo3 },

	// The 'official' spelling seems to be "FM-TOWNS" (e.g. in the Indy4 demo).
	// However, on the net many variations can be seen, like "FMTOWNS",
	// "FM TOWNS", "FmTowns", etc.
	{ "fmtowns", "towns", "fm", "FM-TOWNS", kPlatformFMTowns },

	{ "linux", "linux", "linux", "Linux", kPlatformLinux },
	{ "macintosh", "mac", "mac", "Macintosh", kPlatformMacintosh },
	{ "pce", "pce", "pce", "PC-Engine", kPlatformPCEngine },
	{ "nes", "nes", "nes", "NES", kPlatformNES },
	{ "segacd", "segacd", "sega", "SegaCD", kPlatformSegaCD },
	{ "windows", "win", "win", "Windows", kPlatformWindows },
	{ "playstation", "psx", "psx", "Sony PlayStation", kPlatformPSX },
	{ "cdi", "cdi", "cdi", "Philips CD-i", kPlatformCDi },
	{ "ios", "ios", "ios", "Apple iOS", kPlatformIOS },

	{ 0, 0, 0, "Default", kPlatformUnknown }
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
	{ "hercGreen", _s("Hercules Green"), kRenderHercG },
	{ "hercAmber", _s("Hercules Amber"), kRenderHercA },
	{ "cga", "CGA", kRenderCGA },
	{ "ega", "EGA", kRenderEGA },
	{ "amiga", "Amiga", kRenderAmiga },
	{0, 0, kRenderDefault}
};

DECLARE_TRANSLATION_ADDITIONAL_CONTEXT("Hercules Green", "lowres")
DECLARE_TRANSLATION_ADDITIONAL_CONTEXT("Hercules Amber", "lowres")

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
	{ GUIO_NOSUBTITLES,  "sndNoSubs" },
	{ GUIO_NOMUSIC,      "sndNoMusic" },
	{ GUIO_NOSPEECH,     "sndNoSpeech" },
	{ GUIO_NOSFX,        "sndNoSFX" },
	{ GUIO_NOMIDI,       "sndNoMIDI" },

	{ GUIO_NOLAUNCHLOAD, "launchNoLoad" },

	{ GUIO_MIDIPCSPK,    "midiPCSpk" },
	{ GUIO_MIDICMS,      "midiCMS" },
	{ GUIO_MIDIPCJR,     "midiPCJr" },
	{ GUIO_MIDIADLIB,    "midiAdLib" },
	{ GUIO_MIDIC64,      "midiC64" },
	{ GUIO_MIDIAMIGA,    "midiAmiga" },
	{ GUIO_MIDIAPPLEIIGS,"midiAppleIIgs" },
	{ GUIO_MIDITOWNS,    "midiTowns" },
	{ GUIO_MIDIPC98,     "midiPC98" },
	{ GUIO_MIDIMT32,     "midiMt32" },
	{ GUIO_MIDIGM,       "midiGM" },

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

bool checkGameGUIOptionLanguage(Language lang, const String &str) {
	if (!str.contains("lang_")) // If no languages are specified
		return true;

	if (str.contains(getGameGUIOptionsDescriptionLanguage(lang)))
		return true;

	return false;
}

const String getGameGUIOptionsDescriptionLanguage(Language lang) {
	if (lang == UNK_LANG)
		return "";

	return String(String("lang_") + getLanguageDescription(lang));
}

uint32 parseGameGUIOptions(const String &str) {
	uint32 res = 0;

	for (int i = 0; g_gameOptions[i].desc; i++)
		if (str.contains(g_gameOptions[i].desc))
			res |= g_gameOptions[i].option;

	return res;
}

const String getGameGUIOptionsDescription(uint32 options) {
	String res = "";

	for (int i = 0; g_gameOptions[i].desc; i++)
		if (options & g_gameOptions[i].option)
			res += String(g_gameOptions[i].desc) + " ";

	res.trim();

	return res;
}

void updateGameGUIOptions(const uint32 options, const String &langOption) {
	const String newOptionString = getGameGUIOptionsDescription(options) + " " + langOption;

	if ((options && !ConfMan.hasKey("guioptions")) ||
	    (ConfMan.hasKey("guioptions") && ConfMan.get("guioptions") != newOptionString)) {
		ConfMan.set("guioptions", newOptionString);
		ConfMan.flushToDisk();
	}
}

} // End of namespace Common

