/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_STAR_CONTROL_H
#define TITANIC_STAR_CONTROL_H

#include "titanic/core/game_object.h"
#include "titanic/star_control/star_field.h"
#include "titanic/star_control/star_view.h"

namespace Titanic {

class CPetControl;

class CStarControl : public CGameObject {
	DECLARE_MESSAGE_MAP;
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	bool MouseMoveMsg(CMouseMoveMsg *msg);
	bool KeyCharMsg(CKeyCharMsg *msg);
	bool FrameMsg(CFrameMsg *msg);
	bool MovementMsg(CMovementMsg *msg);
private:
	bool _enabled;
	CStarField _starField;
	CStarView _view;
	Rect _starRect;
	CPetControl *_petControl;
private:
	/**
	 * Called for ever new game frame
	 */
	void newFrame();
public:
	CLASSDEF;
	CStarControl();
	~CStarControl() override;

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;

	/**
	 * Allows the item to draw itself
	 */
	void draw(CScreenManager *screenManager) override;

	/**
	 * _starField is currently showing the starfield
	 */
	bool isStarFieldMode();

	/**
	 * Does an action in the star control
	 */
	void doAction(StarControlAction action);

	/**
	 * Returns true if the starfield puzzle has been solved
	 */
	bool isSolved() const;

	/**
	 * Return true if the starfield puzzle was skipped
	 */
	bool isSkipped() const;

	/**
	 * Forces the starfield to be solved
	 */
	void forceSolved();

	/**
	 * Returns true if a star destination can be set
	 */
	bool canSetStarDestination() const;

	/**
	 * Called when a star destination is set
	 */
	void starDestinationSet();

	/**
	 * Updates the camerea for the star view
	 */
	void updateCamera() { _view.updateCamera(); }
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_H */
