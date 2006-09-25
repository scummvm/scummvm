/*
	io_m3sd.c based on io_m3cf.c by SaTa.

	io_m3cf.c based on

	compact_flash.c
	By chishm (Michael Chisholm)

	Hardware Routines for reading a compact flash card
	using the M3 Perfect CF Adapter

	CF routines modified with help from Darkfader

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/


#include "io_m3sd.h"

#ifdef SUPPORT_M3SD

//SD dir control bit cmddir=bit0 clken=bit1
//output
#define SDDIR			(*(volatile u16*)0x8800000)

//SD send get control bit send=bit0 get=bit1
//output
#define SDCON			(*(volatile u16*)0x9800000)

//SD output data obyte[7:0]=AD[7:0]
//output
#define SDODA			(*(volatile u16*)0x9000000)

//SD input data AD[7:0]=ibyte[7:0]
//input
#define SDIDA			(*(volatile u16*)0x9000000)

//readsector data1
#define SDIDA1			(*(volatile u16*)0x9200000)

//readsector data2
#define SDIDA2			(*(volatile u16*)0x9400000)

//readsector data3
#define SDIDA3			(*(volatile u16*)0x9600000)

//SD stutas cmdneg=bit0 cmdpos=bit1 issend=bit2 isget=bit3
//input
#define SDSTA			(*(volatile u16*)0x9800000)

//#define CARD_TIMEOUT	10000000		// Updated due to suggestion from SaTa, otherwise card will timeout sometimes on a write
#define CARD_TIMEOUT	(500*100)		// M3SD timeout nomal:500

//======================================================
bool M3SD_read1sector(u32 sectorn,u32 TAddr)
{
	u32 i;
	int w;
	
	SDCON=0x8;		//	bit3:コマンドモード？
	SDIDA1=0x40+17;		//	コマンド CMD17
	SDIDA2=(sectorn>>7);//	セクタH 9ビット=アドレスH １６ビット
	SDIDA3=(sectorn<<9);//	セクタL 7ビット=アドレスL １６ビット
	SDDIR=0x29;		//	コマンド送信？
	i=0;

	while ( ((SDSTA&0x01) != 0x01)&&(i < CARD_TIMEOUT) )
	{
		i++;
	}
	SDDIR=0x09;
	i=0;
	SDDIR=0x49;
	while ( ((SDSTA&0x40) != 0x40)&&(i < CARD_TIMEOUT) )
	{
		i++;
	}
	SDDIR=0x09;
		
	SDDIR=0x8;//cmd input clken=0 datadir input clock=0
	SDCON=0x4;//send=0 get=0 en25=1 cmd1=0

	w = SDDIR;
	for(w=0;w<0x100;w++)
	{
		u16 d16;
		u8 *d8=(u8 *)&d16;
//		*(u16*)(TAddr+w*2) = SDDIR;	//	16bit
		d16 = SDDIR;	//	16bit
		*(u8 *)(TAddr+w*2) =d8[0];
		*(u8 *)(TAddr+w*2+1) =d8[1];
		
	}
	w = SDDIR;
	w = SDDIR;
	
	if (i >= CARD_TIMEOUT)
		return false;

	return true;
	
} 
//==================================================


//======================================================
void SD_crc16(u16* buff,u16 num,u16* crc16buff);
void SD_data_write(u16 *buff,u16* crc16buff);

u16  Hal4ATA_StatusByte;

void Hal4ATA_GetStatus(void)
{
	Hal4ATA_StatusByte = SDSTA;
}

bool Hal4ATA_WaitOnBusy(void)
{
	Hal4ATA_GetStatus();
	while ( (Hal4ATA_StatusByte & 0x01) != 0x1)
	{
		Hal4ATA_GetStatus();
	}
	return TRUE;
}

bool Hal4ATA_WaitOnBusyNDrdy(void)
{
	Hal4ATA_GetStatus();
	while ( (Hal4ATA_StatusByte&0x40) !=0x40)
	{
		Hal4ATA_GetStatus();
	}
	return TRUE;
}


void SendCommand(u16 command, u32 sectorn)
{
	SDCON=0x8;
	SDIDA1=0x40+command;
	SDIDA2=(sectorn>>7);
	SDIDA3=(sectorn<<9);

	SDDIR=0x29;
	Hal4ATA_WaitOnBusy();
	SDDIR=0x09;
}


#define DMA3SAD      *(u32*)0x040000D4
#define DMA3DAD      *(u32*)0x040000D8
#define DMA3CNT      *(u32*)0x040000DC

void DMA3(u32 src, u32 dst, u32 cnt)
{
	DMA3SAD=src;
	DMA3DAD=dst;
	DMA3CNT=cnt;
}



void PassRespond(u32 num)
{
	u32 i,dmanum;
	
	dmanum=(64+(num<<3))>>2;
	SDDIR=0x8;
	SDCON=0x4;
	DMA3(0x8800000,(u32)&i,0x80400000+dmanum);
}

//bool M3SD_write1sector(u32 sectorn,u16 * p)
bool M3SD_write1sector(u32 sectorn,u32 p)
{
	u16 crc[4];

	SendCommand(24,sectorn);
	PassRespond(6);

	SDDIR=0x4;
	SDCON=0x0;

	SD_crc16((u16 *)p,512,crc);
	SD_data_write((u16 *)p,crc);
	return true;
} 
//==================================================


// GBAMP CF Addresses

#define M3_REG_STS		*(vu16*)(0x09800000)	// Status of the CF Card / Device control

#define M3_DATA			(vu16*)(0x08800000)		// Pointer to buffer of CF data transered from card

// CF Card status
#define CF_STS_INSERTED1		0x20
#define CF_STS_INSERTED2		0x30

/*-----------------------------------------------------------------
M3SD_IsInserted
Is a compact flash card inserted?
bool return OUT:  true if a CF card is inserted
-----------------------------------------------------------------*/
bool M3SD_IsInserted (void) 
{
	int i;
	u16 sta;
	// Change register, then check if value did change
	M3_REG_STS = CF_STS_INSERTED1;

	for(i=0;i<CARD_TIMEOUT;i++)
	{
		sta=M3_REG_STS;
		if((sta == CF_STS_INSERTED1)||(sta == CF_STS_INSERTED2))
		{
			return true;
			//break;
		}
	}
	return false;

//	return ( (sta == CF_STS_INSERTED1)||(sta == CF_STS_INSERTED2) );
//	return true;
}


