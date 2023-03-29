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

#ifndef HPL_GRAPHICSDRAWER_H
#define HPL_GRAPHICSDRAWER_H

#include "common/array.h"
#include "common/list.h"
#include "hpl1/engine/graphics/BackgroundImage.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/std/multiset.h"

namespace hpl {

class iLowLevelGraphics;
class cResourceImage;
class cGfxObject;
class cMaterialHandler;
class iMaterial;

class cGfxBufferObject {
public:
	cGfxObject *mpObject;
	cVector3f mvTransform;

	bool mbIsColorAndSize;
	cColor mColor;
	cVector2f mvSize;
	bool mbFlipH;
	bool mbFlipV;
	float mfAngle;

	iMaterial *GetMaterial() const;
	float GetZ() const { return mvTransform.z; }
};

typedef Common::Array<cGfxObject> tGfxObjectVec;
typedef tGfxObjectVec::iterator tGfxObjectVecIt;

class cGfxBufferCompare {
public:
	bool operator()(const cGfxBufferObject &aObjectA, const cGfxBufferObject &aObjectB) const;
};

typedef Hpl1::Std::multiset<cGfxBufferObject, cGfxBufferCompare> tGfxBufferSet;
typedef tGfxBufferSet::iterator tGfxBufferSetIt;

class cResources;

typedef Common::List<cGfxObject *> tGfxObjectList;
typedef tGfxObjectList::iterator tGfxObjectListIt;

class cGraphicsDrawer {
public:
	cGraphicsDrawer(iLowLevelGraphics *apLowLevelGraphics, cMaterialHandler *apMaterialHandler,
					cResources *apResources);
	~cGraphicsDrawer();

	/**
	 * Draw Gfx object during next DrawAll call.
	 * \param apObject
	 * \param avPos
	 */
	void DrawGfxObject(cGfxObject *apObject, const cVector3f &avPos);

	/**
	 * Draw Gfx object during next DrawAll call.
	 * \param apObject
	 * \param avPos
	 * \param avSize Size of object
	 * \param aColor color to use
	 * \param abFlipH Flip image horisontally
	 * \param abFlipV Flip image verically
	 */
	void DrawGfxObject(cGfxObject *apObject, const cVector3f &avPos,
					   const cVector2f &avSize, const cColor &aColor,
					   bool abFlipH = false, bool abFlipV = false, float afAngle = 0);

	/**
	 * Draw all gfx obejcts, Called after world is rendered by cScene.
	 */
	void DrawAll();

	/**
	 * Create Gfx object from file
	 * \param &asFileName Filename of image
	 * \param &asMaterialName material to use
	 * \param abAddToList if the engine should delete object at exit, this means DestroyGfxObject must be used. Should almost always be true.
	 * \return
	 */
	cGfxObject *CreateGfxObject(const tString &asFileName, const tString &asMaterialName,
								bool abAddToList = true);
	/**
	 * Create gfx object from Bitmap
	 * \param *apBmp bitmap
	 * \param &asMaterialName material to use
	 * \param abAddToList if the engine should delete object at exit, this means DestroyGfxObject must be used. Should almost always be true.
	 * \return
	 */
	cGfxObject *CreateGfxObject(Bitmap2D *apBmp, const tString &asMaterialName,
								bool abAddToList = true);

	cGfxObject *CreateGfxObjectFromTexture(const tString &asFileName, const tString &asMaterialName,
										   bool abAddToList = true);
	/**
	 * Destroys a gfx object.
	 */
	void DestroyGfxObject(cGfxObject *apObject);

	cBackgroundImage *AddBackgroundImage(const tString &asFileName, const tString &asMaterialName,
										 const cVector3f &avPos, bool abTile,
										 const cVector2f &avSize, const cVector2f &avPosPercent, const cVector2f &avVel);

	void UpdateBackgrounds();
	void DrawBackgrounds(const cRect2f &aCollideRect);
	void ClearBackgrounds();

private:
	iLowLevelGraphics *mpLowLevelGraphics;
	cMaterialHandler *mpMaterialHandler;
	cResources *mpResources;

	tGfxBufferSet m_setGfxBuffer;

	tGfxObjectList mlstGfxObjects;

	tBackgroundImageMap m_mapBackgroundImages;
};

} // namespace hpl

#endif // HPL_GRAPHICSDRAWER_H
