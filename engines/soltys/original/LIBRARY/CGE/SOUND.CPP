#include	<general.h>
#include	"startup.h"
#include	"sound.h"

#ifdef	DROP_H
  #include	"drop.h"
#else
  #include	<stdio.h>
  #include	<stdlib.h>
  #define	DROP(m,n)	{ printf("%s [%s]\n", m, n); _exit(1); }
#endif

#include	"text.h"
#include	<cfile.h>
#include	"vol.h"
#include	<alloc.h>


	Boolean		Music		= TRUE;
	FX		Fx		= 16;	// must precede SOUND!!
	SOUND		Sound;



SOUND::SOUND (void)
{
  if (STARTUP::SoundOk) Open();
}




SOUND::~SOUND (void)
{
  Close();
}





void SOUND::Close (void)
{
  KillMIDI();
  SNDDone();
}





void SOUND::Open (void)
{
  SNDInit();
  Play(Fx[30000], 8);
}




void SOUND::Play (DATACK * wav, int pan, int cnt)
{
  if (wav)
    {
      Stop();
      smpinf.saddr = (char far *) &*(wav->EAddr());
      smpinf.slen = (word)wav->Size();
      smpinf.span = pan;
      smpinf.sflag = cnt;
      SNDDigiStart(&smpinf);
    }
}




void SOUND::Stop (void)
{
  SNDDigiStop(&smpinf);
}


//------------------------------------------------------------------------








FX::FX (int size)
: Emm(0L), Current(NULL)
{
  Cache = new HAN[size];
  for (Size = 0; Size < size; Size ++)
    {
      Cache[Size].Ref = 0;
      Cache[Size].Wav = NULL;
    }
}




FX::~FX (void)
{
  Clear();
  delete[] Cache;
}





void FX::Clear (void)
{
  HAN * p, * q;
  for (p = Cache, q = p+Size; p < q; p ++)
    {
      if (p->Ref)
	{
	  p->Ref = 0;
	  delete p->Wav;
	  p->Wav = NULL;
	}
    }
  Emm.Release();
  Current = NULL;
}





int FX::Find (int ref)
{
  HAN * p, * q;
  int i = 0;
  for (p = Cache, q = p+Size; p < q; p ++)
    {
      if (p->Ref == ref) break;
      else ++ i;
    }
  return i;
}











void FX::Preload (int ref0)
{
  HAN * CacheLim = Cache + Size;
  int ref;

  for (ref = ref0; ref < ref0+10; ref ++)
    {
      static char fname[] = "FX00000.WAV";
      wtom(ref, fname+2, 10, 5);
      DATACK * wav = LoadWave(&INI_FILE(fname), &Emm);
      if (wav)
	{
	  HAN * p = &Cache[Find(0)];
	  if (p >= CacheLim) break;
	  p->Wav = wav;
	  p->Ref = ref;
	}
    }
}





DATACK * FX::Load (int idx, int ref)
{
  static char fname[] = "FX00000.WAV";
  wtom(ref, fname+2, 10, 5);

  DATACK * wav = LoadWave(&INI_FILE(fname), &Emm);
  if (wav)
    {
      HAN * p = &Cache[idx];
      p->Wav = wav;
      p->Ref = ref;
    }
  return wav;
}




DATACK * FX::operator [] (int ref)
{
  int i;
  if ((i = Find(ref)) < Size) Current = Cache[i].Wav;
  else
    {
      if ((i = Find(0)) >= Size)
	{
	  Clear();
	  i = 0;
	}
      Current = Load(i, ref);
    }
  return Current;
}




//-------------------------------------------------------------------------


static	byte far *	midi	= NULL;



void KillMIDI (void)
{
  SNDMIDIStop();
  if (midi)
    {
      delete[] midi;
      midi = NULL;
    }
}





void LoadMIDI (int ref)
{
  static char fn[] = "00.MID";
  wtom(ref, fn, 10, 2);
  if (INI_FILE::Exist(fn))
    {
      KillMIDI();
      INI_FILE mid = fn;
      if (mid.Error == 0)
	{
	  word siz = (word) mid.Size();
	  midi = new far byte[siz];
	  if (midi)
	    {
	      mid.Read(midi, siz);
	      if (mid.Error) KillMIDI();
	      else
		{
		  SNDMIDIStart(midi);
		}
	    }
	}
    }
}






EC void far * Patch (int pat)
{
  void far * p = NULL;
  static char fn[] = "PATCH000.SND";

  wtom(pat, fn+5, 10, 3);
  INI_FILE snd = fn;
  if (! snd.Error)
    {
      word siz = (word) snd.Size();
      p = (byte far *) farmalloc(siz);
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

