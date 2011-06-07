#include	<boot.h>
#include	<dos.h>
#include	<alloc.h>


Boot * ReadBoot (int drive)
{
  struct fatinfo fi; Boot *b;
  getfat(drive+1, &fi);
  if (fi.fi_sclus & 0x80) return NULL;
  if ((b = malloc(fi.fi_bysec)) == NULL) return NULL;
  // read boot sector
  if (absread(drive, 1, 0L, b) == 0) return b;
  free(b);
  return NULL;
}
