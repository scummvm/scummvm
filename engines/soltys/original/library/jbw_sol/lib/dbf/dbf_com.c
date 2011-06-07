#include	"dbf_inc.h"
#include	<alloc.h>
#include	<errno.h>


#ifdef	DEMO
extern	char	Demo[] = "*DEMO*";
#endif

	WorkArea WorkTab[Areas], *WorkPtr = WorkTab;
	Errors	dBError = Ok;
static	Boolean	Found;
	long	LockLength;

static	struct	{
		  PageBox *	Adr;
		  WorkArea *	Wrk;
		  int		Ord;
		  int		Cnt;
		} ShareTab[SHARE_MAX];

extern	int	Share = -1;

extern	Boolean	Network = OFF;
extern	int	DbfRetryTime = 10;



/******************************************************/
/*                  error handling                    */
/******************************************************/

#pragma argsused
static void StdError (Errors e, const char * n)
{
  char m[100];

  *m = '\n';
  strcpy(m+1, n);
  strcat(m, ": Fatal Error!\n");
  SetXY(0, 24); CloseAll();
  puts(m);
  exit(1);
}




extern	void	(*DbfError)(Errors, const char *) = StdError;








Boolean Error (Errors err)
{
  static Boolean ErrSeq = FALSE;

  if (! ErrSeq)
    {
      dBError = err;
      if (err)
	{
	  ErrSeq = TRUE;
	  DbfError(err, Work.dBPath);
	  ErrSeq = FALSE;
	}
    }
  return err == Ok;
}






/******************************************************/
/*                network file access                 */
/******************************************************/




extern Boolean (*Timeout)(void) = NULL;




static long N_TimerLimit (void)
{
  return TimerLimitS(DbfRetryTime);
}



static void NetUnlock (int file)
{
  unlock(file, 0L, -1L);
}




//---- take exclusive access
static Boolean NetLock (int file)
{
  long t = N_TimerLimit();

  while (lock(file, 0L, -1L) != 0)
    if (TimerLimitGone(t))
      if (Timeout == NULL) goto nl_xit;
      else
	if (Timeout()) t = N_TimerLimit();
	else
	  {
	    int e;
	    nl_xit:
	    e = errno;
	    NetUnlock(file);
	    switch(e)
	      {
		case EACCES    :
		case EDEADLOCK : Err(LockErr);
		case EINVAL    : Err(ShareErr);
		default        : Err(999);
	      }
	  }
  return TRUE;
}







/******************************************************/
/*             private index definitions              */
/******************************************************/


#define		IxExt		"IX"
#define		Spare		-1
#define		SparePage	SpareBox.Page

PageBox		SpareBox = { NoRec, FALSE };
long		CurPos = -1L;
int		CurLev;





IPP IxPkP (void)
{
  return Work.IxPk[Work.Order];
}



PageBox * IBoxP (int n)
{
  return (n < 0) ? &SpareBox : &(IxPkP()->PageBoxTab[n]);
}



void InitPageBoxTab (PageBox * bt)
{
  PageBox * z = bt + MaxLevels;
  while (bt < z)
    {
      bt->PgPos = NoRec;
      bt->Updated = FALSE;
      ++ bt;
    }
}






static char *CurKey (void)
{
  IPP ipp = IxPkP();
  return ipp->IProc(Work.DescPtr[ipp->IDesc.Field].FldPtr);
}





static Boolean WritePage (int n)
{
  IPP ipp = IxPkP();
  PageBox *P = IBoxP(n);
  if (P->PgPos == NoRec || ! P->Updated) Err(Ok);
  if (lseek(ipp->IFile, (long)P->PgPos * PageSize, SEEK_SET) == -1L)
    Err(IxSeekErr);
  if (_write(ipp->IFile, &P->Page, PageSize) == -1) Err(IxWriteErr);
  P->Updated = FALSE; P->PgPos = NoRec;
  Err(Ok);
}





Boolean IxFlush (void)
{
  int i; long l;
  IPP ipp = IxPkP();
  int f = ipp->IFile;

  if (ipp != NULL) if (ipp->IUpdt)
    {
      IxFileDesc * id = &ipp->IDesc;
      l = (long) id->New * PageSize;
      chsize(f, l);
      if (lseek(f, l, SEEK_SET) == -1L)
	Err(IxSeekErr);
      if (_write(f, id, sizeof(*id)) == -1)
	Err(IxWriteErr);
      for (i = 0; i < MaxLevels; i ++)
	if (! WritePage(i))
	  return FALSE;
      FileFlush(f);
      ipp->IUpdt = FALSE;
    }
  Err(Ok);
}





static Boolean PutPage(int n)
{
  if (Work.RdOnly) Err(IxRdOnlyErr);
  IBoxP(n)->Updated = TRUE;
  return (IxPkP()->IUpdt = TRUE);
}








static Boolean DelPage(int n)
{
  IPP ipp = IxPkP();
  PageBox *P = IBoxP(n);

  if (Work.RdOnly) Err(IxRdOnlyErr);
  P->Page.Count = -1;
  P->Page.RightPage = ipp->IDesc.Free;
  ipp->IDesc.Free = P->PgPos;
  P->Updated = TRUE;
  return (ipp->IUpdt = TRUE);
}







