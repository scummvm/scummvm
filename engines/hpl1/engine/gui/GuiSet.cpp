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

#include "hpl1/engine/gui/GuiSet.h"

#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/font_data.h"

#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/FrameBitmap.h"
#include "hpl1/engine/resources/ImageManager.h"
#include "hpl1/engine/resources/ResourceImage.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/TextureManager.h"

#include "hpl1/engine/scene/Camera3D.h"
#include "hpl1/engine/scene/Scene.h"

#include "hpl1/engine/gui/Gui.h"
#include "hpl1/engine/gui/GuiGfxElement.h"
#include "hpl1/engine/gui/GuiMaterial.h"
#include "hpl1/engine/gui/GuiPopUp.h"
#include "hpl1/engine/gui/GuiSkin.h"
#include "hpl1/engine/gui/Widget.h"

#include "hpl1/engine/gui/GuiPopUpMessageBox.h"

#include "hpl1/engine/gui/WidgetButton.h"
#include "hpl1/engine/gui/WidgetCheckBox.h"
#include "hpl1/engine/gui/WidgetComboBox.h"
#include "hpl1/engine/gui/WidgetFrame.h"
#include "hpl1/engine/gui/WidgetImage.h"
#include "hpl1/engine/gui/WidgetLabel.h"
#include "hpl1/engine/gui/WidgetListBox.h"
#include "hpl1/engine/gui/WidgetSlider.h"
#include "hpl1/engine/gui/WidgetTextBox.h"
#include "hpl1/engine/gui/WidgetWindow.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// RENDER OBJECT
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cGuiRenderObjectCompare::operator()(const cGuiRenderObject &aObjectA,
										 const cGuiRenderObject &aObjectB) const {
	// Z
	float fZA = aObjectA.mvPos.z;
	float fZB = aObjectB.mvPos.z;
	if (fZA != fZB) {
		return fZA < fZB;
	}

	// Clip Region
	cGuiClipRegion *pClipA = aObjectA.mpClipRegion;
	cGuiClipRegion *pClipB = aObjectB.mpClipRegion;
	if (pClipA != pClipB) {
		return pClipA > pClipB;
	}

	// Material
	iGuiMaterial *pMaterialA = aObjectA.mpCustomMaterial ? aObjectA.mpCustomMaterial : aObjectA.mpGfx->mpMaterial;
	iGuiMaterial *pMaterialB = aObjectB.mpCustomMaterial ? aObjectB.mpCustomMaterial : aObjectB.mpGfx->mpMaterial;
	if (pMaterialA != pMaterialB) {
		return pMaterialA > pMaterialB;
	}

	// Texture
	iTexture *pTextureA = aObjectA.mpGfx->mvTextures[0];
	iTexture *pTextureB = aObjectB.mpGfx->mvTextures[0];
	if (pTextureA != pTextureB) {
		return pTextureA > pTextureB;
	}

	// Equal
	return false;
}

//////////////////////////////////////////////////////////////////////////
// CLIP REGION
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGuiClipRegion::~cGuiClipRegion() {
	Clear();
}

void cGuiClipRegion::Clear() {
	STLDeleteAll(mlstChildren);
}

