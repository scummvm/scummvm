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

struct PersistentCodecData37 {
        byte *deltaBuf;
        byte *deltaBufs[2];
        uint32 deltaSize;
        uint width, height;
        int curtable;
        int unk2,unk3;

        uint16 flags;

        int16 *table1;
        int table_last_pitch;
        byte table_last_flags;
};

struct CodecData {
        byte *out,*src;
        int x,y;
        int outwidth,outheight;
        int w,h,pitch;

        int flags;
};


struct SmushPlayer {
        FILE *_in;
        bool _paletteChanged;
        byte * _block;
        uint32 _blockTag;
        uint32 _blockSize;
        byte *_cur;
        byte *_renderBitmap;
        uint32 _frameSize;
        uint32 _frmeTag, _frmeSize;

        byte *_deltaBuf; 
        int _deltaBufSize; 

        PersistentCodecData37 pcd37;
        
        byte _fluPalette[768];
        uint16 _fluPalMul129[768];
        uint16 _fluPalWords[768];

		/* PSAD: Full Throttle audio */
		uint32 _saudSize[8], _saudSubSize[8];
		uint16 _psadTrk[8], _strkRate[8];
		uint32 _saudSubTag[8];

        void openFile(byte* fileName);
        void nextBlock();

        uint32 fileReadBE32();
        uint32 fileReadLE32();
        void go();

        bool parseTag();		
        void parseAHDR();
        void parseFRME();

        void parseNPAL();
        void parseFOBJ();
        void parsePSAD(); 
        void parseTRES();
        void parseXPAL();
		void parseIACT();
        void fileRead(void *mem, int len);

        uint32 nextBE32();
        void init();

        void startVideo(short int arg, byte* videofile);

	void setPalette();

	long fileSize;

	Scumm *sm;

	int _frameChanged;

};
