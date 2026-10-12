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
#include "ultima/ultima2/data/data.h"
#include "ultima/ultima2/views/title.h"
#include "ultima/ultima2/gfx/composite.h"
#include "ultima/ultima2/gfx/ega_picture.h"
#include "ultima/ultima2/gfx/pic_decoder.h"
#include "ultima/ultima2/gfx/text.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

Title::Title() : View("Title"), _cursor("Cursor", this) {
	// Position the cursor right after "CHOICE:"
	_cursor.setPosition(Common::Point(12, 19));
	_cursor.hide();
}

void Title::loadPic(const Common::String &filename) {
	// The upgrade patch's EGA-family renderers show their own versions
	Data::RenderMode mode = g_engine->getRenderMode();
	if (Data::isEgaMode(mode)) {
		if (!Gfx::loadEgaPicture(filename, mode, _pic))
			error("Could not load %s", filename.c_str());
		return;
	}

	Gfx::PicDecoder decoder;
	Common::File f;
	if (!f.open(filename.c_str()) || !decoder.loadStream(f))
		error("Could not load %s", filename.c_str());

	_pic.create(320, 200);
	_pic.blitFrom(*decoder.getSurface());
	Gfx::compositeFinish(_pic);
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
		loadPic("picdra");
		delaySeconds(3);
		break;
	case DEMO_OUT:
		loadPic("picout");
		delaySeconds(5);
		break;
	case DEMO_TWN:
		loadPic("pictwn");
		delaySeconds(5);
		break;
	case DEMO_CAS:
		loadPic("piccas");
		delaySeconds(5);
		break;
	case DEMO_DNG:
		loadPic("picdng");
		delaySeconds(5);
		break;
	case DEMO_SPA:
		loadPic("picspa");
		delaySeconds(5);
		break;
	case DEMO_MIN:
		loadPic("picmin");
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

	if (hasPicture)
		s.blitFrom(_pic);
	else
		s.clear();
	s.setColor(Gfx::textColor(Gfx::TEXT_NORMAL));

	switch (_state) {
	case TEXT1:
		s.writeString(Common::Point(16, 10), "Origin");
		s.writeString(Common::Point(11, 12), "proudly presents");
		break;

	case TEXT2:
		s.writeString(Common::Point(16, 8), "Part ][");
		s.writeString(Common::Point(9, 10), "of the #1 best selling");
		s.writeString(Common::Point(7, 12), "fantasy role-playing game");
		s.writeString(Common::Point(12, 14), "by Lord British");
		break;

	case PICDRA:
		// Just the picture, no caption
		break;

	case DEMO_OUT:
		s.writeString(Common::Point(0, 23), "        Battle Strange Creatures");
		s.writeString(Common::Point(0, 24), "      Across the Face of the Earth");
		break;

	case DEMO_TWN:
		s.writeString(Common::Point(0, 23), "   Search for Clues in Careless Words");
		s.writeString(Common::Point(2, 24), "      Spoken at the Local Pub");
		break;

	case DEMO_CAS:
		s.writeString(Common::Point(0, 23), "        Plead with Medieval Kings");
		s.writeString(Common::Point(4, 24), "         for Assistance");
		break;

	case DEMO_DNG:
		s.writeString(Common::Point(0, 23), "   Traverse Deep Dark Deadly Dungeons");
		s.writeString(Common::Point(1, 24), "      and Tall Terrifying Towers");
		break;

	case DEMO_SPA:
		s.writeString(Common::Point(0, 23), "      Travel Throughout the Galaxy");
		s.writeString(Common::Point(1, 24), "  to the Planets of Our Solar System");
		break;

	case DEMO_MIN:
		s.writeString(Common::Point(0, 23), "   And Conquer Time Itself to Battle");
		s.writeString(Common::Point(8, 24), "Minax the Enchantress");
		break;

	case MENU:
		s.setColor(Gfx::textColor(Gfx::TEXT_TITLE));
		s.writeString(Common::Point(15, 1), "Ultima ][");
		s.setColor(Gfx::textColor(Gfx::TEXT_NAME));
		s.writeString(Common::Point(16, 3), "Revenge");
		s.writeString(Common::Point(16, 5), "of  the");
		s.writeString(Common::Point(14, 7), "Enchantress");
		s.writeString(Common::Point(6, 22), "(C)-1983,1989 by Lord British");
		s.writeString(Common::Point(15, 23), "and Origin");
		s.setColor(Gfx::textColor(Gfx::TEXT_NORMAL));
		s.writeString(Common::Point(5, 10), "Type -");
		s.writeString(Common::Point(6, 11), "'D' - For a Demonstration");
		s.writeString(Common::Point(6, 13), "'P' - Play a game of Ultima ][");
		s.writeString(Common::Point(6, 15), "'C' - Create a new character");
		s.writeString(Common::Point(6, 17), Common::String::format("'G' - Graphics: %s",
			Data::RENDER_MODE_NAMES[g_engine->getRenderMode()]));
		s.writeString(Common::Point(5, 19), "Choice:");
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
		showState(DEMO_CAS);
		break;
	case DEMO_CAS:
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
			g_engine->loadGameDialog();
			break;
		case Common::KEYCODE_c:
			replaceView("CreateCharacter");
			break;
		case Common::KEYCODE_g: {
			int nextMode = (g_engine->getRenderMode() + 1) % Data::RENDER_MODE_COUNT;
			g_engine->setRenderMode((Data::RenderMode)nextMode);
			redraw();
			break;
		}
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
