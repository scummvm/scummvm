#include	"wav.h"
#include	<alloc.h>

#ifdef		DROP_H
  #include	"drop.h"
#endif

#ifndef		DROP
  #define	DROP(m,n)
#endif



	CKID	RIFF	= "RIFF";
	CKID	WAVE	= "WAVE";
	CKID	FMT	= "fmt ";
	CKID	DATA	= "data";



//----------------------------------------------------------------------




XFILE *		CKID::ckFile	= NULL;



FMTCK::FMTCK (CKHEA& hea)
: CKHEA(hea)
{
  XRead(ckFile, &Wav);
  switch (Wav.wFormatTag)
    {
      case WAVE_FORMAT_PCM    : XRead(ckFile, &Pcm);      break;
      default                 : DROP("Bad format", NULL); break;
    }
}





DATACK::DATACK (CKHEA& hea)
: CKHEA(hea), Buf(farnew(byte, ckSize)), e(FALSE)
{
  if (Buf)
    {
      byte huge * p = Buf, huge * q = (byte huge *)Buf+ckSize;
      long n = 0;
      for (q = (p = Buf) + ckSize; p < q; p += n)
	{
	  n = q - p;
	  if (n > 0x8000) n = 0x8000;
	  ckFile->Read((byte far *)p, (word)n);
	}
    }
  else DROP("No core", NULL);
}





DATACK::DATACK (CKHEA& hea, EMM * emm)
: CKHEA(hea), EBuf(emm->Alloc((word)ckSize)), e(TRUE)
{
  if (EBuf) ckFile->Read((byte far *)&*EBuf, EBuf->Size());
}





DATACK::DATACK (int first, int last)
: CKHEA("data"), Buf(farnew(byte, ckSize = ((first > last) ? (first-last) : (last-first))+1))
{
  if (Buf)
    {
      int i, n = (first > last) ? -1 : 1;
      byte huge * p = Buf;

      for (i = first; i != last; i += n) * (p ++) = i;
      *p = i;
    }
  else DROP("No core", NULL);
}





DATACK::~DATACK (void)
{
  if (!e && Buf) farfree(Buf);
}




//------------------------------------------------------------------------





const char * CKID::Name (void)
{
  static char n[sizeof(Tx)+1];
  memcpy(n, Tx, sizeof(Tx));
  n[sizeof(Tx)] = '\0';
  return n;
}






void CKHEA::Skip (void)
{
  long n = ckFile->Mark() + ckSize, z = ckFile->Size();
  ckFile->Seek(min(n, z));
}




//------------------------------------------------------------------------






DATACK * LoadWave (XFILE * file, EMM * emm)
{
  DATACK * data = NULL;
  if (file)
    {
      if (file->Error == 0)
	{
	  CKHEA hea(file);
	  if (hea == RIFF)
	    {
	      CKID ftype(file);
	      if (ftype == WAVE)
		{
		  do
		    {
		      CKHEA wav_ck(file);
		      if (wav_ck == FMT)
			{
			  FMTCK fmt = wav_ck;
			  if (fmt.Channels() != 1                 ||
			      (fmt.SmplRate()/1000)*1000 != 11000 ||
			      (fmt.ByteRate()/1000)*1000 != 11000 ||
			      fmt.BlckSize() != 1                 ||
			      fmt.SmplSize() != 8) DROP("Unknown format", NULL);
			}
		      else if (wav_ck == DATA)
			{
			  if (emm)
			    {
			      data = new DATACK(wav_ck, emm);
			      if (data->EAddr() == NULL)
				{
				  delete data;
				  data = NULL;
				  break;
				}
			    }
			  else
			    {
			      data = new DATACK(wav_ck);
			    }
			}
		      else
			{
			  wav_ck.Skip();
			}
		    }
		  while (file->Mark() != file->Size());
		}
	      else DROP("Bad file type", NULL);
	    }
	  else DROP("Bad file format", NULL);
	}
    }
  return data;
}

