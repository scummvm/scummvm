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

#include "common/platform.h"

// This is an equivalent of an enum. Feel free to renumerate them
// They are used only internally for making lookups cheaper and for
// possibility to concatenate them as codes to the detection tables
// See the GUIOnn() macros below
// This is encoded like UTF-8 does with either 1 byte or 2 bytes codepoints
// 3 and 4 bytes one may be added later

// All simple options are expected to lie in the 0x01-0x7f range
// The prefixed values lie in the 0x80-0xbf range
// The options using a 2nd byte suffix lie in the 0xc0-0xdf range
// The range 0xe0-0xff is reserved for future use

#define GUIO_NONE            "\x00"
#define GUIO_NOSUBTITLES     "\x01"
#define GUIO_NOMUSIC         "\x02"
// GUIO_NOSPEECH is a combination of GUIO_NOSPEECHVOLUME and GUIO_NOSUBTITLES
#define GUIO_NOSPEECH        "\x03"
#define GUIO_NOSFX           "\x04"
#define GUIO_NOMIDI          "\x05"
#define GUIO_NOLAUNCHLOAD    "\x06"

#define GUIO_NOASPECT        "\x07"

#define GUIO_LINKSPEECHTOSFX "\x08"
#define GUIO_LINKMUSICTOSFX  "\x09"
#define GUIO_NOSPEECHVOLUME  "\x0a"

#define GUIO_NOLANG          "\x0b"

// GUIO flags in range "\x80" - "\xbf" are reserved for prefixed lists of options

// Helper macros to get string for the platform
// Extra level of indirection required to force macro expansion on some compilers
#define GET_PLAT_STR_IMPL(val, hex) hex
#define GET_PLAT_STR_EXPAND(x) GET_PLAT_STR_IMPL x
#define GET_PLAT_STR(name) GET_PLAT_STR_EXPAND((name))

// Here starts 2 bytes GUIO flags
// They are made of a category specific prefix in the range 0xc0-0xdf followed by a byte between 0x80-0xbf

#define GUIO_PLATFORM_PREFIX "\xc0"

// Get hex string literal for the given platform
#define GUIO_PLATFORM(p) GUIO_PLATFORM_PREFIX GET_PLAT_STR(p##_VAL)

#define GUIO_RENDER_PREFIX       "\xc1"
#define GUIO_RENDERHERCGREEN     "\xc1\x80"
#define GUIO_RENDERHERCAMBER     "\xc1\x81"
#define GUIO_RENDERCGA           "\xc1\x82"
#define GUIO_RENDEREGA           "\xc1\x83"
#define GUIO_RENDERVGA           "\xc1\x84"
#define GUIO_RENDERAMIGA         "\xc1\x85"
#define GUIO_RENDERFMTOWNS       "\xc1\x86"
#define GUIO_RENDERPC98_256C     "\xc1\x87"
#define GUIO_RENDERPC98_16C      "\xc1\x88"
#define GUIO_RENDERAPPLE2GS      "\xc1\x89"
#define GUIO_RENDERATARIST       "\xc1\x8a"
#define GUIO_RENDERMACINTOSH     "\xc1\x8b"
#define GUIO_RENDERMACINTOSHBW   "\xc1\x8c"
#define GUIO_RENDERCGACOMP       "\xc1\x8d"
#define GUIO_RENDERCGABW         "\xc1\x8e"
#define GUIO_RENDERCPC           "\xc1\x8f"
#define GUIO_RENDERZX            "\xc1\x90"
#define GUIO_RENDERC64           "\xc1\x91"
#define GUIO_RENDERVGAGREY       "\xc1\x92"
#define GUIO_RENDERPC98_8C       "\xc1\x93"
#define GUIO_RENDERWIN_256C      "\xc1\x94"
#define GUIO_RENDERWIN_16C       "\xc1\x95"

