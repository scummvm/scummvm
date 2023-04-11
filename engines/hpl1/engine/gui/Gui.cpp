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

#include "hpl1/engine/gui/Gui.h"

#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/sound/Sound.h"

#include "hpl1/engine/gui/GuiGfxElement.h"
#include "hpl1/engine/gui/GuiSet.h"
#include "hpl1/engine/gui/GuiSkin.h"

#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/FrameBitmap.h"
#include "hpl1/engine/resources/ImageManager.h"
#include "hpl1/engine/resources/ResourceImage.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/TextureManager.h"

#include "hpl1/engine/gui/GuiMaterialBasicTypes.h"
#include "hpl1/std/multimap.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGui::cGui() : iUpdateable("HPL_Gui") {
	mpSetInFocus = NULL;

	mlLastRenderTime = 0;
}

//-----------------------------------------------------------------------

cGui::~cGui() {
	Log("Exiting Gui Module\n");
	Log("--------------------------------------------------------\n");

	STLMapDeleteAll(m_mapSets);
	STLMapDeleteAll(m_mapSkins);

	STLDeleteAll(mlstGfxElements);

	for (int i = 0; i < eGuiMaterial_LastEnum; ++i) {
		if (mvMaterials[i])
			hplDelete(mvMaterials[i]);
	}

	Log("--------------------------------------------------------\n\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGui::Init(cResources *apResources, cGraphics *apGraphics,
				cSound *apSound, cScene *apScene) {
	mpResources = apResources;
	mpGraphics = apGraphics;
	mpSound = apSound;
	mpScene = apScene;

	//////////////////////////////
	// Create materials
	for (int i = 0; i < eGuiMaterial_LastEnum; ++i)
		mvMaterials[i] = NULL;

	mvMaterials[eGuiMaterial_Diffuse] = hplNew(cGuiMaterial_Diffuse, (mpGraphics->GetLowLevel()));
	mvMaterials[eGuiMaterial_Alpha] = hplNew(cGuiMaterial_Alpha, (mpGraphics->GetLowLevel()));
	mvMaterials[eGuiMaterial_FontNormal] = hplNew(cGuiMaterial_FontNormal, (mpGraphics->GetLowLevel()));
	mvMaterials[eGuiMaterial_Additive] = hplNew(cGuiMaterial_Additive, (mpGraphics->GetLowLevel()));
	mvMaterials[eGuiMaterial_Modulative] = hplNew(cGuiMaterial_Modulative, (mpGraphics->GetLowLevel()));

	//////////////////////////////
	// Skin setup
	GenerateSkinTypeStrings();
}

//-----------------------------------------------------------------------

void cGui::Update(float afTimeStep) {
	////////////////////////////////////
	// Update Sets
	tGuiSetMapIt it = m_mapSets.begin();
	for (; it != m_mapSets.end(); ++it) {
		cGuiSet *pSet = it->second;
		pSet->Update(afTimeStep);
	}

	/////////////////////////////
	// Update gfx elements
	tGuiGfxElementListIt gfxIt = mlstGfxElements.begin();
	for (; gfxIt != mlstGfxElements.end(); ++gfxIt) {
		cGuiGfxElement *pGfx = *gfxIt;
		pGfx->Update(afTimeStep);
	}
}

//-----------------------------------------------------------------------

void cGui::OnPostSceneDraw() {
	///////////////////////////////////////
	// Calculate time since last render
	unsigned long lCurrentTime = GetApplicationTime();
	if (mlLastRenderTime > lCurrentTime)
		mlLastRenderTime = lCurrentTime;

	float fTimeStep = (float)(lCurrentTime - mlLastRenderTime) / 1000.0f;
	mlLastRenderTime = lCurrentTime;

	////////////////////////////////////
	// Draw all sets
	tGuiSetMapIt setIt = m_mapSets.begin();
	for (; setIt != m_mapSets.end(); ++setIt) {
		cGuiSet *pSet = setIt->second;
		pSet->DrawAll(fTimeStep);
	}

	////////////////////////////////////
	// Render sets that are project to world

	////////////////////////////////////
	// Render sets that are project to screen
	setIt = m_mapSets.begin();
	for (; setIt != m_mapSets.end(); ++setIt) {
		cGuiSet *pSet = setIt->second;
		if (pSet->Is3D())
			pSet->Render();
	}
}

//-----------------------------------------------------------------------

void cGui::OnPostGUIDraw() {
	typedef Hpl1::Std::multimap<int, cGuiSet *> tPrioMap;
	tPrioMap mapSortedSets;

	tGuiSetMapIt it = m_mapSets.begin();
	for (; it != m_mapSets.end(); ++it) {
		cGuiSet *pSet = it->second;

		mapSortedSets.insert(tPrioMap::value_type(pSet->GetDrawPriority(), pSet));
	}

	////////////////////////////////////
	// Render sets that are project to screen
	tPrioMap::iterator SortIt = mapSortedSets.begin();
	for (; SortIt != mapSortedSets.end(); ++SortIt) {
		cGuiSet *pSet = SortIt->second;
		if (pSet->Is3D() == false)
			pSet->Render();
	}
}

//-----------------------------------------------------------------------

iGuiMaterial *cGui::GetMaterial(eGuiMaterial aType) {
	return mvMaterials[aType];
}

//-----------------------------------------------------------------------

cGuiSkin *cGui::CreateSkin(const tString &asFile) {
	tString sName = cString::SetFileExt(asFile, "");
	cGuiSkin *pSkin = hplNew(cGuiSkin, (sName, this));

	tString sPath = mpResources->GetFileSearcher()->GetFilePath(asFile);

	if (pSkin->LoadFromFile(sPath) == false) {
		hplDelete(pSkin);
		return NULL;
	}

	m_mapSkins.insert(tGuiSkinMap::value_type(sName, pSkin));
	return pSkin;
}

//-----------------------------------------------------------------------

eGuiSkinGfx cGui::GetSkinGfxFromString(const tString &asType) {
	tGuiSkinGfxMapIt it = m_mapSkinGfxStrings.find(asType);
	if (it == m_mapSkinGfxStrings.end()) {
		Warning("Skin gfx type '%s' does not exist!\n", asType.c_str());
		return eGuiSkinGfx_LastEnum;
	}

	return it->second;
}

eGuiSkinFont cGui::GetSkinFontFromString(const tString &asType) {
	tGuiSkinFontMapIt it = m_mapSkinFontStrings.find(asType);
	if (it == m_mapSkinFontStrings.end()) {
		Warning("Skin Font type '%s' does not exist!\n", asType.c_str());
		return eGuiSkinFont_LastEnum;
	}

	return it->second;
}

eGuiSkinAttribute cGui::GetSkinAttributeFromString(const tString &asType) {
	tGuiSkinAttributeMapIt it = m_mapSkinAttributeStrings.find(asType);
	if (it == m_mapSkinAttributeStrings.end()) {
		Warning("Skin Attribute type '%s' does not exist!\n", asType.c_str());
		return eGuiSkinAttribute_LastEnum;
	}

	return it->second;
}

//-----------------------------------------------------------------------

cGuiSet *cGui::CreateSet(const tString &asName, cGuiSkin *apSkin) {
	cGuiSet *pSet = hplNew(cGuiSet, (asName, this, apSkin, mpResources, mpGraphics, mpSound, mpScene));

	m_mapSets.insert(tGuiSetMap::value_type(asName, pSet));

	return pSet;
}

cGuiSet *cGui::GetSetFromName(const tString &asName) {
	tGuiSetMapIt it = m_mapSets.find(asName);
	if (it == m_mapSets.end())
		return NULL;

	return it->second;
}

void cGui::SetFocus(cGuiSet *apSet) {
	if (mpSetInFocus == apSet)
		return;

	// TODO: Call lost focus or stuff

	mpSetInFocus = apSet;
}

void cGui::SetFocusByName(const tString &asSetName) {
	cGuiSet *pSet = GetSetFromName(asSetName);
	if (pSet)
		SetFocus(pSet);
}

void cGui::DestroySet(cGuiSet *apSet) {
	if (apSet == NULL)
		return;

	tGuiSetMapIt it = m_mapSets.begin();
	for (; it != m_mapSets.end();) {
		if (apSet == it->second) {
			hplDelete(apSet);
			m_mapSets.erase(it++);
			break;
		} else {
			++it;
		}
	}
}

//-----------------------------------------------------------------------

cGuiGfxElement *cGui::CreateGfxFilledRect(const cColor &aColor, eGuiMaterial aMaterial, bool abAddToList) {
	cGuiGfxElement *pGfxElem = hplNew(cGuiGfxElement, (this));

	pGfxElem->SetColor(aColor);
	pGfxElem->SetMaterial(GetMaterial(aMaterial));

	if (abAddToList)
		mlstGfxElements.push_back(pGfxElem);

	return pGfxElem;
}

//-----------------------------------------------------------------------

cGuiGfxElement *cGui::CreateGfxImage(const tString &asFile, eGuiMaterial aMaterial,
									 const cColor &aColor, bool abAddToList) {
	////////////////////////////
	// Load image
	cResourceImage *pImage = mpResources->GetImageManager()->CreateImage(asFile);
	if (pImage == NULL) {
		Error("Could not load image '%s'!\n", asFile.c_str());
		return NULL;
	}

	/////////////////////////////
	// Create element
	cGuiGfxElement *pGfxElem = hplNew(cGuiGfxElement, (this));

	pGfxElem->SetColor(aColor);
	pGfxElem->SetMaterial(GetMaterial(aMaterial));
	pGfxElem->AddImage(pImage);

	if (abAddToList)
		mlstGfxElements.push_back(pGfxElem);

	return pGfxElem;
}

//-----------------------------------------------------------------------

cGuiGfxElement *cGui::CreateGfxTexture(const tString &asFile, eGuiMaterial aMaterial,
									   const cColor &aColor, bool abMipMaps, bool abAddToList) {
	///////////////////
	// Load texture
	iTexture *pTexture = mpResources->GetTextureManager()->Create2D(asFile, abMipMaps, false);
	if (pTexture == NULL) {
		Error("Could not load texture '%s'!\n", asFile.c_str());
		return NULL;
	}

	/////////////////////////////
	// Create element
	cGuiGfxElement *pGfxElem = hplNew(cGuiGfxElement, (this));

	pGfxElem->SetColor(aColor);
	pGfxElem->SetMaterial(GetMaterial(aMaterial));
	pGfxElem->AddTexture(pTexture);

	if (abAddToList)
		mlstGfxElements.push_back(pGfxElem);

	return pGfxElem;
}

//-----------------------------------------------------------------------

cGuiGfxElement *cGui::CreateGfxImageBuffer(const tString &asFile, eGuiMaterial aMaterial,
										   bool abCreateAnimation,
										   const cColor &aColor, bool abAddToList) {
	////////////////////////////
	// Load images
	tString sName = cString::SetFileExt(asFile, "");
	tString sExt = cString::GetFileExt(asFile);

	Common::Array<cResourceImage *> vImages;

	int lFileNum = 0;
	while (true) {
		tString sNum = lFileNum <= 9 ? "0" + cString::ToString(lFileNum) : cString::ToString(lFileNum);
		tString sFile = sName + sNum + "." + sExt;

		if (mpResources->GetFileSearcher()->GetFilePath(sFile) == "")
			break;

		cResourceImage *pImage = mpResources->GetImageManager()->CreateImage(sFile);
		vImages.push_back(pImage);
		++lFileNum;
	}

	if (vImages.empty()) {
		Error("Could not load any images with '%s' as base!\n", asFile.c_str());
		return NULL;
	}

	/////////////////////////////
	// Create element
	cGuiGfxElement *pGfxElem = hplNew(cGuiGfxElement, (this));

	pGfxElem->SetColor(aColor);
	pGfxElem->SetMaterial(GetMaterial(aMaterial));

	for (size_t i = 0; i < vImages.size(); ++i) {
		if (i == 0)
			pGfxElem->AddImage(vImages[i]);
		pGfxElem->AddImageToBuffer(vImages[i]);
	}

	if (abAddToList)
		mlstGfxElements.push_back(pGfxElem);

	///////////////////////////////
	// Create animation
	if (abCreateAnimation) {
		cGuiGfxAnimation *pAnim = pGfxElem->CreateAnimtion("Default");
		for (size_t i = 0; i < vImages.size(); ++i) {
			pAnim->AddFrame((int)i);
		}
	}

	return pGfxElem;
}

//-----------------------------------------------------------------------

void cGui::DestroyGfx(cGuiGfxElement *apGfx) {
	STLFindAndDelete(mlstGfxElements, apGfx);
}

//-----------------------------------------------------------------------

bool cGui::SendMousePos(const cVector2f &avPos, const cVector2f &avRel) {
	if (mpSetInFocus == NULL)
		return false;

	cGuiMessageData data = cGuiMessageData(avPos, avRel);
	return mpSetInFocus->SendMessage(eGuiMessage_MouseMove, data);
}

bool cGui::SendMouseClickDown(eGuiMouseButton aButton) {
	if (mpSetInFocus == NULL)
		return false;

	cGuiMessageData data = cGuiMessageData(aButton);
	return mpSetInFocus->SendMessage(eGuiMessage_MouseDown, data);
}

bool cGui::SendMouseClickUp(eGuiMouseButton aButton) {
	if (mpSetInFocus == NULL)
		return false;

	cGuiMessageData data = cGuiMessageData(aButton);
	return mpSetInFocus->SendMessage(eGuiMessage_MouseUp, data);
}

bool cGui::SendMouseDoubleClick(eGuiMouseButton aButton) {
	if (mpSetInFocus == NULL)
		return false;

	cGuiMessageData data = cGuiMessageData(aButton);
	return mpSetInFocus->SendMessage(eGuiMessage_MouseDoubleClick, data);
}

bool cGui::SendKeyPress(Common::KeyState keyPress) {
	if (mpSetInFocus == NULL)
		return false;

	cGuiMessageData data = cGuiMessageData(keyPress);
	return mpSetInFocus->SendMessage(eGuiMessage_KeyPress, data);
}

//-----------------------------------------------------------------------

/*bool cGui::SentArrowKey(eGuiArrowKey aDir)
{
	if(mpSetInFocus==NULL)return false;

	return false;
}*/

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

#define AddFont(name) m_mapSkinFontStrings.insert(tGuiSkinFontMap::value_type(#name, eGuiSkinFont_##name))
#define AddAttribute(name) m_mapSkinAttributeStrings.insert(tGuiSkinAttributeMap::value_type(#name, eGuiSkinAttribute_##name))
#define AddGfx(name) m_mapSkinGfxStrings.insert(tGuiSkinGfxMap::value_type(#name, eGuiSkinGfx_##name))
void cGui::GenerateSkinTypeStrings() {
	///////////////////////
	// Fonts
	AddFont(Default);
	AddFont(Disabled);
	AddFont(WindowLabel);

	///////////////////////
	// Attributes
	AddAttribute(WindowLabelTextOffset);

	AddAttribute(ButtonPressedContentOffset);

	AddAttribute(SliderButtonSize);

	AddAttribute(ListBoxSliderWidth);

	AddAttribute(ComboBoxButtonWidth);
	AddAttribute(ComboBoxSliderWidth);

	///////////////////////
	// Pointer Graphics
	AddGfx(PointerNormal);
	AddGfx(PointerText);

	///////////////////////
	// Window Graphics
	AddGfx(WindowBorderRight);
	AddGfx(WindowBorderLeft);
	AddGfx(WindowBorderUp);
	AddGfx(WindowBorderDown);

	AddGfx(WindowCornerLU);
	AddGfx(WindowCornerRU);
	AddGfx(WindowCornerRD);
	AddGfx(WindowCornerLD);

	AddGfx(WindowLabel);
	AddGfx(WindowBackground);

	///////////////////////////////////
	// Frame Graphics
	AddGfx(FrameBorderRight);
	AddGfx(FrameBorderLeft);
	AddGfx(FrameBorderUp);
	AddGfx(FrameBorderDown);

	AddGfx(FrameCornerLU);
	AddGfx(FrameCornerRU);
	AddGfx(FrameCornerRD);
	AddGfx(FrameCornerLD);

	AddGfx(FrameBackground);

	///////////////////////////////////
	// Check box Graphics
	AddGfx(CheckBoxEnabledUnchecked);
	AddGfx(CheckBoxEnabledChecked);
	AddGfx(CheckBoxDisabledUnchecked);
	AddGfx(CheckBoxDisabledChecked);

	///////////////////////////////////
	// Text box Graphics
	AddGfx(TextBoxBackground);
	AddGfx(TextBoxSelectedTextBack);
	AddGfx(TextBoxMarker);

	///////////////////////////////////
	// List box Graphics
	AddGfx(ListBoxBackground);

	///////////////////////////////////
	// List box Graphics
	AddGfx(ComboBoxButtonIcon);

	AddGfx(ComboBoxBorderRight);
	AddGfx(ComboBoxBorderLeft);
	AddGfx(ComboBoxBorderUp);
	AddGfx(ComboBoxBorderDown);

	AddGfx(ComboBoxCornerLU);
	AddGfx(ComboBoxCornerRU);
	AddGfx(ComboBoxCornerRD);
	AddGfx(ComboBoxCornerLD);

	AddGfx(ComboBoxBackground);

	///////////////////////////////////
	// Slider Graphics
	AddGfx(SliderVertArrowUp);
	AddGfx(SliderVertArrowDown);
	AddGfx(SliderVertBackground);

	AddGfx(SliderHoriArrowUp);
	AddGfx(SliderHoriArrowDown);
	AddGfx(SliderHoriBackground);

	///////////////////////
	// Button Graphics
	AddGfx(ButtonUpBorderRight);
	AddGfx(ButtonUpBorderLeft);
	AddGfx(ButtonUpBorderUp);
	AddGfx(ButtonUpBorderDown);

	AddGfx(ButtonUpCornerLU);
	AddGfx(ButtonUpCornerRU);
	AddGfx(ButtonUpCornerRD);
	AddGfx(ButtonUpCornerLD);

	AddGfx(ButtonUpBackground);

	AddGfx(ButtonDownBorderRight);
	AddGfx(ButtonDownBorderLeft);
	AddGfx(ButtonDownBorderUp);
	AddGfx(ButtonDownBorderDown);

	AddGfx(ButtonDownCornerLU);
	AddGfx(ButtonDownCornerRU);
	AddGfx(ButtonDownCornerRD);
	AddGfx(ButtonDownCornerLD);

	AddGfx(ButtonDownBackground);
}
//-----------------------------------------------------------------------

} // namespace hpl
