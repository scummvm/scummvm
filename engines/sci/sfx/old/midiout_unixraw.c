/***************************************************************************
 midiout_unixraw.c Copyright (C) 2000 Rickard Lind


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

#include "glib.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "midiout_unixraw.h"

static int fd;

int midiout_unixraw_open(char *devicename)
{
  if (!IS_VALID_FD(fd = open(devicename, O_WRONLY|O_SYNC))) {
    fprintf(stderr, "Open failed (%i): %s\n", fd, devicename);
    return -1;
  }
  return 0;
}

int midiout_unixraw_close()
{
  if (close(fd) < 0)
    return -1;
  return 0;
}

int midiout_unixraw_write(guint8 *buffer, unsigned int count)
{
  if (write(fd, buffer, count) != count)
    return -1;
  return 0;
}
