// ******************************************************
// *  One-Channel Digi (c) 1995 LK AVALON               *
// ******************************************************

#ifndef __DIGI__
#define __DIGI__

// ******************************************************
// *  Constants                                         *
// ******************************************************
// available devices

enum DDEV_TYPE  { DDEV_AUTO = -1,        // auto-detect mode
                  DDEV_QUIET,            // disable sound
                  DDEV_SPK,              // PC speaker
                  DDEV_CVX,              // covox
                  DDEV_SB,               // sb/pro/16/awe32
                  DDEV_GUS               // gus/max
                };

#define                DERR_OK           0        // no error
#define                DERR_INITFAIL     1        // couldn't initialize
#define                DERR_BADDDEV      128      // bad device

#define                SI_SILENT         0        // silent sample
#define                SI_STD            1        // standard 11025Hz, 8-bit sample

// sample info
struct SMPINFO
{
  BYTE far *          saddr;             // address
  WORD                slen;              // length
  WORD                span;              // left/right pan (0-15)
  WORD                sflag;             // flag
};

// ******************************************************
// *  Data                                              *
// ******************************************************
// device id
extern  DDEV_TYPE    DigiDevice;

// device base port
extern  WORD         DigiBasePort;

// device dma no
extern  WORD         DigiDMANo;

// device irq no
extern  WORD         DigiIRQNo;

// sound blaster dsp version
extern  WORD         SBDSPVer;

// gus memory size (in k's)
extern  WORD         GUSDRAMSize;

// call it while interrupting when using pc speaker or covox
// uses ALL registers!
extern  void         (*DigiIRQ0Sub)(void);

// ******************************************************
// *  Driver Code                                       *
// ******************************************************
// Init Digi Device
EC WORD DigiInit        (void);

// Close Digi Device
EC void DigiDone        (void);

// Digi On
EC void DigiOn          (void);

// Digi Off
EC void DigiOff         (void);

// Start Digi
EC WORD DigiStart       (SMPINFO *PSmpInfo);

// Stop Digi
EC word DigiStop        (void);

// GUS Dump Sample
// ReadMe: modify sample info (needs SADDR for gus)
EC void GUSDump         (BYTE far *From, BYTE far *To, WORD Len);
#endif
