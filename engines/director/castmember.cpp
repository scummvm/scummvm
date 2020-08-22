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

#include "graphics/macgui/macbutton.h"
#include "image/image_decoder.h"
#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/castmember.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/stxt.h"

namespace Director {

CastMember::CastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream) : Object<CastMember>("CastMember") {
	_type = kCastTypeNull;
	_cast = cast;
	_castId = castId;
	_hilite = false;
	_autoHilite = false;
	_purgePriority = 3;
	_size = stream.size();
	_flags1 = 0;

	_modified = true;

	_objType = kCastMemberObj;
}


/////////////////////////////////////
// Bitmap
/////////////////////////////////////

BitmapCastMember::BitmapCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint32 castTag, uint16 version, uint8 flags1)
		: CastMember(cast, castId, stream) {
	_type = kCastBitmap;
	_img = nullptr;
	_matte = nullptr;
	_noMatte = false;
	_bytes = 0;
	_pitch = 0;
	_flags2 = 0;
	_regX = _regY = 0;
	_clut = kClutSystemMac;
	_bitsPerPixel = 0;

	if (version < 400) {
		_flags1 = flags1;	// region: 0 - auto, 1 - matte, 2 - disabled, 8 - no auto
		if (_flags1 >> 4 == 0x0)
			_autoHilite = true;

		_bytes = stream.readUint16();
		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
		_regY = stream.readUint16();
		_regX = stream.readUint16();

		if (_bytes & 0x8000) {
			_bitsPerPixel = stream.readUint16();
			_clut = stream.readSint16() - 1;
		} else {
			_bitsPerPixel = 1;
			_clut = kClutSystemMac;
		}

		_pitch = _initialRect.width();
		if (_pitch % 16)
			_pitch += 16 - (_initialRect.width() % 16);

	} else if (version >= 400 && version < 500) {
		_flags1 = flags1;
		_pitch = stream.readUint16();
		_pitch &= 0x0fff;

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
		_regY = stream.readUint16();
		_regX = stream.readUint16();

		_bitsPerPixel = stream.readUint16();

		if (stream.eos()) {
			_bitsPerPixel = 0;
		} else {
			_clut = stream.readSint16() - 1;
			stream.readUint16();
			/* uint16 unk1 = */ stream.readUint16();
			stream.readUint16();

			stream.readUint32();
			stream.readUint32();

			_flags2 = stream.readUint16();
		}

		if (_bitsPerPixel == 0)
			_bitsPerPixel = 1;

		if (_bitsPerPixel == 1)
			_pitch *= 8;

		_autoHilite = (_flags2 % 4 != 0);

		int tail = 0;
		byte buf[256];

		while (!stream.eos()) {
			byte c = stream.readByte();
			if (tail < 256)
				buf[tail] = c;
			tail++;
		}

		if (tail)
			warning("BUILDBOT: BitmapCastMember: %d bytes left", tail);

		if (tail && debugChannelSet(2, kDebugLoading)) {
			debug("BitmapCastMember: tail");
			Common::hexdump(buf, tail);
		}
	} else if (version >= 500) {
		uint16 count = stream.readUint16();
		for (uint16 cc = 0; cc < count; cc++)
			stream.readUint32();

		uint32 stringLength = stream.readUint32();
		for (uint32 s = 0; s < stringLength; s++)
			stream.readByte();

		/*uint16 width =*/ stream.readUint16LE(); //maybe?
		_initialRect = Movie::readRect(stream);

		/*uint32 somethingElse =*/ stream.readUint32();
		_boundingRect = Movie::readRect(stream);

		_bitsPerPixel = stream.readUint16();

		stream.readUint32();
	}

	_tag = castTag;
}

BitmapCastMember::~BitmapCastMember() {
	if (_img)
		delete _img;

	if (_matte)
		delete _matte;
}

Graphics::MacWidget *BitmapCastMember::createWidget(Common::Rect &bbox, Channel *channel) {
	if (!_img) {
		warning("BitmapCastMember::createWidget: No image decoder");
		return nullptr;
	}

	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow(), bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, false);
	widget->getSurface()->blitFrom(*_img->getSurface());
	return widget;
}

