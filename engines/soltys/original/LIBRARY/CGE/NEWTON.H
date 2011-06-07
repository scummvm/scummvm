class NEWTON : public SPRITE
{
public:
  int Ax, Ay;
  int Bx, By;
  int Cx, Cy;
  int Dx, Dy;
  int Tx, Ty;
  NEWTON (BITMAP ** shpl) : SPRITE(shpl),
    Ax(0), Bx(0), Cx(0), Tx(0),
    Ay(0), By(0), Cy(0), Ty(0)   { }
  void Tick (void);
};
