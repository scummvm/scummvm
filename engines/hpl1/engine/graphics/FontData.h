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

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/resources/ResourceBase.h"
#include "hpl1/engine/system/SystemTypes.h"
#include <vector>

namespace hpl {

class ImageManager;
class iLowLevelGraphics;
class cGraphicsDrawer;
class cGfxObject;
class Bitmap2D;
class cGuiGfxElement;
class cGui;

//------------------------------------------------

class cGlyph {
public:
	cGlyph(cGfxObject *apObject, cGuiGfxElement *apGuiGfx, const cVector2f &avOffset,
		   const cVector2f &avSize, float afAdvance);
	~cGlyph();

	cGfxObject *mpGfxObject;
	cGuiGfxElement *mpGuiGfx;
	cVector2f mvOffset;
	cVector2f mvSize;
	float mfAdvance;
};

typedef std::vector<cGlyph *> tGlyphVec;
typedef tGlyphVec::iterator tGlyphVecIt;

class iFontData : public iResourceBase {
public:
	iFontData(const tString &asName, iLowLevelGraphics *apLowLevelGraphics);
	~iFontData();

	virtual bool CreateFromFontFile(const tString &asFileName, int alSize, unsigned short alFirstChar,
									unsigned short alLastChar) = 0;

	virtual bool CreateFromBitmapFile(const tString &asFileName) = 0;

	bool Reload() { return false; }
	void Unload() {}
	void Destroy() {}

	/**
	 * Used internally
	 */
	void SetUp(cGraphicsDrawer *apGraphicsDrawer, LowLevelResources *apLowLevelResources,
			   cGui *apGui) {
		mpGraphicsDrawer = apGraphicsDrawer;
		mpLowLevelResources = apLowLevelResources;
		mpGui = apGui;
	}

	/**
	 * Used internally
	 * \param alNum
	 * \return
	 */
	inline cGlyph *GetGlyph(int alNum) const { return mvGlyphs[alNum]; }

	inline unsigned short GetFirstChar() { return mlFirstChar; }
	inline unsigned short GetLastChar() { return mlLastChar; }

	inline const cVector2f &GetSizeRatio() const { return mvSizeRatio; }

	/**
	 * Draw a string.
	 * \param avPos Screen pos
	 * \param avSize size of the characters
	 * \param aCol color
	 * \param mAlign alignment
	 * \param fmt
	 * \param ...
	 */
	void Draw(const cVector3f &avPos, const cVector2f &avSize, const cColor &aCol, eFontAlign mAlign,
			  const wchar_t *fmt, ...);
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
	int DrawWordWrap(cVector3f avPos, float afLength, float afFontHeight, cVector2f avSize, const cColor &aCol,
					 eFontAlign aAlign, const tWString &asString);

	void GetWordWrapRows(float afLength, float afFontHeight, cVector2f avSize, const tWString &asString,
						 tWStringVec *apRowVec);

	/**
	 * Get height of the font.
	 * \return
	 */
	inline float GetHeight() const { return mfHeight; }

	/**
	 * Get the length in virtual screen size "pixels" of a formated string
	 * \param avSize size of the characters
	 * \param fmt
	 * \param ...
	 * \return
	 */
	float GetLengthFmt(const cVector2f &avSize, const wchar_t *fmt, ...);
	/**
	 * Get the length in virtual screen size "pixels" of a string
	 * \param avSize size of the characters
	 * \param sText
	 * \return
	 */
	float GetLength(const cVector2f &avSize, const wchar_t *sText);

protected:
	iLowLevelGraphics *mpLowLevelGraphics;
	LowLevelResources *mpLowLevelResources;
	cGraphicsDrawer *mpGraphicsDrawer;
	cGui *mpGui;

	tGlyphVec mvGlyphs;

	float mfHeight;
	unsigned short mlFirstChar;
	unsigned short mlLastChar;

	cVector2f mvSizeRatio;

	cGlyph *CreateGlyph(Bitmap2D *apBmp, const cVector2l &avOffset, const cVector2l &avSize,
						const cVector2l &avFontSize, int alAdvance);
	void AddGlyph(cGlyph *apGlyph);
};

};     // namespace hpl
#endif // HPL_FONTDATA_H
