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

#ifndef MM1_VIEWS_ENH_GAME_PARTY_H
#define MM1_VIEWS_ENH_GAME_PARTY_H

#include "mm/mm1/views_enh/text_view.h"
#include "mm/shared/xeen/sprites.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

/**
 * Handles displaying the party portraits
 */
class GameParty : public TextView {
private:
	Shared::Xeen::SpriteResource _restoreSprites;
	Shared::Xeen::SpriteResource _hpSprites;
	Shared::Xeen::SpriteResource _dseFace;
	bool _highlightOn = false;

public:
	GameParty(UIElement *owner);
	virtual ~GameParty() {}

	/**
	 * Draw the view
	 */
	void draw() override;

	/**
	 * Handle game messages
	 */
	bool msgGame(const GameMessage &msg) override;

	bool msgMouseDown(const MouseDownMessage &msg) override;

	bool msgAction(const ActionMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
