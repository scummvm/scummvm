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

#ifndef GNAP_SCENE51_H
#define GNAP_SCENE51_H

#include "gnap/debugger.h"
#include "gnap/scenes/scenecore.h"

namespace Gnap {

class GnapEngine;

struct Scene51Item {
	int _currSequenceId;
	int _droppedSequenceId;
	int _x, _y;
	int _collisionX;
	bool _canCatch;
	bool _isCollision;
	int _x2;
	int _id;
};

class Scene51: public Scene {
public:
	Scene51(GnapEngine *vm);
	~Scene51() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations() {};
	virtual void updateAnimationsCb() {};

private:
	bool _s51_dropLoseCash;

	int _s51_cashAmount;
	int _s51_digits[4];
	int _s51_digitSequenceIds[4];
	int _s51_guySequenceId;
	int _s51_guyNextSequenceId;
	int _s51_itemsCaughtCtr;
	int _s51_dropSpeedTicks;
	int _s51_nextDropItemKind;
	int _s51_itemInsertX;
	int _s51_itemInsertDirection;
	int _s51_platypusSequenceId;
	int _s51_platypusNextSequenceId;
	int _s51_platypusJumpSequenceId;
	int _s51_itemsCtr;
	int _s51_itemsCtr1;
	int _s51_itemsCtr2;

	Scene51Item _s51_items[6];

	void clearItem(Scene51Item *item);
	void dropNextItem();
	void updateItemAnimations();
	int checkCollision(int sequenceId);
	void updateItemAnimation(Scene51Item *item, int index);
	void removeCollidedItems();
	int itemIsCaught(Scene51Item *item);
	bool isJumpingRight(int sequenceId);
	bool isJumpingLeft(int sequenceId);
	bool isJumping(int sequenceId);
	void waitForAnim(int animationIndex);
	int getPosRight(int sequenceId);
	int getPosLeft(int sequenceId);
	void playIntroAnim();
	void updateGuyAnimation();
	int incCashAmount(int sequenceId);
	void winMinigame();
	void playCashAppearAnim();
	void updateCash(int amount);
	void drawDigit(int digit, int position);
	void initCashDisplay();
};

} // End of namespace Gnap
#endif // GNAP_SCENE51_H
