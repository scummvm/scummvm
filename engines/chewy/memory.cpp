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

#include "chewy/memory.h"

namespace Chewy {

extern int16 modul;
extern int16 fcode;
uint32 imagesize[MAXSPRITE];
memory::memory() {
	file = nullptr;
}
memory::~memory() {
	delete (file);
}

taf_info *memory::taf_adr(const char *filename) {
	uint32 size;
	int16 anz_image = 0;
	uint32 kgroesse;
	byte *tempptr, *tmp1;
	taf_info *tinfo = nullptr;
	taf_dateiheader *tafheader;
	tafheader = 0;

	size = file->get_tafinfo(filename, &tafheader);
	kgroesse = 0l;
	if (!modul) {
		anz_image = tafheader->count;
		kgroesse = ((uint32)anz_image) * sizeof(byte *);

		tmp1 = (byte *)calloc(size + 768l + kgroesse, 1);
		if (!modul) {
			tinfo = (taf_info *)tmp1;
			tinfo->palette = tmp1 + size;
			tinfo->anzahl = anz_image;
			file->load_palette(filename, tinfo->palette, TAFDATEI);
			if (!modul) {
				tempptr = tmp1 + (((uint32)sizeof(taf_info)) + kgroesse);
				file ->load_full_taf(filename, tempptr, tinfo);
				tinfo->korrektur = (int16 *)(tmp1 + (size + 768l));
				file ->load_korrektur(filename, (byte *)tinfo->korrektur);
			}
			if (modul)
				free(tmp1);
		} else {
			fcode = NOSPEICHER;
			modul = SPEICHER;
			tinfo = 0;
		}
	}

	return tinfo;
}

taf_seq_info *memory::taf_seq_adr(void *h, int16 image_start,
                                  int16 image_anz) {
	Stream *handle = (Stream *)h;
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
	chewy_fseek(handle, 0, SEEK_SET);
	if (chewy_fread(&header, sizeof(taf_dateiheader), 1, handle)) {
		id = file->get_id(header.id);
		if (id == TAFDATEI) {
			if (header.korrekt > 1) {
				chewy_fseek(handle, (-(int)((header.count - image_start)*sizeof(uint32))), SEEK_END);
				if (!chewy_fread(&ptr, sizeof(uint32), 1, handle)) {
					fcode = READFEHLER;
					modul = DATEI;
				} else {
					chewy_fseek(handle, ptr, SEEK_SET);
					size = 0;
					for (i = 0; i < image_anz && !modul; i++) {
						if (chewy_fread(&iheader, sizeof(taf_imageheader), 1, handle)) {
							size += iheader.width * iheader.height ;
							chewy_fseek(handle, iheader.next, SEEK_SET);
						} else {
							modul = DATEI;
							fcode = READFEHLER;
						}
					}
					if (!modul) {
						size += image_anz * 4l;
						size += image_anz * sizeof(char *);
						size += ((uint32)sizeof(taf_seq_info));
						tmp1 = (byte *)calloc(size + image_anz * 4l, 1);
						if (!modul) {
							ts_info = (taf_seq_info *)tmp1;
							ts_info->anzahl = image_anz;
							ts_info->korrektur = (int16 *)(tmp1 + size);
							chewy_fseek(handle, ptr, SEEK_SET);
							sp_ptr = tmp1 + (((uint32)sizeof(taf_seq_info))
							                 + (image_anz * sizeof(char *)));
							for (i = 0; i < image_anz && !modul; i++) {
								if (chewy_fread(&iheader, sizeof(taf_imageheader), 1, handle)) {
									ts_info->image[i] = sp_ptr;
									abmess = (int16 *)sp_ptr;
									abmess[0] = iheader.width;
									abmess[1] = iheader.height;
									sp_ptr += 4;
									size = (uint32)((uint32)iheader.height) *
									       ((uint32)iheader.width);
									chewy_fseek(handle, iheader.image, SEEK_SET);
									file->load_tafmcga(handle, iheader.komp, size, sp_ptr);
									chewy_fseek(handle, iheader.next, SEEK_SET);
									sp_ptr += size;
								} else {
									modul = DATEI;
									fcode = READFEHLER;
								}
							}
							if (!modul) {
								chewy_fseek(handle, (-(int)(((header.count * 2) - image_start)*sizeof(uint32))), SEEK_END);
								if (!chewy_fread(ts_info->korrektur, image_anz * 4, 1, handle)) {
									{
										fcode = READFEHLER;
										modul = DATEI;
									}
								}
							}
							if (modul)
								free(tmp1);
						}
					}
				}
			} else {
				fcode = NOTTBF;
				modul = DATEI;
			}
		} else {
			fcode = NOTTBF;
			modul = DATEI;
		}
	} else {
		modul = DATEI;
		fcode = READFEHLER;
	}
	return (ts_info);
}

void memory::tff_adr(const char *filename, byte **speicher) {
	uint32 size;
	size = file->size(filename, TFFDATEI);
	if (!modul) {
		*speicher = (byte *)calloc(size, 1);
		if (*speicher) {
			file->load_tff(filename, *speicher);
			if (modul) {
				free(*speicher);
				*speicher = 0;
			}
		} else {
			fcode = NOSPEICHER;
			modul = SPEICHER;
		}
	}
}

byte *memory::void_adr(const char *filename) {
	uint32 size = 0;
	byte *ptr = 0;
	size = file->size(filename, 200);

	if (!modul) {
		ptr = (byte *)calloc(size, sizeof(uint32));
		if (!modul) {
			*(uint32 *)ptr = size;
			file->void_load(filename, ptr + sizeof(uint32), size);
			if (modul)
				free(ptr);
		}
	}

	return ptr;
}

} // namespace Chewy
