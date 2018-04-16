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

#ifndef DIRECTOR_SPRITE_H
#define DIRECTOR_SPRITE_H

#include "common/rect.h"

namespace Director {

enum InkType {
	kInkTypeCopy,
	kInkTypeTransparent,
	kInkTypeReverse,
	kInkTypeGhost,
	kInkTypeNotCopy,
	kInkTypeNotTrans,
	kInkTypeNotReverse,
	kInkTypeNotGhost,
	kInkTypeMatte,
	kInkTypeMask,
	//10-31 Not used (Lingo in a Nutshell)
	kInkTypeBlend = 32,
	kInkTypeAddPin,
	kInkTypeAdd,
	kInkTypeSubPin,
	kInkTypeBackgndTrans,
	kInkTypeLight,
	kInkTypeSub,
	kInkTypeDark
};

// Director v4
enum SpriteType {
	kInactiveSprite, // turns the sprite off
	kBitmapSprite,
	kRectangleSprite,
	kRoundedRectangleSprite,
	kOvalSprite,
	kLineTopBottomSprite, // line from top left to bottom right
	kLineBottomTopSprite, // line from bottom left to top right
	kTextSprite,
	kButtonSprite,
	kCheckboxSprite,
	kRadioButtonSprite,
	kUndeterminedSprite = 16 // use castType property to examine the type of cast member associated with sprite
};

enum SpritePosition {
	kSpritePositionUnk1 = 0,
	kSpritePositionEnabled,
	kSpritePositionUnk2,
	kSpritePositionFlags = 4,
	kSpritePositionCastId = 6,
	kSpritePositionY = 8,
	kSpritePositionX = 10,
	kSpritePositionHeight = 12,
	kSpritePositionWidth = 14
};

enum MainChannelsPosition {
	kScriptIdPosition = 0,
	kSoundType1Position,
	kTransFlagsPosition,
	kTransChunkSizePosition,
	kTempoPosition,
	kTransTypePosition,
	kSound1Position,
	kSkipFrameFlagsPosition = 8,
	kBlendPosition,
	kSound2Position,
	kSound2TypePosition = 11,
	kPaletePosition = 15
};

class Sprite {
public:
	Sprite();
	Sprite(const Sprite &sprite);
	~Sprite();

	byte _x1;
	uint16 _x2;
	uint16 _scriptId;
	byte _flags2;  // x40 editable, 0x80 moveable
	byte _unk2;
	uint32 _unk3;

	bool _enabled;
	uint16 _castId;
	byte _spriteType;
	InkType _ink;
	uint16 _trails;

	BitmapCast *_bitmapCast;
	ShapeCast *_shapeCast;
	//SoundCast *_soundCast;
	TextCast *_textCast;
	ButtonCast *_buttonCast;
	//ScriptCast *_scriptCast;

	uint16 _flags;
	Common::Point _startPoint;
	uint16 _width;
	uint16 _height;
	// TODO: default constraint = 0, if turned on, sprite is constrainted to the bounding rect
	// As i know, constrainted != 0 only if sprite moveable
	byte _constraint;
	byte _moveable;
	byte _backColor;
	byte _foreColor;
	uint16 _left;
	uint16 _right;
	uint16 _top;
	uint16 _bottom;
	byte _blend;
	bool _visible;
	SpriteType _type;
	// Using in digital movie sprites
	byte _movieRate;
	uint16 _movieTime;
	uint16 _startTime;
	uint16 _stopTime;
	byte _volume;
	byte _stretch;
	// Using in shape sprites
	byte _lineSize;
	// Using in text sprites
	Common::String _editableText;
};

} // End of namespace Director

#endif
