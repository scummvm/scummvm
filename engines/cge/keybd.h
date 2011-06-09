#ifndef		__KEYBD__
#define		__KEYBD__

#include	<jbw.h>
#include	"vga13h.h"


#define		KEYBD_INT	9
#define		LSHIFT		42
#define		RSHIFT		54
#define		CTRL		29
#define		ALT		56


class KEYBOARD
{
  static void interrupt (far * OldKeyboard) (...);
  static void interrupt NewKeyboard (...);
  static word Code[0x60];
  static word Current;
  static SPRITE * Client;
public:
  static byte Key[0x60];
  static word Last (void) { _AX = Current; Current = 0; return _AX; }
  static SPRITE * SetClient (SPRITE * spr);
  KEYBOARD (void);
  ~KEYBOARD (void);
};


#endif