static void SwitchSharedIx (IPP ipp)
{
  int sha = ipp->Shared, ord;
  WorkArea * wrk;

  if (sha < 0) return;
  wrk = ShareTab[sha].Wrk;
  ord = ShareTab[sha].Ord;
  if (wrk == WorkPtr && ord == WorkPtr->Order) return;
  if (wrk != NULL)
    {
      WorkArea * wp = WorkPtr;
      WorkPtr = wrk;
      if (WorkPtr->Used)
	{
	  int n = WorkPtr->Order;
	  SetOrder(ord);
	  IxFlush();
	  SetOrder(n);
	}
      WorkPtr = wp;
    }
  InitPageBoxTab(ipp->PageBoxTab);
  ShareTab[sha].Wrk = WorkPtr;
  ShareTab[sha].Ord = WorkPtr->Order;
}






static Boolean GetPage(word rp, int lev)
{
  IPP ipp = IxPkP();
  PageBox *P = IBoxP(lev);
  int f = ipp->IFile;

  if (lev >= MaxLevels) Err(BadIxFileErr);
  SwitchSharedIx(ipp);
  if (rp != NoRec && P->PgPos == rp) Err(Ok);
  if (! WritePage(lev)) return FALSE;
  if (rp == NoRec)
    {
      IxFileDesc * id = &ipp->IDesc;
      if (id->Free == NoRec)
	chsize(f, (long) (P->PgPos = id->New ++) * PageSize);
      else
	{
	  P->PgPos = id->Free;
	  if (lseek(f, (long) P->PgPos * PageSize, SEEK_SET) == -1L)
	    Err(IxSeekErr);
	  if (_read(f, &P->Page, PageSize) == -1) Err(IxReadErr);
	  id->Free = P->Page.RightPage;
	}
      P->Page.LessPage = NoRec;
      P->Page.Count = 0;
     #if CLEAR_PAGE
       memset(P->Page.Node, 0, PageRoom);
     #endif
      PutPage(lev);
    }
  else
    {
      P->PgPos = rp;
      if (lseek(f, (long)rp * PageSize, SEEK_SET) == -1L)
	Err(IxSeekErr);
      if (_read(f, &P->Page, PageSize) == -1) Err(IxReadErr);
      P->Updated = FALSE;
    }
  Err(Ok);
}





static Boolean Find_Cur (char *key, word p, int lev)
{
  int		li, ri, i;
  IPP		ipp;
  PageBox *	pbp;
  long		cmp;
  IxNode *	Np;
  IxPage *	P;

  if (Work.HeadPtr->RecCount == 0) return TRUE;
  if (p == NoRec && Work.RecNo != Work.HeadPtr->RecCount) Err(BadIxFileErr);
  ipp = IxPkP();
  pbp = ipp->PageBoxTab;
  P = &pbp[lev].Page;
  GetPage(p, lev); Np = P->Node;
  if (Work.RecNo == Work.HeadPtr->RecCount)
    { pbp[(CurLev = 0)].CurOff = P->Count; return TRUE; }

  li = 0; ri = P->Count-1; /* binary search */
  do
    {
      i = (li+ri) / 2;
      cmp = (long) memcmp(key, Np[i].Item, ipp->IDesc.KeyLen);
      if (cmp == 0L) cmp = Work.RecNo - NodePos(Np[i]);
      if (cmp <= 0L) ri = i-1; if (cmp >= 0L) li = i+1;
    }
  while (li <= ri);
  CurLev=lev; pbp[CurLev].CurOff=ri; CurPos=Work.RecNo;
  if (li-ri > 1) { ++ pbp[CurLev].CurOff; return TRUE; }
  else return Find_Cur(key, (ri<0) ? P->LessPage : Np[ri].NextPage, lev+1);
}







static Boolean FindCur (void)
{
  return Find_Cur(CurKey(), IxPkP()->IDesc.Root, 0);
}






static void InsNode(IxPage *P, IxNode *N, int pos)
{
  int i = P->Count ++; IxNode *Np = P->Node;
  while (i > pos) { Np[i] = Np[i-1]; -- i; } Np[i] = *N;
}






static Boolean BTAdd (word p, IxNode *NN, int lev)
{
  int		li, ri, i, j;
  long		cmp;
  IxNode	*Np, *Np1;
  Boolean	R, Fin;
  PageBox	*P;

  if (p == NoRec) return FALSE;
  P = IBoxP(lev);
  GetPage(p, lev); Np = P->Page.Node;

  li = 0; ri = P->Page.Count-1; /* binary search */
  do
    {
      i = (li+ri) / 2;
      cmp = (long) memcmp(NN->Item, Np[i].Item, IxPkP()->IDesc.KeyLen);
      if (cmp == 0) cmp = NodePos(*NN) - NodePos(Np[i]);
      if (cmp <= 0) ri = i-1; if (cmp >= 0) li = i+1;
    }
  while (li <= ri); i = ri+1;

  Fin = BTAdd((ri < 0) ? P->Page.LessPage : Np[ri].NextPage, NN, lev+1);
  if (! Fin)
    {
      Fin = P->Page.Count < MaxNodes*2;
      if (Fin)
	{
	  InsNode(&P->Page, NN, i);
	}
      else /* split! */
	{
	  GetPage(NoRec, Spare); Np1 = SpareBox.Page.Node;
	  R = i > MaxNodes;
	  for (j = 0; j < MaxNodes-R; j ++) Np1[j] = (Np+MaxNodes+R)[j];
	  P->Page.Count = MaxNodes+R; SpareBox.Page.Count = MaxNodes-R;
	  if (R) InsNode(&SpareBox.Page, NN, i-MaxNodes-1);
	  else InsNode(&P->Page, NN, i);
	  *NN = Np[MaxNodes];
	  -- P->Page.Count;
	 #if CLEAR_PAGE
	   memset(Np+P->Page.Count, 0, (MaxNodes+MaxNodes-P->Page.Count)*sizeof(IxNode));
	 #endif
	  SpareBox.Page.LessPage  = NN->NextPage;
	  SpareBox.Page.LeftPage  = P->PgPos;
	  SpareBox.Page.RightPage = P->Page.RightPage;
	  P->Page.RightPage     = SpareBox.PgPos;
	  NN->NextPage          = SpareBox.PgPos;
	  PutPage(Spare);

	  /// link Right neighbour
	  if (SpareBox.Page.RightPage != NoRec)
	    {
	      GetPage(SpareBox.Page.RightPage, Spare);
	      SpareBox.Page.LeftPage = P->Page.RightPage;
	      PutPage(Spare);
	    }

	  WritePage(Spare);
	}
      PutPage(lev);
    }
  return Fin;
}






