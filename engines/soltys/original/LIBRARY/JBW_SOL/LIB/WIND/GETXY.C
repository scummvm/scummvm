#include	<wind.h>
#include	<dos.h>


word GetXY (void)
{
  _AH = 0x0F; Video();  /* active page */
  _DH = 0;
  _AH = 0x03; Video();  /* get cursor  */
  return _DX;
}
