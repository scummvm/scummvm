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

#ifndef HPL_FONTDATA_H
#define HPL_FONTDATA_H

#include "common/array.h"
#include "common/ptr.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/resources/ResourceBase.h"
#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/system/SystemTypes.h"

class TiXmlElement;

namespace hpl {

class ImageManager;
class iLowLevelGraphics;
class cGraphicsDrawer;
class cGfxObject;
class Bitmap2D;
class cGuiGfxElement;
class cGui;

class Glyph {
public:
	Glyph(cGfxObject *apObject, cGuiGfxElement *apGuiGfx, const cVector2f &avOffset,
		  const cVector2f &avSize, float afAdvance);
	~Glyph();

	cGfxObject *_gfxObject;
	cGuiGfxElement *_guiGfx;
	cVector2f _offset;
	cVector2f _size;
	float _advance;
};

typedef Common::Array<Glyph *> tGlyphVec;
typedef tGlyphVec::iterator tGlyphVecIt;

class FontData : public iResourceBase {
public:
	FontData(const tString &asName, iLowLevelGraphics *apLowLevelGraphics);
	~FontData();

	bool createFromFontFile(const tString &asFileName, int alSize, unsigned short alFirstChar,
							unsigned short alLastChar);

	bool createFromBitmapFile(const tString &asFileName);

	bool reload() { return false; }
	void unload() {}
	void destroy() {}

	/**
	 * Used internally
	 */
	void setUp(cGraphicsDrawer *apGraphicsDrawer, LowLevelResources *apLowLevelResources,
			   cGui *apGui) {
		_graphicsDrawer = apGraphicsDrawer;
		_lowLevelResources = apLowLevelResources;
		_gui = apGui;
	}

	/**
	 * Used internally
	 * \param alNum
	 * \return
	 */
	inline Glyph *getGlyph(int alNum) const { return _glyphs[alNum]; }

	inline unsigned short getFirstChar() { return _firstChar; }
	inline unsigned short getLastChar() { return _lastChar; }

	inline const cVector2f &getSizeRatio() const { return _sizeRatio; }

	/**
	 * Draw a string.
	 * \param avPos Screen pos
	 * \param avSize size of the characters
	 * \param aCol color
	 * \param mAlign alignment
	 * \param fmt
	 * \param ...
	 */
	void draw(const cVector3f &avPos, const cVector2f &avSize, const cColor &aCol, eFontAlign mAlign,
			  const tWString &text);
	/**
	 * Draw a string  with word wrap.
	 * \param avPos Screen pos
	 * \param afLength Max length of a line
	 * \param afFontHeight The distance from base of character above to base of character below
	 * \param avSize size of the characters
	 * \param aCol color
	 * \param aAlign alignment
	 * \param asString
	 * \return Extra number of rows generated.
	 */
	int drawWordWrap(cVector3f avPos, float afLength, float afFontHeight, cVector2f avSize, const cColor &aCol,
					 eFontAlign aAlign, const tWString &asString);

	void getWordWrapRows(float afLength, float afFontHeight, cVector2f avSize, const tWString &asString,
						 tWStringVec *apRowVec);

	/**
	 * Get height of the font.
	 * \return
	 */
	inline float getHeight() const { return _height; }

	/**
	 * Get the length in screen size "pixels" of a string
	 * \param avSize size of the characters
	 * \param sText
	 * \return
	 */
	float getLength(const cVector2f &avSize, const char32_t *sText);

private:
	iLowLevelGraphics *_lowLevelGraphics;
	LowLevelResources *_lowLevelResources;
	cGraphicsDrawer *_graphicsDrawer;
	cGui *_gui;

	tGlyphVec _glyphs;

	float _height;
	unsigned short _firstChar;
	unsigned short _lastChar;

	cVector2f _sizeRatio;

	Glyph *createGlyph(Bitmap2D *apBmp, const cVector2l &avOffset, const cVector2l &avSize,
					   const cVector2l &avFontSize, int alAdvance);
	void addGlyph(Glyph *apGlyph);
	void loadGlyphs(const TiXmlElement *charsRoot, Common::Array<Common::SharedPtr<Bitmap2D> > &bitmaps, const cVector2l &fontSize);
	void loadNextGlyph(const TiXmlElement *charIt, Common::Array<Common::SharedPtr<Bitmap2D> > &bitmaps, const cVector2l &fontSize);
};

} // namespace hpl

#endif // HPL_FONTDATA_H
