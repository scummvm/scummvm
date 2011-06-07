#include	<general.h>
#include	<cfile.h>
#include	<dir.h>
#include	<wav.h>
#include	<dos.h>
#include	<stdio.h>
#include	<conio.h>
#include	<stdlib.h>
#include	<alloc.h>
#include	<string.h>


#define		DROP(m, n)	{ printf("%s %s\n", m, n); exit(1); }

static	char	Signature[] = "JBW wave file converter v0.02";


struct WAV
{
  FMTCK * fmt;
  DATACK * dat;
};








static WAV * Load (XFILE * f)
{
  WAV * wav = NULL;
  if (f->Error == 0)
    {
      wav = new WAV;
      CKHEA hea(f);
      if (hea == RIFF)
	{
	  CKID ftype(f);
	  if (ftype == WAVE)
	    {
	      do
		{
		  CKHEA wav_ck(f);
		  //printf("\nChunk id: [%s], %lu bytes\n", wav_ck.Name(), wav_ck.Size());
		  if (wav_ck == FMT)
		    {
		      wav->fmt = new FMTCK(wav_ck);
		      //printf("Number of channels=%u\nSampling rate=%lu\nAverage bytes/sec=%lu\nData block size=%u\nSample size=%u\n",
		      //	  fmt.Channels(), fmt.SmplRate(), fmt.ByteRate(), fmt.BlckSize(), fmt.SmplSize());
		    }
		  else if (wav_ck == DATA)
		    {
		      wav->dat = new DATACK(wav_ck);
		      if (wav->dat->Addr() == NULL) DROP("No enough RAM", "");
		    }
		  else
		    {
		      wav_ck.Skip();
		      //printf("skipped\n");
		    }
		}
	      while (f->Mark() != f->Size());
	    }
	}
    }
  else DROP("File error", "");
  return wav;
}





static void Save (XFILE * f, WAV * wav)
{
  if (f && wav)
    {
      byte huge * p, huge * q, huge * r;
      dword x;
      word bps = wav->fmt->SmplSize() / 8,
	   slo = (((word)(wav->fmt->SmplRate())/1000)*1000) / 11000;
      dword size = wav->dat->Size();

      if (bps == 2)
	{
	  p = r = wav->dat->Addr();
	  q = p + size;
	  while (p < q)
	    {
	      * (r ++) = * (++ p);
	      ++ p;
	    }
	  size = (word) (r - wav->dat->Addr());
	}

      if (slo > 1)
	{
	  p = r = wav->dat->Addr();
	  q = p + size;
	  while (p < q)
	    {
	      word i, n = 0;
	      for (i = 0; i < slo; i ++) n += * (p ++);
	      * (r ++) = n / slo;
	    }
	  size = r - wav->dat->Addr();
	}

      f->Write("RIFF", 4);
      x = size + 36;
      f->Write((char far *)&x, 4);
      f->Write("WAVE", 4);
      f->Write("fmt ", 4);
      x = wav->fmt->Size();
      f->Write((char far *)&x, 4);
      x = 1;
      f->Write((char far *)&x, 2);
      f->Write((char far *)&x, 2);
      x = 11025;
      f->Write((char far *)&x, 4);
      f->Write((char far *)&x, 4);
      x = 1;
      f->Write((char far *)&x, 2);
      x = 8;
      f->Write((char far *)&x, 2);
      f->Write("data", 4);
      f->Write((char far *)&size, 4);
      for (q = (p = wav->dat->Addr()) + size; p < q; p += x)
	{
	  x = q - p;
	  if (x > 0x8000) x = 0x8000;
	  f->Write((byte far *)p, (word)x);
	}
    }
}




void main (int argc, char **argv)
{
  printf("\n%s\n", Signature);
  if (argc < 2)
    {
      char nam[MAXFILE];
      fnsplit(argv[0], NULL, NULL, nam, NULL);
      printf("Syntax is:    %s wav_file\n", strupr(nam));
    }
  else
    {
      char pat[MAXPATH], drv[MAXDRIVE], dir[MAXDIR];
      ffblk fb;
      int total = 0, i;

      strupr(MergeExt(pat, argv[1], ".WAV"));
      fnsplit(pat, drv, dir, NULL, NULL);

      for (i = findfirst(pat, &fb, 0); i == 0; i = findnext(&fb))
	{
	  char iname[MAXPATH], oname[MAXPATH], nam[MAXFILE], ext[MAXEXT];

	  fnsplit(fb.ff_name, NULL, NULL, nam, ext);
	  fnmerge(iname, drv, dir, nam, ext);
	  fnmerge(oname, drv, dir, nam, ".WA_");

	  if (stricmp(iname, oname) == 0)
	    {
	      printf("Can\'t update file in place: %s\n", iname);
	    }
	  else
	    {
	      printf("%-13s", iname);
	      CFILE ifile(iname);
	      WAV * wav = Load(&ifile);
	      word smpsiz = wav->fmt->SmplSize(),
		   blksiz = wav->fmt->BlckSize();
	      dword smprat = (wav->fmt->SmplRate()/1000)*1000;
	      if (wav->fmt->Channels() == 1                       &&
		 smprat == (wav->fmt->ByteRate()/1000)*1000       &&
		 ((blksiz == 1 && smpsiz == 8) || (blksiz == 2 || smpsiz == 16)))
		{
		  if (smpsiz == 8 && smprat == 11000) printf("Already good sample - skipped\n");
		  else
		    {
		      printf("Writing %s...", oname);
		      CFILE ofile(oname, CFILE::WRI);
		      Save(&ofile, wav);
		      printf(" done\n");
		    }
		}
	      else printf("Unsupported format - skipped\n");
	      delete wav->fmt;
	      delete wav->dat;
	    }
	  ++ total;
	}
      printf("Total %d file(s) processed.\n", total);
    }
}
