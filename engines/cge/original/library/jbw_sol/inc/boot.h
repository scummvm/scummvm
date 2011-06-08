#ifndef		__BOOT__
#define		__BOOT__

#include	<jbw.h>

#define		BOOTSECT_SIZ	512
#define		BOOTHEAD_SIZ	62
#define		BOOTCODE_SIZ	BOOTSECT_SIZ-BOOTHEAD_SIZ
#define		FreeBoot(b)	free(b)

#ifndef		EC
  #define	EC
#endif

typedef struct {
		 byte	Jmp[3];			// NEAR jump machine code
		 char	OEM_ID[8];		// OEM name and version
		 word	SectSize;		// bytes per sector
		 byte	ClustSize;		// sectors per cluster
		 word	ResSecs;		// sectors before 1st FAT
		 byte	FatCnt;			// number of FATs
		 word	RootSize;		// root directory entries
		 word	TotSecs;		// total sectors on disk
		 byte	Media;			// media descriptor byte
		 word	FatSize;		// sectors per FAT
		 word	TrkSecs;		// sectors per track
		 word	HeadCnt;		// number of sufraces
		 word	HidnSecs;		// special hidden sectors
		 word	_;			//  (unknown: reserved?)
		 dword	lTotSecs;		// total number of sectors
		 word	DriveNum;		// physical drive number
		 byte	XSign;			// extended boot signature
		 dword	Serial;			// volume serial number
		 char	Label[11];		// volume label
		 char	FileSysID[8];		// file system ID
		 char	Code[BOOTCODE_SIZ-8];	// 8 = length of following
		 dword	Secret;			// long secret number
		 byte	BootCheck;		// boot sector checksum
		 byte	BootFlags;		// secret flags
		 word	BootSig;		// boot signature 0xAA55
		} Boot;


EC	Boot *		ReadBoot	(int drive);
EC	byte		CheckBoot	(Boot * boot);
EC	Boolean		WriteBoot	(int drive, Boot * boot);


#endif