cGuiClipRegion *cGuiClipRegion::CreateChild(const cVector3f &avPos, const cVector2f &avSize) {
	cGuiClipRegion *pRegion = hplNew(cGuiClipRegion, ());

	if (mRect.w < 0) {
		pRegion->mRect = cRect2f(cVector2f(avPos.x, avPos.y), avSize);
	} else {
		cRect2f temp = cRect2f(cVector2f(avPos.x, avPos.y), avSize);
		pRegion->mRect = cMath::ClipRect(temp, mRect);
		if (pRegion->mRect.w < 0)
			pRegion->mRect.w = 0;
		if (pRegion->mRect.h < 0)
			pRegion->mRect.h = 0;
	}

	mlstChildren.push_back(pRegion);

	return pRegion;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGuiSet::cGuiSet(const tString &asName, cGui *apGui, cGuiSkin *apSkin,
				 cResources *apResources, cGraphics *apGraphics,
				 cSound *apSound, cScene *apScene) {
	mpGui = apGui;
	mpSkin = NULL;

	msName = asName;

	mpResources = apResources;
	mpGraphics = apGraphics;
	mpSound = apSound;
	mpScene = apScene;

	mpGfxCurrentPointer = NULL;

	mpFocusedWidget = NULL;

	mpAttentionWidget = NULL;

	mvDrawOffset = 0;

	mvVirtualSize = mpGraphics->GetLowLevel()->GetVirtualSize();
	mfVirtualMinZ = -1000;
	mfVirtualMaxZ = 1000;

	mbActive = true;
	mbDrawMouse = true;
	mfMouseZ = 20;

	mbIs3D = false;
	mv3DSize = 1;
	m_mtx3DTransform = cMatrixf::Identity;
	mbCullBackface = false;

	mpWidgetRoot = hplNew(iWidget, (eWidgetType_Root, this, mpSkin));
	mpWidgetRoot->AddCallback(eGuiMessage_OnDraw, this, kGuiCallback(DrawMouse));

	mpCurrentClipRegion = &mBaseClipRegion;

	mbDestroyingSet = false;

	mlDrawPrio = 0;

	for (int i = 0; i < 3; ++i)
		mvMouseDown[i] = false;

	SetSkin(apSkin);
}

//-----------------------------------------------------------------------

cGuiSet::~cGuiSet() {
	mbDestroyingSet = true;

	STLDeleteAll(mlstPopUps);
	STLDeleteAll(mlstWidgets);
	hplDelete(mpWidgetRoot);

	mbDestroyingSet = false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGuiSet::Update(float afTimeStep) {
	/////////////////////////////
	// Popups
	if (mlstPopUps.empty() == false) {
		STLDeleteAll(mlstPopUps);
	}

	/////////////////////////////
	// Update widgets
	tWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		iWidget *pWidget = *it;
		pWidget->Update(afTimeStep);
	}
}

//-----------------------------------------------------------------------

void cGuiSet::DrawAll(float afTimeStep) {
	if (mbActive == false)
		return;

	///////////////////////////////
	// Draw all widgets
	SetCurrentClipRegion(&mBaseClipRegion);
	mpWidgetRoot->Draw(afTimeStep, &mBaseClipRegion);

	SetCurrentClipRegion(&mBaseClipRegion);
}

//-----------------------------------------------------------------------

bool cGuiSet::SendMessage(eGuiMessage aMessage, cGuiMessageData &aData) {
	switch (aMessage) {
	case eGuiMessage_MouseMove:
		return OnMouseMove(aData);
	case eGuiMessage_MouseDown:
		return OnMouseDown(aData);
	case eGuiMessage_MouseUp:
		return OnMouseUp(aData);
	case eGuiMessage_MouseDoubleClick:
		return OnMouseDoubleClick(aData);

	case eGuiMessage_KeyPress:
		return OnKeyPress(aData);
	default:
		break;
	}

	return false;
}

//-----------------------------------------------------------------------

// TODO: Support multi textures
void cGuiSet::Render() {
	iLowLevelGraphics *pLowLevelGraphics = mpGraphics->GetLowLevel();

	///////////////////////////////////
	// Init rendering

	// 3D projection
	if (mbIs3D) {
		cCamera3D *pCam = static_cast<cCamera3D *>(mpScene->GetCamera());

		pLowLevelGraphics->SetDepthTestActive(true);

		// Invert the y coordinate: = -y, this also get the gui into the correct position.
		// Also scale to size
		cMatrixf mtxPreMul = cMath::MatrixScale(cVector3f(mv3DSize.x / mvVirtualSize.x,
														  -mv3DSize.y / mvVirtualSize.y,
														  mv3DSize.z / (mfVirtualMaxZ - mfVirtualMinZ)));

		// Create the final model matrix
		cMatrixf mtxModel = cMath::MatrixMul(m_mtx3DTransform, mtxPreMul);
		mtxModel = cMath::MatrixMul(pCam->GetViewMatrix(), mtxModel);

		pLowLevelGraphics->SetMatrix(eMatrix_ModelView, mtxModel);

		// No need for projection matrix, should be setup, right? :)

		pLowLevelGraphics->SetCullActive(mbCullBackface);
	}
	// Screen projection
	else {
		pLowLevelGraphics->SetDepthTestActive(false);
		pLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);

		pLowLevelGraphics->SetOrthoProjection(mvVirtualSize, mfVirtualMinZ, mfVirtualMaxZ);
	}

	///////////////////////////////
	// Render all clip regions

	RenderClipRegion();

	///////////////////////////////
	// Clear the render object set
	mBaseClipRegion.Clear();

	if (mbIs3D) {
		if (mbCullBackface == false)
			pLowLevelGraphics->SetCullActive(true);
	}
}

//-----------------------------------------------------------------------

