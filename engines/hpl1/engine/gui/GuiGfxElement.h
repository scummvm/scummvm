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

#ifndef HPL_GUI_GFX_ELEMENT_H
#define HPL_GUI_GFX_ELEMENT_H

#include "hpl1/engine/gui/GuiTypes.h"

namespace hpl {

class iGuiMaterial;
class iTexture;
class cResourceImage;

class cGui;

#define kMaxGuiTextures (4)

//------------------------------------------

enum eGuiGfxAnimationType {
	eGuiGfxAnimationType_Loop,
	eGuiGfxAnimationType_StopAtEnd,
	eGuiGfxAnimationType_Oscillate,
	eGuiGfxAnimationType_Random,
	eGuiGfxAnimationType_LastEnum
};

class cGuiGfxAnimation {
	friend class cGuiGfxElement;

public:
	cGuiGfxAnimation() : mfFrameLength(1), mType(eGuiGfxAnimationType_Loop) {}

	void AddFrame(int alNum);
	void SetType(eGuiGfxAnimationType aType);
	void SetFrameLength(float afLength);

private:
	tString msName;
	Common::Array<int> mvFrames;
	float mfFrameLength;
	eGuiGfxAnimationType mType;
};

//------------------------------------------

class cGuiGfxElement {
	friend class cGuiSet;
	friend class cGuiRenderObjectCompare;

public:
	cGuiGfxElement(cGui *apGui);
	~cGuiGfxElement();

	void Update(float afTimeStep);

	void AddImage(cResourceImage *apImage);
	void AddTexture(iTexture *apTexture);

	void AddImageToBuffer(cResourceImage *apImage);

	void SetOffset(const cVector3f &avOffset) { mvOffset = avOffset; }
	const cVector3f &GetOffset() const { return mvOffset; }

	void SetActiveSize(const cVector2f &avSize) { mvActiveSize = avSize; }
	const cVector2f &GetActiveSize() { return mvActiveSize; }

	cGuiGfxAnimation *CreateAnimtion(const tString &asName);
	void PlayAnimation(int alNum);
	cGuiGfxAnimation *GetAnimation(int alIdx) { return mvAnimations[alIdx]; }
	void SetAnimationTime(float afTime);

	void SetAnimationPaused(bool abX) { mbAnimationPaused = abX; }
	bool GSetAnimationPaused() { return mbAnimationPaused; }

	void SetMaterial(iGuiMaterial *apMat);

	void SetColor(const cColor &aColor);

	cVector2f GetImageSize();

	void Flush();

private:
	void SetImage(cResourceImage *apImage, int alNum);

	cVector2f mvImageSize;
	cGui *mpGui;
	tVertexVec mvVtx;

	cVector3f mvOffset;
	cVector2f mvActiveSize;

	iGuiMaterial *mpMaterial;
	iTexture *mvTextures[kMaxGuiTextures];
	cResourceImage *mvImages[kMaxGuiTextures];

	Common::Array<cResourceImage *> mvImageBufferVec;

	Common::Array<cGuiGfxAnimation *> mvAnimations;
	int mlCurrentAnimation;
	float mfCurrentFrame;
	int mlActiveImage;
	bool mbForwardAnim;
	bool mbAnimationPaused;

	int mlTextureNum;

	bool mbFlushed;
};

} // namespace hpl

#endif // HPL_GUI_GFX_ELEMENT_H
