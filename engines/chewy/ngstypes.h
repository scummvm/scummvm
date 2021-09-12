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

#ifndef CHEWY_NGSTYPES_H
#define CHEWY_NGSTYPES_H

#include "chewy/ngsdefs.h"
#include "common/stream.h"

typedef struct {
	uint32 Len;
	int16 StrAnz;
} TxtChunk;

typedef struct {
	char Id[3];
	uint8 Crypt;
	int16 Anz;
} TcfHeader;

typedef struct {
	char id;
	char version;
	char komp;
	char bpp;
	int16 xmin;
	int16 ymin;
	int16 xmax;
	int16 ymax;
	int16 hodpi;
	int16 verdpi;
	char lcmap[16 * 3];
	char reserviert;
	char planes;
	int16 bpz;
	int16 palinfo;
	int16 screenx;
	int16 screeny;
	char dummy[54];
} pcx_header;

typedef struct {
	char id[4];
	int16 mode;
	int16 komp;
	uint32 entpsize;
	uint16 width;
	uint16 height;
	char palette [768];
} tbf_dateiheader;

struct taf_dateiheader {
	char id[4];
	int16 mode;
	int16 count;
	uint32 allsize;
	char palette[768];
	uint32 next;
	int16 korrekt;

	bool load(Common::ReadStream *src);
};

typedef struct {
	int16 komp;
	uint16 width;
	uint16 height;
	uint32 next;
	uint32 image;
} taf_imageheader;

typedef struct {
	int16 anzahl;
	char *palette;
	int16 *korrektur;
	char **image;
} taf_info;

typedef struct {
	int16 anzahl;
	int16 *korrektur;
	char **image;
} taf_seq_info;

typedef struct {
	char id[4];
	uint16 type;
	uint16 PoolAnz;
} NewPhead;

typedef struct {
	char id[4];
	uint32 size;
	int16 count;
	int16 first;
	int16 last;
	int16 width;
	int16 height;
} tff_header;

#define D_GR 16
#define MAXMENUE 50
#define MAXKNOPF 400

typedef struct {
	int16 typ;
	int16 enable;
	int16 x1;
	int16 y1;
	int16 x2;
	int16 y2;
	int16 spritenr1;

	int16 spritenr2;

	char *inhalt1;
	int16 storlen;

	int16 viewspr3;

	int16 textptr;

} knopf;

typedef struct {
	int16 nr;
	int16 disp;
	int16 typ;
	int16 x;
	int16 y;
	int16 width;
	int16 height;
	int16 anzknoepfe;
	knopf *knopfliste[MAXKNOPF];
	int16 spritenr;
	char *sprite;
	char *spritesave;
} menue;

typedef struct {
	char id[4];
	int16 anzmenue;
	menue *menueliste[MAXMENUE];
	char menuetaf[D_GR];
	char knopftaf[D_GR];
} dialogue;

typedef struct {
	uint8 id[4];
	uint8 name[32];
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
} sbi_inst;

typedef struct {
	char id[0x14];
	uint16 offset;
	uint8 ver_low;
	uint8 ver_high;
	uint16 id_code;
} voc_header;

typedef struct {
	int16 x;
	int16 y;
	int16 button;
} maus_info;

typedef struct {
	char key_code;
	byte scan_code;
} kb_info;

typedef struct {
	maus_info *minfo;
	kb_info *kbinfo;
} in_zeiger;

typedef struct {
	uint16 sbase;
	int16 SbIrq;
	int16 DmaKanal;
	uint16 DmaPageReg;
	uint16 DmaAdrReg;
	uint16 DmaLenReg;
	uint8 DmaMode;
	uint8 DmaEnable;
	uint8 DmaDisable;
	uint8 IrqEnable;
	uint8 IrqDisable;
} sb_vars;

typedef struct {
	char name[22];
	uint16 laenge;
	char finetune;

	char insvol;
	int16 repstart;
	int16 replen;
} mod_inst;

typedef struct {
	char name[20];
	mod_inst instrument[31];
	char pattern_anz;
	char dummy;
	char sequenz[128];
	char id[4];
} mod_header;

typedef struct {
	char name[20];
	mod_inst instrument[15];
	char pattern_anz;
	char dummy;
	char sequenz[128];
	char id[4];
} mod15_header;

typedef struct {
	uint8 finetune;
	uint8 insvol;
	uint32 repstart;
	uint32 replen;
	uint32 laenge;
} tmf_inst;

typedef struct {
	char id[4];
	tmf_inst instrument[31];
	uint8 lied_len;
	uint8 pattern_anz;
	uint8 sequenz[128];
	char *ipos[31];
} tmf_header;

typedef struct {
	int16 musik_playing;
	int16 play_mode;
	int16 pattern_line;
	int16 sequence_pos;
	int16 cur_pattnr;
	char *cur_pattern;
} musik_info;

typedef struct {
	uint8 finetune;
	uint8 volume;
	uint32 repstart;
	uint32 replen;
	uint32 len;
	uint32 pointer;
	uint32 pos;
} channel_info;

typedef struct {
	uint8 install;
	char version[5];
	uint32 size;
} himem_block;

typedef struct {
	int16 ModeNr;
	int16 ModeAvail;
	int16 WriteWin;
	uint32 WinSize;
	int16 Page[20];
	int16 PageAnz;
	uint16 WriteSeg;
	uint16 CallSeg;
	uint16 CallOff;
	uint32 ScreenSize;
	uint32 CopyRest;
	char dummy[10];
} VesaInfo;

