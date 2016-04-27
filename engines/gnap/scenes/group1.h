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

#ifndef GNAP_GROUP1_H
#define GNAP_GROUP1_H

#include "gnap/debugger.h"

namespace Gnap {

class GnapEngine;
class CutScene;

class Scene10: public Scene {
public:
	Scene10(GnapEngine *vm);
	~Scene10() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb();

private:
	int _nextCookSequenceId;
	int _currCookSequenceId;
};

class Scene11: public Scene {
public:
	Scene11(GnapEngine *vm);
	~Scene11() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _billardBallCtr;
	int _nextHookGuySequenceId;
	int _currHookGuySequenceId;
	int _nextGoggleGuySequenceId;
	int _currGoggleGuySequenceId;
};

class Scene12: public Scene {
public:
	Scene12(GnapEngine *vm);
	~Scene12() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _nextBeardGuySequenceId;
	int _currBeardGuySequenceId;
	int _nextToothGuySequenceId;
	int _currToothGuySequenceId;
	int _nextBarkeeperSequenceId;
	int _currBarkeeperSequenceId;
};

class Scene13: public Scene {
public:
	Scene13(GnapEngine *vm);
	~Scene13() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _backToiletCtr;

	void showScribble();
};

class Scene14: public Scene {
public:
	Scene14(GnapEngine *vm);
	~Scene14() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};
};

class Scene15: public Scene {
public:
	Scene15(GnapEngine *vm);
	~Scene15() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _nextRecordSequenceId;
	int _currRecordSequenceId;
	int _nextSlotSequenceId;
	int _currSlotSequenceId;
	int _nextUpperButtonSequenceId;
	int _currUpperButtonSequenceId;
	int _nextLowerButtonSequenceId;
	int _currLowerButtonSequenceId;	
};

class Scene17: public Scene {
public:
	Scene17(GnapEngine *vm);
	~Scene17() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	bool _canTryGetWrench;
	int _wrenchCtr;
	int _platPhoneCtr;
	int _platTryGetWrenchCtr;
	int _nextPhoneSequenceId;
	int _currPhoneSequenceId;
	int _nextWrenchSequenceId;
	int _currWrenchSequenceId;
	int _nextCarWindowSequenceId;
	int _currCarWindowSequenceId;

	void update();
	void platHangUpPhone();
};

class Scene18: public Scene {
public:
	Scene18(GnapEngine *vm);
	~Scene18();

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	Graphics::Surface *_cowboyHatSurface;

	int _platPhoneCtr;
	int _platPhoneIter;
	int _nextPhoneSequenceId;
	int _currPhoneSequenceId;

	void gnapCarryGarbageCanTo(int x, int y, int animationIndex, int argC, int a5);
	void putDownGarbageCan(int animationIndex);
	void platEndPhoning(bool platFl);
	void closeHydrantValve();
	void waitForGnapAction();
};

class Scene19: public Scene {
public:
	Scene19(GnapEngine *vm);
	~Scene19();

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currShopAssistantSequenceId;
	int _nextShopAssistantSequenceId;
	int _toyGrabCtr;
	int _shopAssistantCtr;

	Graphics::Surface *_pictureSurface;
};

} // End of namespace Gnap

#endif // GNAP_GROUP1_H
