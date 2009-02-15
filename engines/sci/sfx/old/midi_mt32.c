/***************************************************************************
 midi_mt32.c Copyright (C) 2000 Rickard Lind


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
#include <unistd.h>
#include <string.h>
#include "midi_mt32.h"
#include "midiout.h"

#define RHYTHM_CHANNEL 9

int midi_mt32_poke(guint32 address, guint8 *data, unsigned int n);
int midi_mt32_poke_gather(guint32 address, guint8 *data1, unsigned int count1,
			  guint8 *data2, unsigned int count2);
int midi_mt32_write_block(guint8 *data, unsigned int count);
int midi_mt32_patch001_type(guint8 *data, unsigned int length);
int midi_mt32_patch001_type0_length(guint8 *data, unsigned int length);
int midi_mt32_patch001_type1_length(guint8 *data, unsigned int length);
int midi_mt32_sysex_delay();

static guint8 *data;
static unsigned int length;
static int type;
static guint8 sysex_buffer[266] = {0xF0, 0x41, 0x10, 0x16, 0x12};

static gint8 patch_map[128] = {
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127};
static gint8 keyshift[128] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static gint8 volume_adjust[128] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static guint8 velocity_map_index[128] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static guint8 velocity_map[4][128] = {
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
   16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
   32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
   48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
   64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
   80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
   96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127},
  {0,32,32,33,33,34,34,35,35,36,36,37,37,38,38,39,
   39,40,40,41,41,42,42,43,43,44,44,45,45,46,46,47,
   47,48,48,49,49,50,50,51,51,52,52,53,53,54,54,55,
   55,56,56,57,57,58,58,59,59,60,60,61,61,62,62,63,
   64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
   80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
   96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
   16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
   32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
   48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
   64,65,66,66,67,67,68,68,69,69,70,70,71,71,72,72,
   73,73,74,74,75,75,76,76,77,77,78,78,79,79,80,80,
   81,81,82,82,83,83,84,84,85,85,86,86,87,87,88,88,
   89,89,90,90,91,91,92,92,93,93,94,94,95,95,96,96},
  {0,32,32,33,33,34,34,35,35,36,36,37,37,38,38,39,
   39,40,40,41,41,42,42,43,43,44,44,45,45,46,46,47,
   47,48,48,49,49,50,50,51,51,52,52,53,53,54,54,55,
   55,56,56,57,57,58,58,59,59,60,60,61,61,62,62,63,
   64,65,66,66,67,67,68,68,69,69,70,70,71,71,72,72,
   73,73,74,74,75,75,76,76,77,77,78,78,79,79,80,80,
   81,81,82,82,83,83,84,84,85,85,86,86,87,87,88,88,
   89,89,90,90,91,91,92,92,93,93,94,94,95,95,96,96}};
static gint8 rhythmkey_map[128] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,-1,-1,54,-1,56,-1,-1,-1,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,-1,75,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
/* static struct {
  gint8 sci_patch;
  gint8 sci_volume;
  gint8 sci_pitchwheel;
  gint8 patch;
  gint8 keyshift;
  gint8 volume_adjust;
  guint8 velocity_map_index;
  guint8 
} channel[16]; */
static guint8 master_volume;

int midi_mt32_open(guint8 *data_ptr, unsigned int data_length)
{
  guint8 unknown_sysex[6] = {0x16, 0x16, 0x16, 0x16, 0x16, 0x16};
  guint8 i, memtimbres;
  unsigned int block2, block3;

  if (midiout_open() < 0)
    return -1;

  data = data_ptr;
  length = data_length;

  type = midi_mt32_patch001_type(data, length);
  printf("MT-32: Programming Roland MT-32 with patch.001 (v%i)\n", type);

  if (type == 0) {
    /* Display MT-32 initialization message */
    printf("MT-32: Displaying Text: \"%.20s\"\n", data + 20);
    midi_mt32_poke(0x200000, data + 20, 20);
    midi_mt32_sysex_delay();
    
    /* Write Patches (48 or 96) */
    memtimbres = data[491];
    block2 = (memtimbres * 246) + 492;
    printf("MT-32: Writing Patches #01 - #32\n");
    midi_mt32_poke(0x050000, data + 107, 256);
    midi_mt32_sysex_delay();
    if ((length > block2) &&
	data[block2] == 0xAB &&
	data[block2 + 1] == 0xCD) {
      printf("MT-32: Writing Patches #33 - #64\n");
      midi_mt32_poke_gather(0x050200, data + 363, 128, data + block2 + 2, 128);  
      midi_mt32_sysex_delay();
      printf("MT-32: Writing Patches #65 - #96\n");
      midi_mt32_poke(0x050400, data + block2 + 130, 256);
      midi_mt32_sysex_delay();
      block3 = block2 + 386;
    } else {
      printf("MT-32: Writing Patches #33 - #48\n");
      midi_mt32_poke(0x050200, data + 363, 128);
      midi_mt32_sysex_delay();
      block3 = block2;
    }
    /* Write Memory Timbres */
    for (i = 0; i < memtimbres; i++) {
      printf("MT-32: Writing Memory Timbre #%02d: \"%.10s\"\n",
	     i + 1, data + 492 + i * 246);
      midi_mt32_poke(0x080000 + (i << 9), data + 492 + i * 246, 246);
      midi_mt32_sysex_delay();
    }
    /* Write Rhythm key map and Partial Reserve */
    if ((length > block3) &&
	data[block3] == 0xDC &&
	data[block3 + 1] == 0xBA) {
      printf("MT-32: Writing Rhythm key map\n");
      midi_mt32_poke(0x030110, data + block3 + 2, 256);  
      midi_mt32_sysex_delay();
      printf("MT-32: Writing Partial Reserve\n");
      midi_mt32_poke(0x100004, data + block3 + 258, 9);
      midi_mt32_sysex_delay();
    }
    /* Display MT-32 initialization done message */
    printf("MT-32: Displaying Text: \"%.20s\"\n", data);
    midi_mt32_poke(0x200000, data, 20);
    midi_mt32_sysex_delay();
    /* Write undocumented MT-32(?) SysEx */
    printf("MT-32: Writing {F0 41 10 16 12 52 00 0A 16 16 16 16 16 16 20 F7}\n");
    midi_mt32_poke(0x52000A, unknown_sysex, 6);
    midi_mt32_sysex_delay();
    return 0;
  } else if (type == 1) {
    midi_mt32_write_block(data + 1155, (data[1154] << 8) + data[1153]);
    memcpy(patch_map, data, 128);
    memcpy(keyshift, data + 128, 128);
    memcpy(volume_adjust, data + 256, 128);
    memcpy(velocity_map_index, data + 513, 128);
    for (i = 0; i < 4; i++)
      memcpy(velocity_map[i], data + 641 + i * 128, 128);
    memcpy(rhythmkey_map, data + 384, 128);
    return 0;
  }
  return -1;
}

