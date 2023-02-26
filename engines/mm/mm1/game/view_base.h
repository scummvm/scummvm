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

#ifndef MM1_GAME_VIEW_BASE_H
#define MM1_GAME_VIEW_BASE_H

#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Game {

/**
 * This acts as the base class for the 3d view
 * of the surroundings. It contains the all
 * the gameplay logic except for actual drawing.
 */
class ViewBase : public Views::TextView {
protected:
	bool _isDark = false;
	Common::String _dialogMessage;
	bool _stepRandom = false;
	Common::String _descriptionLine;
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
	void forward(KeybindingAction action);

	/**
	 * Move backwards
	 */
	void backwards();

	/**
	 * Forward movement is obstructed
	 */
	void obstructed(byte mask);

	/**
	 * Barrier blocking forward movement
	 */
	void barrier();

public:
	ViewBase(UIElement *owner);
	virtual ~ViewBase() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgHeader(const HeaderMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;

	/**
	 * Updates game state
	 */
	void update();
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