static Boolean PutIxKey (char *key, RecPos filepos)
{
  IxNode Node; PageBox *P;
  IxFileDesc * idp = &IxPkP()->IDesc;

  if (key[0]==2&&key[1]=='R')
    asm int 3;
  memcpy(Node.Item, key, idp->KeyLen);
  Node.NextPage = NoRec;
  Node.FilePos = (word) filepos;
  Node.FilePosHB = (byte) (filepos >> 16);
  if (! BTAdd(idp->Root, &Node, 0)) /* new Root */
    {
      IxFlush();
      if (! GetPage(NoRec, 0)) return FALSE;
      P = &IxPkP()->PageBoxTab[0];
      P->Page.LeftPage = NoRec;
      P->Page.RightPage = NoRec;
      P->Page.LessPage = idp->Root;
      idp->Root = P->PgPos;
      InsNode(&P->Page, &Node, 0);
      PutPage(0);
    }
  return TRUE;
}






static void DelNode(IxPage *P, int pos)
{
  int cnt = -- P->Count; IxNode *Np = P->Node;
  while (pos < cnt) { Np[pos] = Np[pos+1]; ++ pos; }
 #if CLEAR_PAGE
  memset(Np+cnt, 0, sizeof(IxNode));
 #endif
}







static Boolean Comb (IxPage *P, int pos, word p, int lev)
{
  IxPage *P1, *P2;
  IxNode *Np = P->Node, *Np1, *Np2;
  int i, k;
  IPP ipp = IxPkP();

  P1 = &ipp->PageBoxTab[lev].Page;		Np1 = P1->Node;
  P2 = &SparePage;				Np2 = P2->Node;
  GetPage(p, lev); if (P1->Count >= MaxNodes) return TRUE;
  if (pos < P->Count-1) /* combine with right page */
    {
      GetPage(Np[pos+1].NextPage, Spare);
      Np1[MaxNodes-1] = Np[pos+1]; Np1[MaxNodes-1].NextPage = P2->LessPage;
      k = (P2->Count - MaxNodes + 1) / 2;
      if (k) /* move k nodes */
	{
	  for (i = 0; i < k-1; i ++) Np1[MaxNodes+i] = Np2[i];
	  Np[pos+1] = Np2[i]; Np[pos+1].NextPage = SpareBox.PgPos;
	  P2->LessPage = Np2[i].NextPage; P2->Count -= k;
	  for (i = 0; i < P2->Count; i ++) Np2[i] = Np2[k+i];
	  P1->Count = MaxNodes-1 + k; PutPage(lev-1);
	  PutPage(lev); PutPage(Spare); WritePage(Spare);
	  return TRUE;
	}
      else /* join pages */
	{
	  for (i = 0; i < MaxNodes; i ++) Np1[MaxNodes+i] = Np2[i];
	  P1->Count = 2 * MaxNodes; DelNode(P, pos+1); PutPage(lev-1);
	  PutPage(lev); DelPage(Spare); WritePage(Spare);
	  return FALSE;
	}
    }
  else /* combine with left page */
    {
      GetPage((pos) ? Np[pos-1].NextPage : P->LessPage, Spare);
      k = (P2->Count - MaxNodes + 1) / 2;
      if (k) /* move k nodes */
	{
	  for (i = MaxNodes-2; i >= 0; i --) Np1[k+i] = Np1[i];
	  Np1[k-1] = Np[pos]; Np1[k-1].NextPage = P1->LessPage;
	  P2->Count -= k;
	  for (i = 0; i < k-1; i ++) Np1[i] = Np2[P2->Count+1+i];
	  P1->LessPage = Np2[P2->Count].NextPage;
	  Np[pos] = Np2[P2->Count]; Np[pos].NextPage = p;
	  P1->Count = MaxNodes-1 + k; PutPage(lev-1);
	  PutPage(lev); PutPage(Spare); WritePage(Spare);
	  return TRUE;
	}
      else /* join pages */
	{
	  Np2[MaxNodes] = Np[pos]; Np2[MaxNodes].NextPage = P1->LessPage;
	  for (i = 0; i < MaxNodes-1; i ++) Np2[MaxNodes+1+i] = Np1[i];
	  P2->Count = 2 * MaxNodes; DelNode(P, pos); PutPage(lev-1);
	  DelPage(lev); PutPage(Spare); WritePage(Spare);
	  return FALSE;
	}
    }
}






static GrabNode (IxNode *dn, word p, int lev)
/* dn points to deleted node */
{
  IxPage *P = &IxPkP()->PageBoxTab[lev].Page;
  IxNode *Np = P->Node;
  word dp; int last;

  GetPage(p, lev);
  last = P->Count-1;
  dp = Np[last].NextPage;
  if (dp == NoRec)
    {
      Np[last].NextPage = dn->NextPage;
      *dn = Np[last];
      -- P->Count;
     #if CLEAR_PAGE
       memset(Np+last, 0, sizeof(IxNode));
     #endif
      PutPage(lev);
      return P->Count >= MaxNodes;
    }
  else return (GrabNode(dn, dp, lev+1)) ? TRUE : Comb(P, last, dp, lev+1);
}





