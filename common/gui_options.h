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

#ifndef COMMON_GUI_OPTIONS_H
#define COMMON_GUI_OPTIONS_H

#define GUIO_NONE            "\x00"
#define GUIO_NOSUBTITLES     "\x01"
#define GUIO_NOMUSIC         "\x02"
// GUIO_NOSPEECH is a combination of GUIO_NOSPEECHVOLUME and GUIO_NOSUBTITLES
#define GUIO_NOSPEECH        "\x03"
#define GUIO_NOSFX           "\x04"
#define GUIO_NOMIDI          "\x05"
#define GUIO_NOLAUNCHLOAD    "\x06"

#define GUIO_MIDIPCSPK       "\x07"
#define GUIO_MIDICMS         "\x08"
#define GUIO_MIDIPCJR        "\x09"
#define GUIO_MIDIADLIB       "\x0a"
#define GUIO_MIDIC64         "\x0b"
#define GUIO_MIDIAMIGA       "\x0c"
#define GUIO_MIDIAPPLEIIGS   "\x0d"
#define GUIO_MIDITOWNS       "\x0e"
#define GUIO_MIDIPC98        "\x0f"
#define GUIO_MIDISEGACD      "\x10"
#define GUIO_MIDIMT32        "\x11"
#define GUIO_MIDIGM          "\x12"

#define GUIO_NOASPECT        "\x13"

#define GUIO_RENDERHERCGREEN	"\x18"
#define GUIO_RENDERHERCAMBER	"\x19"
#define GUIO_RENDERCGA			"\x1a"
#define GUIO_RENDEREGA			"\x1b"
#define GUIO_RENDERVGA			"\x1c"
#define GUIO_RENDERAMIGA		"\x1d"
#define GUIO_RENDERFMTOWNS		"\x1e"
#define GUIO_RENDERPC9821		"\x1f"
#define GUIO_RENDERPC9801		"\x20"
#define GUIO_RENDERAPPLE2GS		"\x21"
#define GUIO_RENDERATARIST		"\x22"
#define GUIO_RENDERMACINTOSH	"\x23"
#define GUIO_RENDERMACINTOSHBW	"\x28"	// Setting this to 0x28 is not ideal, but there is no free slot left. Maybe we need to migrate to 3-digit numbers...
#define GUIO_RENDERCGACOMP		"\x29"
#define GUIO_RENDERCGABW		"\x2a"

#define GUIO_LINKSPEECHTOSFX "\x24"
#define GUIO_LINKMUSICTOSFX  "\x25"
#define GUIO_NOSPEECHVOLUME  "\x26"

#define GUIO_NOLANG          "\x27"

// Special GUIO flags for the AdvancedDetector's caching of game specific
// options.
#define GUIO_GAMEOPTIONS1    "\x30"
#define GUIO_GAMEOPTIONS2    "\x31"
#define GUIO_GAMEOPTIONS3    "\x32"
#define GUIO_GAMEOPTIONS4    "\x33"
#define GUIO_GAMEOPTIONS5    "\x34"
#define GUIO_GAMEOPTIONS6    "\x35"
#define GUIO_GAMEOPTIONS7    "\x36"
#define GUIO_GAMEOPTIONS8    "\x37"
#define GUIO_GAMEOPTIONS9    "\x38"
#define GUIO_GAMEOPTIONS10   "\x39"
#define GUIO_GAMEOPTIONS11   "\x3a"
#define GUIO_GAMEOPTIONS12   "\x3b"
#define GUIO_GAMEOPTIONS13   "\x3c"
#define GUIO_GAMEOPTIONS14   "\x3d"
#define GUIO_GAMEOPTIONS15   "\x3e"
#define GUIO_GAMEOPTIONS16   "\x3f"
#define GUIO_GAMEOPTIONS17   "\x40"
#define GUIO_GAMEOPTIONS18   "\x41"
#define GUIO_GAMEOPTIONS19   "\x42"
#define GUIO_GAMEOPTIONS20   "\x43"
#define GUIO_GAMEOPTIONS21   "\x44"
#define GUIO_GAMEOPTIONS22   "\x45"
#define GUIO_GAMEOPTIONS23   "\x46"
#define GUIO_GAMEOPTIONS24   "\x47"
#define GUIO_GAMEOPTIONS25   "\x48"
#define GUIO_GAMEOPTIONS26   "\x49"
#define GUIO_GAMEOPTIONS27   "\x4a"
#define GUIO_GAMEOPTIONS28   "\x4b"
#define GUIO_GAMEOPTIONS29   "\x4c"
#define GUIO_GAMEOPTIONS30   "\x4d"
#define GUIO_GAMEOPTIONS31   "\x4e"
#define GUIO_GAMEOPTIONS32   "\x4f"

#define GUIO0() (GUIO_NONE)
#define GUIO1(a) (a)
#define GUIO2(a,b) (a b)
#define GUIO3(a,b,c) (a b c)
#define GUIO4(a,b,c,d) (a b c d)
#define GUIO5(a,b,c,d,e) (a b c d e)
#define GUIO6(a,b,c,d,e,f) (a b c d e f)
#define GUIO7(a,b,c,d,e,f,g) (a b c d e f g)
#define GUIO8(a,b,c,d,e,f,g,h) (a b c d e f g h)
#define GUIO9(a,b,c,d,e,f,g,h,i) (a b c d e f g h i)
#define GUIO10(a,b,c,d,e,f,g,h,i,j) (a b c d e f g h i j)
#define GUIO11(a,b,c,d,e,f,g,h,i,j,k) (a b c d e f g h i j k)
#define GUIO12(a,b,c,d,e,f,g,h,i,j,k,l) (a b c d e f g h i j k l)

namespace Common {

/**
 * @defgroup common_gui_options GUI options
 * @ingroup common
 *
 * @brief API for managing the options of the graphical user interface (GUI).
 *
 * @{
 */

class String;

bool checkGameGUIOption(const String &option, const String &str);
String parseGameGUIOptions(const String &str);
const String getGameGUIOptionsDescription(const String &options);

/**
 * Update the GUI options of the current config manager
 * domain when they differ to the ones passed as
 * parameter.
 */
void updateGameGUIOptions(const String &options, const String &langOption);

/** @} */

} // End of namespace Common

#endif
