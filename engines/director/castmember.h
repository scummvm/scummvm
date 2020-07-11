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

#ifndef DIRECTOR_CASTMEMBER_H
#define DIRECTOR_CASTMEMBER_H

#include "graphics/font.h"

#include "director/archive.h"

namespace Graphics {
struct Surface;
class FloodFill;
class MacText;
class MacWindowManager;
class MacButton;
class MacWidget;
}

namespace Common {
class SeekableReadStream;
class ReadStreamEndian;
}

namespace Image {
class ImageDecoder;
}

namespace Director {

class Stxt;
class AudioDecoder;
struct Resource;

class CastMember {
public:
	CastMember(Cast *cast, uint16 castId);
	virtual ~CastMember();

	Cast *getCast() { return _cast; }
	uint16 getID() { return _castId; }

	virtual bool isEditable() { return false; }
	virtual bool setEditable(bool editable) { return false; }
	virtual bool isModified() { return _modified; }
	virtual void createWidget() {}

	virtual void setColors(int *fgcolor, int *bgcolor) { return; }
	virtual uint getForeColor() { return 0; }
	virtual uint getBackColor() { return 0; }

	CastType _type;
	Common::Rect _initialRect;
	Common::Rect _boundingRect;
	Common::Array<Resource> _children;

	bool _modified;
	bool _hilite;
	bool _autoHilite;

	Graphics::MacWidget *_widget;

private:
	Cast *_cast;
	uint16 _castId;
};

class BitmapCastMember : public CastMember {
public:
	BitmapCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint32 castTag, uint16 version);
	~BitmapCastMember();
	virtual void createWidget() override;

	void createMatte();
	Graphics::Surface *getMatte();

	Image::ImageDecoder *_img;
	Graphics::FloodFill *_matte;

	uint16 _pitch;
	uint16 _regX;
	uint16 _regY;
	uint8 _flags1;
	uint16 _flags2;
	uint16 _bytes;
	PaletteType _clut;

	uint16 _bitsPerPixel;

	uint32 _tag;
};

class DigitalVideoCastMember : public CastMember {
public:
	DigitalVideoCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version);
	~DigitalVideoCastMember();

	bool _looping;
	bool _pausedAtStart;
	bool _enableVideo;
	bool _enableSound;
	bool _crop;
	bool _center;
	bool _preload;
	bool _showControls;
	bool _directToStage;
	FrameRateType _frameRateType;

	uint16 _frameRate;
};

class SoundCastMember : public CastMember {
public:
	SoundCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version);

	bool _looping;
	AudioDecoder *_audio;
};

class ShapeCastMember : public CastMember {
public:
	ShapeCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version);
	virtual uint getForeColor() override { return _fgCol; }
	virtual uint getBackColor() override { return _bgCol; }

	ShapeType _shapeType;
	uint16 _pattern;
	byte _fillType;
	byte _lineThickness;
	byte _lineDirection;
	InkType _ink;

private:
	byte _fgCol;
	byte _bgCol;
};

class TextCastMember : public CastMember {
public:
	TextCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version, bool asButton = false);
	virtual void setColors(int *fgcolor, int *bgcolor) override;

	void setText(const char *text);
	virtual void createWidget() override;

	virtual bool isModified() override;
	virtual bool isEditable() override;
	virtual bool setEditable(bool editable) override;
	Graphics::TextAlign getAlignment();

	virtual uint getBackColor() override { return _bgcolor; }
	virtual uint getForeColor() override { return _fgcolor; }

	SizeType _borderSize;
	SizeType _gutterSize;
	SizeType _boxShadow;

	byte _flags;
	uint32 _fontId;
	uint16 _fontSize;
	TextType _textType;
	TextAlignType _textAlign;
	SizeType _textShadow;
	byte _textSlant;
	byte _textFlags;
	uint16 _bgpalinfo1, _bgpalinfo2, _bgpalinfo3;
	uint16 _fgpalinfo1, _fgpalinfo2, _fgpalinfo3;
	ButtonType _buttonType;

	Common::String _ftext;
	Common::String _ptext;
	void importStxt(const Stxt *stxt);
	void importRTE(byte* text);

	Common::String getText();

private:
	uint _bgcolor;
	uint _fgcolor;
};

class ScriptCastMember : public CastMember {
public:
	ScriptCastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version);

	uint32 _id;
	ScriptType _scriptType;
};

class RTECastMember : public TextCastMember {
public:
	RTECastMember(Cast *cast, uint16 castId, Common::ReadStreamEndian &stream, uint16 version);

	void loadChunks();
};

struct CastMemberInfo {
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