void BitmapCastMember::createMatte() {
	// Like background trans, but all white pixels NOT ENCLOSED by coloured pixels
	// are transparent
	Graphics::Surface tmp;
	tmp.create(_initialRect.width(), _initialRect.height(), g_director->_pixelformat);
	tmp.copyFrom(*_img->getSurface());

	_noMatte = true;

	// Searching white color in the corners
	uint32 whiteColor = 0;
	bool colorFound = false;

	if (g_director->_pixelformat.bytesPerPixel == 1) {
		for (int y = 0; y < tmp.h; y++) {
			for (int x = 0; x < tmp.w; x++) {
				byte color = *(byte *)tmp.getBasePtr(x, y);

				if (g_director->getPalette()[color * 3 + 0] == 0xff &&
						g_director->getPalette()[color * 3 + 1] == 0xff &&
						g_director->getPalette()[color * 3 + 2] == 0xff) {
					whiteColor = color;
					colorFound = true;
					break;
				}
			}
		}
	} else {
		whiteColor = g_director->_wm->_colorWhite;
		colorFound = true;
	}

	if (!colorFound) {
		debugC(1, kDebugImages, "BitmapCastMember::createMatte(): No white color for matte image");
	} else {
		delete _matte;

		_matte = new Graphics::FloodFill(&tmp, whiteColor, 0, true);

		for (int yy = 0; yy < tmp.h; yy++) {
			_matte->addSeed(0, yy);
			_matte->addSeed(tmp.w - 1, yy);
		}

		for (int xx = 0; xx < tmp.w; xx++) {
			_matte->addSeed(xx, 0);
			_matte->addSeed(xx, tmp.h - 1);
		}

		_matte->fillMask();
		_noMatte = false;
	}

	tmp.free();
}

Graphics::Surface *BitmapCastMember::getMatte() {
	// Lazy loading of mattes
	if (!_matte && !_noMatte) {
		createMatte();
	}

	return _matte ? _matte->getMask() : nullptr;
}


/////////////////////////////////////
// DigitalVideo
/////////////////////////////////////

DigitalVideoCastMember::DigitalVideoCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastDigitalVideo;
	_video = nullptr;
	_lastFrame = nullptr;

	_getFirstFrame = false;
	_duration = 0;

	_initialRect = Movie::readRect(stream);
	_vflags = stream.readUint32();
	_frameRate = (_vflags >> 24) & 0xff;

	_frameRateType = kFrameRateDefault;
	if (_vflags & 0x0800) {
		_frameRateType = (FrameRateType)((_vflags & 0x3000) >> 12);
	}
	_qtmovie = _vflags & 0x8000;
	_avimovie = _vflags & 0x4000;
	_preload = _vflags & 0x0400;
	_enableVideo = !(_vflags & 0x0200);
	_pausedAtStart = _vflags & 0x0100;
	_showControls = _vflags & 0x40;
	_directToStage = _vflags & 0x20;
	_looping = _vflags & 0x10;
	_enableSound = _vflags & 0x08;
	_crop = !(_vflags & 0x02);
	_center = _vflags & 0x01;

	if (debugChannelSet(2, kDebugLoading))
		_initialRect.debugPrint(2, "DigitalVideoCastMember(): rect:");

	debugC(2, kDebugLoading, "DigitalVideoCastMember(): flags: (%d 0x%04x)", _vflags, _vflags);

	debugC(2, kDebugLoading, "_frameRate: %d", _frameRate);
	debugC(2, kDebugLoading, "_frameRateType: %d, _preload: %d, _enableVideo %d, _pausedAtStart %d",
			_frameRateType, _preload, _enableVideo, _pausedAtStart);
	debugC(2, kDebugLoading, "_showControls: %d, _looping: %d, _enableSound: %d, _crop %d, _center: %d, _directToStage: %d",
			_showControls, _looping, _enableSound, _crop, _center, _directToStage);
	debugC(2, kDebugLoading, "_avimovie: %d, _qtmovie: %d", _avimovie, _qtmovie);
}

DigitalVideoCastMember::~DigitalVideoCastMember() {
	delete _video;

	if (g_director->_pixelformat.bytesPerPixel != 1)
		delete _lastFrame;
}

bool DigitalVideoCastMember::loadVideo(Common::String path) {
	// TODO: detect file type (AVI, QuickTime, FLIC) based on magic number,
	// insert the right video decoder

	_filename = path;
	_video = new Video::QuickTimeDecoder();

	debugC(2, kDebugLoading | kDebugImages, "Loading video %s", path.c_str());

	return _video->loadFile(path);
}

