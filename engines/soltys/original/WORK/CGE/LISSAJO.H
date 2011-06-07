class LISSAJOUS : public SPRITE
{
public:
  int Ax, Ay;
  int Bx, By;
  int Cx, Cy;
  int Dx;
  LISSAJOUS (BITMAP ** shpl) : SPRITE (shpl),
    Ax(0), Bx(0), Cx(0),
    Ay(0), By(0), Cy(0)  { }
  void Tick (void);
};
