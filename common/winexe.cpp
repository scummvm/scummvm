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

#include "common/file.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/ustr.h"
#include "common/winexe.h"
#include "common/winexe_ne.h"
#include "common/winexe_pe.h"

namespace Common {

WinResourceID &WinResourceID::operator=(const String &x) {
	_name = x;
	_idType = kIDTypeString;
	return *this;
}

WinResourceID &WinResourceID::operator=(uint32 x) {
	_id = x;
	_idType = kIDTypeNumerical;
	return *this;
}

bool WinResourceID::operator==(const String &x) const {
	return _idType == kIDTypeString && _name.equalsIgnoreCase(x);
}

bool WinResourceID::operator==(const uint32 &x) const {
	return _idType == kIDTypeNumerical && _id == x;
}

bool WinResourceID::operator==(const WinResourceID &x) const {
	if (_idType != x._idType)
		return false;
	if (_idType == kIDTypeString)
		return _name.equalsIgnoreCase(x._name);
	if (_idType == kIDTypeNumerical)
		return _id == x._id;
	return true;
}

String WinResourceID::getString() const {
	if (_idType != kIDTypeString)
		return "";

	return _name;
}

uint32 WinResourceID::getID() const {
	if (_idType != kIDTypeNumerical)
		return 0xffffffff;

	return _id;
}

String WinResourceID::toString() const {
	if (_idType == kIDTypeString)
		return _name;
	else if (_idType == kIDTypeNumerical)
		return String::format("0x%08x", _id);

	return "";
}

bool WinResources::loadFromEXE(const String &fileName) {
	if (fileName.empty())
		return false;

	File *file = new File();

	if (!file->open(fileName)) {
		delete file;
		return false;
	}

	return loadFromEXE(file);
}

bool WinResources::loadFromCompressedEXE(const String &fileName) {
	// Based on http://www.cabextract.org.uk/libmspack/doc/szdd_kwaj_format.html

	// TODO: Merge this with with loadFromEXE() so the handling of the compressed
	// EXE's is transparent

	File file;

	if (!file.open(fileName))
		return false;

	// First part of the signature
	if (file.readUint32BE() != MKTAG('S','Z','D','D'))
		return false;

	// Second part of the signature
	if (file.readUint32BE() != 0x88F02733)
		return false;

	// Compression mode must be 'A'
	if (file.readByte() != 'A')
		return false;

	file.readByte(); // file name character change
	uint32 unpackedLength = file.readUint32LE();

	byte *window = new byte[0x1000];
	int pos = 0x1000 - 16;
	memset(window, 0x20, 0x1000); // Initialize to all spaces

	byte *unpackedData = (byte *)malloc(unpackedLength);
	assert(unpackedData);
	byte *dataPos = unpackedData;

	// Apply simple LZSS decompression
	for (;;) {
		byte controlByte = file.readByte();

		if (file.eos())
			break;

		for (byte i = 0; i < 8; i++) {
			if (controlByte & (1 << i)) {
				*dataPos++ = window[pos++] = file.readByte();
				pos &= 0xFFF;
			} else {
				int matchPos = file.readByte();
				int matchLen = file.readByte();
				matchPos |= (matchLen & 0xF0) << 4;
				matchLen = (matchLen & 0xF) + 3;
				while (matchLen--) {
					*dataPos++ = window[pos++] = window[matchPos++];
					pos &= 0xFFF;
					matchPos &= 0xFFF;
				}
			}

		}
	}

	delete[] window;
	SeekableReadStream *stream = new MemoryReadStream(unpackedData, unpackedLength);

	return loadFromEXE(stream);
}


WinResources *WinResources::createFromEXE(const String &fileName) {
	WinResources *exe;

	// First try loading via the NE code
	exe = new Common::NEResources();
	if (exe->loadFromEXE(fileName)) {
		return exe;
	}
	delete exe;

	// Then try loading via the PE code
	exe = new Common::PEResources();
	if (exe->loadFromEXE(fileName)) {
		return exe;
	}
	delete exe;

	return nullptr;
}

WinResources::VersionInfo *WinResources::parseVersionInfo(SeekableReadStream *res) {
	VersionInfo *info = new VersionInfo;

	while (res->pos() < res->size() && !res->eos()) {
		while (res->pos() % 4 && !res->eos()) // Pad to 4
			res->readByte();

		/* uint16 len = */ res->readUint16LE();
		uint16 valLen = res->readUint16LE();
		uint16 type = res->readUint16LE();
		uint16 c;

		Common::U32String key;
		while ((c = res->readUint16LE()) != 0 && !res->eos())
			key += c;

		while (res->pos() % 4 && !res->eos()) // Pad to 4
			res->readByte();

		if (res->eos())
			break;

		if (type != 0) {	// text
			Common::U32String value;
			for (int j = 0; j < valLen; j++)
				value += res->readUint16LE();

			info->hash.setVal(key.encode(), value);
		} else {
			if (key == "VS_VERSION_INFO") {
				// Signature check
				if (res->readUint32LE() != 0xFEEF04BD)
					return info;

				res->readUint32LE(); // struct version

				// The versions are stored a bit weird
				info->fileVersion[1] = res->readUint16LE();
				info->fileVersion[0] = res->readUint16LE();
				info->fileVersion[3] = res->readUint16LE();
				info->fileVersion[2] = res->readUint16LE();
				info->productVersion[1] = res->readUint16LE();
				info->productVersion[0] = res->readUint16LE();
				info->productVersion[3] = res->readUint16LE();
				info->productVersion[2] = res->readUint16LE();

				info->fileFlagsMask = res->readUint32LE();
				info->fileFlags = res->readUint32LE();
				info->fileOS = res->readUint32LE();
				info->fileType = res->readUint32LE();
				info->fileSubtype = res->readUint32LE();
				info->fileDate[0] = res->readUint32LE();
				info->fileDate[1] = res->readUint32LE();

				info->hash.setVal("File:", Common::String::format("%d.%d.%d.%d", info->fileVersion[0], info->fileVersion[1], info->fileVersion[2], info->fileVersion[3]));
				info->hash.setVal("Prod:", Common::String::format("%d.%d.%d.%d", info->productVersion[0], info->productVersion[1], info->productVersion[2], info->productVersion[3]));
			}
		}
	}

	return info;
}

WinResources::VersionInfo::VersionInfo() {
	fileVersion[0] = fileVersion[1] = fileVersion[2] = fileVersion[3] = 0;
	productVersion[0] = productVersion[1] = productVersion[2] = productVersion[3] = 0;
	fileFlagsMask = 0;
	fileFlags = 0;
	fileOS = 0;
	fileType = 0;
	fileSubtype = 0;
	fileDate[0] = fileDate[1] = 0;
}

} // End of namespace Common