void cGuiSet::DrawGfx(cGuiGfxElement *apGfx, const cVector3f &avPos, const cVector2f &avSize,
					  const cColor &aColor, eGuiMaterial aMaterial) {
	if (mpCurrentClipRegion == NULL)
		return;
	if (mpCurrentClipRegion->mRect.w == 0 || mpCurrentClipRegion->mRect.h == 0)
		return;

	cVector3f vAbsPos = avPos + apGfx->GetOffset() + mvDrawOffset;
	if (mpCurrentClipRegion->mRect.w > 0) {
		cRect2f gfxRect;
		gfxRect.x = vAbsPos.x;
		gfxRect.y = vAbsPos.y;
		if (avSize.x < 0) {
			gfxRect.w = apGfx->GetImageSize().x;
			gfxRect.h = apGfx->GetImageSize().y;
		} else {
			gfxRect.w = avSize.x;
			gfxRect.h = avSize.y;
		}

		if (cMath::BoxCollision(mpCurrentClipRegion->mRect, gfxRect) == false)
			return;
	}

	apGfx->Flush();

	cGuiRenderObject object;

	// Log("Clip: %f %f\n",mpCurrentClipRegion->mRect.w,mpCurrentClipRegion->mRect.h);

	object.mpGfx = apGfx;
	object.mpClipRegion = mpCurrentClipRegion;
	object.mvPos = vAbsPos;
	if (avSize.x < 0)
		object.mvSize = apGfx->GetImageSize();
	else
		object.mvSize = avSize;
	object.mColor = aColor;
	if (aMaterial != eGuiMaterial_LastEnum)
		object.mpCustomMaterial = mpGui->GetMaterial(aMaterial);
	else
		object.mpCustomMaterial = NULL;

	m_setRenderObjects.insert(object);
}

//-----------------------------------------------------------------------

void cGuiSet::DrawFont(const tWString &asText,
					   FontData *apFont, const cVector3f &avPos,
					   const cVector2f &avSize, const cColor &aColor,
					   eFontAlign aAlign, eGuiMaterial aMaterial) {
	int lCount = 0;
	// float lXAdd = 0;
	cVector3f vPos = avPos;

	if (aAlign == eFontAlign_Center) {
		vPos.x -= apFont->getLength(avSize, asText.c_str()) / 2;
	} else if (aAlign == eFontAlign_Right) {
		vPos.x -= apFont->getLength(avSize, asText.c_str());
	}

	while (asText[lCount] != 0) {
		wchar_t lGlyphNum = ((wchar_t)asText[lCount]);
		if (lGlyphNum < apFont->getFirstChar() ||
			lGlyphNum > apFont->getLastChar()) {
			lCount++;
			continue;
		}
		lGlyphNum -= apFont->getFirstChar();

		Glyph *pGlyph = apFont->getGlyph(lGlyphNum);
		if (pGlyph) {
			cVector2f vOffset(pGlyph->_offset * avSize);
			cVector2f vSize(pGlyph->_size * avSize); // *apFont->GetSizeRatio());

			DrawGfx(pGlyph->_guiGfx, vPos + vOffset, vSize, aColor, aMaterial);

			vPos.x += pGlyph->_advance * avSize.x;
		}
		lCount++;
	}
}

//-----------------------------------------------------------------------

cWidgetWindow *cGuiSet::CreateWidgetWindow(const cVector3f &avLocalPos,
										   const cVector2f &avSize,
										   const tWString &asText,
										   iWidget *apParent,
										   const tString &asName) {
	cWidgetWindow *pWindow = hplNew(cWidgetWindow, (this, mpSkin));
	pWindow->SetPosition(avLocalPos);
	pWindow->SetSize(avSize);
	pWindow->SetText(asText);
	pWindow->SetName(asName);
	AddWidget(pWindow, apParent);
	return pWindow;
}

cWidgetFrame *cGuiSet::CreateWidgetFrame(const cVector3f &avLocalPos,
										 const cVector2f &avSize,
										 bool abDrawFrame,
										 iWidget *apParent,
										 const tString &asName) {
	cWidgetFrame *pFrame = hplNew(cWidgetFrame, (this, mpSkin));
	pFrame->SetPosition(avLocalPos);
	pFrame->SetSize(avSize);
	pFrame->SetDrawFrame(abDrawFrame);
	pFrame->SetName(asName);
	AddWidget(pFrame, apParent);
	return pFrame;
}

cWidgetButton *cGuiSet::CreateWidgetButton(const cVector3f &avLocalPos,
										   const cVector2f &avSize,
										   const tWString &asText,
										   iWidget *apParent,
										   const tString &asName) {
	cWidgetButton *pButton = hplNew(cWidgetButton, (this, mpSkin));
	pButton->SetPosition(avLocalPos);
	pButton->SetSize(avSize);
	pButton->SetText(asText);
	pButton->SetName(asName);
	AddWidget(pButton, apParent);
	return pButton;
}

cWidgetLabel *cGuiSet::CreateWidgetLabel(const cVector3f &avLocalPos,
										 const cVector2f &avSize,
										 const tWString &asText,
										 iWidget *apParent,
										 const tString &asName) {
	cWidgetLabel *pLabel = hplNew(cWidgetLabel, (this, mpSkin));
	pLabel->SetPosition(avLocalPos);
	pLabel->SetSize(avSize);
	pLabel->SetText(asText);
	pLabel->SetName(asName);
	AddWidget(pLabel, apParent);
	return pLabel;
}

