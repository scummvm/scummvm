/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */



#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct CostumeData;

struct AkosHeader {
	byte x_1[2];
	byte flags;
	byte x_2;
	uint16 num_anims;
	uint16 x_3;
	uint16 codec;
} GCC_PACK;

struct AkosOffset {
	uint32 akcd;
	uint16 akci;
} GCC_PACK;

struct AkosCI {
	uint16 width, height;
	int16 rel_x, rel_y;
	int16 move_x, move_y;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif

struct AkosRenderer {
//protected:
	CostumeData *cd;
	int x, y;											/* where to draw costume */
	byte scale_x, scale_y;				/* scaling */
	byte clipping;								/* clip mask */
	bool charsetmask;
	byte shadow_mode;
	uint16 codec;
	bool mirror;									/* draw actor mirrored */
	byte dirty_id;
	byte *outptr;
	uint outwidth, outheight;

	/* pointer to various parts of the costume resource */
	byte *akos;
	AkosHeader *akhd;

	/* current move offset */
	int move_x, move_y;
	/* movement of cel to decode */
	int move_x_cur, move_y_cur;
	/* width and height of cel to decode */
	int width, height;

	byte *srcptr;
	byte *shadow_table;

	struct {
		/* codec stuff */
		const byte *scaletable;
		byte mask, shl;
		bool doContinue;
		byte repcolor;
		byte replen;
		int scaleXstep;
		int x, y;
		int tmp_x, tmp_y;
		int y_pitch;
		int skip_width;
		byte *destptr;
		byte *mask_ptr;
		int imgbufoffs;
	} v1;

	/* put less used stuff at the bottom to optimize opcodes */
	int draw_top, draw_bottom;
	byte *akpl, *akci, *aksq;
	AkosOffset *akof;
	byte *akcd;

	byte palette[256];

protected:
	Scumm *_vm;

public:

	// Constructor, sets all data to 0
	  AkosRenderer(Scumm *scumm) {
		memset(this, 0, sizeof(AkosRenderer));
		_vm = scumm;
	} bool drawCostume();
	void setPalette(byte *palette);
	void setCostume(int costume);
	void setFacing(Actor * a);
	bool drawCostumeChannel(int chan);
	void codec1();
	void codec5();
	void codec16();
	void codec1_ignorePakCols(int num);
	void c1_spec2();
	void c1_spec3();
	void akos16SetupBitReader(byte *src);
	void akos16PutOnScreen(byte * dest, byte * src, int32 transparency, int32 count);
	void akos16SkipData(int32 numskip);
	void akos16DecodeLine(byte *buf, int32 numbytes, int32 dir);
	void akos16ApplyMask(byte * dest, byte * maskptr, byte bits, int32 count, byte fillwith);
	void akos16Decompress(byte * dest, int32 pitch, byte * src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency);
	void akos16DecompressMask(byte * dest, int32 pitch, byte * src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency, byte * maskptr, int32 bitpos_start);
};

enum AkosOpcodes {
	AKC_Return = 0xC001,
	AKC_SetVar = 0xC010,
	AKC_CmdQue3 = 0xC015,
	AKC_ComplexChan = 0xC020,
	AKC_Jump = 0xC030,
	AKC_JumpIfSet = 0xC031,
	AKC_AddVar = 0xC040,
	AKC_Ignore = 0xC050,
	AKC_IncVar = 0xC060,
	AKC_CmdQue3Quick = 0xC061,
	AKC_SkipStart = 0xC070,
	AKC_SkipE = 0xC070,
	AKC_SkipNE = 0xC071,
	AKC_SkipL = 0xC072,
	AKC_SkipLE = 0xC073,
	AKC_SkipG = 0xC074,
	AKC_SkipGE = 0xC075,
	AKC_StartAnim = 0xC080,
	AKC_StartVarAnim = 0xC081,
	AKC_Random = 0xC082,
	AKC_SetActorClip = 0xC083,
	AKC_StartAnimInActor = 0xC084,
	AKC_SetVarInActor = 0xC085,
	AKC_HideActor = 0xC086,
	AKC_SetDrawOffs = 0xC087,
	AKC_JumpTable = 0xC088,
	AKC_SoundStuff = 0xC089,
	AKC_Flip = 0xC08A,
	AKC_Cmd3 = 0xC08B,
	AKC_Ignore3 = 0xC08C,
	AKC_Ignore2 = 0xC08D,
	AKC_JumpStart = 0xC090,
	AKC_JumpE = 0xC090,
	AKC_JumpNE = 0xC091,
	AKC_JumpL = 0xC092,
	AKC_JumpLE = 0xC093,
	AKC_JumpG = 0xC094,
	AKC_JumpGE = 0xC095,
	AKC_ClearFlag = 0xC09F
};
