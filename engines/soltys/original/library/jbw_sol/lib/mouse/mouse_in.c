#include	<wind.h>
#include	<dos.h>



extern	Boolean	Mouse = FALSE;
extern volatile Boolean MouseCursorWiped = TRUE;
extern	int	Buttons = 0;



Boolean MouseInit (void)
{
  // Mouse reset
  _AX = 0x0000;
  __int__(0x33);
  Mouse = (_AX == 0xFFFF);
  Buttons = _BX;

  if (Mouse)
    {
      int mh = MaxScrWid-1, mv = MaxScrHig-1;
      switch (* (byte far *) 0x449L)
        {
          case 0x50:
          case 0x54: mh *= 8; mv *= 16; break;
	  case 0x58: mh *= 9; mv *= 16; break;
          case 0x53: mh *= 8; mv *= 14; break;
          case 0x57: mh *= 9; mv *= 14; break;
          case 0x51:
          case 0x55: mh *= 8; mv *= 11; break;
          case 0x59: mh *= 9; mv *= 11; break;
	  case 0x52:
          case 0x56: mh *= 8; mv *= 8; break;
          case 0x5A: mh *= 9; mv *= 8; break;
          case 0 :
          case 1 : mh *= 2;
          case 2 :
          case 3 :
	  case 7 :
          default: mh *= 8; mv *= 8; break;
        }

      // Set X bounds
      _CX = 0;
      _DX = mh;
      _AX = 0x0007;
      __int__(0x33);

      // Set Y bounds
      _CX = 0;
      _DX = mv;
      _AX = 0x0008;
      __int__(0x33);
    }
  MouseCursorWiped = TRUE;
  return (Mouse);
}
