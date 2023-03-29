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

#ifndef HPL_BEAM_H
#define HPL_BEAM_H

#include "RenderList.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Renderable.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/scene/Entity3D.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cMaterialManager;
class cResources;
class cGraphics;
class cFileSearcher;
class iLowLevelGraphics;
class iMaterial;
class iVertexBuffer;

//------------------------------------------

class cBeam;
class cBeamEnd : public iEntity3D {
	friend class cBeam;
	friend class cBeamEnd_UpdateCallback;

public:
	cBeamEnd(const tString asName, cBeam *apBeam) : iEntity3D(asName),
													mColor(1, 1), mpBeam(apBeam) {}

	void SetColor(const cColor &aColor);
	const cColor &GetColor() { return mColor; }

	/////////////////////////////////
	// Entity implementation
	tString GetEntityType() { return "BeamEnd"; }

private:
	cColor mColor;
	cBeam *mpBeam;
};

//------------------------------------------

class cBeamEnd_UpdateCallback : public iEntityCallback {
public:
	void OnTransformUpdate(iEntity3D *apEntity);
};

//------------------------------------------

class cBeam : public iRenderable {
	typedef iRenderable super;
	friend class cBeamEnd;

public:
	cBeam(const tString asName, cResources *apResources, cGraphics *apGraphics);
	~cBeam();

	void SetMaterial(iMaterial *apMaterial);

	const tString &GetFileName() { return msFileName; }

	/**
	 * Set the size. X= the thickness of the line, width of texture used. Y = the length that one texture height takes.
	 * \param avSize
	 */
	void SetSize(const cVector2f &avSize);
	cVector2f GetSize() { return mvSize; }

	void SetColor(const cColor &aColor);
	const cColor &GetColor() { return mColor; }

	void SetTileHeight(bool abX);
	bool GetTileHeight() { return mbTileHeight; }

	void SetMultiplyAlphaWithColor(bool abX);
	bool GetMultiplyAlphaWithColor() { return mbMultiplyAlphaWithColor; }

	cBeamEnd *GetEnd() { return mpEnd; }

	bool LoadXMLProperties(const tString asFile);

	cVector3f GetAxis() { return mvAxis; }
	cVector3f GetMidPosition() { return mvMidPosition; }

	/////////////////////////////////
	// Entity implementation
	tString GetEntityType() { return "Beam"; }

	bool IsVisible();
	void SetVisible(bool abVisible) { SetRendered(abVisible); }

	// Renderable implementations
	iMaterial *GetMaterial() { return mpMaterial; }
	iVertexBuffer *GetVertexBuffer() { return mpVtxBuffer; }

	void UpdateGraphics(cCamera3D *apCamera, float afFrameTime, cRenderList *apRenderList);

	bool IsShadowCaster() { return false; }

	cBoundingVolume *GetBoundingVolume();

	cMatrixf *GetModelMatrix(cCamera3D *apCamera);

	int GetMatrixUpdateCount();

	eRenderableType GetRenderType() { return eRenderableType_ParticleSystem; }

private:
	cMaterialManager *mpMaterialManager;
	cFileSearcher *mpFileSearcher;
	iLowLevelGraphics *mpLowLevelGraphics;

	iMaterial *mpMaterial;
	iVertexBuffer *mpVtxBuffer;

	cBeamEnd *mpEnd;

	tString msFileName;

	int mlStartTransformCount;
	int mlEndTransformCount;

	cMatrixf m_mtxTempTransform;

	int mlLastRenderCount;

	cBeamEnd_UpdateCallback mEndCallback;

	cVector2f mvSize;

	cVector3f mvAxis;
	cVector3f mvMidPosition;

	bool mbTileHeight;
	bool mbMultiplyAlphaWithColor;

	cColor mColor;
};

} // namespace hpl

#endif // HPL_BEAM_H