/*-----------------------------------------------------------------
M3SD_ClearStatus
Tries to make the CF card go back to idle mode
bool return OUT:  true if a CF card is idle
-----------------------------------------------------------------*/
bool M3SD_ClearStatus (void) 
{

//	int i=SDDIR;
	int i;
	u16 sta;

	i = 0;
	M3_REG_STS = CF_STS_INSERTED1;
	while (i < CARD_TIMEOUT)
	{
		sta=M3_REG_STS;
		if(  (sta == CF_STS_INSERTED1)||(sta == CF_STS_INSERTED2)  )break;
		i++;
	}
	if (i >= CARD_TIMEOUT)
		return false;

	return true;
}


/*-----------------------------------------------------------------
M3SD_ReadSectors
Read 512 byte sector numbered "sector" into "buffer"
u32 sector IN: address of first 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool M3SD_ReadSectors (u32 sector, u8 numSecs, void* buffer)
{


	//void M3SD_read1sector(u32 sectorn,u32 TAddr)
	bool r=true;
	int i;
	for(i=0;i<numSecs;i++)
	{
		if(M3SD_read1sector(i + sector , 512*i + (u32) buffer )==false)
		{
			r=false;
			break;
		}
	}
	return r;

}



/*-----------------------------------------------------------------
M3SD_WriteSectors
Write 512 byte sector numbered "sector" from "buffer"
u32 sector IN: address of 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool M3SD_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{

	bool r=true;
	int i;
	for(i=0;i<numSecs;i++)
	{
		if(M3SD_write1sector(i + sector , 512*i + (u32) buffer )==false)
		{
			r=false;
			break;
		}
	}
	return r;

//	return false;


}


/*-----------------------------------------------------------------
M3_Unlock
Returns true if M3 was unlocked, false if failed
Added by MightyMax
-----------------------------------------------------------------*/
bool M3SD_Unlock(void) 
{

	// run unlock sequence
	volatile unsigned short tmp ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08E00002 ;
	tmp = *(volatile unsigned short *)0x0800000E ;
	tmp = *(volatile unsigned short *)0x08801FFC ;
	tmp = *(volatile unsigned short *)0x0800104A ;
	tmp = *(volatile unsigned short *)0x08800612 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08801B66 ;
	tmp = *(volatile unsigned short *)0x08800006 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	// test that we have register access
	vu16 sta;
	sta=M3_REG_STS;
	sta=M3_REG_STS;
	if(  (sta == CF_STS_INSERTED1)||(sta == CF_STS_INSERTED2)  )return true;

	return false;
}

bool M3SD_Shutdown(void) {
	return M3SD_ClearStatus() ;
} ;

bool M3SD_StartUp(void) {
	return M3SD_Unlock() ;
} ;


IO_INTERFACE io_m3sd = {
	DEVICE_TYPE_M3SD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE,
	(FN_MEDIUM_STARTUP)&M3SD_StartUp,
	(FN_MEDIUM_ISINSERTED)&M3SD_IsInserted,
	(FN_MEDIUM_READSECTORS)&M3SD_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&M3SD_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&M3SD_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&M3SD_Shutdown
} ;


LPIO_INTERFACE M3SD_GetInterface(void) {
	return &io_m3sd ;
} ;

#endif // SUPPORT_M3CF