bool DigitalVideoCastMember::isModified() {
	if (!_video || !_video->isVideoLoaded())
		return true;

	if (_getFirstFrame)
		return true;

	if (_channel->_movieRate == 0.0)
		return false;

	return _video->needsUpdate();
}

void DigitalVideoCastMember::startVideo(Channel *channel) {
	_channel = channel;

	if (_pausedAtStart) {
		_getFirstFrame = true;
	} else {
		if (_channel->_movieRate == 0.0)
			_channel->_movieRate = 1.0;
	}

	if (_video->isPlaying())
		_video->rewind();
	else
		_video->start();

	debugC(2, kDebugImages, "STARTING VIDEO %s", _filename.c_str());

	if (_channel->_stopTime == 0)
		_channel->_stopTime = getMovieTotalTime();

	_duration = getMovieTotalTime();
}

Graphics::MacWidget *DigitalVideoCastMember::createWidget(Common::Rect &bbox, Channel *channel) {
	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow(), bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, false);

	_channel = channel;

	if (!_video || !_video->isVideoLoaded()) {
		warning("DigitalVideoCastMember::createWidget: No video decoder");
		delete widget;

		return nullptr;
	}

	// Do not render stopped videos
	if (_channel->_movieRate == 0.0 && !_getFirstFrame) {
		widget->getSurface()->blitFrom(*_lastFrame);

		return widget;
	}

	debugC(1, kDebugImages, "Video time: %d  rate: %f", _channel->_movieTime, _channel->_movieRate);
	const Graphics::Surface *frame = _video->decodeNextFrame();

	_channel->_movieTime = getMovieCurrentTime();

	if (frame) {
		if (g_director->_pixelformat.bytesPerPixel == 1) {
			if (frame->format.bytesPerPixel != 1) {
				warning("STUB: video >8bpp");
			} else {
				_lastFrame = frame;
				widget->getSurface()->blitFrom(*frame);
			}
		} else {
			delete _lastFrame;
			_lastFrame = frame->convertTo(g_director->_pixelformat, g_director->getPalette());
			widget->getSurface()->blitFrom(*_lastFrame);
		}
	} else {
		widget->getSurface()->blitFrom(*_lastFrame);
	}

	if (_getFirstFrame) {
		_video->stop();
		_getFirstFrame = false;
	}

	if (_video->endOfVideo()) {
		if (_looping) {
			_video->rewind();
		} else {
			_channel->_movieRate = 0.0;
		}
	}

	return widget;
}

uint DigitalVideoCastMember::getMovieCurrentTime() {
	if (!_video)
		return 0;

	int stamp = MIN<int>(_video->getTime() * 60 / 1000, getMovieTotalTime());

	return stamp;
}

uint DigitalVideoCastMember::getMovieTotalTime() {
	if (!_video)
		return 0;

	int stamp = _video->getDuration().msecs() * 60 / 1000;

	return stamp;
}

void DigitalVideoCastMember::seekMovie(int stamp) {
	if (!_video)
		return;

	_channel->_startTime = stamp;

	Audio::Timestamp dur = _video->getDuration();

	_video->seek(Audio::Timestamp(_channel->_startTime * 1000 / 60, dur.framerate()));
}

void DigitalVideoCastMember::setStopTime(int stamp) {
	if (!_video)
		return;

	_channel->_stopTime = stamp;

	Audio::Timestamp dur = _video->getDuration();

	_video->setEndTime(Audio::Timestamp(_channel->_stopTime * 1000 / 60, dur.framerate()));
}

void DigitalVideoCastMember::setMovieRate(double rate) {
	if (!_video)
		return;

	_channel->_movieRate = rate;

	if (rate < 0.0)
		warning("STUB: DigitalVideoCastMember::setMovieRate(%g)", rate);
	else
		_video->setRate(Common::Rational((int)(rate * 100.0), 100));

	if (_video->endOfVideo())
		_video->rewind();
}

void DigitalVideoCastMember::setFrameRate(int rate) {
	if (!_video)
		return;

	warning("STUB: DigitalVideoCastMember::setFrameRate(%d)", rate);
}


/////////////////////////////////////
// Sound
/////////////////////////////////////

