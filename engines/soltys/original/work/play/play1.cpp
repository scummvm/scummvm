#include	<general.h>
#include	<engine.h>
#include	<wav.h>
#include	<digi.h>

#include	<string.h>
#include	<stdlib.h>
#include	<alloc.h>
#include	<stdio.h>
#include	<dos.h>
#include	<dir.h>
#include	<io.h>
#include	<fcntl.h>

#define		TMR_RATE		1200
#define		TMR_DIV			((0x8000/TMR_RATE)*2)

#define		DEVICE			DDEV_SB
#define		TIMER_CHAIN             FALSE
#define		TEST_SEC		0

#define		DROP(m,n)		{ printf("%s %s\n", m, n); exit(1); }



struct SCB
{
  byte far *	Ptr;
  word		Siz;
  SCB *		Nxt;
};




static	SCB	Scb	= { NULL, 0, NULL };
static	SCB	wav0;
static	SMPINFO	smpinf;
static	int	WavDev	= DEVICE;




void interrupt ENGINE::NewTimer (...)
{
  static word cntr = 0;
  static byte flag = 0;

  // send E-O-I
  asm	mov	al,0x20
  asm	out	0x20,al

  if (++ cntr == TMR_RATE)
    {
      ++ TimerCount;
      cntr = 0;
    }
  if (Scb.Siz)
    {
      asm	les	bx,dword ptr Scb.Ptr	// take pointer
      asm	mov	al,es:[bx]	// get data byte
      asm	xor	flag,0xFF	// flip flag
      asm	jz	odd		// odd action
      asm	shr	al,2		// div by 4
      asm	out	0x42,al		// play even!
      asm	jmp	short done
      odd:
      asm	inc	bx		// advance pointer
      asm	add	al,es:[bx]	// add adjacent
      asm	rcr	al,1		// average
      asm	shr	al,2
      asm	out	0x42,al		// play odd!
      asm	mov	word ptr Scb.Ptr,bx
      asm	dec	word ptr Scb.Siz // decrement size
      done:
    }
  else
    {
      asm	mov	al,0x7F
      asm	out	0x42,al
    }
  if (! Scb.Siz)
    {
      if (Scb.Nxt)
	{
	  Scb = *Scb.Nxt;
	}
    }
}




//------------------------------------------------------------------------


static void SpkOpen (void)
{
  asm	in	al,0x61
  asm	or	al,0x03
  asm	out	0x61,al
  asm	mov	al,0x90
  asm	out	0x43,al
}





static void SpkClose (void)
{
  asm	in	al,0x61
  asm	and	al,0xFC
  asm	out	0x61,al
}





static void OpenSound (void)
{
  // open device
  switch (WavDev)
    {
      case DDEV_SPK : SpkOpen();
		      break;
      case DDEV_CVX : DROP("Covox is NOT supported yet", "");
		      break;
      case DDEV_SB  : DigiBasePort = 0x220;
		      DigiDMANo = 1;
		      DigiIRQNo = 5;
		      _AX = WavDev;
		      asm	push	si
		      asm	push	di
		      DigiInit();
		      asm	pop	di
		      asm	pop	si
		      break;
      default       : DROP("Invalid device code", "");
		      break;
    }
}





static void CloseSound (void)
{
  if (WavDev == DDEV_SPK) SpkClose();
  else                    DigiDone();
}




static void test (const char * name)
{
  DFILE df(name);
  DATACK * data = LoadWave(&df);
  if (data)
    {
      ENGINE eng = TMR_DIV;
      OpenSound();

      wav0.Ptr = data->Addr();
      wav0.Siz = (word)data->Size();
      wav0.Nxt = NULL; //&wav0;

      smpinf.saddr = data->Addr();
      smpinf.slen = (word)data->Size();
      smpinf.span = 8;
      smpinf.sflag = SI_STD;
      _SI = (word) &smpinf;
      if (WavDev != DDEV_SPK) DigiStart();

      Scb.Nxt = &wav0;

      while (!KeyStat.Alt)
	{
	}
      CloseSound();
    }
}




#pragma argsused
int main (int argc, char ** argv)
{
  test(argv[1]);
  return 0;
}
