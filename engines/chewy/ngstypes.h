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

struct taf_dateiheader {
	char id[4] = { 0 };
	int16 mode = 0;
	int16 count = 0;
	uint32 allsize = 0;
	byte palette[PALETTE_SIZE];
	uint32 next = 0;
	int16 korrekt = 0;

	bool load(Common::SeekableReadStream *src);
};

struct taf_imageheader {
	int16 komp = 0;
	uint16 width = 0;
	uint16 height = 0;
	uint32 next = 0;
	uint32 image = 0;

	bool load(Common::SeekableReadStream *src);
};

struct taf_info {
	int16 anzahl = 0;
	byte *palette = nullptr;
	int16 *korrektur = nullptr;
	byte **image = nullptr;
};

struct taf_seq_info {
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

struct tff_header {
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

#define D_GR 16
#define MAXMENUE 50
#define MAXKNOPF 400

struct knopf {
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

struct menue {
	int16 nr = 0;
	int16 disp = 0;
	int16 typ = 0;
	int16 x = 0;
	int16 y = 0;
	int16 width = 0;
	int16 height = 0;
	int16 anzknoepfe = 0;
	knopf *knopfliste[MAXKNOPF] = { nullptr };
	int16 spritenr = 0;
	char *sprite = nullptr;
	char *spritesave = nullptr;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2 +
			4 * MAXKNOPF + 2 + 4 + 4;
	}
};

struct dialogue {
	char id[4] = { 0 };
	int16 anzmenue = 0;
	menue *menueliste[MAXMENUE] = { nullptr };
	char menuetaf[D_GR] = { '\0' };
	char knopftaf[D_GR] = { '\0' };

	bool load(Common::SeekableReadStream *src);
	bool save(Common::WriteStream *dest);
	static constexpr int SIZE() {
		return 4 + 2 + 4 * MAXMENUE + D_GR + D_GR;
	}
};

struct sbi_inst {
	char id[4];
	char name[32];
	uint8 modmulti;
	uint8 carrmulti;
	uint8 modamp;
	uint8 carramp;
	uint8 modad;
	uint8 carrad;
	uint8 modsr;
	uint8 carrsr;
	uint8 modw;
	uint8 carrw;
	uint8 rv;
	uint8 frei[5];

	bool load(Common::SeekableReadStream *src);
};

struct voc_header {
	char id[0x14];
	uint16 offset;
	uint8 ver_low;
	uint8 ver_high;
	uint16 id_code;

	bool load(Common::SeekableReadStream *src);
};

struct maus_info {
	int16 x = 0;
	int16 y = 0;
	int16 button = 0;
};

struct kb_info {
	char key_code = '\0';
	int scan_code = Common::KEYCODE_INVALID;
};

struct in_zeiger {
	maus_info *minfo = nullptr;
	kb_info *kbinfo = nullptr;
};

struct mod_inst {
	char name[22];
	uint16 laenge;
	char finetune;

	char insvol;
	int16 repstart;
	int16 replen;

	bool load(Common::SeekableReadStream *src);
};

struct mod_header {
	char name[20] = { 0 };
	mod_inst instrument[31];
	char pattern_anz = 0;
	char dummy = 0;
	char sequenz[128] = { 0 };
	char id[4] = { 0 };

	bool load(Common::SeekableReadStream *src);
};

struct mod15_header {
	char name[20];
	mod_inst instrument[15];
	char pattern_anz;
	char dummy;
	char sequenz[128];
	char id[4];

	bool load(Common::SeekableReadStream *src);
};

struct tmf_inst {
	uint8 finetune;
	uint8 insvol;
	uint32 repstart;
	uint32 replen;
	uint32 laenge;

	bool load(Common::SeekableReadStream *src);
};

struct tmf_header {
	char id[4] = { 0 };
	tmf_inst instrument[31];
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

struct channel_info {
	uint8 finetune = 0;
	uint8 volume = 0;
	uint32 repstart = 0;
	uint32 replen = 0;
	uint32 len = 0;
	uint32 pointer = 0;
	uint32 pos = 0;
};

struct VesaInfo {
	int16 ModeNr = 0;
	int16 ModeAvail = 0;
	int16 WriteWin = 0;
	uint32 WinSize = 0;
	int16 Page[20] = { 0 };
	int16 PageAnz = 0;
	uint16 WriteSeg = 0;
	uint16 CallSeg = 0;
	uint16 CallOff = 0;
	uint32 ScreenSize = 0;
	uint32 CopyRest = 0;
	char dummy[10] = { 0 };
};

struct vesa_status_block {
	uint8 id[4] = { 0 };
	uint8 ver_low = 0;
	uint8 ver_high = 0;
	char *name = nullptr;
	uint32 lflag = 0;
	uint16 *codenrs = nullptr;

	uint16 memory = 0;

	char *SoftwareRev = nullptr;
	char *VendorName = nullptr;
	char *ProductName = nullptr;
	char *ProductRev = nullptr;
	char dummy[222] = { 0 };
	char OemData[256] = { 0 };
};

struct vesa_modus_block {
	uint16 mflag = 0;

	uint8 fw_flag = 0;

	uint8 fs_flag = 0;

	uint16 stepgr = 0;
	uint16 wsize = 0;
	uint16 fw_seg = 0;
	uint16 fs_seg = 0;
	void (*page_set)(int16 page) = nullptr;
	uint16 scr_width = 0;
	uint16 x_charsize = 0;
	uint16 y_charsize = 0;
	uint8 x_charwidth = 0;
	uint8 y_charwidth = 0;
	uint8 planes = 0;
	uint8 bppix = 0;
	uint8 memblks = 0;
	uint8 model = 0;
	uint8 blksize = 0;
	char dummy[100] = { 0 };
};

struct iog_init {
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

struct iot_init {
	int16 popx = 0;
	int16 popy = 0;
	char *m_col = nullptr;
	char fname[81] = { 0 };
	uint8 f1 = 0;
	uint8 f2 = 0;
	uint8 f3 = 0;
	uint8 abbruch = 0;

	int16(*save_funktion)(char *fname) = nullptr;
	int16(*load_funktion)(char *fname) = nullptr;
	int16 delay = 0;
};

struct mem_info_blk {
	uint32 size = 0;
	uint32 akt_size = 0;
	uint32 biggest_block = 0;
	uint32 start = 0;
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

struct GedHeader {
	char Id[4] = { 0 };
	int16 X = 0;
	int16 Y = 0;
	uint32 Len = 0;
};

struct cur_blk {
	int16 page_off_x = 0;
	int16 page_off_y = 0;
	byte *cur_back = nullptr;
	int16 xsize = 0;
	int16 ysize = 0;
	byte **sprite = nullptr;
	bool no_back = false;
};

struct cur_ani {
	uint8 ani_anf = 0;
	uint8 ani_end = 0;
	int16 delay = 0;
};

struct fcur_blk {
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

struct real_regs {
	uint32 edi = 0;
	uint32 esi = 0;
	uint32 ebp = 0;
	uint32 reserved = 0;
	uint32 ebx = 0;
	uint32 edx = 0;
	uint32 ecx = 0;
	uint32 eax = 0;
	uint16 flags = 0;
	uint16 es = 0;
	uint16 ds = 0;
	uint16 fs = 0;
	uint16 gs = 0;
	uint16 ip = 0;
	uint16 cs = 0;
	uint16 sp = 0;
	uint16 ss = 0;
};

} // namespace Chewy

#endif
