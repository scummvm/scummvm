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

#ifndef GNAP_GROUP0_H
#define GNAP_GROUP0_H

#include "gnap/debugger.h"

namespace Gnap {

class GnapEngine;
class CutScene;

class Scene01: public Scene {
public:
	Scene01(GnapEngine *vm);
	~Scene01();

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {}

private:
	int _pigsIdCtr;
	int _smokeIdCtr;
	Graphics::Surface *_spaceshipSurface;
};

class Scene02: public Scene {
public:
	Scene02(GnapEngine *vm);
	~Scene02() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {}

private:
	int _truckGrillCtr;
	int _nextChickenSequenceId;
	int _currChickenSequenceId;
	int _gnapTruckSequenceId;
};

class Scene03: public Scene {
public:
	Scene03(GnapEngine *vm);
	~Scene03() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {}

private:
	bool _platypusHypnotized;	
	bool _platypusScared;
	int _nextPlatSequenceId;
	int _nextFrogSequenceId;
	int _currFrogSequenceId;
};

class Scene04: public Scene {
public:
	Scene04(GnapEngine *vm);
	~Scene04() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {}

private:
	bool _triedWindow;
	int _dogIdCtr;
	int _nextDogSequenceId;
	int _currDogSequenceId;
};

class Scene05: public Scene {
public:
	Scene05(GnapEngine *vm);
	~Scene05() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {}

private:
	int _nextChickenSequenceId;
	int _currChickenSequenceId;
};

class Scene06: public Scene {
public:
	Scene06(GnapEngine *vm);
	~Scene06() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {}

private:
	bool _horseTurnedBack;
	int _nextPlatSequenceId;
	int _nextHorseSequenceId;
	int _currHorseSequenceId;
};

class Scene07: public Scene {
public:
	Scene07(GnapEngine *vm);
	~Scene07() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {}
};

class Scene08: public Scene {
public:
	Scene08(GnapEngine *vm);
	~Scene08() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb();

private:
	int _nextDogSequenceId;
	int _currDogSequenceId;
	int _nextManSequenceId;
	int _currManSequenceId;
};

class Scene09: public Scene {
public:
	Scene09(GnapEngine *vm);
	~Scene09() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {}
};

} // End of namespace Gnap

#endif // GNAP_GROUP0_H