static Boolean BTDel (word p, IxNode *NN, int lev)
{
  int		li, ri, i;
  long		cmp;
  word		dp;
  IxNode *	Np;
  IxPage *	P;
  IPP		ipp;
  PageBox *	pbp;

  if (p == NoRec) Err(BadIxFileErr);
  ipp = IxPkP();
  pbp = ipp->PageBoxTab;
  P = &pbp[lev].Page;
  GetPage(p, lev); Np = P->Node;

  li = 0; ri = P->Count-1; /* binary search */
  do
    {
      i = (li+ri) / 2;
      cmp = (long) memcmp(NN->Item, Np[i].Item, ipp->IDesc.KeyLen);
      if (cmp == 0) cmp = NodePos(*NN) - NodePos(Np[i]);
      if (cmp <= 0) ri = i-1; if (cmp >= 0) li = i+1;
    }
  while (li <= ri);
  CurLev=lev; pbp[lev].CurOff=ri; CurPos=Work.RecNo;
  dp = (ri < 0) ? P->LessPage : Np[ri].NextPage;
  if (li-ri > 1)
    { /* found! */
      ++ pbp[lev].CurOff;
      PutPage(lev);
      if (IsLeaf(lev))
	{
	  DelNode(P, ri+1);
	  return P->Count >= MaxNodes;
	}
      else return (GrabNode(&Np[ri+1], dp, lev+1)) ? TRUE
						   : Comb(P, ri, dp, lev+1);
    }
  else return (BTDel(dp, NN, lev+1)) ? TRUE : Comb(P, ri, dp, lev+1);
}






static Boolean DelIxKey (char *key, RecPos filepos)
{
  IxNode Node; PageBox *P = &IxPkP()->PageBoxTab[0];
  IxFileDesc * idp = &IxPkP()->IDesc;

  memcpy(Node.Item, key, idp->KeyLen);
  Node.NextPage = NoRec;
  Node.FilePos = (word) filepos;
  Node.FilePosHB = (byte) (filepos >> 16);
  BTDel(idp->Root, &Node, 0);
  if (P->Page.Count == 0)
    {
      idp->Root = P->Page.LessPage;
      DelPage(0);
      IxFlush();
    }
  return TRUE;
}






Boolean PutAllIx (void)
{
  int i, ord = DbfOrder();

  for (i = 0; i < MaxIxFiles; i ++)
    {
      SetOrder(i);
      if (IxPkP() != NULL) PutIxKey(CurKey(), Work.RecNo);
    }
  SetOrder(ord);
  return TRUE;
}






Boolean DelAllIx (void)
{
  int i, ord = DbfOrder();

  for (i = 0; i < MaxIxFiles; i ++)
    {
      SetOrder(i);
      if (IxPkP() != NULL) DelIxKey(CurKey(), Work.RecNo);
    }
  SetOrder(ord);
  return TRUE;
}






static Boolean Find (const char *Key, word p, int lev)
{
  int		li, ri, i, cmp;
  IxNode *	Np;
  IxPage *	P;
  IPP		ipp = IxPkP();
  PageBox *	pbp = ipp->PageBoxTab;

  if (! lev) CurPos = Work.HeadPtr->RecCount;
  if (p == NoRec) return FALSE;
  P = &pbp[lev].Page;
  GetPage(p, lev); Np = P->Node;

  li = 0; ri = P->Count-1; /* binary search */
  do
    {
      i = (li+ri) / 2;
      cmp = memcmp(Key, Np[i].Item, ipp->IDesc.KeyLen);
      if (cmp == 0)
	{
	  -- cmp;
	  Found = TRUE;
	}
      if (cmp <= 0) ri = i-1; if (cmp >= 0) li = i+1;
    }
  while (li <= ri); i = ri+1;
  CurLev = lev; pbp[CurLev].CurOff = i;
  if (i < P->Count) CurPos = NodePos(Np[i]);
  if (li-ri > 1) return TRUE;
  else return Find(Key, (ri < 0) ? P->LessPage
				 : Np[ri].NextPage, lev+1);
}






static Boolean Pred (void)
{
  IxNode *Np; int i;
  IPP ipp = IxPkP();
  PageBox * pbp = ipp->PageBoxTab;

  if (_DbfBof() || !Work.HeadPtr->RecCount) return FALSE;
  while (TRUE)
    {
      i = -- pbp[CurLev].CurOff; if (IsLeaf(CurLev)) break;
      Np = pbp[CurLev].Page.Node;
      if (! GetPage((i < 0) ? pbp[CurLev].Page.LessPage
			    : Np[i].NextPage, CurLev+1)) return FALSE;
      ++ CurLev; pbp[CurLev].CurOff = pbp[CurLev].Page.Count;
    }
  while (pbp[CurLev].CurOff < 0 && CurLev > 0) -- CurLev;
  if ((i = pbp[CurLev].CurOff) < 0) Err(BadIxFileErr);
  Np = pbp[CurLev].Page.Node;
  CurPos = NodePos(Np[i]);
  return TRUE;
}








