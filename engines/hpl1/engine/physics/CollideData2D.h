/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HPL_COLLIDE_DATA_2D_H
#define HPL_COLLIDE_DATA_2D_H

#include <list>
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/physics/Body2D.h"
#include "hpl1/engine/scene/Tile.h"

namespace hpl {

	class cCollidedTile
	{
	public:
		cCollidedTile(cTile* apTile, int alLayer) : mpTile(apTile), mlLayer(alLayer){}
		cTile* mpTile;
		int mlLayer;
	};

	typedef std::list<cCollidedTile> tCollidedTileList;
	typedef tCollidedTileList::iterator tCollidedTileListIt;

	class cCollideData2D
	{
	public:
		tCollidedTileList mlstTiles;
		tBody2DList mlstBodies;

		cVector2f mvPushVec;

		void Clear(){
			mlstBodies.clear();
			mlstTiles.clear();
			mvPushVec =0;
		}
	};

};
#endif // HPL_COLLIDE_DATA_2D_H
