/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DIRECTOR_CAST_H
#define DIRECTOR_CAST_H

#include "common/rect.h"
#include "common/substream.h"
#include "director/archive.h"
#include "graphics/surface.h"

namespace Director {

class Stxt;
class CachedMacText;

enum CastType {
	kCastTypeNull = 0,
	kCastBitmap = 1,
	kCastFilmLoop = 2,
	kCastText = 3,
	kCastPalette = 4,
	kCastPicture = 5,
	kCastSound = 6,
	kCastButton = 7,
	kCastShape = 8,
	kCastMovie = 9,
	kCastDigitalVideo = 10,
	kCastLingoScript = 11,
	kCastRTE = 12
};

class Cast {
public:
	CastType type;
	Common::Rect initialRect;
	Common::Rect boundingRect;
	Common::Array<Resource> children;

	const Graphics::Surface *surface;

	byte modified;
};

class BitmapCast : public Cast {
public:
	BitmapCast(Common::ReadStreamEndian &stream, uint32 castTag, uint16 version = 2);

	uint16 regX;
	uint16 regY;
	uint8 flags;
	uint16 someFlaggyThing;
	uint16 unk1, unk2;

	uint16 bitsPerPixel;

	uint32 tag;
};

enum ShapeType {
	kShapeRectangle,
	kShapeRoundRect,
	kShapeOval,
	kShapeLine
};

class ShapeCast : public Cast {
public:
	ShapeCast(Common::ReadStreamEndian &stream, uint16 version = 2);

	ShapeType shapeType;
	uint16 pattern;
	byte fgCol;
	byte bgCol;
	byte fillType;
	byte lineThickness;
	byte lineDirection;
};

enum TextType {
	kTextTypeAdjustToFit,
	kTextTypeScrolling,
	kTextTypeFixed
};

enum TextAlignType {
	kTextAlignRight = -1,
	kTextAlignLeft,
	kTextAlignCenter
};

enum TextFlag {
	kTextFlagEditable,
	kTextFlagAutoTab,
	kTextFlagDoNotWrap
};

enum SizeType {
	kSizeNone,
	kSizeSmallest,
	kSizeSmall,
	kSizeMedium,
	kSizeLarge,
	kSizeLargest
};

class TextCast : public Cast {
public:
	TextCast(Common::ReadStreamEndian &stream, uint16 version = 2);

	SizeType borderSize;
	SizeType gutterSize;
	SizeType boxShadow;

	byte flags1;
	uint32 fontId;
	uint16 fontSize;
	TextType textType;
	TextAlignType textAlign;
	SizeType textShadow;
	byte textSlant;
	Common::Array<TextFlag> textFlags;
	uint16 palinfo1, palinfo2, palinfo3;

	Common::String _ftext;
	void importStxt(const Stxt *stxt);
	void importRTE(byte* text);
	CachedMacText *cachedMacText;
};

enum ButtonType {
	kTypeButton,
	kTypeCheckBox,
	kTypeRadio
};

class ButtonCast : public TextCast {
public:
	ButtonCast(Common::ReadStreamEndian &stream, uint16 version = 2);

	ButtonType buttonType;
};

class ScriptCast : public Cast {
public:
	ScriptCast(Common::ReadStreamEndian &stream, uint16 version = 2);

	uint32 id;
};



struct CastInfo {
	Common::String script;
	Common::String name;
	Common::String directory;
	Common::String fileName;
	Common::String type;
};

struct Label {
	Common::String name;
	uint16 number;
	Label(Common::String name1, uint16 number1) { name = name1; number = number1; }
};

} // End of namespace Director

#endif