SoundCastMember::SoundCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastSound;
	_audio = nullptr;
	_looping = 0;
}


/////////////////////////////////////
// Text
/////////////////////////////////////

TextCastMember::TextCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version, uint8 flags1, bool asButton)
		: CastMember(cast, castId, stream) {
	_type = kCastText;

	_borderSize = kSizeNone;
	_gutterSize = kSizeNone;
	_boxShadow = kSizeNone;
	_buttonType = kTypeButton;
	_editable = false;
	_maxHeight = _textHeight = 0;

	_bgcolor = 0;
	_fgcolor = 0;

	_textFlags = 0;
	_scroll = 0;
	_fontId = 1;
	_fontSize = 12;
	_textType = kTextTypeFixed;
	_textAlign = kTextAlignLeft;
	_textShadow = kSizeNone;
	_textSlant = 0;
	_bgpalinfo1 = _bgpalinfo2 = _bgpalinfo3 = 0;
	_fgpalinfo1 = _fgpalinfo2 = _fgpalinfo3 = 0xff;

	if (version < 400) {
		_flags1 = flags1; // region: 0 - auto, 1 - matte, 2 - disabled
		_borderSize = static_cast<SizeType>(stream.readByte());
		_gutterSize = static_cast<SizeType>(stream.readByte());
		_boxShadow = static_cast<SizeType>(stream.readByte());
		byte pad1 = stream.readByte();
		_textAlign = static_cast<TextAlignType>(stream.readUint16());
		_bgpalinfo1 = stream.readUint16();
		_bgpalinfo2 = stream.readUint16();
		_bgpalinfo3 = stream.readUint16();

		uint32 pad2;
		uint16 pad3;
		uint16 pad4 = 0;
		uint16 totalTextHeight;

		if (version >= 200 && version < 300) {
			pad2 = stream.readUint16();
			if (pad2 != 0) { // In D2 there are values
				warning("TextCastMember: pad2: %x", pad2);
			}

			_initialRect = Movie::readRect(stream);
			pad3 = stream.readUint16();

			_textShadow = static_cast<SizeType>(stream.readByte());
			_textFlags = stream.readByte();
			if (_textFlags & 0xf8)
				warning("Unprocessed text cast flags: %x", _textFlags & 0xf8);

			totalTextHeight = stream.readUint16();
		} else {
			pad2 = stream.readUint16();
			_initialRect = Movie::readRect(stream);
			pad3 = stream.readUint16();
			pad4 = stream.readUint16();
			totalTextHeight = stream.readUint16();
		}

		debugC(2, kDebugLoading, "TextCastMember(): flags1: %d, border: %d gutter: %d shadow: %d pad1: %x align: %04x",
				_flags1, _borderSize, _gutterSize, _boxShadow, pad1, _textAlign);
		debugC(2, kDebugLoading, "TextCastMember(): background rgb: 0x%04x 0x%04x 0x%04x, pad2: %x pad3: %d pad4: %d shadow: %d flags: %d totHeight: %d",
				_bgpalinfo1, _bgpalinfo2, _bgpalinfo3, pad2, pad3, pad4, _textShadow, _textFlags, totalTextHeight);
		if (debugChannelSet(2, kDebugLoading)) {
			_initialRect.debugPrint(2, "TextCastMember(): rect:");
		}
	} else if (version >= 400 && version < 500) {
		_flags1 = flags1;
		_borderSize = static_cast<SizeType>(stream.readByte());
		_gutterSize = static_cast<SizeType>(stream.readByte());
		_boxShadow = static_cast<SizeType>(stream.readByte());
		_textType = static_cast<TextType>(stream.readByte());
		_textAlign = static_cast<TextAlignType>(stream.readSint16()); // this is because 'right' is -1? or should that be 255?
		_bgpalinfo1 = stream.readUint16();
		_bgpalinfo2 = stream.readUint16();
		_bgpalinfo3 = stream.readUint16();
		_scroll = stream.readUint16();

		_fontId = 1; // this is in STXT

		_initialRect = Movie::readRect(stream);
		_maxHeight = stream.readUint16();
		_textShadow = static_cast<SizeType>(stream.readByte());
		_textFlags = stream.readByte();

		_textHeight = stream.readUint16();
		_textSlant = 0;
	} else {
		_fontId = 1;

		stream.readUint32();
		stream.readUint32();
		stream.readUint32();
		stream.readUint32();
		uint16 skip = stream.readUint16();
		for (int i = 0; i < skip; i++)
			stream.readUint32();

		stream.readUint32();
		stream.readUint32();
		stream.readUint32();
		stream.readUint32();
		stream.readUint32();
		stream.readUint32();

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);

		stream.readUint32();
		stream.readUint16();
		stream.readUint16();
	}

	if (asButton) {
		_type = kCastButton;

		if (version < 500) {
			_buttonType = static_cast<ButtonType>(stream.readUint16BE() - 1);
		} else {
			warning("TextCastMember(): Attempting to initialize >D4 button castmember");
			_buttonType = kTypeButton;
		}
	}

	_bgcolor = g_director->_wm->findBestColor(_bgpalinfo1 & 0xff, _bgpalinfo2 & 0xff, _bgpalinfo3 & 0xff);

	_modified = true;
}

