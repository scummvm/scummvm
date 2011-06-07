#include	<wind.h>
#include	<string.h>


#define		WHITE_CHAR	219
#define		GRAY_CHAR	177


static	int	ProgressBarLen, ProgressIncr;
static	long	ProgressCntr, ProgressUnit;
static	Wind	*ProgressWind = NULL;


static word ShowProgressChar (void)
{
  asm	cmp	dx,[bx].(Wind)Y
  asm	jne	go_std
  asm	mov	ax,cx
  asm	sub	ax,[bx].(Wind)X
  asm	jc	go_std
  asm	cmp	ax,ProgressBarLen
  asm	jl	showpro
  go_std:
  return ShowWindChar();

  showpro:
  asm	cmp	ax,[bx].(Wind)Hp
  asm	mov	ah,[bx].Color+FLD_N
  asm	mov	al,WHITE_CHAR
  asm	jb	sp_xit
  asm	mov	al,GRAY_CHAR
  sp_xit:
  return _AX;
}




Wind * MakeProgress (int l, int t, ColorBank cb, char *msg, long total)
{
  ProgressBarLen = strlen(msg);
  if ((ProgressWind = CreateWind(0, 0, ProgressBarLen+3, 4, cb, TRUE, TRUE)) == NULL)
    return NULL;
  MoveWind(ProgressWind, l, t);
  ProgressWind->ShowProc = ShowProgressChar;
  ProgressWind->Y = 2;
  ProgressWind->X = 1;
  WriteWindText(ProgressWind, 1, 0, msg);
  ProgressUnit = total / ProgressBarLen;
  ProgressCntr = 0;
  if (total == 0) ++ total;
  ProgressIncr = (ProgressUnit) ? 1 : (ProgressBarLen / (int) total);
  ProgressWind->Hp = 0;
  return ProgressWind;
}





Boolean Progress (void)
{
  if (++ ProgressCntr >= ProgressUnit)
    {
      int x = ProgressWind->Lft + 1 + ProgressWind->X;
      int y = ProgressWind->Top + 1 + ProgressWind->Y;
      ProgressWind->Hp += ProgressIncr;
      ProgressCntr = 0;
      SetRefresh(x, y, x+ProgressBarLen, y);
      Refresh();
      return TRUE;
    }
  return FALSE;
}