static Boolean Succ (void)
{
  IxNode *n; int i;
  IPP ipp;
  PageBox * pbp;

  if (CurPos == Work.HeadPtr->RecCount) return FALSE;
  ipp = IxPkP();
  pbp = ipp->PageBoxTab;
  while (! IsLeaf(CurLev))
    {
      i = pbp[CurLev].CurOff; n = pbp[CurLev].Page.Node;
      if (! GetPage((i < 0) ? pbp[CurLev].Page.LessPage : n[i].NextPage, CurLev+1))
	return FALSE;
      ++ CurLev; pbp[CurLev].CurOff = -1;
    }
  while (CurLev >= 0)
    {
      i = pbp[CurLev].CurOff;
      if (i < pbp[CurLev].Page.Count-1)
	{
	  n = pbp[CurLev].Page.Node;
	  pbp[CurLev].CurOff = ++ i;
	  CurPos = NodePos(n[i]);
	  return TRUE;
	}
      else -- CurLev;
    }
  ++ CurLev; ++ pbp[CurLev].CurOff;
  CurPos = Work.HeadPtr->RecCount; return TRUE;
}






/******************************************************/
/*           private database definitions             */
/******************************************************/






int CmpMirror (void)
{
  word rl = WorkPtr->HeadPtr->RecLength;
  char * rp = WorkPtr->RecPtr;
  return memcmp(rp, rp+rl, rl);
}





void PutMirror (void)
{
  word rl = WorkPtr->HeadPtr->RecLength;
  char * rp = WorkPtr->RecPtr;
  memcpy(rp+rl, rp, rl);
}





static void GetMirror (void)
{
  word rl = WorkPtr->HeadPtr->RecLength;
  char * rp = WorkPtr->RecPtr;
  memcpy(rp, rp+rl, rl);
}





static Boolean Back(void)
{
  if (lseek(Work.FileHan, - (long)Work.HeadPtr->RecLength, SEEK_CUR) == -1L)
    Err(FSeekErr);
  if (! Work.RdOnly) PutMirror();
  Err(Ok);
}






void ClrBuff (void)
{
  memset(Work.RecPtr, ' ', Work.HeadPtr->RecLength * (2 - Work.RdOnly));
}






static Boolean PutRec (void)
{
  UseChk;
  if (! Work.RdOnly && Work.RecPtr != NULL) if (CmpMirror())
    {
      Boolean new;
      if (Work.WriteHook != NULL) Work.WriteHook();
      new = (Work.RecNo == Work.HeadPtr->RecCount);
      if (! new) DelAllIx(); //---- remove old indexes
      GetMirror(); //---- copy updated data
      if (_write(Work.FileHan, Work.RecPtr, Work.HeadPtr->RecLength) == -1)
	Err(FWriteErr);
      Back();
      if (new) ++ Work.HeadPtr->RecCount;
      Work.AnyUpdat = TRUE;
      if (! PutAllIx()) return FALSE;
    }
  Err(Ok);
}





static Boolean GetRec (void)
{
  if (Work.RecPtr != NULL)
    {
      if (! PutRec()) return FALSE;
      if (Work.RecNo == Work.HeadPtr->RecCount) ClrBuff();
      else
	{
	  Hnt(401)
	  if (_read(Work.FileHan, Work.RecPtr, Work.HeadPtr->RecLength) == -1)
	    Err(FReadErr);
	  return Back();
	}
    }
  return TRUE;
}






Boolean DbfUnlock (void)
{
  if  (Work.Locked)
    {
      if (Work.Locked == 1)
	{
	  int i, n;
	  if (! DbfFlush()) return FALSE;
	  n = DbfOrder();
	  for (i = 0; i < MaxIxFiles; i ++)
	    {
	      IPP ipp;
	      SetOrder(i);
	      if ((ipp = IxPkP()) != NULL) NetUnlock(ipp->IFile);
	    }
	  SetOrder(n);
	  NetUnlock(Work.FileHan);
	}
      -- Work.Locked;
    }
  return TRUE;
}





Boolean DbfHardUnlock (void)
{
  do
    {
      if (! DbfUnlock()) return FALSE;
    }
  while (Work.Locked);
  Err(Ok);
}





Boolean DbfNetLock (void)
{
  Boolean ReadIxDesc (void);
  int i, n = DbfOrder();
  Boolean eof;

  if (Network)
    {
      if (Work.Locked)
	{
	  ++ Work.Locked;
	  return TRUE;
	}
      eof = (Work.RecNo == Work.HeadPtr->RecCount);

      // take exclusive access
      if (NetLock(Work.FileHan)) ++ Work.Locked;
      for (i = 0; i < MaxIxFiles; i ++)
	{
	  IPP ipp;
	  SetOrder(i);
	  if ((ipp = IxPkP()) != NULL)
	    {
	      NetLock(ipp->IFile);
	      InitPageBoxTab(ipp->PageBoxTab);
	      ReadIxDesc();
	    }
	}
      SetOrder(n);

      // read actual header
      if (lseek(Work.FileHan, 0L, SEEK_SET) == -1) Err(FSeekErr);
      if (_read(Work.FileHan, Work.HeadPtr, sizeof(Header)) == -1 ||
	 (Work.HeadPtr->Signature != dB3 && Work.HeadPtr->Signature != dB3M))
	Err(FReadErr);

      // set correct RecCount ??? $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
   //   if ((l = filelength(Work.FileHan)) == -1L) goto fail;
   //   Work.HeadPtr->RecCount = (l - Work.HeadPtr->HdrLength) / Work.HeadPtr->RecLength;

      // skip to current record, (or new current record)
      if (eof || Work.RecNo > Work.HeadPtr->RecCount)
	{
	  Work.RecNo = Work.HeadPtr->RecCount;
	}
      if (lseek(Work.FileHan, Work.RecNo * Work.HeadPtr->RecLength + Work.HeadPtr->HdrLength, SEEK_SET) == -1)
	Err(FSeekErr);
      if (! GetRec()) return FALSE;
      if (IxPkP() != NULL) return FindCur();
    }
  return TRUE;
}








