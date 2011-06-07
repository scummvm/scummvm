#ifndef	__SOUND__
#define	__SOUND__

#include	<wav.h>
#include	<snddrv.h>


#define		BAD_SND_TEXT	97
#define		BAD_MIDI_TEXT	98




class SOUND
{
public:
  SMPINFO smpinf;
  SOUND (void);
  ~SOUND (void);
  void Open (void);
  void Close (void);
  void Play (DATACK * wav, int pan, int cnt = 1);
  void Stop (void);
};





class FX
{
  EMM Emm;
  struct HAN { int Ref; DATACK * Wav; } * Cache;
  int Size;
  DATACK * Load (int idx, int ref);
  int Find (int ref);
public:
  DATACK * Current;
  FX (int size = 16);
  ~FX (void);
  void Clear (void);
  void Preload (int ref0);
  DATACK * operator[] (int ref);
};






extern	Boolean	Music;
extern	SOUND	Sound;
extern	FX	Fx;


void		LoadMIDI	(int ref);
void		KillMIDI	(void);


#endif

