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

#ifndef ULTIMA3_VIEWS_TITLE_H
#define ULTIMA3_VIEWS_TITLE_H

#include "graphics/managed_surface.h"
#include "graphics/views/view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

using namespace Graphics::Views;

/**
 * The title screen: BLANK.IBM as a background, then EXOD.IBM (Exodus's
 * portrait) with the wind-direction indicator cycling underneath it. Any
 * key or action moves straight on to the main menu.
 */
class Title : public View {
private:
	enum State {
		BACKGROUND, PORTRAIT
	};
	State _state = BACKGROUND;
	int _windIndex = 0;

	Graphics::ManagedSurface _background, _portrait;

	void loadPic(Graphics::ManagedSurface &surf, const Common::String &filename);
	void showState(State state);
	void showMainMenu();

public:
	Title();
	~Title() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
	void timeout() override;

	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
