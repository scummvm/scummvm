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

#ifndef ULTIMA2_VIEWS_TITLE_H
#define ULTIMA2_VIEWS_TITLE_H

#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "ultima/shared/gfx/view.h"
#include "ultima/ultima2/gfx/text_cursor.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

using namespace Shared::Messages;

/**
 * The attract-mode sequence: two text screens ("ORIGIN / PROUDLY
 * PRESENTS", "PART ][ / OF THE #1 BEST SELLING / ..."), the PICDRA title
 * picture, then the main "ULTIMA ][ / REVENGE OF THE ENCHANTRESS" menu
 * (Demonstration / Play / Create character). Choosing Demonstration
 * cycles through the PICOUT..PICMIN slideshow before looping back to the
 * start; any keypress or action before the menu is reached jumps
 * straight to it.
 */
class Title : public Shared::Gfx::View {
private:
	enum State {
		TEXT1, TEXT2, PICDRA, MENU,
		DEMO_OUT, DEMO_TWN, DEMO_DNG, DEMO_SPA, DEMO_MIN
	};
	State _state = TEXT1;
	bool _demoMode = false;

	// The single picture currently on screen, if any (loaded lazily per state)
	Graphics::ManagedSurface _pic;
	Graphics::Palette _picPalette;

	// Blinking cursor shown after "CHOICE:" while the menu is active
	Gfx::TextCursor _cursor;

	void loadPic(const Common::String &filename);
	void showMenu();
	void showState(State state);

public:
	Title();
	~Title() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
	void timeout() override;

	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
