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

#include "common/system.h"
#include "common/substream.h"
#include "common/tokenizer.h"

#include "graphics/surface.h"

#include "image/bmp.h"

#include "petka/flc.h"
#include "petka/q_manager.h"
#include "petka/q_system.h"
#include "petka/petka.h"

namespace Petka {

QManager::QManager(PetkaEngine &vm)
	: _vm(vm) {}

QManager::~QManager() {
	for (Common::HashMap<uint32, QResource>::iterator it = _resourceMap.begin(); it != _resourceMap.end(); ++it) {
		destructResourceContent(it->_value);
	}
}

bool QManager::init() {
	clear();
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm.openFile("resource.qrc", true));
	if (!stream) {
		return false;
	}

	while (!stream->eos()) {
		Common::StringTokenizer tokenizer(stream->readLine());
		if (tokenizer.empty()) {
			continue;
		}

		const uint32 id = (uint32)atoi(tokenizer.nextToken().c_str());
		_isAlwaysNeededMap.setVal(id, tokenizer.nextToken() == "==");
		_nameMap.setVal(id, tokenizer.nextToken());

	}
	return true;
}

Common::String QManager::findResourceName(uint32 id) const {
	if (_nameMap.contains(id)) {
		return _nameMap.getVal(id);
	}
	return "";
}

Common::String QManager::findSoundName(uint32 id) const {
	Common::String name = findResourceName(id);
	name.toUppercase();
	if (name.empty() || name.hasSuffix(".WAV")) {
		return name;
	}
	name.erase(name.size() - 3, 3);
	return name += "WAV";
}

void QManager::removeResource(uint32 id) {
	if (_resourceMap.contains(id)) {
		destructResourceContent(_resourceMap.getVal(id));
		_resourceMap.erase(id);
	}
}

void QManager::clearUnneeded() {
	for (Common::HashMap<uint32, QResource>::iterator it = _resourceMap.begin(); it != _resourceMap.end(); ++it) {
		if (!_isAlwaysNeededMap.getVal(it->_key)) {
			destructResourceContent(it->_value);
			_resourceMap.erase(it);
		}
	}
}

Graphics::Surface *QManager::findOrCreateSurface(uint32 id, uint16 w, uint16 h) {
	if (_resourceMap.contains(id)) {
		QResource &res = _resourceMap.getVal(id);
		if (res.type != QResource::kSurface) {
			return nullptr;
		}
		return res.surface;
	}

	QResource &res = _resourceMap.getVal(id);
	res.type = QResource::kSurface;
	res.surface = new Graphics::Surface;
	res.surface->create(w, h, _vm._system->getScreenFormat());
	return res.surface;
}

void QManager::destructResourceContent(QResource &res) {
	if (res.type == QResource::kSurface) {
		res.surface->free();
		delete res.surface;
	} else {
		delete res.flcDecoder;
	}
}

Common::SeekableReadStream *QManager::loadFileStream(uint32 id) const {
	const Common::String &name = findResourceName(id);
	return name.empty() ? nullptr : _vm.openFile(name, false);
}

Graphics::Surface *QManager::loadBitmap(uint32 id) {
	if (_resourceMap.contains(id)) {
		QResource &res = _resourceMap.getVal(id);
		if (res.type != QResource::kSurface) {
			return nullptr;
		}
		return res.surface;
	}

	Common::ScopedPtr<Common::SeekableReadStream> stream(loadFileStream(id));
	if (!stream) {
		return nullptr;
	}

	Graphics::Surface *s = loadBitmapSurface(*stream);
	if (s) {
		s->convertToInPlace(g_system->getScreenFormat());
		_vm.getQSystem()->_sceneWidth = s->w;
		_vm.getQSystem()->_xOffset = 0;

		QResource &res = _resourceMap.getVal(id);
		res.type = QResource::kSurface;
		res.surface = s;
		return res.surface;
	}
	return nullptr;
}

FlicDecoder *QManager::loadFlic(uint32 id) {
	if (_resourceMap.contains(id)) {
		QResource &res = _resourceMap.getVal(id);
		if (res.type != QResource::kFlic) {
			return nullptr;
		}
		return res.flcDecoder;
	}

	Common::String name = findResourceName(id);
	Common::SeekableReadStream *stream = _vm.openFile(name, false);
	if (!stream) {
		return nullptr;
	}
	name.erase(name.size() - 3, 3);
	name.toUppercase();
	name += "MSK";

	FlicDecoder *flc = new FlicDecoder;
	flc->load(stream, _vm.openFile(name, false));
	QResource &res = _resourceMap.getVal(id);
	res.type = QResource::kFlic;
	res.flcDecoder = flc;
	return res.flcDecoder;
}

void QManager::clear() {
	for (Common::HashMap<uint32, QResource>::iterator it = _resourceMap.begin(); it != _resourceMap.end(); ++it) {
		destructResourceContent(it->_value);
	}
	_resourceMap.clear();
	_nameMap.clear();
	_isAlwaysNeededMap.clear();
}

Graphics::Surface *QManager::loadBitmapSurface(Common::SeekableReadStream &stream) {
	if (stream.readByte() != 'B')
		return nullptr;

	if (stream.readByte() != 'M')
		return nullptr;

	stream.skip(12);

	uint32 infoSize = stream.readUint32LE();
	assert(infoSize == 40);

	uint32 width = stream.readUint32LE();
	uint32 height = stream.readUint32LE();
	stream.skip(2);
	uint16 bitsPerPixel = stream.readUint16LE();
	if (bitsPerPixel != 16 && bitsPerPixel != 1) {
		stream.seek(0, SEEK_SET);
		Image::BitmapDecoder decoder;
		if (!decoder.loadStream(stream))
			return nullptr;
		return decoder.getSurface()->convertTo(g_system->getScreenFormat(), decoder.getPalette());
	}
	else if (bitsPerPixel == 1) {
		Graphics::Surface *s = new Graphics::Surface;
		s->create(width, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 0, 5, 11, 0));
		return s;
	}

	/* uint32 compression = stream.readUint32BE(); */
	/* uint32 imageSize = stream.readUint32LE(); */
	/* uint32 pixelsPerMeterX = stream.readUint32LE(); */
	/* uint32 pixelsPerMeterY = stream.readUint32LE(); */
	/* uint32 paletteColorCount = stream.readUint32LE(); */
	/* uint32 colorsImportant = stream.readUint32LE(); */
	/* uint32 unk = stream.readUint32LE(); */
	/* uint32 fileSize_ = stream.readUint32LE(); */
	stream.skip(32);

	int srcPitch = width * (bitsPerPixel >> 3);
	int extraDataLength = (srcPitch % 4) ? 4 - (srcPitch % 4) : 0;

	Graphics::Surface *s = new Graphics::Surface;
	s->create(width, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 0, 5, 11, 0));

	uint16 *dst = (uint16 *)s->getPixels();
	for (uint i = 0; i < height; ++i) {
		for (uint j = 0; j < width; ++j) {
			dst[(height - i - 1) * width + j] = stream.readUint16BE();
		}
		stream.skip(extraDataLength);
	}
	return s;
}

} // End of namespace Petka
