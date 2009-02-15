/***************************************************************************
 midi.h Copyright (C) 2000 Rickard Lind


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

#ifndef _MIDI_H_
#define _MIDI_H_

int midi_open(guint8 *data_ptr, unsigned int data_length);
int midi_close();

int midi_noteoff(guint8 channel, guint8 note);
int midi_noteon(guint8 channel, guint8 note, guint8 velocity);

#endif /* _MIDI_H_ */
