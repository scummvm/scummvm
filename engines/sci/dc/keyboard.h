/***************************************************************************
 keyboard.c Copyright (C) 2004 Walter van Niftrik


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


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

   Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>

***************************************************************************/

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

/* Virtual keyboard. Currently only 320x40, 16-bit colour. */

#include <sys/types.h>

#define KBD_RIGHT 0
#define KBD_LEFT 1
#define KBD_UP 2
#define KBD_DOWN 3

void vkbd_init(guint16 *ptr, int line_pitch);
/* Initialises the virtual keyboard
** Parameters: (guint16 *) ptr: buffer where the keyboard is to be drawn.
**             (int) line_pitch: line pitch of buffer in bytes.
** Returns   : (void)
*/

void vkbd_draw(void);
/* Draws the virtual keyboard in its current state.
** Parameters: (void)
** Returns   : (void)
*/

void vkbd_handle_input(int input);
/* Handle cursor moves. Keyboard display is automatically updated.
** Parameters: (int) input: cursor move to handle (see #defines above).
** Returns   : (void)
*/

int vkbd_get_key(int *data, int *buckyb);
/* Get keycode of currently selected key and buckstate. If a special key is
** selected (e.g. the shift key), it will be dealt with appropriately.
** Parameters: (int *) data: pointer to where keycode will be stored.
**             (int *) bucky: pointer to where buckystate will be stored.
** Returns   : (int) 1 when new data and bucky are available, 0 otherwise.
*/

#endif /* __KEYBOARD_H */
