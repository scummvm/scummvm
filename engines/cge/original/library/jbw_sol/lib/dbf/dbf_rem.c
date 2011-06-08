#include	"dbf_inc.h"





// remove current record
Boolean DbfRemove (void)
{
  long rn, rc, next;
  word len;
  char * rec, * mir;

  if (Work.RdOnly) Err(FRdOnlyErr);
  if (! DbfLock()) return FALSE;
  rn = Work.RecNo;
  rc = Work.HeadPtr->RecCount;
  len = Work.HeadPtr->RecLength;
  rec = Work.RecPtr;
  mir = rec + len;

  if (rn == rc) // Eof
    {
      DbfUnlock();
      return TRUE;
    }

  //--- see next record number
  if (IxPkP() != NULL)
    {
      _DbfSkip(1);
      next = Work.RecNo;
      DbfGoto(rn);
      if (next == rc) -- next;
      else
	if (next == rc-1) next = rn;
    }
  else next = rn;

  if (rn == rc-1) // last record?
    {
      //--- deleted record is fully wiped out
      DelAllIx();
      ClrBuff(); //--- looks as non-updated EOF
    }
  else
    {
      char * p, * q;

      //--- get the last (physically) record data
      if (lseek(Work.FileHan, (rc-1) * len + Work.HeadPtr->HdrLength, SEEK_SET) == -1)
	Err(FSeekErr);
      Hnt(410)
      if (_read(Work.FileHan, rec, len) == -1) Err(FReadErr);
      Work.RecNo = rc-1;

      //--- remove indexes of the last record
      DelAllIx();

      //--- exchange record and mirror contents
      for (p = rec, q = mir; p != mir; p ++, q ++)
	{
	  char x = *p;
	  *p = *q;
	  *q = x;
	}

      //--- return to previous file position
      if (lseek(Work.FileHan, rn * len + Work.HeadPtr->HdrLength, SEEK_SET) == -1)
	Err(FSeekErr);
      Work.RecNo = rn;
    }

  Work.AnyUpdat = TRUE;

  //--- remove the last record from file
  if (chsize(Work.FileHan, (-- Work.HeadPtr->RecCount) * len + Work.HeadPtr->HdrLength) == -1)
    Err(FWriteErr);

  //--- solid write
  DbfGoto(next);
  return DbfUnlock(); // matches leading lock
}
