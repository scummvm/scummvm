/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TWINE_SCENE_WAGON_H
#define TWINE_SCENE_WAGON_H

#include "twine/scene/actor.h"
#include "twine/twine.h"

namespace TwinE {

class Wagon {
private:
	//TwinEEngine *_engine;

public:
	Wagon(TwinEEngine *engine) /* : _engine(engine) */ {}

	void DoAnimWagon();
	void DoDirWagon(ActorStruct *ptrobj);
	int32 GetNumBrickWagon(int32 brick);
	void AdjustEssieuWagonAvant(int32 brickw);
	void AdjustEssieuWagonArriere(int32 brickw);
};

} // namespace TwinE

#endif
