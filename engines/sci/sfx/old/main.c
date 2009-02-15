/***************************************************************************
 main.c Copyright (C) 2000 Rickard Lind


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.

***************************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "midi_mt32.h"

int main()
{
  int fd;
  unsigned char *patch;
  unsigned int length;

  patch = (unsigned char *)sci_malloc(65536);

  fd = open("patch.001", O_RDONLY);
  length = read(fd, patch, 65536);
  close(fd);

  if (patch[0] == 0x89 && patch[1] == 0x00)
    midi_mt32_open(patch + 2, length - 2);
  else
    midi_mt32_open(patch, length);

  midi_mt32_close();

  free(patch);
  return 0;
}
