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
#ifndef HPL_TILESET_MANAGER_H
#define HPL_TILESET_MANAGER_H

#include "hpl1/engine/resources/ResourceManager.h"

namespace hpl {

	class cGraphics;
	class cResources;
	class cTileSet;

	class cTileSetManager : public iResourceManager
	{
	public:
		cTileSetManager(cGraphics* apGraphics,cResources *apResources);
		~cTileSetManager();

		iResourceBase* Create(const tString& asName);
		cTileSet* CreateTileSet(const tString& asName);

		void Destroy(iResourceBase* apResource);
		void Unload(iResourceBase* apResource);

	private:
		cGraphics* mpGraphics;
		cResources *mpResources;
	};

};
#endif // HPL_TILESET_MANAGER_H
