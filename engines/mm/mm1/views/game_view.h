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

#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Views {

class GameView : public TextView {
	enum ViewState { VIEW_NORMAL, VIEW_DARKNESS };
private:
	int _mapOffset = 0;
	ViewState _state = VIEW_NORMAL;
	int _val1 = 0, _val2 = 0, _val3 = 0;
	int _val4 = 0, _val5 = 0, _val6 = 0;
	byte _mask = 0;
	byte _arr1[11];
private:
	/**
	 * Turn left
	 */
	void turnLeft();

	/**
	 * Turn right
	 */
	void turnRight();

	/**
	 * Move forwards
	 */
	void forward();

	/**
	 * Move backwards
	 */
	void backwards();

	/**
	 * Forward movement is obstructed
	 */
	void obstructed();

	/**
	 * Draws a tile
	 */
	void drawTile();
public:
	GameView(UIElement *owner);
	virtual ~GameView() {}

	void draw() override;
	bool msgGame(const GameMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;

	/**
	 * Updates game state
	 */
	void update();
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
