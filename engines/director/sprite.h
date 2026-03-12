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

#include "director/spriteinfo.h"

namespace Director {

class Frame;
class BitmapCastMember;
class ShapeCastMember;
class TextCastMember;

/* Director in a Nutshell, page 15:
The following properties of a sprite are auto-puppeted whenever the property is
set: backColor, blend, editable, foreColor, beight, ink, loc, locH, locV, member,
moveable, rect, and width Auto-puppeting of individual properties has no effect
on the puppet of sprite property. */
enum AutoPuppetProperty {
	kAPNone = 0,
	kAPCast,
	kAPBackColor,
	kAPBbox,
	kAPBlend,
	kAPEditable,
	kAPForeColor,
	kAPHeight,
	kAPInk,
	kAPLoc,
	kAPLocH,
	kAPLocV,
	kAPMember,
	kAPMoveable,
	kAPRect,
	kAPWidth,
	kAPThickness,
};

enum ThicknessFlags {
	kTThickness = 0x0F,
	kTHasBlend  = 0x10,
	kTFlipH     = 0x20,
	kTFlipV     = 0x40,
	kTFlip      = (kTFlipH | kTFlipV),
	kTTweened   = 0x80,
};

// Director treats changes of sprites between score frames as deltas.
// Only the delta is applied to what's on the screen.
// If a sprite has the puppet flag, or a field has been autopuppeted,
// then that will block the sprite/fields from being updated by the score.
// In addition, the program can turn off the puppet flag at any time, which
// will revert the sprite to whatever was in the score.

// In order to keep a single frame read and copying pass, when reading the frame
// data we keep track of what fields have changed, so that the frame can be
// stored as a full copy but applied as a delta.

enum SpriteCopyBackMask {
	kSCBNoMask = -1,
	kSCBScriptId = 0x00001,
	kSCBSpriteType = 0x00002,
	kSCBEnabled = 0x00004,
	kSCBForeColor = 0x00008,
	kSCBBackColor = 0x00010,
	kSCBThickness = 0x00020,
	kSCBInk = 0x00040,
	kSCBPattern = 0x00080,
	kSCBCastId = 0x00100,
	kSCBStartPoint = 0x00200,
	kSCBHeight = 0x00400,
	kSCBWidth = 0x00800,
	kSCBMoveable = 0x01000,
	kSCBBlendAmount = 0x02000,
	kSCBSpriteListIdx = 0x04000,
	kSCBFlags = 0x08000,
	kSCBAngle = 0x10000,
};

class Sprite {
public:
	Sprite(Frame *frame = nullptr);
	Sprite(const Sprite &sprite);
	Sprite& operator=(const Sprite &sprite);
	bool operator==(const Sprite &sprite);
	~Sprite();

	Frame *getFrame() const { return _frame; }
	Score *getScore() const { return _score; }

	void reset();

	bool getEditable();

	bool respondsToMouse();
	bool isActive();
	bool shouldHilite();
	bool checkSpriteType();

	uint16 getPattern();
	void setPattern(uint16 pattern);

	void setCast(CastMemberID memberID, bool replaceDims = true);
	bool isQDShape();
	Graphics::Surface *getQDMatte();
	void createQDMatte();
	MacShape *getShape();
	uint32 getForeColor();
	uint32 getBackColor();
	void setAutoPuppet(AutoPuppetProperty property, bool value);
	bool getAutoPuppet(AutoPuppetProperty property);

	inline int getWidth() { return _width; }
	void setWidth(int w);
	inline int getHeight() { return _height; }
	void setHeight(int h);

	Common::Rect getBbox(bool unstretched);
	void setBbox(int l, int t, int r, int b);

	Common::Point getPosition();
	void setPosition(int x, int y);

	Common::String formatInfo();

	void replaceFrom(Sprite *nextSprite);

	Frame *_frame;
	Score *_score;
	Movie *_movie;

	Graphics::FloodFill *_matte; // matte for quickdraw shape

	uint32 _copyBackMask;

	CastMemberID _scriptId;
	byte _colorcode; // x40 editable, 0x80 moveable
	byte _blendAmount;
	uint32 _unk3;

	bool _enabled;
	SpriteType _spriteType;
	byte _inkData;
	InkType _ink;
	bool _trails;

	CastMemberID _castId;
	uint16 _pattern;
	CastMember *_cast;

	byte _thickness;

	// These fields are used for tracking the position, width and height of the sprite,
	// as received from the score frame data.
	// Don't change these; instead adjust the equivalent properties in Channel.
	Common::Point _startPoint;
	int16 _width;
	int16 _height;

	bool _moveable;
	bool _editable;
	bool _puppet;
	uint32 _autoPuppet; // autopuppet, based upon Director in a Nutshell, page 15
	bool _immediate;
	uint32 _backColor;
	uint32 _foreColor;

	byte _volume;
	bool _stretch;

	uint32 _spriteListIdx;	 // D6+
	SpriteInfo _spriteInfo; // D6+

	// D7+
	byte _flags;
	byte _fgColorG, _fgColorB;		// R component sits in _foreColor
	byte _bgColorG, _bgColorB;		// R component sits in _backColor
	int32 _angleRot;
	int32 _angleSkew;

	Common::Array<BehaviorElement> _behaviors; // D6+
};

} // End of namespace Director

#endif
