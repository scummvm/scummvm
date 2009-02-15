/***************************************************************************
 sfx_adlib.h, from
	 midi_device.h Copyright (C) 2001 Solomon Peachy
		       Copytight (C) 2002..04 Christoph Reichenbach

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


#ifndef _SFX_ADLIB_H_
#define _SFX_ADLIB_H_

#include <resource.h>


#define ADLIB_VOICES 12

typedef struct _sci_adlib_def {
  guint8 keyscale1;       /* 0-3 !*/
  guint8 freqmod1;        /* 0-15 !*/
  guint8 feedback1;       /* 0-7 !*/
  guint8 attackrate1;     /* 0-15 !*/
  guint8 sustainvol1;     /* 0-15 !*/
  guint8 envelope1;       /* t/f !*/
  guint8 decayrate1;      /* 0-15 !*/
  guint8 releaserate1;    /* 0-15 !*/
  guint8 volume1;         /* 0-63 !*/
  guint8 ampmod1;         /* t/f !*/
  guint8 vibrato1;        /* t/f !*/
  guint8 keybdscale1;     /* t/f !*/
  guint8 algorithm1;      /* 0,1 REVERSED */
  guint8 keyscale2;       /* 0-3 !*/
  guint8 freqmod2;        /* 0-15 !*/
  guint8 feedback2;       /* 0-7 UNUSED */
  guint8 attackrate2;     /* 0-15 !*/
  guint8 sustainvol2;     /* 0-15 !*/
  guint8 envelope2;       /* t/f !*/
  guint8 decayrate2;      /* 0-15 !*/
  guint8 releaserate2;    /* 0-15 !*/
  guint8 volume2;         /* 0-63 !*/
  guint8 ampmod2;         /* t/f !*/
  guint8 vibrato2;        /* t/f !*/
  guint8 keybdscale2;     /* t/f !*/
  guint8 algorithm2;      /* 0,1 UNUSED */
  guint8 waveform1;       /* 0-3 !*/
  guint8 waveform2;       /* 0-3 !*/
} adlib_def;

typedef unsigned char adlib_instr[12];

extern adlib_instr adlib_sbi[96];

void make_sbi(adlib_def *one, guint8 *buffer);
/* Converts a raw SCI adlib instrument into the adlib register format. */

#endif /* _SFX_ADLIB_H_ */
