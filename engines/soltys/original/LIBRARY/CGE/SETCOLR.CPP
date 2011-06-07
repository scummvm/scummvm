//--------------------------------------------------------------------------

#ifdef DEBUG
static void ColorTest (void)
{
  static char t[] = "R=... G=... B=...";

  itom(SysPal[255].R, t+ 2, 10, 3);
  itom(SysPal[255].G, t+ 8, 10, 3);
  itom(SysPal[255].B, t+14, 10, 3);
  InfoLine.Update(t);
}
#endif
	  #ifdef DEBUG
		 case 'R'   : SysPal[255].R += (KEYBOARD::Key[LSHIFT]) ? 1 : -1;
			      VGA::SetColors(SysPal, 64); break;
		 case 'G'   : SysPal[255].G += (KEYBOARD::Key[LSHIFT]) ? 1 : -1;
			      VGA::SetColors(SysPal, 64); break;
		 case 'B'   : SysPal[255].B += (KEYBOARD::Key[LSHIFT]) ? 1 : -1;
			      VGA::SetColors(SysPal, 64); break;
		 case Enter : SNPOST(SNTEST, -1, -1, (KEYBOARD::Key[CTRL]) ? VGA::SpareQ.First()
									   : VGA::ShowQ.First()); break;
	  #endif