Boolean DbfLock (void)
{
  UseChk;
  if (! DbfNetLock()) return FALSE;
  if (! PutRec()) return FALSE;
  if (IxPkP() != NULL) return FindCur();
  Err(Ok);
}








/******************************************************/
/*                   dBASE service                    */
/******************************************************/






Boolean SetOrder (int IxNr)
{
  if (IxNr < 0 || IxNr >= MaxIxFiles) Err(RangeErr);
  Work.Order = IxNr;
  return TRUE;
}







Boolean DbfFlush (void)
{
  int i, n;
  if (Work.Used)
    {
      if (! PutRec()) return FALSE;
      if (Work.AnyUpdat)
	{
	  Header *h = Work.HeadPtr; struct date d;
	  getdate(&d);
	  h->LastUpdt[0] = d.da_year-1900;
	  h->LastUpdt[1] = d.da_mon;
	  h->LastUpdt[2] = d.da_day;

	  //--- write main piece of base header
	  if (lseek(Work.FileHan, 0L, SEEK_SET) == -1) Err(FSeekErr);
	  if (_write(Work.FileHan, h, sizeof(Header)) == -1)
	    Err(FWriteErr);

	  //--- write field descriptions if just created
	  if (Work.RecPtr == NULL)
	    if (_write(Work.FileHan, Work.DescPtr,
		Work.Fields*sizeof(FieldDesc)+1) == -1) Err(FWriteErr);

	  //--- adjust file size
	  if (chsize(Work.FileHan,
	      h->RecCount * h->RecLength + h->HdrLength) == -1)
		Err(FWriteErr);

	  //--- write trailing EOF char required for some stupid programs
	  if (lseek(Work.FileHan,
	      h->RecCount * h->RecLength + h->HdrLength,
	      SEEK_SET) == -1) Err(FSeekErr);
	  if (_write(Work.FileHan, "\x1A", 1) == -1) Err(FWriteErr);

	  //--- flush buffers to disk
	  FileFlush(Work.FileHan);

	  //--- return file pointer to current position
	  if (lseek(Work.FileHan,
	      Work.RecNo * h->RecLength + h->HdrLength,
	      SEEK_SET) == -1) Err(FSeekErr);

	  n = DbfOrder();
	  for (i = 0; i < MaxIxFiles; i ++)
	    {
	      SetOrder(i);
	      if (IxPkP() != NULL) IxFlush();
	    }
	  SetOrder(n);
	  Work.AnyUpdat = FALSE;
	}
    }
  Err(Ok);
}










Boolean DbfClose (void)
{
  int i;

  if (Work.Used)
    {
      DbfNetLock();
      DbfFlush();
      for (i = 0; i < MaxIxFiles; i ++) // close all indexes
	{ SetOrder(i); if (! IxClose()) return FALSE; }
      DbfHardUnlock();
      if (Work.RecPtr != NULL) free(Work.RecPtr);
      free(Work.HeadPtr);
      free(Work.DescPtr);
      if (_close(Work.FileHan) == -1) Err(FCloseErr);
      Work.WriteHook = NULL;
      Work.Filter = NULL;
      Work.Used = FALSE;
    }
  Err(Ok);
}




static Boolean _DbfOpen (const char *fn, Boolean wrt)
{
  long l; int i; byte * ptr;

  if (! DbfClose()) return FALSE;
  Normalize(Work.dBPath, fn, "DBF");
  if (! IsFile(Work.dBPath)) Err(NoFileErr);
  Work.RdOnly = (wrt == FALSE) ? TRUE : (! IsWritable(Work.dBPath));

  Work.FileHan = (Work.RdOnly) ? -1 : _open(Work.dBPath, O_RDWR | O_DENYNONE);
  if (Work.FileHan == -1)
    {
      if (wrt == TRUE) Err(FOpenErr);
      if ((Work.FileHan = _open(Work.dBPath, O_RDONLY | O_DENYNONE)) == -1)
	Err(FOpenErr);
      Work.RdOnly = TRUE;
    }
  if ((Work.HeadPtr = (Header *) malloc(sizeof(Header))) == NULL)
    Err(NoCoreErr);
  Work.HeadPtr->Signature = 0xFF;

  // take exclusive access
  if (Network) if (NetLock(Work.FileHan)) ++ Work.Locked;

  Hnt(402)
  if (_read(Work.FileHan, Work.HeadPtr, sizeof(Header)) == -1 ||
     (Work.HeadPtr->Signature != dB3 && Work.HeadPtr->Signature != dB3M))
    { free(Work.HeadPtr); Err(FReadErr); }
  Work.Fields = Work.HeadPtr->HdrLength - sizeof(Header);
  if ((Work.DescPtr = (FieldDesc *) malloc(Work.Fields)) == NULL)
    { free(Work.HeadPtr); Err(NoCoreErr); }
  Hnt(403);
  if (_read(Work.FileHan, Work.DescPtr, Work.Fields) == -1)
    { free(Work.HeadPtr); free(Work.DescPtr); Err(FReadErr); }
  Work.Fields /= sizeof(FieldDesc);
  if ((ptr = malloc(Work.HeadPtr->RecLength * (2 - Work.RdOnly))) == NULL)
    { free(Work.HeadPtr); free(Work.DescPtr); Err(NoCoreErr); }
  Work.RecPtr = ptr ++;
  for (i = 0; i < Work.Fields; i ++)
    {
      Work.DescPtr[i].FldPtr = ptr;
      ptr += Work.DescPtr[i].FldLen;
    }
  Work.Used = TRUE;
  Work.AnyUpdat = FALSE;
  for (i = 0; i < MaxIxFiles; i ++) Work.IxPk[i] = NULL;
  SetOrder(0);
  Hnt(404)
  if ((l = filelength(Work.FileHan)) == -1L) Err(FReadErr);
  Work.HeadPtr->RecCount = (l - Work.HeadPtr->HdrLength) / Work.HeadPtr->RecLength;
  ClrBuff();
  Work.Filter = NULL;
  Work.WriteHook = NULL;
  Work.RecNo = 0;
  if (! GetRec()) return FALSE;
  if (Network)
    {
      NetUnlock(Work.FileHan);
      -- Work.Locked;
    }
  Err(Ok);
}






