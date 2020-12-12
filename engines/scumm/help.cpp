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



#include "common/util.h"

#include "scumm/help.h"
#include "scumm/scumm.h"
#include "common/translation.h"

namespace Scumm {

int ScummHelp::numPages(byte gameId) {
	switch (gameId) {
	case GID_MANIAC:
	case GID_ZAK:
		return 4;
	case GID_INDY4:
		return 5;
	case GID_INDY3:
		return 6;
	case GID_LOOM:
	case GID_MONKEY_EGA:
	case GID_MONKEY_VGA:
	case GID_MONKEY:
	case GID_MONKEY2:
	case GID_TENTACLE:
	case GID_SAMNMAX:
	case GID_DIG:
	case GID_FT:
	case GID_CMI:
		return 3;
/*	TODO - I don't know the controls for these games
	case GID_PUTTDEMO:
	case GID_PUTTPUTT:
*/
	default:
		return 2;
	}
}

#define ADD_BIND(k,d) do { key[i] = k; dsc[i] = d; i++; } while (0)
#define ADD_TEXT(d) ADD_BIND(USTR(""),d)
#define ADD_LINE ADD_BIND(USTR(""),USTR(""))

void ScummHelp::updateStrings(byte gameId, byte version, Common::Platform platform,
				int page, U32String &title, U32String *&key, U32String *&dsc) {
	key = new U32String[HELP_NUM_LINES];
	dsc = new U32String[HELP_NUM_LINES];
	int i = 0;
	switch (page) {
	case 1:
		title = _("Common keyboard commands:");
		ADD_BIND(USTR("F5"), _("Save / Load dialog"));
		if (version >= 5)
			ADD_BIND(USTR("."), _("Skip line of text"));
		ADD_BIND(_("Esc"), _("Skip cutscene"));
		ADD_BIND(_("Space"), _("Pause game"));
		ADD_BIND(_("Ctrl") + USTR(" 0-9"), _("Load saved game 1-10"));
		ADD_BIND(_("Alt") + USTR(" 0-9"), _("Save game 1-10"));
#ifdef MACOSX
		ADD_BIND(USTR("Cmd q"), _("Quit"));
#else
		ADD_BIND(_("Alt") + USTR(" x, ") + _("Ctrl") + USTR(" z"), _("Quit"));
#endif
		ADD_BIND(_("Alt") + USTR(" ") + _("Enter"), _("Toggle fullscreen"));
		ADD_BIND(USTR("[, ]"), _("Music volume up / down"));
		ADD_BIND(USTR("-, +"), _("Text speed slower / faster"));
		ADD_BIND(_("Enter"), _("Simulate left mouse button"));
		ADD_BIND(_("Tab"), _("Simulate right mouse button"));
		break;
	case 2:
		title = _("Special keyboard commands:");
		ADD_BIND(USTR("~, #"), _("Show / Hide console"));
		ADD_BIND(_("Ctrl") + USTR(" d"), _("Start the debugger"));
		ADD_BIND(_("Ctrl") + USTR(" s"), _("Show memory consumption"));
		ADD_BIND(_("Ctrl") + USTR(" f"), _("Run in fast mode (*)"));
		ADD_BIND(_("Ctrl") + USTR(" g"), _("Run in really fast mode (*)"));
		ADD_BIND(_("Ctrl") + USTR(" m"), _("Toggle mouse capture"));
		ADD_BIND(_("Ctrl") + USTR(" ") + _("Alt") + USTR(" 1-8"), _("Switch between graphics filters"));
		ADD_BIND(_("Ctrl") + USTR(" ") + _("Alt") + USTR(" +, -"), _("Increase / Decrease scale factor"));
		ADD_BIND(_("Ctrl") + USTR(" ") + _("Alt") + USTR(" a"), _("Toggle aspect-ratio correction"));
		ADD_LINE;
		ADD_LINE;
		// FIXME: This should use word-wrapping, and should not assume
		//        that the font is mono-spaced.
		ADD_TEXT(_("* Note that using ctrl-f and"));
		ADD_TEXT(_("  ctrl-g are not recommended"));
		ADD_TEXT(_("  since they may cause crashes"));
		ADD_TEXT(_("  or incorrect game behavior."));
		break;
	case 3:
		if (gameId == GID_LOOM)
			title = _("Spinning drafts on the keyboard:");
		else
			title = _("Main game controls:");
		switch (gameId) {
		case GID_ZAK:
		case GID_MANIAC:
			if (platform == Common::kPlatformNES) {
				ADD_BIND(USTR("q"), _("Push"));
				ADD_BIND(USTR("a"), _("Pull"));
				ADD_BIND(USTR("z"), _("Give"));
				ADD_BIND(USTR("w"), _("Open"));
				ADD_BIND(USTR("s"), _("Close"));
				ADD_BIND(USTR("x"), _("Go to"));
				ADD_BIND(USTR("e"), _("Get"));
				ADD_BIND(USTR("d"), _("Use"));
				ADD_BIND(USTR("c"), _("Read"));
				ADD_BIND(USTR("r"), _("New kid"));
				ADD_BIND(USTR("f"), _("Turn on"));
				ADD_BIND(USTR("v"), _("Turn off"));
				break;
			}

			ADD_BIND(USTR("q"), _("Push"));
			ADD_BIND(USTR("a"), _("Pull"));
			ADD_BIND(USTR("z"), _("Give"));
			ADD_BIND(USTR("w"), _("Open"));
			ADD_BIND(USTR("s"), _("Close"));
			ADD_BIND(USTR("x"), _("Read"));
			ADD_BIND(USTR("e"), _("Walk to"));
			ADD_BIND(USTR("d"), _("Pick up"));
			ADD_BIND(USTR("c"), _("What is"));
			if (gameId == GID_MANIAC) {
				ADD_BIND(USTR("r"), _("Unlock"));
				ADD_BIND(USTR("f"), _("New kid"));
			} else {
				ADD_BIND(USTR("r"), _("Put on"));
				ADD_BIND(USTR("f"), _("Take off"));
			}
			ADD_BIND(USTR("v"), _("Use"));
			ADD_BIND(USTR("t"), _("Turn on"));
			ADD_BIND(USTR("g"), _("Turn off"));
			if (gameId == GID_MANIAC)
				ADD_BIND(USTR("b"), _("Fix"));
			else
				ADD_BIND(USTR("b"), _("Switch"));
			break;
		case GID_INDY3:
			ADD_BIND(USTR("q"), _("Push"));
			ADD_BIND(USTR("a"), _("Pull"));
			ADD_BIND(USTR("z"), _("Give"));
			ADD_BIND(USTR("w"), _("Open"));
			ADD_BIND(USTR("s"), _("Close"));
			ADD_BIND(USTR("x"), _("Look"));
			ADD_BIND(USTR("e"), _("Walk to"));
			ADD_BIND(USTR("d"), _("Pick up"));
			ADD_BIND(USTR("c"), _("What is"));
			ADD_BIND(USTR("r"), _("Use"));
			ADD_BIND(USTR("f"), _("Turn on"));
			ADD_BIND(USTR("v"), _("Turn off"));
			ADD_BIND(USTR("t"), _("Talk"));
			ADD_BIND(USTR("g"), _("Travel"));
			ADD_BIND(USTR("b"), _("To Henry / To Indy"));
			break;
		case GID_LOOM:
			// I18N: These are different musical notes
			ADD_BIND(USTR("q, c"), _("play C minor on distaff"));
			ADD_BIND(USTR("w, d"), _("play D on distaff"));
			ADD_BIND(USTR("e, e"), _("play E on distaff"));
			ADD_BIND(USTR("r, f"), _("play F on distaff"));
			ADD_BIND(USTR("t, g"), _("play G on distaff"));
			ADD_BIND(USTR("y, a"), _("play A on distaff"));
			ADD_BIND(USTR("u, b"), _("play B on distaff"));
			ADD_BIND(USTR("i, C"), _("play C major on distaff"));
			break;
		case GID_MONKEY_EGA:
		case GID_MONKEY_VGA:
			ADD_BIND(USTR("o"), _("Open"));
			ADD_BIND(USTR("c"), _("Close"));
			ADD_BIND(USTR("s"), _("puSh"));
			ADD_BIND(USTR("y"), _("pull (Yank)"));
			ADD_BIND(USTR("w"), _("Walk to"));
			ADD_BIND(USTR("p"), _("Pick up"));
			ADD_BIND(USTR("t"), _("Talk to"));
			ADD_BIND(USTR("g"), _("Give"));
			ADD_BIND(USTR("u"), _("Use"));
			ADD_BIND(USTR("l"), _("Look at"));
			ADD_BIND(USTR("n"), _("turn oN"));
			ADD_BIND(USTR("f"), _("turn oFf"));
			break;
		case GID_MONKEY:
		case GID_MONKEY2:
		case GID_INDY4:
		case GID_TENTACLE:
			ADD_BIND(USTR("g"), _("Give"));
			ADD_BIND(USTR("o"), _("Open"));
			ADD_BIND(USTR("c"), _("Close"));
			ADD_BIND(USTR("p"), _("Pick up"));
			ADD_BIND(USTR("l"), _("Look at"));
			ADD_BIND(USTR("t"), _("Talk to"));
			ADD_BIND(USTR("u"), _("Use"));
			ADD_BIND(USTR("s"), _("puSh"));
			ADD_BIND(USTR("y"), _("pull (Yank)"));
			if (platform == Common::kPlatformSegaCD) {
				ADD_BIND(_("KeyUp"), _("Highlight prev dialogue"));
				ADD_BIND(_("KeyDown"), _("Highlight next dialogue"));
			}
			break;
		case GID_SAMNMAX:
			ADD_BIND(USTR("w"), _("Walk"));
			ADD_BIND(USTR("t"), _("Talk"));
			ADD_BIND(USTR("u"), _("Use"));
			ADD_BIND(USTR("i"), _("Inventory"));
			ADD_BIND(USTR("o"), _("Object"));
			ADD_BIND(USTR("p"), _("Pick up"));
			ADD_BIND(USTR("l"), _("Look"));
			ADD_BIND(USTR("b"), _("Black and White / Color"));
			break;
		case GID_FT:
			ADD_BIND(USTR("e"), _("Eyes"));
			ADD_BIND(USTR("t"), _("Tongue"));
			ADD_BIND(USTR("i"), _("Inventory"));
			ADD_BIND(USTR("p"), _("Punch"));
			ADD_BIND(USTR("k"), _("Kick"));
			break;
		case GID_DIG:
			ADD_BIND(USTR("e"), _("Examine"));
			ADD_BIND(USTR("t"), _("Regular cursor"));
			ADD_BIND(USTR("i"), _("Inventory"));
			// I18N: Comm is a communication device
			ADD_BIND(USTR("c"), _("Comm"));
			break;
		case GID_CMI:
			ADD_BIND(USTR("F1"), _("Save / Load / Options"));
			ADD_BIND(USTR("e"), _("Examine"));
			ADD_BIND(USTR("t"), _("Talk to"));
			ADD_BIND(USTR("i"), _("Inventory"));
			ADD_BIND(USTR("u"), _("Use"));
			break;
		default:
			break;
		}
		break;
	case 4:
		title = _("Other game controls:");
		if (version <= 2) {
			ADD_TEXT(_("Inventory:"));
			ADD_BIND(USTR("u"), _("Scroll list up"));
			ADD_BIND(USTR("j"), _("Scroll list down"));
			ADD_BIND(USTR("i"), _("Upper left item"));
			ADD_BIND(USTR("k"), _("Lower left item"));
			ADD_BIND(USTR("o"), _("Upper right item"));
			ADD_BIND(USTR("l"), _("Lower right item"));
			ADD_LINE;
		} else if (gameId == GID_INDY3 || gameId == GID_ZAK) {
			// Indy3, or FM-TOWNS Zak
			ADD_TEXT(_("Inventory:"));
			ADD_BIND(USTR("y"), _("Upper left item"));
			ADD_BIND(USTR("h"), _("Middle left item"));
			ADD_BIND(USTR("n"), _("Lower left item"));
			ADD_BIND(USTR("u"), _("Upper right item"));
			ADD_BIND(USTR("j"), _("Middle right item"));
			ADD_BIND(USTR("m"), _("Lower right item"));
			ADD_BIND(USTR("o"), _("Scroll list up"));
			ADD_BIND(USTR("l"), _("Scroll list down"));
			ADD_LINE;
		}
		if (gameId == GID_MANIAC) {
			ADD_TEXT(_("Switching characters:"));
			ADD_BIND(USTR("F1"), USTR("Dave"));
			ADD_BIND(USTR("F2"), _("Second kid"));
			ADD_BIND(USTR("F3"), _("Third kid"));
		} else if (gameId == GID_ZAK) {
			ADD_TEXT(_("Switching characters:"));
			ADD_BIND(USTR("F1"), USTR("Zak"));
			ADD_BIND(USTR("F2"), USTR("Annie"));
			ADD_BIND(USTR("F3"), USTR("Melissa"));
			ADD_BIND(USTR("F4"), USTR("Leslie"));
		}
		if (gameId == GID_INDY4) {
			ADD_BIND(USTR("i"), _("Toggle Inventory/IQ Points display"));
			ADD_BIND(USTR("f"), _("Toggle Keyboard/Mouse Fighting (*)"));
			ADD_LINE;
			ADD_TEXT(_("* Keyboard Fighting is always on,"));
			ADD_TEXT(_("  so despite the in-game message this"));
			ADD_TEXT(_("  actually toggles Mouse Fighting Off/On"));
		}
		break;
	case 5:
		switch (gameId) {
		case GID_INDY3:
		case GID_INDY4:
			title = _("Fighting controls (numpad):");
			ADD_BIND(USTR("7"), _("Step back"));
			ADD_BIND(USTR("4"), _("Step back"));
			ADD_BIND(USTR("1"), _("Step back"));
			ADD_BIND(USTR("8"), _("Block high"));
			ADD_BIND(USTR("5"), _("Block middle"));
			ADD_BIND(USTR("2"), _("Block low"));
			ADD_BIND(USTR("9"), _("Punch high"));
			ADD_BIND(USTR("6"), _("Punch middle"));
			ADD_BIND(USTR("3"), _("Punch low"));
			if (gameId == GID_INDY4) {
				ADD_BIND(USTR("0"), _("Sucker punch"));
			}
			ADD_LINE;
			ADD_TEXT(_("These are for Indy on left."));
			ADD_TEXT(_("When Indy is on the right,"));
			ADD_TEXT(_("7, 4, and 1 are switched with"));
			ADD_TEXT(_("9, 6, and 3, respectively."));
			break;
		default:
			break;
		}
		break;
	case 6:
		switch (gameId) {
		case GID_INDY3:
			title = _("Biplane controls (numpad):");
			ADD_BIND(USTR("7"), _("Fly to upper left"));
			ADD_BIND(USTR("4"), _("Fly to left"));
			ADD_BIND(USTR("1"), _("Fly to lower left"));
			ADD_BIND(USTR("8"), _("Fly upwards"));
			ADD_BIND(USTR("5"), _("Fly straight"));
			ADD_BIND(USTR("2"), _("Fly down"));
			ADD_BIND(USTR("9"), _("Fly to upper right"));
			ADD_BIND(USTR("6"), _("Fly to right"));
			ADD_BIND(USTR("3"), _("Fly to lower right"));
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	while (i < HELP_NUM_LINES) {
		ADD_LINE;
	}
}

#undef ADD_BIND
#undef ADD_TEXT
#undef ADD_LINE

} // End of namespace Scumm
