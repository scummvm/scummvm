// ******************************************************
// *  MIDI Driver (c) 1995 LK AVALON                    *
// ******************************************************

#ifndef __MIDI__
#define __MIDI__

// ******************************************************
// *  Constants                                         *
// ******************************************************
// available devices

enum MDEV_TYPE  { MDEV_AUTO = -1,       // auto-detect mode
                  MDEV_QUIET,           // disable sound
                  MDEV_SPK,             // pc speaker
                  MDEV_CVX,             // covox
                  MDEV_SB,              // sb/pro/16/awe32
                  MDEV_GUS,             // gus/max
                  MDEV_GM               // external general midi synthesizer
                };

#define         MERR_OK         0       // no error
#define         MERR_INITFAIL   1       // couldn't initialize
#define         MERR_BADMDEV    128     // bad device

// ******************************************************
// *  Data                                              *
// ******************************************************
// device id
extern  MDEV_TYPE    MIDIDevice;

// device base port
extern  WORD         MIDIBasePort;

// 0 means do NOT play
extern  BYTE         MIDIEnabled;

// 0 means we are NOT playing at the moment
extern  BYTE         MIDIPlayFlag;

// 0 means we have not ended yet
extern  BYTE         MIDIEndFlag;

// ******************************************************
// *  Driver Code                                       *
// ******************************************************
// Init MIDI Device
EC WORD MIDIInit        (void);

// Close MIDI Device
EC void MIDIDone        (void);

// Start MIDI File
EC void MIDIStart       (BYTE far *MIDFile);

// Stop MIDI File
EC void MIDIStop        (void);

// Play MIDI File (to be called while interrupting)
// WARNING: Uses ALL registers!
EC void MIDIPlay        (void);
#endif
