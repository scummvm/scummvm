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

#include "common/debug.h"
#include "common/system.h"
#include "common/savefile.h"
#include "chewy/datei.h"
#include "chewy/chewy.h"
#include "chewy/defines.h"
#include "chewy/file.h"

namespace Chewy {

#define GRAFIK 1
#define SPEICHER 2
#define DATEI 3
#define MAUS 4
#define DIALOG 5

#undef MAXPATH
#define MAXPATH 1000

char filename[MAXPATH];
uint8 tmp[10000]; // FIXME

extern int16 modul;
extern int16 fcode;
int16 *ft;

datei::datei() {
}

datei::~datei() {
}

void datei::load_pcx(const char *fname, byte *speicher, byte *palette) {
	Stream *handle;
	pcx_header *header;
	uint8 zeichen;
	uint16 count, i = 0, j = 0, index;
	int16 hoehe;
	int16 breite;
	int16 *abmess;
	header = (pcx_header *)tmp;
	abmess = (int16 *)speicher;
	speicher += 4;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.'))strcat(filename, ".PCX\0");

	handle = chewy_fopen(filename, "rb");
	if (handle) {
		if (chewy_fread(header, sizeof(pcx_header), 1, handle)) {
			if ((header->id == 10) && (header->version == 5) && (header->bpp == 8)) {
				hoehe = (header->ymax - header->ymin) + 1;
				breite = header->bpz * header->planes;

				abmess[0] = breite;
				abmess[1] = hoehe;
				chewy_fseek(handle, 128L, SEEK_SET);
				for (i = 0; i < hoehe; i++) {
					index = 0;
					while (index < breite) {
						zeichen = chewy_fgetc(handle);
						if (((zeichen & 0xc0) == 0xc0) && (header->komp)) {
							count = zeichen & 0x3f;

							zeichen = chewy_fgetc(handle);
							for (j = 0; j < count; j++) {

								speicher[index] = zeichen;
								++index;
							}

						}
						else {
							speicher[index] = zeichen;
							index++;
						}
					}
					speicher += breite;
				}
				if (!(chewy_fseek(handle, -769L, SEEK_END))) {
					zeichen = chewy_fgetc(handle);
					if (zeichen == 12) {
						chewy_fseek(handle, -768L, SEEK_END);
						if ((chewy_fread(palette, 768, 1, handle)) != 1) {
							fcode = PALETTEFEHLER;
							modul = DATEI;
						} else {
							j = 0;
							for (i = 0; i < 256; i++) {
								palette[j] >>= 2;
								palette[j + 1] >>= 2;
								palette[j + 2] >>= 2;
								j += 3;
							}
						}
					}
					else {
						fcode = PALETTEFEHLER;
						modul = DATEI;
					}
				}
			}
			else {
				fcode = NOTPCX;
				modul = DATEI;
			}
		}
		else {
			fcode = READFEHLER;
			modul = DATEI;
		}
		chewy_fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::load_image(const char *fname, byte *sp, byte *palette) {
	Stream *handle;
	tbf_dateiheader *header;
	uint16 i = 0;
	int16 *abmess;
	byte *speicher;
	int16 format;
	header = (tbf_dateiheader *)tmp;
	speicher = sp;
	abmess = (int16 *) speicher;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.'))strcat(filename, ".TBF\0");

	if ((speicher) && (palette)) {
		handle = chewy_fopen(filename, "rb");
		if (handle) {
			if (chewy_fread(header, sizeof(tbf_dateiheader), 1, handle)) {
				format = get_id(header->id);
				if (format != -1) {
					for (i = 0; i < 768; i++)
						palette[i] = header->palette[i];
					chewy_fseek(handle, ((long)(sizeof(tbf_dateiheader) + 1)), SEEK_SET);
					switch (format) {
					case TBFDATEI:
						abmess[0] = header->width;
						abmess[1] = header->height;
						speicher += 4;
						read_tbf_image(handle, header->komp,
						               header->entpsize, speicher);
						break;
					case TPFDATEI:
						abmess[0] = header->width;
						abmess[1] = header->height;
						speicher += 4;
						read_tpf_image(handle, header->komp,
						               header->entpsize, speicher);
						break;
					}
				}
				else {
					fcode = NOTTBF;
					modul = DATEI;
				}
			}
			else {
				fcode = READFEHLER;
				modul = DATEI;
			}
			chewy_fclose(handle);
		}
		else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	}
	else {
		fcode = ZEIGERFEHLER;
		modul = GRAFIK;
	}
}

void datei::load_image(Stream *stream, byte *sp, byte *palette) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	tbf_dateiheader *header;
	uint16 i = 0;
	int16 *abmess;
	byte *speicher;
	int16 format;
	ChunkHead ch;
	header = (tbf_dateiheader *)tmp;
	speicher = sp;
	abmess = (int16 *) speicher;

	if ((speicher) && (palette) && (rs)) {
		if (header->load(rs)) {
			format = get_id(header->id);
			if (format != -1) {
				for (i = 0; i < 768; i++)
					palette[i] = header->palette[i];
				switch (format) {
				case TBFDATEI:
					abmess[0] = header->width;
					abmess[1] = header->height;
					speicher += 4;
					read_tbf_image(rs, header->komp,
					               header->entpsize, speicher);
					break;
				case TPFDATEI:
					abmess[0] = header->width;
					abmess[1] = header->height;
					speicher += 4;
					read_tpf_image(rs, header->komp,
					               header->entpsize, speicher);
					break;
				}
			} else {
				fcode = NOTTBF;
				modul = DATEI;
			}
		} else {
			fcode = READFEHLER;
			modul = DATEI;
		}
	} else {
		fcode = ZEIGERFEHLER;
		modul = GRAFIK;
	}

	if (!modul)
		ch.load(rs);
}

uint16 datei::select_pool_item(Stream *stream, uint16 nr) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	uint32 tmp1;
	NewPhead *ph;
	ph = (NewPhead *)tmp;

	if (rs) {
		rs->seek(0, SEEK_SET);
		if (!ph->load(rs)) {
			modul = DATEI;
			fcode = READFEHLER;
			rs->seek(0, SEEK_SET);
		} else {
			if (!strncmp(ph->id, "NGS", 3)) {
				if (nr >= ph->PoolAnz)
					nr = ph->PoolAnz - 1;

				rs->seek(-(int)((ph->PoolAnz - nr) * sizeof(uint32)), SEEK_END);
				tmp1 = rs->readUint32LE();
				rs->seek(tmp1, SEEK_SET);
			}
		}
	}

	return nr;
}

void datei::load_tafmcga(const char *fname, byte *sp, int16 nr) {
	Common::File f;
	taf_dateiheader *header;
	taf_imageheader iheader;
	int16 *abmess, komp;
	int16 id;
	uint16 sprcount = 0;
	uint32 size = 0, next = 0, image = 0;
	byte *speicher;
	speicher = sp;
	header = (taf_dateiheader *)tmp;

	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';
	if (!filename[0])
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.'))
		strcat(filename, ".taf");

	abmess = (int16 *) speicher;
	speicher += 4;
	if (abmess) {
		if (f.open(filename)) {
			if (header->load(&f)) {
				id = get_id(header->id);
				if (id == TAFDATEI) {
					if (header->korrekt == 1) {
						next = header->next;
						while ((sprcount <= nr) && (nr <= header->count)) {
							f.seek(next, SEEK_SET);
							if (iheader.load(&f)) {
								next = iheader.next;
								image = iheader.image;
							} else {
								fcode = READFEHLER;
								modul = DATEI;
							}
							++sprcount;
						}
					} else {
						f.seek((-(int)((header->count - nr) * sizeof(uint32))), SEEK_END);
						next = f.readUint32LE();
						f.seek(next, SEEK_SET);

						if (iheader.load(&f)) {
							next = iheader.next;
							image = iheader.image;
						} else {
							fcode = READFEHLER;
							modul = DATEI;
						}
					}

					abmess[0] = iheader.width;
					abmess[1] = iheader.height;
					size = (uint32)((uint32)iheader.height) * ((uint32)iheader.width);
					komp = iheader.komp;
					f.seek(image, SEEK_SET);
					read_tbf_image(&f, komp, size, speicher);

				} else {
					fcode = NOTTBF;
					modul = DATEI;
				}
			} else {
				fcode = READFEHLER;
				modul = DATEI;
			}

			f.close();
		} else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	} else {
		fcode = ZEIGERFEHLER;
		modul = GRAFIK;
	}
}

void datei::load_tafmcga(Stream *handle, int16 komp, uint32 size, byte *speicher) {
	read_tbf_image(handle, komp, size, speicher);
}

void datei::load_full_taf(const char *fname, byte *hi_sp, taf_info *tinfo) {
	Common::File f;
	taf_dateiheader *header;
	taf_imageheader iheader;
	int16 komp, id;
	uint16 sprcount, anzahl, *abmess;
	uint32 size = 0, next = 0, image = 0;
	byte *speicher;
	header = (taf_dateiheader *)tmp;

	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';
	if (!filename[0])
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.'))
		strcat(filename, ".taf");

	speicher = hi_sp;
	if (speicher) {
		if (f.open(filename)) {
			if (header->load(&f)) {
				id = get_id(header->id);
				if ((id == TAFDATEI) && (header->mode == 19)) {
					next = header->next;
					anzahl = header->count;
					for (sprcount = 0; (sprcount < anzahl) && (!modul); sprcount++) {
						tinfo->image[sprcount] = speicher;
						f.seek(next, SEEK_SET);
						if (iheader.load(&f)) {
							next = iheader.next;
							image = iheader.image;
						} else {
							fcode = READFEHLER;
							modul = DATEI;
						}
						abmess = (uint16 *)speicher;
						abmess[0] = iheader.width;
						abmess[1] = iheader.height;
						speicher += 4;
						size = (uint32)((uint32)iheader.height) *
						       ((uint32)iheader.width);
						komp = iheader.komp;

						f.seek(image, SEEK_SET);
						read_tbf_image(&f, komp, size, speicher);
						speicher += size;
					}
				} else {
					fcode = NOTTBF;
					modul = DATEI;
				}
			} else {
				fcode = READFEHLER;
				modul = DATEI;
			}

			f.close();
		} else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	} else {
		fcode = ZEIGERFEHLER;
		modul = GRAFIK;
	}
}

void datei::load_korrektur(const char *fname, byte *sp) {
	Common::File f;
	taf_dateiheader *header = (taf_dateiheader *)tmp;
	int16 id;
	byte *speicher;

	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';
	if (!filename[0])
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.'))
		strcat(filename, ".taf");

	speicher = sp;
	if (speicher) {
		if (f.open(filename)) {
			if (header->load(&f)) {
				id = get_id(header->id);
				if ((id == TAFDATEI) && (header->korrekt > 0)) {
					f.seek(-((int)(header->count * sizeof(uint32)) * header->korrekt), SEEK_END);
					if (!f.read(speicher, header->count * sizeof(uint32))) {
						fcode = READFEHLER;
						modul = DATEI;
					}
				} else {
					fcode = NOTTBF;
					modul = DATEI;
				}
			} else {
				fcode = READFEHLER;
				modul = DATEI;
			}

			f.close();
		} else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	} else {
		fcode = ZEIGERFEHLER;
		modul = GRAFIK;
	}
}

void datei::load_tff(const char *fname, byte *speicher) {
	Common::File f;
	tff_header *tff;
	uint32 size;

	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';

	if (speicher) {
		if (*filename == 0)
			get_filename(filename, MAXPATH);
		if (!strchr(filename, '.'))
			strcat(filename, ".tff");

		if (f.open(filename)) {
			tff = (tff_header *)speicher;
			if (tff->load(&f)) {
				size = tff->size;
				if (f.read(speicher + sizeof(tff_header), size) != size) {
					fcode = READFEHLER;
					modul = DATEI;
				}
			}

			f.close();
		} else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	} else {
		fcode = ZEIGERFEHLER;
		modul = GRAFIK;
	}
}

void datei::read_tbf_image(Stream *stream, int16 komp, uint32 size, byte *sp) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	assert(rs);

