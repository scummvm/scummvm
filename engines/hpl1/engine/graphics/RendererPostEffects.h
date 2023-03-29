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

#ifndef HPL_RENDERER_POST_EFFECTS_H
#define HPL_RENDERER_POST_EFFECTS_H

#include "common/list.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/std/set.h"

namespace hpl {

#define kFilterProgramNum (1)

class iLowLevelGraphics;
class LowLevelResources;
class iGpuProgram;
class cResources;
class iTexture;
class cGpuProgramManager;
class cRenderList;
class cRenderer3D;

enum ePostEffectFilter {
	ePostEffectFilter_Offset,
	ePostEffectFilter_LastEnum
};

enum ePostEffectProgram {
	ePostEffectProgram_Offset,
	ePostEffectProgram_LastEnum
};

class cImageTrailEffect {
public:
	cImageTrailEffect() : mbActive(false), mbFirstPass(false), mlCurrentBuffer(0) {}

	bool mbActive;
	bool mbFirstPass;
	int mlCurrentBuffer;
	float mfAmount;
};

class cResources;

class cRendererPostEffects {
public:
	cRendererPostEffects(iLowLevelGraphics *apLowLevelGraphics, cResources *apResources,
						 cRenderList *apRenderList, cRenderer3D *apRenderer3D);
	~cRendererPostEffects();

	/**
	 * Render post effects, called by cScene
	 */
	void Render();

	void SetImageTrailActive(bool abX) {
		if (!mImageTrailData.mbActive && abX)
			mImageTrailData.mbFirstPass = true;
		mImageTrailData.mbActive = abX;
	}
	bool GetImageTrailActive() { return mImageTrailData.mbActive; }
	/**
	 * Set the amount of blur
	 * \param afAmount 0.0 - 1.0 are valid
	 */
	void SetImageTrailAmount(float afAmount) { mImageTrailData.mfAmount = afAmount; }

	iTexture *GetScreenBuffer(int alNum) { return mpScreenBuffer[alNum]; }

	void SetActive(bool abX) { mbActive = abX; }
	bool GetActive() { return mbActive; }

	void SetBloomActive(bool abX);
	bool GetBloomActive() { return mbBloomActive; }

	void SetBloomSpread(float afX) { mfBloomSpread = afX; }
	float GetBloomSpread() { return mfBloomSpread; }

	void SetMotionBlurActive(bool abX);
	bool GetMotionBlurActive() { return mbMotionBlurActive; }

	void SetMotionBlurAmount(float afX) { mfMotionBlurAmount = afX; }
	float GetMotionBlurAmount() { return mfMotionBlurAmount; }

	void SetDepthOfFieldActive(bool abX) { mbDofActive = abX; }
	void SetDepthOfFieldMaxBlur(float afX) { mfDofMaxBlur = afX; }
	void SetDepthOfFieldFocalPlane(float afX) { mfDofFocalPlane = afX; }
	void SetDepthOfFieldNearPlane(float afX) { mfDofNearPlane = afX; }
	void SetDepthOfFieldFarPlane(float afX) { mfDofFarPlane = afX; }

	bool GetDepthOfFieldActive() { return mbDofActive; }
	float GetDepthOfFieldMaxBlur() { return mfDofMaxBlur; }
	float GetDepthOfFieldFocalPlane() { return mfDofFocalPlane; }
	float GetDepthOfFieldNearPlane() { return mfDofNearPlane; }
	float GetDepthOfFieldFarPlane() { return mfDofFarPlane; }

	iTexture *GetFreeScreenTexture() { return mpScreenBuffer[mImageTrailData.mlCurrentBuffer == 0 ? 1 : 0]; }

	void RenderBlurTexture(iTexture *apDestination, iTexture *apSource, float afBlurAmount);

private:
	void RenderImageTrail();

	void RenderBloom();

	void RenderMotionBlur();

	void RenderDepthOfField();

	iLowLevelGraphics *mpLowLevelGraphics;
	LowLevelResources *mpLowLevelResources;
	cResources *mpResources;
	cGpuProgramManager *mpGpuManager;
	cRenderer3D *mpRenderer3D;

	cRenderList *mpRenderList;

	cVector2f mvScreenSize;

	iTexture *mpScreenBuffer[2];

	cImageTrailEffect mImageTrailData;

	iGpuProgram *_blur2DProgram;
	iGpuProgram *_blurRectProgram;
	bool mbBlurFallback;

	iGpuProgram *_bloomProgram;

	iTexture *mpBloomBlurTexture;

	iGpuProgram *_motionBlurProgram;

	iGpuProgram *_depthOfFieldProgram;
	iTexture *mpDofBlurTexture;

	tVertexVec mvTexRectVtx;

	bool mbBloomActive;
	float mfBloomSpread;

	bool mbMotionBlurActive;
	float mfMotionBlurAmount;
	bool mbMotionBlurFirstTime;

	bool mbDofActive;
	float mfDofMaxBlur;
	float mfDofFocalPlane;
	float mfDofNearPlane;
	float mfDofFarPlane;

	bool mbActive;
};

} // namespace hpl

#endif // HPL_RENDERER_POST_EFFECTS_H
