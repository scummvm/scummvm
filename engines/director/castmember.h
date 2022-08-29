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

#ifndef DIRECTOR_CASTMEMBER_H
#define DIRECTOR_CASTMEMBER_H

#include "graphics/font.h"

#include "director/archive.h"
#include "director/sprite.h"
#include "director/stxt.h"

#include "director/lingo/lingo-object.h"

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
class SeekableReadStreamEndian;
}

namespace Image {
class ImageDecoder;
}

namespace Video {
class VideoDecoder;
}

namespace Director {

class AudioDecoder;
struct CastMemberInfo;
class Channel;
struct Resource;
class Sprite;
class Stxt;

class CastMember : public Object<CastMember> {
public:
	CastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream);
	CastMember(Cast *cast, uint16 castId);
	virtual ~CastMember() {}

	Cast *getCast() { return _cast; }
	uint16 getID() { return _castId; }
	CastMemberInfo *getInfo();

	virtual bool isEditable() { return false; }
	virtual void setEditable(bool editable) {}
	virtual bool isModified() { return _modified; }
	void setModified(bool modified);
	virtual Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) { return nullptr; }
	virtual void updateWidget(Graphics::MacWidget *widget, Channel *channel) {}
	virtual void updateFromWidget(Graphics::MacWidget *widget) {}
	virtual Common::Rect getInitialRect() { return _initialRect; }

	virtual void setColors(uint32 *fgcolor, uint32 *bgcolor) { return; }
	virtual uint32 getForeColor() { return 0; }
	virtual void setForeColor(uint32 fgCol) { return; }
	virtual uint32 getBackColor() { return 0; }
	virtual void setBackColor(uint32 bgCol) { return; }

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	bool setProp(const Common::String &propName, const Datum &value) override;
	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	// release the control to widget, this happens when we are changing sprites. Because we are having the new cast member and the old one shall leave
	void releaseWidget() { _widget = nullptr; }

	CastType _type;
	Common::Rect _initialRect;
	Common::Rect _boundingRect;
	Common::Array<Resource> _children;

	bool _hilite;
	bool _erase;
	int _purgePriority;
	uint32 _size;
	uint8 _flags1;

protected:
	Cast *_cast;
	uint16 _castId;
	// a link to the widget we created, we may use it later
	Graphics::MacWidget *_widget;
	bool _modified;
	bool _isChanged;
};

class BitmapCastMember : public CastMember {
public:
	BitmapCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint32 castTag, uint16 version, uint8 flags1 = 0);
	BitmapCastMember(Cast *cast, uint16 castId, Image::ImageDecoder *img, uint8 flags1 = 0);
	~BitmapCastMember();
	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	void createMatte(Common::Rect &bbox);
	Graphics::Surface *getMatte(Common::Rect &bbox);
	void copyStretchImg(Graphics::Surface *surface, const Common::Rect &bbox, const byte *pal = 0);

	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	Image::ImageDecoder *_img;
	Graphics::Surface *_ditheredImg;
	Graphics::FloodFill *_matte;

	uint16 _pitch;
	uint16 _regX;
	uint16 _regY;
	uint16 _flags2;
	uint16 _bytes;
	int _clut;

	uint16 _bitsPerPixel;

	uint32 _tag;
	bool _noMatte;

private:
	void ditherImage();
	void ditherFloydImage();

	Graphics::PaletteLookup _paletteLookup;
};

class DigitalVideoCastMember : public CastMember {
public:
	DigitalVideoCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	~DigitalVideoCastMember();

	bool isModified() override;
	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	bool loadVideo(Common::String path);
	void startVideo(Channel *channel);
	void stopVideo(Channel *channel);

	uint getMovieCurrentTime();
	uint getDuration();
	uint getMovieTotalTime();
	void seekMovie(int stamp);
	void setStopTime(int stamp);
	void setMovieRate(double rate);
	void setFrameRate(int rate);

	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	Common::String _filename;

	uint32 _vflags;
	bool _looping;
	bool _pausedAtStart;
	bool _enableVideo;
	bool _enableSound;
	bool _crop;
	bool _center;
	bool _preload;
	bool _showControls;
	bool _directToStage;
	bool _avimovie, _qtmovie;
	FrameRateType _frameRateType;

	uint16 _frameRate;
	bool _getFirstFrame;
	int _duration;

	Video::VideoDecoder *_video;
	Graphics::Surface *_lastFrame;

	Channel *_channel;
};


struct FilmLoopFrame {
	Common::HashMap<int, Sprite> sprites;
};

