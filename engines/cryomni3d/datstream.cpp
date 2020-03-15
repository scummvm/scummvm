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


#include "cryomni3d/datstream.h"

namespace CryOmni3D {

DATSeekableStream *DATSeekableStream::getGame(Common::SeekableReadStream *stream,
        uint32 gameId, uint16 version, Common::Language lang, Common::Platform platform) {
	if (stream == nullptr) {
		return nullptr;
	}

	// Go to start of file
	stream->seek(0, SEEK_SET);

	// ALl of this should match devtools/create_cryomni3d_dat

	// Check header
	byte header[8];
	memset(header, 0, sizeof(header));
	stream->read(header, sizeof(header));
	if (memcmp(header, "CY3DDATA", sizeof(header))) {
		return nullptr;
	}

	// Check version
	uint16 fileVersion = stream->readUint16LE();
	if (fileVersion != kFileVersion) {
		return nullptr;
	}

	uint16 langTranslated = translateLanguage(lang);
	uint32 platformTranslated = translatePlatform(platform);

	uint16 games = stream->readUint16LE();

	// Padding to align to 16 bytes boundary
	(void)stream->readUint32LE();

	for (uint16 game = 0; game < games; game++) {
		// Keep tag readable
		uint32 readGameId = stream->readUint32BE();
		uint16 readVersion = stream->readUint16LE();
		// Keep tag readable
		uint16 readLang = stream->readUint16BE();
		uint32 readPlatforms = stream->readUint32LE();
		uint32 offset = stream->readUint32LE();
		uint32 size = stream->readUint32LE();

		if (gameId != readGameId) {
			continue;
		}
		if (version != readVersion) {
			continue;
		}
		if (langTranslated != readLang) {
			continue;
		}
		if (!(platformTranslated & readPlatforms)) {
			continue;
		}

		// If we are there, we got a match
		return new DATSeekableStream(stream, offset, offset + size);
	}

	// No match
	return nullptr;
}

Common::String DATSeekableStream::readString16() {
	char *buf;
	uint16 len;

	len = readUint16LE();
	buf = (char *)malloc(len);
	read(buf, len);

	Common::String s(buf, len);
	free(buf);

	return s;
}

void DATSeekableStream::readString16Array16(Common::StringArray &array) {
	uint16 items;
	uint16 i;

	items = readUint16LE();

	array.reserve(items);
	for (i = 0; i < items; i++) {
		array.push_back(readString16());
	}
}

uint16 DATSeekableStream::translateLanguage(Common::Language lang) {
	switch (lang) {
	case Common::DE_DEU:
		return MKTAG16('d', 'e');
	case Common::EN_ANY:
		return MKTAG16('e', 'n');
	case Common::ES_ESP:
		return MKTAG16('e', 's');
	case Common::FR_FRA:
		return MKTAG16('f', 'r');
	case Common::IT_ITA:
		return MKTAG16('i', 't');
	case Common::JA_JPN:
		return MKTAG16('j', 'a');
	case Common::KO_KOR:
		return MKTAG16('k', 'o');
	case Common::PT_BRA:
		return MKTAG16('b', 'r');
	case Common::ZH_TWN:
		return MKTAG16('z', 't');
	default:
		// Invalid language
		return 0;
	}
}

uint32 DATSeekableStream::translatePlatform(Common::Platform platform) {
	switch (platform) {
	case Common::kPlatformWindows:
		return 0x1;
	case Common::kPlatformDOS:
		return 0x2;
	case Common::kPlatformMacintosh:
		return 0x4;
	case Common::kPlatformPSX:
		return 0x8;
	case Common::kPlatformSegaCD:
		return 0x10;
	default:
		// Invalid platform
		return 0;
	}
}

} // End of namespace CryOmni3D
