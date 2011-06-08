#include	"game.h"
#include	"mouse.h"
#include	<stdlib.h>
#include	<dos.h>





byte * Glass (DAC far * pal, byte r, byte g, byte b)
{
  byte * x = new byte[256];
  if (x)
    {
      word i;
      for (i = 0; i < 256; i ++)
	{
	  x[i] = Closest(pal, MkDAC(((word)(pal[i].R) * r) / 255,
				    ((word)(pal[i].G) * g) / 255,
				    ((word)(pal[i].B) * b) / 255));
	}
    }
  return x;
}





byte * Mark (DAC far * pal)
{
  #define f(c) (c ^ 63)
  byte * x = new byte[256];
  if (x)
    {
      word i;
      for (i = 0; i < 256; i ++)
	{
	  x[i] = Closest(pal, MkDAC(f(pal[i].R),
				    f(pal[i].G),
				    f(pal[i].B))  );
	}
    }
  return x;
  #undef f
}





//--------------------------------------------------------------------------



int	FLY::L = 20,
	FLY::T = 40,
	FLY::R = 110,
	FLY::B = 100;



FLY::FLY (BITMAP ** shpl)
: SPRITE(shpl), Tx(0), Ty(0)
{
  Step(random(2));
  Goto(L+random(R-L-W), T+random(B-T-H));
}




void FLY::Tick (void)
{
  Step();
  if (! Flags.Kept)
    {
      if (random(10) < 1)
	{
	  Tx = random(3) - 1;
	  Ty = random(3) - 1;
	}
      if (X + Tx < L || X + Tx + W > R) Tx = -Tx;
      if (Y + Ty < T || Y + Ty + H > B) Ty = -Ty;
      Goto(X + Tx, Y + Ty);
    }
}


//--------------------------------------------------------------------------

