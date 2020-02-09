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

#ifndef BBVS_MINIGAMES_BBANT_H
#define BBVS_MINIGAMES_BBANT_H

#include "bbvs/minigames/minigame.h"

namespace Bbvs {

class MinigameBbAnt : public Minigame {
public:
	MinigameBbAnt(BbvsEngine *vm) : Minigame(vm) {};
	bool run(bool fromMainGame) override;
public:

	struct Obj {
		int kind;
		int x, y, priority;
		int xIncr, yIncr;
		const ObjAnimation *anim;
		int frameIndex;
		int ticks;
		int otherObjIndex;
		int animIndex;
		int animIndexIncr;
		int status;
		int field30;
		int damageCtr;
		int smokeCtr;
		int counter;
		int hasSmoke;
		const ObjAnimation *anim2;
		int frameIndex2;
		int ticks2;
		int status2;
		int flag;
	};

	enum {
		kMaxObjectsCount = 256,
		kScaleDim = 28
	};

	struct ObjInit {
		const ObjAnimation *anim1;
		const ObjAnimation *anim2;
		const ObjAnimation *anim3;
		int x, y;
	};

	Obj _objects[kMaxObjectsCount];

	int _score, _hiScore;

	int _totalBugsCount;
	int _bugsChanceByKind[6], _bugsCountByKind[6];
	int _skullBugCtr;

	int _stompX, _stompY;
	int _stompDelay1;
	int _stompCounter1;
	int _stompCounter2;

	int _stompCount;
	int _hasLastStompObj;
	Obj *_lastStompObj;

	int _counter1;
	int _countdown10;
	int _counter4;
	int _levelTimeDelay;
	int _levelTimeLeft;

	int _countdown4;
	int _countdown3;
	int _countdown6;
	int _countdown5;
	int _countdown7;

	byte _scaleBuf[kScaleDim * kScaleDim];

	const ObjAnimation *getAnimation(int animIndex);
	const ObjInit *getObjInit(int index);
	const ObjAnimation * const *getObjKindAnimTable(int kind);
	const ObjAnimation *getObjAnim(int index);

	void buildDrawList0(DrawList &drawList);
	void buildDrawList1(DrawList &drawList);
	void buildDrawList2(DrawList &drawList);
	void buildDrawList3(DrawList &drawList);
	void drawMagnifyingGlass(DrawList &drawList);

	void drawSprites();
	void drawSprites0();
	void drawSprites1();
	void drawSprites2();
	void drawSprites3();

	Obj *getFreeObject();

	void initObjects();
	void initObjects0();
	void initObjects1();

	void initVars();
	void initVars1();
	void initVars2();
	void initVars3();

	bool updateStatus(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus0(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus1(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus2(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus3(int mouseX, int mouseY, uint mouseButtons);

	void getRandomBugObjValues(int &x, int &y, int &animIndexIncr, int &field30);
	void insertBugSmokeObj(int x, int y, int bugObjIndex);
	void insertSmokeObj(int x, int y);
	void resetObj(int objIndex);
	void insertStompObj(int x, int y);
	void removeStompObj(Obj *obj);
	void insertBugObj(int kind, int animIndexIncr, int always0, int x, int y, int field30, int always1);
	void removeBugObj(int objIndex);
	void updateBugObjAnim(int objIndex);
	void updateObjAnim2(int objIndex);
	void insertRandomBugObj(int kind);
	bool isBugOutOfScreen(int objIndex);
	void updateObjAnim3(int objIndex);
	void updateBugObj1(int objIndex);
	void updateObjKind2(int objIndex);
	void updateObjKind3(int objIndex);
	void updateObjKind4(int objIndex);
	void updateObjKind5(int objIndex);
	void updateStompObj(int objIndex);
	void updateSmokeObj(int objIndex);
	void updateFootObj(int objIndex);
	bool isBugAtCandy(int objIndex, int &candyObjIndex);
	bool isMagGlassAtBug(int objIndex);
	bool isMagGlassAtBeavisLeg(int objIndex);
	bool testObj5(int objIndex);
	void updateObjs(uint mouseButtons);

	void update();

	void scale2x(int x, int y);

	void loadSounds();

};

} // End of namespace Bbvs

#endif // BBVS_MINIGAMES_BBANT_H
