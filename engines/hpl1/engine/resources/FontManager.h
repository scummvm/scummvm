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
#ifndef HPL_FONT_MANAGER_H
#define HPL_FONT_MANAGER_H

#include "hpl1/engine/resources/ResourceManager.h"

namespace hpl {

	class cGraphics;
	class cResources;
	class cGui;
	class iFontData;

	class cFontManager : public iResourceManager
	{
	public:
		cFontManager(cGraphics* apGraphics,cGui *apGui,cResources *apResources);
		~cFontManager();

		iResourceBase* Create(const tString& asName);
		/**
		 * Create a new font
		 * \param asName name of the font
		 * \param alSize size the characters are rendered in
		 * \param alFirstChar first ASCII character to be rendered
		 * \param alLastChar last ASCII character to be rendered
		 * \return
		 */
		iFontData* CreateFontData(const tString& asName, int alSize=16,unsigned short alFirstChar=32,
								unsigned short alLastChar=255);

		void Destroy(iResourceBase* apResource);
		void Unload(iResourceBase* apResource);

	private:
		cGraphics* mpGraphics;
		cResources *mpResources;
		cGui *mpGui;
	};

};
#endif // HPL_FONT_MANAGER_H