typedef struct {
	uint8 id[4];
	uint8 ver_low;
	uint8 ver_high;
	char *name;
	uint32 lflag;
	uint16 *codenrs;

	uint16 memory;

	char *SoftwareRev;
	char *VendorName;
	char *ProductName;
	char *ProductRev;
	char dummy [222];
	char OemData[256];
} vesa_status_block;

typedef struct {
	uint16 mflag;

	uint8 fw_flag;

	uint8 fs_flag;

	uint16 stepgr;
	uint16 wsize;
	uint16 fw_seg;
	uint16 fs_seg;
	void (*page_set)(int16 page);
	uint16 scr_width;
	uint16 x_charsize;
	uint16 y_charsize;
	uint8 x_charwidth;
	uint8 y_charwidth;
	uint8 planes;
	uint8 bppix;
	uint8 memblks;
	uint8 model;
	uint8 blksize;
	char dummy[100];
} vesa_modus_block;

typedef struct {
	int16 Adlib;
	int16 Port;
	int16 Irq;
	int16 Dma;
	int16 SoundSource;

	int16 VideoRam;
	int16 VgaDisplay;

	int16 Ems;
	int16 EmsVerV;
	int16 EmsVerN;
	int16 EmsPages;
	int16 EmsFree;
	char *EmsPage0;
	char *EmsPage1;
	char *EmsPage2;
	char *EmsPage3;
	int16 Joy;

	int16 Himem;
	int16 CpuId;

	int16 Fpu;
	int16 Manuell;
} DetectInfo;

typedef struct {
	char id[4];
	char save_path[30];

	int16 popx;
	int16 popy;
	char *m_col;
	uint8 f1;
	uint8 f2;
	uint8 f3;
	uint8 f4;
	uint8 key_nr;
	void (*save_funktion)(void *handle); // FIXME - (FILE *handle);
	void (*load_funktion)(void *handle); // FIXME - (FILE *handle);
	int16 delay;

} iog_init;

typedef struct {
	int16 popx;
	int16 popy;
	char *m_col;
	char fname[81];
	uint8 f1;
	uint8 f2;
	uint8 f3;
	uint8 abbruch;

	int16(*save_funktion)(char *fname);
	int16(*load_funktion)(char *fname);
	int16 delay;

} iot_init;

typedef struct {
	uint32 size;
	uint32 akt_size;
	uint32 biggest_block;
	uint32 start;
} mem_info_blk;

typedef struct {
	int16 anzahl;
	uint32 palette;
	uint32 korrektur;
	uint32 *image;
} far_taf_info;

typedef struct {
	char Id[4];
	int16 Anz;
} GedPoolHeader;

typedef struct {
	uint32 Len;
	int16 X;
	int16 Y;
	int16 Ebenen;
} GedChunkHeader;

typedef struct {

	char Id[4];
	int16 X;
	int16 Y;
	uint32 Len;
} GedHeader;

typedef struct {
	int16 page_off_x;
	int16 page_off_y;
	char *cur_back;
	int16 xsize;
	int16 ysize;
	char **sprite;
	bool no_back;
} cur_blk;

typedef struct {
	uint8 ani_anf;
	uint8 ani_end;
	int16 delay;
} cur_ani;

typedef struct {
	int16 page_off_x;
	int16 page_off_y;
	uint32 cur_back;
	int16 xsize;
	int16 ysize;
	uint32 *sprite;
	bool no_back;
} fcur_blk;

typedef struct {
	uint32 size;
	uint16 type;
	uint16 frames;
	uint16 width;
	uint16 height;
	uint16 depth;
	uint16 flags;
	uint32 speed;
	uint16 reserved1;
	uint32 created;
	uint32 creator;
	uint32 updated;
	uint32 updater;
	uint16 aspect_dx;
	uint16 aspect_dy;
	uint8 reserved2[38];
	uint32 oframe1;
	uint32 oframe2;
	uint8 reserved3[40];
} FlicHead;

typedef struct {
	uint32 size;
	uint16 type;
	uint16 chunks;
	uint8 reserved[8];
} FrameHead;

typedef struct {
	uint32 size;
	uint16 type;
} ChunkHead;

typedef struct {
	char id[4];
	uint32 size;
	uint16 frames;
	uint16 width;
	uint16 height;
	uint32 speed;
	uint32 oframe1;
} CustomFlicHead;

typedef struct {
	uint32 size;
	uint16 type;
	uint16 chunks;
} CustomFrameHead;

typedef struct {
	char *Fname;
	// FIXME: Was FILE
	void *Handle;
	char *VirtScreen;
	char *TempArea;
	char *SoundSlot;
	char *MusicSlot;
	uint32 MaxSoundSize;
	uint32 MaxMusicSize;
} CustomInfo;

typedef struct {
	uint32 edi;
	uint32 esi;
	uint32 ebp;
	uint32 reserved;
	uint32 ebx;
	uint32 edx;
	uint32 ecx;
	uint32 eax;
	uint16 flags;
	uint16 es;
	uint16 ds;
	uint16 fs;
	uint16 gs;
	uint16 ip;
	uint16 cs;
	uint16 sp;
	uint16 ss;
} real_regs;
#endif
