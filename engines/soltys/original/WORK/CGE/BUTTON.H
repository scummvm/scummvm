class BUTTON : public SPRITE
{
  Boolean Dwn;
  Boolean Clk;
protected:
  BITMAP * BB[3];
public:
  Boolean Clicked (void);
  BUTTON (const char * name0, const char * name1, int x, int y);
  void Touch (word mask, int x, int y);
};
