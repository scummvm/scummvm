void SYSTEM::Tick (void)
{
  static DAC pal[256];
  Keys k = InKey();
  if (k)
    {
      switch (k)
	{
	  #ifdef  DESIGN
	  //case Left  : Hero->Turn(WALK::WW); break;
	  //case Right : Hero->Turn(WALK::EE); break;
	  //case Up    : Hero->Turn(WALK::NN); break;
	  //case Down  : Hero->Turn(WALK::SS); break;
	  case ' '   : SwitchColorMode(); break;
	  case '`'   : SwitchMapping(); break;
	  #endif
	  Debug ( case AltX : Mode = 77; break; )
	  case Esc   :
	  case F10   : SwitchCave(-1);
	}
    }
  #ifdef DEMO
  if (ExitBtn->Clicked()) Mode = -1;
  if (MonoBtn->Clicked()) SwitchColorMode();
  #endif
  switch (Mode)
    {
      case  1 : lum = 0; Delay = 0; ++ Mode; // ...
      case  2 : VGA::SetColors(Pal, lum += 2);
		if (lum == 64)
		  {
		    Mode = 0;
		  }
		break;
      case -1 : VGA::GetColors(pal); lum = 64; Delay = 0; -- Mode; // ...
      case -2 : VGA::SetColors(pal, lum -= 2);
		if (lum == 0)
		  {
		    if (Now < 0) Mode = 77;
		    else
		      {
			Mode = 0;
			//POST(SNEXEC, 0, 0, CaveDown);
			//POST(SNEXEC, 0, 0, CaveUp);
		      }
		  }
		break;
    }
  if (Sound) if (-- Sound == 0) nosound();
}
