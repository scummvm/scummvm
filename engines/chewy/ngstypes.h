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
#include "chewy/file.h"

namespace Chewy {

#define D_GR 16
#define MAXMENUE 50
#define MAXKNOPF 400

struct TafInfo {
	int16 anzahl = 0;
	byte *palette = nullptr;
	int16 *korrektur = nullptr;
	byte **image = nullptr;
};

struct TafSeqInfo {
	int16 anzahl = 0;
	int16 *korrektur = nullptr;
	byte **image = nullptr;
};

struct NewPhead {
	char id[4] = { 0 };
	uint16 type = 0;
	uint16 PoolAnz = 0;

	bool load(Common::SeekableReadStream *src);
};

struct TffHeader {
	char id[4] = { 0 };
	uint32 size = 0;
	int16 count = 0;
	int16 first = 0;
	int16 last = 0;
	int16 width = 0;
	int16 height = 0;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() { return 18; }
};

struct Button {
	int16 typ = 0;
	int16 enable = 0;
	int16 x1 = 0;
	int16 y1 = 0;
	int16 x2 = 0;
	int16 y2 = 0;
	int16 spritenr1 = 0;
	int16 spritenr2 = 0;
	char *inhalt1 = nullptr;
	int16 storlen = 0;
	int16 viewspr3 = 0;
	int16 textptr = 0;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 4 + 2 + 2 + 2;
	}
};

struct Menu {
	int16 nr = 0;
	int16 disp = 0;
	int16 typ = 0;
	int16 x = 0;
	int16 y = 0;
	int16 width = 0;
	int16 height = 0;
	int16 anzknoepfe = 0;
	Button *knopfliste[MAXKNOPF] = { nullptr };
	int16 spritenr = 0;
	char *sprite = nullptr;
	char *spritesave = nullptr;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2 +
			4 * MAXKNOPF + 2 + 4 + 4;
	}
};

struct Dialogue {
	char id[4] = { 0 };
	int16 anzmenue = 0;
	Menu *menueliste[MAXMENUE] = { nullptr };
	char menuetaf[D_GR] = { '\0' };
	char knopftaf[D_GR] = { '\0' };

	bool load(Common::SeekableReadStream *src);
	bool save(Common::WriteStream *dest);
	static constexpr int SIZE() {
		return 4 + 2 + 4 * MAXMENUE + D_GR + D_GR;
	}
};

struct MouseInfo {
	int16 x = 0;
	int16 y = 0;
	int16 button = 0;
};

struct KbdInfo {
	char key_code = '\0';
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
	int16 page_off_x = 0;
	int16 page_off_y = 0;
	byte *cur_back = nullptr;
	int16 xsize = 0;
	int16 ysize = 0;
	byte **sprite = nullptr;
	bool no_back = false;
};

struct CurAni {
	uint8 _start = 0;
	uint8 _end = 0;
	int16 _delay = 0;
};

struct FCurBlk {
	int16 page_off_x = 0;
	int16 page_off_y = 0;
	uint32 cur_back = 0;
	int16 xsize = 0;
	int16 ysize = 0;
	uint32 *sprite = nullptr;
	bool no_back = false;
};

struct FlicHead {
	uint32 size = 0;
	uint16 type = 0;
	uint16 frames = 0;
	uint16 width = 0;
	uint16 height = 0;
	uint16 depth = 0;
	uint16 flags = 0;
	uint32 speed = 0;
	uint16 reserved1 = 0;
	uint32 created = 0;
	uint32 creator = 0;
	uint32 updated = 0;
	uint32 updater = 0;
	uint16 aspect_dx = 0;
	uint16 aspect_dy = 0;
	uint8 reserved2[38] = { 0 };
	uint32 oframe1 = 0;
	uint32 oframe2 = 0;
	uint8 reserved3[40] = { 0 };

	bool load(Common::SeekableReadStream *src);
};

struct FrameHead {
	uint32 size = 0;
	uint16 type = 0;
	uint16 chunks = 0;
	uint8 reserved[8] = { 0 };

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() { return 16; }
};

struct ChunkHead {
	uint32 size = 0;
	uint16 type = 0;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() { return 6; }
};

struct CustomFlicHead {
	char id[4] = { 0 };
	uint32 size = 0;
	uint16 frames = 0;
	uint16 width = 0;
	uint16 height = 0;
	uint32 speed = 0;
	uint32 oframe1 = 0;

	bool load(Common::SeekableReadStream *src);
};

struct CustomFrameHead {
	uint32 size = 0;
	uint16 type = 0;
	uint16 chunks = 0;

	bool load(Common::SeekableReadStream *src);
};

struct CustomInfo {
	Stream *Handle = nullptr;
	byte *VirtScreen = 0;
	byte *TempArea = 0;
	byte *MusicSlot = 0;
	uint32 MaxMusicSize = 0;
};

} // namespace Chewy

#endif
