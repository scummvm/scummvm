#ifndef		__LZ__
#define		__LZ__

#include	<jbw.h>
#include	<stddef.h>
#include	<general.h>


#ifndef		IOBUF_SIZ
  #define	IOBUF_SIZ	K(4)
#endif

#ifndef	DATA_LEN
  #define	DATA_LEN	8
#endif

#ifndef	MIN_LEN
  #define	MIN_LEN		(DATA_LEN+1)
#endif

#ifndef	MAX_LEN
  #define	MAX_LEN		12
#endif

#define		TAB_SIZ		(1<<MAX_LEN)
#define		NOTHING		0xFFFF



class LZW
{
  struct entry { word pfx, sfx; entry * nxt; } Tab[TAB_SIZ];
  entry * end[1<<DATA_LEN];
  word Avail;
  word CodeLen;
  word gbuf, grem, pbuf, prem;
  word gptr, glim, pptr;

  word _fastcall GetCode (void);
  Boolean _fastcall PutCode (word code);
  word _fastcall GetByte (void);
  Boolean _fastcall PutByte (word b);
  word _fastcall Find (word pfx, word sfx);
  Boolean _fastcall Store (word pfx, word sfx);
  void (* Echo)(void);

  protected:
  XFILE * Input, * Output;
  void Clear (word org = 0);

  public:
  //LZW (void);
  LZW (XFILE * input, XFILE * output, void (* echo)(void) = NULL);
  //~LZW (void);
  Boolean Pack   (void);
  Boolean Unpack (void);
};


#endif