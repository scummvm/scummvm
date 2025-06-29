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

#include "common/substream.h"
#include "common/macresman.h"
#include "common/memstream.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember/palette.h"


namespace Director {

PaletteCastMember::PaletteCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
	: CastMember(cast, castId, stream) {
	stream.hexdump(stream.size());
	_type = kCastPalette;
	_palette = nullptr;
}

PaletteCastMember::PaletteCastMember(Cast *cast, uint16 castId, PaletteCastMember &source)
	: CastMember(cast, castId) {
	_type = kCastPalette;
	// force a load so we can copy the cast resource information
	source.load();
	_loaded = true;

	_palette = source._palette ? new PaletteV4(*source._palette) : nullptr;
}

PaletteCastMember::PaletteCastMember(Cast *cast, uint16 castId, byte *paletteData, PaletteV4 *pal) 
	: CastMember(cast, castId) {
	_type = kCastPalette;
	_palette = new PaletteV4(pal->id, paletteData, pal->length);
	_loaded = true;
}
 
// Need to make a deep copy
CastMember *PaletteCastMember::duplicate(Cast *cast, uint16 castId) {
	byte *buf = (byte *)malloc(_palette->length);
	memcpy(buf, _palette, _palette->length);

	return (CastMember *)(new PaletteCastMember(cast, castId, buf, _palette));
}

PaletteCastMember::~PaletteCastMember() {
	if (_palette) {
		delete[] _palette->palette;
		delete _palette;
	}
}

CastMemberID PaletteCastMember::getPaletteId() {
	load();
	return _palette ? _palette->id : CastMemberID();
}

void PaletteCastMember::activatePalette() {
	load();
	if (_palette)
		g_director->setPalette(_palette->id);
}

Common::String PaletteCastMember::formatInfo() {
	Common::String result;
	if (_palette) {
		result = "data: ";
		for (size_t i = 0; i < (size_t)_palette->length; i++) {
			result += Common::String::format("%02X%02X%02X", _palette->palette[3 * i], _palette->palette[3 * i + 1], _palette->palette[3 * i + 2]);
		}
	}
	return result;
}

void PaletteCastMember::load() {
	if (_loaded)
		return;

	// TODO: Verify how palettes work in >D4 versions
	int paletteId = 0;
	if (_cast->_version < kFileVer400) {
		// For D3 and below, palette IDs are stored in the CLUT resource as cast ID + 1024
		paletteId = _castId + _cast->_castIDoffset;
	} else if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer600) {
		for (auto &it : _children) {
			if (it.tag == MKTAG('C', 'L', 'U', 'T')) {
				paletteId = it.index;
				break;
			}
		}
		if (!paletteId) {
			warning("PaletteCastMember::load(): No CLUT resource found in %d children", _children.size());
		}
	} else {
		warning("STUB: PaletteCastMember::load(): Palettes not yet supported for version %d", _cast->_version);
	}
	if (paletteId) {

		uint32 tag = MKTAG('C', 'L', 'U', 'T');
		Archive *arch = _cast->getArchive();
		if (arch->hasResource(tag, paletteId)) {
			Common::SeekableReadStreamEndian *pal = arch->getResource(MKTAG('C', 'L', 'U', 'T'), paletteId);
			debugC(2, kDebugImages, "PaletteCastMember::load(): linking palette id %d to cast index %d", paletteId, _castId);
			PaletteV4 palData = _cast->loadPalette(*pal, paletteId);
			palData.id = CastMemberID(_castId, _cast->_castLibID);
			g_director->addPalette(palData.id, palData.palette, palData.length);
			_palette = new PaletteV4(palData);
			delete pal;
		} else {
			warning("PaletteCastMember::load(): no CLUT palette %d for cast index %d found", paletteId, _castId);
		}
	}

	_loaded = true;
}

void PaletteCastMember::unload() {
	// No unload necessary.
}

void PaletteCastMember::writePaletteData(Common::MemoryWriteStream *writeStream, uint32 offset) {
	uint32 castSize = getPaletteDataSize() + 8;
	byte *dumpData = nullptr;
	dumpData = (byte *)calloc(castSize, sizeof(byte));

	writeStream = new Common::SeekableMemoryWriteStream(dumpData, castSize);

	// writeStream->seek(offset);
	writeStream->writeUint32LE(MKTAG('C', 'L', 'U', 'T'));
	writeStream->writeUint32LE(getPaletteDataSize());

	const byte *pal = _palette->palette;

	for (int i = 0; i < _palette->length; i++) {
		writeStream->writeByte(pal[3 * i]);
		writeStream->writeByte(pal[3 * i]);

		writeStream->writeByte(pal[3 * i + 1]);
		writeStream->writeByte(pal[3 * i + 1]);

		writeStream->writeByte(pal[3 * i + 2]);
		writeStream->writeByte(pal[3 * i + 2]);
	}

	Common::DumpFile out;
	char buf[256];
	Common::sprintf_s(buf, "./dumps/%d-%s-%s", _castId, tag2str(MKTAG('C', 'L', 'U', 'T')), "WrittenPalette");

	// Write the movie out, stored in dumpData
	if (out.open(buf, true)) {
		out.write(dumpData, castSize);
		out.flush();
		out.close();
		debugC(3, kDebugLoading, "RIFXArchive::writeStream:Saved the movie as file %s", buf);
	} else {
		warning("RIFXArchive::writeStream: Error saving the file %s", buf);
	}
	free(dumpData);
	delete writeStream;
}

// This is the data in 'CASt' resource 
uint32 PaletteCastMember::getCastDataSize() {
	if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer600) {
		return 0;
	} else if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		if (_flags1 != 0xFF) {
			return 3;
		} else {
			return 4;
		}
	}
	return 0;
}

uint32 PaletteCastMember::getPaletteDataSize() {
	// This is the actual Palette data, in the 'CLUT' resource
	// PaletteCastMembers data stored in the 'CLUT' resource does not change in size (may change in content) (need to verify)
	// Hence their original size can be written
	// This is the length of the 'CLUT' resource without the header and size 
	return _palette->length * 6;
}

void PaletteCastMember::writeCastData(Common::MemoryWriteStream *writeStream) {

}

}	// End of namespace Director
