#include	<wind.h>


extern volatile Boolean MouseCursorWiped;




MouseFunType StdMouseFun
{
  ProtectSuppress();
  if (MouseCursorWiped)
    {
      MouseCursor(ON);
      MouseCursorWiped = FALSE;
    }
}

