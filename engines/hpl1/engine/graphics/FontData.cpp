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

#include "hpl1/engine/graphics/FontData.h"
#include <stdarg.h>
#include <stdlib.h>

#include "hpl1/engine/system/LowLevelSystem.h"

#include "hpl1/engine/graphics/GfxObject.h"
#include "hpl1/engine/graphics/GraphicsDrawer.h"

#include "hpl1/engine/resources/ResourceImage.h"

#include "hpl1/engine/gui/Gui.h"
#include "hpl1/engine/gui/GuiGfxElement.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGlyph::cGlyph(cGfxObject *apObject, cGuiGfxElement *apGuiGfx, const cVector2f &avOffset,
			   const cVector2f &avSize, float afAdvance) {
	mpGfxObject = apObject;
	mpGuiGfx = apGuiGfx;
	mvOffset = avOffset;
	mvSize = avSize;
	mfAdvance = afAdvance;
}

cGlyph::~cGlyph() {
	if (mpGfxObject)
		hplDelete(mpGfxObject);
	if (mpGuiGfx)
		hplDelete(mpGuiGfx);
}

//-----------------------------------------------------------------------

iFontData::iFontData(const tString &asName, iLowLevelGraphics *apLowLevelGraphics)
	: iResourceBase(asName, 0) {
	mpLowLevelGraphics = apLowLevelGraphics;
}

//-----------------------------------------------------------------------

