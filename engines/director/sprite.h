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
};

class Sprite {
public:
	Sprite(Frame *frame = nullptr);
	Sprite(const Sprite &sprite);
	Sprite& operator=(const Sprite &sprite);
	~Sprite();

	Frame *getFrame() const { return _frame; }
	Score *getScore() const { return _score; }

	void reset();

	void updateEditable();

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

	byte _blend;

	byte _volume;
	bool _stretch;
};

} // End of namespace Director

#endif
