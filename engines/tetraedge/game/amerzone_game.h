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

#ifndef TETRAEDGE_GAME_AMERZONE_GAME_H
#define TETRAEDGE_GAME_AMERZONE_GAME_H

#include "tetraedge/game/game.h"
#include "tetraedge/te/te_timer.h"
#include "tetraedge/te/te_warp.h"

namespace Tetraedge {

/** The main Amerzone Game class.  This is known as GameWarp in the original
 *  code, but was renamed to be more descriptive in the ScummVM context */
class AmerzoneGame : public Tetraedge::Game {
public:
	AmerzoneGame();

	~AmerzoneGame() {}

	virtual void addToBag(const Common::String &objname) override;
	virtual bool changeWarp(const Common::String &zone, const Common::String &scene, bool fadeFlag) override;
	virtual void draw() override;
	virtual void enter() override;
	virtual void finishGame() override;
	virtual void initLoadedBackupData() override;
	virtual void leave(bool flag) override;
	virtual void update() override;
	virtual bool onDialogFinished(const Common::String &val) override;
	virtual bool onVideoFinished() override;

private:
	void changeSpeedToMouseDirection();
	void setAngleX(float angle);
	void setAngleY(float angle);
	void speedX(float speed);
	void speedY(float speed);

	bool onHelpButtonValidated();
	bool onMouseLeftUp(const Common::Point &pt);
	bool onMouseLeftDown(const Common::Point &pt);

	TeTimer _dragTimer;
	float _orientationX;
	float _orientationY;
	float _xAngleMin;
	float _xAngleMax;
	float _yAngleMin;
	float _yAngleMax;
	float _speedX;
	float _speedY;
	bool _isInDrag;
	int _edgeButtonRolloverCount;
	TeVector2s32 _mouseDragStart;
	TeVector2s32 _mouseDragLast;
	/*
	TeCurveAnim<AmerzoneGame, float> _decelAnimX;
	TeCurveAnim<AmerzoneGame, float> _decelAnimY;
	*/
	TeWarp *_warpX;
	TeWarp *_warpY;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_AMERZONE_GAME_H
