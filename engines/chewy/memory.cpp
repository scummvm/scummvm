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

#include "common/memstream.h"
#include "chewy/memory.h"
#include "chewy/global.h"
#include "chewy/types.h"
#include "chewy/resource.h"

namespace Chewy {

Memory::Memory() {
	file = new Data();
}

Memory::~Memory() {
	delete (file);
}

taf_info *Memory::taf_adr(const char *filename) {
	SpriteResource *res = new SpriteResource(filename);
	int32 imageCount = res->getChunkCount();
	uint32 size = res->getAllSize() + imageCount * 8 + sizeof(taf_info);
	uint32 kgroesse = imageCount * sizeof(byte *);
	byte *imgPtr, *tmp1;
	taf_info *tinfo = nullptr;

	tmp1 = (byte *)MALLOC(size + PALETTE_SIZE + kgroesse);
	tinfo = (taf_info *)tmp1;
	tinfo->image = (byte **)(tmp1 + sizeof(taf_info));
	tinfo->palette = tmp1 + size;
	tinfo->anzahl = imageCount;
	memcpy(tinfo->palette, res->getSpritePalette(), PALETTE_SIZE);
	imgPtr = tmp1 + sizeof(taf_info) + kgroesse;

	for (int i = 0; i < imageCount; i++) {
		tinfo->image[i] = imgPtr;
		imgPtr += res->getSpriteData(i, &tinfo->image[i], false);
	}

	tinfo->korrektur = (int16 *)(tmp1 + (size + 768l));
	memcpy(tinfo->korrektur, res->getSpriteCorrectionsTable(), imageCount * 2 * sizeof(int16));

	delete res;

	return tinfo;
}

taf_seq_info *Memory::taf_seq_adr(int16 image_start, int16 image_anz) {
	Common::File *rs = new Common::File();
	rs->open(CH_SPZ_FILE);
	taf_dateiheader header;
	taf_imageheader iheader;
	taf_seq_info *ts_info = nullptr;
	uint32 ptr;
	uint32 size;
	int16 i;
	int16 *abmess;
	int16 id;
	byte *tmp1;
	byte *sp_ptr;

	rs->seek(0, SEEK_SET);
	if (header.load(rs)) {
		id = file->get_id(header.id);
		if (id == TAFDATEI) {
			if (header.korrekt > 1) {
				rs->seek((-(int)((header.count - image_start) * sizeof(uint32))), SEEK_END);
				ptr = rs->readUint32LE();

				rs->seek(ptr, SEEK_SET);
				size = 0;
				for (i = 0; i < image_anz && !_G(modul); i++) {
					if (iheader.load(rs)) {
						size += iheader.width * iheader.height;
						rs->seek(iheader.next, SEEK_SET);

					} else {
						error("taf_seq_adr error");
					}
				}
				if (!_G(modul)) {
					size += image_anz * sizeof(byte *);
					size += image_anz * sizeof(char *);
					size += ((uint32)sizeof(taf_seq_info));
					tmp1 = (byte *)MALLOC(size + image_anz * sizeof(byte *));

					if (!_G(modul)) {
						ts_info = (taf_seq_info *)tmp1;
						ts_info->anzahl = image_anz;
						ts_info->image = (byte **)(tmp1 + sizeof(taf_seq_info));
						ts_info->korrektur = (int16 *)(tmp1 + size);
						rs->seek(ptr, SEEK_SET);
						sp_ptr = tmp1 + (((uint32)sizeof(taf_seq_info))
							+ (image_anz * sizeof(char *)));

						for (i = 0; i < image_anz && !_G(modul); i++) {
							if (iheader.load(rs)) {
								ts_info->image[i] = sp_ptr;
								abmess = (int16 *)sp_ptr;
								abmess[0] = iheader.width;
								abmess[1] = iheader.height;
								sp_ptr += 4;
								size = (uint32)((uint32)iheader.height) *
									    ((uint32)iheader.width);

								rs->seek(iheader.image, SEEK_SET);
								file->load_tafmcga(rs, iheader.komp, size, sp_ptr);
								rs->seek(iheader.next, SEEK_SET);
								sp_ptr += size;
							} else {
								error("taf_seq_adr error");
							}
						}
						if (!_G(modul)) {
							rs->seek((-(int)(((header.count * 2) - image_start) * sizeof(uint32))), SEEK_END);

							if ((rs->size() - rs->pos()) < (int)image_anz * 4) {
								error("taf_seq_adr error");
							} else {
								int16 *p = ts_info->korrektur;
								for (i = 0; i < (int)image_anz * 2; ++i, ++p)
									*p = rs->readSint16LE();
							}
						}
						if (_G(modul))
							free(tmp1);
					}
				}
			} else {
				error("taf_seq_adr error");
			}
		} else {
			error("taf_seq_adr error");
		}
	} else {
		error("taf_seq_adr error");
	}

	delete rs;

	return ts_info;
}

void Memory::tff_adr(const char *filename, byte **speicher) {
	uint32 size;
	size = file->size(filename, TFFDATEI);

	if (!_G(modul)) {
		*speicher = (byte *)MALLOC(size);
		if (*speicher) {
			file->load_tff(filename, *speicher);
			if (_G(modul)) {
				free(*speicher);
				*speicher = 0;
			}
		} else {
			_G(fcode) = NOSPEICHER;
			_G(modul) = SPEICHER;
		}
	}
}

// Only called from init_load() with filename blende.rnd
byte *Memory::void_adr(const char *filename) {
	uint32 size = 0;
	byte *ptr = 0;
	size = file->size(filename, 200);

	if (!_G(modul)) {
		ptr = (byte *)MALLOC(size * sizeof(uint32));
		if (!_G(modul)) {
			*(uint32 *)ptr = size;
			file->void_load(filename, ptr + sizeof(uint32), size);
			if (_G(modul))
				free(ptr);
		}
	}

	return ptr;
}

} // namespace Chewy