cWidgetSlider *cGuiSet::CreateWidgetSlider(eWidgetSliderOrientation aOrientation,
										   const cVector3f &avLocalPos,
										   const cVector2f &avSize,
										   int alMaxValue,
										   iWidget *apParent,
										   const tString &asName) {
	cWidgetSlider *pSlider = hplNew(cWidgetSlider, (this, mpSkin, aOrientation));
	pSlider->SetPosition(avLocalPos);
	pSlider->SetSize(avSize);
	pSlider->SetMaxValue(alMaxValue);
	pSlider->SetName(asName);
	AddWidget(pSlider, apParent);
	return pSlider;
}

cWidgetTextBox *cGuiSet::CreateWidgetTextBox(const cVector3f &avLocalPos,
											 const cVector2f &avSize,
											 const tWString &asText,
											 iWidget *apParent,
											 const tString &asName) {
	cWidgetTextBox *pTextBox = hplNew(cWidgetTextBox, (this, mpSkin));
	pTextBox->SetPosition(avLocalPos);
	pTextBox->SetSize(avSize);
	pTextBox->SetText(asText);
	pTextBox->SetName(asName);
	AddWidget(pTextBox, apParent);
	return pTextBox;
}

cWidgetCheckBox *cGuiSet::CreateWidgetCheckBox(const cVector3f &avLocalPos,
											   const cVector2f &avSize,
											   const tWString &asText,
											   iWidget *apParent,
											   const tString &asName) {
	cWidgetCheckBox *pCheckBox = hplNew(cWidgetCheckBox, (this, mpSkin));
	pCheckBox->SetPosition(avLocalPos);
	pCheckBox->SetSize(avSize);
	pCheckBox->SetText(asText);
	pCheckBox->SetName(asName);
	AddWidget(pCheckBox, apParent);
	return pCheckBox;
}

cWidgetImage *cGuiSet::CreateWidgetImage(const tString &asFile,
										 const cVector3f &avLocalPos,
										 const cVector2f &avSize,
										 eGuiMaterial aMaterial,
										 bool abAnimate,
										 iWidget *apParent,
										 const tString &asName) {
	cWidgetImage *pImage = hplNew(cWidgetImage, (this, mpSkin));

	cGuiGfxElement *pGfx = NULL;
	if (asFile != "") {
		if (abAnimate) {
			pGfx = mpGui->CreateGfxImageBuffer(asFile, aMaterial, true);
		} else {
			pGfx = mpGui->CreateGfxImage(asFile, aMaterial);
		}
	}
	pImage->SetPosition(avLocalPos);

	if (pGfx && avSize.x < 0) {
		pImage->SetSize(pGfx->GetImageSize());
	} else {
		pImage->SetSize(avSize);
	}

	pImage->SetImage(pGfx);

	pImage->SetName(asName);

	AddWidget(pImage, apParent);
	return pImage;
}

cWidgetListBox *cGuiSet::CreateWidgetListBox(const cVector3f &avLocalPos,
											 const cVector2f &avSize,
											 iWidget *apParent,
											 const tString &asName) {
	cWidgetListBox *pListBox = hplNew(cWidgetListBox, (this, mpSkin));
	pListBox->SetPosition(avLocalPos);
	pListBox->SetSize(avSize);
	pListBox->SetName(asName);
	AddWidget(pListBox, apParent);
	return pListBox;
}

cWidgetComboBox *cGuiSet::CreateWidgetComboBox(const cVector3f &avLocalPos,
											   const cVector2f &avSize,
											   const tWString &asText,
											   iWidget *apParent,
											   const tString &asName) {
	cWidgetComboBox *pComboBox = hplNew(cWidgetComboBox, (this, mpSkin));
	pComboBox->SetPosition(avLocalPos);
	pComboBox->SetSize(avSize);
	pComboBox->SetText(asText);
	pComboBox->SetName(asName);
	AddWidget(pComboBox, apParent);
	return pComboBox;
}
//-----------------------------------------------------------------------

iWidget *cGuiSet::GetWidgetFromName(const tString &asName) {
	return (iWidget *)STLFindByName(mlstWidgets, asName);
}

//-----------------------------------------------------------------------

void cGuiSet::DestroyWidget(iWidget *apWidget) {
	if (apWidget == mpFocusedWidget)
		mpFocusedWidget = NULL;
	STLFindAndDelete(mlstWidgets, apWidget);
}

//-----------------------------------------------------------------------

void cGuiSet::CreatePopUpMessageBox(const tWString &asLabel, const tWString &asText,
									const tWString &asButton1, const tWString &asButton2,
									void *apCallbackObject, tGuiCallbackFunc apCallback) {
	/* cGuiPopUpMessageBox *pMessageBox = */ (void)hplNew(cGuiPopUpMessageBox, (this, asLabel, asText,
																				asButton1, asButton2,
																				apCallbackObject, apCallback));
}

