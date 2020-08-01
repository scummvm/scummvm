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

	{ GUIO_NOASPECT,     "noAspect" },

	{ GUIO_RENDERHERCGREEN, "hercGreen" },
	{ GUIO_RENDERHERCAMBER, "hercAmber" },
	{ GUIO_RENDERCGA,       "cga" },
	{ GUIO_RENDEREGA,       "ega" },
	{ GUIO_RENDERVGA,       "vga" },
	{ GUIO_RENDERAMIGA,     "amiga" },
	{ GUIO_RENDERFMTOWNS,   "fmtowns" },
	{ GUIO_RENDERPC9821,    "pc9821" },
	{ GUIO_RENDERPC9801,    "pc9801" },
	{ GUIO_RENDERAPPLE2GS,  "2gs" },
	{ GUIO_RENDERATARIST,   "atari" },
	{ GUIO_RENDERMACINTOSH, "macintosh" },

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

	{ GUIO_NONE, nullptr }
};

bool checkGameGUIOption(const String &option, const String &str) {
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

	for (int i = 0; g_gameOptions[i].desc; i++)
		if (str.contains(g_gameOptions[i].desc))
			res += g_gameOptions[i].option;

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
