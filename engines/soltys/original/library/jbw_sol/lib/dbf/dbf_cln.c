#include	"dbf_inc.h"



extern	long	CurPos;



Boolean DbfClone (void)
{
  long n;
  UseChk;
  if (! DbfLock()) return FALSE;
  n = Work.HeadPtr->RecCount;		// Eof's record number
  if (lseek(Work.FileHan, n * Work.HeadPtr->RecLength + Work.HeadPtr->HdrLength, SEEK_SET) == -1) Err(FSeekErr);
  Work.RecNo = (CurPos = n);
  -- WorkPtr->RecPtr[0];		// Set up tiny mirror trick!
  return DbfUnlock();
}
