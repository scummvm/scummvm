//--------------------------------------------------------------------------



void NEWTON::Tick (void)
{
  Step();
  if (Flags.Hold)
    {
      Dx = X - Tx;
      Dy = Y - Ty;
    }
  Tx = X;
  Ty = Y;
  if (Y > Ay) Y = Ay;
  if (LBound(this) || RBound(this))
    Dx = - ((Dx * Bx) / 100);
  if (Y == Ay)
    {
      if ((Dx | Dy) == 0) return;
      Dx =   ((Dx * Bx) / 100);
      Dy = - ((Dy * By) / 100);
    }
  else ++ Dy;
  Goto(X + Dx / (Cx+1),
       Y + Dy / (Cy+1));
}





