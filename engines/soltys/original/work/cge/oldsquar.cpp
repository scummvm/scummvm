class SQUARE : public TALK
{
public:
  SQUARE (word w, word h);
  void Touch (word mask, int x, int y);
};






SQUARE::SQUARE (word w, word h)
{
  byte far * p = farnew(byte, w * h);
  _fmemset(p, 0, w * h);
  TS[0] = new BITMAP(w, h, p);
  Flags.Kill = TRUE;
  Flags.BDel = TRUE;
  SetShapeList(TS);
}
