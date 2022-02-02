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

#ifndef CHEWY_DATEI_H
#define CHEWY_DATEI_H

#include "chewy/chewy.h"
#include "chewy/ngstypes.h"

namespace Chewy {

class datei {
private:
	void assign_filename(const char *fname, const char *ext);
public:
	datei();
	~datei();

	uint16 select_pool_item(Stream *stream, uint16 nr);
	void load_tafmcga(Stream *stream, int16 komp, uint32 size, byte *speicher);
	uint32 load_tmf(Stream *stream, tmf_header *song);

	void load_full_taf(const char *fname, byte *hi_sp, taf_info *tinfo);
	void load_korrektur(const char *fname, int16 *sp);
	void load_tff(const char *fname, byte *speicher);
	void void_load(const char *fname, byte *speicher, uint32 size);
	uint32 size(const char *fname, int16 typ);
	uint32 get_poolsize(const char *fname, int16 chunk_start, int16 chunk_anz);
	uint32 get_tafinfo(const char *fname, taf_dateiheader **tafheader);
	void load_palette(const char *fname, byte *palette, int16 typ);

	short get_id(char *id_code);
	void fcopy(const char *d_fname, const char *s_fname);
	void read_tbf_image(Stream *stream, int16 komp, uint32 size, byte *sp);
};

} // namespace Chewy

#endif
