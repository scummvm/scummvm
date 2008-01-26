/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "debug.h"
#include "driver_ronin.h"

#include <ronin/ronin.h>
#include <ronin/report.h>

EXTERN_C void *memcpy4s(void *s1, const void *s2, unsigned int n);

bool DriverRonin::setSoundCallback(SoundProc proc, void *param)
{
  assert(SAMPLE_MODE == 0);

  _sound_proc_param = param;
  _sound_proc = proc;

  return true;
}

void DriverRonin::clearSoundCallback()
{
  _sound_proc = NULL;
  _sound_proc_param = NULL;
  stop_sound();
}

int DriverRonin::getOutputSampleRate() const
{
	//FIXME
  /// XXX reportf("%s\n", __func__);
  return read_sound_int(&SOUNDSTATUS->freq);
}

void DriverRonin::checkSound()
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
	      2*SAMPLES_TO_BYTES(n));

  if(fillpos+n > curr_ring_buffer_samples) {
    int r = curr_ring_buffer_samples - fillpos;
    memcpy4s(RING_BUF+fillpos, temp_sound_buffer, SAMPLES_TO_BYTES(r));
    fillpos = 0;
    n -= r;
    memcpy4s(RING_BUF, temp_sound_buffer+r, SAMPLES_TO_BYTES(n));
  } else {
    memcpy4s(RING_BUF+fillpos, temp_sound_buffer, SAMPLES_TO_BYTES(n));
  }
  if((fillpos += n) >= curr_ring_buffer_samples)
    fillpos = 0;
}

void DriverRonin::initSound()
{
  _sound_proc = NULL;

  init_arm();
  stop_sound();
  do_sound_command(CMD_SET_FREQ_EXP(FREQ_22050_EXP));
  do_sound_command(CMD_SET_STEREO(1));
  do_sound_command(CMD_SET_BUFFER(SOUND_BUFFER_SHIFT));
}