//-----------------------------------------------------------------------

void cGuiSet::DestroyPopUp(iGuiPopUp *apPopUp) {
	mlstPopUps.push_back(apPopUp);
}

//-----------------------------------------------------------------------

void cGuiSet::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;
}

//-----------------------------------------------------------------------

void cGuiSet::SetDrawMouse(bool abX) {
	if (mbDrawMouse == abX)
		return;

	mbDrawMouse = abX;
}

//-----------------------------------------------------------------------

void cGuiSet::SetRootWidgetClips(bool abX) {
	mpWidgetRoot->SetClipActive(abX);
	if (abX)
		mpWidgetRoot->SetSize(mvVirtualSize);
	else
		mpWidgetRoot->SetSize(0);
}

bool cGuiSet::GetRootWidgetClips() {
	return mpWidgetRoot->GetClipActive();
}

//-----------------------------------------------------------------------

void cGuiSet::SetVirtualSize(const cVector2f &avSize, float afMinZ, float afMaxZ) {
	mvVirtualSize = avSize;
	mfVirtualMinZ = afMinZ;
	mfVirtualMaxZ = afMaxZ;
}

//-----------------------------------------------------------------------

void cGuiSet::SetFocusedWidget(iWidget *apWidget) {
	if (mpFocusedWidget) {
		cGuiMessageData data = cGuiMessageData(mvMousePos, 0);
		mpFocusedWidget->ProcessMessage(eGuiMessage_LostFocus, data);
	}

	mpFocusedWidget = apWidget;
	if (mpFocusedWidget) {
		cGuiMessageData data = cGuiMessageData(mvMousePos, 0);
		mpFocusedWidget->ProcessMessage(eGuiMessage_GotFocus, data);
	}
}

//-----------------------------------------------------------------------

void cGuiSet::SetAttentionWidget(iWidget *apWidget) {
	if (mpAttentionWidget == apWidget)
		return;

	mpAttentionWidget = apWidget;

	// Log("Sett attn: %d\n",mpAttentionWidget);

	if (mpFocusedWidget && mpFocusedWidget->IsConnectedTo(mpAttentionWidget) == false) {
		// Log("Lost focus %d\n",mpFocusedWidget);
		cGuiMessageData data = cGuiMessageData(mvMousePos, 0);
		mpFocusedWidget->ProcessMessage(eGuiMessage_LostFocus, data);
		mpFocusedWidget = NULL;
	}

	if (mpAttentionWidget && mpFocusedWidget == NULL) {
		// Log("Got focus %d\n",apWidget);
		mpFocusedWidget = apWidget;
		if (mpFocusedWidget) {
			cGuiMessageData data = cGuiMessageData(mvMousePos, 0);
			mpFocusedWidget->ProcessMessage(eGuiMessage_GotFocus, data);
		}
	}
}

//-----------------------------------------------------------------------

void cGuiSet::SetIs3D(bool abX) {
	mbIs3D = abX;
}

void cGuiSet::Set3DSize(const cVector3f &avSize) {
	mv3DSize = avSize;
}

void cGuiSet::Set3DTransform(const cMatrixf &a_mtxTransform) {
	m_mtx3DTransform = a_mtxTransform;
}

//-----------------------------------------------------------------------

void cGuiSet::SetCurrentPointer(cGuiGfxElement *apGfx) {
	mpGfxCurrentPointer = apGfx;
}

//-----------------------------------------------------------------------

bool cGuiSet::HasFocus() {
	return mpGui->GetFocusedSet() == this;
}

//-----------------------------------------------------------------------