class FilmLoopCastMember : public CastMember {
public:
	FilmLoopCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	~FilmLoopCastMember();

	bool isModified() override;
	//Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	Common::Array<Channel> *getSubChannels(Common::Rect &bbox, Channel *channel);

	void loadFilmLoopData(Common::SeekableReadStreamEndian &stream);

	bool _enableSound;
	bool _looping;
	bool _crop;
	bool _center;

	Common::Array<FilmLoopFrame> _frames;
	Common::Array<Channel> _subchannels;
};

class MovieCastMember : public CastMember {
public:
	MovieCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);

	uint32 _flags;
	bool _looping;
	bool _enableScripts;
	bool _enableSound;
	bool _crop;
	bool _center;
};

class SoundCastMember : public CastMember {
public:
	SoundCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	~SoundCastMember();

	bool _looping;
	AudioDecoder *_audio;
};

class ShapeCastMember : public CastMember {
public:
	ShapeCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	uint32 getForeColor() override { return _fgCol; }
	uint32 getBackColor() override { return _bgCol; }
	void setBackColor(uint32 bgCol) override;
	void setForeColor(uint32 fgCol) override;

	ShapeType _shapeType;
	uint16 _pattern;
	byte _fillType;
	byte _lineThickness;
	byte _lineDirection;
	InkType _ink;

private:
	uint32 _fgCol;
	uint32 _bgCol;
};

class TextCastMember : public CastMember {
public:
	TextCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version, uint8 flags1 = 0, bool asButton = false);
	void setColors(uint32 *fgcolor, uint32 *bgcolor) override;

	void setText(const Common::U32String &text);
	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	bool isEditable() override { return _editable; }
	void setEditable(bool editable) override { _editable = editable; }
	void updateFromWidget(Graphics::MacWidget *widget) override;
	Graphics::TextAlign getAlignment();

	uint32 getBackColor() override { return _bgcolor; }
	void setBackColor(uint32 bgCol) override;
	uint32 getForeColor() override { return _fgcolor; }
	void setForeColor(uint32 fgCol) override;

	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	bool hasChunkField(int field);
	Datum getChunkField(int field, int start, int end);
	bool setChunkField(int field, int start, int end, const Datum &value);

	int getTextHeight();

	int getTextSize();
	void setTextSize(int textSize);

	SizeType _borderSize;
	SizeType _gutterSize;
	SizeType _boxShadow;
	uint16 _maxHeight;
	uint16 _textHeight;

	uint32 _fontId;
	uint16 _fontSize;
	TextType _textType;
	TextAlignType _textAlign;
	SizeType _textShadow;
	uint16 _scroll;
	byte _textSlant;
	byte _textFlags;
	uint16 _bgpalinfo1, _bgpalinfo2, _bgpalinfo3;
	uint16 _fgpalinfo1, _fgpalinfo2, _fgpalinfo3;
	ButtonType _buttonType;
	bool _editable;
	int _lineSpacing;

	Common::U32String _ftext;
	Common::U32String _ptext;
	void importStxt(const Stxt *stxt);
	void importRTE(byte *text);

	Common::U32String getText();

private:
	uint32 _bgcolor;
	uint32 _fgcolor;
};

class ScriptCastMember : public CastMember {
public:
	ScriptCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);

	ScriptType _scriptType;
};

class RTECastMember : public TextCastMember {
public:
	RTECastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);

	void loadChunks();
};

struct EditInfo {
	Common::Rect rect;
	int32 selStart;
	int32 selEnd;
	byte version;
	byte rulerFlag;
};

struct CastMemberInfo {
	bool autoHilite;
	uint32 scriptId;
	Common::String script;
	Common::String name;
	Common::String directory;
	Common::String fileName;
	Common::String type;
	EditInfo scriptEditInfo;
	FontStyle scriptStyle;
	EditInfo textEditInfo;
	Common::String modifiedBy;
	Common::String comments;

	CastMemberInfo() : autoHilite(false), scriptId(0) {}
};

struct Label {
	Common::String comment;
	Common::String name;
	uint16 number;
	Label(Common::String name1, uint16 number1, Common::String comment1) { name = name1; number = number1; comment = comment1;}
};

class PaletteCastMember : public CastMember {
public:
	PaletteCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	int getPaletteId() { return _palette ? _palette->id : 0; }
	void activatePalette() { if (_palette) g_director->setPalette(_palette->id); }

	PaletteV4 *_palette;
};

} // End of namespace Director

#endif
