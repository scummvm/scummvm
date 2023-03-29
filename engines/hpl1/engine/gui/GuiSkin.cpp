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

#include "hpl1/engine/gui/GuiSkin.h"

#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"

#include "hpl1/engine/gui/Gui.h"
#include "hpl1/engine/gui/GuiGfxElement.h"

#include "hpl1/engine/resources/FontManager.h"
#include "hpl1/engine/resources/Resources.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// FONT
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGuiSkinFont::cGuiSkinFont(cGui *apGui) {
	mpGui = apGui;

	mpMaterial = NULL;
}
cGuiSkinFont::~cGuiSkinFont() {
	// Skip deleting font...
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGuiSkin::cGuiSkin(const tString &asName, cGui *apGui) {
	mpGui = apGui;
	msName = asName;

	Hpl1::resizeAndFill(mvGfxElements, eGuiSkinGfx_LastEnum, nullptr);
	Hpl1::resizeAndFill(mvFonts, eGuiSkinGfx_LastEnum, nullptr);
	mvAttributes.resize(eGuiSkinAttribute_LastEnum);
}

//-----------------------------------------------------------------------

cGuiSkin::~cGuiSkin() {
	for (size_t i = 0; i < mvGfxElements.size(); ++i)
		mpGui->DestroyGfx(mvGfxElements[i]);

	STLDeleteAll(mvFonts);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

static eGuiGfxAnimationType ToAnimType(const char *apString) {
	if (apString == NULL)
		return eGuiGfxAnimationType_Loop;

	tString sLow = cString::ToLowerCase(apString);

	if (sLow == "loop")
		return eGuiGfxAnimationType_Loop;
	if (sLow == "oscillate")
		return eGuiGfxAnimationType_Oscillate;
	if (sLow == "random")
		return eGuiGfxAnimationType_Random;
	if (sLow == "stop_at_end")
		return eGuiGfxAnimationType_StopAtEnd;

	Warning("Animation type '%s' does not exist!\n", sLow.c_str());

	return eGuiGfxAnimationType_Loop;
}

static eGuiMaterial ToMaterial(const char *apString) {
	if (apString == NULL)
		return eGuiMaterial_Alpha;

	tString sLow = cString::ToLowerCase(apString);

	if (sLow == "alpha")
		return eGuiMaterial_Alpha;
	if (sLow == "diffuse")
		return eGuiMaterial_Diffuse;
	if (sLow == "font_normal")
		return eGuiMaterial_FontNormal;
	if (sLow == "additive")
		return eGuiMaterial_Additive;
	if (sLow == "modulative")
		return eGuiMaterial_Modulative;

	Warning("Material type '%s' does not exist!\n", sLow.c_str());

	return eGuiMaterial_Alpha;
}

bool cGuiSkin::LoadFromFile(const tString &asFile) {
	//////////////////////////////////
	// Open document
	TiXmlDocument *pXmlDoc = hplNew(TiXmlDocument, (asFile.c_str()));
	if (pXmlDoc->LoadFile() == false) {
		Error("Couldn't load skin file '%s'!\n", asFile.c_str());
		hplDelete(pXmlDoc);
		return false;
	}

	/////////////////////////
	// Get the root.
	TiXmlElement *pRootElem = pXmlDoc->RootElement();

	//////////////////////////////////////
	// ATTRIBUTES
	TiXmlElement *pAttributesElement = pRootElem->FirstChildElement("Attributes");

	// Iterate font elements
	TiXmlElement *pAttributeElem = pAttributesElement->FirstChildElement();
	for (; pAttributeElem != NULL; pAttributeElem = pAttributeElem->NextSiblingElement()) {
		tString sType = cString::ToString(pAttributeElem->Attribute("type"), "");

		eGuiSkinAttribute type = mpGui->GetSkinAttributeFromString(sType);
		if (type == eGuiSkinAttribute_LastEnum)
			continue;

		tString sValue = cString::ToString(pAttributeElem->Attribute("value"), "");

		cVector3f vVal(0);
		tFloatVec vValues;
		tString sSepp = " ";
		cString::GetFloatVec(sValue, vValues, &sSepp);

		if (vValues.size() > 0)
			vVal.x = vValues[0];
		if (vValues.size() > 1)
			vVal.y = vValues[1];
		if (vValues.size() > 2)
			vVal.z = vValues[2];

		mvAttributes[type] = vVal;
	}

	//////////////////////////////////////
	// FONTS
	TiXmlElement *pFontsElement = pRootElem->FirstChildElement("Fonts");

	// Iterate font elements
	TiXmlElement *pFontElem = pFontsElement->FirstChildElement();
	for (; pFontElem != NULL; pFontElem = pFontElem->NextSiblingElement()) {
		tString sType = cString::ToString(pFontElem->Attribute("type"), "");

		eGuiSkinFont type = mpGui->GetSkinFontFromString(sType);
		if (type == eGuiSkinFont_LastEnum)
			continue;

		tString sFontFile = cString::ToString(pFontElem->Attribute("file"), "");
		cVector2f vSize = cString::ToVector2f(pFontElem->Attribute("size"), 1);
		cColor color = cString::ToColor(pFontElem->Attribute("color"), cColor(1, 1));

		cGuiSkinFont *pFont = hplNew(cGuiSkinFont, (mpGui));

		pFont->mpFont = mpGui->GetResources()->GetFontManager()->CreateFontData(sFontFile);
		pFont->mvSize = vSize;
		pFont->mColor = color;

		mvFonts[type] = pFont;
	}

	//////////////////////////////////////
	// GFX ELEMENTS

	// Get gfx element
	TiXmlElement *pGfxElementsElement = pRootElem->FirstChildElement("GfxElements");

	// Iterate gfx elements
	TiXmlElement *pGfxElem = pGfxElementsElement->FirstChildElement();
	for (; pGfxElem != NULL; pGfxElem = pGfxElem->NextSiblingElement()) {
		tString sType = cString::ToString(pGfxElem->Attribute("type"), "");

		eGuiSkinGfx type = mpGui->GetSkinGfxFromString(sType);
		if (type == eGuiSkinGfx_LastEnum)
			continue;

		tString sFile = cString::ToString(pGfxElem->Attribute("file"), "");
		cVector2f vOffset = cString::ToVector2f(pGfxElem->Attribute("offset"), 0);
		cVector2f vSize = cString::ToVector2f(pGfxElem->Attribute("active_size"), -1);
		cColor color = cString::ToColor(pGfxElem->Attribute("color"), cColor(1, 1));
		eGuiMaterial material = ToMaterial(pGfxElem->Attribute("material"));

		bool bAnimated = cString::ToBool(pGfxElem->Attribute("animated"), false);
		float fAnimFrameTime = cString::ToFloat(pGfxElem->Attribute("anim_frame_time"), 1);
		eGuiGfxAnimationType animType = ToAnimType(pGfxElem->Attribute("anim_mode"));

		cGuiGfxElement *pGfx = NULL;
		if (sFile != "") {
			if (bAnimated) {
				pGfx = mpGui->CreateGfxImageBuffer(sFile, material, true, color);
			} else {
				pGfx = mpGui->CreateGfxImage(sFile, material, color);
			}
		} else {
			pGfx = mpGui->CreateGfxFilledRect(color, material);
		}

		if (pGfx) {
			if (bAnimated) {
				cGuiGfxAnimation *pAnim = pGfx->GetAnimation(0);
				pAnim->SetFrameLength(fAnimFrameTime);
				pAnim->SetType(animType);
			}

			if (vSize.x >= 0)
				pGfx->SetActiveSize(vSize);
			pGfx->SetOffset(cVector3f(vOffset.x, vOffset.y, 0));

			mvGfxElements[type] = pGfx;
		}
	}

	hplDelete(pXmlDoc);

	return true;
}

//-----------------------------------------------------------------------

cGuiGfxElement *cGuiSkin::GetGfx(eGuiSkinGfx aType) {
	return mvGfxElements[aType];
}

cGuiSkinFont *cGuiSkin::GetFont(eGuiSkinFont aType) {
	return mvFonts[aType];
}

const cVector3f &cGuiSkin::GetAttribute(eGuiSkinAttribute aType) {
	return mvAttributes[aType];
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

} // namespace hpl
