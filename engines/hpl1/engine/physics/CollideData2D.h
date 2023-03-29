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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_COLLIDE_DATA_2D_H
#define HPL_COLLIDE_DATA_2D_H

#include "common/list.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/physics/Body2D.h"
#include "hpl1/engine/scene/Tile.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cCollidedTile {
public:
	cCollidedTile(cTile *apTile, int alLayer) : mpTile(apTile), mlLayer(alLayer) {}
	cTile *mpTile;
	int mlLayer;
};

typedef Common::List<cCollidedTile> tCollidedTileList;
typedef tCollidedTileList::iterator tCollidedTileListIt;

class cCollideData2D {
public:
	tCollidedTileList mlstTiles;
	tBody2DList mlstBodies;

	cVector2f mvPushVec;

	void Clear() {
		mlstBodies.clear();
		mlstTiles.clear();
		mvPushVec = 0;
	}
};

} // namespace hpl

#endif // HPL_COLLIDE_DATA_2D_H
