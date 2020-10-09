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

#ifndef BBVS_MINIGAMES_BBLOOGIE_H
#define BBVS_MINIGAMES_BBLOOGIE_H

#include "bbvs/minigames/minigame.h"

namespace Bbvs {

class MinigameBbLoogie : public Minigame {
public:
	MinigameBbLoogie(BbvsEngine *vm) : Minigame(vm) {};
	bool run(bool fromMainGame) override;
public:

	struct Obj {
		int kind;
		int x, y;
		int xIncr, yIncr;
		const ObjAnimation *anim;
		int frameIndex;
		int ticks;
		int status;
		int16 frameIndexAdd;
		int16 unk2;
	};

	enum {
		kMaxObjectsCount = 256
	};

	enum {
		kGSTitleScreen		= 0,	// Title screen
		kGSMainGame			= 1,	// Game when called as part of the main game
		kGSStandaloneGame	= 2,	// Game when called as standalone game
		kGSScoreCountUp		= 3		// Score countup and next level text
	};

	Obj _objects[kMaxObjectsCount];

	int _playerKind;
	const ObjAnimation *_playerAnim;
	const uint *_playerSounds1, *_playerSounds2;
	uint _playerSounds1Count, _playerSounds2Count;

	int _level, _levelTimeLeft, _levelTimeDelay;
	int _numberOfHits, _currScore, _hiScore;
	int _doubleScore, _megaLoogieCount;

	int _dispLevelScore, _nextLevelScore;

	int _timeBonusCtr, _bonusDisplayDelay1, _bonusDisplayDelay2, _bonusDisplayDelay3;

	int _carDelay;
	int _bikeDelay;
	int _squirrelDelay;
	bool _squirrelDirection;
	int _paperPlaneDelay;
	int _principalDelay;

	int _prevPrincipalStatus;
	int _principalCtr, _principalFirstFrameIndex, _principalLastFrameIndex;
	bool _principalAngry;

	char _prefix[20];

	const ObjAnimation *getAnimation(int animIndex);

	void buildDrawList(DrawList &drawList);
	void buildDrawList0(DrawList &drawList);
	void buildDrawList1(DrawList &drawList);
	void buildDrawList2(DrawList &drawList);
	void buildDrawList3(DrawList &drawList);

	void drawSprites();

	void initObjs();
	Obj *getFreeObject();
	Obj *findLoogieObj(int startObjIndex);
	bool isHit(Obj *obj1, Obj *obj2);
	bool isCursorAtObj(int objIndex);

	void initObjects();
	void initObjects0();
	void initObjects1();
	void initObjects3();

	void initVars();
	void initVars0();
	void initVars1();
	void initVars2();
	void initVars3();

	bool updateStatus(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus0(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus1(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus2(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus3(int mouseX, int mouseY, uint mouseButtons);

	void updateObjs(uint mouseButtons);
	void updatePlayer(int objIndex, uint mouseButtons);
	void updateObjKind2(int objIndex);
	void updateLoogie(int objIndex);
	void updateCar(int objIndex);
	void updateBike(int objIndex);
	void updateSquirrel(int objIndex);
	void updatePaperPlane(int objIndex);
	void updateIndicator(int objIndex);
	void updatePrincipal(int objIndex);

	void incNumberOfHits();
	void incScore(int incrAmount);
	void playRndSound();

	void update();

	void loadSounds();

};

} // End of namespace Bbvs

#endif // BBVS_MINIGAMES_BBLOOGIE_H
