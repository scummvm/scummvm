/***************************************************************************
 sci_graphics.h Copyright (C) 2000 Christoph Reichenbach


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

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

/* SCI constants and definitions */

#ifndef _SCI_GRAPHICS_H_
#define _SCI_GRAPHICS_H_

#define MAX_TEXT_WIDTH_MAGIC_VALUE 192
/* This is the real width of a text with a specified width of 0 */

#define SELECTOR_STATE_SELECTABLE 1
#define SELECTOR_STATE_FRAMED 2
#define SELECTOR_STATE_DISABLED 4
#define SELECTOR_STATE_SELECTED 8
/* Internal states */
#define SELECTOR_STATE_DITHER_FRAMED 0x1000


#endif /* !_SCI_GRAPHICS_H_ */
