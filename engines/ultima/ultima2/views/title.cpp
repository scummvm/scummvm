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

#include "common/file.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "ultima/ultima2/views/title.h"
#include "ultima/ultima2/gfx/pic_decoder.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

// Color used for the picture-caption text, an index into the picture's
// own 4-color CGA palette (see Gfx::PicDecoder) rather than the engine's
// standard 16-color EGA one used for the plain text screens/menu
constexpr int COLOR_CAPTION = 3; // white, within CGA_PALETTE1

constexpr int COLOR_TEXT = 15; // white, within the standard EGA palette

Title::Title() : View("Title"), _cursor("Cursor", this) {
	// Position the cursor right after "CHOICE:"
	_cursor.setPosition(Common::Point(12, 19));
	_cursor.hide();
}

void Title::loadPic(const Common::String &filename) {
	Gfx::PicDecoder decoder;
	Common::File f;
	if (!f.open(filename.c_str()) || !decoder.loadStream(f))
		error("Could not load %s", filename.c_str());

	_pic.create(320, 200);
	_pic.blitFrom(*decoder.getSurface());
	_picPalette = decoder.getPalette();
}

bool Title::msgFocus(const FocusMessage &msg) {
	showState(TEXT1);
	return View::msgFocus(msg);
}

bool Title::msgUnfocus(const UnfocusMessage &msg) {
	_pic.free();
	return View::msgUnfocus(msg);
}

void Title::showState(State state) {
	_state = state;
	_cursor.hide();

	switch (state) {
	case TEXT1:
		delaySeconds(3);
		break;
	case TEXT2:
		delaySeconds(4);
		break;
	case PICDRA:
		loadPic("PICDRA");
		delaySeconds(3);
		break;
	case DEMO_OUT:
		loadPic("PICOUT");
		delaySeconds(5);
		break;
	case DEMO_TWN:
		loadPic("PICTWN");
		delaySeconds(5);
		break;
	case DEMO_DNG:
		loadPic("PICDNG");
		delaySeconds(5);
		break;
	case DEMO_SPA:
		loadPic("PICSPA");
		delaySeconds(5);
		break;
	case DEMO_MIN:
		loadPic("PICMIN");
		delaySeconds(5);
		break;
	case MENU:
		// No timeout; this state waits indefinitely for a keypress
		_cursor.show();
		break;
	}

	redraw();
}

void Title::showMenu() {
	showState(MENU);
}

void Title::draw() {
	auto s = getSurface();
	bool hasPicture = _state == PICDRA || _state >= DEMO_OUT;

	if (hasPicture) {
		g_system->getPaletteManager()->setPalette(_picPalette);
		s.blitFrom(_pic);
		s.setColor(COLOR_CAPTION);
	} else {
		g_system->getPaletteManager()->setPalette(Graphics::Palette::createEGAPalette());
		s.clear();
		s.setColor(COLOR_TEXT);
	}

	switch (_state) {
	case TEXT1:
		s.writeString(Common::Point(16, 10), "ORIGIN");
		s.writeString(Common::Point(11, 12), "PROUDLY PRESENTS");
		break;

	case TEXT2:
		s.writeString(Common::Point(16, 8), "PART ][");
		s.writeString(Common::Point(9, 10), "OF THE #1 BEST SELLING");
		s.writeString(Common::Point(7, 12), "FANTASY ROLE-PLAYING GAME");
		s.writeString(Common::Point(12, 14), "BY LORD BRITISH");
		break;

	case PICDRA:
		// Just the picture, no caption
		break;

	case DEMO_OUT:
		s.writeString(Common::Point(0, 23), "        BATTLE STRANGE CREATURES");
		s.writeString(Common::Point(0, 24), "      ACROSS THE FACE OF THE EARTH");
		break;

	case DEMO_TWN:
		s.writeString(Common::Point(0, 23), "   SEARCH FOR CLUES IN CARELESS WORDS");
		s.writeString(Common::Point(2, 24), "      SPOKEN AT THE LOCAL PUB");
		break;

	case DEMO_DNG:
		s.writeString(Common::Point(0, 23), "   TRAVERSE DEEP DARK DEADLY DUNGEONS");
		s.writeString(Common::Point(1, 24), "      AND TALL TERRIFYING TOWERS");
		break;

	case DEMO_SPA:
		s.writeString(Common::Point(0, 23), "      TRAVEL THROUGHOUT THE GALAXY");
		s.writeString(Common::Point(1, 24), "  TO THE PLANETS OF OUR SOLAR SYSTEM");
		break;

	case DEMO_MIN:
		s.writeString(Common::Point(0, 23), "   AND CONQUER TIME ITSELF TO BATTLE");
		s.writeString(Common::Point(8, 24), "MINAX THE ENCHANTRESS");
		break;

	case MENU:
		s.writeString(Common::Point(15, 1), "ULTIMA ][");
		s.writeString(Common::Point(16, 3), "REVENGE");
		s.writeString(Common::Point(16, 5), "OF  THE");
		s.writeString(Common::Point(14, 7), "ENCHANTRESS");
		s.writeString(Common::Point(6, 22), "(C)-1983,1989 BY LORD BRITISH");
		s.writeString(Common::Point(15, 23), "AND ORIGIN");
		s.writeString(Common::Point(5, 11), "TYPE -");
		s.writeString(Common::Point(6, 13), "'D' - FOR A DEMONSTRATION");
		s.writeString(Common::Point(6, 15), "'P' - PLAY A GAME OF ULTIMA ][");
		s.writeString(Common::Point(6, 17), "'C' - CREATE A NEW CHARACTER");
		s.writeString(Common::Point(5, 19), "CHOICE:");
		break;
	}
}

void Title::timeout() {
	switch (_state) {
	case TEXT1:
		showState(TEXT2);
		break;
	case TEXT2:
		showState(PICDRA);
		break;
	case PICDRA:
		// Once a demonstration has been started, loop straight back into
		// it instead of showing the menu again
		showState(_demoMode ? DEMO_OUT : MENU);
		break;
	case DEMO_OUT:
		showState(DEMO_TWN);
		break;
	case DEMO_TWN:
		showState(DEMO_DNG);
		break;
	case DEMO_DNG:
		showState(DEMO_SPA);
		break;
	case DEMO_SPA:
		showState(DEMO_MIN);
		break;
	case DEMO_MIN:
		showState(TEXT1);
		break;
	case MENU:
		break;
	}
}

bool Title::msgKeypress(const KeypressMessage &msg) {
	if (_state == MENU) {
		switch (msg.keycode) {
		case Common::KEYCODE_d:
			_demoMode = true;
			showState(DEMO_OUT);
			break;
		case Common::KEYCODE_p:
			// TODO: play a saved game -- needs the Savegame/data model, added in a later stage
			break;
		case Common::KEYCODE_c:
			// TODO: create a new character -- needs the Savegame/data model, added in a later stage
			break;
		default:
			break;
		}
	} else {
		showMenu();
	}

	return true;
}

bool Title::msgAction(const ActionMessage &msg) {
	if (_state != MENU)
		showMenu();

	return true;
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
