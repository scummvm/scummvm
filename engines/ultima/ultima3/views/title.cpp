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
#include "common/util.h"
#include "ultima/ultima3/views/title.h"
#include "ultima/ultima3/gfx/pic_decoder.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int COLOR_TEXT = 3; // white, within CGA_PALETTE1

// Cycling order confirmed from WIND_DIRECTION_TABLE, distinct from the
// strings' own in-memory layout order
static const char *const WIND_NAMES[] = {
	"Calm Wind", "North Wind", "East Wind", "South Wind", "West Wind"
};
constexpr int WIND_COUNT = ARRAYSIZE(WIND_NAMES);
constexpr int WIND_TICK_FRAMES = 25;

Title::Title() : View("Title") {
}

void Title::loadPic(Graphics::ManagedSurface &surf, const Common::String &filename) {
	Gfx::PicDecoder decoder;
	Common::File f;
	if (!f.open(filename.c_str()) || !decoder.loadStream(f))
		error("Could not load %s", filename.c_str());

	surf.create(320, 200);
	surf.blitFrom(*decoder.getSurface());
}

bool Title::msgFocus(const FocusMessage &msg) {
	loadPic(_background, "BLANK.IBM");
	loadPic(_portrait, "EXOD.IBM");
	_windIndex = 0;
	showState(BACKGROUND);
	return View::msgFocus(msg);
}

bool Title::msgUnfocus(const UnfocusMessage &msg) {
	_background.free();
	_portrait.free();
	return View::msgUnfocus(msg);
}

void Title::showState(State state) {
	_state = state;

	switch (state) {
	case BACKGROUND:
		delaySeconds(3);
		break;
	case PORTRAIT:
		delayFrames(WIND_TICK_FRAMES);
		break;
	}

	redraw();
}

void Title::draw() {
	auto s = getSurface();

	switch (_state) {
	case BACKGROUND:
		s.blitFrom(_background);
		break;
	case PORTRAIT:
		s.blitFrom(_portrait);
		s.setColor(COLOR_TEXT);
		s.writeString(Common::Point(0, 24), WIND_NAMES[_windIndex]);
		break;
	}
}

void Title::timeout() {
	switch (_state) {
	case BACKGROUND:
		showState(PORTRAIT);
		break;
	case PORTRAIT:
		_windIndex = (_windIndex + 1) % WIND_COUNT;
		delayFrames(WIND_TICK_FRAMES);
		redraw();
		break;
	}
}

void Title::showMainMenu() {
	replaceView("MainMenu");
}

bool Title::msgKeypress(const KeypressMessage &msg) {
	showMainMenu();
	return true;
}

bool Title::msgMouseDown(const MouseDownMessage &msg) {
	showMainMenu();
	return true;
}

bool Title::msgAction(const ActionMessage &msg) {
	showMainMenu();
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
