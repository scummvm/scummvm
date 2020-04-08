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

namespace Director {

class BitmapCast;
class ButtonCast;
class ShapeCast;
class TextCast;

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
	kPalettePosition = 15
};

class Sprite {
public:
	Sprite();
	Sprite(const Sprite &sprite);
	~Sprite();

	uint16 getPattern();
	void setPattern(uint16 pattern);

	uint16 _scriptId;
	uint16 _scriptCastIndex;
	byte _colorcode;  // x40 editable, 0x80 moveable
	byte _blendAmount;
	uint32 _unk3;

	bool _enabled;
	uint16 _castId;
	uint16 _castIndex;
	byte _spriteType;
	byte _inkData;
	InkType _ink;
	uint16 _trails;

	Cast *_cast;

	byte _thickness;
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

	// Using in text sprites
	Common::String _editableText;
};

} // End of namespace Director

#endif
