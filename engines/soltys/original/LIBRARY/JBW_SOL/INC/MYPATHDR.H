typedef struct
{
unsigned int  DataSize;
unsigned int  LoopBeg;
unsigned int  LoopEnd;
unsigned int  SampleRate;
unsigned int  BaseFreq;
unsigned char Pan;
unsigned char Flag;
// bit 0: 8 (0) or 16 (1) bit data
// bit 1: signed (0) or unsigned (1) data
// bit 2: set if looping enabled
// bit 3: set if bidirectional looping
// bit 4: set if backward looping
// bit 5: set if sustaining of (wait for note off)
// bit 6: set if envelopes enabled
// bit 7: set if fast release (ignore last 3 envelope points)
unsigned char EnvRate[6];
unsigned char EnvOffs[6];
} MYPATHDR;
