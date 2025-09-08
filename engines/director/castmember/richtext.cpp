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

#include "common/stream.h"
#include "graphics/macgui/macwidget.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/images.h"
#include "director/movie.h"
#include "director/picture.h"
#include "director/rte.h"
#include "director/score.h"
#include "director/window.h"
#include "director/castmember/richtext.h"
#include "director/lingo/lingo-the.h"

namespace Director {

RichTextCastMember::RichTextCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {

	_pf32 = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);

	if (version >= kFileVer500 && version < kFileVer1100) {
		if (debugChannelSet(5, kDebugLoading)) {
			debugC(5, kDebugLoading, "RichTextCastMember():");
			stream.hexdump(stream.size());
		}

		_initialRect = Movie::readRect(stream);
		_boundingRect = Movie::readRect(stream);
		_antialiasFlag = stream.readByte();
		_cropFlags = stream.readByte();
		_scrollPos = stream.readUint16BE();
		_antialiasFontSize = stream.readUint16BE();
		_displayHeight = stream.readUint16BE();

		uint8 r = 0, g = 0, b = 0;
		stream.readByte(); // skip one byte
		r = stream.readByte();
		g = stream.readByte();
		b = stream.readByte();
		_foreColor = _pf32.RGBToColor(r, g, b);

		r = (stream.readUint16BE() >> 8);
		g = (stream.readUint16BE() >> 8);
		b = (stream.readUint16BE() >> 8);
		_bgColor = _pf32.RGBToColor(r, g, b);

		debugC(3, kDebugLoading, "  RichTextCastMember(): initialRect: [%s], boundingRect: [%s], antialiasFlag: 0x%02x, cropFlags: 0x%02x, scrollPos: %d, antialiasFontSize: %d, displayHeight: %d",
			_initialRect.toString().c_str(),
			_boundingRect.toString().c_str(),
			_antialiasFlag,
			_cropFlags,
			_scrollPos,
			_antialiasFontSize,
			_displayHeight);
		debugC(3, kDebugLoading, "  RichTextCastMember(): foreColor: 0x%08x, bgColor: 0x%08x", _foreColor, _bgColor);
	} else {
		warning("STUB: RichTextCastMember: RTE not yet supported for version v%d (%d)", humanVersion(_cast->_version), _cast->_version);
	}

	_type = kCastRichText;
	_picture = nullptr;
}

RichTextCastMember::RichTextCastMember(Cast *cast, uint16 castId, RichTextCastMember &source)
		: CastMember(cast, castId) {

	_type = kCastRichText;
	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	_bgColor = source._bgColor;
	if (cast == source._cast)
		_children = source._children;

}

RichTextCastMember::~RichTextCastMember() {
	if (_picture)
		delete _picture;
}

void RichTextCastMember::load() {
	if (_loaded)
		return;

	// RichText casts consist of 3 files:
	// RTE0: Editor data, used only by the Authoring Tool
	// RTE1: Plain text data
	// RTE2: Bitmap representation for rendering
	//
	// RTE0 is using Paige editor by Hermes, which was recently
	// open sourced. So, if anyone wants to look into internals,
	// https://github.com/nmatavka/Hermes-Paige/tree/main
	// the pgReadDoc() is the code entry:
	// https://github.com/nmatavka/Hermes-Paige/blob/main/PGSOURCE/PGREAD.C#L767
	uint rte0id = 0;
	uint rte1id = 0;
	uint rte2id = 0;
	for (auto &it : _children) {
		if (it.tag == MKTAG('R', 'T', 'E', '0')) {
			rte0id = it.index;
		} else if (it.tag == MKTAG('R', 'T', 'E', '1')) {
			rte1id = it.index;
		} else if (it.tag == MKTAG('R', 'T', 'E', '2')) {
			rte2id = it.index;
		}
	}

	if (_cast->_loadedRTE0s.contains(rte0id)) {
		// TODO: Copy the formatted text data
		// There doesn't appear to be a way of using it outside of the Director editor.
	} else {
		warning("RichTextCastMember::load(): rte0tid %i isn't loaded", rte0id);
	}
	if (_cast->_loadedRTE1s.contains(rte1id)) {
		const RTE1 *rte1 =  _cast->_loadedRTE1s.getVal(rte1id);
		if (!rte1->data.empty())
			_plainText = Common::U32String((const char *)&rte1->data[0], rte1->data.size(), g_director->getPlatformEncoding());
	} else {
		warning("RichTextCastMember::load(): rte1tid %i isn't loaded, no plain text!", rte1id);
	}
	if (_cast->_loadedRTE2s.contains(rte2id)) {
		_picture = new Picture();
		const RTE2 *rte2 =  _cast->_loadedRTE2s.getVal(rte2id);
		Graphics::ManagedSurface *surface = rte2->createSurface(_foreColor, _bgColor, _pf32);
		if (surface) {
			_picture->_surface.copyFrom(surface->rawSurface());
			surface->free();
			delete surface;
		}
	} else {
		warning("RichTextCastMember::load(): rte2tid %i isn't loaded, no bitmap text!", rte2id);
	}

	_loaded = true;
}


