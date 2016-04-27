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

#ifndef GNAP_GROUP2_H
#define GNAP_GROUP2_H

#include "gnap/debugger.h"

namespace Gnap {

class GnapEngine;
class CutScene;

class Scene20: public Scene {
public:
	Scene20(GnapEngine *vm);
	~Scene20() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb();

private:
	int _currStonerGuySequenceId;
	int _nextStonerGuySequenceId;
	int _currGroceryStoreGuySequenceId;
	int _nextGroceryStoreGuySequenceId;
	int _stonerGuyCtr;
	int _groceryStoreGuyCtr;
	bool _stonerGuyShowingJoint;

	void stopSounds();
};

class Scene21: public Scene {
public:
	Scene21(GnapEngine *vm);
	~Scene21() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currOldLadySequenceId;
	int _nextOldLadySequenceId;
};

class Scene22: public Scene {
public:
	Scene22(GnapEngine *vm);
	~Scene22() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currCashierSequenceId;
	int _nextCashierSequenceId;
	bool _caughtBefore;
	int _cashierCtr;
};

class Scene23: public Scene {
public:
	Scene23(GnapEngine *vm);
	~Scene23() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currStoreClerkSequenceId;
	int _nextStoreClerkSequenceId;
};

class Scene24: public Scene {
public:
	Scene24(GnapEngine *vm);
	~Scene24() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currWomanSequenceId;
	int _nextWomanSequenceId;
	int _boySequenceId;
	int _girlSequenceId;
};

class Scene25: public Scene {
public:
	Scene25(GnapEngine *vm);
	~Scene25() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currTicketVendorSequenceId;
	int _nextTicketVendorSequenceId;

	void playAnims(int index);
};

class Scene26: public Scene {
public:
	Scene26(GnapEngine *vm);
	~Scene26() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currKidSequenceId;
	int _nextKidSequenceId;
};

class Scene27: public Scene {
public:
	Scene27(GnapEngine *vm);
	~Scene27() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _nextJanitorSequenceId;
	int _currJanitorSequenceId;
};

class Scene28: public Scene {
public:
	Scene28(GnapEngine *vm);
	~Scene28() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currClownSequenceId;
	int _nextClownSequenceId;
	int _clownTalkCtr;
};

class Scene29: public Scene {
public:
	Scene29(GnapEngine *vm);
	~Scene29() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currMonkeySequenceId;
	int _nextMonkeySequenceId;
	int _currManSequenceId;
	int _nextManSequenceId;
};

} // End of namespace Gnap

#endif // GNAP_GROUP1_H
