#include	"vol.h"
#include	<alloc.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<dir.h>

#ifdef	DROP_H
  #include	"drop.h"
#else
  #include	<stdio.h>
  #define	DROP(m,n)	{ printf("%s [%s]\n", (m), (n)); _exit(1); }
#endif

#define		CAT_MAX		3000
#define		BSZ		((word)K(48))



Boolean DAT::Write (CFILE& cf)
{

  if (cf.Error == 0)
    {
      byte far * buf = new far byte[BSZ];
      if (buf == NULL) DROP("No core", NULL);
      while (TRUE)
	{
	  word n = cf.Read(buf, BSZ);
	  if ((File.Error = cf.Error) != 0) break;
	  n = File.Write(buf, n);
	  if (File.Error || n < IOBUF_SIZE) break;
	}
      delete[] buf;
    }
  return File.Error == 0;
}







void VFILE::Make (const char * spec)
{
  BT_KEYPACK far * buf = (BT_KEYPACK far *) farmalloc(sizeof(BT_KEYPACK)*CAT_MAX);
  BT_KEYPACK far * bpt = buf;
  char drv[MAXDRIVE], dir[MAXDIR], nam[MAXFILE], ext[MAXEXT], pat[128];
  char olddir[MAXPATH];
  ffblk ffb;
  int i, olddrv = getdisk();
  word n = 0;

  if (buf == NULL) DROP("No core for key table", NULL);
  strupr(strcpy(pat, spec));
  fnsplit(pat, drv, dir, nam, ext);

  if (*drv)
    {
      setdisk(*drv - 'A');
      if (getdisk() != *drv - 'A') DROP("Unable to find disk", drv);
    }
  if (*dir)
    {
      int l = strlen(dir);
      if (l > 1 && dir[l-1] == '\\') dir[l-1] = '\0';
      getcwd(olddir, MAXPATH);
      chdir(dir);
    }
  fnmerge(pat, NULL, NULL, nam, ext);
  for (i = findfirst(pat, &ffb, 0); i == 0; i = findnext(&ffb))
    {
      if (stricmp(ffb.ff_name, CAT_NAME) == 0) continue;
      if (stricmp(ffb.ff_name, DAT_NAME) == 0) continue;
      _fmemset(bpt->Key, 0, BT_KEYLEN);
      _fstrupr(_fstrcpy(bpt->Key, ffb.ff_name));
      bpt->Mark = Dat.File.Mark();

      CFILE f = ffb.ff_name;
      if (f.Size() < 0x10000L) printf("adding file %s\n", ffb.ff_name);
      else
	{
	  printf("file %s too long - skipped\n", ffb.ff_name);
	  continue;
	}
      Dat.Write(CFILE(ffb.ff_name));

      bpt->Size = (word) (Dat.File.Mark() - bpt->Mark);
      ++ bpt;
      if (++n >= CAT_MAX) break;
    }
  if (*drv) setdisk(olddrv);
  if (*dir) chdir(olddir);
  if (n) Cat.Make(buf, n);
  farfree(buf);
}
