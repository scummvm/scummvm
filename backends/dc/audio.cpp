/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002  Marcus Comstedt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <common/scummsys.h>
#include <common/stdafx.h>
#include <common/engine.h>
#include "dc.h"

EXTERN_C void *memcpy4(void *s1, const void *s2, unsigned int n);

void initSound()
{
  stop_sound();
  do_sound_command(CMD_SET_FREQ_EXP(FREQ_44100_EXP));
  do_sound_command(CMD_SET_BUFFER(3));
}

bool OSystem_Dreamcast::set_sound_proc(SoundProc *proc, void *param, SoundFormat format)
{
#if SAMPLE_MODE == 0
  assert(format == SOUND_16BIT);
#elif SAMPLE_MODE == 1
  assert(format == SOUND_8BIT);
#else
#error Invalid SAMPLE_MODE
#endif
  _sound_proc_param = param;
  _sound_proc = proc;

  return true;
}

void OSystem_Dreamcast::clear_sound_proc()
{
  _sound_proc = NULL;
  _sound_proc_param = NULL;
}

void OSystem_Dreamcast::checkSound()
{
  int n;
  int curr_ring_buffer_samples;

  if(!_sound_proc)
    return;

  if(read_sound_int(&SOUNDSTATUS->mode) != MODE_PLAY)
    start_sound();

  curr_ring_buffer_samples = read_sound_int(&SOUNDSTATUS->ring_length);

  n = read_sound_int(&SOUNDSTATUS->samplepos);

  if((n-=fillpos)<0)
    n += curr_ring_buffer_samples;

  n = ADJUST_BUFFER_SIZE(n-10);

  if(n<100)
    return;

  _sound_proc(_sound_proc_param, (byte*)temp_sound_buffer,
	      SAMPLES_TO_BYTES(n));

  if(fillpos+n > curr_ring_buffer_samples) {
    int r = curr_ring_buffer_samples - fillpos;
    memcpy4(RING_BUF+fillpos, temp_sound_buffer, SAMPLES_TO_BYTES(r));
    fillpos = 0;
    n -= r;
    memcpy4(RING_BUF, temp_sound_buffer+r, SAMPLES_TO_BYTES(n));
  } else {
    memcpy4(RING_BUF+fillpos, temp_sound_buffer, SAMPLES_TO_BYTES(n));
  }
  if((fillpos += n) >= curr_ring_buffer_samples)
    fillpos = 0;
}