	uint32 pos = 0;
	uint8 value;
	uint8 count, i;
	byte *speicher;
	speicher = sp;

	if (komp == 1) {
		// Run length encoding using count/value pairs
		for (pos = 0; pos < size;) {
			count = rs->readByte();
			value = rs->readByte();

			for (i = 0; (i < count) && (pos < size); i++) {
				speicher[pos] = value;
				++pos;
			}
		}
	} else {
		rs->read(speicher, size);
		speicher += size;
	}
}

void datei::read_tpf_image(Stream *handle, int16 komp, uint32 size,
                           byte *speicher) {
	uint32 pos = 0;
	char zeichen;
	uint8 count, i = 0;
	int16 plane;
	if (komp == 1) {
		for (plane = 0; plane < 4; plane++) {
			for (pos = (uint32)plane; pos < (size + plane);) {
				count = chewy_fgetc(handle);
				zeichen = chewy_fgetc(handle);
				for (i = 0; i < count && pos < (size + plane); i++) {
					speicher[pos] = zeichen;
					pos += 4;
				}
			}
		}
	}
	else {
		for (plane = 0; plane < 4; plane++) {
			for (pos = (uint32) plane; pos < size + plane;) {
				zeichen = chewy_fgetc(handle);
				speicher[pos] = zeichen;
				pos += 4;
			}
		}
	}
}

