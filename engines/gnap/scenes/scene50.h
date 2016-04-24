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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GNAP_SCENE50_H
#define GNAP_SCENE50_H

#include "gnap/debugger.h"
#include "gnap/scenes/scenecore.h"

namespace Gnap {

class GnapEngine;

class Scene50: public Scene {
public:
	Scene50(GnapEngine *vm);
	~Scene50() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	bool _s50_fightDone;
	int _s50_timesPlayed;
	int _s50_timesPlayedModifier;
	int _s50_attackCounter;
	int _s50_roundNum;
	int _s50_timeRemaining;
	int _s50_leftTongueRoundsWon;
	int _s50_rightTongueRoundsWon;
	int _s50_leftTongueEnergyBarPos;
	int _s50_rightTongueEnergyBarPos;
	int _s50_leftTongueSequenceId;
	int _s50_leftTongueId;
	int _s50_leftTongueNextSequenceId;
	int _s50_leftTongueNextId;
	int _s50_leftTongueNextIdCtr;
	int _s50_rightTongueSequenceId;
	int _s50_rightTongueId;
	int _s50_rightTongueNextSequenceId;
	int _s50_rightTongueNextId;
	int _s50_rightTongueNextIdCtr;
	int _s50_leftTongueEnergy;
	int _s50_rightTongueEnergy;

	bool tongueWinsRound(int tongueNum);
	void playWinAnim(int tongueNum, bool fightOver);
	void delayTicks();
	void initRound();
	bool updateCountdown();
	void drawCountdown(int value);
	void playTonguesIdle();
	void playRoundAnim(int roundNum);
	bool updateEnergyBars(int newLeftBarPos, int newRightBarPos);
	void waitForAnim(int animationIndex);
	int checkInput();
	int getRightTongueAction();
	int getRightTongueActionTicks();
	int getLeftTongueNextId();
	int getRightTongueNextId();
	void playWinBadgeAnim(int tongueNum);
};

} // End of namespace Gnap
#endif // GNAP_SCENE50_H
