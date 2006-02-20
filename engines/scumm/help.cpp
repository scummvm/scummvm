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
 *
 */

#include "common/stdafx.h"

#include "common/str.h"
#include "common/util.h"

#include "scumm/help.h"
#include "scumm/scumm.h"

namespace Scumm {

int ScummHelp::numPages(byte gameId) {
	switch (gameId) {
	case GID_MANIAC:
	case GID_ZAK:
		return 4;
		break;
	case GID_INDY3:
		return 6;
		break;
	case GID_LOOM:
	case GID_MONKEY_EGA:
	case GID_MONKEY_VGA:
	case GID_MONKEY:
	case GID_MONKEY2:
	case GID_INDY4:
 	case GID_TENTACLE:
	case GID_SAMNMAX:
	case GID_DIG:
	case GID_FT:
	case GID_CMI:
		return 3;
		break;
/*	TODO - I don't know the controls for these games
	case GID_PUTTDEMO:
	case GID_PUTTPUTT:
*/
	default:
		return 2;
	}
}

#define ADD_BIND(k,d) do { key[i] = k; dsc[i] = d; i++; } while (0)
#define ADD_TEXT(d) ADD_BIND("",d)
#define ADD_LINE ADD_BIND("","")

void ScummHelp::updateStrings(byte gameId, byte version, Common::Platform platform,
				int page, String &title, String *&key, String *&dsc) {
	key = new String[HELP_NUM_LINES];
	dsc = new String[HELP_NUM_LINES];
	int i = 0;
	switch (page) {
	case 1:
		title = "Common keyboard commands:";
		ADD_BIND("F5", "Save / Load dialog");
		if (version >= 5)
			ADD_BIND(".", "Skip line of text");
		ADD_BIND("Esc", "Skip cutscene");
		ADD_BIND("Space", "Pause game");
		ADD_BIND("Ctrl 0-9", "Load game state 1-10");
		ADD_BIND("Alt 0-9", "Save game state 1-10");
#ifdef MACOSX
		ADD_BIND("Cmd q", "Quit");
#else
		ADD_BIND("Alt x, Ctrl z", "Quit");
#endif
		ADD_BIND("Alt Enter", "Toggle fullscreen");
		ADD_BIND("[, ]", "Music volume up / down");
		ADD_BIND("-, +", "Text speed slower / faster");
		ADD_BIND("Enter", "Simulate left mouse button");
		ADD_BIND("Tab", "Simulate right mouse button");
		break;
	case 2:
		title = "Special keyboard commands:";
		ADD_BIND("~, #", "Show / Hide console");
		ADD_BIND("Ctrl d", "Start the debugger");
		ADD_BIND("Ctrl s", "Show memory consumption");
		ADD_BIND("Ctrl f", "Run in fast mode (*)");
		ADD_BIND("Ctrl g", "Run in really fast mode (*)");
		ADD_BIND("Ctrl m", "Toggle mouse capture");
		ADD_BIND("Ctrl Alt 1-8", "Switch between graphics filters");
		ADD_BIND("Ctrl Alt +, -", "Increase / Decrease scale factor");
		ADD_BIND("Ctrl Alt a", "Toggle aspect-ratio correction");
		ADD_LINE;
		ADD_LINE;
		// FIXME: This should use word-wrapping, and should not assume
		//        that the font is mono-spaced.
		ADD_TEXT("* Note that using ctrl-f and");
		ADD_TEXT("  ctrl-g are not recommended");
		ADD_TEXT("  since they may cause crashes");
		ADD_TEXT("  or incorrect game behaviour.");
		break;
	case 3:
		if (gameId == GID_LOOM)
			title = "Spinning drafts on the keyboard:";
		else
			title = "Main game controls:";
		switch (gameId) {
		case GID_ZAK:
		case GID_MANIAC:
			// HACK. I know use of g_scumm here is evil, however, 
			// introducing new GID and putting it everywhere will
			// pollute code much more that this single instance
			if (g_scumm->_game.platform == Common::kPlatformNES) {
				ADD_BIND("q", "Push");
				ADD_BIND("a", "Pull");
				ADD_BIND("z", "Give");
				ADD_BIND("w", "Open");
				ADD_BIND("s", "Close");
				ADD_BIND("x", "Go to");
				ADD_BIND("e", "Get");
				ADD_BIND("d", "Use");
				ADD_BIND("c", "Read");
				ADD_BIND("r", "New kid");
				ADD_BIND("f", "Turn on");
				ADD_BIND("v", "Turn off");
				break;
			}

			ADD_BIND("q", "Push");
			ADD_BIND("a", "Pull");
			ADD_BIND("z", "Give");
			ADD_BIND("w", "Open");
			ADD_BIND("s", "Close");
			ADD_BIND("x", "Read");
			ADD_BIND("e", "Walk to");
			ADD_BIND("d", "Pick up");
			ADD_BIND("c", "What is");
			if (gameId == GID_MANIAC) {
				ADD_BIND("r", "Unlock");
				ADD_BIND("f", "New kid");
			} else {
				ADD_BIND("r", "Put on");
				ADD_BIND("f", "Take off");
			}
			ADD_BIND("v", "Use");
			ADD_BIND("t", "Turn on");
			ADD_BIND("g", "Turn off");
			if (gameId == GID_MANIAC)
				ADD_BIND("b", "Fix");
			else
				ADD_BIND("b", "Switch");
			break;
		case GID_INDY3:
			ADD_BIND("q", "Push");
			ADD_BIND("a", "Pull");
			ADD_BIND("z", "Give");
			ADD_BIND("w", "Open");
			ADD_BIND("s", "Close");
			ADD_BIND("x", "Look");
			ADD_BIND("e", "Walk to");
			ADD_BIND("d", "Pick up");
			ADD_BIND("c", "What is");
			ADD_BIND("r", "Use");
			ADD_BIND("f", "Turn on");
			ADD_BIND("v", "Turn off");
			ADD_BIND("t", "Talk");
			ADD_BIND("g", "Travel");
			ADD_BIND("b", "To Henry / To Indy");
			break;
		case GID_LOOM:
			ADD_BIND("q, c", "play C minor on distaff");
			ADD_BIND("w, d", "play D on distaff");
			ADD_BIND("e, e", "play E on distaff");
			ADD_BIND("r, f", "play F on distaff");
			ADD_BIND("t, g", "play G on distaff");
			ADD_BIND("y, a", "play A on distaff");
			ADD_BIND("u, b", "play B on distaff");
			ADD_BIND("i, C", "play C major on distaff");
			break;
		case GID_MONKEY_EGA:
		case GID_MONKEY_VGA:
			ADD_BIND("o", "Open");
			ADD_BIND("c", "Close");
			ADD_BIND("s", "puSh");
			ADD_BIND("y", "pull (Yank)");
			ADD_BIND("w", "Walk to");
			ADD_BIND("p", "Pick up");
			ADD_BIND("t", "Talk to");
			ADD_BIND("g", "Give");
			ADD_BIND("u", "Use");
			ADD_BIND("l", "Look at");
			ADD_BIND("n", "turn oN");
			ADD_BIND("f", "turn oFf");
			break;
		case GID_MONKEY:
		case GID_MONKEY2:
		case GID_INDY4:
		case GID_TENTACLE:
			ADD_BIND("g", "Give");
			ADD_BIND("o", "Open");
			ADD_BIND("c", "Close");
			ADD_BIND("p", "Pick up");
			ADD_BIND("l", "Look at");
			ADD_BIND("t", "Talk to");
			ADD_BIND("u", "Use");
			ADD_BIND("s", "puSh");
			ADD_BIND("y", "pull (Yank)");
			if (platform == Common::kPlatformSegaCD) {
			// FIXME look at scripts to figure all options out...
			// keys 1->4 seem to do something as well
				ADD_BIND("6", "Highlight prev dialogue");
				ADD_BIND("7", "Highlight next dialogue");
			}
			break;
		case GID_SAMNMAX:
			ADD_BIND("w", "Walk");
			ADD_BIND("t", "Talk");
			ADD_BIND("u", "Use");
			ADD_BIND("i", "Inventory");
			ADD_BIND("o", "Object");
			ADD_BIND("p", "Pick up");
			ADD_BIND("l", "Look");
			ADD_BIND("b", "Black and White / Color");
			break;
		case GID_FT:
			ADD_BIND("e", "Eyes");
			ADD_BIND("t", "Tongue");
			ADD_BIND("i", "Inventory");
			ADD_BIND("p", "Punch");
			ADD_BIND("k", "Kick");
			break;
		case GID_DIG:
			ADD_BIND("e", "Examine");
			ADD_BIND("t", "Regular cursor");
			ADD_BIND("i", "Inventory");
			ADD_BIND("c", "Comm");
			break;
		case GID_CMI:
			ADD_BIND("F1", "Save / Load / Options");
			ADD_BIND("e", "Examine");
			ADD_BIND("t", "Talk to");
			ADD_BIND("i", "Inventory");
			ADD_BIND("u", "Use");
			break;
		}
		break;
	case 4:
		title = "Other game controls:";
		if (version <= 2) {
			ADD_TEXT("Inventory: (not yet implemented)");
			ADD_BIND("u", "Scroll list up");
			ADD_BIND("j", "Scroll list down");
			ADD_BIND("i", "Upper left item");
			ADD_BIND("k", "Lower left item");
			ADD_BIND("o", "Upper right item");
			ADD_BIND("l", "Lower right item");
			ADD_LINE;
			ADD_TEXT("Switching characters:");
			if (gameId == GID_MANIAC) {
				ADD_BIND("F1", "Dave");
				ADD_BIND("F2", "Second kid");
				ADD_BIND("F3", "Third kid");
			} else {
				ADD_BIND("F1", "Zak");
				ADD_BIND("F2", "Annie");
				ADD_BIND("F3", "Melissa");
				ADD_BIND("F4", "Leslie");
			}
		} else if (gameId == GID_INDY3 || gameId == GID_ZAK) {
			// Indy3, or FM-TOWNS Zak
			ADD_TEXT("Inventory:");
			ADD_BIND("y", "Upper left item");
			ADD_BIND("h", "Middle left item");
			ADD_BIND("n", "Lower left item");
			ADD_BIND("u", "Upper right item");
			ADD_BIND("j", "Middle right item");
			ADD_BIND("m", "Lower right item");
			ADD_BIND("o", "Scroll list up");
			ADD_BIND("l", "Scroll list down");
			if (gameId == GID_ZAK) {
				ADD_LINE;
				ADD_TEXT("Switching characters:");
				ADD_BIND("F1", "Zak");
				ADD_BIND("F2", "Annie");
				ADD_BIND("F3", "Melissa");
				ADD_BIND("F4", "Leslie");
			}
		}
		break;
	case 5:
		switch (gameId) {
		case GID_INDY3:
			title = "Fighting controls (numpad):";
			ADD_BIND("7", "Step back");
			ADD_BIND("4", "Step back");
			ADD_BIND("1", "Step back");
			ADD_BIND("8", "Block high");
			ADD_BIND("5", "Block middle");
			ADD_BIND("2", "Block low");
			ADD_BIND("9", "Punch high");
			ADD_BIND("6", "Punch middle");
			ADD_BIND("3", "Punch low");
			ADD_LINE;
			ADD_LINE;
			ADD_TEXT("These are for Indy on left.");
			ADD_TEXT("When Indy is on the right,");
			ADD_TEXT("7, 4, and 1 are switched with");
			ADD_TEXT("9, 6, and 3, respectively.");
			break;
		}
		break;
	case 6:
		switch (gameId) {
		case GID_INDY3:
			title = "Biplane controls (numpad):";
			ADD_BIND("7", "Fly to upper left");
			ADD_BIND("4", "Fly to left");
			ADD_BIND("1", "Fly to lower left");
			ADD_BIND("8", "Fly upwards");
			ADD_BIND("5", "Fly straight");
			ADD_BIND("2", "Fly down");
			ADD_BIND("9", "Fly to upper right");
			ADD_BIND("6", "Fly to right");
			ADD_BIND("3", "Fly to lower right");
			break;
		}
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
