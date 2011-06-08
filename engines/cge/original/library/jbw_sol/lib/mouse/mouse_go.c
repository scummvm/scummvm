#include	<wind.h>
#include	<dos.h>



void MouseGoto (int x, int y)
{
  switch (* (byte far *) 0x449L)
    {
      case 0x50:
      case 0x54: x *= 8; y *= 16; break;
      case 0x58: x *= 9; y *= 16; break;
      case 0x53: x *= 8; y *= 14; break;
      case 0x57: x *= 9; y *= 14; break;
      case 0x51:
      case 0x55: x *= 8; y *= 11; break;
      case 0x59: x *= 9; y *= 11; break;
      case 0x52:
      case 0x56: x *= 8; y *= 8; break;
      case 0x5A: x *= 9; y *= 8; break;
      case 0 :
      case 1 : x *= 2;
      case 2 :
      case 3 :
      case 7 :
      default: x *= 8; y *= 8; break;
    }
  _CX = x; _DX = y; _AX = 0x0004; __int__(0x33);
}
