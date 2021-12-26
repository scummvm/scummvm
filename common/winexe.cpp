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

	byte *window = new byte[0x1000]();
	int pos = 0x1000 - 16;

	byte *unpackedData = (byte *)malloc(unpackedLength);
	assert(unpackedData);
	byte *dataPos = unpackedData;

	uint32 remaining = unpackedLength;

	// Apply simple LZSS decompression
	for (;;) {
		byte controlByte = file.readByte();

		if (remaining == 0 || file.eos())
			break;

		for (byte i = 0; i < 8; i++) {
			if (controlByte & (1 << i)) {
				*dataPos++ = window[pos++] = file.readByte();
				pos &= 0xFFF;
				if (--remaining == 0)
					break;
			} else {
				int matchPos = file.readByte();
				int matchLen = file.readByte();
				matchPos |= (matchLen & 0xF0) << 4;
				matchLen = (matchLen & 0xF) + 3;
				if ((uint32)matchLen > remaining)
					matchLen = remaining;
				remaining -= matchLen;

				while (matchLen--) {
					*dataPos++ = window[pos++] = window[matchPos++];
					pos &= 0xFFF;
					matchPos &= 0xFFF;
				}

				if (remaining == 0)
					break;
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

WinResources *WinResources::createFromEXE(SeekableReadStream *stream) {
	WinResources *exe;

	// First try loading via the NE code
	stream->seek(0);
	exe = new Common::NEResources();
	if (exe->loadFromEXE(stream, DisposeAfterUse::NO)) {
		return exe;
	}
	delete exe;

	// Then try loading via the PE code
	stream->seek(0);
	exe = new Common::PEResources();
	if (exe->loadFromEXE(stream, DisposeAfterUse::NO)) {
		return exe;
	}
	delete exe;

	return nullptr;
}

WinResources::VersionInfo *WinResources::getVersionResource(const WinResourceID &id) {
		VersionInfo *info = nullptr;

		SeekableReadStream *res = getResource(kWinVersion, id);
		if (res) {
			info = parseVersionInfo(res);
			delete res;
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


bool WinResources::VersionInfo::readVSVersionInfo(SeekableReadStream *res) {
	// Signature check
	if (res->readUint32LE() != 0xFEEF04BD)
		return false;

	res->readUint32LE(); // struct version

	// The versions are stored a bit weird
	fileVersion[1] = res->readUint16LE();
	fileVersion[0] = res->readUint16LE();
	fileVersion[3] = res->readUint16LE();
	fileVersion[2] = res->readUint16LE();
	productVersion[1] = res->readUint16LE();
	productVersion[0] = res->readUint16LE();
	productVersion[3] = res->readUint16LE();
	productVersion[2] = res->readUint16LE();

	fileFlagsMask = res->readUint32LE();
	fileFlags = res->readUint32LE();
	fileOS = res->readUint32LE();
	fileType = res->readUint32LE();
	fileSubtype = res->readUint32LE();
	fileDate[0] = res->readUint32LE();
	fileDate[1] = res->readUint32LE();

	hash.setVal("File:", Common::U32String::format("%d.%d.%d.%d", fileVersion[0], fileVersion[1], fileVersion[2], fileVersion[3]));
	hash.setVal("Prod:", Common::U32String::format("%d.%d.%d.%d", productVersion[0], productVersion[1], productVersion[2], productVersion[3]));

	return true;
}

} // End of namespace Common
