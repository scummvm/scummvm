#include	"vga13h.h"
#include	<alloc.h>
#include	<conio.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dos.h>
#include	<dir.h>
#include	<fcntl.h>
#include	<bios.h>
#include	<io.h>


#define		USER_MAX	100
#define		F_MAX		100

#define		PCOL		8
#define		PROW		5
#define		PCNT		(PCOL*PROW)
#define		SNAP		5
#define		PWID		(SCR_WID/PCOL)
#define		PHIG		(SCR_HIG/PROW)
#define		XSNP		(PWID/SNAP)
#define		YSNP		(PHIG/SNAP)

#define		newSHAPE(tab)	(new SHAPE(tab, ArrayCount(tab)))

static	fcnt = 0;


static	VGA	vga;
static	SPRITE	* Scene = NULL;
static	int	Steps = 0;



//--------------------------------------------------------------------------










#ifdef	PUZZLE



static byte RareColor (byte * bmp, word siz)
{
  word i, n, cnt[256];
  for (i = 0; i < siz; i ++) ++ cnt[bmp[i]];
  n = 0;
  for (i = 1; i < ArrayCount(cnt); i ++) if (cnt[i] < cnt[n]) n = i;
  return n;
}






static void Puzzle (const char * fn)
{
  SHP * tab[PROW * PCOL];
  word n = 0, w = PWID, h = PHIG;
  int i, j, k, x = 0, y = 0;
  BMP * bmp = new BMP(fn);
  DAC * pal = bmp->TakePal();
  SHAPE * girl;
  SPRITE * spt[PCNT], /** help,*/ * hold = NULL, * sp;

  for (i = 0; i < PROW; i ++)
    for (j = 0; j < PCOL; j ++)
      tab[n ++] = new SHP(bmp, w * j, h * i, w, h);

  vga.Clear(RareColor(bmp->Map(), bmp->Hig() * bmp->Wid()));
  delete bmp;

  girl = new SHAPE(tab, n);

  srand((word) Timer());


  n = 0;
  for (i = 0; i < PROW; i ++)
    for (j = 0; j < PCOL; j ++)
      {
	SPRITE * s = new SPRITE(girl, j * PWID, i * PHIG);
	s->drag = TRUE;
	s->phase = n;
	spt[n ++] = s;
	s->Append(Scene);
	s->Show();
      }
  vga.Update();
  vga.Sunrise(pal);
  n = 0;

  while (1)
    {
      int MX = vga.mouse.X(), MY = vga.mouse.Y();

      for (sp = Scene; sp != NULL; sp = sp->next)
	{
	  sp->Show();
	}

      vga.mouse.Cursor->Goto(MX, MY);

      if (vga.mouse.Left())
	{
	  if (n == 0)
	    {
	      while (n < 3 * PCNT)
		{
		  word p = random(PCNT), q = random(PCNT), z;
		  if (p == q) q = (q + 1) % PCNT;
		  z = spt[p]->X;
		  spt[p]->X = spt[q]->X;
		  spt[q]->X = z;
		  z = spt[p]->Y;
		  spt[p]->Y = spt[q]->Y;
		  spt[q]->Y = z;

		  //spt[p]->Show();
		  //spt[q]->Show();
		  for (sp = Scene; sp != NULL; sp = sp->next)
		    {
		      sp->Show();
		    }
		  vga.Update();
		  ++ n;
		}
	    }
	  else
	    {
	      if (hold && hold->drag)
		{
		  int nx = MX - x, ny = MY - y;
		  i = nx % PWID;
		  if (i <= XSNP) nx -= i;
		  if (i >= PWID-XSNP) nx += PWID - i;
		  i = ny % PHIG;
		  if (i <= YSNP) ny -= i;
		  if (i >= PHIG-YSNP) ny += PHIG - i;
		  hold->Goto(nx, ny);
		}
	      else
		{
		  hold = SpriteAt(MX, MY);
		  if (hold->next)
		    for (sp = Scene; sp != NULL && sp->next != NULL; sp = sp->next)
		      if (sp->next == hold)
			{
			  sp->next = hold->next;
			  hold->next = NULL;
			  while (sp->next) sp = sp->next;
			  sp->next = hold;
			  break;
			}
		  x = MX - hold->X;
		  y = MY - hold->Y;
		  if (hold->LeftClick) hold->LeftClick();
		}
	    }
	}
      else
	{
	  hold = NULL;
	}

      /* if (! hold) */ vga.mouse.Cursor->Show();
      vga.Update();
      if (vga.mouse.Right()) break;
      while ((k = InKey()) != 0)
	{
	  switch (k)
	    {
	      case F10 : exit(0); break;
	      case Esc : goto kill;
	    }
	}
    }

  kill:
  vga.Sunset();
  while (Scene)
    {
      SPRITE * s = Scene;
      Scene = Scene->next;
      delete s;
    }
  for (i = 0; i < PCNT; i ++) delete tab[i];
  delete[] pal;

  while (InKey());
}







#pragma argsused
int main (int argc, char **argv)
{
  #ifdef PUZZLE
  for (Steps = 0; Steps < argc-1; Steps ++) Puzzle(argv[Steps+1]);
  #else
//  if (argc > 1)
//    {
//      BMP * b = new BMP(argv[1]);
//      Pal = b->TakePal();
//      delete b;
//    }
//  for (Steps = 0; Steps < argc-1 && Steps < USER_MAX; Steps ++)
//    {
//      User[Steps] = new SHP(argv[Steps+1]);
//    }
  BMP * b = new BMP("sf0");
  Pal = b->TakePal();
  delete b;
  Steps = ArrayCount(User);
  if (argc > 1) fcnt = atoi(argv[1]);
  if (fcnt > F_MAX) fcnt = F_MAX;
  if (Steps) Anim();
  #endif
  return 0;
}
