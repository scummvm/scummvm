/***************************************************************************
 dc.h Copyright (C) 2002,2003 Walter van Niftrik


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

#ifndef __DC_H
#define __DC_H

#include <stdio.h>

/* Functions implemented in dc_save.c */

/* Returns a string with the VFS path to the first VMU.
** Parameters: void.
** Returns   : Pointer to VFS path string on success, NULL on error.
*/
char *dc_get_first_vmu();

/* Constructs the save game filename as it'll go on the VMU.
** Parameters: (char *) game_name: Game id of the current game.
**             (int) nr: The current save game number.
** Returns   : Pointer to save game name.
*/
char *dc_get_cat_name(char *game_name, int nr);

/* Deletes all save game files from a directory.
** Parameters: (char *) dir: Path of the directory to delete the save game
**                files from.
** Returns   : void.
*/
void dc_delete_save_files(char *dir);

/* Deletes the temporary file which is used for constructing the save files.
** Parameters: void.
** Returns   : void.
*/
int dc_delete_temp_file();

/* Retrieves a save game from the first VMU and puts it on the ram disk.
** Parameters: (char *) game_name: Game id of the current game.
**             (int) nr: The number of the save game to retrieve.
** Returns   : 0 on success, -1 on error.
*/
int dc_retrieve_savegame(char *game_name, int nr);

/* Stores a save game from the ram disk on the first VMU.
** Parameters: (char *) game_name: Game id of the current game.
**             (char *) desc: Description of the save game.
**             (int) nr: The number of the save game to store.
** Returns   : 0 on success, -1 on error.
*/
int dc_store_savegame(char *game_name, char *desc, int nr);

/* Retrieves the mirrored files from the first VMU to the ram disk.
** Parameters: (char *) game_name: Game id of the current game.
** Returns   : 0 on success, -1 on error.
*/
int dc_retrieve_mirrored(char *game_name);

/* Stores the mirrored files from the ram disk on the first VMU.
** Parameters: (char *) game_name: Game id of the current game.
** Returns   : 0 on success, -1 on error.
*/
int dc_store_mirrored(char *game_name);

#endif  /* __DC_H */
