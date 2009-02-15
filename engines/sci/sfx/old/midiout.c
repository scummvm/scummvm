/***************************************************************************
 midiout.c Copyright (C) 2000 Rickard Lind


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
#include "midiout.h"
#include "midiout_alsaraw.h"
#include "midiout_unixraw.h"

static int (*midiout_ptr_close)();
static int (*midiout_ptr_write)(guint8 *, unsigned int);

static unsigned char running_status = 0;

int midiout_open()
{
  midiout_ptr_close = midiout_alsaraw_close;
  midiout_ptr_write = midiout_alsaraw_write;
  return midiout_alsaraw_open(0, 0);

  /*
  midiout_ptr_close = midiout_unixraw_close;
  midiout_ptr_write = midiout_unixraw_write;
  return midiout_unixraw_open("/dev/midi00");
  */
}

int midiout_close()
{
  return midiout_ptr_close();
}

int midiout_write_event(guint8 *buffer, unsigned int count)
{
  if (buffer[0] == running_status)
    return midiout_ptr_write(buffer + 1, count - 1);
  else {
    running_status = buffer[0];
    return midiout_ptr_write(buffer, count);
  }
}

int midiout_write_block(guint8 *buffer, unsigned int count)
{
  running_status = 0;
  return midiout_ptr_write(buffer, count);
}