void TextCastMember::setColors(uint32 *fgcolor, uint32 *bgcolor) {
	if (fgcolor)
		_fgcolor = *fgcolor;

	if (bgcolor)
		_bgcolor = *bgcolor;
}

Graphics::TextAlign TextCastMember::getAlignment() {
	switch (_textAlign) {
	case kTextAlignRight:
		return Graphics::kTextAlignRight;
	case kTextAlignCenter:
		return Graphics::kTextAlignCenter;
	case kTextAlignLeft:
	default:
		return Graphics::kTextAlignLeft;
	}
}

void TextCastMember::importStxt(const Stxt *stxt) {
	_fontId = stxt->_style.fontId;
	_textSlant = stxt->_style.textSlant;
	_fontSize = stxt->_style.fontSize;
	_fgpalinfo1 = stxt->_style.r;
	_fgpalinfo2 = stxt->_style.g;
	_fgpalinfo3 = stxt->_style.b;
	_ftext = stxt->_ftext;
	_ptext = stxt->_ptext;
}

Graphics::MacWidget *TextCastMember::createWidget(Common::Rect &bbox, Channel *channel) {
	Graphics::MacFont *macFont = new Graphics::MacFont(_fontId, _fontSize, _textSlant);
	Graphics::MacWidget *widget = nullptr;

	switch (_type) {
	case kCastText:
		widget = new Graphics::MacText(g_director->getCurrentWindow(), bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, _ftext, macFont, getForeColor(), getBackColor(), bbox.width(), getAlignment(), 0, _borderSize, _gutterSize, _boxShadow, _textShadow);
		((Graphics::MacText *)widget)->draw();
		((Graphics::MacText *)widget)->_focusable = _editable;
		((Graphics::MacText *)widget)->setEditable(_editable);
		((Graphics::MacText *)widget)->_selectable = _editable;
		break;

	case kCastButton:
		// note that we use _initialRect for the dimensions of the button;
		// the values provided in the sprite bounding box are ignored
		widget = new Graphics::MacButton(Graphics::MacButtonType(_buttonType), getAlignment(), g_director->getCurrentWindow(), bbox.left, bbox.top, _initialRect.width(), _initialRect.height(), g_director->_wm, _ftext, macFont, getForeColor(), 0xff);
		((Graphics::MacButton *)widget)->draw();
		widget->_focusable = true;

		((Graphics::MacButton *)widget)->draw();
		break;

	default:
		break;
	}

	delete macFont;
	return widget;
}

void TextCastMember::importRTE(byte *text) {
	//assert(rteList.size() == 3);
	//child0 is probably font data.
	//child1 is the raw text.
	_ptext = _ftext = Common::String((char*)text);
	//child2 is positional?
}

void TextCastMember::setText(const char *text) {
	// Do nothing if text did not change
	if (_ptext.equals(text))
		return;

	// If text has changed, use the cached formatting from first STXT in this castmember.
	Common::String formatting = Common::String::format("\001\016%04x%02x%04x%04x%04x%04x", _fontId, _textSlant, _fontSize, _fgpalinfo1, _fgpalinfo2, _fgpalinfo3);
	_ptext = text;
	_ftext = formatting + text;

	_modified = true;
}

Common::String TextCastMember::getText() {
	return _ptext;
}

bool TextCastMember::isEditable() {
	return _editable;
}