iFontData::~iFontData() {
	for (int i = 0; i < (int)mvGlyphs.size(); i++) {
		if (mvGlyphs[i])
			hplDelete(mvGlyphs[i]);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void iFontData::Draw(const cVector3f &avPos, const cVector2f &avSize, const cColor &aCol,
					 eFontAlign aAlign, const wchar_t *fmt, ...) {
	wchar_t sText[256];
	va_list ap;
	if (fmt == NULL)
		return;
	va_start(ap, fmt);
	vswprintf(sText, 255, fmt, ap);
	va_end(ap);

	int lCount = 0;
	float lXAdd = 0;
	cVector3f vPos = avPos;

	if (aAlign == eFontAlign_Center) {
		vPos.x -= GetLength(avSize, sText) / 2;
	} else if (aAlign == eFontAlign_Right) {
		vPos.x -= GetLength(avSize, sText);
	}

	while (sText[lCount] != 0) {
		wchar_t lGlyphNum = ((wchar_t)sText[lCount]);
		if (lGlyphNum < mlFirstChar || lGlyphNum > mlLastChar) {
			lCount++;
			continue;
		}
		lGlyphNum -= mlFirstChar;

		cGlyph *pGlyph = mvGlyphs[lGlyphNum];
		if (pGlyph) {
			cVector2f vOffset(pGlyph->mvOffset * avSize);
			cVector2f vSize(pGlyph->mvSize * avSize);

			mpGraphicsDrawer->DrawGfxObject(pGlyph->mpGfxObject, vPos + vOffset, vSize, aCol);

			vPos.x += pGlyph->mfAdvance * avSize.x;
		}
		lCount++;
	}
}

//-----------------------------------------------------------------------

int iFontData::DrawWordWrap(cVector3f avPos, float afLength, float afFontHeight, cVector2f avSize, const cColor &aCol,
							eFontAlign aAlign, const tWString &asString) {
	int rows = 0;

	unsigned int pos;
	unsigned int first_letter = 0;
	unsigned int last_space = 0;

	tUIntList RowLengthList;

	float fTextLength;

	for (pos = 0; pos < asString.size(); pos++) {
		if (asString[pos] == _W(' ') || asString[pos] == _W('\n')) {
			tWString temp = asString.substr(first_letter, pos - first_letter);
			fTextLength = GetLength(avSize, temp.c_str());

			bool nothing = true;
			if (fTextLength > afLength) {
				rows++;
				RowLengthList.push_back(last_space);
				first_letter = last_space + 1;
				last_space = pos;
				nothing = false;
			}
			if (asString[pos] == _W('\n')) {
				last_space = pos;
				first_letter = last_space + 1;
				RowLengthList.push_back(last_space - 1);
				rows++;
				nothing = false;
			}
			if (nothing) {
				last_space = pos;
			}
		}
	}
	tWString temp = asString.substr(first_letter, pos - first_letter);
	fTextLength = GetLength(avSize, temp.c_str());
	if (fTextLength > afLength) {
		rows++;
		RowLengthList.push_back(last_space);
	}

	if (rows == 0) {
		Draw(avPos, avSize, aCol, aAlign, _W("%ls"), asString.c_str());
	} else {
		first_letter = 0;
		unsigned int i = 0;

		for (tUIntListIt it = RowLengthList.begin(); it != RowLengthList.end(); ++it) {
			Draw(cVector3f(avPos.x, avPos.y + i * afFontHeight, avPos.z), avSize, aCol, aAlign,
				 _W("%ls"), asString.substr(first_letter, *it - first_letter).c_str());
			i++;
			first_letter = *it + 1;
		}
		Draw(cVector3f(avPos.x, avPos.y + i * afFontHeight, avPos.z), avSize, aCol, aAlign,
			 _W("%ls"), asString.substr(first_letter).c_str());
	}

	return rows;
}

//-----------------------------------------------------------------------

void iFontData::GetWordWrapRows(float afLength, float afFontHeight, cVector2f avSize,
								const tWString &asString, tWStringVec *apRowVec) {
	int rows = 0;

	unsigned int pos;
	unsigned int first_letter = 0;
	unsigned int last_space = 0;

	tUIntList RowLengthList;

	float fTextLength;

	for (pos = 0; pos < asString.size(); pos++) {
		// Log("char: %d\n",(char)asString[pos]);
		if (asString[pos] == _W(' ') || asString[pos] == _W('\n')) {
			tWString temp = asString.substr(first_letter, pos - first_letter);
			fTextLength = GetLength(avSize, temp.c_str());

			// Log("r:%d p:%d f:%d l:%d Temp:'%s'\n",rows,pos,first_letter,last_space,
			// temp.c_str());
			bool nothing = true;
			if (fTextLength > afLength) {
				rows++;
				RowLengthList.push_back(last_space);
				first_letter = last_space + 1;
				last_space = pos;
				nothing = false;
			}
			if (asString[pos] == _W('\n')) {
				last_space = pos;
				first_letter = last_space + 1;
				RowLengthList.push_back(last_space);
				rows++;
				nothing = false;
			}
			if (nothing) {
				last_space = pos;
			}
		}
	}
	tWString temp = asString.substr(first_letter, pos - first_letter);
	fTextLength = GetLength(avSize, temp.c_str());
	if (fTextLength > afLength) {
		rows++;
		RowLengthList.push_back(last_space);
	}

	if (rows == 0) {
		apRowVec->push_back(asString.c_str());
	} else {
		first_letter = 0;
		unsigned int i = 0;

		for (tUIntListIt it = RowLengthList.begin(); it != RowLengthList.end(); ++it) {
			apRowVec->push_back(asString.substr(first_letter, *it - first_letter).c_str());
			i++;
			first_letter = *it + 1;
		}
		apRowVec->push_back(asString.substr(first_letter).c_str());
	}
}

//-----------------------------------------------------------------------

float iFontData::GetLength(const cVector2f &avSize, const wchar_t *sText) {
	int lCount = 0;
	float lXAdd = 0;
	float fLength = 0;
	while (sText[lCount] != 0) {
		unsigned short lGlyphNum = ((wchar_t)sText[lCount]);
		if (lGlyphNum < mlFirstChar || lGlyphNum > mlLastChar) {
			lCount++;
			continue;
		}
		lGlyphNum -= mlFirstChar;

		cGlyph *pGlyph = mvGlyphs[lGlyphNum];
		if (pGlyph) {
			cVector2f vOffset(pGlyph->mvOffset * avSize);
			cVector2f vSize(pGlyph->mvSize * avSize);

			fLength += pGlyph->mfAdvance * avSize.x;
		}
		lCount++;
	}

	return fLength;
}

//-----------------------------------------------------------------------

float iFontData::GetLengthFmt(const cVector2f &avSize, const wchar_t *fmt, ...) {
	wchar_t sText[256];
	va_list ap;
	if (fmt == NULL)
		return 0;
	va_start(ap, fmt);
	vswprintf(sText, 255, fmt, ap);
	va_end(ap);

	return GetLength(avSize, sText);
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGlyph *iFontData::CreateGlyph(iBitmap2D *apBmp, const cVector2l &avOffset, const cVector2l &avSize,
							   const cVector2l &avFontSize, int alAdvance) {
	// Here the bitmap should be saved to diskk for faster loading.

	//////////////////////////
	// Gfx object
	cGfxObject *pObject = mpGraphicsDrawer->CreateGfxObject(apBmp, "fontnormal", false);

	//////////////////////////
	// Gui gfx
	cGuiGfxElement *pGuiGfx = mpGui->CreateGfxFilledRect(cColor(1, 1), eGuiMaterial_FontNormal, false);
	cResourceImage *pImage = pObject->GetMaterial()->GetImage(eMaterialTexture_Diffuse);
	pImage->IncUserCount();
	pGuiGfx->AddImage(pImage);

	//////////////////////////
	// Sizes
	cVector2f vSize;
	vSize.x = ((float)avSize.x) / ((float)avFontSize.x) * mvSizeRatio.x;
	vSize.y = ((float)avSize.y) / ((float)avFontSize.y) * mvSizeRatio.y;

	cVector2f vOffset;
	vOffset.x = ((float)avOffset.x) / ((float)avFontSize.x);
	vOffset.y = ((float)avOffset.y) / ((float)avFontSize.y);

	float fAdvance = ((float)alAdvance) / ((float)avFontSize.x) * mvSizeRatio.x;

	cGlyph *pGlyph = hplNew(cGlyph, (pObject, pGuiGfx, vOffset, vSize, fAdvance));

	return pGlyph;
}

//-----------------------------------------------------------------------

void iFontData::AddGlyph(cGlyph *apGlyph) {
	mvGlyphs.push_back(apGlyph);
}

//-----------------------------------------------------------------------

} // namespace hpl
