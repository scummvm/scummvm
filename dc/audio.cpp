#include "stdafx.h"
#include "scumm.h"
#include "dc.h"

#include <ronin/soundcommon.h>

EXTERN_C void *memcpy4(void *s1, const void *s2, unsigned int n);

void checkSound(Scumm *s)
{
  int n;
  int curr_ring_buffer_samples;

  if(read_sound_int(&SOUNDSTATUS->mode) != MODE_PLAY)
    start_sound();

  curr_ring_buffer_samples = read_sound_int(&SOUNDSTATUS->ring_length);

  n = read_sound_int(&SOUNDSTATUS->samplepos);

  if((n-=fillpos)<0)
    n += curr_ring_buffer_samples;

  n = ADJUST_BUFFER_SIZE(n-10);

  if(n<100)
    return;

  s->mixWaves((short*)temp_sound_buffer, n);

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
