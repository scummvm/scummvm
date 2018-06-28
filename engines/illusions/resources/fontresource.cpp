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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/illusions.h"
#include "illusions/resources/fontresource.h"
#include "illusions/dictionary.h"

namespace Illusions {

// FontResourceLoader

void FontResourceLoader::load(Resource *resource) {
	FontInstance *fontInstance = new FontInstance(_vm);
	fontInstance->load(resource);
	resource->_instance = fontInstance;
}

bool FontResourceLoader::isFlag(int flag) {
	return
		flag == kRlfLoadFile;
}

// CharInfo

void CharInfo::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_width = stream.readUint16LE();
	_field_2 = stream.readUint16LE();
	uint32 pixelsOffs = stream.readUint32LE();
	_pixels = dataStart + pixelsOffs;
	debug(2, "CharInfo::load() _width: %d; _field_2: %d; pixelsOffs: %08X",
		_width, _field_2, pixelsOffs);
}

// CharRange

void CharRange::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_firstChar = stream.readUint16LE();
	_lastChar = stream.readUint16LE();
	uint count = _lastChar - _firstChar + 1;
	uint32 charInfosOffs = stream.readUint32LE();
	_charInfos = new CharInfo[count];
	for (uint i = 0; i < count; ++i) {
		stream.seek(charInfosOffs + i * 8);
		_charInfos[i].load(dataStart, stream);
	}
	debug(2, "CharRange::load() _firstChar: %d; _lastChar: %d; charInfosOffs: %08X",
		_firstChar, _lastChar, charInfosOffs);
}

CharInfo *CharRange::getCharInfo(uint16 c) {
	return &_charInfos[c - _firstChar];
}

bool CharRange::containsChar(uint16 c) {
	return c >= _firstChar && c <= _lastChar;
}

// FontResource

FontResource::FontResource() {
}

FontResource::~FontResource() {
}

void FontResource::load(Resource *resource) {
	byte *data = resource->_data;
	uint32 dataSize = resource->_dataSize;
	Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::NO);
	_totalSize = stream.readUint32LE();
	_charHeight = stream.readUint16LE();
	_defaultChar = stream.readUint16LE();
	_colorIndex = stream.readUint16LE();
	_lineIncr = stream.readUint16LE();
	_widthC = stream.readUint16LE();
	_charRangesCount = stream.readUint16LE();
	uint32 charRangesOffs = stream.pos();
	_charRanges = new CharRange[_charRangesCount];
	for (uint i = 0; i < _charRangesCount; ++i) {
		stream.seek(charRangesOffs + i * 8);
		_charRanges[i].load(data, stream);
	}
	debug(2, "FontResource::load() _charHeight: %d; _defaultChar: %d; _colorIndex: %d; _lineIncr: %d; _widthC: %d; _charRangesCount: %d",
		_charHeight, _defaultChar, _colorIndex, _lineIncr, _widthC, _charRangesCount);
}

CharInfo *FontResource::getCharInfo(uint16 c) {
	for (uint i = 0; i < _charRangesCount; ++i) {
		if (_charRanges[i].containsChar(c))
			return _charRanges[i].getCharInfo(c);
	}
	return 0;
}

const Common::Rect FontResource::calculateRectForText(uint16 *text, uint textLength) {
	int16 width = 0;
	for (uint i = 0; i < textLength && *text; i++) {
		width += getCharInfo(*text)->_width;
		text++;
	}
	return Common::Rect(width, getCharHeight() + getLineIncr());
}

// FontInstance

FontInstance::FontInstance(IllusionsEngine *vm) : _vm(vm) {
}

void FontInstance::load(Resource *resource) {
	_fontResource = new FontResource();
	_fontResource->load(resource);
	_resId = resource->_resId;
	_vm->_dict->addFont(resource->_resId, _fontResource);
}

void FontInstance::unload() {
	delete _fontResource;
	_vm->_dict->removeFont(_resId);
}

} // End of namespace Illusions
