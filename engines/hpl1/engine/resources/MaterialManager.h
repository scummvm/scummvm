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
#ifndef HPL_MATERIAL_MANAGER_H
#define HPL_MATERIAL_MANAGER_H

#include "hpl1/engine/resources/ResourceManager.h"
#include "hpl1/engine/graphics/Texture.h"
#include "hpl1/engine/graphics/Material.h"

namespace hpl {

	class cGraphics;
	class cResources;
	class iMaterial;

	class cMaterialManager : public iResourceManager
	{
	public:
		cMaterialManager(cGraphics* apGraphics,cResources *apResources);
		~cMaterialManager();

		iResourceBase* Create(const tString& asName);
		iMaterial* CreateMaterial(const tString& asName);

		void Update(float afTimeStep);

		void Destroy(iResourceBase* apResource);
		void Unload(iResourceBase* apResource);

		void SetTextureSizeLevel(unsigned int alLevel){ mlTextureSizeLevel = alLevel;}
		int GetTextureSizeLevel(){ return mlTextureSizeLevel;}

		void SetTextureFilter(eTextureFilter aFilter);
		eTextureFilter GetTextureFilter(){ return mTextureFilter;}

		void SetTextureAnisotropy(float afX);
		float GetTextureAnisotropy(){ return mfTextureAnisotropy;}

		tString GetPhysicsMaterialName(const tString& asName);

	private:
		iMaterial* LoadFromFile(const tString& asName,const tString& asPath);

		eTextureTarget GetTarget(const tString& asType);
		tString GetTextureString(eMaterialTexture aType);
		eTextureWrap GetWrap(const tString& asType);
		eTextureAnimMode GetAnimMode(const tString& asType);

		unsigned int mlTextureSizeLevel;
		eTextureFilter mTextureFilter;
		float mfTextureAnisotropy;

		tStringList mlstFileFormats;

		tStringVec mvCubeSideSuffixes;

		cGraphics* mpGraphics;
		cResources* mpResources;

		int mlIdCounter;
	};

};
#endif // HPL_MATERIAL_MANAGER_H
