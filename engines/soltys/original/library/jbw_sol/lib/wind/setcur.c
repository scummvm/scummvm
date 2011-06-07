#include	<wind.h>
#include	<dos.h>


word SetCursor (word lines)
{
  word r;
  _AH = 0x0F; Video();  /* active page */
  _AH = 0x03; Video();  /* get cursor size */
  r = _CX; _CX = lines; 
  _AH = 0x01; Video();  /* set cursor size */
  return r;
}
