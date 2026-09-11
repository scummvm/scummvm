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

#include "igor/igor.h"
#include "common/debug-channels.h"

namespace Igor {

void IgorEngine::readTableFile() {
	Common::File tblFile;
	uint32 stringsEntriesOffset = 0, resourcesEntriesOffset = 0, soundEntriesOffset = 0;
	if (tblFile.open("IGOR.TBL") && tblFile.readUint32BE() == 'ITBL' && tblFile.readUint32BE() == 4) {
		stringsEntriesOffset = tblFile.readUint32BE();
		uint32 borlandOverlaySize = _ovlFile.size();
		int gameVersionsCount = tblFile.readByte();
		for (int i = 0; i < gameVersionsCount; ++i) {
			uint32 size = tblFile.readUint32BE();
			if (size == borlandOverlaySize) {
				resourcesEntriesOffset = tblFile.readUint32BE();
				soundEntriesOffset = tblFile.readUint32BE();
				break;
			}
			tblFile.skip(8);
		}
	}
	if (resourcesEntriesOffset != 0 && soundEntriesOffset != 0) {
		tblFile.seek(resourcesEntriesOffset);
		_resourceEntriesCount = tblFile.readUint16BE();
		_resourceEntries = (ResourceEntry *)malloc(sizeof(ResourceEntry) * _resourceEntriesCount);
		for (int i = 0; i < _resourceEntriesCount; ++i) {
			_resourceEntries[i].id = tblFile.readUint16BE();
			_resourceEntries[i].offs = tblFile.readUint32BE();
			_resourceEntries[i].size = tblFile.readUint32BE();
		}
		tblFile.seek(soundEntriesOffset);
		_soundOffsetsCount = tblFile.readUint16BE();
		_soundOffsets = (uint32 *)malloc(sizeof(uint32) * _soundOffsetsCount);
		for (int i = 0; i < _soundOffsetsCount; ++i) {
			_soundOffsets[i] = tblFile.readUint32BE();
		}
		tblFile.seek(stringsEntriesOffset);
		int stringsCount = tblFile.readUint16BE();
		for (int i = 0; i < stringsCount; ++i) {
			int id = tblFile.readUint16BE();
			int lang = tblFile.readByte();
			int len = tblFile.readByte();
			bool skipString = (lang == 1 && _game.language != Common::EN_ANY) || (lang == 2 && _game.language != Common::ES_ESP);
			if (skipString) {
				tblFile.skip(len);
			} else {
				char buf[256];
				tblFile.read(buf, len);
				buf[len] = 0;
				_stringEntries.push_back(StringEntry(id, buf));
			}
		}
		return;
	}
	error("Unable to read 'IGOR.TBL'");
}

static void decodeMainString(const uint8 *src, char *dst) {
	int sz = *src - 0x6D;
	if (sz != 0) {
		++src;
		for (int i = 0; i < sz; ++i) {
			uint8 code = src[i] - 0x6D;
			dst[i] = (char)code;
		}
	}
	dst[sz] = '\0';
}

void IgorEngine::loadMainTexts() {
	loadData(IMG_VerbsPanel, _verbsPanelBuffer);
	debugC(9, kDebugResource, "loadMainTexts()");
	if (_game.version == kIdSpaCD) {
		const struct {
			int strId;
			int x;
		} verbTexts[] = {
			{ STR_Talk,   21 },
			{ STR_Take,   67 },
			{ STR_Look,  113 },
			{ STR_Use,   159 },
			{ STR_Open,  205 },
			{ STR_Close, 251 },
			{ STR_Give,  297 }
		};
		for (int i = 0; i < 7; ++i) {
			const char *s = getString(verbTexts[i].strId);
			int x = verbTexts[i].x - getStringWidth(s) / 2;
			drawString(_verbsPanelBuffer, s, x, 0, 0xF2, -1, 0);
		}
	}
	int dataSize;
	uint8 *p = loadData(TXT_MainTable, 0, &dataSize);
	const uint8 *src = &p[0] + _language * 7;
	for (int i = 0; i < 3; ++i, src += 7 * 2) {
		decodeMainString(src, _verbPrepositions[i]);
		debugC(9, kDebugResource, "loadMainTexts() _verbPrepositions[%d] '%s'", i, _verbPrepositions[i]);
	}
	src = &p[0x2A] + _language * 31;
	for (int i = 0; i < 35; ++i, src += 31 * 2) {
		decodeMainString(src, _globalObjectNames[i]);
		debugC(9, kDebugResource, "loadMainTexts() _globalObjectNames[%d] '%s'", i, _globalObjectNames[i]);
	}
	src = &p[0x8BA] + _language * 51;
	for (int i = 0; i < 250; ++i, src += 51 * 2) {
		decodeMainString(src, _globalDialogueTexts[i]);
		debugC(9, kDebugResource, "loadMainTexts() _globalDialogueTexts[%d] '%s'", i, _globalDialogueTexts[i]);
	}
	src = &p[0x6CA4] + _language * 12;
	for (int i = 0; i < 9; ++i, src += 12 * 2) {
		decodeMainString(src, _verbsName[i]);
		debugC(9, kDebugResource, "loadMainTexts() _verbsName[%d] '%s'", i, _verbsName[i]);
	}
	free(p);
}

const char *IgorEngine::getString(int id) const {
	const char *str = 0;
	for (Common::Array<StringEntry>::const_iterator it = _stringEntries.begin(); it != _stringEntries.end(); ++it) {
		if ((*it).id == id) {
			str = (*it).str.c_str();
			break;
		}
	}
	return str;
}

void IgorEngine::loadIgorFrames() {
	loadData(FRM_IgorDirBack, _facingIgorFrames[0]);
	loadData(FRM_IgorDirRight, _facingIgorFrames[1]);
	loadData(FRM_IgorDirFront, _facingIgorFrames[2]);
	loadData(FRM_IgorDirLeft, _facingIgorFrames[3]);
	loadData(FRM_IgorHead, _igorHeadFrames);
	memcpy(_igorPalette, PAL_IGOR_1, 48);
}

static int compareResourceEntry(const void *a, const void *b) {
	int id = *(const int *)a;
	const ResourceEntry *entry = (const ResourceEntry *)b;
	return id - entry->id;
}

ResourceEntry *IgorEngine::findData(int id) {
	ResourceEntry *re = (ResourceEntry *)bsearch(&id, _resourceEntries, _resourceEntriesCount, sizeof(ResourceEntry), compareResourceEntry);
	assert(re);
	return re;
}

uint8 *IgorEngine::loadData(int id, uint8 *dst, int *size) {
	debugC(9, kDebugResource, "loadData() id %d", id);
	ResourceEntry *re = findData(id);
	if (!dst) {
		dst = (uint8 *)malloc(re->size);
		if (!dst) {
			error("Unable to allocate %d bytes", re->size);
		}
	}
	_ovlFile.seek(re->offs);
	_ovlFile.read(dst, re->size);
	if (size) {
		*size = re->size;
	}
	return dst;
}

} // End of namespace Igor
