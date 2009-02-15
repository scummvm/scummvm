/***************************************************************************
 adlib.c Copyright (C) 2001 Solomon Peachy

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

#include "sci/sfx/adlib.h"

adlib_instr adlib_sbi[96];

void
make_sbi(adlib_def *one, guint8 *buffer) {
	memset(buffer, 0, sizeof(adlib_instr));

#if 0
	printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ", one->keyscale1, one->freqmod1, one->feedback1, one->attackrate1, one->sustainvol1, one->envelope1, one->decayrate1, one->releaserate1, one->volume1, one->ampmod1, one->vibrato1, one->keybdscale1, one->algorithm1, one->waveform1);

	printf(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ", one->keyscale2, one->freqmod2, one->feedback2, one->attackrate2, one->sustainvol2, one->envelope2, one->decayrate2, one->releaserate2, one->volume2, one->ampmod2, one->vibrato2, one->keybdscale2, one->algorithm2, one->waveform2);

	printf("\n");
#endif

	buffer[0] |= ((one->ampmod1 & 0x1) << 7);
	buffer[0] |= ((one->vibrato1 & 0x1) << 6);
	buffer[0] |= ((one->envelope1 & 0x1) << 5);
	buffer[0] |= ((one->keybdscale1 & 0x1) << 4);
	buffer[0] |= (one->freqmod1 & 0xf);
	buffer[1] |= ((one->ampmod2 & 0x1) << 7);
	buffer[1] |= ((one->vibrato2 & 0x1) << 6);
	buffer[1] |= ((one->envelope2 & 0x1) << 5);
	buffer[1] |= ((one->keybdscale2 & 0x1) << 4);
	buffer[1] |= (one->freqmod2 & 0xf);
	buffer[2] |= ((one->keyscale1 & 0x3) << 6);
	buffer[2] |= (one->volume1 & 0x3f);
	buffer[3] |= ((one->keyscale2 & 0x3) << 6);
	buffer[3] |= (one->volume2 & 0x3f);
	buffer[4] |= ((one->attackrate1 & 0xf) << 4);
	buffer[4] |= (one->decayrate1 & 0xf);
	buffer[5] |= ((one->attackrate2 & 0xf) << 4);
	buffer[5] |= (one->decayrate2 & 0xf);
	buffer[6] |= ((one->sustainvol1 & 0xf) << 4);
	buffer[6] |= (one->releaserate1 & 0xf);
	buffer[7] |= ((one->sustainvol2 & 0xf) << 4);
	buffer[7] |= (one->releaserate2 & 0xf);
	buffer[8] |= (one->waveform1 & 0x3);
	buffer[9] |= (one->waveform2 & 0x3);

	buffer[10] |= ((one->feedback1 & 0x7) << 1);
	buffer[10] |= (1 - (one->algorithm1 & 0x1));

	return;
}
