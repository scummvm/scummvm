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

        void fileRead(void *mem, int len);

        uint32 nextBE32();
        void init();

        void startVideo(short int arg, byte* videofile);

	void setPalette();

	long fileSize;

	Scumm *sm;

	int _frameChanged;

};
