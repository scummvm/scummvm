#include	<wind.h>
#include	<dos.h>


void SetXY (int x, int y)
{
  _AH = 0x0F; Video();  /* active page */
  _DL = x; _DH = y; 
  _AH = 0x02; Video();  /* set cursor  */
}
