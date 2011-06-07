#include	<wind.h>
#include	<errno.h>
#include	<values.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<dos.h>
#include	<bios.h>

#define		BIOS_KEY	0x16
#define		CHECK_KEY	1
#define		TAKE_KEY	0
#define		DosFunInt	0x21
//#define		GetChFun	0x08

extern	int		MaxScrHig = 25, MaxScrWid = 80;
extern	int		ZoomTop = 0, ZoomBot = 24;
extern	int		ProtectDelay = 180;
extern	int		ProtectSpeed = 10;
extern	long		ProtectTime = -1L;
extern	Keys		LastKey	= NoKey;
extern	Keys		NextKey	= NoKey;
extern	Keys *		KeyPipe = NULL;
extern	int		MX = 0, MY = 0;
extern	word		MouseDelay = 5;
extern	word far * 	Screen = (void far *) NULL;
extern	Boolean		Mono = -1;
extern	Boolean		Critic = FALSE;
extern	char		FrDes[] = "ÚÄ¿"
				  "³ ³"
				  "ÀÄÙ"
				  "ÉÍ»"
				  "º º"
				  "ÈÍ¼" ;


static	int	L = MAXINT, T = MAXINT, R = -1, B = -1;
extern	Wind *	WindStack = NULL;

extern	void    (*IdleProc)	(void);
extern	void	(*HelpProc)	(void);
extern	Keys	(*KeyProc)	(Keys);
extern	Wind *	(*SaveProc)	(void);
extern	int	(*UpperProc)	(int);

static	Wind *	ProtectWind = NULL;



void Protect (void)
{
  ProtectTime = TimerLimit(0);
}






void ProtectSuppress (void)
{
  ProtectTime = TimerLimitS(ProtectDelay);
}




static void ProtectKill (void)
{
  CloseWind(ProtectWind);
  ProtectWind = NULL;
  while (MousePressed(3));
  ClearKeyboard();
  MouseCursor(TRUE);
  ProtectSuppress();
}





void Idle (void)
{
  Wind * w;

  //---- check timers
  for (w = WindStack; w != NULL; w = w->Next)
    if (w->Time != -1 && TimerLimitGone(w->Time))
      {
	w->Flags.Repaint = TRUE;
      }

  //---- perform user job
  if (IdleProc != NULL) IdleProc();

  //---- repaint tagged windows
  w = WindStack;
  while (w != NULL)
    {
      Wind * nw = w->Next;
      if (w->Flags.Repaint)
	{
	  w->Flags.Repaint = FALSE;
	  if (w->Delay) w->Time = TimerLimit(w->Delay);
	  w->ReptProc(w);
	}
      w = nw;
    }

  Refresh();
  if (KeyPipe != NULL && *KeyPipe != NoKey && NextKey == NoKey)
    {
      NextKey = *(KeyPipe ++);
    }
}





Keys GetKey (void)
{
  static Boolean Released = TRUE;
  static long T = -1L;
  Keys k;
  int b;

  ProtectSuppress();
  do
    {
      b = 0;
      while (! NextKey && ! bioskey(1))
	{
	  if (Mouse)
	    {
	      if ((b |= MousePressed(3)) != 0)
		{
		  if (TimerLimitGone(T)) break;
		}
	      else Released = TRUE;
	      MX = MouseX(); MY = MouseY();
	    }
	  if (ProtectDelay)
	    {
	      if (TimerLimitGone(ProtectTime))
		{
		  if (SaveProc != NULL && ProtectWind == NULL)
		    {
		      ProtectWind = SaveProc();
		      MouseCursor(FALSE);
		    }
		}
	      else
		{
		  if (ProtectWind != NULL) ProtectKill();
		}
	    }
	  Idle();
	}
      ProtectSuppress();
      if (ProtectWind != NULL)
	{
	  ProtectKill();
	  continue;
	}

      if (Mouse) { MX = MouseX(); MY = MouseY(); }
      if (NextKey) { k = NextKey; NextKey = NoKey; }
      else
	{
	  if (b)
	    {
	      k = 512 + b;
	      if (Released)
		{
		  Boolean twice = FALSE;
		  T = TimerLimit(MouseDelay);
		  while (! TimerLimitGone(T))
		    {
		      if (MousePressed(b))
			{
			  if (twice)
			    {
			      k += 256;
			      while (MousePressed(b));
			      break;
			    }
			}
		      else twice = TRUE;
		    }
		}
	      Released = FALSE;
	      T = TimerLimit(1);
	    }
	  else
	    {
	      bioskey(0);
	      asm  or   al,al	// ASCII?
	      asm  jnz  msb
	      asm  xchg al,ah
	      msb:
	      asm  mov  ah,0
	      asm  jnz  sto
	      asm  inc  ah
	      sto:
	      k = _AX;
	      Released = TRUE;
	    }
	}
      if (KeyProc != NULL) k = KeyProc(k);
    }
  while (k == NoKey);
  return (LastKey = k);
}





