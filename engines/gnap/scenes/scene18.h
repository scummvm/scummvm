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

#ifndef GNAP_SCENE18_H
#define GNAP_SCENE18_H

#include "gnap/debugger.h"
#include "gnap/scenes/scenecore.h"

namespace Gnap {

class GnapEngine;

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
	Graphics::Surface *_s18_cowboyHatSurface;

	int _s18_platPhoneCtr;
	int _s18_platPhoneIter;
	int _s18_nextPhoneSequenceId;
	int _s18_currPhoneSequenceId;

	void gnapCarryGarbageCanTo(int x, int y, int animationIndex, int argC, int a5);
	void putDownGarbageCan(int animationIndex);
	void platEndPhoning(bool platFl);
	void closeHydrantValve();
	void waitForGnapAction();
};

} // End of namespace Gnap
#endif // GNAP_SCENE18_H
