/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/gui_options.h"

#include "common/config-manager.h"
#include "common/str.h"

namespace Common {

const struct GameOpt {
	const char *option;
	// Each description must be a unique identifier not containing a substring
	// of any other description
	const char *desc;
} g_gameOptions[] = {
	{ GUIO_NOSUBTITLES,  "sndNoSubs" },
	{ GUIO_NOMUSIC,      "sndNoMusic" },
	{ GUIO_NOSPEECH,     "sndNoSpeech" },
	{ GUIO_NOSFX,        "sndNoSFX" },
	{ GUIO_NOMIDI,       "sndNoMIDI" },
	{ GUIO_LINKSPEECHTOSFX, "sndLinkSpeechToSfx" },
	{ GUIO_LINKMUSICTOSFX,  "sndLinkMusicToSfx" },
	{ GUIO_NOSPEECHVOLUME,  "sndNoSpchVolume" },

	{ GUIO_NOLANG,        "noLang"},

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
	{ GUIO_MIDISEGACD,   "midiSegaCD" },
	{ GUIO_MIDIMT32,     "midiMt32" },
	{ GUIO_MIDIGM,       "midiGM" },
	{ GUIO_MIDIMAC,      "midiMac" },

	{ GUIO_NOASPECT,     "noAspect" },

	{ GUIO_RENDERHERCGREEN,		"hercGreen" },
	{ GUIO_RENDERHERCAMBER,		"hercAmber" },
	{ GUIO_RENDERCGA,			"cga" },
	{ GUIO_RENDERCGACOMP,		"cgaComp" },
	{ GUIO_RENDERCGABW,			"cgaBW" },
	{ GUIO_RENDEREGA,			"ega" },
	{ GUIO_RENDERVGA,			"vga" },
	{ GUIO_RENDERAMIGA,			"amiga" },
	{ GUIO_RENDERFMTOWNS,		"fmtowns" },
	{ GUIO_RENDERPC98_256C,		"pc98-256c" },
	{ GUIO_RENDERPC98_16C,		"pc98-16c" },
	{ GUIO_RENDERAPPLE2GS,		"2gs" },
	{ GUIO_RENDERATARIST,		"atari" },
	{ GUIO_RENDERMACINTOSH,		"macintosh" },
	{ GUIO_RENDERMACINTOSHBW,	"macintoshbw" },
	{ GUIO_RENDERCPC,			"cpc" },
	{ GUIO_RENDERZX,			"zx" },
	{ GUIO_RENDERC64,			"c64" },
	{ GUIO_RENDERVGAGREY,		"vgaGray" },
	{ GUIO_RENDERPC98_8C,		"pc98-8c" },
	{ GUIO_RENDERWIN_256C,		"win256c" },
	{ GUIO_RENDERWIN_16C,		"win16c" },

	{ GUIO_GAMEOPTIONS1, "gameOption1" },
	{ GUIO_GAMEOPTIONS2, "gameOption2" },
	{ GUIO_GAMEOPTIONS3, "gameOption3" },
	{ GUIO_GAMEOPTIONS4, "gameOption4" },
	{ GUIO_GAMEOPTIONS5, "gameOption5" },
	{ GUIO_GAMEOPTIONS6, "gameOption6" },
	{ GUIO_GAMEOPTIONS7, "gameOption7" },
	{ GUIO_GAMEOPTIONS8, "gameOption8" },
	{ GUIO_GAMEOPTIONS9, "gameOption9" },
	// Option strings must not contain substrings of any other options, so
	// "gameOption10" would be invalid here because it contains "gameOption1"
	{ GUIO_GAMEOPTIONS10, "gameOptionA" },
	{ GUIO_GAMEOPTIONS11, "gameOptionB" },
	{ GUIO_GAMEOPTIONS12, "gameOptionC" },
	{ GUIO_GAMEOPTIONS13, "gameOptionD" },
	{ GUIO_GAMEOPTIONS14, "gameOptionE" },
	{ GUIO_GAMEOPTIONS15, "gameOptionF" },
	{ GUIO_GAMEOPTIONS16, "gameOptionG" },
	{ GUIO_GAMEOPTIONS17, "gameOptionH" },
	{ GUIO_GAMEOPTIONS18, "gameOptionI" },
	{ GUIO_GAMEOPTIONS19, "gameOptionJ" },
	{ GUIO_GAMEOPTIONS20, "gameOptionK" },
	{ GUIO_GAMEOPTIONS21, "gameOptionL" },
	{ GUIO_GAMEOPTIONS22, "gameOptionM" },
	{ GUIO_GAMEOPTIONS23, "gameOptionN" },
	{ GUIO_GAMEOPTIONS24, "gameOptionO" },
	{ GUIO_GAMEOPTIONS25, "gameOptionP" },
	{ GUIO_GAMEOPTIONS26, "gameOptionQ" },
	{ GUIO_GAMEOPTIONS27, "gameOptionR" },
	{ GUIO_GAMEOPTIONS28, "gameOptionS" },
	{ GUIO_GAMEOPTIONS29, "gameOptionT" },
	{ GUIO_GAMEOPTIONS30, "gameOptionU" },
	{ GUIO_GAMEOPTIONS31, "gameOptionV" },
	{ GUIO_GAMEOPTIONS32, "gameOptionW" },

	{ GUIO_NONE, nullptr }
};

bool checkGameGUIOption(const String &option, const String &str) {
#ifndef RELEASE_BUILD
	static bool firstRun = true;

	// Check GUIO constants for duplicates
	if (firstRun) {
		HashMap<String, bool, CaseSensitiveString_Hash, CaseSensitiveString_EqualTo> allOptions;

		for (int i = 0; g_gameOptions[i].desc; i++) {
			if (allOptions.contains(g_gameOptions[i].option)) {
				error("Duplicate GUIO constant, id: \\x%02x, renumerate them", g_gameOptions[i].option[0]);
			}

			allOptions[g_gameOptions[i].option] = true;
		}

		firstRun = false;
	}
#endif

	for (int i = 0; g_gameOptions[i].desc; i++) {
		if (option.contains(g_gameOptions[i].option)) {
			if (str.contains(g_gameOptions[i].desc))
				return true;
			else
				return false;
		}
	}
	return false;
}

String parseGameGUIOptions(const String &str) {
	String res;

	for (int i = 0; g_gameOptions[i].desc; i++) {
		for (uint32 ii = 0; ii < str.size(); ++ii) {
			uint32 c_end = str.find(' ', ii);
			if (c_end == (uint32)-1)
				c_end = str.size();
			if (str.substr(ii, c_end - ii).equals((g_gameOptions[i].desc)))
				res += g_gameOptions[i].option;
			ii = c_end;
		}
	}

	return res;
}

const String getGameGUIOptionsDescription(const String &options) {
	String res;

	for (int i = 0; g_gameOptions[i].desc; i++)
		if (options.contains(g_gameOptions[i].option[0]))
			res += String(g_gameOptions[i].desc) + " ";

	res.trim();

	return res;
}

void updateGameGUIOptions(const String &options, const String &langOption) {
	const String newOptionString = getGameGUIOptionsDescription(options) + " " + langOption;
	ConfMan.setAndFlush("guioptions", newOptionString);
}


} // End of namespace Common
