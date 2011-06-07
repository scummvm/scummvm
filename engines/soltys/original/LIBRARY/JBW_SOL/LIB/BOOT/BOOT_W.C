#include	<boot.h>
#include	<dos.h>


Boolean WriteBoot (int drive, Boot * boot)
{
  boot->BootCheck -= CheckBoot(boot);
  return abswrite(drive, 1, 0L, boot) == 0;
}