void cGuiSet::SetSkin(cGuiSkin *apSkin) {
	// if(mpSkin == apSkin) return; Remove til there is a real skin

	mpSkin = apSkin;

	if (mpSkin) {
		mpGfxCurrentPointer = mpSkin->GetGfx(eGuiSkinGfx_PointerNormal);
	} else {
		mpGfxCurrentPointer = NULL;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////
#define kLogRender (false)

static void SetClipArea(iLowLevelGraphics *pLowLevelGraphics, cGuiClipRegion *apRegion) {
	cRect2f &clipRect = apRegion->mRect;

	//////////////////////////////////
	// Set up clip area
	if (apRegion->mRect.w > 0) {
		cPlanef plane;

		cVector2f vVirtualSize = pLowLevelGraphics->GetVirtualSize();
		cVector2f vScreenSize = pLowLevelGraphics->GetScreenSize();

		cRect2l vScissorRect((int)((clipRect.x / vVirtualSize.x) * vScreenSize.x), (int)((clipRect.y / vVirtualSize.y) * vScreenSize.y),
							 (int)((clipRect.w / vVirtualSize.x) * vScreenSize.x), (int)((clipRect.h / vVirtualSize.y) * vScreenSize.y));

		pLowLevelGraphics->SetScissorActive(true);
		pLowLevelGraphics->SetScissorRect(vScissorRect);

		// Bottom
		/*plane.FromNormalPoint(cVector3f(0,-1,0),cVector3f(0,clipRect.y+clipRect.h,0));
		pLowLevelGraphics->SetClipPlane(0, plane);
		pLowLevelGraphics->SetClipPlaneActive(0, true);

		//Top
		plane.FromNormalPoint(cVector3f(0,1,0),cVector3f(0,clipRect.y,0));
		pLowLevelGraphics->SetClipPlane(1, plane);
		pLowLevelGraphics->SetClipPlaneActive(1, true);

		//Right
		plane.FromNormalPoint(cVector3f(1,0,0),cVector3f(clipRect.x,0,0));
		pLowLevelGraphics->SetClipPlane(2, plane);
		pLowLevelGraphics->SetClipPlaneActive(2, true);

		//Left
		plane.FromNormalPoint(cVector3f(-1,0,0),cVector3f(clipRect.x+clipRect.w,0,0));
		pLowLevelGraphics->SetClipPlane(3, plane);
		pLowLevelGraphics->SetClipPlaneActive(3, true);*/

		if (kLogRender)
			Log("-- Clip region: %d Clipping: x %f y %f w %f h %f\n", apRegion,
				apRegion->mRect.x, apRegion->mRect.y,
				apRegion->mRect.w, apRegion->mRect.h);
	} else {
		if (kLogRender)
			Log("-- Clip region: %d No clipping!\n", apRegion);
	}
}

//-----------------------------------------------------------------------

void cGuiSet::RenderClipRegion() {
	iLowLevelGraphics *pLowLevelGraphics = mpGraphics->GetLowLevel();

	if (kLogRender)
		Log("-------------------\n");

	///////////////////////////////////////
	// See if there is anything to draw
	tGuiRenderObjectSet &setRenderObjects = m_setRenderObjects;
	if (setRenderObjects.empty()) {
		if (kLogRender)
			Log("------------------------\n");
		return;
	}

	//////////////////////////////////
	// Graphics setup
	pLowLevelGraphics->SetTexture(0, NULL);

	//////////////////////////////////
	// Set up variables

	tGuiRenderObjectSetIt it = setRenderObjects.begin();

	iGuiMaterial *pLastMaterial = NULL;
	iTexture *pLastTexture = NULL;
	cGuiClipRegion *pLastClipRegion = NULL;

	cGuiGfxElement *pGfx = it->mpGfx;
	iGuiMaterial *pMaterial = it->mpCustomMaterial ? it->mpCustomMaterial : pGfx->mpMaterial;
	iTexture *pTexture = pGfx->mvTextures[0];
	cGuiClipRegion *pClipRegion = it->mpClipRegion;

	int lIdxAdd = 0;

	///////////////////////////////////
	// Iterate objects
	while (it != setRenderObjects.end()) {
		///////////////////////////////
		// Start rendering
		if (pLastMaterial != pMaterial) {
			pMaterial->BeforeRender();
			if (kLogRender)
				Log("Material %s before\n", pMaterial->GetName().c_str());
		}

		////////////////////////////
		// SetClip area
		if (pLastClipRegion != pClipRegion) {
			SetClipArea(pLowLevelGraphics, pClipRegion);
		}

		pLowLevelGraphics->SetTexture(0, pTexture);
		if (kLogRender)
			Log("Texture %d\n", pTexture);

		//////////////////////////
		// Iterate for all with same texture and material
		do {
			cGuiRenderObject object = *it;
			pGfx = object.mpGfx;

			if (kLogRender) {
				if (pGfx->mvImages[0])
					Log(" gfx: %d '%s'\n", pGfx, pGfx->mvImages[0]->GetName().c_str());
				else
					Log(" gfx: %d 'null'\n");
			}

			///////////////////////////
			// Add object to batch
			for (int i = 0; i < 4; ++i) {
				cVertex &vtx = pGfx->mvVtx[i];
				cVector3f &vVtxPos = vtx.pos;
				cVector3f &vPos = object.mvPos;
				pLowLevelGraphics->AddVertexToBatch_Raw(
					cVector3f(vVtxPos.x * object.mvSize.x + vPos.x,
							  vVtxPos.y * object.mvSize.y + vPos.y,
							  vPos.z),
					vtx.col * object.mColor,
					vtx.tex);
			}

			for (int i = 0; i < 4; i++)
				pLowLevelGraphics->AddIndexToBatch(lIdxAdd + i);

			lIdxAdd += 4;

			///////////////////////////
			// Set last texture
			pLastMaterial = pMaterial;
			pLastTexture = pTexture;
			pLastClipRegion = pClipRegion;

			/////////////////////////////
			// Get next object
			++it;
			if (it == setRenderObjects.end())
				break;

			pGfx = it->mpGfx;
			pMaterial = it->mpCustomMaterial ? it->mpCustomMaterial : pGfx->mpMaterial;
			pTexture = it->mpGfx->mvTextures[0];
			pClipRegion = it->mpClipRegion;
		} while (pTexture == pLastTexture &&
				 pMaterial == pLastMaterial &&
				 pClipRegion == pLastClipRegion);

		//////////////////////////////
		// Render batch
		pLowLevelGraphics->FlushQuadBatch(eVtxBatchFlag_Position | eVtxBatchFlag_Texture0 |
											  eVtxBatchFlag_Color0,
										  false);
		pLowLevelGraphics->ClearBatch();
		lIdxAdd = 0;

		/////////////////////////////////
		// Clip region end
		if (pLastClipRegion != pClipRegion || it == setRenderObjects.end()) {
			if (pLastClipRegion->mRect.w > 0) {
				pLowLevelGraphics->SetScissorActive(false);
				// for(int i=0; i<4; ++i) pLowLevelGraphics->SetClipPlaneActive(i, false);
			}
		}

		/////////////////////////////////
		// Material end
		if (pLastMaterial != pMaterial || it == setRenderObjects.end()) {
			pLastMaterial->AfterRender();
			if (kLogRender)
				Log("Material %d '%s' after. new: %d '%s'\n", pLastMaterial, pLastMaterial->GetName().c_str(),
					pMaterial, pMaterial->GetName().c_str());
		}
	}

	///////////////////////////////
	// Clear render objects
	m_setRenderObjects.clear();

	if (kLogRender)
		Log("---------- END %d -----------\n");
}
//-----------------------------------------------------------------------

void cGuiSet::AddWidget(iWidget *apWidget, iWidget *apParent) {
	mlstWidgets.push_front(apWidget);

	if (apParent)
		apParent->AttachChild(apWidget);
	else
		mpWidgetRoot->AttachChild(apWidget);

	apWidget->Init();
}

//-----------------------------------------------------------------------

bool cGuiSet::OnMouseMove(cGuiMessageData &aData) {
	///////////////////////////
	// Set up variables
	mvMousePos = aData.mvPos;

	aData.mlVal = 0;
	if (mvMouseDown[0])
		aData.mlVal |= eGuiMouseButton_Left;
	if (mvMouseDown[1])
		aData.mlVal |= eGuiMouseButton_Middle;
	if (mvMouseDown[2])
		aData.mlVal |= eGuiMouseButton_Right;

	///////////////////////////
	// Call widgets
	bool bRet = false;
	bool bPointerSet = false;
	tWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		iWidget *pWidget = *it;

		if (pWidget->PointIsInside(mvMousePos, false)) {
			////////////////////////////
			// Mouse enter event
			if (pWidget->GetMouseIsOver() == false) {
				pWidget->SetMouseIsOver(true);
				if (pWidget->ProcessMessage(eGuiMessage_MouseEnter, aData)) {
					bRet = true;
				}
			}

			////////////////////////////
			// Set pointer
			if (bPointerSet == false && pWidget->GetPointerGfx()) {
				if (mpAttentionWidget &&
					pWidget->IsConnectedTo(mpAttentionWidget) == false) {
				} else {
					if (mpGfxCurrentPointer != pWidget->GetPointerGfx()) {
						if (pWidget->IsEnabled()) {
							SetCurrentPointer(pWidget->GetPointerGfx());
						}
					}
					bPointerSet = true;
				}
			}
		} else {
			////////////////////////////
			// Mouse leave event
			if (pWidget->GetMouseIsOver()) {
				pWidget->SetMouseIsOver(false);
				pWidget->ProcessMessage(eGuiMessage_MouseLeave, aData);

				// In case the widget is moved under the mouse again, check:
				if (mpFocusedWidget == pWidget && pWidget->PointIsInside(mvMousePos, false)) {
					pWidget->SetMouseIsOver(true);
					if (pWidget->ProcessMessage(eGuiMessage_MouseEnter, aData))
						bRet = true;
				}
			}
		}

		////////////////////////////
		// Mouse move event
		if (pWidget->GetMouseIsOver() || mpFocusedWidget == pWidget) {
			if (pWidget->ProcessMessage(eGuiMessage_MouseMove, aData))
				bRet = true;
		}
	}

	return bRet;
}

//-----------------------------------------------------------------------

bool cGuiSet::OnMouseDown(cGuiMessageData &aData) {
	///////////////////////////
	// Set up variables
	mvMouseDown[cMath::Log2ToInt(aData.mlVal)] = true;

	aData.mvPos = mvMousePos;

	iWidget *pOldFocus = mpFocusedWidget;

	///////////////////////////
	// Call widgets
	bool bRet = false;
	tWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		iWidget *pWidget = *it;

		// If these is an attention set, do send clicks to any other widgets
		if (mpAttentionWidget && pWidget->IsConnectedTo(mpAttentionWidget) == false) {
			continue;
		}

		if (pWidget->GetMouseIsOver()) {
			if (mpFocusedWidget != pWidget) {
				if (pWidget->ProcessMessage(eGuiMessage_GotFocus, aData)) {
					mpFocusedWidget = pWidget;
				}
			} else {
				mpFocusedWidget = pWidget;
			}

			// Log("Got focus %d\n",pWidget);

			if (pWidget->ProcessMessage(eGuiMessage_MouseDown, aData)) {
				bRet = true;
				break;
			}
		}
	}

	// Se if anything was clicked
	if (bRet == false) {
		mpFocusedWidget = NULL;
	}

	// Lost focus callback
	if (mpFocusedWidget != pOldFocus) {
		// Log("Lost focus %d\n",pOldFocus);
		if (pOldFocus)
			pOldFocus->ProcessMessage(eGuiMessage_LostFocus, aData);
	}

	return bRet;
}

