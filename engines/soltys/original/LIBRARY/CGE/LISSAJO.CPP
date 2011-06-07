//--------------------------------------------------------------------------




void LISSAJOUS::Tick (void)
{
  long t = ((* (long *) &Dx) ++) * 10;

  Step();
  Goto(Ax + Sinus(t / Bx) / Cx,
       Ay + Sinus(t / By) / Cy );
}



