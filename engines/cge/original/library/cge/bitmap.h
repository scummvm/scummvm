#ifndef		__BITMAP__
#define		__BITMAP__

#include	<general.h>

#define		EOI		0x0000
#define		SKP		0x4000
#define		REP		0x8000
#define		CPY		0xC000

#define		TRANS		0xFE


typedef	struct	{ word b : 2;
		  word B : 6;
		  word g : 2;
		  word G : 6;
		  word r : 2;
		  word R : 6;
		  word Z : 8;
		} BGR4;


typedef	struct	{ word skip; word hide; } HideDesc;




class BITMAP
{
  Boolean BMPLoad (XFILE * f);
  Boolean VBMLoad (XFILE * f);
public:
  static DAC far * Pal;
  word W, H;
  byte far * M, far * V; HideDesc far * B;
  BITMAP (const char * fname, Boolean rem = TRUE);
  BITMAP (word w, word h, byte far * map);
  BITMAP (word w, word h, byte fill);
  BITMAP (const BITMAP& bmp);
  ~BITMAP (void);
  BITMAP * FlipH (void);
  BITMAP * Code ();
  BITMAP& operator = (const BITMAP& bmp);
  void Hide (int x, int y);
  void Show (int x, int y);
  void XShow (int x, int y);
  Boolean SolidAt (int x, int y);
  Boolean VBMSave (XFILE * f);
  word MoveVmap (byte far * buf);
};



typedef	BITMAP *	BMP_PTR;




#endif