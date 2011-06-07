#include	<general.h>
#include	<snddrv.h>

#include	<alloc.h>
#include	<stdio.h>
#include	<conio.h>


#define         MIDI_DEV	DEV_AUTO
#define         DIGI_DEV	DEV_AUTO
#define         XD_FILE		IOHAND

#define		TMR_RATE	64
#define		TMR_DIV		((0x8000/TMR_RATE)*2)



static		ENGINE		Heart	= TMR_DIV;


//------------------------------------------------------------------------





void interrupt ENGINE::NewTimer (...)
{
  static word cntr = TMR_RATE;

  ___1152_Hz___:

  SNDMIDIPlay();
  asm	dec	cntr
  asm	jz	___18_Hz___
  asm	mov	al,0x20		// send...
  asm	out	0x020,al	// ...e-o-i
  return;

  ___18_Hz___:

  asm	mov	cntr,TMR_RATE
  OldTimer();
}






static byte far * Song (const char * fname)
{
  byte far * s = NULL;
  XD_FILE sng = fname;
  if (! sng.Error)
    {
      word siz = (word) sng.Size();
      s = farnew(byte, siz);
      if (s)
	{
	  sng.Read(s, siz);
	  if (sng.Error)
	    {
	      farfree(s);
	      s = NULL;
	    }
	}
    }
  return s;
}



EC void far * Patch (int pat)
{
  void far * p = NULL;
  static char fn[] = "PATCH000.SND";

  wtom(pat, fn+5, 10, 3);
  XD_FILE snd = fn;
  if (! snd.Error)
    {
      word siz = (word) snd.Size();
      p = farnew(byte, siz);
      if (p)
	{
	  snd.Read(p, siz);
	  if (snd.Error)
	    {
	      farfree(p);
	      p = NULL;
	    }
	}
    }
  return p;
}



//------------------------------------------------------------------------



void main (int argc, char ** argv)
{
  if (argc < 2)
    {
      printf("Syntax is:    %s midi_file\n", ProgName());
    }
  else
    {
      byte far * s = Song(argv[1]); //--- Wczytanie pliku MIDI
      if (s)
	{
	  //--------- Przyk’ad wykorzystania funkcji Patch() -------------
	  //char far * pat = (char far *) Patch(1), far * p;
	  //if (pat) for (p = pat; *p != CtrlZ; p ++) putch(*p);
	  //farfree(pat);
	  //------------------ Granie pliku MIDI -------------------------
	  SNDDrvInfo.DDEV = DIGI_DEV;
	  SNDDrvInfo.MDEV = MIDI_DEV;
	  SNDInit();
	  SNDMIDIStart(s);
	  while (! MIDIEndFlag) if (kbhit()) break;
	  while (kbhit()) getch();
	  SNDMIDIStop();
	  farfree(s);
	  SNDDone();
	}
    }
}

