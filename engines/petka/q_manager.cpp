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

#include "common/memstream.h"
#include "common/system.h"
#include "common/substream.h"
#include "common/tokenizer.h"

#include "graphics/surface.h"

#include "image/bmp.h"

#include "petka/flc.h"
#include "petka/q_manager.h"
#include "petka/petka.h"

namespace Petka {

QManager::QManager(PetkaEngine &vm)
	: _vm(vm) {}

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
	return _nameMap.contains(id) ? _nameMap.getVal(id) : "";
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
		_resourceMap.erase(id);
	}
}

void QManager::clearUnneeded() {
	for (auto it = _resourceMap.begin(); it != _resourceMap.end(); ++it) {
		if (!_isAlwaysNeededMap.getVal(it->_key)) {
			_resourceMap.erase(it);
		}
	}
}

Graphics::Surface *QManager::getSurface(uint32 id, uint16 w, uint16 h) {
	if (_resourceMap.contains(id)) {
		QResource &res = _resourceMap.getVal(id);
		return res.type == QResource::kSurface ? res.surface : nullptr;
	}

	QResource &res = _resourceMap.getVal(id);
	res.type = QResource::kSurface;
	res.surface = new Graphics::Surface;
	res.surface->create(w, h, _vm._system->getScreenFormat());

	return res.surface;
}

Common::SeekableReadStream *QManager::loadFileStream(uint32 id) const {
	const Common::String &name = findResourceName(id);
	return name.empty() ? nullptr : _vm.openFile(name, false);
}

Graphics::Surface *QManager::getSurface(uint32 id) {
	if (_resourceMap.contains(id)) {
		QResource &res = _resourceMap.getVal(id);
		return res.type == QResource::kSurface ? res.surface : nullptr;
	}

	Common::ScopedPtr<Common::SeekableReadStream> stream(loadFileStream(id));
	if (!stream) {
		return nullptr;
	}

	Graphics::Surface *s = loadBitmapSurface(*stream);
	if (s) {
		QResource &res = _resourceMap.getVal(id);
		res.type = QResource::kSurface;
		res.surface = s;
		return res.surface;
	}

	return nullptr;
}

FlicDecoder *QManager::getFlic(uint32 id) {
	if (_resourceMap.contains(id)) {
		QResource &res = _resourceMap.getVal(id);
		return res.type == QResource::kFlic ? res.flcDecoder : nullptr;
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
	_resourceMap.clear();
	_nameMap.clear();
	_isAlwaysNeededMap.clear();
}

Graphics::Surface *QManager::loadBitmapSurface(Common::SeekableReadStream &stream) {
	const uint32 kHeaderSize = 14 + 40;
	const uint32 kAdditionalDataSize = 8;

	if (stream.readByte() != 'B')
		return nullptr;

	if (stream.readByte() != 'M')
		return nullptr;

	uint32 realFileSize = stream.readUint32LE();

	stream.skip(12);

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

	stream.seek(0, SEEK_SET);
	byte *convertedBmp = new byte[realFileSize];

	stream.read(convertedBmp, kHeaderSize);
	WRITE_LE_INT16(convertedBmp + 28, 24); // bitsPerPixel

	uint32 align = stream.readUint32LE();
	uint32 fileSize = stream.readUint32LE();

	byte *pixels = convertedBmp + kHeaderSize;
	uint32 pixelsCount = (fileSize - (kHeaderSize + kAdditionalDataSize) - align + 1) / 2;

	Graphics::PixelFormat fmt(2, 5, 6, 5, 0, 0, 5, 11, 0);
	while (pixelsCount) {
		fmt.colorToRGB(stream.readUint16BE(), *(pixels + 2), *(pixels + 1), *pixels);
		pixels += 3;
		pixelsCount--;
	}

	Common::MemoryReadStream convBmpStream(convertedBmp, realFileSize, DisposeAfterUse::YES);
	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(convBmpStream))
		return nullptr;

	return decoder.getSurface()->convertTo(g_system->getScreenFormat(), decoder.getPalette());
}

QManager::QResource::~QResource() {
	if (type == QResource::kSurface && surface) {
		surface->free();
		delete surface;
	} else {
		delete flcDecoder;
	}
}

} // End of namespace Petka
