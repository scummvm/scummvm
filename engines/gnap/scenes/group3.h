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

#ifndef GNAP_GROUP3_H
#define GNAP_GROUP3_H

#include "gnap/debugger.h"

namespace Gnap {

class GnapEngine;
class CutScene;

class Scene30: public Scene {
public:
	Scene30(GnapEngine *vm);
	~Scene30() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _kidSequenceId;
};

class Scene31: public Scene {
public:
	Scene31(GnapEngine *vm);
	~Scene31() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	bool _beerGuyDistracted;
	int _currClerkSequenceId;
	int _nextClerkSequenceId;
	int _clerkMeasureCtr;
	int _clerkMeasureMaxCtr;
};

class Scene32: public Scene {
public:
	Scene32(GnapEngine *vm);
	~Scene32() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};
};

class Scene33: public Scene {
public:
	Scene33(GnapEngine *vm);
	~Scene33() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currChickenSequenceId;
	int _nextChickenSequenceId;
};

class Scene38: public Scene {
public:
	Scene38(GnapEngine *vm);
	~Scene38() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};
};

class Scene39: public Scene {
public:
	Scene39(GnapEngine *vm);
	~Scene39() {}

	virtual int init();
	virtual void updateHotspots();
	virtual void run();
	virtual void updateAnimations();
	virtual void updateAnimationsCb() {};

private:
	int _currGuySequenceId;
	int _nextGuySequenceId;
};

} // End of namespace Gnap

#endif // GNAP_GROUP3_H
