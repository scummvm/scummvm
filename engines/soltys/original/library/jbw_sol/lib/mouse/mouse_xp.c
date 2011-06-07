#include	<wind.h>
#include	<dos.h>



int MouseX (void)
{
  register int x;
  _AX = 0x0003; __int__(0x33); x = _CX;
  switch (* (byte far *) 0x449L)
    {
      case 0x57:
      case 0x59:
      case 0x58:
      case 0x5A: x /= 9; break;
      case 0 :
      case 1 : x /= 2;
      case 2 :
      case 3 :
      case 7 :
      case 0x50:
      case 0x51:
      case 0x52:
      case 0x53:
      case 0x54:
      case 0x55:
      case 0x56:
      default: x /= 8;
    }
  return x;
}
