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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_RESOURCES_H
#define RIPPER_RESOURCES_H

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Ripper {

struct BitmapAssetFrame {
	uint16 width;
	uint16 height;
	byte transparentColor;
	Common::Array<byte> pixels;
	Common::Array<byte> palette;

	BitmapAssetFrame() : width(0), height(0), transparentColor(0) {}
};

struct BitmapAssetSequence {
	Common::Array<BitmapAssetFrame> frames;
};

struct BitmapFontGlyph {
	uint32 pixelOffset;
	byte width;
	byte height;
	int8 xOffset;
	int8 yOffset;

	BitmapFontGlyph() : pixelOffset(0), width(0), height(0), xOffset(0), yOffset(0) {}
};

struct BitmapFontAsset {
	byte firstCharacter;
	byte lineHeight;
	byte characterSpacing;
	byte spaceWidth;
	byte transparentColor;
	Common::Array<BitmapFontGlyph> glyphs;
	Common::Array<byte> pixels;

	BitmapFontAsset() : firstCharacter(0), lineHeight(0), characterSpacing(0),
		spaceWidth(0), transparentColor(0) {}
};

class AssetLibrary {
public:
	AssetLibrary();

	bool open(const Common::Path &filename);
	bool hasMember(const Common::String &memberName) const;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &memberName) const;

	uint getEntryCount() const { return _entries.size(); }
	bool isModernFormat() const { return _modernFormat; }
	const Common::Path &getFilename() const { return _filename; }

private:
	struct Entry {
		Common::String key;
		uint32 offset;
		uint32 size;
		uint32 timestamp;
	};

	static Common::String readFixedString(const char *data, uint length);
	static Common::String normalizeMemberName(const Common::String &memberName, bool includeExtension);
	const Entry *findEntry(const Common::String &memberName) const;

	Common::Path _filename;
	Common::Array<Entry> _entries;
	bool _modernFormat;
};

class ResourceManager {
public:
	bool initialize();
	bool loadInterfaceBitmapSequence(const Common::String &memberName,
		BitmapAssetSequence &sequence) const;
	bool loadInterfaceBitmapFont(const Common::String &memberName, BitmapFontAsset &font) const;
	bool loadGameText(Common::Array<Common::String> &strings) const;

	AssetLibrary &scripts() { return _scripts; }
	AssetLibrary &interface() { return _interface; }
	AssetLibrary &sound() { return _sound; }

private:
	AssetLibrary _scripts;
	AssetLibrary _interface;
	AssetLibrary _sound;
};

} // End of namespace Ripper

#endif // RIPPER_RESOURCES_H