void datei::load_dialog(const char *fname, dialogue *dial, menue *men,
                        knopf *knpf) {
	Stream *handle;
	int16 i = 0, j = 0;
	knopf *lknpf;
	menue *lmen;
	lknpf = knpf;
	lmen = men;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	i = 0;
	while ((filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)))
		i++;
	filename[i] = '.';
	filename[i + 1] = 'T';
	filename[i + 2] = 'D';
	filename[i + 3] = 'F';
	filename[i + 4] = 0;

	handle = chewy_fopen(filename, "rb");
	if (handle) {
		if (!(chewy_fread(dial, sizeof(dialogue), 1, handle))) {
			fcode = READFEHLER;
			modul = DATEI;
		}
		else {
			for (i = 0; (i < dial->anzmenue) && (!modul); i++) {
				if (!(chewy_fread(lmen, sizeof(menue), 1, handle))) {
					fcode = READFEHLER;
					modul = DATEI;
				}
				else {
					dial->menueliste[i] = lmen;
					for (j = 0; (j < lmen->anzknoepfe) && (!modul); j++) {
						if (!(chewy_fread(lknpf, sizeof(knopf), 1, handle))) {
							fcode = READFEHLER;
							modul = DATEI;
						}
						else {
							lmen->knopfliste[j] = lknpf;
							lknpf++;
						}
					}
					++lmen;
				}
			}
		}
		chewy_fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::load_sbi(const char *fname, sbi_inst *speicher) {
	Stream *handle;
	int16 i = 0;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	i = 0;
	while ((filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)))
		i++;
	filename[i] = '.';
	filename[i + 1] = 'S';
	filename[i + 2] = 'B';
	filename[i + 3] = 'I';
	filename[i + 4] = 0;

	handle = chewy_fopen(filename, "rb");
	if (handle) {
		if (!(chewy_fread(speicher, sizeof(sbi_inst), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		chewy_fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
	if (strncmp((char *)speicher->id, "SBI", 3)) {
		modul = DATEI;
		fcode = NOTSBI;
	}
}

uint32 datei::load_voc(const char *fname, byte *speicher) {
	Stream *handle;
	int16 i = 0;
	voc_header *header;
	uint32 s;
	byte *sp = nullptr;
	header = (voc_header *)tmp;
	sp = speicher;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	i = 0;
	while ((filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)))
		i++;
	filename[i] = '.';
	filename[i + 1] = 'V';
	filename[i + 2] = 'O';
	filename[i + 3] = 'C';
	filename[i + 4] = 0;

	s = size(filename, VOCDATEI);

	if (!modul) {
		handle = chewy_fopen(filename, "rb");
		if (handle) {
			if (!(chewy_fread(header, sizeof(voc_header), 1, handle))) {
				modul = DATEI;
				fcode = READFEHLER;
			} else {
				if (strncmp(header->id, "Creative Voice File", 19)) {
					modul = DATEI;
					fcode = NOTVOC;
				}
				else {
					chewy_fseek(handle, header->offset, SEEK_SET);
					if (!(chewy_fread(sp, s, 1, handle))) {
						modul = DATEI;
						fcode = READFEHLER;
					}
				}
			}
			chewy_fclose(handle);
		}
		else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	}
	return (s);
}

uint32 datei::load_voc(Stream *handle, byte *speicher) {
	ChunkHead *ch;
	ch = (ChunkHead *) tmp;
	if (handle) {
		chewy_fseek(handle, -(int)sizeof(ChunkHead), SEEK_CUR);
		if (!(chewy_fread(ch, sizeof(ChunkHead), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		} else {
			if (ch->type == VOCDATEI) {
				if (!(chewy_fread(speicher, ch->size, 1, handle))) {
					modul = DATEI;
					fcode = READFEHLER;
				}
			}
			else {
				modul = DATEI;
				fcode = NOTVOC;
			}
		}
	}
	return (ch->size);
}

void datei::load_vocinfo(const char *fname, voc_header *speicher) {
	Stream *handle;
	int16 i = 0;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	i = 0;
	while ((filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)))
		i++;
	filename[i] = '.';
	filename[i + 1] = 'V';
	filename[i + 2] = 'O';
	filename[i + 3] = 'C';
	filename[i + 4] = 0;

	handle = chewy_fopen(filename, "rb");
	if (handle) {
		if (!(chewy_fread(speicher, sizeof(voc_header), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		chewy_fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
	if (strncmp(speicher->id, "CREATIVE VOICE FILE", 0x14)) {
		modul = DATEI;
		fcode = NOTVOC;
	}
}

void datei::void_load(const char *fname, byte *speicher, uint32 size) {
	Common::File f;

	strncpy(filename, fname, MAXPATH - 1);
	filename[MAXPATH - 1] = '\0';

	if (f.open(filename)) {
		if (!f.read(speicher, size)) {
			modul = DATEI;
			fcode = READFEHLER;
		}

		f.close();
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

uint32 datei::load_file(const char *fname, byte *speicher) {
	Common::File f;
	uint32 size = 0;

	strncpy(filename, fname, MAXPATH - 1);
	filename[MAXPATH - 1] = '\0';

	if (f.open(filename)) {
		size = f.size();

		if (!f.read(speicher, size)) {
			modul = DATEI;
			fcode = READFEHLER;
		}

		f.close();
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}

	return size;
}

uint32 datei::load_item(Stream *handle, byte *speicher) {
	ChunkHead *ch;
	ch = (ChunkHead *) tmp;
	if (handle) {
		chewy_fseek(handle, -(int)sizeof(ChunkHead), SEEK_CUR);
		if (!(chewy_fread(ch, sizeof(ChunkHead), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		else {
			if (!(chewy_fread(speicher, ch->size, 1, handle))) {
				modul = DATEI;
				fcode = READFEHLER;
			}
		}
	}
	return (ch->size);
}

uint32 datei::load_tmf(const char *fname, tmf_header *th) {
	Common::File f;
	uint32 size = 0;
	byte *speicher = nullptr;
	int16 ok, i;

	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';
	if (!strchr(filename, '.'))
		strcat(filename, ".tmf");

	if (f.open(filename)) {
		size = f.size();

		if (!th->load(&f)) {
			modul = DATEI;
			fcode = READFEHLER;
		} else {
			size -= tmf_header::SIZE();
			speicher = (byte *)th + sizeof(tmf_header);
			ok = 0;
			if (!strncmp(th->id, "TMF", 3))
				ok = 1;
			if (!ok) {
				modul = DATEI;
				fcode = NOTTBF;
			}
		}
		if (!modul) {
			if (f.read(speicher, size) != size) {
				modul = DATEI;
				fcode = READFEHLER;
			}
		}

		f.close();
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}

	if (!modul) {
		speicher = (byte *)th;
		speicher += sizeof(tmf_header);
		speicher += ((uint32)th->pattern_anz) * 1024l;
		for (i = 0; i < 31; i++) {
			if (th->instrument[i].laenge) {
				th->ipos[i] = speicher;
				speicher += th->instrument[i].laenge;
			}
		}
	}

	return size + sizeof(tmf_header);
}

uint32 datei::load_tmf(Stream *handle, tmf_header *song) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(handle);
	ChunkHead *ch = (ChunkHead *)tmp;
	byte *speicher = nullptr;
	uint32 size = 0;
	int16 i;

	if (rs) {
		rs->seek(-(int)ChunkHead::SIZE(), SEEK_CUR);
		if (!ch->load(rs)) {
			modul = DATEI;
			fcode = READFEHLER;
		} else {
			if (ch->type == TMFDATEI) {
				assert(ch->size > tmf_header::SIZE());

				if (!song->load(rs)) {
					modul = DATEI;
					fcode = READFEHLER;
				} else {
					size = ch->size + sizeof(tmf_header);
					speicher = (byte *)song + sizeof(tmf_header);
					speicher += ((uint32)song->pattern_anz) * 1024l;
					for (i = 0; i < 31; ++i) {
						if (song->instrument[i].laenge) {
							song->ipos[i] = speicher;
							speicher += song->instrument[i].laenge;
						}
					}
				}
			} else {
				modul = DATEI;
				fcode = NOTTBF;
			}
		}
	}

	return size;
}

void datei::save_pcx(const char *fname, byte *speicher, byte *palette) {
	Stream *handle;
	pcx_header *header;

	byte *line_ptr, *sptr;
	uint16 i;
	int16 curr_data;
	int16 prev_data;
	int16 data_count;
	int16 line_count;
	int16 hoehe;
	int16 breite;
	int16 *abmess;
	header = (pcx_header *)tmp;
	abmess = (int16 *)speicher;
	sptr = speicher + 4;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	for (i = 0; (filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)); i++);
	filename[i] = '.';
	filename[i + 1] = 'P';
	filename[i + 2] = 'C';
	filename[i + 3] = 'X';
	filename[i + 4] = 0;

	breite = abmess[0];
	hoehe = abmess[1];

	header->id = 10;
	header->version = 5;
	header->bpp = 8;
	header->bpz = breite;
	header->planes = 1;
	header->screenx = breite;
	header->screeny = hoehe;
	header->hodpi = breite;
	header->verdpi = hoehe;
	header->xmax = breite - 1;
	header->xmin = 0;
	header->ymax = hoehe - 1;
	header->ymin = 0;
	header->komp = 1;
	header->reserviert = 0;
	header->palinfo = 1;
	handle = chewy_fopen(filename, "wb");
	if (handle) {
		if (chewy_fwrite(header, sizeof(pcx_header), 1, handle)) {
			chewy_fseek(handle, 128L, SEEK_SET);
			line_ptr = sptr;
			for (i = 0; i < hoehe; i++) {
				prev_data = *line_ptr++;
				data_count = 1;
				line_count = 1;

				while (line_count < breite) {
					curr_data = *line_ptr++;
					line_count++;
					if (curr_data == prev_data) {
						data_count++;
						if (data_count == 0x3f) {
							if (((prev_data & 0xc0) == 0xc0) || data_count > 1)
								chewy_fputc(0xc0 | data_count, handle);
							chewy_fputc(prev_data, handle);
							data_count = 0;
						}
					}
					else {
						if (data_count > 0) {
							if (((prev_data & 0xc0) == 0xc0) || data_count > 1)
								chewy_fputc(0xc0 | data_count, handle);
							chewy_fputc(prev_data, handle);
						}
						prev_data = curr_data;
						data_count = 1;
					}
				}
				if (data_count > 0) {
					if (((prev_data & 0xc0) == 0xc0) || data_count > 1)
						chewy_fputc(0xc0 | data_count, handle);
					chewy_fputc(prev_data, handle);
				}
			}
			chewy_fputc(12, handle);
			for (i = 0; i < 768; i++)
				chewy_fputc(palette[i] << 2, handle);
		}
		else {
			fcode = WRITEFEHLER;
			modul = DATEI;
		}
		chewy_fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::save_tafmcga(const char *fname, byte **spvekt, byte *palette,
                         int16 komp, int16 *korrektur) {
	Stream *handle;
	taf_dateiheader *header;
	taf_imageheader iheader;
	uint16 i = 0;
	uint16 sprcount = 0;
	int16 *abmess;
	byte *speicher;
	uint32 size = 0, allsize = 0, next = 0;
	header = (taf_dateiheader *)tmp;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	i = 0;
	while ((filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)))
		i++;
	filename[i] = '.';
	filename[i + 1] = 'T';
	filename[i + 2] = 'A';
	filename[i + 3] = 'F';
	filename[i + 4] = 0;

	handle = chewy_fopen(filename, "wb");
	if (handle) {
		strncpy(header->id, "TAF\0", 4);
		header->mode = 19;
		for (i = 0; i < 768; i++)
			header->palette[i] = palette[i];
		chewy_fseek(handle, sizeof(taf_dateiheader) + 1, SEEK_SET);
		header->next = chewy_ftell(handle);
		abmess = (int16 *) spvekt[sprcount];
		speicher = spvekt[sprcount];
		speicher += 4;
		next = chewy_ftell(handle);
		while ((abmess != 0) && (sprcount < MAXSPRITE)) {
			chewy_fseek(handle, next + (sizeof(taf_imageheader) + 1), SEEK_SET);
			iheader.image = chewy_ftell(handle);
			iheader.komp = komp;
			iheader.width = abmess[0];
			iheader.height = abmess[1];
			size = (uint32)((long)iheader.height) * ((long)iheader.width);
			allsize += size;
			write_tbf_image(handle, komp, size, speicher);
			if (!modul) {
				iheader.next = chewy_ftell(handle);
				chewy_fseek(handle, next, SEEK_SET);
				if (!chewy_fwrite(&iheader, sizeof(taf_imageheader), 1, handle)) {
					fcode = WRITEFEHLER;
					modul = DATEI;
				}
				next = iheader.next;
			}
			++sprcount;
			abmess = (int16 *) spvekt[sprcount];
			speicher = spvekt[sprcount];
			speicher += 4;
		}
		header->allsize = allsize;
		header->count = sprcount;
		if (!korrektur)
			header->korrekt = 0;
		else {
			header->korrekt = 1;
			chewy_fseek(handle, 0l, SEEK_END);
			if (!chewy_fwrite(korrektur, 4 * sprcount, 1, handle)) {
				fcode = WRITEFEHLER;
				modul = DATEI;
			}
		}
		chewy_fseek(handle, 0L, SEEK_SET);
		if (!chewy_fwrite(header, sizeof(taf_dateiheader), 1, handle)) {
			fcode = WRITEFEHLER;
			modul = DATEI;
		}
		chewy_fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::save_tff(const char *fname, byte *speicher) {
	Stream *handle;
	uint32 size;
	uint16 i = 0;
	tff_header *tff;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if (speicher) {
		tff = (tff_header *) speicher;
		if ((*filename) == 0)
			get_filename(filename, MAXPATH);
		if (!strchr(filename, '.'))strcat(filename, ".TFF\0");
		i = 0;
		while ((filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)))
			i++;
		filename[i] = '.';
		filename[i + 1] = 'T';
		filename[i + 2] = 'F';
		filename[i + 3] = 'F';
		filename[i + 4] = 0;

		handle = chewy_fopen(filename, "wb");
		if (handle) {
			tff->size = (uint32)tff->count * ((uint32)(tff->width / 8) *
			                                       (uint32)tff->height);
			size = tff->size + sizeof(tff_header);
			if (!(chewy_fwrite(speicher, (size_t)size, 1, handle))) {
				fcode = WRITEFEHLER;
				modul = DATEI;
			}
			chewy_fclose(handle);
		}
		else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	}
	else {
		fcode = ZEIGERFEHLER;
		modul = GRAFIK;
	}
}

void datei::write_tbf_image(Stream *handle, int16 komp, uint32 size, byte *speicher) {
	uint32 pos = 0;
	int16 fehler;
	uint8 zeichen;
	uint8 count;
	if (komp == 1) {
		for (pos = 0; pos < size;) {
			count = 0;
			zeichen = speicher[pos];
			while (speicher[pos] == zeichen && count < 254 && pos < size) {
				++pos;
				++count;
			}
			fehler = chewy_fputc((int16)count, handle);
			fehler = chewy_fputc((int16)zeichen, handle);
			if (fehler == EOF) {
				fcode = WRITEFEHLER;
				modul = DATEI;
				break;
			}
		}
	} else {
		if (!chewy_fwrite(speicher, size, 1, handle)) {
			fcode = WRITEFEHLER;
			modul = DATEI;
		}
	}
}

void datei::save_dialog(const char *fname, dialogue *dial) {
	Stream *handle;
	menue *men;
	knopf *knpf;
	int16 i = 0, j = 0;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	i = 0;
	while ((filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)))
		i++;
	filename[i] = '.';
	filename[i + 1] = 'T';
	filename[i + 2] = 'D';
	filename[i + 3] = 'F';
	filename[i + 4] = 0;

	handle = chewy_fopen(filename, "wb");
	if (handle) {
		strncpy(dial->id, "TDF\0", 4);
		if (!(chewy_fwrite(dial, sizeof(dialogue), 1, handle))) {
			fcode = WRITEFEHLER;
			modul = DATEI;
		}
		else {
			for (i = 0; (i < dial->anzmenue) && (!modul); i++) {
				men = dial->menueliste[i];
				if (!(chewy_fwrite(men, sizeof(menue), 1, handle))) {
					fcode = WRITEFEHLER;
					modul = DATEI;
				}
				else {
					for (j = 0; (j < men->anzknoepfe) && (!modul); j++) {
						knpf = men->knopfliste[j];
						if (knpf != 0) {
							if (!(chewy_fwrite(knpf, sizeof(knopf), 1, handle))) {
								fcode = WRITEFEHLER;
								modul = DATEI;
							}
						}
					}
				}
			}
		}
		chewy_fclose(handle);
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::save_tmf(const char *fname, tmf_header *thead) {
	uint16 i = 0;
	char *sp;
	sp = (char *)thead;
	sp += sizeof(tmf_header);
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	i = 0;
	while ((filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)))
		i++;
	filename[i] = '.';
	filename[i + 1] = 'T';
	filename[i + 2] = 'M';
	filename[i + 3] = 'F';
	filename[i + 4] = 0;

	Stream *handle = chewy_fopen(filename, "wb");
	if (handle) {
		if ((chewy_fwrite(thead, sizeof(tmf_header), 1, handle))) {
			for (i = 0; (i < thead->pattern_anz) && (!modul); i++) {
				if (!(chewy_fwrite(sp, 1024, 1, handle))) {
					fcode = WRITEFEHLER;
					modul = DATEI;
				}
				sp += 1024;
			}
			for (i = 0; (i < 31) && (!modul); i++) {
				if (thead->instrument[i].laenge) {
					if (!(chewy_fwrite(thead->ipos[i], (size_t)thead->instrument[i].laenge, 1,
					             handle))) {
						fcode = WRITEFEHLER;
						modul = DATEI;
					}
				}
			}
		} else {
			fcode = WRITEFEHLER;
			modul = DATEI;
		}
		chewy_fclose(handle);
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::get_tdfanz(const char *fname, int16 *menueanz, int16 *knopfanz) {
	Stream *handle;
	menue men;
	int16 i = 0;
	dialogue dial;
	*menueanz = 0;
	*knopfanz = 0;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	i = 0;
	while ((filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)))
		i++;
	filename[i] = '.';
	filename[i + 1] = 'T';
	filename[i + 2] = 'D';
	filename[i + 3] = 'F';
	filename[i + 4] = 0;

	handle = chewy_fopen(filename, "rb");
	if (handle) {
		if (!(chewy_fread(&dial, sizeof(dialogue), 1, handle))) {
			fcode = READFEHLER;
			modul = DATEI;
		}
		else {
			if ((!(strncmp(dial.id, "TDF", 3)))) {
				*menueanz = dial.anzmenue;
				for (i = 0; (i < dial.anzmenue) && (!modul); i++) {
					if (!(chewy_fread(&men, sizeof(menue), 1, handle))) {
						fcode = READFEHLER;
						modul = DATEI;
					}
					else {
						*knopfanz += men.anzknoepfe;
						chewy_fseek(handle, (long)(men.anzknoepfe * sizeof(knopf)), SEEK_CUR);
					}
				}
			}
		}
		chewy_fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

uint32 datei::size(const char *fname, int16 typ) {
	Common::File f;
	tbf_dateiheader *tbfheader;
	tff_header tff;
	pcx_header *pcxheader;
	mod_header *mh;
	uint16 hoehe, breite;
	uint32 size = 0;
	int16 id = 0;
	tbfheader = (tbf_dateiheader *)tmp;
	pcxheader = (pcx_header *) tmp;
	mh = (mod_header *)tmp;

	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';
	if (!filename[0])
		get_filename(filename, MAXPATH);

	if (!strchr(filename, '.')) {
		switch (typ) {
		case TBFDATEI:
		case TPFDATEI:
			strcat(filename, ".tbf");
			break;
		case PCXDATEI:
			strcat(filename, ".pcx");
			break;
		case TFFDATEI:
			strcat(filename, ".tff");
			break;
		case VOCDATEI:
			strcat(filename, ".voc");
			break;
		case MODDATEI:
			strcat(filename, ".mod");
			break;
		case TMFDATEI:
			strcat(filename, ".tmf");
			typ = 300;
			break;
		}
	}

	// SCUMMVM: Note to self, use sizeof(structures) for
	// allocating size, not custom ::SIZE() functions
	if (f.open(filename)) {
		switch (typ) {
		case TBFDATEI:
		case TPFDATEI:
			if (chewy_fread(tbfheader, sizeof(tbf_dateiheader), 1, &f)) {
				id = get_id(tbfheader->id);
				if ((id == TBFDATEI) || (id == TPFDATEI)) {
					size = tbfheader->entpsize + 4;
				} else {
					fcode = NOTTBF;
					modul = DATEI;
					size = 0;
				}
			} else {
				fcode = READFEHLER;
				modul = DATEI;
			}
			break;

		case PCXDATEI:
			if (chewy_fread(pcxheader, sizeof(pcx_header), 1, &f)) {
				if ((pcxheader->id == 10) && (pcxheader->version == 5)
				        && (pcxheader->bpp == 8)) {
					hoehe = (pcxheader->ymax - pcxheader->ymin) + 1;
					breite = pcxheader->bpz * pcxheader->planes;
					size = (uint32)((long)hoehe) * ((long)breite) + 4;
				} else {
					fcode = NOTTBF;
					modul = DATEI;
					size = 0;
				}
			} else {
				fcode = READFEHLER;
				modul = DATEI;
			}
			break;

		case TFFDATEI:
			if (tff.load(&f)) {
				id = get_id(tff.id);
				if (id == TFFDATEI) {
					size = tff.size + sizeof(tff_header);
				} else {
					modul = DATEI;
					fcode = NOTTBF;
				}
			} else {
				modul = DATEI;
				fcode = READFEHLER;
			}
			break;

		case VOCDATEI:
			size = (uint32)f.size() - sizeof(voc_header);
			break;

		case MODDATEI:
			size = (uint32)f.size();
			id = 0;

			if (chewy_fread(mh, sizeof(mod_header), 1, &f)) {
				if (!strncmp(mh->id, "M.K.", 4))
					id = 1;
				if (!strncmp(mh->id, "M!K!", 4))
					id = 2;
				if (!strncmp(mh->id, "FLT4", 4))
					id = 3;
				if (!strncmp(mh->id, "FLT8", 4))
					id = 4;
			}
			if (id)
				size -= (uint32)sizeof(mod_header);
			else
				size -= (uint32)sizeof(mod15_header);
			size += sizeof(tmf_header);
			break;

		default:
			size = (uint32)f.size();
			break;
		}

		f.close();
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
		size = 0;
	}

	return size;
}

uint32 datei::get_poolsize(const char *fname, int16 chunk_start, int16 chunk_anz) {
	Common::File f;
	NewPhead *Nph;
	ChunkHead ch;
	int16 i;
	uint32 size;
	Nph = (NewPhead *)tmp;
	size = 0;

	if (f.open(fname)) {
		if (!Nph->load(&f)) {
			modul = DATEI;
			fcode = READFEHLER;
		} else {
			if (!strncmp(Nph->id, "NGS", 3)) {
				select_pool_item(&f, chunk_start);
				f.seek(-(int)ChunkHead::SIZE(), SEEK_CUR);

				for (i = chunk_start; (i < Nph->PoolAnz) && (!modul)
				        && i < (chunk_start + chunk_anz); i++) {
					if (!ch.load(&f)) {
						modul = DATEI;
						fcode = READFEHLER;
						size = 0;
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
		fcode = OPENFEHLER;
		modul = DATEI;
	}

	return size;
}

uint32 datei::get_tafinfo(const char *fname, taf_dateiheader **tafheader) {
	Common::File tafFile;
	uint32 size = 0;
	int16 id;
	taf_dateiheader *tdh;
	tdh = (taf_dateiheader *)tmp;
	*tafheader = tdh;

	strncpy(filename, fname, MAXPATH - 4);
	filename[MAXPATH - 4] = '\0';

	if (!filename[0])
		get_filename(filename, MAXPATH);

	if (!strchr(filename, '.'))
		strcat(filename, ".taf");

	if (tafFile.open(filename)) {
		if (tdh->load(&tafFile)) {
			id = get_id(tdh->id);
			if ((id == TAFDATEI) && (tdh->mode == 19)) {
				size = tdh->allsize + (((uint32)tdh->count) * 8l);

				size += ((uint32)sizeof(taf_info));
			} else {
				fcode = NOTTBF;
				modul = DATEI;
				size = 0;
			}
		} else {
			fcode = NOTTBF;
			modul = DATEI;
			size = 0;
		}

		tafFile.close();
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
		size = 0;
	}

	return size;
}

void datei::load_palette(const char *fname, byte *palette, int16 typ) {
	Common::File f;
	uint8 zeichen;
	uint16 j = 0, i;
	tbf_dateiheader *tbfheader;
	taf_dateiheader *tafheader;
	pcx_header *pcxheader;
	int16 id;

	tbfheader = (tbf_dateiheader *)tmp;
	pcxheader = (pcx_header *)tmp;
	tafheader = (taf_dateiheader *)tmp;

	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';
	if (!filename[0])
		get_filename(filename, MAXPATH);

	if (!strchr(filename, '.')) {
		switch (typ) {
		case TBFDATEI:
		case TPFDATEI:
			strcat(filename, ".tbf");
			break;
		case PCXDATEI:
			strcat(filename, ".pcx");
			break;
		case TAFDATEI:
			strcat(filename, ".taf");
			break;
		default:
			modul = DATEI;
			fcode = NOTTBF;
		}
	}

	if (!modul) {
		if (f.open(filename)) {
			switch (typ) {
			case TBFDATEI:
			case TPFDATEI:
				if (chewy_fread(tbfheader, sizeof(tbf_dateiheader), 1, &f)) {
					id = get_id(tbfheader->id);
					if (((id == TBFDATEI) || (id == TPFDATEI)) &&
					        (tbfheader->mode == 19)) {
						for (i = 0; i < 768; i++)
							palette[i] = tbfheader->palette[i];
					} else {
						fcode = NOTTBF;
						modul = DATEI;
					}
				} else {
					fcode = READFEHLER;
					modul = DATEI;
				}
				break;

			case PCXDATEI:
				if (chewy_fread(pcxheader, sizeof(pcx_header), 1, &f)) {
					if ((pcxheader->id == 10) && (pcxheader->version == 5)
					        && (pcxheader->bpp == 8)) {
						if (!f.seek(-769L, SEEK_END)) {
							zeichen = f.readByte();
							if (zeichen == 12) {
								f.seek(-768L, SEEK_END);
								if ((chewy_fread(palette, 768, 1, &f)) != 1) {
									fcode = PALETTEFEHLER;
									modul = DATEI;
								} else {
									j = 0;
									for (i = 0; i < 255; i++) {
										palette[j] >>= 2;
										palette[j + 1] >>= 2;
										palette[j + 2] >>= 2;
										j += 3;
									}
								}
							} else {
								fcode = PALETTEFEHLER;
								modul = DATEI;
							}
						}
					} else {
						fcode = NOTTBF;
						modul = DATEI;
					}
				} else {
					fcode = READFEHLER;
					modul = DATEI;
				}
				break;

			case TAFDATEI:
				if (tafheader->load(&f)) {
					id = get_id(tafheader->id);
					if ((id == TAFDATEI) && (tafheader->mode == 19)) {
						for (i = 0; i < 768; i++)
							palette[i] = tafheader->palette[i];
					} else {
						fcode = NOTTBF;
						modul = DATEI;
					}
				} else {
					fcode = READFEHLER;
					modul = DATEI;
				}
				break;

			default:
				modul = DATEI;
				fcode = NOTTBF;
			}

			f.close();
		} else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	}
}

void datei::load_palette(Stream *handle, byte *palette) {
	tbf_dateiheader *header;
	uint16 i = 0;
	int format;
	header = (tbf_dateiheader *)tmp;
	if (palette) {
		if (handle) {
			if (chewy_fread(header, sizeof(tbf_dateiheader), 1, handle)) {
				format = get_id(header->id);
				if (format != -1) {
					for (i = 0; i < 768; i++)
						palette[i] = header->palette[i];
					chewy_fseek(handle, -(int)sizeof(tbf_dateiheader), SEEK_CUR);
				} else {
					fcode = NOTTBF;
					modul = DATEI;
				}
			} else {
				fcode = READFEHLER;
				modul = DATEI;
			}
		} else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	} else {
		fcode = ZEIGERFEHLER;
		modul = GRAFIK;
	}
}

void datei::imsize(const char *fname, uint32 *svekt) {
	Common::File f;
	taf_dateiheader *header;
	taf_imageheader iheader;
	uint16 sprcount = 0;
	uint32 next = 0;
	int16 id;
	header = (taf_dateiheader *)tmp;

	strncpy(filename, fname, MAXPATH - 5);
	filename[MAXPATH - 5] = '\0';
	if (!filename[0])
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.'))
		strcat(filename, ".taf");

	if (f.open(filename)) {
		if (header->load(&f)) {
			id = get_id(header->id);
			if ((id == TAFDATEI) && (header->mode == 19)) {
				next = header->next;
				while (sprcount < header->count) {
					f.seek(next, SEEK_SET);
					if (iheader.load(&f)) {
						next = iheader.next;
						svekt[sprcount] = ((uint32)iheader.width) * ((uint32)iheader.width);
					} else {
						fcode = READFEHLER;
						modul = DATEI;
					}
					++sprcount;
				}
			}
		}

		f.close();
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

int16 datei::get_filename(const char *fname, int16 maxlen) {
	warning("STUB - get_filename");

	int16 ret = 0;
#if 0
	char z = 0;
	debugN("Dateiname: ");
	while ((z != 27) && (z != 13) && (ret < maxlen)) {
		z = getch();
		if ((((z & 0xdf) >= 65) && ((z & 0xdf) <= 90))) {
			z &= 0xdf;
			debugN("%c", z);
			fname[ret] = z;
			ret += 1;
		} else if ((z >= 0x30) && (z <= 0x39)) {
			debugN("%c", z);
			fname[ret] = z;
			ret += 1;
		} else if (z == 92) {
			debugN("%c", z);
			fname[ret] = '\\';
			ret += 1;
		} else if (z == 46) {
			debugN("%c", z);
			fname[ret] = z;
			ret += 1;
		} else if (z == 58) {
			debugN("%c", z);
			fname[ret] = z;
			ret += 1;
		} else if (z == 8) {
			debugN("%c", z);
			debugN("%c", 32);
			debugN("%c", 8);
			if (ret > 0)
				ret -= 1;
		} else if ((z != 13) && (z != 27))
			debugN("%c", 7);
	}
	fname[ret] = 0;
	debugN("\n");
	if (z == 27)
		ret = 0;
#endif
	return (ret);
}

void datei::respfad(char *respath, char *resdir) {
	warning("STUB: respfad()");

#if 0
	getcwd(respath, MAXPATH);
	strcat(respath, "/");
	strcat(respath, resdir);
	strcat(respath, "/");
#endif
}

void datei::res2dname(char *respath, char *fnme, char *dtname) {
	strcpy(fnme, respath);
	strcat(fnme, dtname);
}

void datei::cwd(char *buf, int16 buflen) {
	warning("STUB: cwd()");

#if 0
	getcwd(buf, buflen);
#endif
}

void datei::cd(char *dir) {
	warning("STUB: cd()");

#if 0
	char tmpname[16] = {0};
	strncpy(tmpname, "cd ", 3);
	strcat(tmpname, dir);
	system(tmpname);
#endif
}

void datei::write_tpf_image(Stream *handle, int16 komp, uint32 size, byte *speicher) {
	uint32 pos = 0;
	int16 fehler;
	uint8 zeichen;
	uint8 count;
	int16 plane;
	if (komp == 1) {
		plane = 0;
		while (plane <= 3) {
			for (pos = plane; pos < (size);) {
				count = 0;
				zeichen = speicher[pos];
				while ((speicher[pos] == zeichen) && (count < 255) && (pos < (size))) {
					pos += 4;
					++count;
				}
				fehler = chewy_fputc(count, handle);
				fehler = chewy_fputc(zeichen, handle);
				if (fehler == EOF) {
					fcode = WRITEFEHLER;
					modul = DATEI;
					break;
				}
			}
			++plane;
		}
	} else {
		plane = 0;
		while (plane <= 3) {
			for (pos = plane; pos < (size); pos += 4) {
				zeichen = speicher[pos];
				fehler = chewy_fputc(zeichen, handle);
				if (fehler == EOF) {
					fcode = WRITEFEHLER;
					modul = DATEI;
					break;
				}
			}
			++plane;
		}
	}
}

int16 datei::get_id(char *id_code) {
	int16 id;
	id = -1;
	if (!(scumm_strnicmp(id_code, "TBF", 3)))
		id = TBFDATEI;
	if (!(scumm_strnicmp(id_code, "TPF", 3)))
		id = TPFDATEI;
	if (!(scumm_strnicmp(id_code, "TAF", 3)))
		id = TAFDATEI;
	if (!(scumm_strnicmp(id_code, "TFF", 3)))
		id = TFFDATEI;
	return (id);
}

void datei::fcopy(const char *d_fname, const char *s_fname) {
	assert(!strcmp(d_fname, ADSH_TMP));

	Common::File f;
	if (f.open(s_fname)) {
		Common::SeekableWriteStream *ws = g_engine->_tempFiles.createWriteStreamForMember(ADSH_TMP);
		ws->writeStream(&f);
		delete ws;
	} else {
		error("Could not find - %s", s_fname);
	}

#if 0
	Common::File src;
	Common::OutSaveFile *dest;

	if (src.open(s_fname)) {
		dest = g_system->getSavefileManager()->openForSaving(d_fname);
		if (dest) {
			dest->writeStream(&src);
			delete dest;
		} else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}

		src.close();
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
#endif
}

} // namespace Chewy
