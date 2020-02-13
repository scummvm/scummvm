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

#ifndef BBVS_MINIGAMES_BBTENNIS_H
#define BBVS_MINIGAMES_BBTENNIS_H

#include "bbvs/minigames/minigame.h"

namespace Bbvs {

class MinigameBbTennis : public Minigame {
public:
	MinigameBbTennis(BbvsEngine *vm) : Minigame(vm) {};
	bool run(bool fromMainGame) override;
public:

	struct Obj {
		int kind;
		int x, y;
		const ObjAnimation *anim;
		int frameIndex;
		int ticks;
		int status;
		int blinkCtr;
		float fltStepX;
		float fltStepY;
		float fltX;
		float fltY;
		int targetX;
		int targetY;
		int ballStep;
		int ballStepCtr;
		int netPlayDirection;
	};

	enum {
		kMaxObjectsCount = 256
	};

	enum {
		kGSTitleScreen      = 0,    // Title screen
		kGSMainGame         = 1,    // Game when called as part of the main game
		kGSStandaloneGame   = 2,    // Game when called as standalone game
		kGSScoreCountUp     = 3     // Score countup and next level text
	};

	Obj _objects[kMaxObjectsCount];

	int _numHearts;
	int _squirrelDelay;
	int _tennisPlayerDelay;
	int _throwerDelay;
	int _netPlayerDelay;
	int _playerDecrease;
	int _delayDecreaseTimer;
	int _numBalls;
	int _newBallTimer;
	int _initBallTimer;
	int _maxBalls;
	int _rapidFireBallsCount;
	int _score, _hiScore;
	int _hitMissRatio;
	bool _allHeartsGone;
	bool _playedThisIsTheCoolest;
	bool _startSoundPlayed;
	bool _endSoundPlaying;

	const ObjAnimation *getAnimation(int animIndex);

	void buildDrawList(DrawList &drawList);
	void buildDrawList0(DrawList &drawList);
	void buildDrawList1(DrawList &drawList);
	void buildDrawList2(DrawList &drawList);

	void drawSprites();

	void initObjs();
	Obj *getFreeObject();
	Obj *findTennisBall(int startObjIndex);
	bool isHit(Obj *obj1, Obj *obj2);

	void initObjects();
	void initObjects0();
	void initObjects1();
	void initObjects2();

	void initVars();
	void initVars0();
	void initVars1();
	void initVars2();

	bool updateStatus(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus0(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus1(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus2(int mouseX, int mouseY, uint mouseButtons);

	void updateObjs();
	void updateTennisBall(int objIndex);
	void updateSquirrel(int objIndex);
	void updateTennisPlayer(int objIndex);
	void updateThrower(int objIndex);
	void updateNetPlayer(int objIndex);
	void updateEnemyTennisBall(int objIndex);
	void makeEnemyBall(int x, int y, int frameIndex);
	void hitSomething();

	void update();

	void loadSounds();

};

} // End of namespace Bbvs

#endif // BBVS_MINIGAMES_BBTENNIS_H