Graphics::MacWidget *RichTextCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	if (!_picture) {
		warning("RichTextCastMember::createWidget: No picture");
		return nullptr;
	}

	// skip creating widget when the bbox is not available, maybe we should create it using initialRect
	if (!bbox.width() || !bbox.height())
		return nullptr;

	// Check if we need to dither the image
	int dstBpp = g_director->_wm->_pixelformat.bytesPerPixel;

	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow(), bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, false);

	Graphics::Surface *dithered = nullptr;

	if (dstBpp == 1) {
		dithered = _picture->_surface.convertTo(g_director->_wm->_pixelformat, nullptr, 0, g_director->_wm->getPalette(), g_director->_wm->getPaletteSize());
	}

	// scale for drawing a different size sprite
	copyStretchImg(
		dithered ? dithered : &_picture->_surface,
		widget->getSurface()->surfacePtr(),
		_initialRect,
		bbox,
		g_director->_wm->getPalette()
	);

	if (dithered) {
		dithered->free();
		delete dithered;
	}

	return widget;
}



bool RichTextCastMember::hasField(int field) {
	switch (field) {
	case kTheText:
	case kThePageHeight:
	case kTheScrollTop:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum RichTextCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheText:
		d = Datum(Common::String(_plainText));
		break;
	case kThePageHeight:
	case kTheScrollTop:
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

void RichTextCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheText:
		_plainText = Common::U32String(d.asString());
		warning("STUB: RichTextCastMember::setField: text set to \"%s\", but won't rerender!", d.asString().c_str());
		return;
	case kThePageHeight:
	case kTheScrollTop:
	default:
		break;
	}

	CastMember::setField(field, d);
}

Common::String RichTextCastMember::formatInfo() {
	// need to pull the data from the STXT resource before the
	// debug output will be visible
	load();
	Common::String format = formatStringForDump(_plainText.encode());

	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, text: \"%s\"",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		format.c_str()
	);
}

uint32 RichTextCastMember::getCastDataSize() {
	if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer600) {
		// 8 bytes (_initialRect)
		// 8 bytes (_boundingRect)
		// Ignored 9 bytes
		// 3 bytes r, g, b (foreground, each a byte)
		// 6 bytes r, g, b (background, each 2 bytes)
		return 26;
	} else {
		warning("RichTextCastMember()::getCastDataSize():>D5 isn't handled");
		return 0;
	}
}

void RichTextCastMember::writeCastData(Common::SeekableWriteStream *writeStream) {
	if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer600) {
		Movie::writeRect(writeStream, _initialRect);
		Movie::writeRect(writeStream, _boundingRect);

		writeStream->write(0, 8);
		writeStream->writeByte(0);

		uint8 r, g, b;
		_pf32.colorToRGB(_foreColor, r, g, b);
		writeStream->writeByte(r);
		writeStream->writeByte(g);
		writeStream->writeByte(b);

		_pf32.colorToRGB(_bgColor, r, g, b);
		writeStream->writeUint16BE(r << 8);
		writeStream->writeUint16BE(g << 8);
		writeStream->writeUint16BE(b << 8);
	} else {
		warning("RichTextCastMember()::writeCastData(): >D5 isn't handled");
	}
}

}	// End of namespace Director
