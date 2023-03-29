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

#ifndef HPL_IMAGE_ENTITY_H
#define HPL_IMAGE_ENTITY_H

#include "common/array.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/scene/Entity2D.h"

class TiXmlElement;

namespace hpl {

class cResources;
class cGraphics;
class cImageEntityData;
class cImageAnimation;

class cImageEntity : public iEntity2D {
public:
	cImageEntity(tString asName, cResources *apResources, cGraphics *apGraphics, bool abAutoDeleteData = true);
	~cImageEntity();

	const cRect2f &GetBoundingBox();
	bool UpdateBoundingBox();
	tString GetEntityType() { return "ImageEntity"; }

	void Render();

	void UpdateLogic(float afTimeStep);

	bool LoadData(TiXmlElement *apRootElem);

	bool LoadEntityData(tString asDataName, int alAnimNum = 0);
	bool LoadEntityData(cImageEntityData *apData, int alAnimNum = 0);

	cImageEntityData *GetEntityData() { return mpEntityData; }

	///// Animation ////////////////////////

	bool SetAnimation(const tString &asName, bool abLoop = true);
	const tString &GetCurrentAnimation() const;
	bool AnimationIsLooped() { return mbLoopAnimation; }
	bool AnimationIsPlaying();
	void StopAnimation();

	void SetAnimationSpeed(float afSpeed) { mfAnimSpeed = afSpeed; }
	float GetAnimationSpeed() { return mfAnimSpeed; }

	void SetFrameNum(float afFrame) { mfFrameNum = afFrame; }
	float GetFrameNum() { return mfFrameNum; }
	int GetMaxFrameNum();

	void SetAnimationPaused(bool abX) { mbAnimationPaused = abX; }
	bool GetAnimationPaused() { return mbAnimationPaused; }

	void SetFlipH(bool abX);
	bool GetFlipH() { return mbFlipH; }
	void SetFlipV(bool abX);
	bool GetFlipV() { return mbFlipV; }

	cVector2f GetSize() { return mvSize; }
	void SetSize(const cVector2f &avSize);

	void SetAlpha(float afX);
	float GetAlpha() { return mfAlpha; }

	void Flash(float afAdd);

	bool IsCollidable() { return mbCollidable; }

private:
	cResources *mpResources;
	cGraphics *mpGraphics;
	cImageEntityData *mpEntityData;

	float mfAlpha;
	float mfFlashAdd;
	bool mbFlashing;

	bool mbCollidable;

	bool mbAutoDeleteData;

	cVector2f mvSize;
	bool mbFlipH;
	bool mbFlipV;

	bool mbRotationHasChanged;
	bool mbSizeHasChanged;
	float mfCurrentAngle;

	cImageAnimation *mpAnimation;
	float mfFrameNum;
	float mfAnimSpeed;
	int mlFrame;
	int mlLastFrame;
	bool mbAnimationPaused;
	bool mbLoopAnimation;

	tVertexVec mvBaseVtx;
	tVertexVec mvVtx;
	tUIntVec mvIdxVec;

	cVector3f mvTransform;

	tString msTempString;
};

typedef Common::Array<cImageEntity *> tImageEntityVec;
typedef tImageEntityVec::iterator tImageEntityVecIt;

} // namespace hpl

#endif // HPL_IMAGE_ENTITY_H
