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

#include "hpl1/engine/graphics/font_data.h"

#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/graphics/GfxObject.h"
#include "hpl1/engine/graphics/GraphicsDrawer.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/resources/ResourceImage.h"

#include "common/array.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "hpl1/algorithms.h"
#include "hpl1/debug.h"
#include "hpl1/engine/gui/Gui.h"
#include "hpl1/engine/gui/GuiGfxElement.h"
#include "hpl1/string.h"

namespace hpl {

using BitmapArray = Common::Array<Common::SharedPtr<Bitmap2D> >;

Glyph::Glyph(cGfxObject *apObject, cGuiGfxElement *apGuiGfx, const cVector2f &avOffset,
			 const cVector2f &avSize, float afAdvance) {
	_gfxObject = apObject;
	_guiGfx = apGuiGfx;
	_offset = avOffset;
	_size = avSize;
	_advance = afAdvance;
}

Glyph::~Glyph() {
	if (_gfxObject)
		hplDelete(_gfxObject);
	if (_guiGfx)
		hplDelete(_guiGfx);
}

FontData::FontData(const tString &asName, iLowLevelGraphics *apLowLevelGraphics)
	: iResourceBase(asName, 0) {
	_lowLevelGraphics = apLowLevelGraphics;
}

FontData::~FontData() {
	for (int i = 0; i < (int)_glyphs.size(); i++) {
		if (_glyphs[i])
			hplDelete(_glyphs[i]);
	}
}

static Common::ScopedPtr<TiXmlDocument> loadFontXML(const Common::String &filepath) {
	Common::ScopedPtr<TiXmlDocument> doc(new TiXmlDocument(filepath.c_str()));
	if (!doc->LoadFile(filepath.c_str())) {
		debugCN(Hpl1::kDebugLevelError, Hpl1::kDebugResourceLoading,
				"Couldn't load font file at: %s", filepath.c_str());
		return nullptr;
	}
	return doc;
}

cVector2l loadXMLCommon(const TiXmlElement *commonElement, uint16 &firstChar, uint16 &lastchar, float &height, cVector2f &sizeRatio) {
	int lineHeight = Hpl1::stoi(commonElement->Attribute("lineHeight"));
	int base = Hpl1::stoi(commonElement->Attribute("base"));
	firstChar = 0;
	lastchar = 3000; // engine max
	height = static_cast<float>(lineHeight);
	sizeRatio.x = static_cast<float>(base) / height;
	sizeRatio.y = 1;
	return {base, lineHeight};
}

bool loadNextBitmap(BitmapArray &bitmaps, const Common::String &file, const Common::String &fontDir, LowLevelResources *resources) {
	Common::String bitmapPath = fontDir + file;
	Common::SharedPtr<Bitmap2D> bitmap(resources->loadBitmap2D(bitmapPath.c_str()));
	if (bitmap == nullptr) {
		debugCN(Hpl1::kDebugLevelError, Hpl1::kDebugResourceLoading,
				"Couldn't load bitmap %s", file.c_str());
		return false;
	}
	bitmaps.push_back(bitmap);
	return true;
}

bool loadBitmaps(BitmapArray &bitmaps, const TiXmlElement *pagesRoot, const Common::String &fontDir, LowLevelResources *resources) {
	bitmaps.reserve(3000); // engine max
	const TiXmlElement *pageIt = pagesRoot->FirstChildElement("page");
	for (; pageIt != nullptr; pageIt = pageIt->NextSiblingElement("page")) {
		if (!loadNextBitmap(bitmaps, pageIt->Attribute("file"), fontDir, resources))
			return false;
	}
	return true;
}

void FontData::loadNextGlyph(const TiXmlElement *charIt, BitmapArray &bitmaps, const cVector2l &fontSize) {
	int id = Hpl1::stoi(charIt->Attribute("id"));
	int x = Hpl1::stoi(charIt->Attribute("x"));
	int y = Hpl1::stoi(charIt->Attribute("y"));
	int w = Hpl1::stoi(charIt->Attribute("width"));
	int h = Hpl1::stoi(charIt->Attribute("height"));
	int xOffset = Hpl1::stoi(charIt->Attribute("xoffset"));
	int yOffset = Hpl1::stoi(charIt->Attribute("yoffset"));
	int advance = Hpl1::stoi(charIt->Attribute("xadvance"));
	int page = Hpl1::stoi(charIt->Attribute("page"));

	cVector2l size(w, h);
	Bitmap2D &src = *bitmaps[page];
	Common::ScopedPtr<Bitmap2D> charBmp(_lowLevelGraphics->CreateBitmap2D(size));
	src.drawToBitmap(*charBmp, {0, 0}, Common::Rect(x, y, x + w, y + h));

	charBmp->copyRedToAlpha();
	_glyphs[id] = createGlyph(charBmp.get(), {xOffset, yOffset}, {w, h}, fontSize, advance);
}

void FontData::loadGlyphs(const TiXmlElement *charsRoot, BitmapArray &bitmaps, const cVector2l &fontSize) {
	const TiXmlElement *charIt = charsRoot->FirstChildElement("char");
	Hpl1::resizeAndFill(_glyphs, 3000, nullptr);
	for (; charIt != nullptr; charIt = charIt->NextSiblingElement("char"))
		loadNextGlyph(charIt, bitmaps, fontSize);
}

bool FontData::createFromBitmapFile(const tString &filename) {
	Common::ScopedPtr<TiXmlDocument> xmlDoc = loadFontXML(filename);
	if (!xmlDoc)
		return false;
	TiXmlElement *xmlRoot = xmlDoc->RootElement();
	const cVector2l fontsize = loadXMLCommon(xmlRoot->FirstChildElement("common"), _firstChar, _lastChar, _height, _sizeRatio);
	BitmapArray bitmaps;
	if (!loadBitmaps(bitmaps, xmlRoot->FirstChildElement("pages"), Hpl1::fileDir(filename), _lowLevelResources))
		return false;
	loadGlyphs(xmlRoot->FirstChildElement("chars"), bitmaps, fontsize);
	return true;
}

bool FontData::createFromFontFile(const tString &asFileName, int alSize, unsigned short alFirstChar,
								  unsigned short alLastChar) {
	error("call to unimplemented function FontData::CreateFromFontFile");
}

void FontData::draw(const cVector3f &avPos, const cVector2f &avSize, const cColor &aCol,
					eFontAlign aAlign, const tWString &text) {
	int lCount = 0;
	// float lXAdd = 0;
	cVector3f vPos = avPos;

	if (aAlign == eFontAlign_Center) {
		vPos.x -= getLength(avSize, text.c_str()) / 2;
	} else if (aAlign == eFontAlign_Right) {
		vPos.x -= getLength(avSize, text.c_str());
	}

	while (lCount < static_cast<int>(text.size())) {
		char32_t lGlyphNum = text[lCount];
		if (lGlyphNum < _firstChar || lGlyphNum > _lastChar) {
			lCount++;
			continue;
		}
		lGlyphNum -= _firstChar;

		Glyph *pGlyph = _glyphs[lGlyphNum];
		if (pGlyph) {
			cVector2f vOffset(pGlyph->_offset * avSize);
			cVector2f vSize(pGlyph->_size * avSize);

			_graphicsDrawer->DrawGfxObject(pGlyph->_gfxObject, vPos + vOffset, vSize, aCol);

			vPos.x += pGlyph->_advance * avSize.x;
		}
		lCount++;
	}
}

int FontData::drawWordWrap(cVector3f avPos, float afLength, float afFontHeight, cVector2f avSize, const cColor &aCol,
						   eFontAlign aAlign, const tWString &asString) {
	int rows = 0;

	unsigned int pos;
	unsigned int first_letter = 0;
	unsigned int last_space = 0;

	tUIntList RowLengthList;

	float fTextLength;

	for (pos = 0; pos < asString.size(); pos++) {
		if (asString[pos] == ' ' || asString[pos] == '\n') {
			tWString temp = asString.substr(first_letter, pos - first_letter);
			fTextLength = getLength(avSize, temp.c_str());

			bool nothing = true;
			if (fTextLength > afLength) {
				rows++;
				RowLengthList.push_back(last_space);
				first_letter = last_space + 1;
				last_space = pos;
				nothing = false;
			}
			if (asString[pos] == '\n') {
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
	fTextLength = getLength(avSize, temp.c_str());
	if (fTextLength > afLength) {
		rows++;
		RowLengthList.push_back(last_space);
	}

	if (rows == 0) {
		draw(avPos, avSize, aCol, aAlign, asString);
	} else {
		first_letter = 0;
		unsigned int i = 0;

		for (tUIntListIt it = RowLengthList.begin(); it != RowLengthList.end(); ++it) {
			draw(cVector3f(avPos.x, avPos.y + i * afFontHeight, avPos.z), avSize, aCol, aAlign,
				 asString.substr(first_letter, *it - first_letter));
			i++;
			first_letter = *it + 1;
		}
		draw(cVector3f(avPos.x, avPos.y + i * afFontHeight, avPos.z), avSize, aCol, aAlign,
			 asString.substr(first_letter));
	}

	return rows;
}

void FontData::getWordWrapRows(float afLength, float afFontHeight, cVector2f avSize,
							   const tWString &asString, tWStringVec *apRowVec) {
	int rows = 0;

	unsigned int pos;
	unsigned int first_letter = 0;
	unsigned int last_space = 0;

	tUIntList RowLengthList;

	float fTextLength;

	for (pos = 0; pos < asString.size(); pos++) {
		// Log("char: %d\n",(char)asString[pos]);
		if (asString[pos] == ' ' || asString[pos] == '\n') {
			tWString temp = asString.substr(first_letter, pos - first_letter);
			fTextLength = getLength(avSize, temp.c_str());

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
			if (asString[pos] == '\n') {
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
	fTextLength = getLength(avSize, temp.c_str());
	if (fTextLength > afLength) {
		rows++;
		RowLengthList.push_back(last_space);
	}

	if (rows == 0) {
		apRowVec->push_back(asString);
	} else {
		first_letter = 0;
		unsigned int i = 0;

		for (tUIntListIt it = RowLengthList.begin(); it != RowLengthList.end(); ++it) {
			apRowVec->push_back(asString.substr(first_letter, *it - first_letter));
			i++;
			first_letter = *it + 1;
		}
		apRowVec->push_back(asString.substr(first_letter));
	}
}

float FontData::getLength(const cVector2f &avSize, const char32_t *sText) {
	int lCount = 0;
	// float lXAdd = 0;
	float fLength = 0;
	while (sText[lCount] != 0) {
		unsigned short lGlyphNum = (sText[lCount]);
		if (lGlyphNum < _firstChar || lGlyphNum > _lastChar) {
			lCount++;
			continue;
		}
		lGlyphNum -= _firstChar;

		Glyph *pGlyph = _glyphs[lGlyphNum];
		if (pGlyph) {
			// cVector2f vOffset(pGlyph->_offset * avSize);
			// cVector2f vSize(pGlyph->_size * avSize);

			fLength += pGlyph->_advance * avSize.x;
		}
		lCount++;
	}

	return fLength;
}

Glyph *FontData::createGlyph(Bitmap2D *apBmp, const cVector2l &avOffset, const cVector2l &avSize,
							 const cVector2l &avFontSize, int alAdvance) {
	// Here the bitmap should be saved to diskk for faster loading.

	//////////////////////////
	// Gfx object
	cGfxObject *pObject = _graphicsDrawer->CreateGfxObject(apBmp, "fontnormal", false);

	//////////////////////////
	// Gui gfx
	cGuiGfxElement *pGuiGfx = _gui->CreateGfxFilledRect(cColor(1, 1), eGuiMaterial_FontNormal, false);
	cResourceImage *pImage = pObject->GetMaterial()->GetImage(eMaterialTexture_Diffuse);
	pImage->IncUserCount();
	pGuiGfx->AddImage(pImage);

	//////////////////////////
	// Sizes
	cVector2f vSize;
	vSize.x = ((float)avSize.x) / ((float)avFontSize.x) * _sizeRatio.x;
	vSize.y = ((float)avSize.y) / ((float)avFontSize.y) * _sizeRatio.y;

	cVector2f vOffset;
	vOffset.x = ((float)avOffset.x) / ((float)avFontSize.x);
	vOffset.y = ((float)avOffset.y) / ((float)avFontSize.y);

	float fAdvance = ((float)alAdvance) / ((float)avFontSize.x) * _sizeRatio.x;

	Glyph *pGlyph = hplNew(Glyph, (pObject, pGuiGfx, vOffset, vSize, fAdvance));

	return pGlyph;
}

void FontData::addGlyph(Glyph *apGlyph) {
	_glyphs.push_back(apGlyph);
}

} // namespace hpl
