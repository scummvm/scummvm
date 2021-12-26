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

#ifndef DIRECTOR_SPRITE_H
#define DIRECTOR_SPRITE_H

namespace Director {

class Frame;
class BitmapCastMember;
class ShapeCastMember;
class TextCastMember;

enum SpritePosition {
	kSpritePositionUnk1 = 0,
	kSpritePositionEnabled = 1,
	kSpritePositionUnk2 = 2,
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
	Sprite(Frame *frame = nullptr);
	Sprite(const Sprite &sprite);
	Sprite& operator=(const Sprite &sprite);
	~Sprite();

	Frame *getFrame() const { return _frame; }
	Score *getScore() const { return _score; }

	void updateEditable();

	bool respondsToMouse();
	bool isActive();
	bool shouldHilite();
	bool checkSpriteType();

	uint16 getPattern();
	void setPattern(uint16 pattern);

	void setCast(CastMemberID memberID);
	bool isQDShape();
	Graphics::Surface *getQDMatte();
	void createQDMatte();
	MacShape *getShape();
	uint32 getForeColor();
	uint32 getBackColor();
	Common::Point getRegistrationOffset();

	Frame *_frame;
	Score *_score;
	Movie *_movie;

	Graphics::FloodFill *_matte; // matte for quickdraw shape

	CastMemberID _scriptId;
	byte _colorcode; // x40 editable, 0x80 moveable
	byte _blendAmount;
	uint32 _unk3;

	bool _enabled;
	SpriteType _spriteType;
	byte _inkData;
	InkType _ink;
	uint16 _trails;

	CastMemberID _castId;
	uint16 _pattern;
	CastMember *_cast;

	byte _thickness;
	Common::Point _startPoint;
	int16 _width;
	int16 _height;
	bool _moveable;
	bool _editable;
	bool _puppet;
	bool _immediate;
	uint32 _backColor;
	uint32 _foreColor;

	byte _blend;

	byte _volume;
	byte _stretch;
};

} // End of namespace Director

#endif
