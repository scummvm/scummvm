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

#ifndef HODJNPODJ_METAGAME_VIEWS_MINIGAMES_H
#define HODJNPODJ_METAGAME_VIEWS_MINIGAMES_H

#include "bagel/hodjnpodj/views/view.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define MINIGAMES_HOTSPOTS_COUNT 20

class Minigames : public View {
	struct MinigameRect {
		int left, top, right, bottom;
	};
private:
	static MinigameRect MINIGAME_RECTS[MINIGAMES_HOTSPOTS_COUNT];
	static int MINIGAME_IDS[MINIGAMES_HOTSPOTS_COUNT];
	static const char *MINIGAME_TEXTS[MINIGAMES_HOTSPOTS_COUNT];
	GfxSurface _background;
	ColorButton _exitButton;
	Common::Rect _textRect;
	Common::Point _mousePos;

	int getSelectedMinigame() const;

public:
	Minigames();

	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
	void draw() override;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