int AbsX (Wind *W, int x)
{
  return ((W == NULL || W->Flags.Zoomed) ? x : (x + W->Lft))
       + W->Flags.Frame;
}





int AbsY (Wind *W, int y)
{
  return y
       + ((W == NULL || W->Flags.Zoomed) ? ZoomTop : W->Top)
       + W->Flags.Frame;
}






int WindHit (void)
{
  int x = _CX, y = _DX;

  _CX = MaxScrWid-1;
  _DX = ZoomBot-ZoomTop;

  asm mov  AX,[BX].Flags
  asm test AX,FrameMask
  asm jnz  check_zoom
  asm mov  AX,4
  asm jz   WH4  // jmp
  check_zoom:
  asm test AX,ZoomedMask
  asm jnz  check_WH
  asm mov  CX,[BX].Rgt
  asm sub  CX,[BX].Lft
  asm mov  DX,[BX].Bot
  asm sub  DX,[BX].Top
  check_WH:
  asm xor  AX,AX
  asm cmp  x,0
  asm je   WH1
  asm inc  AX
  WH1:
  asm cmp  y,0
  asm je   WH2
  asm add  AX,3
  WH2:
  asm cmp  x,CX
  asm jl   WH3
  asm inc  AX
  WH3:
  asm cmp  y,DX
  asm jl   WH4
  asm add  AX,3
  WH4:
  asm mov  CX,x
  asm mov  DX,y
  asm cmp  AX,4
  return _AX;
}





word ShowFrameChar (void)
{
  asm lea SI,DS:FrDes
  asm cmp AX,1
  asm jne framch
  asm cmp CX,[BX].Title.X2
  asm jg  framch
  asm mov DX,[BX].Title.X1
  asm cmp CX,DX
  asm jl  framch

  // title
  title:

  asm mov AH,[BX].Color+FRM_H
  asm mov SI,[BX].Title.Text
  asm add SI,CX
  asm sub SI,DX
  asm mov AL,[SI]
  asm cmp BX,WindStack
  asm jne titnor
  asm inc BX		// next color = bold
  titnor:
  asm mov AH,[bx].Color+FRM_N
  return _AX;

  // standard frame char
  framch:
  asm add SI,AX

  //
  asm cmp BX,WindStack
  asm jne norm

  // front
  asm mov AH,[bx].Color+FRM_H
  asm add SI,9
  asm mov AL,[SI]
  return _AX;

  // normal
  norm:
  asm mov AH,[bx].Color+FRM_N
  asm mov AL,[SI]
  return _AX;
}




void SetRefresh (int x, int y, int r, int b)
{
  x = max(x, 0); L = min(x, L);
  y = max(y, 0); T = min(y, T);
  r = min(r, MaxScrWid-1); R = max(r, R);
  b = min(b, MaxScrHig-1); B = max(b, B);
}






void ResetRefresh (void)
{
  L = MAXINT, T = MAXINT, R = -1, B = -1;
}






void Refresh (void)
{
  int X; word sa;
  int mx = MouseX(), my = MouseY();
  Boolean M = (Mouse && mx >= L && mx <= R && my >= T && my <= B);
  //Wind *W;

  if (M) MouseCursor(FALSE);
  for (; T <= B; T ++)
    {
      asm mov AX,MaxScrWid
      asm mov AH,byte ptr T
      asm mul AH
      asm add AX,L
      asm shl AX,1
      asm mov sa,AX
      for (X = L; X <= R; X ++)
	{
	  asm mov BX,WindStack

	  chk0:
	  asm or   bx,bx
	  asm je   nowin
	  asm mov  ax,[bx].Flags

	  asm test ax,AppearMask
	  asm jz   next

	  asm mov  CX,ZoomTop
	  asm mov  DX,ZoomBot
	  asm test ax,ZoomedMask
	  asm jnz  chkY
	  asm mov  cx,[bx].Top
	  asm mov  dx,[bx].Bot
	  asm mov  ax,X
	  asm cmp  ax,[bx].Lft
	  asm jl   next
	  asm cmp  ax,[bx].Rgt
	  asm jg   next

	  chkY:
	  asm mov  ax,T
	  asm cmp  ax,cx
	  asm jl   next
	  asm cmp  ax,dx
	  asm jng  inside

	  next:
	  asm mov  bx,[bx].Next
	  asm jmp  chk0



	  inside:
	  asm mov CX,X
	  asm mov DX,T
	  asm mov  ax,[BX].Flags
	  asm test AX,FrameMask
	  asm pushf
	  asm test AX,ZoomedMask
	  asm mov  AX,ZoomTop
	  asm jne  rel
	  asm mov  AX,[BX].Top
	  asm sub  CX,[BX].Lft

	  rel:
	  asm sub  DX,AX

	  // Frame?
	  asm popf
	  asm jz   call_SP

	  //_AX = (word) WindHit;
	  //asm call AX
	  WindHit();
	  asm jz   decXY

	  //_DX = (word) ShowFrameChar;
	  //asm call DX
	  ShowFrameChar();
	  asm jmp  short display

	  // Skip frames
	  decXY:
	  asm dec  CX
	  asm dec  DX

	  // Invoke user display routine
	  call_SP:
	  asm mov  AX,[BX].Wid
	  asm mul DL
	  asm add AX,CX
	  asm mov SI,AX

	  asm mov  AX,[BX].ShowProc
	  asm call AX

	  display:
	  _ES = FP_SEG(Screen);
	  asm mov BX,sa
	  asm mov ES:[BX],AX
	  asm inc BX
	  asm inc BX
	  asm mov sa,BX

	  nowin:
	}
    }
  if (T > B) ResetRefresh();
  if (M) MouseCursor(TRUE);

  SetXY(AbsX(WindStack, WindStack->Cursor.X), AbsY(WindStack, WindStack->Cursor.Y));
  SetCursor((WindStack->Flags.Appear) ? WindStack->Cursor.Shape : OFF_Cursor);
}