int midi_mt32_close()
{
if (type == 0) {
  printf("MT-32: Displaying Text: \"%.20s\"\n", data + 40);
  midi_mt32_poke(0x200000, data + 40, 20);
  midi_mt32_sysex_delay();
}
return midiout_close();
}

int midi_mt32_noteoff(guint8 channel, guint8 note)
{
  return 0;
}

int midi_mt32_noteon(guint8 channel, guint8 note, guint8 velocity)
{
/*  guint8 buffer[3] = {0x90};
  if (channel == RHYTHM_CHANNEL)
    if (rhythmkey_map[note] == -1)
      return 0;
    else {
      buffer[0] |= channel
      buffer[1] = rhythmkey_map[note];
      buffer[2] = velo
      midi_write_event(buffer, 3);
    }; */
  return 0;
}

int midi_mt32_poke(guint32 address, guint8 *data, unsigned int count)
{
  guint8 checksum = 0;
  unsigned int i;

  if (count > 256) return -1;

  checksum -= (sysex_buffer[5] = (char)((address >> 16) & 0x7F));
  checksum -= (sysex_buffer[6] = (char)((address >> 8) & 0x7F));
  checksum -= (sysex_buffer[7] = (char)(address & 0x7F));

  for (i = 0; i < count; i++)
    checksum -= (sysex_buffer[i + 8] = data[i]);

  sysex_buffer[count + 8] = checksum & 0x7F;
  sysex_buffer[count + 9] = 0xF7;

  return midiout_write_block(sysex_buffer, count + 10);
}

int midi_mt32_poke_gather(guint32 address, guint8 *data1, unsigned int count1,
			  guint8 *data2, unsigned int count2)
{
  guint8 checksum = 0;
  unsigned int i;

  if ((count1 + count2) > 256) return -1;

  checksum -= (sysex_buffer[5] = (char)((address >> 16) & 0x7F));
  checksum -= (sysex_buffer[6] = (char)((address >> 8) & 0x7F));
  checksum -= (sysex_buffer[7] = (char)(address & 0x7F));

  for (i = 0; i < count1; i++)
    checksum -= (sysex_buffer[i + 8] = data1[i]);
  for (i = 0; i < count2; i++)
    checksum -= (sysex_buffer[i + 8 + count1] = data2[i]);

  sysex_buffer[count1 + count2 + 8] = checksum & 0x7F;
  sysex_buffer[count1 + count2 + 9] = 0xF7;

  return midiout_write_block(sysex_buffer, count1 + count2 + 10);
}

int midi_mt32_write_block(guint8 *data, unsigned int count)
{
  unsigned int block_start = 0;
  unsigned int i = 0;

  while (i < count) {
    if ((data[i] == 0xF0) && (i != block_start)) {
      midiout_write_block(data + block_start, i - block_start);
      block_start = i;
    }
    if (data[i] == 0xF7) {
      midiout_write_block(data + block_start, i - block_start + 1);
      midi_mt32_sysex_delay();
      block_start = i + 1;
    }
    i++;
  }
  if (count >= block_start)
    midiout_write_block(data + block_start, count - block_start);
  return 0;
}

int midi_mt32_patch001_type(guint8 *data, unsigned int length)
{
  /* length test */
  if (length < 1155)
    return 0;
  if (length > 16889)
    return 1;
  if (midi_mt32_patch001_type0_length(data, length) &&
      !midi_mt32_patch001_type1_length(data, length))
    return 0;
  if (midi_mt32_patch001_type1_length(data, length) &&
      !midi_mt32_patch001_type0_length(data, length))
    return 1;
  return -1;
}

int midi_mt32_patch001_type0_length(guint8 *data, unsigned int length)
{
  unsigned int pos = 492 + 246 * data[491];

  if ((length >= (pos + 386)) && (data[pos] == 0xAB) && (data[pos + 1] == 0xCD))
    pos += 386;
  if ((length >= (pos + 267)) && (data[pos] == 0xDC) && (data[pos + 1] == 0xBA))
    pos += 267;
  if (pos == length)
    return 1;
  return 0;
}

int midi_mt32_patch001_type1_length(guint8 *data, unsigned int length)
{
  if ((length >= 1155) && (((data[1154] << 8) + data[1153] + 1155) == length))
    return 1;
  return 0;
}

int midi_mt32_sysex_delay()
{
  usleep(320 * 63); /* One MIDI byte is 320us, 320us * 63 > 20ms */
  return 0;
}
