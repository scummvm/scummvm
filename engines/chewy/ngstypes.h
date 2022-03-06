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

#ifndef CHEWY_NGSTYPES_H
#define CHEWY_NGSTYPES_H

#include "graphics/screen.h"
#include "chewy/ngsdefs.h"

namespace Chewy {

struct TafInfo {
	int16 _count = 0;
	byte *_palette = nullptr;
	int16 *_correction = nullptr;
	byte **_image = nullptr;
};

struct TafSeqInfo {
	int16 _count = 0;
	int16 *_correction = nullptr;
	byte **_image = nullptr;
};

struct NewPhead {
	char id[4] = { 0 };
	uint16 type = 0;
	uint16 PoolAnz = 0;

	bool load(Common::SeekableReadStream *src);
};

struct MouseInfo {
	int16 _button = 0;
};

struct KbdInfo {
	char _keyCode = '\0';
	int scan_code = Common::KEYCODE_INVALID;
};

struct KbdMouseInfo {
	KbdInfo *kbinfo = nullptr;
};

struct TmfInst {
	uint8 finetune;
	uint8 insvol;
	uint32 repstart;
	uint32 replen;
	uint32 laenge;

	bool load(Common::SeekableReadStream *src);
};

struct TmfHeader {
	char id[4] = { 0 };
	TmfInst instrument[31];
	uint8 lied_len = 0;
	uint8 pattern_anz = 0;
	uint8 sequenz[128] = { 0 };
	byte *ipos[31] = { nullptr };

	bool load(Common::SeekableReadStream *src);
	static int SIZE() { return 0x284; }
};

struct musik_info {
	int16 musik_playing = 0;
	int16 play_mode = 0;
	int16 pattern_line = 0;
	int16 sequence_pos = 0;
	int16 cur_pattnr = 0;
	char *cur_pattern = nullptr;
};

struct IogInit {
	char id[4] = { 0 };
	char save_path[30] = { 0 };

	int16 popx = 0;
	int16 popy = 0;
	const byte *m_col = nullptr;
	uint8 f1 = 0;
	uint8 f2 = 0;
	uint8 f3 = 0;
	uint8 f4 = 0;
	uint8 key_nr = 0;
	int16 delay = 0;
};

struct GedPoolHeader {
	char Id[4] = { 0 };
	int16 Anz = 0;

	bool load(Common::SeekableReadStream *src);
};

struct GedChunkHeader {
	uint32 Len = 0;
	int16 X = 0;
	int16 Y = 0;
	int16 Ebenen = 0;

	bool load(Common::SeekableReadStream *src);
};

struct CurBlk {
	byte **sprite = nullptr;
};

struct CurAni {
	uint8 _start = 0;
	uint8 _end = 0;
	int16 _delay = 0;
};

struct ChunkHead {
	uint32 size = 0;
	uint16 type = 0;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() { return 6; }
};

struct CustomInfo {
	byte *TempArea = 0;
	byte *MusicSlot = 0;
	uint32 MaxMusicSize = 0;
};

} // namespace Chewy

#endif