void CenterTitle (Wind *w)
{
  if (w->Title.Text != NULL)
    {
      int i = strlen(w->Title.Text), ww = WindBodyWid(w);
      SetRefreshTitle(w);
      w->Title.X1 = (i > ww) ? 1 : ((ww - i) / 2 + 1);
      w->Title.X2 = w->Title.X1 + i - 1;
      SetRefreshTitle(w);
    }
}





void SetRefreshWind (Wind *W)
{
  memcpy(W->Color, Colors[Mono][W->ColBank], ColorRegs);
  if (W->Flags.TCenter) CenterTitle(W);
  if (W->Flags.Zoomed) SetRefresh(0, ZoomTop, MaxScrWid-1, ZoomBot);
  else SetRefresh(W->Lft, W->Top, W->Rgt, W->Bot);
}






void SetRefreshTitle (Wind *W)
{
  if (W->Flags.Frame && W->Title.Text != NULL && W->Title.X1 <= W->Title.X2)
    {
      int top = (W->Flags.Zoomed) ? ZoomTop : W->Top,
	  lft = (W->Flags.Zoomed) ? 0 : W->Lft;
      SetRefresh(lft+W->Title.X1, top, lft+W->Title.X2, top);
    }
}





void ShowWind (Wind *W)
{
  W->Flags.Appear = TRUE;
  W->Flags.Repaint = TRUE;
  SetRefreshWind(W);
}





word ShowWindChar (void)
{
  asm mov AH,[BX].Color+FLD_N

  asm cmp CL,[BX].Wid
  asm jae spac
  asm cmp DL,[BX].Hig
  asm jae spac

  asm add SI,[BX].Body.Near
  asm mov AL,[SI]

  return _AX;

  spac:
  asm mov AL,' '
  return _AX;
}




Wind * CreateWind (int l, int t, int r, int b, ColorBank c, Boolean room, Boolean frm)
{
  Wind *wi = calloc(1, sizeof(Wind));
  int i, d = (frm) ? -1 : 1;

  if (wi == NULL) return NULL;
  wi->Lft = l; wi->Top = t; wi->Rgt = r; wi->Bot = b;
  wi->Wid = r - l + d;
  wi->Hig = b - t + d;
  wi->ColBank = c;
  wi->Cursor.Shape = OFF_Cursor;
  if (room)
    {
      i = (wi->Hig) * (wi->Wid);
      if ((Image(wi) = malloc(i+1)) == NULL) { free(wi); return NULL; }
      memset(Image(wi), ' ', i);
      Image(wi)[i] = '\0';
      wi->Flags.Allocated = TRUE;
    }
  if (frm) wi->Flags.Frame = TRUE;
  wi->Next = WindStack;
  if (WindStack != NULL)
    if (WindStack->Flags.Frame)
      SetRefreshWind(WindStack);
  wi->ShowProc = ShowWindChar;
  wi->ReptProc = SetRefreshWind;
  wi->Time = -1L;
  wi->Help = -1;
  return (WindStack = wi);
}





void HideWind (Wind *W)
{
  W->Flags.Appear = FALSE;
  SetRefreshWind(W);
}





Boolean CloseWind (Wind *W)
{
  if (W->KillProc != NULL) W->KillProc(W);
  HideWind(W);
  if (W == WindStack)
    {
      WindStack = WindStack->Next;
      if (WindStack != NULL) SetRefreshWind(WindStack);
    }
  else
    {
      Wind *w = WindStack;
      while (w->Next != W) w = w->Next;
      w->Next = W->Next;
    }
  if (W->Flags.Allocated) free(Image(W));
  free(W);
  return TRUE;
}
