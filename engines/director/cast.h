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

#include "director/archive.h"
#include "director/types.h"

#include "graphics/font.h"

namespace Graphics {
struct Surface;
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

class Cast {
public:
	Cast();
	virtual ~Cast() {};
	virtual bool isEditable() { return false; }
	virtual bool setEditable(bool editable) { return false; }
	virtual bool isModified() { return _modified; }
	virtual void createWidget();

	virtual void setColors(int *fgcolor, int *bgcolor) { return; }
	virtual void getColors(int *fgcolor, int *bgcolor) { return; }

	CastType _type;
	Score *_score;
	Common::Rect _initialRect;
	Common::Rect _boundingRect;
	Common::Array<Resource> _children;

	bool _modified;
	bool _hilite;

	Graphics::MacWidget *_widget;
};

class BitmapCast : public Cast {
public:
	BitmapCast(Common::ReadStreamEndian &stream, uint32 castTag, uint16 version);
	~BitmapCast();
	virtual void createWidget() override;
	// virtual void setColors(int *fgcolor, int *bgcolor) override;

	Image::ImageDecoder *_img;

	uint16 _pitch;
	uint16 _regX;
	uint16 _regY;
	uint8 _flags;
	uint16 _bytes;
	uint16 _clut;

	uint16 _bitsPerPixel;

	uint32 _tag;
};

class DigitalVideoCast : public Cast {
public:
	DigitalVideoCast(Common::ReadStreamEndian &stream, uint16 version);

	bool _looping;
	bool _pauseAtStart;
	bool _enableVideo;
	bool _enableSound;
	bool _enableCrop;
	bool _center;
	bool _preload;
	bool _showControls;
	FrameRateType _frameRateType;

	uint16 _frameRate;
};

class SoundCast : public Cast {
public:
	SoundCast(Common::ReadStreamEndian &stream, uint16 version);

	bool _looping;
	AudioDecoder *_audio;
};

class ShapeCast : public Cast {
public:
	ShapeCast(Common::ReadStreamEndian &stream, uint16 version);
	ShapeCast();

	ShapeType _shapeType;
	uint16 _pattern;
	byte _fgCol;
	byte _bgCol;
	byte _fillType;
	byte _lineThickness;
	byte _lineDirection;
	InkType _ink;
};

class TextCast : public Cast {
public:
	TextCast(Common::ReadStreamEndian &stream, uint16 version, bool asButton = false);
	virtual void setColors(int *fgcolor, int *bgcolor) override;
	virtual void getColors(int *fgcolor, int *bgcolor) override;

	void setText(const char *text);
	virtual void createWidget() override;

	virtual bool isModified() override;
	virtual bool isEditable() override;
	virtual bool setEditable(bool editable) override;
	Graphics::TextAlign getAlignment();

	uint getBackColor() { return _bgcolor; }
	uint getForeColor() { return _fgcolor; }

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

class ScriptCast : public Cast {
public:
	ScriptCast(Common::ReadStreamEndian &stream, uint16 version);

	uint32 _id;
	ScriptType _scriptType;
};

class RTECast : public TextCast {
public:
	RTECast(Common::ReadStreamEndian &stream, uint16 version);

	void loadChunks();
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
