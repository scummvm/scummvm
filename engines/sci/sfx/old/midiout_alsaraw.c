/***************************************************************************
 midiout_alsaraw.c Copyright (C) 2000 Rickard Lind


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
#include <sys/asoundlib.h>
#include "midiout_alsaraw.h"

static snd_rawmidi_t *handle;

int midiout_alsaraw_open(int card, int device)
{
  int err;

  if ((err = snd_rawmidi_open(&handle, card, device, SND_RAWMIDI_OPEN_OUTPUT)) < 0) {
    fprintf(stderr, "Open failed (%i): /dev/snd/midiC%iD%i\n", err, card, device);
    return -1;
  }
  return 0;
}

int midiout_alsaraw_close()
{
  if (snd_rawmidi_close(handle) < 0)
    return -1;
  return 0;
}

int midiout_alsaraw_write(guint8 *buffer, unsigned int count)
{
  if (snd_rawmidi_write(handle, buffer, count) != count)
    return -1;
  return 0;
}
