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
#ifndef HPL_TILEMAP_IT_H
#define HPL_TILEMAP_IT_H

#include "hpl1/engine/scene/Tile.h"

namespace hpl {

	class iTileMapIt
	{
	public:
		virtual ~iTileMapIt(){}

		virtual bool HasNext()=0;
		virtual cTile* Next()=0;
		virtual cTile* PeekNext()=0;
		virtual int GetNum()=0;
		virtual int GetCurrentLayer()=0;
	};
};
#endif // HPL_TILEMAP_IT_H
