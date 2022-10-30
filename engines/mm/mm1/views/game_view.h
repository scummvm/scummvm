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

#ifndef MM1_VIEWS_GAME_VIEW_H
#define MM1_VIEWS_GAME_VIEW_H

#include "mm/mm1/game/view_base.h"

namespace MM {
namespace MM1 {
namespace Views {

class GameView : public Game::ViewBase {
private:
	int _tileIndex = 0, _srcWidth = 0, _srcPitch = 0;
	int _destLeft = 0, _destTop = 0, _srcLeft = 0;
	byte _mask = 0;
	byte _drawFlags[10];
private:
	/**
	 * Draws a tile
	 */
	void drawTile();

	/**
	 * Draws the scene
	 */
	void drawScene();

	/**
	 * Draws the dialog message
	 */
	void drawDialogMessage();

	/**
	 * Draws a business name or other information like
	 * container container for searches
	 */
	void drawDescriptionLine();
public:
	GameView(UIElement *owner);
	virtual ~GameView() {}

	void draw() override;
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
