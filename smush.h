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

#define SP_MAX_FONTS 5

class SmushPlayer {

private:

struct PersistentCodecData37 {
		byte *deltaBuf;
		byte *deltaBufs[2];
		uint32 deltaSize;
		int32 width, height;
		int32 curtable;
		int32 unk2, unk3;
		uint32 flags;
		int16 *table1;
		int32 table_last_pitch;
		byte table_last_flags;
};

struct CodecData {
		byte * out, * src;
		int32 x, y;
		int32 outwidth, outheight;
		int32 w, h, pitch;
		uint32 flags;
};

		FILE * _in;
		int32 fileSize;
		byte * _block;
		uint32 _blockTag;
		uint32 _blockSize;

		byte * _cur;
		byte * _renderBitmap;
		uint32 _frameSize;
		uint32 _frmeTag, _frmeSize;
		bool _frameChanged;

		// Palette
		uint16 _fluPalMul129[768];
		uint16 _fluPalWords[768];
		bool _paletteChanged;
		byte _fluPalette[768];

		// TRES
		byte * _fonts[SP_MAX_FONTS];
		byte * _bufferTres;

		int32 _mixerNum;

		// PSAD: Full Throttle audio
		uint32 _saudSize[8], _saudSubSize[8];
		uint32 _psadTrk[8], _strkRate[8];
		uint32 _saudSubTag[8];

		// IACT: The Dig audio
		uint32 _imusSize[8], _imusSubSize[8];
		uint32 _imusTrk[8], _imusRate[8], _imusChan[8];
		uint32 _imusSubTag[8];
		byte _imusData[8][3];
		uint32 _imusPos[8], _imusCodec[8];

		// Codec37
		PersistentCodecData37 pcd37;
		byte * _deltaBuf; 
		int32 _deltaBufSize;

		bool _lock;
		Scumm * _scumm;

		// TRES
		byte * loadTres();
		void loadFonts();
		uint32 getCharWidth(uint8 c_font, byte txt);
		uint32 getFontHeight(uint8 c_font);
		byte * getStringTRES(int32 number);
		void drawStringTRES(uint32 x, uint32 y, byte * txt);
		void drawCharTRES(uint32 * x, uint32 y, uint32 c_line, uint8 c_font, uint8 color, uint8 txt);

		void openFile(byte* fileName);

		void setPalette();
		void codec44Depack(byte *dst, byte *src, uint32 len);
		void codec1(CodecData * cd);
		void codec37Maketable(PersistentCodecData37 * pcd, int32 pitch, byte idx);
		bool codec37(int32 game, CodecData * cd, PersistentCodecData37 * pcd);
		void codec37Init(PersistentCodecData37 * pcd, int32 width, int32 height);
		void codec37Proc5(int32 game, byte *dst, byte *src, int32 next_offs, int32 bw, int32 bh, int32 pitch, int16 *table);
		void codec37BompDepack(byte *dst, byte *src, int32 len);
		void codec37Proc4(byte *dst, byte *src, int32 next_offs, int32 bw, int32 bh, int32 pitch, int16 *table);
		uint32 nextBE32();
		void nextBlock();

		bool parseTag();		
		void parseAHDR();
		void parseFRME();
		void parseNPAL();
		void parseFOBJ();
		void parsePSAD(); 
		void parseTRES();
		void parseXPAL();
		void parseIACT();

		void init();
		void deinit();
		void go();

public:
		SmushPlayer(Scumm * parent);
		~SmushPlayer();
		void startVideo(short int arg, byte* videofile);
		void update();
};
