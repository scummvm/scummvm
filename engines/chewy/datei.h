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

#ifndef CHEWY_DATEI_H
#define CHEWY_DATEI_H

#include "chewy/chewy.h"
#include "chewy/ngstypes.h"

class datei {
public:
	datei();
	~datei();

	void load_pcx(const char *fname, byte *speicher, byte *palette);
	void load_image(const char *fname, byte *sp, byte *palette);
	void load_tafmcga(const char *fname, byte *sp, int16 nr);
// FIXME: Were using FILE*
	/**/
	void load_image(void *handle, byte *sp, byte *palette);
	uint16 select_pool_item(void *handle, uint16 nr);
	void load_tafmcga(void *handle, int16 komp, uint32 size, byte *speicher);
	void read_tpf_image(void *handle, int16 komp, uint32 size, byte *speicher);
	uint32 load_voc(void *handle, byte *speicher);
	uint32 load_item(void *handle, byte *speicher);
	uint32 load_tmf(void *handle, tmf_header *song);
	void write_tbf_image(void *handle, int16 komp, uint32 size, byte *speicher);
	void write_tpf_image(void *handle, int16 komp, uint32 size, byte *speicher);
	void load_palette(void *handle, byte *palette);
	/**/
	void load_full_taf(const char *fname, byte *hi_sp, taf_info *tinfo);
	void load_korrektur(const char *fname, byte *sp);
	void load_tff(const char *fname, byte *speicher);
	void load_dialog(const char *fname, dialogue *dial, menue *men, knopf *knpf);
	void load_sbi(const char *fname, sbi_inst *speicher);
	uint32 load_voc(const char *fname, byte *speicher);
	void load_vocinfo(const char *fname, voc_header *speicher);
	void void_load(const char *fname, byte *speicher, uint32 size);
	uint32 load_file(const char *fname, byte *speicher);
	uint32 load_tmf(const char *fname, tmf_header *th);
	void save_pcx(const char *fname, byte *speicher, byte *palette);
	void save_tafmcga(const char *fname, byte **spvekt, byte *palette, int16 komp, int16 *korrektur);
	void save_tff(const char *fname, byte *speicher);
	void save_dialog(const char *fname, dialogue *dial);
	void save_tmf(const char *fname, tmf_header *thead);
	void get_tdfanz(const char *fname, int16 *menueanz, int16 *knopfanz);
	uint32 size(const char *fname, int16 typ);
	uint32 get_poolsize(const char *fname, int16 chunk_start, int16 chunk_anz);
	uint32 get_tafinfo(const char *fname, taf_dateiheader **tafheader);
	void load_palette(const char *fname, byte *palette, int16 typ);
	void imsize(const char *fname, uint32 *svekt);
	void correct_fname(char *fname);
	short get_filename(const char *fname, int16 maxlen);
	void respfad(char *respath, char *resdir);
	void res2dname(char *respath, char *fnme, char *dtname);
	void cwd(char *buf, int16 buflen);
	void cd(char *dir);
	short get_id(char *id_code);
	void fcopy(const char *d_fname, const char *s_fname);
	void read_tbf_image(void *h, int16 komp, uint32 size, byte *sp);

};
#endif
