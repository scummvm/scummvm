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

#ifndef MM1_VIEWS_ENH_GAME_COMMANDS_H
#define MM1_VIEWS_ENH_GAME_COMMANDS_H

#include "mm/mm1/views_enh/button_container.h"
#include "mm/mm1/views_enh/map.h"
#include "mm/shared/xeen/sprites.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

class GameCommands : public ButtonContainer {
	class Minimap : public Map {
	public:
		bool _minimapOn = false;
		Minimap(UIElement *owner) : Map(owner) {
			_bounds = Common::Rect(236, 11, 308, 69);
		}

		void toggleMinimap() {
			_minimapOn = !_minimapOn;
			g_events->redraw();
		}
		void draw() override;
	};

private:
	Minimap _minimap;
	Shared::Xeen::SpriteResource _iconSprites;
public:
	GameCommands(UIElement *owner);
	virtual ~GameCommands() {}

	bool msgAction(const ActionMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