Boolean DbfOpen (const char *fn)
{
  return _DbfOpen(fn, -1);
}




Boolean DbfOpenToWrite (const char *fn)
{
  return _DbfOpen(fn, TRUE);
}




Boolean DbfOpenToRead (const char *fn)
{
  return _DbfOpen(fn, FALSE);
}





Boolean DbfGoto (RecPos recn)
{
  UseChk;
  if (! DbfLock()) return FALSE;
  if (recn < 0 || recn > Work.HeadPtr->RecCount)
    {
      recn = Work.HeadPtr->RecCount;
      dBError = RangeErr;
    }
  if (lseek(Work.FileHan, recn * Work.HeadPtr->RecLength + Work.HeadPtr->HdrLength, SEEK_SET) == -1)
    Err(FSeekErr);
  Work.RecNo = (CurPos = recn);
  if (! GetRec()) return FALSE;
  return DbfUnlock();
}






Boolean _DbfGoTop (void)
{
  int i; word p; IxNode *n;
  IPP ipp = IxPkP();
  PageBox * pbp = ipp->PageBoxTab;

  if (ipp == NULL) return DbfGoto(0);
  if (! DbfNetLock()) return FALSE;
  for (i = 0, p = ipp->IDesc.Root; p != NoRec; i ++)
    {
      GetPage(p, i);
      p = pbp[i].Page.LessPage;
    }
  if (! i) return DbfUnlock();
  n = pbp[i-1].Page.Node;
  Work.RecNo = NodePos(n[0]);
  if (lseek(Work.FileHan, Work.RecNo * Work.HeadPtr->RecLength + Work.HeadPtr->HdrLength, SEEK_SET) == -1)
    Err(FSeekErr);
  if (! GetRec()) return FALSE;
  return DbfUnlock();
}







Boolean _DbfGoBottom (void)
{
  UseChk;
  DbfLock();
  PutRec(); // ???????????????????????? $$$$$$$$$$$$$$$$$$$$$$
  DbfGoto(Work.HeadPtr->RecCount);
  DbfUnlock();
  Err(Ok);
}







Boolean _DbfSkip (int n)
{
  RecPos fp;

  UseChk;
  if (! DbfLock()) return FALSE;
  if (n == 0) fp = Work.RecNo;
  else
    {
      if ((n > 0) ? _DbfEof() : _DbfBof()) goto xit;
      if (IxPkP() != NULL)
	{
	  while (n > 0) { if (! Succ()) break; -- n; }
	  while (n < 0) { if (! Pred()) break; ++ n; }
	  if (dBError) goto xit;
	  fp = CurPos;
	}
      else /* !indexed */
	{
	  fp = Work.RecNo + n; n = 0;
	  fp = max(fp, 0L); /* not less */
	  fp = min(fp, Work.HeadPtr->RecCount); /* not greater */
	}
      Work.RecNo = fp;
    }
  if (lseek(Work.FileHan, fp * Work.HeadPtr->RecLength + Work.HeadPtr->HdrLength, SEEK_SET) == -1)
    Err(FSeekErr);
  if (! GetRec()) n = 1;
  xit:
  DbfUnlock();
  return n == 0;
}






/******************************************************/
/*                   index service                    */
/******************************************************/


static	void	(*EchoProc)(void) = NULL;


void SetEchoProc (void (*ep)())
{
  EchoProc = ep;
}





static Boolean MakeIndexPack (void)
{
  IPP ipp = malloc(sizeof(*ipp));
  PageBox * pb;

  if (ipp == NULL) Err(NoCoreErr);

  if (Share >= 0)
    {
      if (Share >= SHARE_MAX) Err(RangeErr);
      if (ShareTab[Share].Cnt == 0)
	{
	  ShareTab[Share].Adr = malloc(sizeof(PageBox) * MaxLevels);
	  ShareTab[Share].Wrk = NULL;
	}
      pb = ShareTab[Share].Adr;
      ++ ShareTab[Share].Cnt;
    }
  else pb = malloc(sizeof(PageBox) * MaxLevels);
  if ((ipp->PageBoxTab = pb) == NULL)
    {
      free(ipp);
      Err(NoCoreErr);
    }
  ipp->Shared = Share;
  Share = -1;
  Work.IxPk[Work.Order] = ipp;
  Err(Ok);
}






Boolean IxClose (void)
{
  IPP ipp = IxPkP();
  if (ipp != NULL)
    {
      int sha = ipp->Shared;
      IxFlush();
      if (_close(ipp->IFile) == -1) Err(IxCloseErr);

      if (sha >= 0)
	{
	  ShareTab[sha].Wrk = NULL;
	  if (-- ShareTab[sha].Cnt == 0) sha = -1;
	}
      if (sha < 0) free(ipp->PageBoxTab);

      free(ipp);
      Work.IxPk[Work.Order] = NULL;
    }
  Err(Ok);
}




Boolean ReadIxDesc (void)
{
  IPP ipp = IxPkP();
  int f = ipp->IFile;

  if (lseek(f, - (long)sizeof(IxFileDesc), SEEK_END) == -1L)
    Err(IxSeekErr);
  if (_read(f, &ipp->IDesc, sizeof(ipp->IDesc)) == -1) Err(IxReadErr);
  Err(Ok);
}





