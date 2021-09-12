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

#define FORBIDDEN_SYMBOL_EXCEPTION_fread
#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_fseek
#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_fgetc
#define FORBIDDEN_SYMBOL_EXCEPTION_fputc
#define FORBIDDEN_SYMBOL_EXCEPTION_ftell
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#define FORBIDDEN_SYMBOL_EXCEPTION_getch


#include "common/debug.h"
#include "common/file.h"
#include "chewy/datei.h"

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
int16 *ft ;
datei::datei() {
#if 0
	int16 i;
	char *enstr;
	char a[] = {129, 122, 134, 96, 133, 120, 137, 120, 116, 127, 0}; //"NGS-REVEAL"
	char
	b[] = {121, 124, 127, 120, 146, 128, 116, 129, 116, 122, 120, 133, 146, 91, 118, 92, 146,
		129, 120, 138, 146, 122, 120, 129, 120, 133, 116, 135, 124, 130, 129, 146, 134, 130,
		121, 135, 138, 116, 133, 120, 0 }; // "FILE_MANAGER_(C)_NEW_GENERATION_SOFTWARE"
	for (i = 0; a[i] != 0; i++)
		a[i] -= 51;
	for (i = 0; b[i] != 0; i++)
		b[i] -= 51;
	enstr = (char *)getenv(a);
	if (enstr) {
		printf(b);
		printf("\n");
		delay(800);
	}
#endif
}
datei::~datei() {
}

