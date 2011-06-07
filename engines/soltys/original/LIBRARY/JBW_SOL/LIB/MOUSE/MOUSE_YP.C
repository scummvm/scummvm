#include	<wind.h>
#include	<dos.h>



int MouseY (void)
{
  register int y;
  _AX = 0x0003; __int__(0x33); y = _DX;
  switch (* (byte far *) 0x449L)
    {
      case 0x50:
      case 0x54:
      case 0x58: y /= 16; break;
      case 0x53:
      case 0x57: y /= 14; break;
      case 0x51:
      case 0x55:
      case 0x59: y /= 11; break;
      case 0x52:
      case 0x56:
      case 0x5A:
      case 0 :
      case 1 :
      case 2 :
      case 3 :
      case 7 :
      default: y /= 8; break;
    }
  return y;
}
