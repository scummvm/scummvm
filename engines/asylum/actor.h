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
 * $URL: https://asylumengine.googlecode.com/svn/trunk/asylum.cpp $
 * $Id: asylum.cpp 138 2009-06-15 14:37:23Z bluegr@gmail.com $
 *
 */

#ifndef ASYLUM_ACTOR_H_
#define ASYLUM_ACTOR_H_

#include "asylum/respack.h"
#include "asylum/graphics.h"

namespace Asylum {

// TODO properly use this enum as opposed to just
// using it for visual reference :P
enum ActorResources {
	kSound1 = 0,
	kSound2 = 1,
	kSound3 = 2,
	kSound4 = 3,
	kUnused = 4,
	kFlags  = 5,

	kWalkN  = 6,
	kWalkNW = 7,
	kWalkW  = 8,
	kWalkSW = 9,
	kWalkS  = 10
};

/*
uint32 faceN;
	uint32 faceNW;
	uint32 faceW;
	uint32 faceSW;
	uint32 faceS;

	uint32 faceN2;
*/

class MainActor {
public:
	MainActor(uint8 *data);
	virtual ~MainActor();

	// TODO I know md5 won't like this, but it's
	// still WIP code :P
	void setAction(ResourcePack *res, int action);

	GraphicFrame *getFrame();

	uint32 x;
	uint32 y;

private:
	GraphicResource *_graphic;
	uint32 _resources[61];
	uint8  _currentFrame;

}; // end of class MainActor

} // end of namespace Asylum

#endif