void datei::load_pcx(const char *fname, byte *speicher, byte *palette) {
	FILE *handle;
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
	correct_fname(filename);
	handle = fopen(filename, "rb");
	if (handle) {
		if (fread(header, sizeof(pcx_header), 1, handle)) {
			if ((header->id == 10) && (header->version == 5) && (header->bpp == 8)) {
				hoehe = (header->ymax - header->ymin) + 1;
				breite = header->bpz * header->planes;

				abmess[0] = breite;
				abmess[1] = hoehe;
				fseek(handle, 128L, SEEK_SET);
				for (i = 0; i < hoehe; i++) {
					index = 0;
					while (index < breite) {
						zeichen = fgetc(handle);
						if (((zeichen & 0xc0) == 0xc0) && (header->komp)) {
							count = zeichen & 0x3f;

							zeichen = fgetc(handle);
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
				if (!(fseek(handle, -769L, SEEK_END))) {
					zeichen = fgetc(handle);
					if (zeichen == 12) {
						fseek(handle, -768L, SEEK_END);
						if ((fread(palette, 768, 1, handle)) != 1) {
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
		fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::load_image(const char *fname, byte *sp, byte *palette) {
	FILE *handle;
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
	correct_fname(filename);
	if ((speicher) && (palette)) {
		handle = fopen(filename, "rb");
		if (handle) {
			if (fread(header, sizeof(tbf_dateiheader), 1, handle)) {
				format = get_id(header->id);
				if (format != -1) {
					for (i = 0; i < 768; i++)
						palette[i] = header->palette[i];
					fseek(handle, ((long)(sizeof(tbf_dateiheader) + 1)), SEEK_SET);
					switch (format) {
					case TBFDATEI:
						abmess[0] = header->width;
						abmess[1] = header->height;
						speicher += 4;
						read_tbf_image((void *)handle, header->komp,
						               header->entpsize, speicher);
						break;
					case TPFDATEI:
						abmess[0] = header->width;
						abmess[1] = header->height;
						speicher += 4;
						read_tpf_image((void *)handle, header->komp,
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
			fclose(handle);
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

void datei::load_image(void *h, byte *sp, byte *palette) {
	FILE *handle = (FILE *)h;
	tbf_dateiheader *header;
	uint16 i = 0;
	int16 *abmess;
	byte *speicher;
	int16 format;
	ChunkHead ch;
	header = (tbf_dateiheader *)tmp;
	speicher = sp;
	abmess = (int16 *) speicher;
	if ((speicher) && (palette) && (handle)) {
		if (fread(header, sizeof(tbf_dateiheader), 1, handle)) {
			format = get_id(header->id);
			if (format != -1) {
				for (i = 0; i < 768; i++)
					palette[i] = header->palette[i];
				switch (format) {
				case TBFDATEI:
					abmess[0] = header->width;
					abmess[1] = header->height;
					speicher += 4;
					read_tbf_image((void *)handle, header->komp,
					               header->entpsize, speicher);
					break;
				case TPFDATEI:
					abmess[0] = header->width;
					abmess[1] = header->height;
					speicher += 4;
					read_tpf_image((void *)handle, header->komp,
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
	}
	else {
		fcode = ZEIGERFEHLER;
		modul = GRAFIK;
	}
	if (!modul)
		fread(&ch, sizeof(ChunkHead), 1, handle);
}

uint16 datei::select_pool_item(void *h, uint16 nr) {
	FILE *handle = (FILE *)h;
	uint32 tmp1;
	NewPhead *ph;
	ph = (NewPhead *)tmp;
	if (handle) {
		fseek(handle, 0, SEEK_SET);
		if (!(fread(ph, sizeof(NewPhead), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
			fseek(handle, 0, SEEK_SET);
		} else {
			if (!strncmp(ph->id, "NGS", 3)) {
				if (nr >= ph->PoolAnz)
					nr = ph->PoolAnz - 1;
				fseek(handle, -(int)((ph->PoolAnz - nr) * sizeof(uint32)), SEEK_END);
				if (!(fread(&tmp1, sizeof(uint32), 1, handle))) {
					modul = DATEI;
					fcode = READFEHLER;
					fseek(handle, 0, SEEK_SET);
				} else
					fseek(handle, tmp1, SEEK_SET);
			}
		}
	}
	return (nr);
}

void datei::load_tafmcga(const char *fname, byte *sp, int16 nr) {
	FILE *handle;
	taf_dateiheader *header;
	taf_imageheader iheader;
	int16 *abmess, komp;
	int16 id, i;
	uint16 sprcount = 0;
	uint32 size = 0, next = 0, image = 0;
	byte *speicher;
	speicher = sp;
	header = (taf_dateiheader *)tmp;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.'))strcat(filename, ".TAF\0");
	correct_fname(filename);
	abmess = (int16 *) speicher;
	speicher += 4;
	if (abmess) {
		handle = fopen(filename, "rb");
		if (handle) {
			if (fread(header, sizeof(taf_dateiheader), 1, handle)) {
				id = get_id(header->id);
				if (id == TAFDATEI) {
					if (header->korrekt == 1) {
						next = header->next;
						while ((sprcount <= nr) && (nr <= header->count)) {
							fseek(handle, next, SEEK_SET);
							if (fread(&iheader, sizeof(taf_imageheader), 1, handle)) {
								next = iheader.next;
								image = iheader.image;
							} else {
								fcode = READFEHLER;
								modul = DATEI;
							}
							++sprcount;
						}
					}
					else {
						fseek(handle, (-(int)((header->count - nr)*sizeof(uint32))), SEEK_END);
						if (!fread(&next, sizeof(uint32), 1, handle)) {
							fcode = READFEHLER;
							modul = DATEI;
						} else {
							fseek(handle, next, SEEK_SET);
							if (fread(&iheader, sizeof(taf_imageheader), 1, handle)) {
								next = iheader.next;
								image = iheader.image;
							} else {
								fcode = READFEHLER;
								modul = DATEI;
							}
						}
					}
					abmess[0] = iheader.width;
					abmess[1] = iheader.height;
					size = (uint32)((uint32)iheader.height) * ((uint32)iheader.width);
					komp = iheader.komp;
					fseek(handle, image, SEEK_SET);
					read_tbf_image((void *)handle, komp, size, speicher);
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
			fclose(handle);
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

void datei::load_tafmcga(void *h, int16 komp, uint32 size, byte *speicher) {
	read_tbf_image(h, komp, size, speicher);
}

void datei::load_full_taf(const char *fname, byte *hi_sp, taf_info *tinfo) {
	FILE *handle;
	taf_dateiheader *header;
	taf_imageheader iheader;
	int16 komp, id, i;
	uint16 sprcount, anzahl, *abmess;
	uint32 size = 0, next = 0, image = 0;
	byte *speicher;
	header = (taf_dateiheader *)tmp;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	i = 0;
	if ((filename[0]) == 0)
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.'))strcat(filename, ".TAF\0");
	correct_fname(filename);
	speicher = hi_sp;
	if (speicher) {
		handle = fopen(filename, "rb");
		if (handle) {
			if (fread(header, sizeof(taf_dateiheader), 1, handle)) {
				id = get_id(header->id);
				if ((id == TAFDATEI) && (header->mode == 19)) {
					next = header->next;
					anzahl = header->count;
					for (sprcount = 0; (sprcount < anzahl) && (!modul); sprcount++) {
						tinfo->image[sprcount] = speicher;
						fseek(handle, next, SEEK_SET);
						if (fread(&iheader, sizeof(taf_imageheader), 1, handle)) {
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
						fseek(handle, image, SEEK_SET);
						read_tbf_image((void *)handle, komp, size, speicher);
						speicher += size;
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
			fclose(handle);
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

void datei::load_korrektur(const char *fname, byte *sp) {
	FILE *handle;
	taf_dateiheader *header;
	int16 id, i;
	byte *speicher;
	header = (taf_dateiheader *)tmp;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	i = 0;
	if ((filename[0]) == 0)
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.'))strcat(filename, ".TAF\0");
	correct_fname(filename);
	speicher = sp;
	if (speicher) {
		handle = fopen(filename, "rb");
		if (handle) {
			if (fread(header, sizeof(taf_dateiheader), 1, handle)) {
				id = get_id(header->id);
				if ((id == TAFDATEI) && (header->korrekt > 0)) {
					fseek(handle, -((int)(header->count * sizeof(uint32))*header->korrekt), SEEK_END);
					if (!fread(speicher, header->count * sizeof(uint32), 1, handle)) {
						fcode = READFEHLER;
						modul = DATEI;
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
			fclose(handle);
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

void datei::load_tff(const char *fname, byte *speicher) {
	FILE *handle;
	tff_header *tff;
	uint32 size;
	int16 i;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if (speicher) {
		tff = (tff_header *)speicher;
		if ((*filename) == 0)
			get_filename(filename, MAXPATH);
		if (!strchr(filename, '.'))strcat(filename, ".TFF\0");
		correct_fname(filename);
		handle = fopen(filename, "rb");
		if (handle) {
			if (fread(speicher, sizeof(tff_header), 1, handle)) {
				size = tff->size;
				if (!(fread((speicher + sizeof(tff_header)),
				            (uint16)size, 1, handle))) {
					fcode = READFEHLER;
					modul = DATEI;
				}
			}
			fclose(handle);
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

void datei::read_tbf_image(void *h, int16 komp, uint32 size, byte *sp) {
	FILE *handle = (FILE *)h;
	uint32 pos;
	char zeichen;
	uint8 count, i;
	byte *speicher;
	speicher = sp;
	pos = 0;
	if (komp == 1) {
		for (pos = 0; pos < size;) {
			count = fgetc(handle);
			zeichen = fgetc(handle);
			for (i = 0; (i < count) && (pos < size); i++) {
				speicher[pos] = zeichen;
				++pos;
			}
		}
	}
	else {
		for (pos = 0; pos < size; pos++)
			*speicher++ = (uint8)fgetc(handle);
	}
}

void datei::read_tpf_image(void *h, int16 komp, uint32 size,
                           byte *speicher) {
	FILE *handle = (FILE *)h;
	uint32 pos = 0;
	char zeichen;
	uint8 count, i = 0;
	int16 plane;
	if (komp == 1) {
		for (plane = 0; plane < 4; plane++) {
			for (pos = (uint32)plane; pos < (size + plane);) {
				count = fgetc(handle);
				zeichen = fgetc(handle);
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
				zeichen = fgetc(handle);
				speicher[pos] = zeichen;
				pos += 4;
			}
		}
	}
}

void datei::load_dialog(const char *fname, dialogue *dial, menue *men,
                        knopf *knpf) {
	FILE *handle;
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
	correct_fname(filename);
	handle = fopen(filename, "rb");
	if (handle) {
		if (!(fread(dial, sizeof(dialogue), 1, handle))) {
			fcode = READFEHLER;
			modul = DATEI;
		}
		else {
			for (i = 0; (i < dial->anzmenue) && (!modul); i++) {
				if (!(fread(lmen, sizeof(menue), 1, handle))) {
					fcode = READFEHLER;
					modul = DATEI;
				}
				else {
					dial->menueliste[i] = lmen;
					for (j = 0; (j < lmen->anzknoepfe) && (!modul); j++) {
						if (!(fread(lknpf, sizeof(knopf), 1, handle))) {
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
		fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::load_sbi(const char *fname, sbi_inst *speicher) {
	FILE *handle;
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
	correct_fname(filename);
	handle = fopen(filename, "rb");
	if (handle) {
		if (!(fread(speicher, sizeof(sbi_inst), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		fclose(handle);
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
	FILE *handle;
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
	correct_fname(filename);
	s = size(filename, VOCDATEI);

	if (!modul) {
		handle = fopen(filename, "rb");
		if (handle) {
			if (!(fread(header, sizeof(voc_header), 1, handle))) {
				modul = DATEI;
				fcode = READFEHLER;
			} else {
				if (strncmp(header->id, "Creative Voice File", 19)) {
					modul = DATEI;
					fcode = NOTVOC;
				}
				else {
					fseek(handle, header->offset, SEEK_SET);
					if (!(fread(sp, s, 1, handle))) {
						modul = DATEI;
						fcode = READFEHLER;
					}
				}
			}
			fclose(handle);
		}
		else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	}
	return (s);
}

uint32 datei::load_voc(void *h, byte *speicher) {
	FILE *handle = (FILE *)h;
	ChunkHead *ch;
	ch = (ChunkHead *) tmp;
	if (handle) {
		fseek(handle, -(int)sizeof(ChunkHead), SEEK_CUR);
		if (!(fread(ch, sizeof(ChunkHead), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		} else {
			if (ch->type == VOCDATEI) {
				if (!(fread(speicher, ch->size, 1, handle))) {
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
	FILE *handle;
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
	correct_fname(filename);
	handle = fopen(filename, "rb");
	if (handle) {
		if (!(fread(speicher, sizeof(voc_header), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		fclose(handle);
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
	FILE *handle;
	int16 i;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	correct_fname(filename);
	handle = fopen(filename, "rb");
	if (handle) {
		if (!(fread(speicher, (uint16)size, 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

uint32 datei::load_file(const char *fname, byte *speicher) {
	FILE *handle;
	uint32 size = 0;
	int16 i = 0;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	correct_fname(filename);
	handle = fopen(filename, "rb");
	if (handle) {
		fseek(handle, 0l, SEEK_END);
		size = ftell(handle);
		fseek(handle, 0l, SEEK_SET);
		if (!(fread(speicher, (size_t)size, 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
	return (size);
}

uint32 datei::load_item(void *h, byte *speicher) {
	FILE *handle = (FILE *)h;
	ChunkHead *ch;
	ch = (ChunkHead *) tmp;
	if (handle) {
		fseek(handle, -(int)sizeof(ChunkHead), SEEK_CUR);
		if (!(fread(ch, sizeof(ChunkHead), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		else {
			if (!(fread(speicher, ch->size, 1, handle))) {
				modul = DATEI;
				fcode = READFEHLER;
			}
		}
	}
	return (ch->size);
}

uint32 datei::load_tmf(const char *fname, tmf_header *th) {
	FILE *handle;
	uint32 size = 0;
	byte *speicher;
	int16 ok, i;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	i = 0;
	while ((filename[i] != '.') && (filename[i] != 0) && (i < (MAXPATH - 5)))
		i++;
	filename[i] = '.';
	filename[i + 1] = 'T';
	filename[i + 2] = 'M';
	filename[i + 3] = 'F';
	filename[i + 4] = 0;
	speicher = (byte *)th;
	correct_fname(filename);
	handle = fopen(filename, "rb");
	if (handle) {
		fseek(handle, 0l, SEEK_END);
		size = ftell(handle);
		fseek(handle, 0l, SEEK_SET);
		if (!(fread(th, sizeof(tmf_header), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		} else {
			size -= sizeof(tmf_header);
			speicher += sizeof(tmf_header);
			ok = 0;
			if (!strncmp(th->id, "TMF", 3))
				ok = 1;
			if (!ok) {
				modul = DATEI;
				fcode = NOTTBF;
			}
		}
		if (!modul) {
			if (!(fread(speicher, size, 1, handle))) {
				modul = DATEI;
				fcode = READFEHLER;
			}
		}
		fclose(handle);
	}
	else {
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
	return (size + sizeof(tmf_header));
}

uint32 datei::load_tmf(void *h, tmf_header *song) {
	FILE *handle = (FILE *)h;
	ChunkHead *ch;
	byte *speicher;
	int16 i;
	ch = (ChunkHead *) tmp;
	speicher = (byte *)song;
	if (handle) {
		fseek(handle, -(int)sizeof(ChunkHead), SEEK_CUR);
		if (!(fread(ch, sizeof(ChunkHead), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		else {
			if (ch->type == TMFDATEI) {
				if (!(fread(speicher, ch->size, 1, handle))) {
					modul = DATEI;
					fcode = READFEHLER;
				}
				else {
					speicher = (byte *)song;
					speicher += sizeof(tmf_header);
					speicher += ((uint32)song->pattern_anz) * 1024l;
					for (i = 0; i < 31; i++) {
						if (song->instrument[i].laenge) {
							song->ipos[i] = speicher;
							speicher += song->instrument[i].laenge;
						}
					}
				}
			}
			else {
				modul = DATEI;
				fcode = NOTTBF;
			}
		}
	}
	return (ch->size);
}

void datei::save_pcx(const char *fname, byte *speicher, byte *palette) {
	FILE *handle;
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
	correct_fname(filename);

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
	handle = fopen(filename, "wb");
	if (handle) {
		if (fwrite(header, sizeof(pcx_header), 1, handle)) {
			fseek(handle, 128L, SEEK_SET);
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
								fputc(0xc0 | data_count, handle);
							fputc(prev_data, handle);
							data_count = 0;
						}
					}
					else {
						if (data_count > 0) {
							if (((prev_data & 0xc0) == 0xc0) || data_count > 1)
								fputc(0xc0 | data_count, handle);
							fputc(prev_data, handle);
						}
						prev_data = curr_data;
						data_count = 1;
					}
				}
				if (data_count > 0) {
					if (((prev_data & 0xc0) == 0xc0) || data_count > 1)
						fputc(0xc0 | data_count, handle);
					fputc(prev_data, handle);
				}
			}
			fputc(12, handle);
			for (i = 0; i < 768; i++)
				fputc(palette[i] << 2, handle);
		}
		else {
			fcode = WRITEFEHLER;
			modul = DATEI;
		}
		fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::save_tafmcga(const char *fname, byte **spvekt, byte *palette,
                         int16 komp, int16 *korrektur) {
	FILE *handle;
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
	correct_fname(filename);
	handle = fopen(filename, "wb");
	if (handle) {
		strncpy(header->id, "TAF\0", 4);
		header->mode = 19;
		for (i = 0; i < 768; i++)
			header->palette[i] = palette[i];
		fseek(handle, sizeof(taf_dateiheader) + 1, SEEK_SET);
		header->next = ftell(handle);
		abmess = (int16 *) spvekt[sprcount];
		speicher = spvekt[sprcount];
		speicher += 4;
		next = ftell(handle);
		while ((abmess != 0) && (sprcount < MAXSPRITE)) {
			fseek(handle, next + (sizeof(taf_imageheader) + 1), SEEK_SET);
			iheader.image = ftell(handle);
			iheader.komp = komp;
			iheader.width = abmess[0];
			iheader.height = abmess[1];
			size = (uint32)((long)iheader.height) * ((long)iheader.width);
			allsize += size;
			write_tbf_image(handle, komp, size, speicher);
			if (!modul) {
				iheader.next = ftell(handle);
				fseek(handle, next, SEEK_SET);
				if (!fwrite(&iheader, sizeof(taf_imageheader), 1, handle)) {
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
			fseek(handle, 0l, SEEK_END);
			if (!fwrite(korrektur, 4 * sprcount, 1, handle)) {
				fcode = WRITEFEHLER;
				modul = DATEI;
			}
		}
		fseek(handle, 0L, SEEK_SET);
		if (!fwrite(header, sizeof(taf_dateiheader), 1, handle)) {
			fcode = WRITEFEHLER;
			modul = DATEI;
		}
		fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::save_tff(const char *fname, byte *speicher) {
	FILE *handle;
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
		correct_fname(filename);
		handle = fopen(filename, "wb");
		if (handle) {
			tff->size = (uint32)tff->count * ((uint32)(tff->width / 8) *
			                                       (uint32)tff->height);
			size = tff->size + sizeof(tff_header);
			if (!(fwrite(speicher, (size_t)size, 1, handle))) {
				fcode = WRITEFEHLER;
				modul = DATEI;
			}
			fclose(handle);
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

void datei::write_tbf_image(void *h, int16 komp, uint32 size, byte *speicher) {
	FILE *handle = (FILE *)h;
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
			fehler = fputc((int16)count, handle);
			fehler = fputc((int16)zeichen, handle);
			if (fehler == EOF) {
				fcode = WRITEFEHLER;
				modul = DATEI;
				break;
			}
		}
	} else {
		if (!fwrite(speicher, size, 1, handle)) {
			fcode = WRITEFEHLER;
			modul = DATEI;
		}
	}
}

void datei::save_dialog(const char *fname, dialogue *dial) {
	FILE *handle;
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
	correct_fname(filename);
	handle = fopen(filename, "wb");
	if (handle) {
		strncpy(dial->id, "TDF\0", 4);
		if (!(fwrite(dial, sizeof(dialogue), 1, handle))) {
			fcode = WRITEFEHLER;
			modul = DATEI;
		}
		else {
			for (i = 0; (i < dial->anzmenue) && (!modul); i++) {
				men = dial->menueliste[i];
				if (!(fwrite(men, sizeof(menue), 1, handle))) {
					fcode = WRITEFEHLER;
					modul = DATEI;
				}
				else {
					for (j = 0; (j < men->anzknoepfe) && (!modul); j++) {
						knpf = men->knopfliste[j];
						if (knpf != 0) {
							if (!(fwrite(knpf, sizeof(knopf), 1, handle))) {
								fcode = WRITEFEHLER;
								modul = DATEI;
							}
						}
					}
				}
			}
		}
		fclose(handle);
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
	correct_fname(filename);
	FILE *handle = fopen(filename, "wb");
	if (handle) {
		if ((fwrite(thead, sizeof(tmf_header), 1, handle))) {
			for (i = 0; (i < thead->pattern_anz) && (!modul); i++) {
				if (!(fwrite(sp, 1024, 1, handle))) {
					fcode = WRITEFEHLER;
					modul = DATEI;
				}
				sp += 1024;
			}
			for (i = 0; (i < 31) && (!modul); i++) {
				if (thead->instrument[i].laenge) {
					if (!(fwrite(thead->ipos[i], (size_t)thead->instrument[i].laenge, 1,
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
		fclose(handle);
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::get_tdfanz(const char *fname, int16 *menueanz, int16 *knopfanz) {
	FILE *handle;
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
	correct_fname(filename);
	handle = fopen(filename, "rb");
	if (handle) {
		if (!(fread(&dial, sizeof(dialogue), 1, handle))) {
			fcode = READFEHLER;
			modul = DATEI;
		}
		else {
			if ((!(strncmp(dial.id, "TDF", 3)))) {
				*menueanz = dial.anzmenue;
				for (i = 0; (i < dial.anzmenue) && (!modul); i++) {
					if (!(fread(&men, sizeof(menue), 1, handle))) {
						fcode = READFEHLER;
						modul = DATEI;
					}
					else {
						*knopfanz += men.anzknoepfe;
						fseek(handle, (long)(men.anzknoepfe * sizeof(knopf)), SEEK_CUR);
					}
				}
			}
		}
		fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

uint32 datei::size(const char *fname, int16 typ) {
	FILE *handle;
	tbf_dateiheader *tbfheader;
	tff_header tff;
	pcx_header *pcxheader;
	mod_header *mh;
	uint16 hoehe, breite;
	uint32 size = 0;
	int16 id = 0, i;
	tbfheader = (tbf_dateiheader *)tmp;
	pcxheader = (pcx_header *) tmp;
	mh = (mod_header *)tmp;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	i = 0;
	correct_fname(filename);
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.')) {
		switch (typ) {
		case TBFDATEI:
		case TPFDATEI:
			strcat(filename, ".TBF\0");
			break;
		case PCXDATEI:
			strcat(filename, ".PCX\0");
			break;
		case TFFDATEI:
			strcat(filename, ".TFF\0");
			break;
		case VOCDATEI:
			strcat(filename, ".VOC\0");
			break;
		case MODDATEI:
			strcat(filename, ".MOD\0");
			break;
		case TMFDATEI:
			strcat(filename, ".TMF\0");
			typ = 300;
			break;
		}
	}
	handle = fopen(filename, "rb");
	if (handle) {
		switch (typ) {
		case TBFDATEI:
		case TPFDATEI:
			if (fread(tbfheader, sizeof(tbf_dateiheader), 1, handle)) {
				id = get_id(tbfheader->id);
				if ((id == TBFDATEI) || (id == TPFDATEI)) {
					size = tbfheader->entpsize + 4;
				}
				else {
					fcode = NOTTBF;
					modul = DATEI;
					size = 0;
				}
			}
			else {
				fcode = READFEHLER;
				modul = DATEI;
			}
			break;

		case PCXDATEI:
			if (fread(pcxheader, sizeof(pcx_header), 1, handle)) {
				if ((pcxheader->id == 10) && (pcxheader->version == 5)
				        && (pcxheader->bpp == 8)) {
					hoehe = (pcxheader->ymax - pcxheader->ymin) + 1;
					breite = pcxheader->bpz * pcxheader->planes;
					size = (uint32)((long)hoehe) * ((long)breite) + 4;
				}
				else {
					fcode = NOTTBF;
					modul = DATEI;
					size = 0;
				}
			}
			else {
				fcode = READFEHLER;
				modul = DATEI;
			}
			break;

		case TFFDATEI:
			if (fread(&tff, sizeof(tff_header), 1, handle)) {
				id = get_id(tff.id);
				if (id == TFFDATEI)
					size = tff.size + sizeof(tff_header);
				else {
					modul = DATEI;
					fcode = NOTTBF;
				}
			}
			else {
				modul = DATEI;
				fcode = READFEHLER;
			}
			break;

		case VOCDATEI:
			fseek(handle, 0, SEEK_END);
			size = (uint32)ftell(handle);
			size -= sizeof(voc_header);
			break;

		case MODDATEI:
			fseek(handle, 0, SEEK_END);
			size = (uint32)ftell(handle);
			id = 0;
			if (fread(mh, sizeof(mod_header), 1, handle)) {
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
			fseek(handle, 0, SEEK_END);
			size = (uint32)ftell(handle);
			break;
		}
		fclose(handle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
		size = 0;
	}
	return (size);
}

uint32 datei::get_poolsize(const char *fname, int16 chunk_start, int16 chunk_anz) {
	FILE *handle;
	NewPhead *Nph;
	ChunkHead ch;
	int16 i;
	uint32 size;
	Nph = (NewPhead *)tmp;
	size = 0;
	handle = fopen(fname, "rb");
	if (handle) {
		if (!(fread(Nph, sizeof(NewPhead), 1, handle))) {
			modul = DATEI;
			fcode = READFEHLER;
		} else {
			if (!strncmp(Nph->id, "NGS", 3)) {
				select_pool_item(handle, chunk_start);
				fseek(handle, -(int)sizeof(ChunkHead), SEEK_CUR);
				for (i = chunk_start; (i < Nph->PoolAnz) && (!modul)
				        && i < (chunk_start + chunk_anz); i++) {
					if (!fread(&ch, sizeof(ChunkHead), 1, handle)) {
						modul = DATEI;
						fcode = READFEHLER;
						size = 0;
					} else {
						if (ch.size > size)
							size = ch.size;
						fseek(handle, ch.size, SEEK_CUR);
					}
				}
			}
		}
		fclose(handle);
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
	return (size);
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
	FILE *handle;
	uint8 zeichen;
	uint16 j = 0;
	tbf_dateiheader *tbfheader;
	taf_dateiheader *tafheader;
	pcx_header *pcxheader;
	uint i = 0;
	int16 id;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	tbfheader = (tbf_dateiheader *)tmp;
	pcxheader = (pcx_header *) tmp;
	tafheader = (taf_dateiheader *)tmp;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.')) {
		switch (typ) {
		case TBFDATEI:
		case TPFDATEI:
			strcat(filename, ".TBF\0");
			break;
		case PCXDATEI:
			strcat(filename, ".PCX\0");
			break;
		case TAFDATEI:
			strcat(filename, ".TAF\0");
			break;
		default:
			modul = DATEI;
			fcode = NOTTBF;
		}
	}
	if (!modul) {
		correct_fname(filename);
		handle = fopen(filename, "rb");
		if (handle) {
			switch (typ) {
			case TBFDATEI:
			case TPFDATEI:
				if (fread(tbfheader, sizeof(tbf_dateiheader), 1, handle)) {
					id = get_id(tbfheader->id);
					if (((id == TBFDATEI) || (id == TPFDATEI)) &&
					        (tbfheader->mode == 19)) {
						for (i = 0; i < 768; i++)
							palette[i] = tbfheader->palette[i];
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
				break;

			case PCXDATEI:
				if (fread(pcxheader, sizeof(pcx_header), 1, handle)) {
					if ((pcxheader->id == 10) && (pcxheader->version == 5)
					        && (pcxheader->bpp == 8)) {
						if (!(fseek(handle, -769L, SEEK_END))) {
							zeichen = fgetc(handle);
							if (zeichen == 12) {
								fseek(handle, -768L, SEEK_END);
								if ((fread(palette, 768, 1, handle)) != 1) {
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
							}
							else {
								fcode = PALETTEFEHLER;
								modul = DATEI;
							}
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
				break;

			case TAFDATEI:
				if (fread(tafheader, sizeof(taf_dateiheader), 1, handle)) {
					id = get_id(tafheader->id);
					if ((id == TAFDATEI) && (tafheader->mode == 19)) {
						for (i = 0; i < 768; i++)
							palette[i] = tafheader->palette[i];
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
				break;

			default:
				modul = DATEI;
				fcode = NOTTBF;
			}
			fclose(handle);
		}
		else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
	}
}

void datei::load_palette(void *h, byte *palette) {
	FILE *handle = (FILE *)h;
	tbf_dateiheader *header;
	uint16 i = 0;
	int format;
	header = (tbf_dateiheader *)tmp;
	if (palette) {
		if (handle) {
			if (fread(header, sizeof(tbf_dateiheader), 1, handle)) {
				format = get_id(header->id);
				if (format != -1) {
					for (i = 0; i < 768; i++)
						palette[i] = header->palette[i];
					fseek(handle, -(int)sizeof(tbf_dateiheader), SEEK_CUR);
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
	taf_dateiheader *header;
	taf_imageheader iheader;
	uint16 sprcount = 0;
	uint32 next = 0;
	int16 id, i;
	header = (taf_dateiheader *)tmp;
	for (i = 0; (i < MAXPATH) && (fname[i] != 0); i++)
		filename[i] = fname[i];
	filename[i] = 0;
	if ((*filename) == 0)
		get_filename(filename, MAXPATH);
	if (!strchr(filename, '.'))strcat(filename, ".TAF\0");
	correct_fname(filename);
	FILE *handle = fopen(filename, "rb");
	if (handle) {
		if (fread(header, sizeof(taf_dateiheader), 1, handle)) {
			id = get_id(header->id);
			if ((id == TAFDATEI) && (header->mode == 19)) {
				next = header->next;
				while (sprcount < header->count) {
					fseek(handle, next, SEEK_SET);
					if (fread(&iheader, sizeof(taf_imageheader), 1, handle)) {
						next = iheader.next;
						svekt[sprcount] = ((uint32)iheader.width) * ((uint32)iheader.width);
					}
					else {
						fcode = READFEHLER;
						modul = DATEI;
					}
					++sprcount;
				}
			}
		}
		fclose(handle);
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}

void datei::correct_fname(char *fname) {
	int16 i = 0;
	while ((fname[i] != '.') && (fname[i] != 0)) {
		if (fname[i] == '/')
			fname[i] = '\\';
		i++;
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

void datei::write_tpf_image(void *h, int16 komp, uint32 size, byte *speicher) {
	FILE *handle = (FILE *)h;
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
				fehler = fputc(count, handle);
				fehler = fputc(zeichen, handle);
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
				fehler = fputc(zeichen, handle);
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
	FILE *shandle;
	FILE *dhandle;
	int16 ch;
	shandle = fopen(s_fname, "rb");
	if (shandle) {
		dhandle = fopen(d_fname, "wb+");
		if (shandle) {
			while ((ch = fgetc(shandle)) != EOF)
				fputc(ch, dhandle);
			fclose(dhandle);
		}
		else {
			fcode = OPENFEHLER;
			modul = DATEI;
		}
		fclose(shandle);
	}
	else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
}