void TextCastMember::setEditable(bool editable) {
	_editable = editable;
}

void TextCastMember::updateFromWidget(Graphics::MacWidget *widget) {
	if (widget && _type == kCastText) {
		_ptext = ((Graphics::MacText *)widget)->getEditedString().encode();
	}
}


/////////////////////////////////////
// Shape
/////////////////////////////////////

ShapeCastMember::ShapeCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastShape;

	byte unk1;

	_ink = kInkTypeCopy;

	if (version < 400) {
		unk1 = stream.readByte();
		_shapeType = static_cast<ShapeType>(stream.readByte());
		_initialRect = Movie::readRect(stream);
		_pattern = stream.readUint16BE();
		// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
		_fgCol = g_director->transformColor((128 + stream.readByte()) & 0xff);
		_bgCol = g_director->transformColor((128 + stream.readByte()) & 0xff);
		_fillType = stream.readByte();
		_ink = static_cast<InkType>(_fillType & 0x3f);
		_lineThickness = stream.readByte();
		_lineDirection = stream.readByte();
	} else if (version >= 400 && version < 500) {
		unk1 = stream.readByte();
		_shapeType = static_cast<ShapeType>(stream.readByte());
		_initialRect = Movie::readRect(stream);
		_pattern = stream.readUint16BE();
		_fgCol = g_director->transformColor((uint8)stream.readByte());
		_bgCol = g_director->transformColor((uint8)stream.readByte());
		_fillType = stream.readByte();
		_ink = static_cast<InkType>(_fillType & 0x3f);
		_lineThickness = stream.readByte();
		_lineDirection = stream.readByte();
	} else {
		stream.readByte(); // FIXME: Was this copied from D4 by mistake?
		unk1 = stream.readByte();

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);

		_shapeType = kShapeRectangle;
		_pattern = 0;
		_fgCol = _bgCol = 0;
		_fillType = 0;
		_lineThickness = 1;
		_lineDirection = 0;
	}
	_modified = false;

	debugC(3, kDebugLoading, "ShapeCastMember: unk1: %x type: %d pat: %d fg: %d bg: %d fill: %d thick: %d dir: %d",
		unk1, _shapeType, _pattern, _fgCol, _bgCol, _fillType, _lineThickness, _lineDirection);

	if (debugChannelSet(3, kDebugLoading))
		_initialRect.debugPrint(0, "ShapeCastMember: rect:");
}


/////////////////////////////////////
// Script
/////////////////////////////////////

ScriptCastMember::ScriptCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastLingoScript;
	_scriptType = kNoneScript;

	if (version < 400) {
		error("Unhandled Script cast");
	} else if (version >= 400 && version < 500) {
		byte unk1 = stream.readByte();
		byte type = stream.readByte();

		switch (type) {
		case 1:
			_scriptType = kScoreScript;
			break;
		case 3:
			_scriptType = kMovieScript;
			break;
		default:
			error("ScriptCastMember: Unprocessed script type: %d", type);
		}

		debugC(3, kDebugLoading, "CASt: Script type: %s (%d), unk1: %d", scriptType2str(_scriptType), type, unk1);

		stream.readByte(); // There should be no more data
		assert(stream.eos());
	} else if (version >= 500) {
		stream.readByte();
		stream.readByte();

		debugC(4, kDebugLoading, "CASt: Script");

		// WIP need to complete this!
	}
}


/////////////////////////////////////
// RTE
/////////////////////////////////////

RTECastMember::RTECastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: TextCastMember(cast, castId, stream, version) {

	_type = kCastRTE;
}

void RTECastMember::loadChunks() {
	//TODO: Actually load RTEs correctly, don't just make fake STXT.
#if 0
	Common::SeekableReadStream *rte1 = _movieArchive->getResource(res->children[child].tag, res->children[child].index);
	byte *buffer = new byte[rte1->size() + 2];
	rte1->read(buffer, rte1->size());
	buffer[rte1->size()] = '\n';
	buffer[rte1->size() + 1] = '\0';
	_loadedText->getVal(id)->importRTE(buffer);

	delete rte1;
#endif
}


/////////////////////////////////////
// Palette
/////////////////////////////////////

PaletteCastMember::PaletteCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
	: CastMember(cast, castId, stream) {
	_type = kCastPalette;
	_palette = nullptr;
}

} // End of namespace Director
