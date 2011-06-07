#include	"dbf_inc.h"



Boolean DbfCreate (const char *name, FldDef *fd)
{
  int i, fdsiz, fc;

  if (! DbfClose()) return FALSE;
  Normalize(Work.dBPath, name, "DBF");
  if ((Work.FileHan = open(Work.dBPath, O_DENYALL |
			   O_CREAT | O_TRUNC | O_RDWR | O_BINARY,
			   S_IREAD | S_IWRITE | S_IFREG)) == -1)
    Err(FOpenErr);

  if ((Work.HeadPtr = malloc(sizeof(Header))) == NULL) Err(NoCoreErr);
  memset(Work.HeadPtr, 0, sizeof(Header));

  for (fc = 0; fc < MaxIxFiles; fc ++) if (fd[fc].Tpe == '\0') break;
  fdsiz = fc * sizeof(FieldDesc);

  Work.HeadPtr->Signature = dB3;
  Work.HeadPtr->RecCount  = 0;
  Work.HeadPtr->HdrLength = sizeof(Header) + fdsiz + 1;
  Work.HeadPtr->RecLength = 1;

  if ((Work.DescPtr = malloc(fdsiz+1)) == NULL)
    { free(Work.HeadPtr); Err(NoCoreErr); }
  memset(Work.DescPtr, 0, fdsiz+1);

  *(((char *) Work.DescPtr) + fdsiz) = 13;
  for (i = 0; i < fc; i ++)
    {
      char T = (fd->Tpe = Upper(fd->Tpe));
      strcpy(Work.DescPtr[i].FldNme, fd->Nme);
      Work.DescPtr[i].FldTpe = T;
      if (T == 'L') fd->Len = 1;
      if (T == 'D') fd->Len = 8;
      if (T != 'N') fd->Dec = 0;
      Work.DescPtr[i].FldLen = fd->Len;
      Work.DescPtr[i].FldDec = fd->Dec;
      Work.HeadPtr->RecLength += fd->Len;
      ++ fd;
    }

  Work.Used	= TRUE;
  Work.AnyUpdat	= TRUE;
  Work.Fields	= fc;
  Work.Order	= 0;
  Work.RecPtr	= NULL;
  Work.RecNo	= 0;
  for (i = 0; i < MaxIxFiles; i ++) Work.IxPk[i] = NULL;

  return DbfOpen(name);
}







