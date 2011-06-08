#ifndef		__MIXER__
#define		__MIXER__

#include	"vga13h.h"

#define		MIX_MAX		16	// count of Leds
#define		MIX_Z		64	// mixer Z position
#define		MIX_DELAY	12 	// 6/s
#define		MIX_FALL	6 	// in MIX_DELAY units
#define		MIX_BHIG	6	// mixer button high
#define		MIX_NAME	105	// sprite name

class MIXER : public SPRITE
{
  BMP_PTR mb[2];
  BMP_PTR lb[MIX_MAX+1];
  SEQ ls[MIX_MAX];
  SPRITE * Led[2];
  int Fall;
  void Update (void);
public:
  static Boolean Appear;
  MIXER (int x, int y);
  ~MIXER (void);
  void Touch (word mask, int x, int y);
  void Tick (void);
};



#endif