//-----------------------------------------------------------------------

bool cGuiSet::OnMouseUp(cGuiMessageData &aData) {
	///////////////////////////
	// Set up variables
	mvMouseDown[cMath::Log2ToInt(aData.mlVal)] = false;

	aData.mvPos = mvMousePos;

	///////////////////////////
	// Call widgets
	bool bRet = false;

	if (mpFocusedWidget) {
		bRet = mpFocusedWidget->ProcessMessage(eGuiMessage_MouseUp, aData);
	}

	if (bRet == false) {
		tWidgetListIt it = mlstWidgets.begin();
		for (; it != mlstWidgets.end(); ++it) {
			iWidget *pWidget = *it;

			// If these is an attention set, do send clicks to any other widgets
			if (mpAttentionWidget && pWidget->IsConnectedTo(mpAttentionWidget) == false) {
				continue;
			}

			if (pWidget != mpFocusedWidget && pWidget->GetMouseIsOver()) {
				if (pWidget->ProcessMessage(eGuiMessage_MouseUp, aData)) {
					bRet = true;
					break;
				}
			}
		}
	}

	return bRet;
}

//-----------------------------------------------------------------------

bool cGuiSet::OnMouseDoubleClick(cGuiMessageData &aData) {
	///////////////////////////
	// Set up variables
	aData.mvPos = mvMousePos;

	///////////////////////////
	// Call widgets
	bool bRet = false;
	tWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		iWidget *pWidget = *it;

		// If these is an attention set, do send clicks to any other widgets
		if (mpAttentionWidget && pWidget->IsConnectedTo(mpAttentionWidget) == false) {
			continue;
		}

		if (pWidget->GetMouseIsOver()) {
			if (pWidget->ProcessMessage(eGuiMessage_MouseDoubleClick, aData)) {
				bRet = true;
				break;
			}
		}
	}

	return bRet;
}

