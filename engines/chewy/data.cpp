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

#include "common/debug.h"
#include "common/system.h"
#include "chewy/data.h"
#include "chewy/chewy.h"
#include "chewy/defines.h"
#include "chewy/file.h"
#include "chewy/globals.h"

namespace Chewy {

Data::Data() {
}

Data::~Data() {
}

uint16 Data::select_pool_item(Stream *stream, uint16 nr) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	NewPhead ph;

	if (rs) {
		rs->seek(0, SEEK_SET);
		if (!ph.load(rs)) {
			error("select_pool_item error");
		} else {
			if (!strncmp(ph.id, "NGS", 3)) {
				if (nr >= ph.PoolAnz)
					nr = ph.PoolAnz - 1;

				rs->seek(-(int)((ph.PoolAnz - nr) * sizeof(uint32)), SEEK_END);
				uint32 tmp1 = rs->readUint32LE();
				rs->seek(tmp1, SEEK_SET);
			}
		}
	}

	return nr;
}

void Data::load_tafmcga(Stream *stream, int16 komp, uint32 size, byte *speicher) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	assert(rs);

	byte *sp = speicher;

	if (komp == 1) {
		// Run length encoding using count/value pairs
		for (uint32 pos = 0; pos < size;) {
			uint8 count = rs->readByte();
			uint8 value = rs->readByte();

			for (uint8 i = 0; (i < count) && (pos < size); i++) {
				sp[pos] = value;
				++pos;
			}
		}
	} else {
		rs->read(sp, size);
		sp += size;
	}
}

void Data::load_tff(const char *fname, byte *speicher) {
	strncpy(_filename, fname, MAXPATH - 5);
	_filename[MAXPATH - 5] = '\0';

	if (speicher) {
		if (!strchr(_filename, '.'))
			strcat(_filename, ".tff");

		Common::File f;
		if (f.open(_filename)) {
			TffHeader *tff = (TffHeader *)speicher;
			if (tff->load(&f)) {
				uint32 size = tff->size;
				if (f.read(speicher + sizeof(TffHeader), size) != size) {
					error("load_tff error");
				}
			}

			f.close();
		} else {
			error("load_tff error");
		}
	} else {
		error("load_tff error");
	}
}

void Data::void_load(const char *fname, byte *speicher, uint32 size) {
	strncpy(_filename, fname, MAXPATH - 1);
	_filename[MAXPATH - 1] = '\0';

	Common::File f;
	if (f.open(_filename)) {
		if (!f.read(speicher, size)) {
			error("void_load error");
		}

		f.close();
	} else {
		error("void_load error");
	}
}

uint32 Data::load_tmf(Stream *handle, TmfHeader *song) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(handle);
	ChunkHead ch;
	uint32 size = 0;

	if (rs) {
		rs->seek(-ChunkHead::SIZE(), SEEK_CUR);
		if (!ch.load(rs)) {
			error("load_tmf error");
		} else {
			if (ch.type == TMFDATEI) {
				assert(ch.size > (uint32)TmfHeader::SIZE());

				if (!song->load(rs)) {
					error("load_tmf error");
				} else {
					size = ch.size + sizeof(TmfHeader);
					byte *speicher = (byte *)song + sizeof(TmfHeader);
					speicher += ((uint32)song->pattern_anz) * 1024l;
					for (int16 i = 0; i < 31; ++i) {
						if (song->instrument[i].laenge) {
							song->ipos[i] = speicher;
							speicher += song->instrument[i].laenge;
						}
					}
				}
			} else {
				error("load_tmf error");
			}
		}
	}

	return size;
}

// Only used in 2 places, will be removed eventually:
// tff_adr() with type TFFDATEI
// void_adr() with type 200
uint32 Data::size(const char *fname, int16 typ) {
	uint32 size = 0;

	strncpy(_filename, fname, MAXPATH - 5);
	_filename[MAXPATH - 5] = '\0';

	if (!strchr(_filename, '.') && typ == TFFDATEI) {
		strcat(_filename, ".tff");
	}

	// SCUMMVM: Note to self, use sizeof(structures) for
	// allocating size, not custom ::SIZE() functions
	Common::File f;
	if (f.open(_filename)) {
		switch (typ) {
		case TFFDATEI: {
			TffHeader tff;
			if (tff.load(&f)) {
				int16 id = get_id(tff.id);
				if (id == TFFDATEI) {
					size = tff.size + sizeof(TffHeader);
				} else {
					error("size error");
				}
			} else {
				error("size error");
			}
			}
			break;
		default:
			size = (uint32)f.size();
			break;
		}

		f.close();
	} else {
		error("size error");
		size = 0;
	}

	return size;
}

uint32 Data::get_poolsize(const char *fname, int16 chunk_start, int16 chunk_anz) {
	NewPhead Nph;
	uint32 size = 0;

	Common::File f;
	if (f.open(fname)) {
		if (!Nph.load(&f)) {
			error("get_poolsize error");
		} else {
			if (!strncmp(Nph.id, "NGS", 3)) {
				select_pool_item(&f, chunk_start);
				f.seek(-ChunkHead::SIZE(), SEEK_CUR);

				for (int16 i = chunk_start; (i < Nph.PoolAnz) && i < (chunk_start + chunk_anz); i++) {
					ChunkHead ch;
					if (!ch.load(&f)) {
						error("get_poolsize error");
					} else {
						if (ch.size > size)
							size = ch.size;

						f.seek(ch.size, SEEK_CUR);
					}
				}
			}
		}

		f.close();
	} else {
		error("get_poolsize error");
	}

	return size;
}

int16 Data::get_id(char *id_code) {
	int16 id = -1;
	if (!(scumm_strnicmp(id_code, "TAF", 3)))
		id = TAFDATEI;
	if (!(scumm_strnicmp(id_code, "TFF", 3)))
		id = TFFDATEI;

	return id;
}

void Data::fcopy(const char *d_fname, const char *s_fname) {
	assert(!strcmp(d_fname, ADSH_TMP));

	Common::File f;
	if (f.open(s_fname)) {
		Common::SeekableWriteStream *ws = g_engine->_tempFiles.createWriteStreamForMember(ADSH_TMP);
		ws->writeStream(&f);
		delete ws;
	} else {
		error("Could not find - %s", s_fname);
	}
}

} // namespace Chewy
