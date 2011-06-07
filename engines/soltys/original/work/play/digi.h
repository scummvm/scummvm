// ******************************************************
// *  Digi Driver by Hedges, (c) 1994 LK AVALON         *
// *                                                    *
// *  Ver 2.00: 07-Dec-1994                             *
// ******************************************************

// ******************************************************
// *  Constants                                         *
// ******************************************************
// available devices

#define		DDEV_SPK	1	// pc speaker
#define		DDEV_CVX	2	// covox
#define		DDEV_SB		3	// sb/pro/16/awe32
#define		DDEV_GUS	4	// gus/max

#define		DERR_OK		0	// no error
#define		DERR_INITFAIL	1	// couldn't initialize
#define		DERR_BADDDEV	128	// bad device

#define		SI_SILENT	0	// silent sample
#define		SI_STD		1	// standard 11025Hz, 8-bit sample
#define		SI_LOOPED	2	// ditto but looped

typedef	unsigned char	BYTE;
typedef	unsigned int	WORD;
typedef	unsigned long	DWORD;


// sample info
struct SMPINFO
{
  BYTE far *	saddr;		// address
  WORD		slen;		// length
  WORD		span;		// lenght
  WORD		sflag;		// left/right pane (0-15)
};


// ******************************************************
// *  Data                                              *
// ******************************************************

// '$' terminated copyright message
extern	BYTE	DigiCoprMsg;

// device base port, MUST be initialized!
extern	WORD	DigiBasePort;

// device dma no, MUST be initialized!
extern	WORD	DigiDMANo;

// device irq no, MUST be initialized!
extern	WORD	DigiIRQNo;

// call it while interrupting when using pc speaker or covox - does NOT preserve registers!
extern	void	(*DigiIRQ0Sub)(void);

// gus memory size (in k's)
extern	WORD	GUSDRAMSize;

// 0 means do not play, 1 means do
extern	BYTE	DigiPlayFlag;


// ******************************************************
// *  Driver Code                                       *
// ******************************************************

// Init Digi Device
//  Input: AX contains device id
// Output: AX contains status
//	 DX contains pointer to status text
EC word	DigiInit	(void);

// Close Digi Device
EC void	DigiDone	(void);

// Start Digi
//  Input: DS:SI contain pointer to sample info
// Output: AX contains chan id (-1 if no free chan)
EC word	DigiStart	(void);

// Stop Digi
//  Input: AX contains chan id
EC void	StopDigi	(void);

// GUS Dump Sample
//  Input: ES:SI contain pointer to sample
//	 BX:CX contain pointer to gus dram (from 0 to 1024*[GUSDRAMSize]-1)
//	 DX contains len
// ReadMe: GUSDump, modify sample info (needs SADDR for gus), DigiStart)
EC void	GUSDump		(void);


// ******************************************************
// *  Digi Player Code                                  *
// ******************************************************

// Init Digi Player (PIT at IRQ0)
//  Input: AX contains tempo
EC void	PITDigiInit	(void);

// Digi Player Done (PIT at IRQ0)
EC void	PITDigiDone	(void);
