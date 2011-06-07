//--------------------------------------------------------------------------



BUTTON::BUTTON (const char * name0, const char * name1, int x, int y)
: SPRITE(NULL), Clk(FALSE), Dwn(FALSE)
{
  BB[0] = new BITMAP(name0, FALSE);
  BB[1] = new BITMAP(name1, FALSE);
  BB[2] = NULL;
  SetShapeList(BB);
  Goto(x, y);
}






Boolean BUTTON::Clicked (void)
{
  Boolean c = Clk;
  Clk = FALSE;
  return c;
}






#pragma argsused
void BUTTON::Touch (word mask, int x, int y)
{
  //Boolean stay = Flags.Stay;
  SPRITE::Touch(mask, x, y);
  /*
  if (stay)
    {
      if (mask & L_DN)
	{
	  Step(! SeqPtr);
	}
      if (mask & L_UP)
	{
	  if (SeqPtr && ! (mask & ATTN)) Clk = TRUE;
	}
    }
  else
  */
    {
      if (mask & L_DN) Dwn = TRUE;
      if (mask & L_UP)
	{
	  if (Dwn && ! (mask & ATTN)) Clk = TRUE;
	  Dwn = FALSE;
	}
      Step(Dwn && ! (mask & ATTN));
    }
}