Boolean IxOpen (const char *iname, char *(*ip)(const char *))
{
  char IxN[120];

  UseChk;
  if (! IxClose()) return FALSE;
  Normalize(IxN, iname, IxExt);
  if (! IsFile(IxN)) Err(NoIxFileErr);
  if (! Work.RdOnly) if (! IsWritable(IxN)) Err(IxOpenErr);
  if (! MakeIndexPack()) return FALSE;
  else
    {
      IPP ipp = IxPkP();

      if ((ipp->IFile = _open(IxN, ((Work.RdOnly) ? O_RDONLY : O_RDWR) | O_DENYNONE)) == -1)
	Err(IxOpenErr);
      if (Network) if (! NetLock(ipp->IFile)) return FALSE;
      if (! ReadIxDesc()) return FALSE;
      if (Network) NetUnlock(ipp->IFile);
      ipp->IProc = ip;
      ipp->IUpdt = FALSE;
      InitPageBoxTab(ipp->PageBoxTab);
    }
  return TRUE;
}





typedef struct { char key[MaxKeyLen]; RecPos fpos; } tel;

static tel huge* T;

/*
static void IQSort (dword a, dword b)
{
  dword i = a, j = b; tel t = T[(a + b) / 2];

  do
    {
      while (_fmemcmp(T[i].key, (char far *)t.key, MaxKeyLen) < 0) ++ i;
      while (_fmemcmp(T[j].key, (char far *)t.key, MaxKeyLen) > 0) -- j;
      if (i <= j)
	{
	  tel t = T[i];
	  T[i] = T[j];
	  T[j] = t;
	  ++ i;
	  -- j;
	}
    }
  while (i <= j);
  if (a < j) IQSort(a, j);
  if (i < b) IQSort(i, b);
}
*/





Boolean IxCreat (const char *iname, int fld, char *(*ip)(const char *))
{
  char IxN[120];
  dword i, j;
  word l = Work.HeadPtr->RecLength;
  int k;
  long rn = 0L;
  Boolean ok = TRUE;
  tel el, huge *t;
  IPP ipp;
  IxFileDesc * idp;
  dword tsiz = farcoreleft();

  if (tsiz < K(2)) Err(NoCoreErr);
  tsiz = (tsiz - K(1)) / sizeof(*T);

  UseChk;
  DbfNetLock();
  if (! DbfFlush()) return FALSE;

  Normalize(IxN, iname, IxExt);
  if (! IxClose()) return FALSE;

  if (IsFile(IxN) && !IsWritable(IxN)) Err(IxRdOnlyErr);
  if (! MakeIndexPack()) return FALSE;
  ipp = IxPkP();
  idp = &ipp->IDesc;
  if ((ipp->IFile = open(IxN, O_CREAT|O_TRUNC|O_RDWR|O_BINARY|O_DENYNONE,
			      S_IREAD|S_IWRITE|S_IFREG)) == -1) Err(IxOpenErr);
  ipp->IProc	= ip;
  ipp->IUpdt	= TRUE;
  idp->New	= 0;
  idp->Free	= NoRec;
  idp->Root	= NoRec;
  idp->Field	= fld;

  k = DbfFldLen(fld);
  idp->KeyLen = min(k, MaxKeyLen);

  InitPageBoxTab(ipp->PageBoxTab);

  T = farmalloc(tsiz * sizeof(*T));
  if (T == NULL) Err(NoCoreErr);

  if (lseek(Work.FileHan, (long)Work.HeadPtr->HdrLength, SEEK_SET) == -1)
    Err(FSeekErr);

  while (ok)
    {
      for (i = 0, t = T; i < tsiz; i ++, t ++)
	{
	  if (EchoProc != NULL) EchoProc();
	  ok = (k = _read(Work.FileHan, Work.RecPtr, l)) == l;
	  Hnt(405)
	  if (k == -1) Err(FReadErr);
	  if (! ok) break;
	  memcpy(el.key, ipp->IProc(Work.DescPtr[fld].FldPtr), idp->KeyLen);
	  el.fpos = rn ++;
	  *t = el;
	}
      //---IQSort(0, tsiz-1);//--------------------
      for (j = 0, t = T; j < i; j ++, t ++)
	{
	  if (EchoProc != NULL) EchoProc();
	  el = *t;
	  PutIxKey(el.key, el.fpos);
	}
    }
  farfree(T);
  if (lseek(Work.FileHan, Work.RecNo * Work.HeadPtr->RecLength + Work.HeadPtr->HdrLength, SEEK_SET) == -1)
    Err(FSeekErr);
  if (Work.RecNo == Work.HeadPtr->RecCount) ClrBuff();
  else
    {
      Hnt(406)
      if (_read(Work.FileHan, Work.RecPtr, l) != l) Err(FReadErr);
      Back();
    }
  DbfUnlock();
  return TRUE;
}








Boolean DbfFind (const char *key)
{
  IPP ipp;
  UseChk; IxChk;
  if (! DbfLock()) return FALSE;
  ipp = IxPkP();
  Found = FALSE;
  Find(ipp->IProc(key), ipp->IDesc.Root, 0);
  if (dBError) return FALSE;
  if (lseek(Work.FileHan, CurPos * Work.HeadPtr->RecLength + Work.HeadPtr->HdrLength, SEEK_SET) == -1)
    Err(FSeekErr);
  Work.RecNo = CurPos;
  if (! GetRec()) return FALSE;
  DbfUnlock();
  return Found;
}






/*** end of file ***/