#define GUIO_MIDI_PREFIX     "\xc2"
#define GUIO_MIDIPCSPK       "\xc2\x80"
#define GUIO_MIDICMS         "\xc2\x81"
#define GUIO_MIDIPCJR        "\xc2\x82"
#define GUIO_MIDIADLIB       "\xc2\x83"
#define GUIO_MIDIC64         "\xc2\x84"
#define GUIO_MIDIAMIGA       "\xc2\x85"
#define GUIO_MIDIAPPLEIIGS   "\xc2\x86"
#define GUIO_MIDITOWNS       "\xc2\x87"
#define GUIO_MIDIPC98        "\xc2\x88"
#define GUIO_MIDISEGACD      "\xc2\x89"
#define GUIO_MIDIMT32        "\xc2\x8a"
#define GUIO_MIDIGM          "\xc2\x8b"
#define GUIO_MIDIMAC         "\xc2\x8c"

// Special GUIO flags for the AdvancedDetector's caching of game specific
// options.
// Putting them to the end of the range so less renumerations required
#define GUIO_GAME_PREFIX     "\xdf"
#define GUIO_GAMEOPTIONS1    "\xdf\x80"
#define GUIO_GAMEOPTIONS2    "\xdf\x81"
#define GUIO_GAMEOPTIONS3    "\xdf\x82"
#define GUIO_GAMEOPTIONS4    "\xdf\x83"
#define GUIO_GAMEOPTIONS5    "\xdf\x84"
#define GUIO_GAMEOPTIONS6    "\xdf\x85"
#define GUIO_GAMEOPTIONS7    "\xdf\x86"
#define GUIO_GAMEOPTIONS8    "\xdf\x87"
#define GUIO_GAMEOPTIONS9    "\xdf\x88"
#define GUIO_GAMEOPTIONS10   "\xdf\x89"
#define GUIO_GAMEOPTIONS11   "\xdf\x8a"
#define GUIO_GAMEOPTIONS12   "\xdf\x8b"
#define GUIO_GAMEOPTIONS13   "\xdf\x8c"
#define GUIO_GAMEOPTIONS14   "\xdf\x8d"
#define GUIO_GAMEOPTIONS15   "\xdf\x8e"
#define GUIO_GAMEOPTIONS16   "\xdf\x8f"
#define GUIO_GAMEOPTIONS17   "\xdf\x90"
#define GUIO_GAMEOPTIONS18   "\xdf\x91"
#define GUIO_GAMEOPTIONS19   "\xdf\x92"
#define GUIO_GAMEOPTIONS20   "\xdf\x93"
#define GUIO_GAMEOPTIONS21   "\xdf\x94"
#define GUIO_GAMEOPTIONS22   "\xdf\x95"
#define GUIO_GAMEOPTIONS23   "\xdf\x96"
#define GUIO_GAMEOPTIONS24   "\xdf\x97"
#define GUIO_GAMEOPTIONS25   "\xdf\x98"
#define GUIO_GAMEOPTIONS26   "\xdf\x99"
#define GUIO_GAMEOPTIONS27   "\xdf\x9a"
#define GUIO_GAMEOPTIONS28   "\xdf\x9b"
#define GUIO_GAMEOPTIONS29   "\xdf\x9c"
#define GUIO_GAMEOPTIONS30   "\xdf\x9d"
#define GUIO_GAMEOPTIONS31   "\xdf\x9e"
#define GUIO_GAMEOPTIONS32   "\xdf\x9f"

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

/**
* Check if given option exists in a string
*/
bool checkGameGUIOption(const String &option, const String &str);

/**
* Parse GUIOptions string to GUIO literals defined in this file
*/
String parseGameGUIOptions(const String &str);

/**
* Return string containing gui options description based on GUIO literals
*/
const String getGameGUIOptionsDescription(const String &options);

/**
 * Update the GUI options of the current config manager
 * domain when they differ to the ones passed as
 * parameter.
 */
void updateGameGUIOptions(const String &options, const String &langOption, const String &platOption);

/** @} */

} // End of namespace Common

#endif
