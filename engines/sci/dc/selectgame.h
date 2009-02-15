/***************************************************************************
 selectgame.h Copyright (C) 2002,2003 Walter van Niftrik


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

#ifndef __SELECTGAME_H
#define __SELECTGAME_H

/* This function changes to the directory of the game that the user wants to
** run. Currently implemented by a Dreamcast-specific graphical interface based
** on Daniel Potter's GhettoPlay interface. It also creates a config file on
** the ram disk, based on options the user has set in the interface.
** Parameters: void.
** Returns   : void.
*/
void choose_game(void);

#endif	/* __SELECTGAME_H */