//-----------------------------------------------------------------------

bool cGuiSet::OnKeyPress(cGuiMessageData &aData) {
	///////////////////////////
	// Set up variables
	aData.mvPos = mvMousePos;

	///////////////////////////
	// Call widgets
	bool bRet = false;

	if (mpFocusedWidget) {
		bRet = mpFocusedWidget->ProcessMessage(eGuiMessage_KeyPress, aData);
	}

	if (bRet == false) {
		tWidgetListIt it = mlstWidgets.begin();
		for (; it != mlstWidgets.end(); ++it) {
			iWidget *pWidget = *it;

			// If these is an attention set, do send clicks to any other widgets
			if (mpAttentionWidget && pWidget->IsConnectedTo(mpAttentionWidget) == false) {
				continue;
			}

			if (pWidget->GetMouseIsOver() && mpFocusedWidget != pWidget) {
				if (pWidget->ProcessMessage(eGuiMessage_KeyPress, aData)) {
					bRet = true;
					break;
				}
			}
		}
	}

	return bRet;
}

//-----------------------------------------------------------------------

bool cGuiSet::DrawMouse(iWidget *apWidget, cGuiMessageData &aData) {
	if (HasFocus() && mbDrawMouse && mpGfxCurrentPointer) {
		DrawGfx(mpGfxCurrentPointer, cVector3f(mvMousePos.x, mvMousePos.y, mfMouseZ),
				mpGfxCurrentPointer->GetImageSize(), cColor(1, 1));
	}

	return true;
}
kGuiCalllbackDeclaredFuncEnd(cGuiSet, DrawMouse)

//-----------------------------------------------------------------------

} // namespace hpl
