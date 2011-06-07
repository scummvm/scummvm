#include	<base.h>
#include	<string.h>
#include	<dir.h>


	char *	OpenMsg = NULL;
	Boolean	ReindexMode = FALSE;



IndexProcType BaseIdxProc
{
  static char Bezo[] = "ACELNOSZZ";
  static char key_[2*MaxKeyLen+1];
  int i, j, c;

  char *s = key_;

  for (i = 0; i < MaxKeyLen; i ++)
    {
      c = key[i];
      if (c >= 'a')
	if (c <= 'z') c -= 'a' - 'A';
	else
	  for (j = 0; j < 9; j ++)
	    if (c == Male[j] || c == Duze[j])
	      {
		*(s ++) = Bezo[j];
		c = (j == 7) ? '|' : '~';
		break;
	      }
      *(s ++) = c;
    }
  key_[MaxKeyLen] = 0; return key_;
}





void BaseSelect (int B)
{
  BaseHan * bh = &Base[B];
  char *bn = bh->Name;

  DbfSelect(B);
  if (DbfUsed()) return;

  if (OpenMsg) Message(HLP, OpenMsg);

  /// see if base file exists
  if (! IsFile(bn))
    {
      if (Log) Log("Utworzenie brakuj†cego pliku", bn);
      DbfCreate(bn, bh->Format);
    }

  /// attach base to use
  if (DbfOpenToWrite(bn))
    {
      char xn[MAXFILE+MAXEXT], *p = strchr(bn, '.');
      int f, o, i = p-bn; Boolean ix_ok;
      Boolean empty = (DbfRCount() == 0);
      FldDef * fd;

      /// check base structure
      for (f = 0, fd = bh->Format; fd->Tpe != '\0'; f ++, fd ++)
	{
	  if ((~0x20 & fd->Tpe) != DbfFldTpe(f)) break;
	  if (          fd->Len != DbfFldLen(f)) break;
	  if (          fd->Dec != DbfFldDec(f)) break;
	}
      if (f != DbfFCount() || fd->Tpe != '\0') DbfError(BadFileErr, bn);

      /// attach index file
      memcpy(xn, bn, i);
      strcpy(xn+i, ".I0x");
      for (o = 0; o < bh->Orders; o ++)
	{
	  SetOrder(o);
	  xn[strlen(xn)-1] = '1' + o;
	  ix_ok = (! ReindexMode && IsFile(xn));
	  if (ix_ok)
	    {
	      dword dbf = DwordFileTime(bn),
		    idx = DwordFileTime(xn);
	      ix_ok = (idx > 0 && idx+2 >= dbf);
	    }
	  if (! ix_ok)
	    {
	      Wind *w;
	      if (! empty)
		{
		  char *m = "Trwa indeksowanie, prosz‘ czeka...";
		  w = MakeProgress(0, 0, HLP, m, 2 * DbfRCount());
		  StdMove(w);
		  ShowWind(w);
		  Refresh();
		  SetEchoProc((void(*)(void))Progress);
		}
	      IxCreat(xn, o, (o && bh->IdxProc) ? bh->IdxProc : BaseIdxProc);
	      if (! empty)
		{
		  CloseWind(w);
		  SetEchoProc(NULL);
		}
	    }
	  Share = bh->Share;
	  IxOpen(xn, (o && bh->IdxProc) ? bh->IdxProc : BaseIdxProc);
	}
      DbfSetFilter(bh->Filter);
      if (! ReindexMode) GoCurrent(B);
    }
}
