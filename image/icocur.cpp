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
#include "common/substream.h"
#include "common/memstream.h"

#include "image/icocur.h"

#include "graphics/wincursor.h"

namespace Image {

IcoCurDecoder::IcoCurDecoder() : _type(kTypeInvalid), _stream(nullptr), _disposeAfterUse(DisposeAfterUse::NO) {
}

IcoCurDecoder::~IcoCurDecoder() {
	close();
}

void IcoCurDecoder::close() {
	if (_disposeAfterUse == DisposeAfterUse::YES && _stream != nullptr)
		delete _stream;

	_stream = nullptr;
	_type = kTypeInvalid;
	_items.clear();
}

bool IcoCurDecoder::open(Common::SeekableReadStream &stream, DisposeAfterUse::Flag disposeAfterUse) {
	close();

	_stream = &stream;
	_disposeAfterUse = disposeAfterUse;

	bool loadedOK = load();
	if (!loadedOK)
		close();

	return loadedOK;
}

bool IcoCurDecoder::load() {
	uint8 iconDirData[6];

	if (_stream->read(iconDirData, 6) != 6)
		return false;

	if (iconDirData[0] != 0 || iconDirData[1] != 0 || (iconDirData[2] != 1 && iconDirData[2] != 2) || iconDirData[3] != 0) {
		warning("Malformed ICO/CUR header");
		return false;
	}

	uint16 numImages = READ_LE_UINT16(iconDirData + 4);
	_type = static_cast<Type>(iconDirData[2]);

	if (numImages == 0)
		return true;

	uint32 dirSize = static_cast<uint32>(numImages) * 16;

	Common::Array<uint8> iconDir;
	iconDir.resize(dirSize);

	if (_stream->read(&iconDir[0], dirSize) != dirSize)
		return false;

	_items.resize(numImages);
	for (uint i = 0; i < numImages; i++) {
		const uint8 *entryData = &iconDir[i * 16u];
		Item &item = _items[i];

		item.width = entryData[0];
		if (item.width == 0)
			item.width = 256;

		item.height = entryData[1];
		if (item.height == 0)
			item.height = 256;

		item.numColors = entryData[2];

		item.data.ico.numPlanes = READ_LE_UINT16(entryData + 4);
		item.data.ico.bitsPerPixel = READ_LE_UINT16(entryData + 6);
		item.dataSize = READ_LE_UINT32(entryData + 8);
		item.dataOffset = READ_LE_UINT32(entryData + 12);
	}

	return true;
}

IcoCurDecoder::Type IcoCurDecoder::getType() const {
	return _type;
}

uint IcoCurDecoder::numItems() const {
	return _items.size();
}

const IcoCurDecoder::Item &IcoCurDecoder::getItem(uint itemIndex) const {
	return _items[itemIndex];
}

Graphics::Cursor *IcoCurDecoder::loadItemAsCursor(uint itemIndex) const {
	const IcoCurDecoder::Item &dirItem = _items[itemIndex];

	if (_type != kTypeCUR)
		warning("ICO/CUR file type wasn't a cursor, but is being requested as a cursor anyway");

	if (static_cast<int64>(dirItem.dataOffset) > _stream->size()) {
		warning("ICO/CUR data offset was outside of the file");
		return nullptr;
	}

	if (_stream->size() - static_cast<int64>(dirItem.dataOffset) < static_cast<int64>(dirItem.dataSize)) {
		warning("ICO/CUR data bounds were outside of the file");
		return nullptr;
	}

	Common::SeekableSubReadStream substream(_stream, dirItem.dataOffset, dirItem.dataOffset + dirItem.dataSize);
	return Graphics::loadWindowsCursorFromDIB(substream, dirItem.data.cur.hotspotX, dirItem.data.cur.hotspotY);
}

} // End of namespace Image
