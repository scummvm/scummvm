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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "tsage/ringworld_demo.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Ringworld Demo scene
 *
 *--------------------------------------------------------------------------*/

void RingworldDemoScene::postInit(SceneObjectList *OwnerList) {
	signal();
}

void RingworldDemoScene::signal() {
	_soundHandler.startSound(4);
	_actor1.postInit();
	_actor2.postInit();
	_actor3.postInit();
	_actor4.postInit();
	_actor5.postInit();
	_actor6.postInit();
	
	setAction(&_sequenceManager, this, 22, &_actor1, &_actor2, &_actor3, &_actor4, &_actor5, &_actor6, NULL);
}

void RingworldDemoScene::process(Event &event) {
	
}

} // End of namespace tSage
